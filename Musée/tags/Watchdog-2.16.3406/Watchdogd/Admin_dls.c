/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_dls.c        Gestion des responses Admin DLS au serveur watchdog                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_dls.c
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

/******************************************************************************************************************************/
/* Admin_dls_reload: Demande le rechargement des conf DLS                                                                     */
/* Entrée: la response                                                                                                       */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Admin_dls_reload ( gchar *response )
  { Partage->com_dls.Thread_reload = TRUE;
    while (Partage->com_dls.Thread_reload) sched_yield();
    return(Admin_write ( response, " | - DLS Reload done" ));
  }
/******************************************************************************************************************************/
/* Admin_dls_list: Print la liste des plugins dls actif ou non, mais chargés                                                  */
/* Entrée: La response                                                                                                       */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Admin_dls_list ( gchar *response )
  { GSList *liste_dls;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules D.L.S" );
    response = Admin_write ( response, chaine );
     
    pthread_mutex_lock( &Partage->com_dls.synchro );
    liste_dls = Partage->com_dls.Plugins;
    while ( liste_dls )
     { struct PLUGIN_DLS *dls;
       struct tm *temps;
       gchar date[80];
       dls = (struct PLUGIN_DLS *)liste_dls->data;
       temps = localtime( (time_t *)&dls->start_date );
       if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
       else       { g_snprintf( date, sizeof(date), "Erreur" ); }

       g_snprintf( chaine, sizeof(chaine), " | - DLS[%06d] -> actif=%d, start=%s, debug=%d, conso=%08.03f, nom=%s",
                   dls->plugindb.id, dls->plugindb.on, date, dls->debug, dls->conso, dls->plugindb.nom );
       response = Admin_write ( response, chaine );
       liste_dls = liste_dls->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_dls_gcc: compile le plugin dont l'id est en parametre                                                                */
/* Entrée: Le buffer a compléter, l'id du plugin                                                                              */
/* Sortie: Le buffer complété                                                                                                 */
/******************************************************************************************************************************/
 static gchar *Admin_dls_gcc ( gchar *response, gint id )
  { GSList *liste_dls;
    gchar chaine[256], buffer[1024];

    g_snprintf( chaine, sizeof(chaine), " -- Compilation des plugins D.L.S" );
    response = Admin_write ( response, chaine );

    if (id == -1)
     { pthread_mutex_lock( &Partage->com_dls.synchro );                                                      /* Lock du mutex */
       liste_dls = Partage->com_dls.Plugins;
       while ( liste_dls )
        { struct PLUGIN_DLS *dls;
          dls = (struct PLUGIN_DLS *)liste_dls->data;

          Compiler_source_dls ( FALSE, dls->plugindb.id, buffer, sizeof(buffer) );
          g_snprintf( chaine, sizeof(chaine), " | - Compilation du DLS[%06d] done (no reset): %s", dls->plugindb.id, buffer );
          response = Admin_write ( response, chaine );
          liste_dls = liste_dls->next;
        }
       pthread_mutex_unlock( &Partage->com_dls.synchro );
     } else
        { Compiler_source_dls ( FALSE, id, buffer, sizeof(buffer) );
          g_snprintf( chaine, sizeof(chaine), " | - Compilation du DLS[%06d] done (no reset): %s", id, buffer );
          response = Admin_write ( response, chaine );
        }
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_dls_start: Demarre un plugin DLS                                                                                     */
/* Entrée: Le buffer a compléter, l'id du plugin                                                                              */
/* Sortie: Le buffer complété                                                                                                 */
/******************************************************************************************************************************/
 static gchar *Admin_dls_start ( gchar *response, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Demarrage d'un plugin D.L.S" );
    response = Admin_write ( response, chaine );

    while (Partage->com_dls.admin_start) sched_yield();
    Partage->com_dls.admin_start = id;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: impossible d'ouvrir la Base de données %s", __func__,
                 Config.db_database );
       return(response);
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=1 WHERE id=%d", NOM_TABLE_DLS, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(response);
     }
    Libere_DB_SQL( &db );

    g_snprintf( chaine, sizeof(chaine), " | - Module DLS %d started", id );
    return(Admin_write ( response, chaine ));
  }
/******************************************************************************************************************************/
/* Admin_dls_debug: Active ou non le debug du plugin                                                                          */
/* Entrée: La response, le numéro du plugin, et le statut du debug                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_dls_debug ( gchar *response, gint id, gboolean debug )
  { gchar chaine[128];
    GSList *liste_dls;
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Modification du statut de debug d'un plugin D.L.S" );
    response = Admin_write ( response, chaine );

    pthread_mutex_lock( &Partage->com_dls.synchro );                                                         /* Lock du mutex */
    liste_dls = Partage->com_dls.Plugins;
    while ( liste_dls )                                                      /* Recherche du plugin et positionnement du flag */
     { struct PLUGIN_DLS *dls;
       dls = (struct PLUGIN_DLS *)liste_dls->data;

       if (dls->plugindb.id == id)
        { dls->debug = debug;
          break;
        }
       liste_dls=liste_dls->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );

    g_snprintf( chaine, sizeof(chaine), " | - Module DLS: debug set to '%d'", debug );
    return(Admin_write ( response, chaine ));
  }
