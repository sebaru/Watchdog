/******************************************************************************************************************************/
/* Watchdogd/thread.c        Gestion des Threads                                                                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 11 avr 2009 12:21:45 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * thread.c
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

 #define _GNU_SOURCE
 #include <sys/resource.h>
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
/* Thread_send_comm_to_master: Envoi le statut de la comm au master                                                       */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Thread_send_comm_to_master ( struct THREAD *module, gboolean etat )
  { if (module->comm_status != etat || module->comm_next_update <= Partage->top)
     { Http_Post_to_local_BUS_WATCHDOG ( module, "IO_COMM", (etat ? 900 : 0) );
       module->comm_next_update = Partage->top + 600;                                                   /* Toutes les minutes */
       module->comm_status = etat;
     }
  }
/******************************************************************************************************************************/
/* Thread_loop: S'occupe de la telemetrie, de la comm périodique, de la vitesse de rotation                                   */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Thread_loop ( struct THREAD *module )
  { Thread_send_comm_to_master ( module, module->comm_status );

/********************************************************* tour par secondes **************************************************/
    if (Partage->top >= module->nbr_tour_top+10)                                                     /* Toutes les 1 secondes */
     { module->nbr_tour_par_sec = module->nbr_tour;
       module->nbr_tour = 0;
       if(module->nbr_tour_par_sec > 50) module->nbr_tour_delai += 50;
       else if(module->nbr_tour_delai>0) module->nbr_tour_delai -= 50;
       module->nbr_tour_top = Partage->top;
     } else module->nbr_tour++;
    usleep(module->nbr_tour_delai);

/********************************************************* Toutes les minutes *************************************************/
    if (Partage->top >= module->telemetrie_top+600)                                                     /* Toutes les minutes */
     { struct rusage conso;
       if (getrusage ( RUSAGE_THREAD, &conso ) == 0)
        { Http_Post_to_local_BUS_AI ( module, module->maxrss, conso.ru_maxrss, TRUE ); }
       module->telemetrie_top = Partage->top;
     }
  }
/******************************************************************************************************************************/
/* Thread_ws_on_master_message_CB: Appelé par libsoup lorsque l'on recoit un message sur la websocket connectée au master     */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Thread_ws_on_master_message_CB ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { struct THREAD *module = user_data;
    Info_new( Config.log, Config.log_bus, LOG_INFO, "%s: WebSocket Message received !", __func__ );
    gsize taille;

    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response)
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING, "%s: WebSocket Message Dropped (not JSON) !", __func__ );
       return;
     }

    if (!Json_has_member ( response, "bus_tag" ))
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING, "%s: WebSocket Message Dropped (no 'bus_tag') !", __func__ );
       Json_node_unref(response);
       return;
     }

    gchar *bus_tag = Json_get_string ( response, "bus_tag" );
    Info_new( Config.log, module->Thread_debug, LOG_INFO, "%s: receive bus_tag '%s'  !", __func__, bus_tag );

    pthread_mutex_lock ( &module->synchro );                                             /* on passe le message au thread */
    module->Master_messages = g_slist_append ( module->Master_messages, response );
    pthread_mutex_unlock ( &module->synchro );
  }
/******************************************************************************************************************************/
/* Http_ws_on_master_closed: Traite une deconnexion du master                                                                 */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Thread_ws_on_master_close_CB ( SoupWebsocketConnection *connexion, gpointer user_data )
  { struct THREAD *module = user_data;
    Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: WebSocket Close. Reboot Needed!", __func__ );
    /* Partage->com_msrv.Thread_run = FALSE; */
  }
 static void Thread_ws_on_master_error_CB ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { struct THREAD *module = user_data;
    Info_new( Config.log, module->Thread_debug, LOG_INFO, "%s: WebSocket Error received %p!", __func__, self );
  }
