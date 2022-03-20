/******************************************************************************************************************************/
/* Watchdogd/process.c        Gestion des process                                                                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 11 avr 2009 12:21:45 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * process.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <dirent.h>
 #include <string.h>
 #include <stdio.h>
 #include <locale.h>

 #include <sys/wait.h>
 #include <fcntl.h>
 #include <errno.h>
 #include <dlfcn.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Thread_init: appelé par chaque thread, lors de son démarrage                                                               */
/* Entrée: Le nom du thread, sa classe, la structure afférente, sa version, et sa description                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Process_set_database_version ( struct PROCESS *lib, gint version )
  { lib->database_version = version;
    SQL_Write_new ( "UPDATE processes SET database_version='%d' WHERE uuid='%s'", lib->database_version, lib->uuid );
  }
/******************************************************************************************************************************/
/* Thread_init: appelé par chaque thread, lors de son démarrage                                                               */
/* Entrée: Le nom du thread, sa classe, la structure afférente, sa version, et sa description                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Thread_init ( gchar *name, gchar *classe, struct PROCESS *lib, gchar *version, gchar *description )
  { gchar chaine[128];

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */

    g_snprintf( chaine, sizeof(chaine), "W-%s", name );                           /* Positionne le nom du thread niveau noyau */
    gchar *upper_name = g_ascii_strup ( chaine, -1 );
    prctl(PR_SET_NAME, upper_name, 0, 0, 0 );
    g_free(upper_name);

    lib->Thread_run = TRUE;                                                                             /* Le thread tourne ! */
    time ( &lib->start_time );                                                                           /* Date de demarrage */
    g_snprintf( lib->description, sizeof(lib->description), description );
    g_snprintf( lib->version,     sizeof(lib->version),     version );

    SQL_Write_new ( "UPDATE processes SET started=1, start_time=NOW(), classe='%s', version='%s', database_version='%d', "
                    "description='%s' WHERE uuid='%s'",
                    classe, lib->version, lib->database_version, lib->description, lib->uuid );

    lib->zmq_from_bus  = Zmq_Connect ( ZMQ_SUB, "listen-to-bus", "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    lib->zmq_to_master = Zmq_Connect ( ZMQ_PUSH, "pub-to-master", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
              "%s: UUID %s: Process is UP '%s' (%s) de classe '%s' (debug = %d)", __func__,
              lib->uuid, lib->name, lib->version, classe, lib->Thread_debug );
  }
/******************************************************************************************************************************/
/* Thread_end: appelé par chaque thread, lors de son arret                                                                    */
/* Entrée: Le nom du thread                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Thread_end ( struct PROCESS *lib )
  { Zmq_Close ( lib->zmq_from_bus );
    Zmq_Close ( lib->zmq_to_master );
    SQL_Write_new ( "UPDATE processes SET started = NULL, start_time = NULL, version = NULL WHERE uuid='%s'", lib->uuid );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: UUID %s: Process is DOWN '%s' %s", __func__,
              lib->uuid, lib->name, lib->version );
    if (lib->config) json_node_unref (lib->config);
    lib->Thread_run = FALSE;                                                                    /* Le thread ne tourne plus ! */
    pthread_exit(GINT_TO_POINTER(0));
  }
/******************************************************************************************************************************/
/* Thread_Listen_to_master: appelé par chaque thread pour écouter les messages ZMQ du master                                  */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: JSonNode * sir il y a un message, sinon NULL                                                                       */
/******************************************************************************************************************************/
 JsonNode *Thread_Listen_to_master ( struct PROCESS *lib )
  { return ( Recv_zmq_with_json( lib->zmq_from_bus, lib->name, (gchar *)&lib->zmq_buffer, sizeof(lib->zmq_buffer) ) ); }
/******************************************************************************************************************************/
/* Thread_Listen_to_master: appelé par chaque thread pour écouter les messages ZMQ du master                                  */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: JSonNode * sir il y a un message, sinon NULL                                                                       */
/******************************************************************************************************************************/
 JsonNode *SubProcess_Listen_to_master_new ( struct SUBPROCESS *module )
  { return ( Recv_zmq_with_json( module->zmq_from_bus, Json_get_string ( module->config, "thread_tech_id" ),
             (gchar *)&module->zmq_buffer, sizeof(module->zmq_buffer) ) );
  }
