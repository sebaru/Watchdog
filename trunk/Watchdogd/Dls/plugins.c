/******************************************************************************************************************************/
/* Watchdogd/Dls/plugins.c  -> Gestion des plugins pour DLS                                                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        dim. 02 janv. 2011 19:04:47 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
 /*****************************************************************************************************************************/
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
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/file.h>                                                                /* Gestion des verrous sur les fichiers */
 #include <sys/wait.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"

 #ifndef REP_INCLUDE_GLIB
 #define REP_INCLUDE_GLIB  "/usr/include/glib-2.0"
 #endif

/******************************************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                                              */
/* Entrée: Le nom de fichier correspondant                                                                                    */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Charger_un_plugin ( struct PLUGIN_DLS *dls )
  { gchar nom_fichier_absolu[60];
    gboolean retour;

    g_snprintf( nom_fichier_absolu, sizeof(nom_fichier_absolu), "Dls/libdls%d.so", dls->plugindb.id );
    strncpy( dls->nom_fichier, nom_fichier_absolu, sizeof(dls->nom_fichier) );                 /* Init des variables communes */
    dls->starting = 1;
    dls->conso    = 0.0;

    retour = FALSE;                                                                          /* Par défaut, on retourne FALSE */
    dls->handle = dlopen( nom_fichier_absolu, RTLD_GLOBAL | RTLD_NOW );                     /* Ouverture du fichier librairie */
    if (!dls->handle)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                "Charger_un_plugin: Candidat %04d failed (%s)", dls->plugindb.id, dlerror() );
     }
    else
     { dls->go = dlsym( dls->handle, "Go" );                                                 /* Recherche de la fonction 'Go' */
       if (!dls->go) { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                                "Charger_un_plugin: Candidat %04d failed sur absence GO", dls->plugindb.id ); 
                       dlclose( dls->handle );
                       dls->handle = NULL;
                     }
       else
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "Charger_un_plugin: plugin %04d loaded (%s)",
                    dls->plugindb.id, dls->plugindb.nom );
          retour = TRUE;
        }
     }
    pthread_mutex_lock( &Partage->com_dls.synchro );                                  /* Ajout dans la liste de travail D.L.S */
    Partage->com_dls.Plugins = g_slist_append( Partage->com_dls.Plugins, dls );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    return(retour);
  }
/******************************************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                                              */
/* Entrée: Le nom de fichier correspondant                                                                                    */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Charger_un_plugin_by_id ( gint id )
  { struct CMD_TYPE_PLUGIN_DLS *plugin_dls;
    struct PLUGIN_DLS *dls;
    struct DB *db;                                                                                       /* Database Watchdog */

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "Charger_un_plugin_by_id: Unable to open database" );
       return(FALSE);
     }

    plugin_dls = Rechercher_plugin_dlsDB( id );
    Libere_DB_SQL( &db );

    if (!plugin_dls)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING, "Charger_un_plugin_by_id: Plugin %04d non trouvé", id );
       return(FALSE);
     }

    dls = (struct PLUGIN_DLS *)g_try_malloc0( sizeof(struct PLUGIN_DLS) );
    if (!dls)
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "Charger_un_plugin_by_id: out of memory for id=%04d", id );
       g_free(plugin_dls);
       return(FALSE);
     }

    memcpy( &dls->plugindb, plugin_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
    g_free(plugin_dls);

    return ( Charger_un_plugin ( dls ) );
  }
