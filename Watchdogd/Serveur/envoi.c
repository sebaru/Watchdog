/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi.c        Procedures d'envoi de données au(x) client(s) connecté(s)             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       ven 04 mar 2005 10:16:04 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 #include <string.h>
 #include <errno.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <sys/unistd.h>
 #include <bonobo/bonobo-i18n.h>

 #define DEFAUT_MAX 3
/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Envoi_client: Envoi le buffer au client id                                                             */
/* Entrée: structure identifiant le client, et le buffer à envoyer                                        */
/* Sortie: code d'erreur                                                                                  */
/**********************************************************************************************************/
 gint Envoi_client( struct CLIENT *client, gint tag, gint ss_tag, gchar *buffer, gint taille )
  { gint retour;

    if ( !client ) return(0);
    if ( client->mode >= DECONNECTE )
     { Info_n( Config.log, DEBUG_NETWORK, "Envoi_client : envoi interdit", client->connexion->socket);
       return(0);
     }

    pthread_mutex_lock( &client->mutex_write );
    retour = Envoyer_reseau( Config.log, client->connexion, W_CLIENT, tag, ss_tag, buffer, taille );
    pthread_mutex_unlock( &client->mutex_write );
    if (retour)
     { client->defaut++;
       Info_n( Config.log, DEBUG_NETWORK, "Defaut envoi client id", client->connexion->socket);
       Info_c( Config.log, DEBUG_NETWORK, "                   sur", client->machine);
       Info_n( Config.log, DEBUG_NETWORK, "                 errno", retour);

       if (client->defaut>=DEFAUT_MAX)
        { Info( Config.log, DEBUG_NETWORK, "Deconnexion client sur défaut" );
          Client_mode ( client, DECONNECTE );
        }
       else switch(retour)
        { case EPIPE:
          case ECONNRESET: Client_mode ( client, DECONNECTE );          /* Connection resettée par le clt */
                           Info( Config.log, DEBUG_NETWORK, "     decision: deconnexion client" );
                           break;
        }
     }
    else client->defaut=0;                                                    /* Ok, pas de defaut client */
    return(retour);
  }
/**********************************************************************************************************/
/* Envoi_clients: Envoi le buffer à tous les clients connectés au système                                 */
/* Entrée: le buffer à envoyer                                                                            */
/**********************************************************************************************************/
 void Envoi_clients( gint ss_id, gint tag, gint ss_tag, gchar *buffer, gint taille )
  { struct CLIENT *client;
    GList *liste;
    liste = Partage->Sous_serveur[ss_id].Clients;
    while( liste )                        /* On prend chaque client un par un, et on lui envoie le buffer */
     { client = (struct CLIENT *)liste->data;
       if (client->mode>=VALIDE) Envoi_client( client, tag, ss_tag, buffer, taille );
       liste=liste->next;
     }
  }
/**********************************************************************************************************/
/* Envoyer_donnees: Transmet les données pour le fonctionnement correct du client distant                 */
/* Entrée: structure cliente distante                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 gboolean Envoyer_gif( struct CLIENT *client )
  { gint taille;
    if (Attendre_envoi_disponible(Config.log, client->connexion)==-1)     /* Attente de liberation reseau */
     { return( FALSE ); }
printf("Envoi_gif: envoi disponible !!\n");

    if (!client->transfert.buffer)
     { /*struct CMD_REZO_ENREG nbr;*/
       client->transfert.buffer = g_malloc0( client->connexion->taille_bloc );
       if (!client->transfert.buffer)
        { printf("Envoyer_donnees: Pas de mémoire\n");
          client->transfert.en_cours = FALSE;
          return(TRUE);
        }
       /*nbr.num = client->transfert.taille;
       g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading data") );
       Envoi_client ( client, TAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct REZO_ENREG) );*/
     }

    if (client->transfert.en_cours)                    /* A-t-on deja un fichier en cours de transfert ?? */
     { taille = read( client->transfert.fd, client->transfert.buffer + sizeof(struct CMD_FICHIER),
                                            client->connexion->taille_bloc - sizeof(struct CMD_FICHIER) );
       printf("taille lue: %d  taille max=%d\n", taille,
              client->connexion->taille_bloc - sizeof(struct CMD_FICHIER) );
       if (taille>=0)                                            /* On n'est tjrs pas a la fin du fichier */
        { gint retour;
          retour = Envoi_client( client, TAG_FICHIER, SSTAG_SERVEUR_FICHIER, client->transfert.buffer,
                                 taille + sizeof(struct CMD_FICHIER) );
          printf("Retour write: %d\n", retour );
        }

       if (taille != client->connexion->taille_bloc - sizeof(struct CMD_FICHIER) )
                                                       /* Est-on a la fin du fichier ?? (ou -1 si erreur) */
        { printf("Fin fichier %s\n", ((struct CMD_FICHIER *)client->transfert.buffer)->nom );
          close(client->transfert.fd);                         /* Oui, on le ferme et on passe au suivant */
          client->transfert.en_cours = FALSE;
          g_free(client->transfert.fichiers->data);
          client->transfert.fichiers = g_list_remove( client->transfert.fichiers,
                                                      client->transfert.fichiers->data );
          if (!client->transfert.fichiers)
           { struct CMD_VERSION cmd_version;
             Liberer_liste( client );                                                 /* Fin du transfert */
             g_free(client->transfert.buffer);
             cmd_version.version = Lire_version_donnees( Config.log );
             Envoi_client( client, TAG_FICHIER, SSTAG_SERVEUR_VERSION,
                           (gchar *)&cmd_version, sizeof( struct CMD_VERSION ) );
             return(TRUE);
           }
        }
     }
    else
     { struct LISTE_FICH *liste;
       suivant:
       if (!client->transfert.fichiers) return(TRUE);
       liste = (struct LISTE_FICH *)(client->transfert.fichiers->data);
       printf("Ouverture fichier %s\n", liste->fichier_absolu );
       client->transfert.fd = open( liste->fichier_absolu, O_RDONLY );
       if (client->transfert.fd<0)
        { Info_c( Config.log, DEBUG_INFO, "Transfert impossible", liste->fichier );
          g_free(client->transfert.fichiers->data);
          client->transfert.fichiers = g_list_remove( client->transfert.fichiers, liste );
          goto suivant;                                                           /* On essaie le suivant */
        }

       g_snprintf( ((struct CMD_FICHIER *)client->transfert.buffer)->nom,
                   sizeof(((struct CMD_FICHIER *)client->transfert.buffer)->nom), "%s", liste->fichier );
       Envoi_client( client, TAG_FICHIER, SSTAG_SERVEUR_DEL_FICHIER,
                     client->transfert.buffer, sizeof(struct CMD_FICHIER) );
       client->transfert.en_cours = TRUE;
     }
    return(FALSE);
  }
/*--------------------------------------------------------------------------------------------------------*/
