/**********************************************************************************************************/
/* Watchdogd/Dls/plugins.c  -> Gestion des plugins pour DLS                                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 05 avr 2009 19:54:47 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * plugins.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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
 #include "Dls.h"

/**********************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                          */
/* Entrée: Le nom de fichier correspondant                                                                */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 static gboolean Charger_un_plugin ( struct PLUGIN_DLS *dls )
  { struct PLUGIN_DLS *plugin;
    gchar nom_fichier_absolu[60];
    void (*Go)(int);
    void *handle;

    g_snprintf( nom_fichier_absolu, sizeof(nom_fichier_absolu), "%s/libdls%d.so", Config.home, dls->id );

    handle = dlopen( nom_fichier_absolu, RTLD_LAZY );
    if (!handle) { Info_n( Config.log, DEBUG_DLS, "DLS: Candidat rejeté ", dls->id );
                   Info_c( Config.log, DEBUG_DLS, "DLS: -- sur ouverture", dlerror() );
                   return(FALSE);
                 }
    Go = dlsym( handle, "Go" );                                         /* Recherche de la fonction 'Go' */
    if (!Go) { Info_n( Config.log, DEBUG_DLS, "DLS: Candidat rejeté sur absence GO", dls->id ); 
               dlclose( handle );
               return(FALSE);
             }

    Info_n( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin: handle", GPOINTER_TO_INT(handle) );
    strncpy( plugin->nom_fichier, nom_fichier_absolu, sizeof(plugin->nom_fichier) );
    plugin->handle  = handle;
    plugin->go      = Go;
    plugin->starting= 1;
    pthread_mutex_lock( &Partage->com_dls.synchro );
    Partage->com_dls.Plugins = g_list_append( Partage->com_dls.Plugins, plugin );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                          */
/* Entrée: Le nom de fichier correspondant                                                                */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 static gboolean Charger_un_plugin_by_id ( gint id )
  { struct PLUGIN_DLS *dls;
    struct DB *db;                                                                   /* Database Watchdog */

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin_by_id: Unable to open database" );
       return(FALSE);
     }

    dls = Rechercher_plugin_dlsDB( Config.log, db, id );
    Libere_DB_SQL( Config.log, &db );

    if (!dls)
     { Info_c( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin_by_id: Plugin non trouvé", id );
       return(FALSE);
     }

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

       if ( plugin->id == id )
        { dlclose( plugin->handle );
          Partage->com_dls.Plugins = g_list_remove( Partage->com_dls.Plugins, plugin );
          g_free( plugin );
          Info_n( Config.log, DEBUG_DLS, "DLS: Retirer_plugin: Dechargé", plugin->id );
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
/* Retirer_plugins: Decharge toutes les librairies                                                        */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Decharger_plugins ( void )
  { struct PLUGIN_DLS *plugin;
    GList *plugins;

    pthread_mutex_lock( &Partage->com_dls.synchro );
    plugins = Partage->com_dls.Plugins;
    while(plugins)                                                       /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS *)plugins->data;
       Info_n( Config.log, DEBUG_DLS, "DLS: Retirer_plugin: tentative dechargement:", plugin->id );
       dlclose( plugin->handle );
       Partage->com_dls.Plugins = g_list_remove( Partage->com_dls.Plugins, plugin );
                                                         /* Destruction de l'entete associé dans la GList */
       g_free( plugin );
       Info_n( Config.log, DEBUG_DLS, "DLS: Retirer_plugin: Dechargé", plugin->id );
       plugins = plugins->next;
     }
    g_list_free( Partage->com_dls.Plugins );
    Partage->com_dls.Plugins = NULL;
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/**********************************************************************************************************/
/* Charger_plugins: Ouverture de toutes les librairies possibles pour le DLS                              */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Charger_plugins ( void )
  { struct PLUGIN_DLS *dls;
    struct DB *db;                                                                   /* Database Watchdog */

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info( Config.log, DEBUG_DLS, "DLS: Charger_plugins: Unable to open database" );
       return;
     }

    Recuperer_plugins_dlsDB( Config.log, db );
    do
     { dls = Recuperer_plugins_dlsDB_suite( Config.log, db );
       if (!dls)
        { Libere_DB_SQL( Config.log, &db );
          return;
        }

       if (Charger_un_plugin( dls )==TRUE)
        { Info_c( Config.log, DEBUG_DLS, "DLS: Plugin DLS charge", dls->nom ); }
     } while ( TRUE );
 }
/*--------------------------------------------------------------------------------------------------------*/
