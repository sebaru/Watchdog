/**********************************************************************************************************/
/* Watchdogd/Imsg/admin_imsg.c        Gestion des connexions Admin IMSG au serveur watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                  sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_imsg.c
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
 #include "Imsg.h"

/**********************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                 */
/* Entrée: Le client d'admin, la ligne a traiter                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "send" ) )
     { gchar to[256];
       sscanf ( ligne, "%s %s", commande, to );                      /* Découpage de la ligne de commande */
       Imsg_Envoi_message_to ( to, ligne + strlen (to) + 6 );
       Write_admin ( client->connexion, " Message sent.\n" );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { gchar chaine[128];
       GSList *liste;
       pthread_mutex_lock ( &Cfg_imsg.lib->synchro );
       liste = Cfg_imsg.contacts;
       while(liste)
        { struct IMSG_CONTACT *contact;
          contact  =(struct IMSG_CONTACT *)liste->data;
          g_snprintf( chaine, sizeof(chaine), " User %s is %s\n",
                      contact->nom, (contact->available ? "available" : "UNavailable") 
                    );
          Write_admin ( client->connexion, chaine );
          liste = liste->next;
        }
       pthread_mutex_unlock ( &Cfg_imsg.lib->synchro );
     }
    else if ( ! strcmp ( commande, "status" ) )
     { gchar chaine[128];
       if (Cfg_imsg.connection)
        { LmConnectionState state;
          state =  lm_connection_get_state ( Cfg_imsg.connection );
          switch (state)
           { case LM_CONNECTION_STATE_CLOSED:
                  g_snprintf( chaine, sizeof(chaine), " Connexion closed.\n"); break;
             case LM_CONNECTION_STATE_OPENING:
                  g_snprintf( chaine, sizeof(chaine), " Connexion is opening.\n"); break;
             case LM_CONNECTION_STATE_OPEN:
                  g_snprintf( chaine, sizeof(chaine), " Connexion opened.\n"); break;
             case LM_CONNECTION_STATE_AUTHENTICATING:
                  g_snprintf( chaine, sizeof(chaine), " Connexion is authenticating.\n"); break;
             case LM_CONNECTION_STATE_AUTHENTICATED:
                  g_snprintf( chaine, sizeof(chaine), " Connexion authenticated (OK).\n"); break;
             default:
                  g_snprintf( chaine, sizeof(chaine), " Connexion Status Unknown.\n"); break;
           }
          Write_admin ( client->connexion, chaine );
        }
       else Write_admin ( client->connexion, " No connexion ... strange ! \n" );
     }
    else if ( ! strcmp ( commande, "presence" ) )
     { g_snprintf( Cfg_imsg.new_status, sizeof(Cfg_imsg.new_status), "%s", commande + 9 );
       Cfg_imsg.set_status = TRUE;
       Write_admin ( client->connexion, " Presence Status changed ! \n" );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'IMSG'\n" );
       Write_admin ( client->connexion,
                     "  send user@domain/resource message      - Send a message to user\n" );
       Write_admin ( client->connexion,
                     "  list                                   - List contact and availability\n" );
       Write_admin ( client->connexion,
                     "  presence status                        - Change Presence status\n" );
       Write_admin ( client->connexion,
                     "  status                                 - See connexion status\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown IMSG command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
