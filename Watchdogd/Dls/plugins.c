/******************************************************************************************************************************/
/* Watchdogd/Dls/plugins.c  -> Gestion des plugins pour DLS                                                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        dim. 02 janv. 2011 19:04:47 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
 /*****************************************************************************************************************************/
/*
 * plugins.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
/* Dls_get_plugin_by_tech_id: Recupere le plugin depuis son tech_id                                                           */
/* Entrée: Le tech_id du plugin a récupérer                                                                                   */
/* Sortie: le plugin DLS ou NULL si besoin                                                                                    */
/******************************************************************************************************************************/
 struct DLS_PLUGIN *Dls_get_plugin_by_tech_id ( gchar *tech_id )
  { GSList *liste = Partage->com_dls.Dls_plugins;
    while (liste)
     { struct DLS_PLUGIN *plugin;
       plugin = (struct DLS_PLUGIN *)liste->data;
       if ( ! strcasecmp(plugin->tech_id, tech_id ) ) return(plugin);
       liste = liste->next;
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_foreach_dls_tree: Parcours recursivement l'arbre DLS et execute des commandes en parametres                            */
/* Entrée : le Dls_tree et les fonctions a appliquer                                                                          */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 void Dls_foreach_plugins ( gpointer user_data, void (*do_plugin) (gpointer user_data, struct DLS_PLUGIN *) )
  { GSList *liste;
    pthread_mutex_lock( &Partage->com_dls.synchro );
    liste = Partage->com_dls.Dls_plugins;
    while (liste)
     { struct DLS_PLUGIN *plugin = liste->data;
       do_plugin( user_data, plugin );
       liste = liste->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Dls_stop_plugin_reel: Stoppe un plugin                                                                                     */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_stop_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = user_data;
    if ( strcasecmp ( tech_id, plugin->tech_id ) ) return;

    plugin->enable = FALSE;
    plugin->start_date = 0;
    plugin->conso  = 0.0;
    Info_new( __func__, Partage->com_dls.Thread_debug, LOG_NOTICE, "'%s' stopped (%s)",
              plugin->tech_id, plugin->name );
  }
/******************************************************************************************************************************/
/* Dls_auto_create_plugin: Créé automatiquement le plugin en parametre (tech_id, nom)                                         */
/* Entrées: le tech_id (unique) et le nom associé                                                                             */
/* Sortie: -1 si pb, id sinon                                                                                                 */
/******************************************************************************************************************************/
 gboolean Dls_auto_create_plugin( gchar *tech_id, gchar *description )
  { JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_ERR, "Memory error for DLS create %s", tech_id );
       return(FALSE);
     }
    Json_node_add_string ( RootNode, "tech_id", tech_id );
    Json_node_add_string ( RootNode, "description", description );

    JsonNode *api_result = Http_Post_to_global_API ( "/run/dls/create", RootNode );
    if (api_result == NULL || Json_get_int ( api_result, "api_status" ) != SOUP_STATUS_OK)
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_ERR,
                 "API Request for DLS CREATE failed. '%s' not created.", tech_id );
       Json_node_unref ( api_result );
       Json_node_unref ( RootNode );
       return(FALSE);
     }
    Json_node_unref ( api_result );
    Json_node_unref ( RootNode );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Dls_start_plugin_reel: Demarre le plugin en parametre                                                                      */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_start_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = user_data;
    if ( strcasecmp ( tech_id, plugin->tech_id ) ) return;

    plugin->enable = TRUE;
    plugin->conso  = 0.0;
    plugin->start_date = time(NULL);
    plugin->vars.resetted = FALSE;
    Info_new( __func__, Partage->com_dls.Thread_debug, LOG_NOTICE, "'%s' started (%s)", plugin->tech_id, plugin->name );
  }
/******************************************************************************************************************************/
/* Activer_plugin_by_id: Active ou non un plugin by id                                                                        */
/* Entrée: l'ID du plugin                                                                                                     */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Activer_plugin ( gchar *tech_id, gboolean actif )
  { if (actif) Dls_foreach_plugins ( tech_id, Dls_start_plugin_reel );
          else Dls_foreach_plugins ( tech_id, Dls_stop_plugin_reel );
  }
