/**********************************************************************************************************/
/* Watchdogd/Teleinfo/admin_teleinfo.c        Gestion des connexions Admin TLEINFO au serveur watchdog    */
/* Projet WatchDog version 2.0       Gestion d'habitat                    mer. 13 juin 2012 23:02:08 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

/**********************************************************************************************************/
/* Admin_command: Fonction gerant les différentes commandes possible pour l'administration teleinfo       */
/* Entrée: le connexion d'admin et la ligne de commande                                                   */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'TELEINFO'\n" );
       Admin_write ( connexion, "  dbcfg ...             - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  reload                - Reload config from Database\n" );
       Admin_write ( connexion, "  status                - Affiche les status du device Teleinfo\n" );
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)   /* Si changement de parametre */
        { gboolean retour;
          retour = Teleinfo_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Teleinfo Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "status" ) )
     { g_snprintf( chaine, sizeof(chaine), " Port '%s' mode %d (retry in %02.1f) -> Last_view = %d (%.1fs ago)\n",
                   Cfg_teleinfo.port, Cfg_teleinfo.mode, (Partage->top - Cfg_teleinfo.date_next_retry)/10.0,
                   Cfg_teleinfo.last_view, (Partage->top - Cfg_teleinfo.last_view)/10.0
                 );
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { g_snprintf( chaine, sizeof(chaine), " Reloading Teleinfo from Database\n" );
       Admin_write ( connexion, chaine );
       Cfg_teleinfo.reload = TRUE;
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown Teleinfo command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
