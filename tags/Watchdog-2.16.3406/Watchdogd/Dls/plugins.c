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

/******************************************************************************************************************************/
/* Check_action_bit_use: Vérifie que les bits d'actions positionnés par le module sont bien owné par celui-ci                 */
/* Entrée: Le plugin D.L.S                                                                                                    */
/* Sortie: FALSE si problème                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Check_action_bit_use ( struct PLUGIN_DLS *dls )
  { gint cpt, dls_id;
    if(!dls) return(FALSE);
    if(!dls->Get_Tableau_bit) return(TRUE);
    if(!dls->Get_Tableau_num) return(TRUE);
    if(!dls->Get_Tableau_msg) return(TRUE);

    for ( cpt=0; dls->Get_Tableau_bit(cpt) != -1; cpt++ )                                          /* Check des bits internes */
     { struct CMD_TYPE_NUM_MNEMONIQUE critere;
       struct CMD_TYPE_MNEMO_BASE *mnemo;
       critere.type = dls->Get_Tableau_bit(cpt);
       critere.num  = dls->Get_Tableau_num(cpt);
       mnemo = Rechercher_mnemo_baseDB_type_num ( &critere );
       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "%s: Test Mnemo %d %d for id %d: mnemo %p", __func__, critere.type, critere.num, dls->plugindb.id, mnemo ); 
       if (!mnemo) return(FALSE);
       dls_id = mnemo->dls_id;
       g_free(mnemo);
       if (dls_id != dls->plugindb.id) return(FALSE);
     }

    for ( cpt=0; dls->Get_Tableau_msg(cpt) != -1; cpt++ )                                          /* Check des bits internes */
     { struct CMD_TYPE_MESSAGE *message;
       message = Rechercher_messageDB ( dls->Get_Tableau_msg(cpt) );
       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "%s: Test MSG %d for id %d: mnemo %p", __func__, dls->Get_Tableau_msg(cpt), dls->plugindb.id, message ); 
       if (!message) return(FALSE);
       dls_id = message->dls_id;
       g_free(message);
       if (dls_id != dls->plugindb.id) return(FALSE);
     }

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                                              */
/* Entrée: Le plugin D.L.S                                                                                                    */
/* Sortie: FALSE si problème                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Charger_un_plugin ( struct PLUGIN_DLS *dls )
  { gchar nom_fichier_absolu[60];
    gboolean retour;

    g_snprintf( nom_fichier_absolu, sizeof(nom_fichier_absolu), "Dls/libdls%06d.so", dls->plugindb.id );
    strncpy( dls->nom_fichier, nom_fichier_absolu, sizeof(dls->nom_fichier) );                 /* Init des variables communes */
    dls->starting = 1;                                                             /* au chargement, le bit de start vaut 1 ! */
    dls->conso    = 0.0;

    retour = FALSE;                                                                          /* Par défaut, on retourne FALSE */
    dls->handle = dlopen( nom_fichier_absolu, RTLD_GLOBAL | RTLD_NOW );                     /* Ouverture du fichier librairie */
    if (!dls->handle)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                "%s: Candidat %06d failed (%s)", __func__, dls->plugindb.id, dlerror() );
     }
    else
     { dls->go = dlsym( dls->handle, "Go" );                                                 /* Recherche de la fonction 'Go' */
       if (!dls->go)
        { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                    "%s: Candidat %06d failed sur absence GO", __func__, dls->plugindb.id ); 
          dlclose( dls->handle );
          dls->handle = NULL;
        }
       if (dls->handle)
        { dls->Get_Tableau_bit = dlsym( dls->handle, "Get_Tableau_bit" );                         /* Recherche de la fonction */
          if (!dls->Get_Tableau_bit)
           { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                      "%s: Candidat %06d does not provide Get_Tableau_bit function", __func__, dls->plugindb.id ); 
             Set_compil_status_plugin_dlsDB( dls->plugindb.id, DLS_COMPIL_WARNING_FUNCTION_MISSING );
           }

          dls->Get_Tableau_num = dlsym( dls->handle, "Get_Tableau_num" );                         /* Recherche de la fonction */
          if (!dls->Get_Tableau_num)
           { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                      "%s: Candidat %06d does not provide Get_Tableau_num function", __func__, dls->plugindb.id ); 
             Set_compil_status_plugin_dlsDB( dls->plugindb.id, DLS_COMPIL_WARNING_FUNCTION_MISSING );
           }

          dls->Get_Tableau_msg = dlsym( dls->handle, "Get_Tableau_msg" );                         /* Recherche de la fonction */
          if (!dls->Get_Tableau_msg)
           { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                      "%s: Candidat %06d does not provide Get_Tableau_msg function", __func__, dls->plugindb.id ); 
             Set_compil_status_plugin_dlsDB( dls->plugindb.id, DLS_COMPIL_WARNING_FUNCTION_MISSING );
           }
         }
       
       Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: plugin %06d loaded (%s)", __func__, dls->plugindb.id, dls->plugindb.nom );
       retour = TRUE;

       if (Check_action_bit_use( dls ) == FALSE )
        { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                   "%s: Candidat %06d -> bit(s) set but not owned by itself... Disabling", __func__, dls->plugindb.id ); 
          Set_compil_status_plugin_dlsDB( dls->plugindb.id, DLS_COMPIL_ERROR_BIT_SET_BUT_NOT_OWNED );
          dls->plugindb.on=FALSE;
        }
       else Info_new( Config.log, Config.log_dls, LOG_INFO,
                     "%s: Candidat %06d -> bit(s) ownership OK", __func__, dls->plugindb.id ); 

     }
    if (dls->plugindb.on) dls->start_date = time(NULL);
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
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Unable to open database", __func__ );
       return(FALSE);
     }

    plugin_dls = Rechercher_plugin_dlsDB( id );
    Libere_DB_SQL( &db );

    if (!plugin_dls)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING, "%s: Plugin %06d non trouvé", __func__, id );
       return(FALSE);
     }

    dls = (struct PLUGIN_DLS *)g_try_malloc0( sizeof(struct PLUGIN_DLS) );
    if (!dls)
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: out of memory for id=%06d", __func__, id );
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
                   "%s: plugin %06d unloaded", __func__, plugin->plugindb.id );
          break;
        }
       plugins = plugins->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    if (plugins == NULL)
     { Info_new( Config.log, Config.log_dls, LOG_INFO,
                "%s: plugin %06d not found", __func__, id );
     }
  }
