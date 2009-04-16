/**********************************************************************************************************/
/* Watchdogd/Admin/admin_process.c        Gestion des connexions Admin PROCESS au serveur watchdog        */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_process.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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

 #include "Admin.h"
 #include "Modbus.h"
 #include "watchdogd.h"

 extern struct CONFIG Config;
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */

/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_process ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "SHUTDOWN" ) )
     { Info( Config.log, DEBUG_INFO, "Admin_process : SHUTDOWN demandé" );
       Write_admin ( client->connexion, "SHUTDOWN in progress\n" );
       Partage->Arret = FIN;
     } else
    if ( ! strcmp ( commande, "REBOOT" ) )
     { Info( Config.log, DEBUG_INFO, "Admin_process : REBOOT demandé" );
       Write_admin ( client->connexion, "REBOOT in progress\n" );
       Partage->Arret = REBOOT;
     } else
    if ( ! strcmp ( commande, "CLEAR-REBOOT" ) )
     { Info( Config.log, DEBUG_INFO, "Admin_process : CLEAR-REBOOT demandé" );
       Write_admin ( client->connexion, "CLEAR-REBOOT in progress\n" );
       Partage->Arret = CLEARREBOOT;
     } else
    if ( ! strcmp ( commande, "RELOAD" ) )
     { Info( Config.log, DEBUG_INFO, "Admin_process : RELOAD demandé" );
       Write_admin ( client->connexion, "RELOAD in progress\n" );
       Partage->Arret = RELOAD;
     } else
    if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion, "  -- Watchdog ADMIN -- Help du mode 'PROCESS'\n" );
       Write_admin ( client->connexion, "  RELOAD               - Reload configuration\n" );
       Write_admin ( client->connexion, "  REBOOT               - Restart all processes\n" );
       Write_admin ( client->connexion, "  CLEAR-REBOOT         - Restart all processes with no DATA import/expot\n" );
       Write_admin ( client->connexion, "  SHUTDOWN             - Stop processes\n" );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