/******************************************************************************************************************************/
/* Thread_send_comm_to_master: appelé par chaque thread pour envoyer le statut de la comm au master                           */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void SubProcess_send_comm_to_master_new ( struct SUBPROCESS *module, gboolean etat )
  { if (module->comm_status != etat || module->comm_next_update <= Partage->top)
     { Http_Post_to_local_BUS_WATCHDOG ( module, Json_get_string ( module->config, "thread_tech_id" ), "IO_COMM", (etat ? 900 : 0) );
       module->comm_next_update = Partage->top + 600;                                                      /* Toutes les minutes */
       module->comm_status = etat;
     }
  }
/******************************************************************************************************************************/
/* Http_ws_on_master_message: Appelé par libsoup lorsque l'on recoit un message sur la websocket connectée au master          */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void SubProcess_ws_on_master_message_CB ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { struct SUBPROCESS *module = user_data;
    Info_new( Config.log, Config.log_bus, LOG_INFO, "%s: WebSocket Message received !", __func__ );
    gsize taille;

    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: WebSocket Message Dropped (not JSON) !", __func__ );
       return;
     }

    if (!Json_has_member ( response, "bus_tag" ))
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: WebSocket Message Dropped (no 'bus_tag') !", __func__ );
       json_node_unref(response);
       return;
     }

    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: receive bus_tag '%s'  !", __func__, Json_get_string ( response, "bus_tag" ) );
    if (module->lib->Run_subprocess_message)                                             /* on passe le message au subprocess */
     { module->lib->Run_subprocess_message ( module, Json_get_string ( response, "bus_tag" ), response ); }
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Http_ws_on_master_closed: Traite une deconnexion du master                                                                 */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void SubProcess_ws_on_master_close_CB ( SoupWebsocketConnection *connexion, gpointer user_data )
  { struct SUBPROCESS *module = user_data;
    g_object_unref(module->Master_websocket);
  }
 static void SubProcess_ws_on_master_error_CB ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { struct SUBPROCESS *module = user_data;
    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: WebSocket Error received %p!", __func__, self );
  }
/******************************************************************************************************************************/
/* Traiter_connect_ws_CB: Termine la creation de la connexion websocket MSGS et raccorde le signal handler                    */
/* Entrée: les variables traditionnelles de libsous                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void SubProcess_ws_on_master_connected ( GObject *source_object, GAsyncResult *res, gpointer user_data )
  { struct SUBPROCESS *module = user_data;
    GError *error = NULL;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    module->Master_websocket = soup_session_websocket_connect_finish ( module->Master_session, res, &error );
    if (!module->Master_websocket)                                                                   /* No limit on incoming packet ! */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: UUID %s/%s: WebSocket error.", __func__,
                 module->lib->uuid, thread_tech_id );
       g_error_free (error);
       return;
     }
    /*g_object_set ( G_OBJECT(infos->ws_motifs), "max-incoming-payload-size", G_GINT64_CONSTANT(0), NULL );*/
    g_signal_connect ( module->Master_websocket, "message", G_CALLBACK(SubProcess_ws_on_master_message_CB), module );
    g_signal_connect ( module->Master_websocket, "closed",  G_CALLBACK(SubProcess_ws_on_master_close_CB), module );
    g_signal_connect ( module->Master_websocket, "error",   G_CALLBACK(SubProcess_ws_on_master_error_CB), module );
  }
