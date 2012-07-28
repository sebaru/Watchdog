/**********************************************************************************************************/
/* Watchdogd/Dls/plugins.c  -> Gestion des plugins pour DLS                                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 02 janv. 2011 19:04:47 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * plugins.c
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
 #include <string.h>
 #include <stdio.h>
 #include <dlfcn.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                          */
/* Entrée: Le nom de fichier correspondant                                                                */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 static gboolean Charger_un_plugin ( struct PLUGIN_DLS *dls )
  { gchar nom_fichier_absolu[60];
    void (*Go)(int);
    void *handle;

    g_snprintf( nom_fichier_absolu, sizeof(nom_fichier_absolu), "%s/libdls%d.so", Config.home, dls->plugindb.id );

    handle = dlopen( nom_fichier_absolu, RTLD_LAZY );
    if (!handle) { Info_n( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin: Candidat rejeté ", dls->plugindb.id );
                   Info_c( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin: -- sur ouverture", dlerror() );
                   return(FALSE);
                 }
    Go = dlsym( handle, "Go" );                                         /* Recherche de la fonction 'Go' */
    if (!Go) { Info_n( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin: Candidat rejeté sur absence GO", dls->plugindb.id ); 
               dlclose( handle );
               return(FALSE);
             }

    Info_n( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin: id", dls->plugindb.id );
    strncpy( dls->nom_fichier, nom_fichier_absolu, sizeof(dls->nom_fichier) );
    dls->handle   = handle;
    dls->go       = Go;
    dls->starting = 1;
    dls->conso    = 0.0;
    pthread_mutex_lock( &Partage->com_dls.synchro );
    Partage->com_dls.Plugins = g_list_append( Partage->com_dls.Plugins, dls );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                          */
/* Entrée: Le nom de fichier correspondant                                                                */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 static gboolean Charger_un_plugin_by_id ( gint id )
  { struct CMD_TYPE_PLUGIN_DLS *plugin_dls;
    struct PLUGIN_DLS *dls;
    struct DB *db;                                                                   /* Database Watchdog */

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin_by_id: Unable to open database" );
       return(FALSE);
     }

    plugin_dls = Rechercher_plugin_dlsDB( Config.log, db, id );
    Libere_DB_SQL( Config.log, &db );

    if (!plugin_dls)
     { Info_n( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin_by_id: Plugin non trouvé", id );
       return(FALSE);
     }

    dls = (struct PLUGIN_DLS *)g_malloc0( sizeof(struct PLUGIN_DLS) );
    if (!dls)
     { Info_n( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin_by_id: out of memory", id );
       g_free(plugin_dls);
       return(FALSE);
     }

    memcpy( &dls->plugindb, plugin_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
    g_free(plugin_dls);

    return ( Charger_un_plugin ( dls ) );
  }
/**********************************************************************************************************/
/* Retirer_plugins: Decharge toutes les librairies                                                        */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Decharger_un_plugin_by_id ( gint id )
  { struct PLUGIN_DLS *plugin;
    GList *plugins;

    pthread_mutex_lock( &Partage->com_dls.synchro );
    plugins = Partage->com_dls.Plugins;
    while(plugins)                                                       /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS *)plugins->data;

       if ( plugin->plugindb.id == id )
        { dlclose( plugin->handle );
          Partage->com_dls.Plugins = g_list_remove( Partage->com_dls.Plugins, plugin );
          g_free( plugin );
          Info_n( Config.log, DEBUG_DLS, "DLS: Decharger_un_plugin_by_id: Dechargé", plugin->plugindb.id );
          break;
        }
       plugins = plugins->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/**********************************************************************************************************/
/* Retirer_un_plugin: Decharge le plugin dont le numero est en parametre                                  */
/* Entrée: L'identifiant du plugin                                                                        */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Reseter_un_plugin ( gint id )
  { Info_n( Config.log, DEBUG_DLS, "DLS: Reseter_un_plugin: Demande de reset plugin", id );

    Decharger_un_plugin_by_id ( id );
    Charger_un_plugin_by_id ( id );
  }
/**********************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                       */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Decharger_plugins ( void )
  { struct PLUGIN_DLS *plugin;

    pthread_mutex_lock( &Partage->com_dls.synchro );
    while(Partage->com_dls.Plugins)                                     /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS *)Partage->com_dls.Plugins->data;
       Info_n( Config.log, DEBUG_DLS, "DLS: Decharger_plugins: tentative dechargement:", plugin->plugindb.id );
       dlclose( plugin->handle );
       Partage->com_dls.Plugins = g_list_remove( Partage->com_dls.Plugins, plugin );
                                                         /* Destruction de l'entete associé dans la GList */
       Info_n( Config.log, DEBUG_DLS, "DLS: Decharger_plugins: Dechargé", plugin->plugindb.id );
       g_free( plugin );
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/**********************************************************************************************************/
/* Charger_plugins: Ouverture de toutes les librairies possibles pour le DLS                              */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Charger_plugins ( void )
  { struct CMD_TYPE_PLUGIN_DLS *plugin;
    struct PLUGIN_DLS *dls;
    struct DB *db;                                                                   /* Database Watchdog */

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info( Config.log, DEBUG_DLS, "DLS: Charger_plugins: Unable to open database" );
       return;
     }

    if (Recuperer_plugins_dlsDB( Config.log, db ))
     { do
        { plugin = Recuperer_plugins_dlsDB_suite( Config.log, db );
          if (!plugin)
           { Libere_DB_SQL( Config.log, &db );
             Config.compil = 0;
             return;
           }
   
          dls = (struct PLUGIN_DLS *)g_malloc0( sizeof(struct PLUGIN_DLS) );
          if (!dls)
           { Info( Config.log, DEBUG_DLS, "DLS: Charger_plugins: out of memory" );
             g_free(plugin);
             return;
           }
   
          memcpy( &dls->plugindb, plugin, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
          g_free(plugin);

                                                                      /* Si option "compil" au demarrage" */
          if (Config.compil == 1) Compiler_source_dls( NULL, dls->plugindb.id );
          if (Charger_un_plugin( dls )==TRUE)
           { Info_c( Config.log, DEBUG_DLS, "DLS: Plugin DLS charge", dls->plugindb.nom ); }
        } while ( TRUE );
     }
    else  { Info( Config.log, DEBUG_DLS, "DLS: Charger_plugins: Unable to load plugins" );
            return;
          }
 }
/**********************************************************************************************************/
/* Activer_plugin_by_id: Active ou non un plugin by id                                                    */
/* Entrée: l'ID du plugin                                                                                 */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Activer_plugin_by_id ( gint id, gboolean actif )
  { struct PLUGIN_DLS *plugin;
    GList *plugins;

    pthread_mutex_lock( &Partage->com_dls.synchro );
    plugins = Partage->com_dls.Plugins;
    while(plugins)                                                       /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS *)plugins->data;

       if ( plugin->plugindb.id == id )
        { plugin->plugindb.on = actif;
          plugin->conso = 0.0;
          plugin->starting = 1;
          Info_n( Config.log, DEBUG_DLS, "DLS: Activer_plugin_by_id done", plugin->plugindb.id );
          break;
        }
       plugins = plugins->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/*--------------------------------------------------------------------------------------------------------*/
