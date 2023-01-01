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
       JsonNode *RootNode = Json_node_create();
       if (RootNode)
        { Json_node_add_string ( RootNode, "thread_classe",  Json_get_string ( module->config, "thread_classe"  ) );
          Json_node_add_string ( RootNode, "thread_tech_id", Json_get_string ( module->config, "thread_tech_id" ) );
          Json_node_add_bool   ( RootNode, "io_comm", module->comm_status );
          JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/heartbeat", RootNode );
          Json_node_unref ( api_result );
          Json_node_unref ( RootNode );
        }
       module->comm_next_update = Partage->top + 600;                                                   /* Toutes les minutes */
       module->comm_status = etat;
     }
  }
/******************************************************************************************************************************/
/* Thread_ws_on_master_message_CB: Appelé par libsoup lorsque l'on recoit un message sur la websocket connectée au master     */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Thread_ws_on_master_message_CB ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { struct THREAD *module = user_data;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Info_new( __func__, Config.log_bus, LOG_DEBUG, "'%s': WebSocket Message received !", thread_tech_id );
    gsize taille;

    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response)
     { Info_new( __func__, Config.log_bus, LOG_WARNING, "'%s': WebSocket Message Dropped (not JSON) !", thread_tech_id );
       return;
     }

    if (!Json_has_member ( response, "tag" ))
     { Info_new( __func__, Config.log_bus, LOG_WARNING, "'%s': WebSocket Message Dropped (no 'tag') !", thread_tech_id );
       Json_node_unref(response);
       return;
     }

    gchar *tag = Json_get_string ( response, "tag" );
    Info_new( __func__, Config.log_bus, LOG_DEBUG, "'%s': receive tag '%s'  !", thread_tech_id, tag );

    pthread_mutex_lock ( &module->synchro );                                             /* on passe le message au thread */
    module->WS_messages = g_slist_append ( module->WS_messages, response );
    pthread_mutex_unlock ( &module->synchro );
  }
/******************************************************************************************************************************/
/* Http_ws_on_master_closed: Traite une deconnexion du master                                                                 */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Thread_ws_on_master_close_CB ( SoupWebsocketConnection *connexion, gpointer user_data )
  { struct THREAD *module = user_data;
    Info_new( __func__, module->Thread_debug, LOG_ERR, "WebSocket Close. Reboot Needed!" );
    module->Master_websocket = NULL;
  }
 static void Thread_ws_on_master_error_CB ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { struct THREAD *module = user_data;
    Info_new( __func__, module->Thread_debug, LOG_INFO, "WebSocket Error received %p!", self );
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
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': WebSocket error: %s.", thread_tech_id, error->message );
       g_error_free (error);
       return;
     }
    /*g_object_set ( G_OBJECT(infos->ws_motifs), "max-incoming-payload-size", G_GINT64_CONSTANT(0), NULL );*/
    g_object_set ( G_OBJECT(module->Master_websocket), "keepalive-interval", G_GINT64_CONSTANT(30), NULL );
    g_signal_connect ( module->Master_websocket, "message", G_CALLBACK(Thread_ws_on_master_message_CB), module );
    g_signal_connect ( module->Master_websocket, "closed",  G_CALLBACK(Thread_ws_on_master_close_CB), module );
    g_signal_connect ( module->Master_websocket, "error",   G_CALLBACK(Thread_ws_on_master_error_CB), module );
    Info_new( __func__, module->Thread_debug, LOG_INFO, "'%s': WebSocket to Master connected", thread_tech_id );
  }
/******************************************************************************************************************************/
/* Thread_ws_bus_init: appelé par chaque thread pour demarrer le websocket vers le master                                     */
/* Entrée: La thread                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Thread_ws_bus_init ( struct THREAD *module )
  { gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    if (module->Master_websocket )
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': WebSocket error: Already UP.", thread_tech_id );
       return;
     }

    module->Master_session = soup_session_new();
    g_object_set ( G_OBJECT(module->Master_session), "ssl-strict", FALSE, NULL );
    static gchar *protocols[] = { "live-bus", NULL };
    gchar chaine[256];
    g_snprintf(chaine, sizeof(chaine), "wss://%s:5559/ws_bus", Config.master_hostname );
    SoupMessage *query   = soup_message_new ( "GET", chaine );
    GCancellable *cancel = g_cancellable_new();
    soup_session_websocket_connect_async ( module->Master_session, query,
                                           NULL, protocols, cancel, Thread_ws_on_master_connected, module );
    g_object_unref(query);
    g_object_unref(cancel);
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
     { Http_Post_to_local_BUS_AI ( module, module->ai_nbr_tour_par_sec, module->nbr_tour_par_sec, TRUE );
       if (!module->Master_websocket) Thread_ws_bus_init ( module );               /* si perte de la websocket, on reconnecte */
       module->telemetrie_top = Partage->top;
     }
  }