/******************************************************************************************************************************/
/* Traiter_connect_ws_CB: Termine la creation de la connexion websocket MSGS et raccorde le signal handler                    */
/* Entrée: les variables traditionnelles de libsous                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Thread_ws_on_master_connected ( GObject *source_object, GAsyncResult *res, gpointer user_data )
  { struct THREAD *module = user_data;
    GError *error = NULL;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    module->Master_websocket = soup_session_websocket_connect_finish ( module->Master_session, res, &error );
    if (!module->Master_websocket)                                                                   /* No limit on incoming packet ! */
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': WebSocket error: %s.", __func__, thread_tech_id, error->message );
       g_error_free (error);
       return;
     }
    /*g_object_set ( G_OBJECT(infos->ws_motifs), "max-incoming-payload-size", G_GINT64_CONSTANT(0), NULL );*/
    g_object_set ( G_OBJECT(module->Master_websocket), "keepalive-interval", G_GINT64_CONSTANT(30), NULL );
    g_signal_connect ( module->Master_websocket, "message", G_CALLBACK(Thread_ws_on_master_message_CB), module );
    g_signal_connect ( module->Master_websocket, "closed",  G_CALLBACK(Thread_ws_on_master_close_CB), module );
    g_signal_connect ( module->Master_websocket, "error",   G_CALLBACK(Thread_ws_on_master_error_CB), module );
    Info_new( Config.log, module->Thread_debug, LOG_INFO, "%s: '%s': WebSocket to Master connected", __func__, thread_tech_id );
  }
/******************************************************************************************************************************/
/* Thread_init: appelé par chaque thread, lors de son démarrage                                                               */
/* Entrée: Le nom du thread, sa classe, la structure afférente, sa version, et sa description                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Thread_init ( struct THREAD *module, gint sizeof_vars )
  { gchar chaine[128];
    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    g_snprintf( chaine, sizeof(chaine), "W-%s", thread_tech_id );                                  /* Positionne le nom noyau */
    gchar *upper_name = g_ascii_strup ( chaine, -1 );
    prctl(PR_SET_NAME, upper_name, 0, 0, 0 );
    g_free(upper_name);

    if (sizeof_vars)
     { module->vars = g_try_malloc0 ( sizeof_vars );
       if (!module->vars)
        { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': Memory error for vars.", __func__, thread_tech_id );
          Thread_end ( module );                            /* Pas besoin de return : Thread_end fait un pthread_exit */
        }
     }

/******************************************************* Ecoute du Master *****************************************************/
    module->Master_session = soup_session_new();
    g_object_set ( G_OBJECT(module->Master_session), "ssl-strict", FALSE, NULL );
    static gchar *protocols[] = { "live-bus", NULL };
    g_snprintf(chaine, sizeof(chaine), "wss://%s:5559/ws_bus", Config.master_hostname );
    SoupMessage *query   = soup_message_new ( "GET", chaine );
    GCancellable *cancel = g_cancellable_new();
    soup_session_websocket_connect_async ( module->Master_session, query,
                                           NULL, protocols, cancel, Thread_ws_on_master_connected, module );
    g_object_unref(query);
    g_object_unref(cancel);

    gchar *description = "Add description to database table";
    if (Json_has_member ( module->config, "description" )) description = Json_get_string ( module->config, "description" );
    if (Dls_auto_create_plugin( thread_tech_id, description ) == FALSE)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR (%s)\n", __func__, thread_tech_id, description ); }

    module->maxrss = Mnemo_create_thread_AI ( module, "MAXRSS", "Memory Usage", "kb", ARCHIVE_1_MIN );
    Mnemo_auto_create_WATCHDOG ( FALSE, thread_tech_id, "IO_COMM", "Statut de la communication" );
    Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: Thread '%s' is UP", __func__, thread_tech_id );
  }