/******************************************************************************************************************************/
/* Thread_init: appelé par chaque thread, lors de son démarrage                                                               */
/* Entrée: Le nom du thread, sa classe, la structure afférente, sa version, et sa description                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void SubProcess_init ( struct SUBPROCESS *module, gint sizeof_vars )
  { gchar chaine[128];

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    g_snprintf( chaine, sizeof(chaine), "W-%s-%s", module->lib->name, thread_tech_id );            /* Positionne le nom noyau */
    gchar *upper_name = g_ascii_strup ( chaine, -1 );
    prctl(PR_SET_NAME, upper_name, 0, 0, 0 );
    g_free(upper_name);

    if (sizeof_vars)
     { module->vars = g_try_malloc0 ( sizeof_vars );
       if (!module->vars)
        { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: UUID %s/%s: Memory error.", __func__,
                    module->lib->uuid, thread_tech_id );
          SubProcess_end ( module );                            /* Pas besoin de return : SubProcess_end fait un pthread_exit */
        }
     }

    module->zmq_from_bus  = Zmq_Connect ( ZMQ_SUB, "listen-to-bus", "inproc", ZMQUEUE_LOCAL_BUS, 0 );

    module->Master_session = soup_session_new();
    g_object_set ( G_OBJECT(module->Master_session), "ssl-strict", FALSE, NULL );
    static gchar *protocols[] = { "live-slaves", NULL };
    g_snprintf(chaine, sizeof(chaine), "wss://%s:5559/bus", Json_get_string ( Config.config, "master_hostname" ) );
    soup_session_websocket_connect_async ( module->Master_session, soup_message_new ( "GET", chaine ),
                                            NULL, protocols, g_cancellable_new(), SubProcess_ws_on_master_connected, module );

    gchar *description = "Add description to database table";
    if (Json_has_member ( module->config, "description" )) description = Json_get_string ( module->config, "description" );
    if (Dls_auto_create_plugin( thread_tech_id, description ) == FALSE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR (%s)\n", __func__, thread_tech_id, description ); }

    Mnemo_auto_create_WATCHDOG ( FALSE, thread_tech_id, "IO_COMM", "Statut de la communication" );
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: UUID %s/%s is UP",
              __func__, module->lib->uuid, thread_tech_id );
  }
/******************************************************************************************************************************/
/* Thread_init: appelé par chaque thread, lors de son démarrage                                                               */
/* Entrée: Le nom du thread, sa classe, la structure afférente, sa version, et sa description                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void SubProcess_end ( struct SUBPROCESS *module )
  { SubProcess_send_comm_to_master_new ( module, FALSE );
    if (module->vars) g_free(module->vars);
    soup_websocket_connection_close ( module->Master_websocket, 0, "Thanks, Bye !" );
    soup_session_abort ( module->Master_session );
    Zmq_Close ( module->zmq_from_bus );
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: UUID %s/%s is DOWN",
              __func__, module->lib->uuid, Json_get_string ( module->config, "thread_tech_id") );
    pthread_exit(0);
  }
/******************************************************************************************************************************/
/* Process_Load_one_subprocess: Demarre u nmodule du thread en parametre                                                      */
/* Entrée: la structure librairie du thread et la configuration du module, dans 'element' au format json                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Process_Load_one_subprocess (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct PROCESS *lib = user_data;
    pthread_attr_t attr;

    struct SUBPROCESS *module = g_try_malloc0( sizeof(struct SUBPROCESS) );
    if (!module)
     { Info_new( Config.log, lib->Thread_debug, LOG_ERR, "%s: UUID %s/%s: Not Enought Memory",
                 __func__, module->lib->uuid, Json_get_string ( element, "thread_tech_id" ) );
       return;
     }
    module->lib    = lib;
    module->config = element;

    if ( pthread_attr_init(&attr) )                                                 /* Initialisation des attributs du thread */
     { Info_new( Config.log, lib->Thread_debug, LOG_ERR, "%s: UUID %s/%s: pthread_attr_init failed",
                 __func__, module->lib->uuid, Json_get_string ( element, "thread_tech_id" ) );
       return;
     }

    if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) )                       /* On le laisse joinable au boot */
     { Info_new( Config.log, lib->Thread_debug, LOG_ERR, "%s: UUID %s/%s: pthread_setdetachstate failed",
                 __func__, module->lib->uuid, Json_get_string ( element, "thread_tech_id" ) );
       return;
     }

    if ( pthread_create( &module->TID, &attr, (void *)lib->Run_subprocess, module ) )
     { Info_new( Config.log, lib->Thread_debug, LOG_ERR, "%s: UUID %s/%s: pthread_create failed (%s)",
                 __func__, module->lib->uuid, Json_get_string ( element, "thread_tech_id" ) );
       return;
     }
    pthread_attr_destroy(&attr);                                                                        /* Libération mémoire */
    lib->modules = g_slist_append ( lib->modules, module );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: UUID %s/%s: loaded",
              __func__, module->lib->uuid, Json_get_string ( element, "thread_tech_id" ) );
  }
