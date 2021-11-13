/******************************************************************************************************************************/
/* Watchdogd/Teleinfo/admin_teleinfo.c        Gestion des connexions Admin TLEINFO au serveur watchdog                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        mer. 13 juin 2012 23:02:08 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_teleinfo.c
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
 #include "Teleinfo.h"

/******************************************************************************************************************************/
/* Admin_command: Fonction gerant les différentes commandes possible pour l'administration teleinfo                           */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 gchar *Admin_command( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'TELEINFO'" );
       response = Admin_write ( response, "  dbcfg ...             - Get/Set Database Parameters" );
       response = Admin_write ( response, "  reload                - Reload config from Database" );
       response = Admin_write ( response, "  status                - Affiche les status du device Teleinfo" );
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { gboolean retour;
       response =  Admin_dbcfg_thread ( response, NOM_THREAD, ligne+6 );                        /* Si changement de parametre */
       retour = Teleinfo_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " Reloading Teleinfo Thread Parameters from Database -> %s", (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "status" ) )
     { g_snprintf( chaine, sizeof(chaine), " Port '%s' mode %d (retry in %02.1f) -> Last_view = %d (%.1fs ago)",
                   Cfg_teleinfo.port, Cfg_teleinfo.mode, (Partage->top - Cfg_teleinfo.date_next_retry)/10.0,
                   Cfg_teleinfo.last_view, (Partage->top - Cfg_teleinfo.last_view)/10.0
                 );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { g_snprintf( chaine, sizeof(chaine), " Reloading Teleinfo from Database" );
       response = Admin_write ( response, chaine );
       Cfg_teleinfo.reload = TRUE;
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown Teleinfo command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
