/**********************************************************************************************************/
/* Watchdogd/Serveur/serveur.c                Comportement d'un sous-serveur Watchdog                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                       jeu 02 fév 2006 13:01:57 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Serveur.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2007 - Sébastien Lefevre
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
 #include <bonobo/bonobo-i18n.h>
 #include <openssl/err.h>
 #include <openssl/rsa.h>
 #include <openssl/x509.h>
 #include <signal.h>
 #include <unistd.h>
 #include <time.h>
 #include <openssl/ssl.h>
 #include <openssl/rand.h>
 #include <sys/socket.h>
 #include <sys/prctl.h>
 #include <netinet/tcp.h>
 #include <netinet/in.h>                                          /* Pour les structures d'entrées SOCKET */
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <fcntl.h>
 #include <string.h>
 #include <pthread.h>

 #include "Reseaux.h"
 #include "Config.h"
 #include "Client.h"
 #include "Utilisateur_DB.h"
 #include "watchdogd.h"

 extern gint Socket_ecoute;                                  /* Socket de connexion (d'écoute) du serveur */
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */

/******************************************** Prototypes de fonctions *************************************/
 #include "prototype.h"                                                          /* Acces à Connecter_ssl */
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Ref_client et Unref_client servent a referencer ou non une structure CLIENT en mémoire                 */
/* Entrée: un client                                                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Unref_client ( struct CLIENT *client )
  {
    pthread_mutex_lock( &client->mutex_struct_used );
    if (client->struct_used) client->struct_used--;
    pthread_mutex_unlock( &client->mutex_struct_used );
  };
/**********************************************************************************************************/
/* Ref_client et Unref_client servent a referencer ou non une structure CLIENT en mémoire                 */
/* Entrée: un client                                                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Ref_client ( struct CLIENT *client )
  {
    pthread_mutex_lock( &client->mutex_struct_used );
    client->struct_used++;
    pthread_mutex_unlock( &client->mutex_struct_used );
  };
/**********************************************************************************************************/
/* Mode_vers_string: Conversion d'un mode vers une chaine de caracteres                                   */
/* Entrée: un mode                                                                                        */
/* Sortie: un gchar *                                                                                     */
/**********************************************************************************************************/
 static gchar *Mode_vers_string ( gint mode )
  { switch (mode)
     { case ATTENTE_CONNEXION_SSL       : return("ATTENTE_CONNEXION_SSL");
       case ATTENTE_IDENT               : return("ATTENTE_IDENT");
       case ENVOI_AUTORISATION          : return("ENVOI_AUTORISATION");
       case ATTENTE_NEW_PASSWORD        : return("ATTENTE_NEW_PASSWORD");
       case ENVOI_DONNEES               : return("ENVOI_DONNEES");
       case ENVOI_HISTO                 : return("ENVOI_HISTO");
       case ENVOI_INHIB                 : return("ENVOI_INHIB");
       case ENVOI_PALETTE               : return("ENVOI_PALETTE");

       case ENVOI_GROUPE                : return("ENVOI_GROUPE");
       case ENVOI_UTIL                  : return("ENVOI_UTIL");
       case ENVOI_GROUPE_FOR_UTIL       : return("ENVOI_GROUPE_FOR_UTIL");
       case ENVOI_DLS                   : return("ENVOI_DLS");
       case ENVOI_SOURCE_DLS            : return("ENVOI_SOURCE_DLS");
       case ENVOI_MESSAGE               : return("ENVOI_MESSAGE");
       case ENVOI_SYNOPTIQUE            : return("ENVOI_SYNOPTIQUE");
       case ENVOI_MNEMONIQUE            : return("ENVOI_MNEMONIQUE");
       case ENVOI_MNEMONIQUE_FOR_COURBE : return("ENVOI_MNEMONIQUE_FOR_COURBE");
       case ENVOI_MNEMONIQUE_FOR_HISTO_COURBE : return("ENVOI_MNEMONIQUE_FOR_HISTO_COURBE");
       case ENVOI_MOTIF_ATELIER         : return("ENVOI_MOTIF_ATELIER");
       case ENVOI_COMMENT_ATELIER       : return("ENVOI_COMMENT_ATELIER");
       case ENVOI_PASSERELLE_ATELIER    : return("ENVOI_PASSERELLE_ATELIER");
       case ENVOI_PALETTE_ATELIER       : return("ENVOI_PALETTE_ATELIER");
       case ENVOI_CAPTEUR_ATELIER       : return("ENVOI_CAPTEUR_ATELIER");
       case ENVOI_COMMENT_SUPERVISION   : return("ENVOI_COMMENT_SUPERVISION");
       case ENVOI_MOTIF_SUPERVISION     : return("ENVOI_MOTIF_SUPERVISION");
       case ENVOI_PASSERELLE_SUPERVISION: return("ENVOI_PASSERELLE_SUPERVISION");
       case ENVOI_PALETTE_SUPERVISION   : return("ENVOI_PALETTE_SUPERVISION");
       case ENVOI_CAPTEUR_SUPERVISION     : return("ENVOI_CAPTEUR_SUPERVISION");
       case ENVOI_IXXX_SUPERVISION      : return("ENVOI_IXXX_SUPERVISION");
       case ENVOI_GROUPE_FOR_SYNOPTIQUE : return("ENVOI_GROUPE_FOR_SYNOPTIQUE");
       case ENVOI_ICONE                 : return("ENVOI_ICONE");
       case ENVOI_SCENARIO              : return("ENVOI_SCENARIO");
       case ENVOI_SCENARIO_SUP          : return("ENVOI_SCENARIO_SUP");
       case ENVOI_CLASSE                : return("ENVOI_CLASSE");
       case ENVOI_HISTO_HARD            : return("ENVOI_HISTO_HARD");
       case ENVOI_ENTREEANA             : return("ENVOI_ENTREEANA");
       case ENVOI_ENTREEANA_FOR_COURBE  : return("ENVOI_ENTREEANA_FOR_COURBE");
       case ENVOI_ENTREEANA_FOR_HISTO_COURBE  : return("ENVOI_ENTREEANA_FOR_HISTO_COURBE");
       
       case ENVOI_CLASSE_FOR_ATELIER    : return("ENVOI_CLASSE_FOR_ATELIER");
       case ENVOI_ICONE_FOR_ATELIER     : return("ENVOI_ICONE_FOR_ATELIER");
       case ENVOI_SYNOPTIQUE_FOR_ATELIER: return("ENVOI_SYNOPTIQUE_FOR_ATELIER");
       case ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE
                                        : return("ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE");
       case ENVOI_PALETTE_FOR_ATELIER_PALETTE
                                        : return("ENVOI_PALETTE_FOR_ATELIER_PALETTE");

       case ENVOI_GIF                   : return("ENVOI_GIF");

       case VALIDE_NON_ROOT             : return("VALIDE_NON_ROOT");
       case VALIDE                      : return("VALIDE");
       case DECONNECTE                  : return("DECONNECTE");
     }
    return("Inconnu");
  }