/******************************************************************************************************************************/
/* Process_Unload_one_subprocess: Demarre u nmodule du thread en parametre                                                    */
/* Entrée: la structure librairie du thread et la configuration du module, dans 'element' au format json                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Process_Unload_all_subprocess ( struct PROCESS *lib )
  { while(lib->modules)
     { struct SUBPROCESS *module = lib->modules->data;
       lib->modules = g_slist_remove ( lib->modules, module );
       Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: UUID %s/%s: Wait for sub-process end",
                 __func__, module->lib->uuid, Json_get_string ( module->config, "thread_tech_id" ) );
       pthread_join( module->TID, NULL );                                                              /* Attente fin du fils */
       Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: UUID %s/%s: unloaded",
                 __func__, module->lib->uuid, Json_get_string ( module->config, "thread_tech_id" ) );
       g_free(module);
     }
  }
/******************************************************************************************************************************/
/* Process_start: Demarre le thread en paremetre                                                                              */
/* Entrée: La structure associée au thread                                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Process_start ( struct PROCESS *lib )
  { pthread_attr_t attr;
    if (!lib) return(FALSE);
    if (lib->Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: UUID %s: Process %s already seems to be running",
                 __func__, lib->uuid, lib->nom_fichier );
       return(FALSE);
     }

    if ( pthread_attr_init(&attr) )                                                 /* Initialisation des attributs du thread */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: UUID %s: pthread_attr_init failed (%s)",
                 __func__, lib->uuid, lib->nom_fichier );
       return(FALSE);
     }

    if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) )                       /* On le laisse joinable au boot */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: UUID %s: pthread_setdetachstate failed (%s)",
                 __func__, lib->uuid, lib->nom_fichier );
       return(FALSE);
     }

    if ( pthread_create( &lib->TID, &attr, (void *)lib->Run_process, lib ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: UUID %s: pthread_create failed (%s)",
                 __func__, lib->uuid, lib->nom_fichier );
       return(FALSE);
     }
    pthread_attr_destroy(&attr);                                                                        /* Libération mémoire */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: UUID %s: Process %s started",
                 __func__, lib->uuid, lib->nom_fichier );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Process_stop: Arrete le thread en paremetre                                                                                */
/* Entrée: La structure associée au thread                                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Process_stop ( struct PROCESS *lib )
  { if (!lib) return(FALSE);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: UUID %s: Process %s, stopping in progress",
              __func__, lib->uuid, lib->nom_fichier );
    lib->Thread_run = FALSE;                                                             /* On demande au thread de s'arreter */
    if (!lib->TID)
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: UUID %s: Process %s not started, stopping aborted",
              __func__, lib->uuid, lib->nom_fichier );
       return(FALSE);
     }
    pthread_join( lib->TID, NULL );                                                                    /* Attente fin du fils */
    lib->TID = 0;
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: UUID %s: Process %s stopped",
              __func__, lib->uuid, lib->nom_fichier );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Process_dlopen: Ouverture d'un process via dlopen                                                                          */
/* Entrée: La structure PROCESS associée                                                                                      */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Process_dlopen ( struct PROCESS *lib )
  { pthread_mutexattr_t attr;                                                          /* Initialisation des mutex de synchro */
    lib->dl_handle = dlopen( lib->nom_fichier, RTLD_GLOBAL | RTLD_NOW );
    if (!lib->dl_handle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: UUID %s: Process %s failed (%s)",
                 __func__, lib->uuid, lib->nom_fichier, dlerror() );
       return(FALSE);
     }

    lib->Run_process = dlsym( lib->dl_handle, "Run_process" );                                      /* Recherche de la fonction */
    if (!lib->Run_process)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: UUID %s: Process %s rejected (Run_process not found)",
                 __func__, lib->uuid, lib->nom_fichier );
       dlclose( lib->dl_handle );
       return(FALSE);
     }

    lib->Run_subprocess = dlsym( lib->dl_handle, "Run_subprocess" );                              /* Recherche de la fonction */
    lib->Run_subprocess_message = dlsym( lib->dl_handle, "Run_subprocess_message" );/* Reception d'un message depuis le master */
    lib->Admin_config   = dlsym( lib->dl_handle, "Admin_config" );                                /* Recherche de la fonction */
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: UUID %s: %s loaded", __func__, lib->uuid, lib->nom_fichier );

    pthread_mutexattr_init( &attr );                                                          /* Creation du mutex de synchro */
    pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_SHARED );
    pthread_mutex_init( &lib->synchro, &attr );

    JsonNode *RootNode = Json_node_create();
    if(RootNode)
     { SQL_Select_to_json_node ( RootNode, NULL, "SELECT debug, enable, database_version FROM processes WHERE uuid='%s'", lib->uuid );
       lib->Thread_debug     = Json_get_bool ( RootNode, "debug" );
       lib->database_version = Json_get_int  ( RootNode, "database_version" );
       if ( !strcasecmp( lib->name, "http" ) || Json_get_bool ( RootNode, "enable" ) == TRUE ) { Process_start( lib ); }
       else { Info_new( Config.log, Config.log_msrv, LOG_INFO,
                       "%s: UUID %s: Process '%s' is not enabled : Loaded but not started", __func__, lib->uuid, lib->name );
            }
       json_node_unref(RootNode);
     }
    else { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: UUID %s: Process '%s': Memory Error", __func__, lib->uuid, lib->name ); }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Process_reload_by_uuid: Restart le process en parametre                                                                    */
/* Entrée: L'uuid du thread                                                                                                   */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Process_reload_by_uuid ( gchar *uuid )
  { gboolean found = FALSE;
    GSList *liste = Partage->com_msrv.Librairies;                                        /* Parcours de toutes les librairies */
    while(liste)
     { struct PROCESS *lib = liste->data;
       if ( ! strcasecmp( uuid, lib->uuid ) )
        { Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                   "%s: UUID %s: Reloading '%s' -> Library found. Reloading.", __func__, lib->uuid, lib->name );
          Process_stop(lib);
          if (lib->dl_handle) dlclose( lib->dl_handle );
          Process_dlopen ( lib );
          found = TRUE;
        }
       liste = g_slist_next(liste);
     }
    return(found);
  }
