/**********************************************************************************************************/
/* Commun/Reseaux/Reseaux.c:   Utilisation/Gestion du reseaux pour watchdog 2.0    par lefevre            */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 16 fév 2008 19:19:49 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

 #include <glib.h>
 #include <openssl/ssl.h>
 #include <sys/types.h>                                                     /* Prototypes pour read/write */
 #include <sys/stat.h>
 #include <sys/time.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <stdio.h>
 #include <errno.h>
 #include <signal.h>

 #include "Reseaux.h"
 #include "Erreur.h"

/**********************************************************************************************************/
/* Attendre_envoi_disponible: Attend que le reseau se libere pour envoi futur                             */
/* Entrée: log, connexion                                                                                 */
/* Sortie: Code d'erreur le cas echeant                                                                   */
/**********************************************************************************************************/
 gint Attendre_envoi_disponible ( struct LOG *Log, struct CONNEXION *connexion )
  { gint retour, cpt;
    struct timeval tv;
    fd_set fd;

    if (!connexion) return(0);
    cpt = 0;
one_again:
    FD_ZERO(&fd);
    FD_SET( connexion->socket, &fd );
    tv.tv_sec=TIMEOUT_BUFFER_PLEIN;
    tv.tv_usec=0;
    retour = select( connexion->socket+1, NULL, &fd, NULL, &tv );
    if (retour==-1)
     { gint err;
       err = errno;
       Info_c( Log, DEBUG_NETWORK, "Attendre_envoi_disponible: erreur select", strerror(errno) );
       Info_n( Log, DEBUG_NETWORK, "                              code errno", err );
       switch(err)
        { case EINTR: if (cpt<10) { cpt++; goto one_again; }
                      else Info( Log, DEBUG_NETWORK,
                           "Attendre_envoi_disponible: Sortie apres 10 Interrupt Call." );
                      break;
        }
       return(err);
     }
    if (retour==0)                                   /* Traiter ensuite les signaux du genre relais brisé */
     { Info( Log, DEBUG_NETWORK, "Attendre_envoi_disponible: select timeout" ); return(-1); }
    return(0);
  }
/**********************************************************************************************************/
/* Nouvelle_connexion: Prepare la structure CONNEXION pour pouvoir envoyer et recevoir des données        */
/* Entrée: socket distante, l'id de l'appli hote                                                          */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 struct CONNEXION *Nouvelle_connexion ( struct LOG *Log, gint socket, gint emetteur,
                                        gint taille_bloc )
  { struct CONNEXION *connexion;

    connexion = g_malloc0( sizeof(struct CONNEXION) );
    if (!connexion) { Info( Log, DEBUG_MEM, "Nouvelle_connexion: not enought memory" );
                      return(NULL);
                    }

    connexion->donnees = g_malloc0( taille_bloc );
    if (!connexion->donnees)
     { Info( Log, DEBUG_MEM, "Nouvelle_connexion: not enought memory (buffer)" );
       g_free(connexion);
       return(NULL);
     }

    fcntl( socket, F_SETFL, O_NONBLOCK );
    connexion->index_entete  = 0;
    connexion->index_donnees = 0;
    connexion->socket        = socket;                   /* Sauvegarde de la socket pour ecoute prochaine */
    connexion->emetteur      = emetteur;
    connexion->taille_bloc   = taille_bloc;
    connexion->ssl           = NULL;
    return(connexion);
  }