/******************************************************************************************************************************/
/* Thread_every_hour: Renvoie TRUE une fois par heure                                                                         */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: TRUE une fois par heure                                                                                            */
/******************************************************************************************************************************/
 gboolean Thread_every_hour ( struct THREAD *module )
  { if (Partage->top >= module->hour_top + 36000)                                                    /* Toutes les 1 secondes */
     { module->hour_top = Partage->top;
       return(TRUE);
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Thread_init: appelé par chaque thread, lors de son démarrage                                                               */
/* Entrée: La structure afférente                                                                                             */
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
        { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': Memory error for vars.", thread_tech_id );
          Thread_end ( module );                            /* Pas besoin de return : Thread_end fait un pthread_exit */
        }
     }

/******************************************************* Ecoute du Master *****************************************************/
    Thread_ws_bus_init( module );

    gchar *description = "Add description to database table";
    if (Json_has_member ( module->config, "description" )) description = Json_get_string ( module->config, "description" );
    if (Dls_auto_create_plugin( thread_tech_id, description ) == FALSE)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: DLS Create ERROR (%s)\n", thread_tech_id, description ); }

    module->IOs = Json_node_create();
    Json_node_add_array ( module->IOs, "IOs" );

    module->ai_nbr_tour_par_sec = Mnemo_create_thread_AI ( module, "THREAD_TOUR_PAR_SEC", "Nombre de tour par seconde", "t/s", ARCHIVE_1_MIN );
    Mnemo_create_thread_WATCHDOG ( module, "IO_COMM", "Statut de la communication" );
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Thread '%s' is UP", thread_tech_id );
  }
/******************************************************************************************************************************/
/* Thread_end: appelé par chaque thread, lors de son arret                                                                    */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Thread_end ( struct THREAD *module )
  { Thread_send_comm_to_master ( module, FALSE );
    if (module->vars) g_free(module->vars);
    Json_node_unref ( module->IOs );
    if (module->Master_websocket && soup_websocket_connection_get_state (module->Master_websocket) == SOUP_WEBSOCKET_STATE_OPEN)
     { soup_websocket_connection_close ( module->Master_websocket, 0, "Thanks, Bye !" );
       while ( module->Master_websocket ) sched_yield();
     }
    soup_session_abort ( module->Master_session );
    g_slist_foreach ( module->WS_messages, (GFunc) Json_node_unref, NULL );
    g_slist_free ( module->WS_messages );
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: '%s' is DOWN",
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
       Info_new( __func__, Config.log_msrv, LOG_NOTICE, "'%s': thread unloaded", Json_get_string ( module->config, "thread_tech_id" ) );
       Json_node_unref ( module->config );
       g_free( module );
     }

    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* Thread_Push_API_message: Recoit une commande depuis l'API, au travers du master                                            */
/* Entrée: L'element json decrivant la requete                                                                                */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Thread_Push_API_message ( JsonNode *request )
  { if (!request)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "request not provided" ); return; }

    if (!Json_has_member ( request, "thread_tech_id" ))
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "no 'thread_tech_id' in Json" ); return; }
    gchar *thread_tech_id = Json_get_string ( request, "thread_tech_id" );

    struct THREAD *module = NULL;
    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    GSList *liste = Partage->com_msrv.Threads;            /* Envoie une commande d'arret pour toutes les librairies d'un coup */
    while(liste)
     { struct THREAD *search_module = liste->data;
       if (!strcasecmp ( thread_tech_id, Json_get_string ( search_module->config, "thread_tech_id" ) ) )
        { module = search_module;                                                        /* On demande au thread de s'arreter */
          break;
        }
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );

    if (!module)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': thread not found", thread_tech_id ); return; }

    Info_new( __func__, Config.log_msrv, LOG_INFO, "'%s': '%s' sent", thread_tech_id, Json_get_string ( request, "tag" ) );

    pthread_mutex_lock ( &module->synchro );                                                 /* on passe le message au thread */
    json_node_ref ( request );
    module->WS_messages = g_slist_append ( module->WS_messages, request );
    pthread_mutex_unlock ( &module->synchro );
  }