/******************************************************************************************************************************/
/* Retirer_un_plugin: Decharge le plugin dont le numero est en parametre                                                      */
/* Entrée: L'identifiant du plugin                                                                                            */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Reseter_un_plugin ( gint id )
  { Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: Reset plugin %06d", __func__, id );

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
       Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: plugin %06d unloaded (%s)", __func__,
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

    if (!Recuperer_plugins_dlsDB( &db ))
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Unable to load plugins", __func__ );
       return;
     }
    
    while( (plugin = Recuperer_plugins_dlsDB_suite( &db )) != NULL )
     { dls = (struct PLUGIN_DLS *)g_try_malloc0( sizeof(struct PLUGIN_DLS) );
       if (!dls)
        { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: out of memory", __func__ );
          g_free(plugin);
        }
       else { memcpy( &dls->plugindb, plugin, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
              g_free(plugin);
                                                                                          /* Si option "compil" au demarrage" */
              if (Config.compil == 1) Compiler_source_dls( FALSE, dls->plugindb.id, NULL, 0 );
              Charger_un_plugin( dls );                                                               /* Chargement du plugin */
            }
     }

    Config.compil = 0;                                                                                   /* fin de traitement */
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
        { if (actif == FALSE)
           { plugin->plugindb.on = FALSE;
             plugin->conso = 0.0;
             Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: id %06d stopped (%s)", __func__, plugin->plugindb.id, plugin->plugindb.nom );
           }
          else if (Check_action_bit_use( plugin ) == TRUE )
           { plugin->plugindb.on = TRUE;
             plugin->conso = 0.0;
             plugin->starting = 1;
             plugin->start_date = time(NULL);
             Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: id %06d started (%s)", __func__, plugin->plugindb.id, plugin->plugindb.nom );
           }
          else
           { plugin->plugindb.on = 0;
             plugin->conso = 0.0;
             plugin->starting = 0;
             Info_new( Config.log, Config.log_dls, LOG_WARNING,
                      "%s: Candidat %06d -> bit(s) set but not owned by itself... Disabling", __func__, plugin->plugindb.id ); 
           }
          break;
        }
       plugins = plugins->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Proto_compiler_source_dls: Compilation de la source DLS                                                                    */
/* Entrée: reset=1 s'il faut resetter le plugin après compil, l'id associé, et le buffer de sortie                            */
/* Sortie: code d'erreur ou 0 si OK                                                                                           */
/******************************************************************************************************************************/
 gint Compiler_source_dls( gboolean reset, gint id, gchar *buffer, gint taille_buffer )
  { gint retour, pidgcc, id_fichier;
    gchar chaine[128];
    gint taille_source;
    gchar *Source;
  
    Info_new( Config.log, Config.log_dls, LOG_NOTICE, "%s: Compilation module DLS %06d", __func__, id );
    if (buffer) memset (buffer, 0, taille_buffer);                                                 /* RAZ du buffer de sortie */

    if ( Get_source_dls_from_DB ( id, &Source, &taille_source ) == FALSE )              /* On récupère le source depuis la DB */
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Cannot get Source DLS for id '%06d'", __func__, id );
       return(DLS_COMPIL_EXPORT_DB_FAILED);
     }
     
    g_snprintf( chaine, sizeof(chaine), "Dls/%06d.dls", id );
    unlink(chaine);
    id_fichier = open( chaine, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id_fichier<0 || lockf( id_fichier, F_TLOCK, 0 ) )
     { Info_new( Config.log, Config.log_dls, LOG_WARNING, "%s: Open file '%s' for write failed for %06d (%s)", __func__,
                 chaine, id, strerror(errno) );
       close(id_fichier);
       return(DLS_COMPIL_EXPORT_DB_FAILED);
     }
    else
     { if (write( id_fichier, Source, taille_source )<0)
        { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Write %d bytes to file '%s' failed for %06d (%s)", __func__, 
                    taille_source, chaine, id, strerror(errno) );
         close(id_fichier);
         return(DLS_COMPIL_EXPORT_DB_FAILED);
        }
       else
        { Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: Write %d bytes to file '%s' OK for %06d", __func__,
                    taille_source, chaine, id );
          close(id_fichier);
        }
     }

    retour = Traduire_DLS( id );                                                                            /* Traduction DLS */
    Info_new( Config.log, Config.log_dls, LOG_DEBUG,
             "%s: fin traduction %06d : %d", __func__, id, retour );

    if (retour == TRAD_DLS_ERROR_FILE)                                                /* Retour de la traduction D.L.S vers C */
     { Info_new( Config.log, Config.log_dls, LOG_DEBUG,
               "%s: envoi erreur file Traduction D.L.S %06d", id );
      Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_LOAD_SOURCE );
      return( DLS_COMPIL_ERROR_LOAD_SOURCE );
     }

    if ( buffer )                             /* Chargement de fichier de log dans le buffer mis à disposition par l'appelant */
     { gint id_fichier;
       gchar log[20];

       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "%s: Chargement du fichier de log D.L.S %d", __func__, id );
       g_snprintf( log, sizeof(log), "Dls/%06d.log", id );

       id_fichier = open( log, O_RDONLY, 0 );                /* Ouverture du fichier log et chargement du contenu dans buffer */
       if (id_fichier<0)
        { Info_new( Config.log, Config.log_dls, LOG_ERR,
                "%s: Impossible de charger le fichier de log '%s' : %s", __func__, log, strerror(errno) );
          Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_LOAD_LOG );
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

    pidgcc = fork();
    if (pidgcc<0)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING, "%s_Fils: envoi erreur Fork GCC %06d", __func__, id );
       Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_FORK_GCC );
       return(DLS_COMPIL_ERROR_FORK_GCC);
     }
    else if (!pidgcc)
     { gchar source[80], cible[80];
       g_snprintf( source, sizeof(source), "Dls/%06d.c", id );
       g_snprintf( cible,  sizeof(cible),  "Dls/libdls%06d.so", id );
       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "%s: GCC start (pid %d) source %s cible %s!",
                 __func__, pidgcc, source, cible );
       execlp( "gcc", "gcc", "-I/usr/include/glib-2.0","-I/usr/lib/glib-2.0/include", "-shared", "-o3",
               "-Wall", "-lwatchdog-dls", source, "-fPIC", "-o", cible, NULL );
       Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s_Fils: lancement GCC failed", __func__ );
       _exit(0);
     }

    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: Waiting for gcc to finish pid %d", __func__, pidgcc );
    wait4(pidgcc, NULL, 0, NULL );
    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: gcc is down, OK %d", __func__, pidgcc );

    if (reset)
     { pthread_mutex_lock( &Partage->com_dls.synchro );                                 /* Demande le reset du plugin à D.L.S */
       Partage->com_dls.liste_plugin_reset = g_slist_append ( Partage->com_dls.liste_plugin_reset, GINT_TO_POINTER(id) );
       pthread_mutex_unlock( &Partage->com_dls.synchro );
     }

    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: end of %06d", __func__, id );
    if (retour == TRAD_DLS_WARNING)
     { Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_OK_WITH_WARNINGS );
       return( DLS_COMPIL_OK_WITH_WARNINGS );
     }

    Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_OK );
    return( DLS_COMPIL_OK );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
