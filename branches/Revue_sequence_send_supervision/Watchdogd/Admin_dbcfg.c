/**********************************************************************************************************/
/* Watchdogd/Admin/admin_dbcfg.c        Gestion des connexions Admin GET au serveur watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                    jeu. 05 janv. 2012 23:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_dbcfg.c
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

/**********************************************************************************************************/
/* Admin_dbcfg: Gere une commande 'admin dbcfg' depuis une connexion admin                                */
/* Entrée: le connexion et la ligne de commande                                                           */
/* Sortie: TRUE si un des parametres à été modifié et necessite un rechargement de la conf DB             */
/**********************************************************************************************************/
 gboolean Admin_dbcfg_thread ( struct CONNEXION *connexion, gchar *thread, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                     /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'DBCFG'\n" );
       Admin_write ( connexion, "  list               - List all parameters\n" );
       Admin_write ( connexion, "  reload             - Reload all Parameters from DB\n" );
       Admin_write ( connexion, "  set name value     - Set parameter name to value\n" );
       Admin_write ( connexion, "  del name           - Erase parameter name\n" );
       Admin_write ( connexion, "  help               - This help\n" );
     } else
    if ( ! strcmp ( commande, "list" ) )
     { gchar *nom, *valeur;
       struct DB *db;

       if ( ! Recuperer_configDB( &db, thread ) )                       /* Connexion a la base de données */
        { g_snprintf(chaine, sizeof(chaine), "Database connexion failed\n" );
          Admin_write ( connexion, chaine );
        }
       else 
        { g_snprintf(chaine, sizeof(chaine), " | Instance_id '%s', Thread '%s'\n", Config.instance_id, thread );
          Admin_write ( connexion, chaine );
          while (Recuperer_configDB_suite( &db, &nom, &valeur ) )/* Récupération d'une config dans la DB */
           { g_snprintf(chaine, sizeof(chaine), " | - '%20s' = '%s'\n", nom, valeur );
             Admin_write ( connexion, chaine );
           }
          Admin_write( connexion, " |-\n" );
        }
     } else
    if ( ! strcmp ( commande, "set" ) )
     { gchar param[80],valeur[80];
       gboolean retour;
       sscanf ( ligne, "%s %s %s", commande, param, valeur );/* Découpage de la ligne de commande */
       retour = Modifier_configDB( thread, param, valeur );
       g_snprintf( chaine, sizeof(chaine), " Instance_id '%s', Thread '%s' -> Setting %s = %s -> %s\n",
                   Config.instance_id, thread, param, valeur,
                  (retour ? "Success" : "Failed") );
       Admin_write ( connexion, chaine );
       return(retour);
     } else
    if ( ! strcmp ( commande, "del" ) )
     { gchar param[80];
       gboolean retour;
       sscanf ( ligne, "%s %s", commande, param );        /* Découpage de la ligne de commande */
       retour = Retirer_configDB( thread, param );
       g_snprintf( chaine, sizeof(chaine), " Instance_id '%s', Thread '%s' -> Erasing %s -> %s\n",
                   Config.instance_id, thread, param,
                   (retour ? "Success" : "Failed") );
       Admin_write ( connexion, chaine );
       return(retour);
     } else
    if ( ! strcmp ( commande, "reload" ) )
     { g_snprintf( chaine, sizeof(chaine), " Instance_id '%s', Thread '%s' -> Reloading ...\n",
                   Config.instance_id, thread );
       Admin_write ( connexion, chaine );
       return(TRUE);
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Unknown DBCFG command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
    return(FALSE);
  }
/*--------------------------------------------------------------------------------------------------------*/