/******************************************************************************************************************************/
/* Dls_Save_CodeC_to_disk: Enregistre un codec sur le disque pour le tech_id en parametre                                     */
/* Entrée: Le tech_id du DLS a sauver, et le codeC associé                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Dls_Save_CodeC_to_disk ( gchar *tech_id, gchar *codec )
  { gchar source_file[128];

    Info_new( __func__, Partage->com_dls.Thread_debug, LOG_NOTICE, "Saving '%s' started", tech_id );
    g_snprintf( source_file, sizeof(source_file), "Dls/%s.c", tech_id );
    unlink(source_file);
    gint id_fichier = open( source_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id_fichier<0 || lockf( id_fichier, F_TLOCK, 0 ) )
     { Info_new( __func__, Config.log_trad, LOG_WARNING, "Open file '%s' for write failed (%s)",
                 source_file, strerror(errno) );
       close(id_fichier);
       return(FALSE);
     }

    gint taille_codec = strlen(codec);
    gint retour_write = write( id_fichier, codec, taille_codec );
    close(id_fichier);
    if (retour_write<0)
     { Info_new( __func__, Config.log_trad, LOG_ERR, "Write %d bytes to file '%s' failed (%s)",
                 taille_codec, source_file, strerror(errno) );
       return(FALSE);
     }
    Info_new( __func__, Config.log_trad, LOG_DEBUG, "Write %d bytes to file '%s' OK.", taille_codec, source_file );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Compiler_source_dls: Compilation de la source DLS en librairie                                                             */
/* Entrée: Le tech_id du DLS a compiler, et le codeC associé                                                                  */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Dls_Compiler_source_dls( gchar *tech_id )
  { gchar source_file[128], target_file[128];

    Info_new( __func__, Partage->com_dls.Thread_debug, LOG_NOTICE, "Compilation of '%s' started", tech_id );
    gint top = Partage->top;
    g_snprintf( source_file, sizeof(source_file), "Dls/%s.c", tech_id );
    g_snprintf( target_file, sizeof(target_file),  "Dls/libdls%s.so", tech_id );
    Info_new( __func__, Config.log_trad, LOG_DEBUG, "Starting GCC." );

    gint pidgcc = fork();
    if (pidgcc<0)
     { Info_new( __func__, Config.log_trad, LOG_WARNING, "Fils: envoi erreur Fork GCC '%s'", tech_id );
       return(FALSE);
     }
    else if (!pidgcc)
     { execlp( "gcc", "gcc", "-I/usr/include/glib-2.0", "-I/usr/lib/glib-2.0/include", "-I/usr/lib64/glib-2.0/include",
               "-I/usr/lib/i386-linux-gnu/glib-2.0/include", "-I/usr/lib/x86_64-linux-gnu/glib-2.0/include",
               "-shared", "--no-gnu-unique", "-Wno-unused-variable", "-ggdb", "-Wall", "-lwatchdog-dls",
               source_file, "-fPIC", "-o", target_file, NULL );
       _exit(0);
     }

    Info_new( __func__, Config.log_trad, LOG_DEBUG, "Waiting for gcc to finish pid %d", pidgcc );
    gint wcode;
    waitpid(pidgcc, &wcode, 0 );
    gint gcc_return_code = WEXITSTATUS(wcode);
    if (gcc_return_code == 1) unlink(target_file);
    Info_new( __func__, Config.log_trad, LOG_DEBUG, "gcc pid %d is down with return code %d", pidgcc, gcc_return_code );
    Info_new( __func__, Config.log_trad, LOG_INFO, "Compilation of '%s' finished in %05.1fs", tech_id, (Partage->top - top)/10.0 );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Dls_Dlopen_plugin: Ouverture dyamique d'un plugin dont la structure est en parametre                                       */
/* Entrée: Le plugin D.L.S                                                                                                    */
/* Sortie: FALSE si problème                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Dls_Dlopen_plugin ( struct DLS_PLUGIN *plugin )
  { gchar nom_fichier_absolu[60];

    if (Partage->com_dls.Thread_run == FALSE) return(FALSE);          /* si l'instance est en cours d'arret, on sort de suite */
    g_snprintf( nom_fichier_absolu, sizeof(nom_fichier_absolu), "Dls/libdls%s.so", plugin->tech_id );

    if (plugin->handle)                                                                     /* Si deja chargé, on le décharge */
     { if(plugin->go && plugin->enable) { plugin->go = NULL; sleep(1); }      /* Si le plugin tourne, on le sort de la boucle */
       if (dlclose( plugin->handle ))
        { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_NOTICE, "'%s': dlclose error '%s' (%s)",
                    plugin->tech_id, dlerror(), plugin->shortname );
        }
       plugin->handle = NULL;
       Info_new( __func__, Partage->com_dls.Thread_debug, LOG_NOTICE, "'%s' unloaded (%s)",
                 plugin->tech_id, plugin->shortname );
     }

    plugin->handle = dlopen( nom_fichier_absolu, RTLD_LOCAL | RTLD_NOW );                   /* Ouverture du fichier librairie */
    if (!plugin->handle)
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_WARNING,
                   "'%s': dlopen failed (%s)", plugin->tech_id, dlerror() );
       return(FALSE);
     }

    plugin->version = dlsym( plugin->handle, "version" );                                         /* Recherche de la fonction */
    if (!plugin->version)
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_WARNING,
                "'%s' does not provide version function", plugin->tech_id );
       dlclose( plugin->handle );
       plugin->handle = NULL;
       return(FALSE);
     }

    plugin->remap_all_alias = dlsym( plugin->handle, "remap_all_alias" );                         /* Recherche de la fonction */
    if (!plugin->remap_all_alias)
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_WARNING,
                "'%s' does not provide remap_all_alias function", plugin->tech_id );
       dlclose( plugin->handle );
       plugin->handle = NULL;
       return(FALSE);
     }