/******************************************************************************************************************************/
/* Thread_Stop_one_thread: Decharge un seul et unique thread                                                                  */
/* Entrée: Le tech_id du thread                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Thread_Stop_one_thread ( JsonNode *element )
  { if (!element)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "element not provided" ); return; }

    if (!Json_has_member ( element, "thread_tech_id" ))
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "no 'thread_tech_id' in Json" ); return; }
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
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': thread not found", thread_tech_id ); return; }

    module->Thread_run = FALSE;
    Info_new( __func__, Config.log_msrv, LOG_INFO, "'%s': Stopping", Json_get_string ( module->config, "thread_tech_id" ) );
    if (module->TID) pthread_join( module->TID, NULL );                                                /* Attente fin du fils */
    Info_new( __func__, Config.log_msrv, LOG_INFO, "'%s': Stopped", Json_get_string ( module->config, "thread_tech_id" ) );

    if (module->dl_handle) dlclose( module->dl_handle );
    pthread_mutex_destroy( &module->synchro );
                                                                             /* Destruction de l'entete associé dans la GList */
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "'%s': Unloaded", Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_unref ( module->config );
    g_free( module );
  }
/******************************************************************************************************************************/
/* Thread_Start_one_thread: Création d'un sous thread                                                                         */
/* Entrée: La structure JSON de issue de la requete Global API de Load Thread                                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Thread_Start_one_thread (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { if (!element)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "element not provided" ); return; }

    if (!Json_has_member ( element, "thread_tech_id" ))
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "no 'thread_tech_id' in Json" ); return; }
    gchar *thread_tech_id = Json_get_string ( element, "thread_tech_id" );

    if (!Json_has_member ( element, "thread_classe" ))
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "no 'thread_classe' for starting '%s'", thread_tech_id ); return; }
    gchar *thread_classe  = Json_get_string ( element, "thread_classe" );

    if (!Json_has_member ( element, "agent_uuid" ))
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "no 'agent_uuid' for starting '%s'", thread_tech_id ); return; }
    gchar *agent_uuid  = Json_get_string ( element, "agent_uuid" );

    if (strcmp ( agent_uuid, Json_get_string ( Config.config, "agent_uuid" ) ))
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'agent_uuid' is not our agent_uuid. Dropping." ); return; }

    struct THREAD *module = g_try_malloc0( sizeof(struct THREAD) );
    if (!module)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': Not Enought Memory", thread_tech_id );
       return;
     }

    gchar nom_fichier[128];
    g_snprintf( nom_fichier,  sizeof(nom_fichier), "%s/libwatchdog-server-%s.so", Config.librairie_dir, thread_classe );

    module->dl_handle = dlopen( nom_fichier, RTLD_GLOBAL | RTLD_NOW );
    if (!module->dl_handle)
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "%s: '%s': Thread '%s' loading failed (%s)",
                 __func__, thread_tech_id, nom_fichier, dlerror() );
       g_free(module);
       return;
     }

    module->Run_thread = dlsym( module->dl_handle, "Run_thread" );                        /* Recherche de la fonction */
    if (!module->Run_thread)
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "%s: '%s': Thread '%s' rejected (Run_thread not found)",
                 __func__, thread_tech_id, nom_fichier );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }

    module->config = Http_Get_from_global_API ( "/run/thread/config", "thread_tech_id=%s", thread_tech_id );
    if (module->config && Json_get_int ( module->config, "api_status" ) == SOUP_STATUS_OK)
     { module->Thread_debug = Json_get_bool ( module->config, "debug" );
       module->Thread_run   = Json_get_bool ( module->config, "enable" );
     }
    else
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': GET_CONFIG from API Failed. Unloading.", thread_tech_id );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }

    pthread_attr_t attr;                                                       /* Attribut de mutex pour parametrer le module */
    if ( pthread_attr_init(&attr) )                                                 /* Initialisation des attributs du thread */
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': pthread_attr_init failed. Unloading.", thread_tech_id );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }

    if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) )                       /* On le laisse joinable au boot */
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': pthread_setdetachstate failed. Unloading.", thread_tech_id );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }

    pthread_mutexattr_t param;                                                                /* Creation du mutex de synchro */
    pthread_mutexattr_init( &param );                                                         /* Creation du mutex de synchro */
    pthread_mutexattr_setpshared( &param, PTHREAD_PROCESS_SHARED );
    pthread_mutex_init( &module->synchro, &param );

    if ( module->Thread_run && pthread_create( &module->TID, &attr, (void *)module->Run_thread, module ) )
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': pthread_create failed. Unloading.", thread_tech_id );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }
    pthread_attr_destroy(&attr);                                                                        /* Libération mémoire */
    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Partage->com_msrv.Threads = g_slist_append ( Partage->com_msrv.Threads, module );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
    if (module->Thread_run)
     { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "%s: '%s': thread of class '%s' loaded (enable=1)",
                 __func__, thread_tech_id, thread_classe );
     }
    else
     { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "%s: '%s': thread of class '%s' not loaded (enable=0)",
                 __func__, thread_tech_id, thread_classe );
     }
  }