/******************************************************************************************************************************/
/* Admin_dls_stop: Arrete un plugin DLS                                                                                       */
/* Entrée: Le buffer a compléter, l'id du plugin                                                                              */
/* Sortie: Le buffer complété                                                                                                 */
/******************************************************************************************************************************/
 static gchar *Admin_dls_stop ( gchar *response, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Arret d'un plugin D.L.S" );
    response = Admin_write ( response, chaine );

    while (Partage->com_dls.admin_stop) sched_yield();
    Partage->com_dls.admin_stop = id;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: impossible d'ouvrir la Base de données %s", __func__,
                 Config.db_database );
       return(response);
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=0 WHERE id=%d", NOM_TABLE_DLS, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(response);
     }
    Libere_DB_SQL( &db );

    g_snprintf( chaine, sizeof(chaine), " | - Module DLS %d stopped", id );
    return(Admin_write ( response, chaine ));
  }
/******************************************************************************************************************************/
/* Admin_dls: Appellée lorsque l'admin envoie une commande en mode dls dans la ligne de commande                              */
/* Entrée: Le buffer a compléter, l'id du plugin                                                                              */
/* Sortie: Le buffer complété                                                                                                 */
/******************************************************************************************************************************/
 gchar *Admin_dls ( gchar *response, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       return(Admin_dls_start ( response, num ));
     }
    else if ( ! strcmp ( commande, "gcc" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       return(Admin_dls_gcc ( response, num ));
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       return(Admin_dls_stop ( response, num ));
     }
    else if ( ! strcmp ( commande, "debug" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       return(Admin_dls_debug ( response, num, TRUE ));
     }
    else if ( ! strcmp ( commande, "nodebug" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       return(Admin_dls_debug ( response, num, FALSE ));
     }
    else if ( ! strcmp ( commande, "reset" ) )
     { gchar chaine[64];
       int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       Reseter_un_plugin ( num );
       g_snprintf( chaine, sizeof(chaine), " | - Module DLS: Plugin %d resetted", num );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { return(Admin_dls_list ( response ));
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { return(Admin_dls_reload( response ));
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'D.L.S'" );
       response = Admin_write ( response, "  debug $id                              - Active le mode debug du plugin $id" );
       response = Admin_write ( response, "  nodebug $id                            - Desactive le mode debug du plugin $id" );
       response = Admin_write ( response, "  start $id                              - Demarre le module $id" );
       response = Admin_write ( response, "  stop $id                               - Stop le module $id" );
       response = Admin_write ( response, "  reset $id                              - Stop/Unload/Load/Start module $id" );
       response = Admin_write ( response, "  list                                   - D.L.S. Status" );
       response = Admin_write ( response, "  gcc $id                                - Compile le plugin $id (-1 for all)" );
       response = Admin_write ( response, "  reload                                 - Recharge la configuration" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown DLS command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