/*------------------------------------------------------- Init des variables -------------------------------------------------*/
    plugin->conso = 0.0;
    if (plugin->enable) plugin->start_date = time(NULL);
                else plugin->start_date = 0;
    plugin->vars.debug = plugin->debug;                            /* Recopie du champ de debug depuis la DB vers la zone RUN */

/*------------------------------------------------------- Chargement GO ------------------------------------------------------*/
    plugin->go = dlsym( plugin->handle, "Go" );                                              /* Recherche de la fonction 'Go' */
    if (!plugin->go)
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_WARNING,
                 "'%s' failed sur absence GO", plugin->tech_id );
       dlclose( plugin->handle );
       plugin->handle = NULL;
       return(FALSE);
     }

    Info_new( __func__, Partage->com_dls.Thread_debug, LOG_NOTICE,
              "'%s' dlopened (%s)", plugin->tech_id, plugin->shortname );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Reseter_all_bit_interne: Met a 0 et decharge tous les bits interne d'un plugin                                             */
/* Entrée: le plugin                                                                                                          */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Reseter_all_bit_interne ( struct DLS_PLUGIN *plugin )
  { GSList *liste_bit;

    liste_bit = plugin->Dls_data_TEMPO;
    while(liste_bit)
     { struct DLS_TEMPO *tempo = liste_bit->data;
       tempo->status = DLS_TEMPO_NOT_COUNTING;                                              /* La tempo ne compte pas du tout */
       tempo->state  = FALSE;
       tempo->init   = FALSE;
       liste_bit = g_slist_next(liste_bit);
     }

    liste_bit = plugin->Dls_data_MESSAGE;                                             /* Decharge tous les messages du module */
    while(liste_bit)
     { struct DLS_MESSAGE *msg = liste_bit->data;
       Dls_data_set_MESSAGE ( &plugin->vars, msg, FALSE, FALSE );
       liste_bit = g_slist_next(liste_bit);
     }

    liste_bit = plugin->Dls_data_MONO;                                             /* Decharge tous les monostables du module */
    while(liste_bit)
     { struct DLS_MONO *mono = liste_bit->data;
       Dls_data_set_MONO ( &plugin->vars, mono, FALSE );
       liste_bit = g_slist_next(liste_bit);
     }

    liste_bit = plugin->Dls_data_BI;                                                    /* Decharge tous les bieens du module */
    while(liste_bit)
     { struct DLS_BI *bi = liste_bit->data;
       Dls_data_set_BI   ( &plugin->vars, bi, FALSE );
       liste_bit = g_slist_next(liste_bit);
     }

    liste_bit = plugin->Dls_data_WATCHDOG;                                          /* Decharge tous les watchdogs du module */
    while(liste_bit)
     { struct DLS_WATCHDOG *wtd = liste_bit->data;
       Dls_data_set_WATCHDOG ( &plugin->vars, wtd, FALSE );
       liste_bit = g_slist_next(liste_bit);
     }

    liste_bit = plugin->Dls_data_VISUEL;                                              /* Decharge tous les visuels du module */
    while(liste_bit)
     { struct DLS_VISUEL *visu = liste_bit->data;
       Dls_data_set_VISUEL ( &plugin->vars, visu, "resetted", "black", FALSE, "resetted" );
       liste_bit = g_slist_next(liste_bit);
     }

  }