/******************************************************************************************************************************/
/* Thread_init: appelé par chaque thread, lors de son démarrage                                                               */
/* Entrée: Le nom du thread, sa classe, la structure afférente, sa version, et sa description                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Thread_end ( struct THREAD *module )
  { Thread_send_comm_to_master ( module, FALSE );
    if (module->vars) g_free(module->vars);
    Json_node_unref ( module->maxrss );
    if (module->Master_websocket && soup_websocket_connection_get_state (module->Master_websocket) == SOUP_WEBSOCKET_STATE_OPEN)
     { soup_websocket_connection_close ( module->Master_websocket, 0, "Thanks, Bye !" );
       while (soup_websocket_connection_get_state (module->Master_websocket) != SOUP_WEBSOCKET_STATE_CLOSED) sched_yield();
     }
    module->Master_websocket = NULL;
    soup_session_abort ( module->Master_session );
    g_slist_foreach ( module->Master_messages, (GFunc) Json_node_unref, NULL );
    g_slist_free ( module->Master_messages );
    Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: '%s' is DOWN",
              __func__, Json_get_string ( module->config, "thread_tech_id") );
    pthread_exit(0);
  }
/******************************************************************************************************************************/
/* Decharger_librairies: Decharge toutes les librairies                                                                       */
/* EntrÃée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Decharger_librairies ( void )
  { GSList *liste;

    pthread_mutex_lock ( &Partage->com_msrv.synchro );

    liste = Partage->com_msrv.Threads;                 /* Envoie une commande d'arret pour toutes les librairies d'un coup */
    while(liste)
     { struct THREAD *module = liste->data;
       module->Thread_run = FALSE;                                                       /* On demande au thread de s'arreter */
       liste = liste->next;
     }

    liste = Partage->com_msrv.Threads;                 /* Envoie une commande d'arret pour toutes les librairies d'un coup */
    while(liste)
     { struct THREAD *module = liste->data;
       if (module->TID) pthread_join( module->TID, NULL );                                             /* Attente fin du fils */
       liste = liste->next;
     }

    while(Partage->com_msrv.Threads)                                                     /* Liberation mémoire des modules */
     { struct THREAD *module = Partage->com_msrv.Threads->data;
       if (module->dl_handle) dlclose( module->dl_handle );
       pthread_mutex_destroy( &module->synchro );
       Partage->com_msrv.Threads = g_slist_remove( Partage->com_msrv.Threads, module );
                                                                             /* Destruction de l'entete associé dans la GList */
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: '%s': thread unloaded", __func__, Json_get_string ( module->config, "thread_tech_id" ) );
       Json_node_unref ( module->config );
       g_free( module );
     }

    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* Thread_Delete_one_thread: Decharge un seul et unique thread                                                                */
/* Entrée: Le tech_id du thread                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Thread_Delete_one_thread ( JsonNode *element )
  { if (!element)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: element not provided", __func__ ); return; }

    if (!Json_has_member ( element, "thread_tech_id" ))
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: no 'thread_tech_id' in Json", __func__ ); return; }
    gchar *thread_tech_id = Json_get_string ( element, "thread_tech_id" );

    struct THREAD *module = NULL;
    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    GSList *liste = Partage->com_msrv.Threads;            /* Envoie une commande d'arret pour toutes les librairies d'un coup */
    while(liste)
     { struct THREAD *search_module = liste->data;
       if (!strcasecmp ( thread_tech_id, Json_get_string ( search_module->config, "thread_tech_id" ) ) )
        { module = search_module;                                                        /* On demande au thread de s'arreter */
          Partage->com_msrv.Threads = g_slist_remove( Partage->com_msrv.Threads, module );
          break;
        }
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );

    if (!module)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: '%s': thread not found", __func__, thread_tech_id ); return; }

    module->Thread_run = FALSE;
    if (module->TID) pthread_join( module->TID, NULL );                                                /* Attente fin du fils */

    if (module->dl_handle) dlclose( module->dl_handle );
    pthread_mutex_destroy( &module->synchro );
                                                                             /* Destruction de l'entete associé dans la GList */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: '%s': thread unloaded", __func__, Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_unref ( module->config );
    g_free( module );
  }