/******************************************************************************************************************************/
/* Charger_librairies: Ouverture de toutes les librairies possibles pour Watchdog                                             */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_librairies ( void )
  { JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/load", NULL );
    if (!api_result) { Info_new( __func__, Config.log_msrv, LOG_ERR, "%s: API Error for /run/thread LOAD",__func__ ); return; }

    if (Json_get_int ( api_result, "api_status" ) == SOUP_STATUS_OK)                                /* Chargement des modules */
     { JsonArray *array = Json_get_array ( api_result, "threads" );
       Info_new( __func__, Config.log_msrv, LOG_INFO, "%s: Loading %d thread",__func__, json_array_get_length(array) );
       Json_node_foreach_array_element ( api_result, "threads", Thread_Start_one_thread, NULL );
     }
    Json_node_unref(api_result);
  }
/******************************************************************************************************************************/
/* Demarrer_dls: Processus D.L.S                                                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_dls ( void )
  { Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Demande de demarrage %d", getpid() );
    if (Partage->com_dls.Thread_run == TRUE)
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "%s: An instance is already running %d",__func__, Partage->com_dls.TID );
       return(FALSE);
     }
    memset( &Partage->com_dls, 0, sizeof(Partage->com_dls) );                       /* Initialisation des variables du thread */
    if ( pthread_create( &Partage->com_dls.TID, NULL, (void *)Run_dls, NULL ) )
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "pthread_create failed" );
       return(FALSE);
     }
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "thread dls (%p) seems to be running", Partage->com_dls.TID );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Demarrer_API_sync: Processus de synchronisation avec l'API                                                                 */
/* Entrée: rien                                                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_api_sync ( void )
  { Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Demande de demarrage %d", getpid() );
    if ( pthread_create( &Partage->com_msrv.TID_api_sync, NULL, (void *)Run_api_sync, NULL ) )
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "pthread_create failed" );
       return(FALSE);
     }
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "thread api_sync (%p) seems to be running", Partage->com_msrv.TID_api_sync );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Demarrer_ARCH_sync: Processus de synchronisation d'archive                                                                 */
/* Entrée: rien                                                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_arch_sync ( void )
  { Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Demande de demarrage %d", getpid() );
    if ( pthread_create( &Partage->com_msrv.TID_arch_sync, NULL, (void *)Run_arch_sync, NULL ) )
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "pthread_create failed" );
       return(FALSE);
     }
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "thread arch_sync (%p) seems to be running", Partage->com_msrv.TID_arch_sync );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Demarrer_http: Processus HTTP                                                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_http ( void )
  { Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Demande de demarrage %d", getpid() );
    if (Partage->com_http.Thread_run == TRUE)
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "%s: An instance is already running %d",__func__, Partage->com_http.TID );
       return(FALSE);
     }
    memset( &Partage->com_http, 0, sizeof(Partage->com_http) );                     /* Initialisation des variables du thread */
    if ( pthread_create( &Partage->com_http.TID, NULL, (void *)Run_HTTP, NULL ) )
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "pthread_create failed" );
       return(FALSE);
     }
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "thread http (%p) seems to be running", Partage->com_http.TID );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Stopper_fils: arret de tous les fils Watchdog                                                                              */
/* Entré/Sortie: néant                                                                                                        */
/******************************************************************************************************************************/
 void Stopper_fils ( void )
  { Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Debut stopper_fils" );

    Info_new( __func__, Config.log_msrv, LOG_INFO, "Waiting for DLS (%p) to finish", Partage->com_dls.TID );
    Partage->com_dls.Thread_run = FALSE;
    while ( Partage->com_dls.TID != 0 ) sched_yield();                                                     /* Attente fin DLS */
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "ok, DLS is down" );

    Info_new( __func__, Config.log_msrv, LOG_INFO, "Waiting for API_SYNC (%p) to finish", Partage->com_msrv.TID_api_sync );
    if ( Partage->com_msrv.TID_api_sync ) pthread_join ( Partage->com_msrv.TID_api_sync, NULL );
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "ok, API_SYNC is down" );

    Info_new( __func__, Config.log_msrv, LOG_INFO, "Waiting for ARCH_SYNC (%p) to finish", Partage->com_msrv.TID_arch_sync );
    if ( Partage->com_msrv.TID_arch_sync ) pthread_join ( Partage->com_msrv.TID_arch_sync, NULL );
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "ok, ARCH_SYNC is down" );

    Info_new( __func__, Config.log_msrv, LOG_INFO, "Waiting for HTTP (%p) to finish", Partage->com_http.TID );
    Partage->com_http.Thread_run = FALSE;
    while ( Partage->com_http.TID != 0 ) sched_yield();                                                    /* Attente fin DLS */
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "ok, HTTP is down" );

    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Fin stopper_fils" );
    sleep(1);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