/**********************************************************************************************************/
/* Fermer_connexion: Libere la mémoire et ferme la socket associé à une connexion                         */
/* Entrée: le connexion de reference                                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Fermer_connexion ( struct CONNEXION *connexion )
  { 
    if (!connexion) return;

    if (connexion->donnees) g_free(connexion->donnees);
    if (connexion->ssl) { SSL_shutdown( connexion->ssl );
                          SSL_free( connexion->ssl );
                        }
    close ( connexion->socket );
    g_free(connexion);
  }
/**********************************************************************************************************/
/* Recevoir_reseau: Ecoute la connexion en parametre et essai de reunir un bloc entier de donnees         */
/* Entrée: la connexion                                                                                   */
/* Sortie: -1 si erreur, 0 si kedal, le code de controle sinon                                            */
/**********************************************************************************************************/
 gint Recevoir_reseau( struct LOG *Log, struct CONNEXION *connexion )
  { int taille_recue;
  
    if (!connexion) return( RECU_ERREUR );

    if ( connexion->index_entete != sizeof(struct ENTETE_CONNEXION) )               /* Entete complete ?? */
     { if (connexion->ssl)
        { taille_recue = SSL_read( connexion->ssl,
                                   ((unsigned char *)&connexion->entete ) + connexion->index_entete,
                                   sizeof(struct ENTETE_CONNEXION)-connexion->index_entete
                                 );
        }
       else
        { taille_recue = read( connexion->socket,
                               ((unsigned char *)&connexion->entete ) + connexion->index_entete,
                               sizeof(struct ENTETE_CONNEXION)-connexion->index_entete
                             );
        }

       if (taille_recue==0) return(RECU_RIEN);
       if (taille_recue<0)
        { int err;
          err=errno;
          switch (err)
           { case EPIPE     :
             case ECONNRESET: return( RECU_ERREUR_CONNRESET );
             case ESPIPE    : return( RECU_ERREUR );                                  /* Reperage illegal */
             case EAGAIN    : return( RECU_RIEN );               /* Ressource temporairement indisponible */
           }
          printf( "recv_rezo, socket %d Errno=%d %s taille %d\n", 
                  connexion->socket, err, strerror(err), taille_recue );
          return(RECU_RIEN);
        } else Info_n( Log, DEBUG_NETWORK, "taille entete recue", taille_recue );

       connexion->index_entete += taille_recue;                                 /* Indexage pour la suite */

       if (connexion->index_entete == sizeof(struct ENTETE_CONNEXION))
        { Info_n( Log, DEBUG_NETWORK, "Recu paquet de", connexion->socket );
          Info_n( Log, DEBUG_NETWORK, "           tag", connexion->entete.tag );
          Info_n( Log, DEBUG_NETWORK, "        ss_tag", connexion->entete.ss_tag );
          Info_n( Log, DEBUG_NETWORK, "       donnees", connexion->entete.taille_donnees );

          if ( connexion->entete.taille_donnees>connexion->taille_bloc )
           { connexion->entete.taille_donnees=connexion->taille_bloc;
             Info_n( Log, DEBUG_NETWORK, "Paquet trop grand !!\n", connexion->socket );
           }
        }
     }
    else                                    /* Ok, on a l'entete parfaite, maintenant fo voir les donnees */
     { if ( connexion->index_donnees == connexion->entete.taille_donnees )
        { connexion->index_entete  = 0;                                                 /* Raz des indexs */
          connexion->index_donnees = 0;
          Info_n( Log, DEBUG_NETWORK, " recue donnees", connexion->entete.taille_donnees );
          return(RECU_OK);
        }
       if (connexion->ssl)
        { taille_recue = SSL_read( connexion->ssl,
                                   ((unsigned char *)connexion->donnees ) + connexion->index_donnees,
                                   connexion->entete.taille_donnees-connexion->index_donnees
                                 );
        }
       else
        { taille_recue = read( connexion->socket,
                               ((unsigned char *)connexion->donnees ) + connexion->index_donnees,
                               connexion->entete.taille_donnees-connexion->index_donnees
                             );
        }

       if (taille_recue<=0)
        { int err;
          err=errno;
          switch (err)
           { case ECONNRESET: return( RECU_ERREUR_CONNRESET );
             case ESPIPE    : return( RECU_ERREUR );                                  /* Reperage illegal */
             case EAGAIN    : return( RECU_RIEN );               /* Ressource temporairement indisponible */
           }
          perror("recevoir_reseau (taille=-1 donnees)");
          printf("Errno=%d  %d %d %d\n", err, EAGAIN, EINTR, ESPIPE );
          return(RECU_RIEN);
        }
       connexion->index_donnees += taille_recue;                                /* Indexage pour la suite */
     }
    return( RECU_EN_COURS );
  }
