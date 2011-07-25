/**********************************************************************************************************/
/* Watchdogd/Admin/admin_tellstick.c        Gestion des connexions Admin PROCESS au serveur watchdog      */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 20 févr. 2011 21:29:59 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_tellstick.c
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
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_tellstick ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "learn" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Partage->com_tellstick.Admin_tellstick_learn(client, num);
     }
    else if ( (!strcmp ( commande, "start" )) && Partage->com_tellstick.Admin_tellstick_start )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Partage->com_tellstick.Admin_tellstick_start(client, num);
     }
    else if ( (!strcmp ( commande, "stop" )) && Partage->com_tellstick.Admin_tellstick_stop )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Partage->com_tellstick.Admin_tellstick_stop(client, num);
     }
    else if ( (! strcmp ( commande, "list" )) && Partage->com_tellstick.Admin_tellstick_list )
     { Partage->com_tellstick.Admin_tellstick_list(client); }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'TELLSTICK'\n" );
       Write_admin ( client->connexion,
                     "  learn id             - Envoie une commande LEARN au device\n" );
       Write_admin ( client->connexion,
                     "  stop id              - Stop module id\n" );
       Write_admin ( client->connexion,
                     "  start id             - Start module id\n" );
       Write_admin ( client->connexion,
                     "  list                 - list all devices\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown TELLSTICK command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