/******************************************************************************************************************************/
/* Process_reload_by_uuid: Restart le process en paremetre                                                                    */
/* Entrée: Le nom du thread                                                                                                   */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Process_set_debug ( gchar *uuid, gboolean debug )
  { gboolean found = FALSE;
    GSList *liste = Partage->com_msrv.Librairies;                                        /* Parcours de toutes les librairies */
    while(liste)
     { struct PROCESS *lib = liste->data;
       if ( ! strcasecmp( uuid, lib->uuid ) )
        { Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                   "%s: UUID %s: Setting '%s' debug %s.", __func__, lib->uuid, lib->name, (debug ? "ON" : "OFF") );
          lib->Thread_debug = debug;
          found = TRUE;
        }
       liste = g_slist_next(liste);
     }
    return(found);
  }
/******************************************************************************************************************************/
/* Decharger_librairies: Decharge toutes les librairies                                                                       */
/* EntrÃée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Decharger_librairies ( void )
  { struct PROCESS *lib;
    GSList *liste;

    liste = Partage->com_msrv.Librairies;                 /* Envoie une commande d'arret pour toutes les librairies d'un coup */
    while(liste)
     { lib = (struct PROCESS *)liste->data;
       lib->Thread_run = FALSE;                                                          /* On demande au thread de s'arreter */
       liste = liste->next;
     }

    liste = Partage->com_msrv.Librairies;
    while(liste)
     { lib = (struct PROCESS *)liste->data;
       if (lib->TID) pthread_join( lib->TID, NULL );                                                   /* Attente fin du fils */
       liste = liste->next;
     }

    while(Partage->com_msrv.Librairies)                                                     /* Liberation mémoire des modules */
     { lib = (struct PROCESS *)Partage->com_msrv.Librairies->data;
       pthread_mutex_destroy( &lib->synchro );
       if (lib->dl_handle) dlclose( lib->dl_handle );
       Partage->com_msrv.Librairies = g_slist_remove( Partage->com_msrv.Librairies, lib );
                                                                             /* Destruction de l'entete associé dans la GList */
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: UUID %s: process %s unloaded", __func__, lib->uuid, lib->nom_fichier );
       g_free( lib );
     }
  }