/**********************************************************************************************************/
/* Client_mode: Mise d'un client dans un mode precis                                                      */
/* Entrée: un client et un mode                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Client_mode ( struct CLIENT *client, gint mode )
  { gchar chaine[80];

    if (client->mode == VALIDE_NON_ROOT && mode == VALIDE)                    /* Nous prevenons le client */
     { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_CLI_VALIDE, NULL, 0 ); }
    client->mode = mode;
    g_snprintf( chaine, sizeof(chaine), _("SSRV: client mode %s"), Mode_vers_string(mode) );
    if (!client->util) Info_n( Config.log, DEBUG_CONNEXION, chaine, client->connexion->socket );
                  else Info_c( Config.log, DEBUG_CONNEXION, chaine, client->util->nom );
  }
/**********************************************************************************************************/
/* Deconnecter: Deconnection d'un client                                                                  */
/* Entrée: un client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Deconnecter ( struct CLIENT *client )
  {
    client->mode = VALIDE;                            /* Envoi un dernier paquet "OFF" avant deconnexion" */
    Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_OFF, NULL, 0 );
    client->mode = DECONNECTE;

    Partage->Sous_serveur[client->Id_serveur].Clients = g_list_remove( Partage->Sous_serveur[client->Id_serveur].Clients, client );
    Fermer_connexion( client->connexion );
    pthread_mutex_destroy( &client->mutex_write );
    pthread_mutex_destroy( &client->mutex_struct_used );
    Info_n( Config.log, DEBUG_NETWORK, _("SSRV: Deconnecter: Connexion stopped"), client->connexion->socket );
    if (client->util) { g_free( client->util ); }
    if (client->bit_syns)     { g_list_free(client->bit_syns); }
    if (client->bit_init_syn) { g_list_free(client->bit_init_syn); }
    if (client->bit_capteurs) { g_list_foreach( client->bit_capteurs, (GFunc) g_free, NULL );
                                g_list_free(client->bit_capteurs);
                              }
    if (client->bit_init_capteur)
                              { g_list_foreach( client->bit_init_capteur, (GFunc) g_free, NULL );
                                g_list_free(client->bit_init_capteur);
                              }
    if (client->courbes)      { g_list_foreach( client->courbes, (GFunc)g_free, NULL );
                                g_list_free(client->courbes);
                              }
    if (client->Db_watchdog)  { DeconnexionDB( Config.log, &client->Db_watchdog ); }    /* Deconnexion DB */
    g_free( client );
  }
