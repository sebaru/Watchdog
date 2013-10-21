/**********************************************************************************************************/
/* Watchdogd/admin_master.c        Gestion des connexions Admin du thread "Master" de watchdog            */
/* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 02 févr. 2013 14:04:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_master.c
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
 
 #include <unistd.h>                                                                  /* Pour gethostname */
 #include "watchdogd.h"
 #include "Satellite.h"

/**********************************************************************************************************/
/* Admin_master: Gere une commande 'admin masterdepuis une connexion admin                                */
/* Entrée: le connexion et la ligne de commande                                                           */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'Satellite'\n" );
       Admin_write ( connexion, "  reload            - Reload config from Database\n" );
       Admin_write ( connexion, "  help                    - This help\n" );
     } else
    if ( ! strcmp ( commande, "reload" ) )                /* Rechargement de la configuration en Database */
     { gboolean retour;
       retour = Satellite_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " Reloading Satellite Parameters -> %s\n",
                   (retour ? "Success" : "Failed") );
       Admin_write ( connexion, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }

  }
/*--------------------------------------------------------------------------------------------------------*/
