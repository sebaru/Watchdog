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
 #include "Audio.h"

/**********************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                 */
/* Entrée: Le client d'admin, la ligne a traiter                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128], chaine[80];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "tell_mp3" ) )
     { struct CMD_TYPE_MESSAGE msg;
       sscanf ( ligne, "%s %d", commande, &msg.num );                /* Découpage de la ligne de commande */
       Jouer_mp3 ( &msg );
       g_snprintf( chaine, sizeof(chaine), " Message id %d sent\n", msg.num );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "tell_espeak" ) )
     { struct CMD_TYPE_MESSAGE msg;
       sscanf ( ligne, "%s %d", commande, &msg.num );                /* Découpage de la ligne de commande */
       Jouer_espeak ( &msg );
       g_snprintf( chaine, sizeof(chaine), " Message id %d sent\n", msg.num );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion, "  -- Watchdog ADMIN -- Help du mode 'AUDIO'\n" );
       Write_admin ( client->connexion, "  tell_mp3 num          - Send message num with mp3 format\n" );
       Write_admin ( client->connexion, "  tell_espeak num       - Send message num with espeak format\n" );
       Write_admin ( client->connexion, "  help                  - This help\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
