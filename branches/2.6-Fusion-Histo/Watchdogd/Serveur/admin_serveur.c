/**********************************************************************************************************/
/* Watchdogd/Serveur/admin_serveur.c        Gestion des connexions Admin IMSG au serveur watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                  sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_serveur.c
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
 
 #include "watchdogd.h"
 #include "Sous_serveur.h"

/**********************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                 */
/* Entrée: Le connexion d'admin, la ligne a traiter                                                       */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[80];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "list" ) )
     { struct CLIENT *client;
       GSList *liste;
        
       g_snprintf( chaine, sizeof(chaine), " -- Liste des clients connectés au serveur\n" );
       Admin_write ( connexion, chaine );

       pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
       liste = Cfg_ssrv.Clients;
       while ( liste )                              /* Parcours de la liste des ssrv (et donc de clients) */
        { client = (struct CLIENT *)liste->data;
          g_snprintf( chaine, sizeof(chaine), " SSRV%06d - v%s %s@%s - mode %d defaut %d date %s",
                          client->ssrv_id, client->ident.version,
                         (client->util ? client->util->nom : "unknown"), client->machine,
                          client->mode, client->defaut, ctime(&client->date_connexion) );
          Admin_write ( connexion, chaine );
          liste = g_slist_next(liste);
        }
       pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
     } else
    if ( ! strcmp ( commande, "msgs" ) )
     { struct CMD_GTK_MESSAGE erreur;
       struct CLIENT *client;
       GSList *liste;

       g_snprintf( chaine, sizeof(chaine), " -- Liste des connexions recevant le message\n" );
       Admin_write ( connexion, chaine );
       g_snprintf( erreur.message, sizeof(erreur.message), "AdminMSG : %s", ligne + 5 );

       pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
       liste = Cfg_ssrv.Clients;
       while ( liste )                              /* Parcours de la liste des ssrv (et donc de clients) */
        { client = (struct CLIENT *)liste->data;
          Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_WARNING,
                        (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
          g_snprintf( chaine, sizeof(chaine), " SSRV%06d - %s@%s\n", client->ssrv_id,
                      (client->util ? client->util->nom : "unknown"), client->machine );
          Admin_write ( connexion, chaine );
          liste = g_slist_next(liste);
        }
       pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
     } else
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'SSRV'\n" );
       Admin_write ( connexion, "  list                  - Listes les sous serveurs\n" );
       Admin_write ( connexion, "  msgs                  - Send message to all connected client\n" );
       Admin_write ( connexion, "  help                  - This help\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
