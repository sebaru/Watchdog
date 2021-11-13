/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_dbcfg.c        Gestion des connexions Admin GET au serveur watchdog                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        jeu. 05 janv. 2012 23:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

/******************************************************************************************************************************/
/* Admin_dbcfg_reload: Gere une commande 'admin dbcfg' depuis une connexion admin                                             */
/* Entrée: la response et le thread a reloader                                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_dbcfg_reload ( gchar *thread )
  { struct LIBRAIRIE *lib;
    GSList *liste;
    liste = Partage->com_msrv.Librairies;                                                /* Parcours de toutes les librairies */
    while(liste)
     { lib = (struct LIBRAIRIE *)liste->data;
       if ( ! strcmp( thread, lib->admin_prompt ) ) { lib->Thread_sigusr1 = TRUE; break; }
       liste = liste->next;
     }
  }
/******************************************************************************************************************************/
/* Admin_dbcfg: Gere une commande 'admin dbcfg' depuis une connexion admin                                                    */
/* Entrée: le connexion et la ligne de commande                                                                               */
/* Sortie: TRUE si un des parametres à été modifié et necessite un rechargement de la conf DB                                 */
/******************************************************************************************************************************/
 gchar *Admin_dbcfg ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128], thread[20];

    if ( sscanf ( ligne, "%s %s", thread, commande ) != 2) return(response);             /* Découpage de la ligne de commande */

    if ( !strcmp ( commande, "help" ) )
     { response = Admin_write ( response, " | -- Watchdog ADMIN -- Help du mode 'DBCFG'" );
       response = Admin_write ( response, " | - $thread list               - List all parameters" );
       response = Admin_write ( response, " | - $thread reload             - Reload all Parameters from DB" );
       response = Admin_write ( response, " | - $thread set $name $value   - Set parameter name to value" );
       response = Admin_write ( response, " | - $thread del $name          - Erase parameter name" );
       response = Admin_write ( response, " | - help                       - This help" );
     } else
    if ( ! strcmp ( commande, "list" ) )
     { gchar *nom, *valeur;
       struct DB *db;

       if ( ! Recuperer_configDB( &db, thread ) )                       /* Connexion a la base de données */
        { g_snprintf(chaine, sizeof(chaine), " | - Database connexion failed" );
          response = Admin_write ( response, chaine );
        }
       else 
        { g_snprintf(chaine, sizeof(chaine), " | - Instance_id '%s', Thread '%s'", g_get_host_name(), thread );
          response = Admin_write ( response, chaine );
          while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                     /* Récupération d'une config dans la DB */
           { g_snprintf(chaine, sizeof(chaine), " | - '%20s' = '%s'", nom, valeur );
             response = Admin_write ( response, chaine );
           }
        }
     } else
    if ( ! strcmp ( commande, "set" ) )
     { gchar param[80],valeur[80];
       gboolean retour;
       if (sscanf ( ligne, "%s %s %s %s", thread, commande, param, valeur )!=4) return(response);
       retour = Modifier_configDB( thread, param, valeur );
       g_snprintf( chaine, sizeof(chaine), " | - Instance_id '%s', Thread '%s' -> Setting %s = %s -> %s",
                   g_get_host_name(), thread, param, valeur, (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
       Admin_dbcfg_reload(thread);
     } else
    if ( ! strcmp ( commande, "del" ) )
     { gchar param[80];
       gboolean retour;
       if (sscanf ( ligne, "%s %s %s", thread, commande, param )!=3) return(response);
       retour = Retirer_configDB( thread, param );
       g_snprintf( chaine, sizeof(chaine), " | - Instance_id '%s', Thread '%s' -> Erasing %s -> %s",
                   g_get_host_name(), thread, param, (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
       Admin_dbcfg_reload(thread);
     } else
    if ( ! strcmp ( commande, "reload" ) )
     { g_snprintf( chaine, sizeof(chaine), " | - Instance_id '%s', Thread '%s' -> Reloading ...",
                   g_get_host_name(), thread );
       response = Admin_write ( response, chaine );
       Admin_dbcfg_reload(thread);
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " | - Unknown DBCFG command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