/******************************************************************************************************************************/
/* Dls_plugins_remap_all_alias: remap les alias d'un plugin donné                                                             */
/* Entrée: le plugin                                                                                                          */
/* Sortie : les alias sont mappés                                                                                             */
/******************************************************************************************************************************/
 static void Dls_plugins_remap_all_alias ( void )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    GSList *liste = Partage->com_dls.Dls_plugins;
    while (liste)
     { struct DLS_PLUGIN *plugin = liste->data;
       if (plugin->remap_all_alias) plugin->remap_all_alias();
       liste = g_slist_next(liste);
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Dls_Importer_un_plugin: Ajoute ou Recharge un plugin dans la liste des plugins                                             */
/* Entrée: les données JSON recu de la requete HTTP                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static struct DLS_PLUGIN *Dls_Importer_un_plugin ( gchar *tech_id )
  { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_INFO, "Starting import of plugin '%s'", tech_id );

    JsonNode *api_result = Http_Get_from_global_API ( "/run/dls/load", "tech_id=%s", tech_id );
    if (api_result == NULL || Json_get_int ( api_result, "api_status" ) != SOUP_STATUS_OK)
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_ERR, "'%s': API Error.", tech_id );
       Json_node_unref ( api_result );
       return(NULL);
     }

    if ( !Json_has_member ( api_result, "codec" ) )
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_ERR, "'%s': Missing CodeC.", tech_id );
       Json_node_unref(api_result);
       return(NULL);
     }

    if ( !Json_has_member ( api_result, "enable" ) || Json_get_bool ( api_result, "enable" ) == FALSE )
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_WARNING, "'%s': Not enabled.", tech_id );
       Json_node_unref(api_result);
       return(NULL);
     }

    Dls_Save_CodeC_to_disk ( tech_id, Json_get_string ( api_result, "codec" ) );
    Dls_Compiler_source_dls ( tech_id );

    struct DLS_PLUGIN *plugin = Dls_get_plugin_by_tech_id ( tech_id );
    if (plugin == NULL)                                                            /* si pas trouvé, on créé l'enregistrement */
     { plugin = g_try_malloc0 ( sizeof (struct DLS_PLUGIN) );
       if (plugin)                                                                              /* Peuplement de la structure */
        { g_snprintf ( plugin->tech_id,   sizeof(plugin->tech_id),   "%s", tech_id );
          g_snprintf ( plugin->name,      sizeof(plugin->name),      "%s", Json_get_string ( api_result, "name" ) );
          g_snprintf ( plugin->shortname, sizeof(plugin->shortname), "%s", Json_get_string ( api_result, "shortname" ) );
          plugin->debug  = Json_get_bool ( api_result, "debug" );
          plugin->enable = Json_get_bool ( api_result, "enable" );
          pthread_mutex_lock( &Partage->com_dls.synchro );
          Partage->com_dls.Dls_plugins = g_slist_append( Partage->com_dls.Dls_plugins, plugin );          /* Ajout a la liste */
          pthread_mutex_unlock( &Partage->com_dls.synchro );
        }
     }
    if (!plugin)                                       /* si vraiment on arrive pas a reserver ou trouver la mémoire, on sort */
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_ERR, "'%s' Memory error", tech_id );
       Json_node_unref(api_result);
       return(NULL);
     }

    if (plugin->Dls_data_CI) { g_slist_free_full ( plugin->Dls_data_CI, (GDestroyNotify) g_free ); plugin->Dls_data_CI = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_CI", Dls_data_CI_create_by_array, plugin );

    if (plugin->Dls_data_CH) { g_slist_free_full ( plugin->Dls_data_CH, (GDestroyNotify) g_free ); plugin->Dls_data_CH = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_CH", Dls_data_CH_create_by_array, plugin );

    if (plugin->Dls_data_DI) { g_slist_free_full ( plugin->Dls_data_DI, (GDestroyNotify) g_free ); plugin->Dls_data_DI = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_DI", Dls_data_DI_create_by_array, plugin );

    if (plugin->Dls_data_DO) { g_slist_free_full ( plugin->Dls_data_DO, (GDestroyNotify) g_free ); plugin->Dls_data_DO = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_DO", Dls_data_DO_create_by_array, plugin );

    if (plugin->Dls_data_AI) { g_slist_free_full ( plugin->Dls_data_AI, (GDestroyNotify) g_free ); plugin->Dls_data_AI = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_AI", Dls_data_AI_create_by_array, plugin );

    if (plugin->Dls_data_AO) { g_slist_free_full ( plugin->Dls_data_AO, (GDestroyNotify) g_free ); plugin->Dls_data_AO = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_AO", Dls_data_AO_create_by_array, plugin );

    if (plugin->Dls_data_MONO) { g_slist_free_full ( plugin->Dls_data_MONO, (GDestroyNotify) g_free ); plugin->Dls_data_MONO = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_MONO", Dls_data_MONO_create_by_array, plugin );

    if (plugin->Dls_data_BI) { g_slist_free_full ( plugin->Dls_data_BI, (GDestroyNotify) g_free ); plugin->Dls_data_BI = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_BI", Dls_data_BI_create_by_array, plugin );

    if (plugin->Dls_data_VISUEL) { g_slist_free_full ( plugin->Dls_data_VISUEL, (GDestroyNotify) g_free ); plugin->Dls_data_VISUEL = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_VISUEL", Dls_data_VISUEL_create_by_array, plugin );

    if (plugin->Dls_data_MESSAGE) { g_slist_free_full ( plugin->Dls_data_MESSAGE, (GDestroyNotify) g_free ); plugin->Dls_data_MESSAGE = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_MESSAGE", Dls_data_MESSAGE_create_by_array, plugin );

    if (plugin->Dls_data_REGISTRE) { g_slist_free_full ( plugin->Dls_data_REGISTRE, (GDestroyNotify) g_free ); plugin->Dls_data_REGISTRE = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_REGISTRE", Dls_data_REGISTRE_create_by_array, plugin );

    if (plugin->Dls_data_WATCHDOG) { g_slist_free_full ( plugin->Dls_data_WATCHDOG, (GDestroyNotify) g_free ); plugin->Dls_data_WATCHDOG = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_WATCHDOG", Dls_data_WATCHDOG_create_by_array, plugin );

    if (plugin->Dls_data_TEMPO) { g_slist_free_full ( plugin->Dls_data_TEMPO, (GDestroyNotify) g_free ); plugin->Dls_data_TEMPO = NULL; }
    Json_node_foreach_array_element ( api_result, "mnemos_TEMPO", Dls_data_TEMPO_create_by_array, plugin );

    if (plugin->Thread_tech_ids) { g_slist_free_full ( plugin->Thread_tech_ids, (GDestroyNotify) g_free ); plugin->Thread_tech_ids = NULL; }

    GList *Thread_tech_ids = json_array_get_elements ( Json_get_array ( api_result, "thread_tech_ids" ) );
    GList *thread_tech_ids = Thread_tech_ids;
    while(thread_tech_ids)
     { JsonNode *element = thread_tech_ids->data;
       plugin->Thread_tech_ids = g_slist_append ( plugin->Thread_tech_ids, Json_get_string ( element, "thread_tech_id" ) );
       thread_tech_ids = g_list_next(thread_tech_ids);
     }
    g_list_free(Thread_tech_ids);
    Json_node_unref(api_result);

    if (!strcasecmp ( tech_id, "SYS" ) )     /* mutex lock non necessaire car si reset, c'est locké par Dls_Reseter_un_plugin */
     { Partage->com_dls.sys_flipflop_5hz      = Dls_data_lookup_BI   ( "SYS", "FLIPFLOP_5HZ" );
       Partage->com_dls.sys_flipflop_2hz      = Dls_data_lookup_BI   ( "SYS", "FLIPFLOP_2HZ" );
       Partage->com_dls.sys_flipflop_1sec     = Dls_data_lookup_BI   ( "SYS", "FLIPFLOP_1SEC" );
       Partage->com_dls.sys_flipflop_2sec     = Dls_data_lookup_BI   ( "SYS", "FLIPFLOP_2SEC" );
       Partage->com_dls.sys_top_5hz           = Dls_data_lookup_MONO ( "SYS", "TOP_5HZ" );
       Partage->com_dls.sys_top_2hz           = Dls_data_lookup_MONO ( "SYS", "TOP_2HZ" );
       Partage->com_dls.sys_top_1sec          = Dls_data_lookup_MONO ( "SYS", "TOP_1SEC" );
       Partage->com_dls.sys_top_5sec          = Dls_data_lookup_MONO ( "SYS", "TOP_5SEC" );
       Partage->com_dls.sys_top_10sec         = Dls_data_lookup_MONO ( "SYS", "TOP_10SEC" );
       Partage->com_dls.sys_top_1min          = Dls_data_lookup_MONO ( "SYS", "TOP_1MIN" );
       Partage->com_dls.sys_bit_per_sec       = Dls_data_lookup_AI   ( "SYS", "DLS_BIT_PER_SEC" );
       Partage->com_dls.sys_tour_per_sec      = Dls_data_lookup_AI   ( "SYS", "DLS_TOUR_PER_SEC" );
       Partage->com_dls.sys_dls_wait          = Dls_data_lookup_AI   ( "SYS", "DLS_WAIT" );
       Partage->com_dls.sys_nbr_msg_queue     = Dls_data_lookup_AI   ( "SYS", "NBR_MSG_QUEUE" );
       Partage->com_dls.sys_nbr_visuel_queue  = Dls_data_lookup_AI   ( "SYS", "NBR_VISUEL_QUEUE" );
       Partage->com_dls.sys_nbr_archive_queue = Dls_data_lookup_AI   ( "SYS", "NBR_ARCHIVE_QUEUE" );
       Partage->com_dls.sys_maxrss            = Dls_data_lookup_AI   ( "SYS", "MAXRSS" );
     }

    plugin->vars.dls_osyn_acquit             = Dls_data_lookup_DI   ( plugin->tech_id, "OSYN_ACQUIT" );
    plugin->vars.dls_comm                    = Dls_data_lookup_MONO ( plugin->tech_id, "COMM" );
    plugin->vars.dls_memsa_ok                = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSA_OK" );
    plugin->vars.dls_memsa_defaut            = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSA_DEFAUT" );
    plugin->vars.dls_memsa_defaut_fixe       = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSA_DEFAUT_FIXE" );
    plugin->vars.dls_memsa_alarme            = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSA_ALARME" );
    plugin->vars.dls_memsa_alarme_fixe       = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSA_ALARME_FIXE" );
    plugin->vars.dls_memssb_veille           = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSSB_VEILLE" );
    plugin->vars.dls_memssb_alerte           = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSSB_ALERTE" );
    plugin->vars.dls_memssb_alerte_fugitive  = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSSB_ALERTE_FUGITIVE" );
    plugin->vars.dls_memssb_alerte_fixe      = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSSB_ALERTE_FIXE" );
    plugin->vars.dls_memssp_ok               = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSSP_OK" );
    plugin->vars.dls_memssp_derangement      = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSSP_DERANGEMENT" );
    plugin->vars.dls_memssp_derangement_fixe = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSSP_DERANGEMENT_FIXE" );
    plugin->vars.dls_memssp_danger           = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSSP_DANGER" );
    plugin->vars.dls_memssp_danger_fixe      = Dls_data_lookup_MONO ( plugin->tech_id, "MEMSSP_DANGER_FIXE" );
    plugin->vars.dls_msg_comm_ok             = Dls_data_lookup_MESSAGE ( plugin->tech_id, "MSG_COMM_OK" );
    plugin->vars.dls_msg_comm_hs             = Dls_data_lookup_MESSAGE ( plugin->tech_id, "MSG_COMM_HS" );

    if (Dls_Dlopen_plugin ( plugin ) == FALSE)
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_ERR, "'%s' Error when dlopening", tech_id ); }

    return(plugin);
  }
/******************************************************************************************************************************/
/* Dls_Importer_un_plugin_by_array: Ajoute un plugin dans la liste des plugins                                                */
/* Entrée: les données JSON recu de la requete HTTP                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Importer_un_plugin_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { Dls_Importer_un_plugin ( Json_get_string ( element, "tech_id" ) ); }
/******************************************************************************************************************************/
/* Dls_Importer_plugins: Importe tous les plugins depuis l'API                                                                */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Importer_plugins ( void )
  { gint top = Partage->top;
    JsonNode *api_result = Http_Post_to_global_API ( "/run/dls/plugins", NULL );
    if (api_result == NULL || Json_get_int ( api_result, "api_status" ) != SOUP_STATUS_OK)
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_ERR, "API Request for /run/dls/plugins failed. No plugin loaded." );
       Json_node_unref ( api_result );
       return;
     }
    Info_new( __func__, Partage->com_dls.Thread_debug, LOG_INFO, "API Request for /run/dls/plugins OK." );
    Json_node_foreach_array_element ( api_result, "plugins", Dls_Importer_un_plugin_by_array, NULL );
    Info_new( __func__, Partage->com_dls.Thread_debug, LOG_NOTICE, "%03d plugins loaded in %03.1fs",
              Json_get_int ( api_result, "nbr_plugins" ), (Partage->top-top)/10.0 );
    Json_node_unref ( api_result );
    Dls_plugins_remap_all_alias();
    Dls_Load_horloge_ticks();
  }
/******************************************************************************************************************************/
/* Dls_Reseter_un_plugin: Recharge un plugin par tech_id                                                                      */
/* Entrée: Le numéro du plugin a décharger                                                                                    */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Reseter_un_plugin ( gchar *tech_id )
  { struct DLS_PLUGIN *found = Dls_get_plugin_by_tech_id ( tech_id );
    if (found) Dls_Export_Data_to_API ( found );      /* Si trouvé, on sauve les valeurs des bits internes avant rechargement */
    struct DLS_PLUGIN *dls = Dls_Importer_un_plugin ( tech_id );
    if (dls)
     { Reseter_all_bit_interne ( dls );
       dls->vars.resetted = TRUE;                                                  /* au chargement, le bit de start vaut 1 ! */
       Info_new( __func__, Partage->com_dls.Thread_debug, LOG_NOTICE, "'%s': resetted", tech_id );
     }
    else Info_new( __func__, Partage->com_dls.Thread_debug, LOG_INFO, "'%s': error when resetting", tech_id );
    Dls_plugins_remap_all_alias();
    Dls_Load_horloge_ticks();
  }
/******************************************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Decharger_plugins ( void )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    while(Partage->com_dls.Dls_plugins)                                                     /* Liberation mémoire des modules */
     { struct DLS_PLUGIN *plugin = Partage->com_dls.Dls_plugins->data;
       Dls_Export_Data_to_API ( plugin );                                         /* Sauvegarde les valeurs des bits internes */
       if (plugin->handle && dlclose( plugin->handle ))
        { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_NOTICE, "dlclose error '%s' for '%s' (%s)",
                    dlerror(), plugin->tech_id, plugin->shortname );
        }

       if (plugin->Dls_data_CI)       g_slist_free_full ( plugin->Dls_data_CI, (GDestroyNotify) g_free );
       if (plugin->Dls_data_CH)       g_slist_free_full ( plugin->Dls_data_CH, (GDestroyNotify) g_free );
       if (plugin->Dls_data_DI)       g_slist_free_full ( plugin->Dls_data_DI, (GDestroyNotify) g_free );
       if (plugin->Dls_data_DO)       g_slist_free_full ( plugin->Dls_data_DO, (GDestroyNotify) g_free );
       if (plugin->Dls_data_AI)       g_slist_free_full ( plugin->Dls_data_AI, (GDestroyNotify) g_free );
       if (plugin->Dls_data_AO)       g_slist_free_full ( plugin->Dls_data_AO, (GDestroyNotify) g_free );
       if (plugin->Dls_data_BI)       g_slist_free_full ( plugin->Dls_data_BI, (GDestroyNotify) g_free );
       if (plugin->Dls_data_MONO)     g_slist_free_full ( plugin->Dls_data_MONO, (GDestroyNotify) g_free );
       if (plugin->Dls_data_REGISTRE) g_slist_free_full ( plugin->Dls_data_REGISTRE, (GDestroyNotify) g_free );
       if (plugin->Dls_data_TEMPO)    g_slist_free_full ( plugin->Dls_data_TEMPO, (GDestroyNotify) g_free );
       if (plugin->Dls_data_WATCHDOG) g_slist_free_full ( plugin->Dls_data_WATCHDOG, (GDestroyNotify) g_free );
       if (plugin->Dls_data_VISUEL)   g_slist_free_full ( plugin->Dls_data_VISUEL, (GDestroyNotify) g_free );
       if (plugin->Dls_data_MESSAGE)  g_slist_free_full ( plugin->Dls_data_MESSAGE, (GDestroyNotify) g_free );

       Partage->com_dls.Dls_plugins = g_slist_remove( Partage->com_dls.Dls_plugins, plugin );
       if (plugin->Arbre_Comm) g_slist_free(plugin->Arbre_Comm);
                                                                             /* Destruction de l'entete associé dans la GList */
       Info_new( __func__, Partage->com_dls.Thread_debug, LOG_INFO, "plugin '%s' unloaded (%s)",
                 plugin->tech_id, plugin->name );
       g_free( plugin );
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Dls_Debug_plugin_reel: Active le debug d'un plugin                                                                         */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Debug_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = (gchar *)user_data;
    if ( ! strcasecmp ( plugin->tech_id, tech_id ) )
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_DEBUG, "'%s' debug started ('%s')",
                 plugin->tech_id, plugin->name );
       plugin->debug = plugin->vars.debug = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Dls_Undebug_plugin_reel: Désactive le debug d'un plugin                                                                    */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Undebug_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = (gchar *)user_data;
    if ( ! strcasecmp ( plugin->tech_id, tech_id ) )
     { Info_new( __func__, Partage->com_dls.Thread_debug, LOG_DEBUG, "'%s' debug stopped ('%s')",
                 plugin->tech_id, plugin->name );
       plugin->debug = plugin->vars.debug = FALSE;
     }
  }
/******************************************************************************************************************************/
/* Dls_Debug_plugin: Active ou non le debug d'un plugin                                                                       */
/* Entrée: le tech_id et le choix actif ou non                                                                                */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Debug_plugin ( gchar *tech_id, gboolean actif )
  { if (actif) Dls_foreach_plugins ( tech_id, Dls_Debug_plugin_reel );
          else Dls_foreach_plugins ( tech_id, Dls_Undebug_plugin_reel );
  }
/******************************************************************************************************************************/
/* Dls_Acquitter_plugin_reel: Acquitte le plugin DLS en parametre                                                             */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Acquitter_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = user_data;
    if ( ! strcasecmp ( plugin->tech_id, tech_id ) )
     { Info_new( __func__, plugin->vars.debug, LOG_NOTICE,
                 "'%s' acquitté ('%s')", plugin->tech_id, plugin->shortname );
       Envoyer_commande_dls_data ( plugin->tech_id, "OSYN_ACQUIT" );
     }
  }
/******************************************************************************************************************************/
/* Activer_plugin_by_id: Active ou non un plugin by id                                                                        */
/* Entrée: l'ID du plugin                                                                                                     */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Acquitter_plugin ( gchar *tech_id )
  { Dls_foreach_plugins ( tech_id, Dls_Acquitter_plugin_reel ); }
/*----------------------------------------------------------------------------------------------------------------------------*/