/******************************************************************************************************************************/
/* Thread_Create_one_thread: Création d'un sous thread                                                                       */
/* Entrée: La structure JSON de issue de la requete Global API de Load Thread                                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Thread_Create_one_thread (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { if (!element)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: element not provided", __func__ ); return; }

    if (!Json_has_member ( element, "thread_tech_id" ))
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: no 'thread_tech_id' in Json", __func__ ); return; }
    gchar *thread_tech_id = Json_get_string ( element, "thread_tech_id" );

    if (!Json_has_member ( element, "thread_classe" ))
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: no 'thread_classe' in Json", __func__ ); return; }
    gchar *thread_classe  = Json_get_string ( element, "thread_classe" );

    struct THREAD *module = g_try_malloc0( sizeof(struct THREAD) );
    if (!module)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: '%s': Not Enought Memory", __func__, thread_tech_id );
       return;
     }

    gchar nom_fichier[128];
    g_snprintf( nom_fichier,  sizeof(nom_fichier), "%s/libwatchdog-server-%s.so", Config.librairie_dir, thread_classe );

    module->dl_handle = dlopen( nom_fichier, RTLD_GLOBAL | RTLD_NOW );
    if (!module->dl_handle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: '%s': Process '%s' loading failed (%s)",
                 __func__, thread_tech_id, nom_fichier, dlerror() );
       g_free(module);
       return;
     }

    module->Run_thread = dlsym( module->dl_handle, "Run_thread" );                        /* Recherche de la fonction */
    if (!module->Run_thread)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: '%s': Process %s rejected (Run_thread not found)",
                 __func__, thread_tech_id, nom_fichier );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }

    JsonNode *RootNode = Json_node_create();
    Json_node_add_string ( RootNode, "thread_tech_id", thread_tech_id );
    Json_node_add_string ( RootNode, "thread_classe",  thread_classe );
    if(!RootNode)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: '%s': Process: Memory Error. Unloading.", __func__, thread_tech_id );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }
    module->config = Http_Post_to_global_API ( "/run/thread", "GET_CONFIG", RootNode );
    Json_node_unref(RootNode);
    if (module->config && Json_get_int ( module->config, "api_status" ) == SOUP_STATUS_OK)
     { module->Thread_debug = Json_get_bool ( module->config, "debug" );
       module->Thread_run   = Json_get_bool ( module->config, "enable" );
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: '%s': GET_CONFIG from API Failed. Unloading.", __func__, thread_tech_id );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }

    pthread_attr_t attr;                                                       /* Attribut de mutex pour parametrer le module */
    if ( pthread_attr_init(&attr) )                                                 /* Initialisation des attributs du thread */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: '%s': pthread_attr_init failed. Unloading.", __func__, thread_tech_id );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }

    if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) )                       /* On le laisse joinable au boot */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: '%s': pthread_setdetachstate failed. Unloading.", __func__, thread_tech_id );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }

    pthread_mutexattr_t param;                                                                /* Creation du mutex de synchro */
    pthread_mutexattr_init( &param );                                                         /* Creation du mutex de synchro */
    pthread_mutexattr_setpshared( &param, PTHREAD_PROCESS_SHARED );
    pthread_mutex_init( &module->synchro, &param );

    if ( module->Thread_run && pthread_create( &module->TID, &attr, (void *)module->Run_thread, module ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: '%s': pthread_create failed. Unloading.", __func__, thread_tech_id );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }
    pthread_attr_destroy(&attr);                                                                        /* Libération mémoire */
    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Partage->com_msrv.Threads = g_slist_append ( Partage->com_msrv.Threads, module );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: '%s': thread of class '%s' loaded with enable=%d",
              __func__, thread_tech_id, thread_classe, module->Thread_run );
  }
/******************************************************************************************************************************/
/* Charger_librairies: Ouverture de toutes les librairies possibles pour Watchdog                                             */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_librairies ( void )
  { JsonNode *api_result = Http_Post_to_global_API ( "/run/thread", "LOAD", NULL );
    if (!api_result) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: API Error for /run/thread LOAD",__func__ ); return; }

    if (Json_get_int ( api_result, "api_status" ) == SOUP_STATUS_OK)                                /* Chargement des modules */
     { JsonArray *array = Json_get_array ( api_result, "threads" );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Loading %d thread",__func__, json_array_get_length(array) );
       Json_node_foreach_array_element ( api_result, "threads", Thread_Create_one_thread, NULL );
     }
    Json_node_unref(api_result);
  }
/******************************************************************************************************************************/
/* Demarrer_dls: Processus D.L.S                                                                                              */
/* EntrÃ©e: rien                                                                                                              */
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
/* Demarrer_http: Processus HTTP                                                                                              */
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
/* Demarrer_arch: Processus d'archivage                                                                                       */
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