/**********************************************************************************************************/
/* Accueillir_nouveaux_clients: Cette fonction permet de loguer d'éventuels nouveaux clients distants     */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE si un nouveau client est arrivé                                                           */
/**********************************************************************************************************/
 static gboolean Accueillir_un_client( guint ss_id )
  { struct sockaddr_in distant;
    guint taille_distant, id;
    struct CLIENT *client;
    struct hostent *host;
    gchar nom[16];

    g_snprintf(nom, sizeof(nom), "Watchdogd-SRV%d", ss_id );
    prctl(PR_SET_NAME, nom, 0, 0, 0 );

    if (!Socket_ecoute)                       /* Si la connexion n'est pas activée, on ne peut rien faire */
     { Info_n( Config.log, DEBUG_CONNEXION, "SSRV: Accueillir_nouveaux_client: Socket_ecoute=NULL -> return", id );
       return(FALSE);
     }
    taille_distant = sizeof(distant);
    if ( (id=accept( Socket_ecoute, (struct sockaddr *)&distant, &taille_distant )) != -1)  /* demande ?? */
     { Info_n( Config.log, DEBUG_CONNEXION, "SSRV: Accueillir_nouveaux_client: Connexion wanted. ID", id );

       client = g_malloc0( sizeof(struct CLIENT) );      /* On alloue donc une nouvelle structure cliente */
       if (!client) { Info_n ( Config.log, DEBUG_MEM,
                               "SSRV: Accueillir_nouveaux_client: Not enought memory to connect", id );
                      close(id);
                      return(FALSE);                                    /* On traite bien sûr les erreurs */
                    }

       client->connexion = Nouvelle_connexion( Config.log, id,
                                               W_SERVEUR, Config.taille_bloc_reseau );
       if (!client->connexion)
        { Info_n( Config.log, DEBUG_MEM, "SSRV: Accueillir_nouveaux_client: Not enought memory", id );
          close(id);
          g_free( client );
          return(FALSE);
        }

       host = gethostbyaddr( (char*)&distant.sin_addr, sizeof(distant.sin_addr), AF_INET );/*InfosClients */
       if (host) { g_snprintf( client->machine, TAILLE_MACHINE, "%s", host->h_name ); }   /* Nom en clair */
            else { g_snprintf( client->machine, TAILLE_MACHINE, "%s",               /* Ou bien Adresse IP */
                               (gchar *)inet_ntoa(distant.sin_addr) );
                 }
       time( &client->seconde );                       /* Enregistrement de la date de debut de connexion */
       client->timeout = client->seconde + Config.timeout_connexion;
       client->Id_serveur = ss_id;
       client->courbe.id = -1;
       pthread_mutex_init( &client->mutex_write, NULL );
       pthread_mutex_init( &client->mutex_struct_used, NULL );
       client->struct_used = 0;                            /* Par défaut, personne n'utilise la structure */

       client->Db_watchdog = ConnexionDB( Config.log, Config.db_name,
                                          Config.db_admin_username, Config.db_password );
       if (!client->Db_watchdog)
        { Info_c( Config.log, DEBUG_DB,
                  _("SSRV: Accueillir_nouveaux_client: Unable to open database (dsn)"), Config.db_name );
          Deconnecter( client );
        }
       else
        { Partage->Sous_serveur[ss_id].Clients = g_list_append( Partage->Sous_serveur[ss_id].Clients, client );
          Info_n( Config.log, DEBUG_CONNEXION, "SSRV: Connexion acceptée ID", id);
          Info_c( Config.log, DEBUG_CONNEXION, "SSRV: ------------- Machine", client->machine );
          Partage->Sous_serveur[ss_id].nb_client++;                      /* Nous gerons un client de plus !! */
          Client_mode( client, ATTENTE_CONNEXION_SSL );     /* On attend que le client demande crypté ou non */
        }
       return(TRUE);
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Run_serveur boucle principale d'un serveur Watchdog                                                    */
/* Entree: l'id du serveur et le pid du pere                                                              */
/* Sortie: un code d'erreur EXIT_xxx                                                                      */
/**********************************************************************************************************/
 void Run_serveur ( gint id )
  { time_t version_d_serveur;
    struct sigaction sig;
    guint Top, Arret;
    pthread_t tid;

    sig.sa_handler = SIG_IGN;
    sig.sa_flags = SA_RESTART;        /* Voir Linux mag de novembre 2002 pour le flag anti cut read/write */
    sigaction( SIGINT, &sig, NULL );                                               /* On ignore le SIGINT */

    Top = time(NULL);                                                        /* On prend l'heure actuelle */
    Arret = 0;
    Partage->Sous_serveur[id].Clients = NULL;                     /* Au départ, nous n'avons aucun client */

                                                  /* Initialisation de la zone interne et comm du serveur */
    Partage->Sous_serveur[id].nb_client = 0;
   /* Partage->Sous_serveur[id].pid = getpid(); /*pthread_self(); /* Le fils est pret et en informe le pere */

    Info_n( Config.log, DEBUG_INFO, _("SSRV: Run_serveur: Enable"), id );
         
    while( Partage->Arret < FIN && Arret != FIN )  /* On tourne tant que le pere est en vie et arret!=fin */
     { if (Partage->jeton == id)                                                /* Avons nous le jeton ?? */
        { if (Accueillir_un_client( id ) == TRUE)                         /* Un client vient d'arriver ?? */
           { Partage->jeton = -1;                                /* On signale que l'on accepte le client */
             Info_n( Config.log, DEBUG_INFO, _("SSRV: Run_serveur: jeton rendu"), id );
           }
        }

       if (Partage->Sous_serveur[id].sigusr1)                                 /* Gestion des signaux USR1 */
        { Partage->Sous_serveur[id].sigusr1 = FALSE;
          Info_n( Config.log, DEBUG_INFO, "SSRV: Run_serveur: SIGUSR1", id );
        }

       if (Partage->Sous_serveur[id].Clients)                                    /* Si il y a des clients */
        { gint new_mode;
          GList *liste;

          Top = time(NULL);
          liste = Partage->Sous_serveur[id].Clients;
          while(liste)                                                /* Parcours de la liste des clients */
           { struct CLIENT *client;
             client = (struct CLIENT *)liste->data;

             if (client->mode == DECONNECTE && client->struct_used == 0)      /* Deconnection des clients */
              { Deconnecter( client );
                if (Partage->Sous_serveur[id].nb_client)                 /* Nous avons un client de moins */
                 { Partage->Sous_serveur[id].nb_client--; }
                break;
              }

             switch (client->mode)
              { case ATTENTE_CONNEXION_SSL: Connecter_ssl ( client ); /* Tentative de connexion securisée */
                                            break;
                case ENVOI_AUTORISATION : new_mode = Tester_autorisation( id, client );
                                          if (new_mode == ENVOI_DONNEES)/* Optimisation si pas necessaire */
                                           { version_d_serveur = Lire_version_donnees( Config.log );
                                             if ( version_d_serveur > client->ident.version_d )
                                              {  gint taille;
                                                 taille = 0;
                                                 taille += Ajouter_repertoire_liste( client, "Gif",
                                                                                     client->ident.version_d );
                                                 client->transfert.taille = taille;
                                                 Client_mode (client, ENVOI_GIF );
                                                 break;
                                              }
                                           }
                                          Client_mode ( client, new_mode );
                                          break;
                case ENVOI_GIF          : if(Envoyer_gif( client )) Client_mode (client, ENVOI_HISTO);
                                          break;
                case ENVOI_DONNEES      : /*if (Envoyer_donnees( client ))*/ Client_mode (client, ENVOI_HISTO);
                                          break;
                case ENVOI_HISTO        : Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_histo_thread, client );
                                          pthread_detach( tid );
                                          Client_mode( client, VALIDE_NON_ROOT );
                                          break;
                case ENVOI_INHIB        : /*if(Envoyer_inhib( client ))*/ Client_mode (client, ENVOI_PALETTE);
                                          break;
                case ENVOI_PALETTE      : /*if(Envoyer_palette( client ))*/ Client_mode (client, VALIDE_NON_ROOT);
                                          break;

                case VALIDE_NON_ROOT    : /*Client_mode(client, VALIDE); */           /* Etat transitoire */
                                          break;

                case ENVOI_GROUPE       : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_groupes_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_GROUPE_FOR_UTIL:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_groupes_pour_util_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_GROUPE_FOR_SYNOPTIQUE:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_groupes_pour_synoptique_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_GROUPE_FOR_PROPRIETE_SYNOPTIQUE:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                  (void *)Envoyer_groupes_pour_propriete_synoptique_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_UTIL         : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_utilisateurs_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_DLS          : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_plugins_dls_thread, client );
                                          pthread_detach( tid );
                                          break;

                case ENVOI_SOURCE_DLS   : if(Envoyer_source_dls( client )) Client_mode(client, VALIDE);
                                          break;
                case ENVOI_MESSAGE      : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_messages_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_SYNOPTIQUE   : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_synoptiques_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_MNEMONIQUE   : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_mnemoniques_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_MNEMONIQUE_FOR_COURBE:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_mnemoniques_for_courbe_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_MNEMONIQUE_FOR_HISTO_COURBE:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_mnemoniques_for_histo_courbe_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_MOTIF_ATELIER: Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_motif_atelier_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_COMMENT_ATELIER:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_comment_atelier_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_PASSERELLE_ATELIER:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_passerelle_atelier_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_PALETTE_ATELIER:
                                          Client_mode( client, ENVOI_CAPTEUR_ATELIER );
                                          break;
                case ENVOI_CAPTEUR_ATELIER: Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_capteur_atelier_thread, client );
                                          pthread_detach( tid );
                                          break;

                case ENVOI_MOTIF_SUPERVISION:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL, (void *)Envoyer_motif_supervision_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;   
                case ENVOI_COMMENT_SUPERVISION:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_comment_supervision_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_PASSERELLE_SUPERVISION:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_passerelle_supervision_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;   
                case ENVOI_PALETTE_SUPERVISION:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_palette_supervision_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;   
                case ENVOI_CAPTEUR_SUPERVISION:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_capteur_supervision_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;   
                case ENVOI_IXXX_SUPERVISION :
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_bit_init_supervision_thread,
                                                          client );
                                          pthread_detach( tid );
                                              break;   
                case ENVOI_ICONE        : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL, (void *)Envoyer_icones_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_ICONE_FOR_ATELIER:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_icones_pour_atelier_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_CLASSE       : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL, (void *)Envoyer_classes_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_CLASSE_FOR_ATELIER:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_classes_pour_atelier_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_SYNOPTIQUE_FOR_ATELIER:
                                          Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_synoptiques_pour_atelier_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE:
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_synoptiques_pour_atelier_palette_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_PALETTE_FOR_ATELIER_PALETTE:
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_palette_atelier_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;

                case ENVOI_ENTREEANA    : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL, (void *)Envoyer_entreeANA_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_ENTREEANA_FOR_COURBE:
                                          Client_mode ( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_entreeANA_for_courbe_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_ENTREEANA_FOR_HISTO_COURBE:
                                          Client_mode ( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_entreeANA_for_histo_courbe_thread,
                                                          client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_SCENARIO     : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL, (void *)Envoyer_scenario_thread, client );
                                          pthread_detach( tid );
                                          break;
                case ENVOI_SCENARIO_SUP : Client_mode( client, VALIDE );
                                          Ref_client( client );  /* Indique que la structure est utilisée */
                                          pthread_create( &tid, NULL,
                                                          (void *)Envoyer_scenario_sup_thread, client );
                                          pthread_detach( tid );
                                          break;
              }
/****************************************** Envoi des chaines capteurs ************************************/
             if (client->mode == VALIDE && client->bit_capteurs)
              { struct CAPTEUR *capteur;
                GList *liste_capteur;
                liste_capteur = client->bit_capteurs;
                while (liste_capteur)                                 /* Pour tous les capteurs du client */
                 { capteur = (struct CAPTEUR *)liste_capteur->data;

                   if (Tester_update_capteur(capteur))             /* Doit-on updater le capteur client ? */
                    { struct CMD_ETAT_BIT_CAPTEUR *etat;
                      printf("Formatage capteur type %d bit %d\n", capteur->type, capteur->bit_controle );
                      etat = Formater_capteur(capteur);                /* Formatage de la chaine associée */
                      if (etat)
                       { printf("Envoi client capteur type %d bit %d\n", etat->type, etat->bit_controle );
                                                                                          /* envoi client */
                         Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_CAPTEUR,
                                       (gchar *)etat, sizeof(struct CMD_ETAT_BIT_CAPTEUR) );
                         g_free(etat);                                            /* On libere la mémoire */
                       }
                      else Info( Config.log, DEBUG_MEM, "SSRV: pb alloc mem envoi capteur" );
                    }
                   liste_capteur = liste_capteur->next;                    /* On passe au capteur suivant */
                 }
              }
/****************************************** Envoi des courbes analogiques *********************************/
             if (client->mode == VALIDE && client->courbes)
              { static gint update;
                struct COURBE *courbe;
                GList *liste_courbe;

                if (update < Partage->top)
                 { struct CMD_APPEND_COURBE envoi_courbe;
                   update = Partage->top + COURBE_TEMPS_TOP*10;/* Refresh toutes les 5 secondes au client */
                   envoi_courbe.date = time(NULL);

                   liste_courbe = client->courbes;
                   while (liste_courbe)
                    { courbe = (struct COURBE *)liste_courbe->data;

                      envoi_courbe.slot_id = courbe->slot_id;
                      envoi_courbe.type    = courbe->type;
                              
                      switch (courbe->type)
                       { case MNEMO_SORTIE:
                              envoi_courbe.val  = A(courbe->id);
                              Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                            (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                              break;
                         case MNEMO_ENTREE:
                              envoi_courbe.val  = E(courbe->id);
                              Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                            (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                              break;
                         case MNEMO_ENTREE_ANA:
                              envoi_courbe.val  = Partage->ea[courbe->id].val;
                              Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                            (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                              break;
                         default: printf("SSRV: type courbe inconnu\n");
                       }
                      liste_courbe = liste_courbe->next;
                    }
                 }
              }


             if (client->mode >= ATTENTE_IDENT) Ecouter_client( id, client );
#ifdef bouh
             if (Top > client->timeout)                                           /* Gestion du KEEPALIVE */
              { Info_n( Config.log, DEBUG_NETWORK, _("Keep alive failed"), client->connexion->socket );
                Deconnecter( client );
              }
#endif

             liste = liste->next;
           }
        }
       else
       if ( time(NULL) - Top > Config.max_inactivite )                          /* Detection d'inactivite */
        { Info( Config.log, DEBUG_INFO, "Inactivity time reached" );
          Arret = FIN;                      /* Arret "Local" du process: n'impacte pas les autres process */
        }
/****************************************** Ecoute des paroles du superviseur *****************************/
       if (Partage->Sous_serveur[id].type_info != TYPE_INFO_VIDE)
        { GList *liste_clients;
          struct CLIENT *client;
/* Faire le tri selon que le client est autorisé ou non à recevoir l'information !!!! */
          Info( Config.log, DEBUG_INFO, "SSRV: Run_serveur: type_info != vide" );
       
          switch( Partage->Sous_serveur[id].type_info )
           { case TYPE_INFO_NEW_HISTO:
                  Envoi_clients( id, TAG_HISTO, SSTAG_SERVEUR_SHOW_HISTO,
                                (gchar *)&Partage->new_histo, sizeof(struct CMD_SHOW_HISTO) );
                  break;
             case TYPE_INFO_DEL_HISTO:
                  Envoi_clients( id, TAG_HISTO, SSTAG_SERVEUR_DEL_HISTO,
                                (gchar *)&Partage->del_histo, sizeof(struct CMD_ID_HISTO) );
                  break;
             case TYPE_INFO_NEW_MOTIF:
                  liste_clients = Partage->Sous_serveur[id].Clients;
                  while (liste_clients)
                   { client = (struct CLIENT *)liste_clients->data;
                     if ( g_list_find( client->bit_syns, GINT_TO_POINTER(Partage->new_motif.num) ) )
                      { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF,
                                      (gchar *)&Partage->new_motif, sizeof(struct CMD_ETAT_BIT_CTRL) );
                      }
                     liste_clients = liste_clients->next;
                   }
                  break;
           }
          Info( Config.log, DEBUG_INFO, "SSRV: Run_serveur: type_info traité" );
          Partage->Sous_serveur[id].type_info = TYPE_INFO_VIDE;                    /* Information traitée */
        } 

       usleep(1000); sched_yield();                                /* On ne sature pas le microprocesseur */
     }
/********************************************* Arret du serveur *******************************************/
    while(Partage->Sous_serveur[id].Clients)                          /* Parcours de la liste des clients */
     { struct CLIENT *client;                                         /* Deconnection de tous les clients */
       client = (struct CLIENT *)Partage->Sous_serveur[id].Clients->data;
       Info_c( Config.log, DEBUG_INFO, "SSRV: Run_serveur: deconnexion client", client->machine );
       Deconnecter(client);
     }
    g_list_free(Partage->Sous_serveur[id].Clients);

    Partage->Sous_serveur[id].nb_client = -1;
    Partage->Sous_serveur[id].pid = -1;
    Partage->Sous_serveur[id].type_info = TYPE_INFO_VIDE;                          /* Information traitée */
    Partage->Sous_serveur[id].Clients = NULL;
    if (Partage->jeton == id)
     { Partage->jeton = -1;                                            /* On rend le jeton le cas échéant */
       Info_n( Config.log, DEBUG_INFO, "SSRV: Run_serveur: jeton rendu", id );
     }

    Info_n( Config.log, DEBUG_INFO, "SSRV: Run_serveur: Down", id );
    pthread_exit( NULL );
  }
/*--------------------------------------------------------------------------------------------------------*/
