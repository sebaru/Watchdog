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
#ifdef bouh
    else if ( ! strcmp ( commande, "change" ) )
     { struct IMSGDB imsg;
       memset( &imsg, 0, sizeof(struct IMSGDB) );
       sscanf ( ligne, "%s %d %d %d %d %d %d %s", commande,          /* Découpage de la ligne de commande */
                &imsg.id, (gint *)&imsg.type, (gint *)&imsg.canal,
                &imsg.e_min, &imsg.ea_min, &imsg.a_min, imsg.libelle );
       Admin_imsg_change ( client, &imsg );
     }
    else if ( ! strcmp ( commande, "cmd" ) )
     { sscanf ( ligne, "%s %d %d %d %d %d %d %d", commande,          /* Découpage de la ligne de commande */
                &Partage->com_imsg.learn.id1, &Partage->com_imsg.learn.id2,
                &Partage->com_imsg.learn.id3, &Partage->com_imsg.learn.id4,
                &Partage->com_imsg.learn.unitcode,
                &Partage->com_imsg.learn.cmd,
                &Partage->com_imsg.learn.level
               );
       Partage->com_imsg.Thread_commande = TRUE;
       Write_admin ( client->connexion, " IMSG Sending CMD....\n" );
       while (Partage->com_imsg.Thread_commande) sched_yield();
       Write_admin ( client->connexion, " IMSG Done.\n" );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_imsg_del ( client, num );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_imsg_reload(client);
     }
#endif
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'IMSG'\n" );
       Write_admin ( client->connexion,
                     "  send user@domain/resource message      - Send a message to user\n" );
       Write_admin ( client->connexion,
                     "  list                                   - List contact and availability\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown IMSG command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