/******************************************************************************************************************************/
/* Charger_librairies: Ouverture de toutes les librairies possibles pour Watchdog                                             */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_librairies ( void )
  { struct dirent *fichier;
    DIR *repertoire;

    repertoire = opendir ( Config.librairie_dir );                                  /* Ouverture du répertoire des librairies */
    if (!repertoire)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Directory %s Unknown", __func__, Config.librairie_dir );
       return;
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Loading Directory %s in progress", __func__, Config.librairie_dir );

    while( (fichier = readdir( repertoire )) )                                      /* Pour chacun des fichiers du répertoire */
     { if (    ! strncmp( fichier->d_name, "libwatchdog-server-", 19 )                      /* Chargement unitaire d'une librairie */
           &&  ! strncmp( fichier->d_name + strlen(fichier->d_name) - 3, ".so", 4 ) )
        { struct PROCESS *lib = g_try_malloc0( sizeof ( struct PROCESS ) );
          if (!lib) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: MemoryAlloc failed", __func__ );
                      continue;
                    }

          g_snprintf( lib->name, strlen(fichier->d_name)-21, "%s", fichier->d_name + 19 );
          g_snprintf( lib->nom_fichier,  sizeof(lib->nom_fichier), "%s/libwatchdog-server-%s.so", Config.librairie_dir, lib->name );

          UUID_Load ( lib->name, lib->uuid );                                              /* Récupère l'UUID sur le FS local */

          SQL_Write_new ( "INSERT INTO processes SET instance='%s', uuid='%s', name='%s', database_version=0, enable=0, debug=0 "
                          "ON DUPLICATE KEY UPDATE instance=VALUES(instance)", g_get_host_name(), lib->uuid, lib->name );

          Process_dlopen( lib );
          Partage->com_msrv.Librairies = g_slist_prepend( Partage->com_msrv.Librairies, lib );
        }
     }
    closedir( repertoire );                                                 /* Fermeture du rÃ©pertoire a la fin du traitement */

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: %d Process loaded", __func__, g_slist_length( Partage->com_msrv.Librairies ) );
  }
/******************************************************************************************************************************/
/* Demarrer_dls: Thread un process DLS                                                                                        */
/* EntrÃ©e: rien                                                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_dls ( void )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Demande de demarrage %d", __func__, getpid() );
    if (Partage->com_dls.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: An instance is already running %d",__func__, Partage->com_dls.TID );
       return(FALSE);
     }
    memset( &Partage->com_dls, 0, sizeof(Partage->com_dls) );                       /* Initialisation des variables du thread */
    if ( pthread_create( &Partage->com_dls.TID, NULL, (void *)Run_dls, NULL ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: pthread_create failed", __func__ );
       return(FALSE);
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: thread dls (%p) seems to be running", __func__, Partage->com_dls.TID );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Demarrer_http: Thread un process HTTP                                                                                      */
/* EntrÃ©e: rien                                                                                                              */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_http ( void )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Demande de demarrage %d", __func__, getpid() );
    if (Partage->com_http.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: An instance is already running %d",__func__, Partage->com_http.TID );
       return(FALSE);
     }
    memset( &Partage->com_http, 0, sizeof(Partage->com_http) );                     /* Initialisation des variables du thread */
    if ( pthread_create( &Partage->com_http.TID, NULL, (void *)Run_HTTP, NULL ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: pthread_create failed", __func__ );
       return(FALSE);
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: thread http (%p) seems to be running", __func__, Partage->com_http.TID );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Demarrer_arch: Thread un process arch                                                                                      */
/* EntrÃée: rien                                                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_arch ( void )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Demande de demarrage %d", __func__, getpid() );
    if (Partage->com_arch.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: An instance is already running", __func__, Partage->com_arch.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_arch.TID, NULL, (void *)Run_arch, NULL ))
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: pthread_create failed", __func__ );
       return(FALSE);
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: thread arch (%p) seems to be running", __func__, Partage->com_arch.TID );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Stopper_fils: arret de tous les fils Watchdog                                                                              */
/* Entré/Sortie: néant                                                                                                        */
/******************************************************************************************************************************/
 void Stopper_fils ( void )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Debut stopper_fils", __func__ );

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Waiting for DLS (%p) to finish", __func__, Partage->com_dls.TID );
    Partage->com_dls.Thread_run = FALSE;
    while ( Partage->com_dls.TID != 0 ) sched_yield();                                                     /* Attente fin DLS */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: ok, DLS is down", __func__ );

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Waiting for ARCH (%p) to finish", __func__, Partage->com_arch.TID );
    Partage->com_arch.Thread_run = FALSE;
    while ( Partage->com_arch.TID != 0 ) sched_yield();                                                    /* Attente fin DLS */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: ok, ARCH is down", __func__ );

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Waiting for HTTP (%p) to finish", __func__, Partage->com_http.TID );
    Partage->com_http.Thread_run = FALSE;
    while ( Partage->com_http.TID != 0 ) sched_yield();                                                    /* Attente fin DLS */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: ok, HTTP is down", __func__ );

    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Fin stopper_fils", __func__ );
    sleep(1);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