/**********************************************************************************************************/
/* Envoi_paquet: Transmet un paquet reseau au client id                                                   */
/* Entrée: la socket, l'entete, le buffer                                                                 */
/* Sortie: non nul si pb                                                                                  */
/**********************************************************************************************************/
 gint Envoyer_reseau( struct LOG *Log, struct CONNEXION *connexion, gint destinataire, gint tag, gint ss_tag,
                      gchar *buffer, gint taille_totale )
  { struct ENTETE_CONNEXION Entete;
    gint err, retour, cpt;

    if (!connexion) return(-1);

    if ( (retour=Attendre_envoi_disponible(Log, connexion)) )
     { Info_n( Log, DEBUG_NETWORK, "Envoyer_reseau: timeout depassé (ou erreur)", connexion->socket );
       Info_n( Log, DEBUG_NETWORK, "                             code de retour", retour );
       return(retour);
     }

    /*Entete.emetteur       = connexion->emetteur;                             /* Recopie dans la structure */
    /*Entete.destinataire   = destinataire;*/
    Entete.tag            = tag;
    Entete.ss_tag         = ss_tag;
    Entete.taille_donnees = taille_totale;

    Info_n( Log, DEBUG_NETWORK, "Envoi du paquet au client", connexion->socket );
    Info_n( Log, DEBUG_NETWORK, "                      tag", tag );
    Info_n( Log, DEBUG_NETWORK, "                  donnees", taille_totale );

    cpt = sizeof(struct ENTETE_CONNEXION);
    while(cpt)
     { if (connexion->ssl)
        { retour = SSL_write( connexion->ssl, &Entete, cpt ); }                      /* Envoi de l'entete */
       else retour = write( connexion->socket, &Entete, cpt );                       /* Envoi de l'entete */

       if (retour<=0)
        { err = errno;
          Info_n( Log, DEBUG_NETWORK, "Envoyer_reseau: code de retour", err );
          Info_c( Log, DEBUG_NETWORK, "              chaine de retour", strerror(err) );
          if (connexion->ssl)
           { Info_n( Log, DEBUG_NETWORK, " retour SSL", SSL_get_error( connexion->ssl, retour ) );
           }
          return(err);
        } else Info_n( Log, DEBUG_NETWORK, "donnees envoyees entete", retour );
          
       cpt -= retour;
     }

    if (buffer)                                                        /* Preparation de l'envoi du buffer */
     { /*cpt = Entete.taille_donnees;*/
       cpt = 0;
       while(cpt < Entete.taille_donnees)
        {

          if ( (retour=Attendre_envoi_disponible(Log, connexion)) )
           { Info_n( Log, DEBUG_NETWORK, "Envoyer_reseau: timeout depassé (ou erreur)", connexion->socket );
             Info_n( Log, DEBUG_NETWORK, "                             code de retour", retour );
             return(retour);
           }

          if (connexion->ssl)
           { retour = SSL_write( connexion->ssl, buffer + cpt, Entete.taille_donnees-cpt ); }/* Envoi du buffer */
          else retour = write( connexion->socket, buffer + cpt, Entete.taille_donnees-cpt );
          
          err = errno;
          if (retour<=0)
           { Info_n( Log, DEBUG_NETWORK, "Envoyer_reseau: code de retour", err );
             Info_c( Log, DEBUG_NETWORK, "              chaine de retour", strerror(err) );
             return(err);
           } else Info_n( Log, DEBUG_NETWORK, "donnees envoyees buffer", retour );
          cpt += retour;
        }
     }
    return(0);
  }
/**********************************************************************************************************/
/* Reseau_tag: Renvoi le numero de tag correspondant au paquet recu                                       */
/* Entrée: la connexion                                                                                   */
/**********************************************************************************************************/
 gint Reseau_tag( struct CONNEXION *connexion )
  { if (!connexion) return(0);
    return( connexion->entete.tag );
  }
/**********************************************************************************************************/
/* Reseau_ss_tag: Renvoi le numero de ss_tag correspondant au paquet recu                                 */
/* Entrée: la connexion                                                                                   */
/**********************************************************************************************************/
 gint Reseau_ss_tag( struct CONNEXION *connexion )
  { if (!connexion) return(0);
    return( connexion->entete.ss_tag );                                 /* On teste le buffer avec le tag */
  }
/*--------------------------------------------------------------------------------------------------------*/