/******************************************************************************************************************************/
/* Retirer_plugins: Decharge toutes les librairies                                                                            */
/* Entrée: Le numéro du plugin a décharger                                                                                    */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Decharger_un_plugin_by_id ( gint id )
  { struct PLUGIN_DLS *plugin;
    GSList *plugins;

    pthread_mutex_lock( &Partage->com_dls.synchro );
    plugins = Partage->com_dls.Plugins;
    while(plugins)                                                                                 /* Pour chacun des plugins */
     { plugin = (struct PLUGIN_DLS *)plugins->data;

       if ( plugin->plugindb.id == id )
        { if (plugin->handle) dlclose( plugin->handle );
          Partage->com_dls.Plugins = g_slist_remove( Partage->com_dls.Plugins, plugin );
          g_free( plugin );
          Info_new( Config.log, Config.log_dls, LOG_INFO,
                   "Decharger_un_plugin_by_id: plugin %04d unloaded", plugin->plugindb.id );
          break;
        }
       plugins = plugins->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    if (plugins == NULL)
     { Info_new( Config.log, Config.log_dls, LOG_INFO,
                "Decharger_un_plugin_by_id: plugin %04d not found", id );
     }
  }
/******************************************************************************************************************************/
/* Retirer_un_plugin: Decharge le plugin dont le numero est en parametre                                                      */
/* Entrée: L'identifiant du plugin                                                                                            */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Reseter_un_plugin ( gint id )
  { Info_new( Config.log, Config.log_dls, LOG_INFO, "Reseter_un_plugin: Reset plugin %04d", id );

    Decharger_un_plugin_by_id ( id );
    Charger_un_plugin_by_id ( id );
  }
/******************************************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Decharger_plugins ( void )
  { struct PLUGIN_DLS *plugin;

    pthread_mutex_lock( &Partage->com_dls.synchro );
    while(Partage->com_dls.Plugins)                                                         /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS *)Partage->com_dls.Plugins->data;
       if (plugin->handle) dlclose( plugin->handle );
       Partage->com_dls.Plugins = g_slist_remove( Partage->com_dls.Plugins, plugin );
                                                                             /* Destruction de l'entete associé dans la GList */
       Info_new( Config.log, Config.log_dls, LOG_INFO, "Decharger_plugins: plugin %04d unloaded (%s)",
                 plugin->plugindb.id, plugin->plugindb.nom );
       g_free( plugin );
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Charger_plugins: Ouverture de toutes les librairies possibles pour le DLS                                                  */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_plugins ( void )
  { struct CMD_TYPE_PLUGIN_DLS *plugin;
    struct PLUGIN_DLS *dls;
    struct DB *db;                                                                                       /* Database Watchdog */

    if (Recuperer_plugins_dlsDB( &db ))
     { do
        { plugin = Recuperer_plugins_dlsDB_suite( &db );
          if (!plugin)
           { Config.compil = 0;                                                                          /* fin de traitement */
             return;
           }
   
          dls = (struct PLUGIN_DLS *)g_try_malloc0( sizeof(struct PLUGIN_DLS) );
          if (!dls)
           { Info_new( Config.log, Config.log_dls, LOG_ERR, "Charger_plugins: out of memory" );
             g_free(plugin);
           }
          else { memcpy( &dls->plugindb, plugin, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
                 g_free(plugin);
                                                                                          /* Si option "compil" au demarrage" */
                 if (Config.compil == 1) Compiler_source_dls( FALSE, FALSE, dls->plugindb.id, NULL, 0 );
                 Charger_un_plugin( dls );                                                            /* Chargement du plugin */
               }
        } while ( TRUE );
     }
    else  { Info_new( Config.log, Config.log_dls, LOG_ERR, "Charger_plugins: Unable to load plugins" );
            return;
          }
 }
/******************************************************************************************************************************/
/* Activer_plugin_by_id: Active ou non un plugin by id                                                                        */
/* Entrée: l'ID du plugin                                                                                                     */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Activer_plugin_by_id ( gint id, gboolean actif )
  { struct PLUGIN_DLS *plugin;
    GSList *plugins;

    pthread_mutex_lock( &Partage->com_dls.synchro );
    plugins = Partage->com_dls.Plugins;
    while(plugins)                                                                                  /* Pour chacun des plugin */
     { plugin = (struct PLUGIN_DLS *)plugins->data;

       if ( plugin->plugindb.id == id )
        { plugin->plugindb.on = actif;
          plugin->conso = 0.0;
          plugin->starting = 1;
          Info_new( Config.log, Config.log_dls, LOG_INFO, "Activer_plugin_by_id: id %04d %s (%s)",
                    plugin->plugindb.id, (actif ? "started" : "stopped"), plugin->plugindb.nom );
          break;
        }
       plugins = plugins->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Proto_compiler_source_dls: Compilation de la source DLS                                                                    */
/* Entrée: le client demandeur et le groupe en question                                                                       */
/* Sortie: code d'erreur ou 0 si OK                                                                                           */
/******************************************************************************************************************************/
 gint Compiler_source_dls( gboolean new, gboolean reset, gint id, gchar *buffer, gint taille_buffer )
  { gint retour;

    if (buffer) memset (buffer, 0, taille_buffer);                                                 /* RAZ du buffer de sortie */

    Info_new( Config.log, Config.log_dls, LOG_NOTICE,
             "THRCompil: Compiler_source_dls: Compilation module DLS %d", id );
    retour = Traduire_DLS( new, id );
    Info_new( Config.log, Config.log_dls, LOG_DEBUG,
             "THRCompil: Compiler_source_dls: fin traduction %d", retour );

    if (retour == TRAD_DLS_ERROR_FILE)                                                /* Retour de la traduction D.L.S vers C */
     { Info_new( Config.log, Config.log_dls, LOG_DEBUG,
               "THRCompil: Compiler_source_dls: envoi erreur file Traduction D.L.S %d", id );
      Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_LOAD_SOURCE );
      return( DLS_COMPIL_ERROR_LOAD_SOURCE );
     }

    if ( buffer )                             /* Chargement de fichier de log dans le buffer mis à disposition par l'appelant */
     { gint id_fichier;
       gchar log[20];

       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "THRCompil: Compiler_source_dls: Chargement du fichier de log D.L.S %d", id );
       g_snprintf( log, sizeof(log), "Dls/%d.log", id );

       id_fichier = open( log, O_RDONLY, 0 );                /* Ouverture du fichier log et chargement du contenu dans buffer */
       if (id_fichier<0)
        { Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_LOAD_LOG );
          return(DLS_COMPIL_ERROR_LOAD_LOG);
        }
       else { int nbr_car, index_buffer_erreur;
              nbr_car = index_buffer_erreur = 0; 
              while ( (nbr_car = read (id_fichier, buffer + index_buffer_erreur,
                                       taille_buffer-1-index_buffer_erreur )) > 0 )
               { index_buffer_erreur+=nbr_car; }
              close(id_fichier);
            }
     }

    if ( retour == TRAD_DLS_ERROR )
     { Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_TRAD );
       return ( DLS_COMPIL_ERROR_TRAD );
     }

    if (retour == TRAD_DLS_WARNING || retour == TRAD_DLS_OK)
     { gint pidgcc;
       pidgcc = fork();
       if (pidgcc<0)
        { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                   "THRCompilFils: Compiler_source_dls: envoi erreur Fork GCC %d", id );
          Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_FORK_GCC );
          return(DLS_COMPIL_ERROR_FORK_GCC);
        }
       else if (!pidgcc)
        { gchar source[80], cible[80];
          g_snprintf( source, sizeof(source), "Dls/%d.c", id );
          g_snprintf( cible,  sizeof(cible),  "Dls/libdls%d.so", id );
          Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                   "THRCompilFils: Proto_compiler_source_dls: GCC start (pid %d) source %s cible %s!",
                    pidgcc, source, cible );
          execlp( "gcc", "gcc", "-I", REP_INCLUDE_GLIB, "-shared", "-o3",
                  "-Wall", "-lwatchdog-dls", source, "-fPIC", "-o", cible, NULL );
          Info_new( Config.log, Config.log_dls, LOG_DEBUG, "THRCompilFils: Proto_compiler_source_dls: lancement GCC failed" );
          _exit(0);
        }

       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
               "THRCompil: Proto_compiler_source_dls: Waiting for gcc to finish pid %d", pidgcc );
       wait4(pidgcc, NULL, 0, NULL );
       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
               "THRCompil: Proto_compiler_source_dls: gcc is down, OK %d", pidgcc );

       if (reset)
        { pthread_mutex_lock( &Partage->com_dls.synchro );                              /* Demande le reset du plugin à D.L.S */
          Partage->com_dls.liste_plugin_reset = g_slist_append ( Partage->com_dls.liste_plugin_reset,
                                                                 GINT_TO_POINTER(id) );
          pthread_mutex_unlock( &Partage->com_dls.synchro );
        }

       Info_new( Config.log, Config.log_dls, LOG_DEBUG, "THRCompil: Compiler_source_dls: end of %d", id );
     }

    if (retour == TRAD_DLS_WARNING)
     { Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_OK_WITH_WARNINGS );
       return( DLS_COMPIL_OK_WITH_WARNINGS );
     }

    Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_OK );
    return( DLS_COMPIL_OK );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
