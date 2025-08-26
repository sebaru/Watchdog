/******************************************************************************************************************************/
/* Watchdogd/thread.c        Gestion des Threads                                                                              */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                      sam 11 avr 2009 12:21:45 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * thread.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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
/* Thread_send_comm_to_master: Envoi le statut de la comm au master                                                           */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Thread_send_comm_to_master ( struct THREAD *module, gboolean etat )
  { if (module->comm_status != etat || module->comm_next_update <= Partage->top)
     { MQTT_Send_WATCHDOG ( module, "IO_COMM", (etat ? 900 : 0) );

       JsonNode *RootNode = Json_node_create();
       Json_node_add_string ( RootNode, "thread_classe",  Json_get_string ( module->config, "thread_classe"  ) );
       Json_node_add_string ( RootNode, "thread_tech_id", Json_get_string ( module->config, "thread_tech_id" ) );
       Json_node_add_bool   ( RootNode, "io_comm",        module->comm_status );
       Json_node_add_bool   ( RootNode, "mqtt_connected", module->MQTT_connected );
       MQTT_Send_to_API ( RootNode, "HEARTBEAT" );
       Json_node_unref ( RootNode );

       module->comm_next_update = Partage->top + 600;                                                   /* Toutes les minutes */
       module->comm_status = etat;
     }
  }
/******************************************************************************************************************************/
/* Thread_on_mqtt_master_message_CB: Appelé lorsque l'on recoit un message MQTT                                               */
/* Entrée: les parametres MQTT                                                                                                */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Thread_MQTT_on_message_CB(struct mosquitto *MQTT_session, void *obj, const struct mosquitto_message *msg)
  { struct THREAD *module = obj;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    if (module->Thread_run == FALSE) return;                     /* Si le module est en arret, on ne lui donne pas le message */
    JsonNode *response = Json_get_from_string ( msg->payload );
    if (!response)
     { Info_new( __func__, Config.log_bus, LOG_WARNING, "'%s': MQTT Message Dropped (not JSON) !", thread_tech_id );
       return;
     }
    else Info_new( __func__, module->Thread_debug, LOG_DEBUG, "'%s': MQTT Message received at %s: %s", thread_tech_id, msg->topic, msg->payload );

    gchar **tokens = g_strsplit ( msg->topic, "/", 3 );
    if (!tokens)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': MQTT token split failed at %s: %s", thread_tech_id, msg->topic, msg->payload );
       Json_node_unref ( response );
       return;
     }

    gint token_num = 0;
    while(tokens[token_num])
     { gchar token_name[16];
       g_snprintf ( token_name, sizeof ( token_name ), "token_lvl%d", token_num );
       Json_node_add_string ( response, token_name, tokens[token_num] );
       token_num++;
     }
    g_strfreev ( tokens );

    if (!strcasecmp (Json_get_string ( response, "token_lvl0" ), "SET_DEBUG"))
     { module->Thread_debug = Json_get_bool ( response, "debug" );
       Info_new( __func__, Config.log_msrv, LOG_NOTICE, "'%s': debug set to %d", thread_tech_id, module->Thread_debug );
       Json_node_unref ( response );
       return;
     }

    pthread_mutex_lock ( &module->synchro );                                                 /* on passe le message au thread */
    module->MQTT_messages = g_slist_append ( module->MQTT_messages, response );
    pthread_mutex_unlock ( &module->synchro );
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
     { MQTT_Send_AI ( module, module->ai_nbr_tour_par_sec, module->nbr_tour_par_sec, TRUE );
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
/* Thread_on_MQTT_connect_CB: appelé par la librairie quand le broker est connecté                                            */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Thread_MQTT_on_connect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { struct THREAD *module = obj;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "'%s': Connection to '%s': return code %d: %s",
              thread_tech_id, Config.master_hostname, return_code, mosquitto_connack_string( return_code ) );
    if (return_code == 0)
     { module->MQTT_connected = TRUE;
       MQTT_Subscribe ( module->MQTT_session, "SET_AO/%s/#", thread_tech_id );
       MQTT_Subscribe ( module->MQTT_session, "SET_DO/%s/#", thread_tech_id );
       MQTT_Subscribe ( module->MQTT_session, "SET_TEST/%s/#", thread_tech_id );
       MQTT_Subscribe ( module->MQTT_session, "SET_DEBUG/%s/#", thread_tech_id );
#warning a supprimer
       MQTT_Subscribe ( module->MQTT_session, "threads/#" );
     }
  }
/******************************************************************************************************************************/
/* Thread_on_MQTT_disconnect_CB: appelé par la librairie quand le broker est déconnecté                                       */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Thread_MQTT_on_disconnect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { struct THREAD *module = obj;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "'%s': Disconnected with return code %d: %s",
              thread_tech_id, return_code, mosquitto_connack_string( return_code ) );
    module->MQTT_connected = FALSE;
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

/******************************************************* Ecoute du MQTT *******************************************************/
    module->MQTT_session = mosquitto_new( thread_tech_id, TRUE, module );
    if (!module->MQTT_session)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': MQTT session error.", thread_tech_id ); }
    else
     { mosquitto_message_callback_set    ( module->MQTT_session, Thread_MQTT_on_message_CB );
       mosquitto_reconnect_delay_set     ( module->MQTT_session, 10, 60, TRUE );
       mosquitto_log_callback_set        ( module->MQTT_session, MQTT_on_log_CB );
       mosquitto_connect_callback_set    ( module->MQTT_session, Thread_MQTT_on_connect_CB );
       mosquitto_disconnect_callback_set ( module->MQTT_session, Thread_MQTT_on_disconnect_CB );
       mosquitto_username_pw_set         ( module->MQTT_session, thread_tech_id, NULL );

       if ( mosquitto_connect( module->MQTT_session, Config.master_hostname, 1883, 60 ) != MOSQ_ERR_SUCCESS )
        { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': MQTT connection to '%s' error.", thread_tech_id, Config.master_hostname ); }
     }

    if ( mosquitto_loop_start( module->MQTT_session ) != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': MQTT loop not started.", thread_tech_id ); }

    gchar *description = "Add description to database table";
    if (Json_has_member ( module->config, "description" )) description = Json_get_string ( module->config, "description" );
    gchar package[128];
    g_snprintf ( package, sizeof(package), "Thread_%s", Json_get_string ( module->config, "thread_classe" ) );
    if (Dls_auto_create_plugin( thread_tech_id, description, package ) == FALSE)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: DLS Create ERROR (%s)\n", thread_tech_id, description ); }

    module->IOs = Json_node_create();
    Json_node_add_array ( module->IOs, "IOs" );

    module->ai_nbr_tour_par_sec = Mnemo_create_thread_AI ( module, "THREAD_TOUR_PAR_SEC", "Nombre de tour par seconde", "t/s", ARCHIVE_5_MIN );
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
    mosquitto_disconnect( module->MQTT_session );
    mosquitto_loop_stop( module->MQTT_session, FALSE );
    mosquitto_destroy( module->MQTT_session );
    g_slist_foreach ( module->MQTT_messages, (GFunc) Json_node_unref, NULL );
    g_slist_free    ( module->MQTT_messages );   module->MQTT_messages = NULL;
    if (module->vars) { g_free(module->vars);  module->vars   = NULL; }
    Json_node_unref ( module->IOs );           module->IOs    = NULL;
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "'%s' is DOWN", Json_get_string ( module->config, "thread_tech_id") );
    sleep(1);                       /* le temps d'un appel libsoup a Thread_ws_on_master_connected si Operation was cancelled */
    pthread_exit(0);
  }
/******************************************************************************************************************************/
/* Thread_List_by_classe: Récupère la liste des thread_tech_id de classe en parametre, sous forme de liste                         */
/* Entrée: La classe a chercher                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static GSList *Thread_List_by_classe ( gchar *thread_classe )
  { if (!thread_classe) return(NULL);
    GSList *results = NULL;
    pthread_rwlock_rdlock ( &Partage->Threads_synchro );
    GSList *liste = Partage->Threads;                                                                 /* Parcours de la liste */
    while(liste)
     { struct THREAD *module = liste->data;
       if (!strcasecmp ( thread_classe, Json_get_string ( module->config, "thread_classe" ) ) )
        { results = g_slist_append ( results, strdup ( Json_get_string ( module->config, "thread_tech_id" ) ) );
          break;
        }
       liste = liste->next;
     }
    pthread_rwlock_unlock ( &Partage->Threads_synchro );
    return(results);
  }
/******************************************************************************************************************************/
/* Thread_take_first_module: Récupère le premier thread, en le supprimant de la liste des modules                             */
/* Entrée: Le tech_id du thread                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static struct THREAD *Thread_take_first_module ( void )
  { if (!Partage->Threads) return(NULL);
    pthread_rwlock_wrlock ( &Partage->Threads_synchro );
    struct THREAD *module = Partage->Threads->data;                                                  /* On a trouvé le thread */
    Partage->Threads = g_slist_remove( Partage->Threads, module );
    pthread_rwlock_unlock ( &Partage->Threads_synchro );
    return(module);
  }
/******************************************************************************************************************************/
/* Thread_take_module_by_tech_id: Récupère le pointeur du thread, en le supprimant de la liste des modules                    */
/* Entrée: Le tech_id du thread                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static struct THREAD *Thread_take_module_by_tech_id ( gchar *thread_tech_id )
  { if (!thread_tech_id) return(NULL);
    struct THREAD *module = NULL;
    pthread_rwlock_wrlock ( &Partage->Threads_synchro );
    GSList *liste = Partage->Threads;                                                                 /* Parcours de la liste */
    while(liste)
     { struct THREAD *search_module = liste->data;
       if (!strcasecmp ( thread_tech_id, Json_get_string ( search_module->config, "thread_tech_id" ) ) )
        { module = search_module;                                                                    /* On a trouvé le thread */
          Partage->Threads = g_slist_remove( Partage->Threads, module );
          break;
        }
       liste = liste->next;
     }
    pthread_rwlock_unlock ( &Partage->Threads_synchro );
    return(module);
  }
/******************************************************************************************************************************/
/* Thread_Stop_safe: Stop un thread                                                                                           */
/* Entrée: Le module, hors de la liste des threads                                                                            */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Thread_Stop_safe ( struct THREAD *module )
  { if (!module) { Info_new( __func__, Config.log_msrv, LOG_ERR, "Module is NULL" ); return; }
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    module->Thread_run = FALSE;
    Info_new( __func__, Config.log_msrv, LOG_INFO, "'%s': Stopping", thread_tech_id );
    if (module->TID) pthread_join( module->TID, NULL );                                                /* Attente fin du fils */
    Info_new( __func__, Config.log_msrv, LOG_INFO, "'%s': Stopped", thread_tech_id );

    if (module->dl_handle) dlclose( module->dl_handle );
    pthread_mutex_destroy( &module->synchro );
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "'%s': Unloaded and freed", thread_tech_id );
    if (module->config) Json_node_unref ( module->config );
    g_free( module );
  }
/******************************************************************************************************************************/
/* Thread_Stop_all: Decharge tous les threads                                                                                 */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Thread_Stop_all ( void )
  { pthread_rwlock_rdlock ( &Partage->Threads_synchro );
    GSList *liste = Partage->Threads;                     /* Envoie une commande d'arret pour toutes les librairies d'un coup */
    while(liste)
     { struct THREAD *module = liste->data;
       module->Thread_run = FALSE;                                                       /* On demande au thread de s'arreter */
       liste = liste->next;
     }
    pthread_rwlock_unlock ( &Partage->Threads_synchro );

    while(Partage->Threads)                                                                 /* Liberation mémoire des modules */
     { struct THREAD *module = Thread_take_first_module();
       Thread_Stop_safe ( module );
     }
  }
/******************************************************************************************************************************/
/* Thread_Start_by_thread_tech_id: Création d'un thread                                                                       */
/* Entrée: La classe et le tech_id                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Thread_Start_by_thread_tech_id ( gchar *thread_tech_id )
  { if (!thread_tech_id) { Info_new( __func__, Config.log_msrv, LOG_ERR, "no 'thread_tech_id'" ); return; }

    struct THREAD *module = g_try_malloc0( sizeof(struct THREAD) );
    if (!module)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': Not Enought Memory", thread_tech_id );
       return;
     }

    module->config = Http_Get_from_global_API ( "/run/thread/config", "thread_tech_id=%s", thread_tech_id );
    if (module->config && Json_get_int ( module->config, "http_code" ) == 200)
     { module->Thread_debug = Json_get_bool ( module->config, "debug" );
       module->Thread_run   = Json_get_bool ( module->config, "enable" );
     }
    else
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': GET_CONFIG from API Failed. Unloading.", thread_tech_id );
       Thread_Stop_safe ( module );
       return;
     }

    if (!module->Thread_run)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s' is not enabled. Unloading.", thread_tech_id );
       Thread_Stop_safe ( module );
       return;
     }

    gchar *thread_classe = Json_get_string ( module->config, "thread_classe" );
    if (!thread_classe)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "no 'thread_classe' in Json" );
       Thread_Stop_safe ( module );
       return;
     }

    gchar nom_fichier[256];
    g_snprintf( nom_fichier,  sizeof(nom_fichier), "%s/libwatchdog-server-%s.so", Config.librairie_dir, thread_classe );

    module->dl_handle = dlopen( nom_fichier, RTLD_GLOBAL | RTLD_NOW );
    if (!module->dl_handle)
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "'%s': Thread '%s' loading failed (%s)",
                 thread_tech_id, nom_fichier, dlerror() );
       g_free(module);
       return;
     }

    module->Run_thread = dlsym( module->dl_handle, "Run_thread" );                        /* Recherche de la fonction */
    if (!module->Run_thread)
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "'%s': Thread '%s' rejected (Run_thread not found)",
                 thread_tech_id, nom_fichier );
       dlclose( module->dl_handle );
       g_free(module);
       return;
     }


    pthread_attr_t attr;                                                       /* Attribut de mutex pour parametrer le module */
    if ( pthread_attr_init(&attr) )                                                 /* Initialisation des attributs du thread */
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': pthread_attr_init failed. Unloading.", thread_tech_id );
       Thread_Stop_safe ( module );
       return;
     }

    if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) )                       /* On le laisse joinable au boot */
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': pthread_setdetachstate failed. Unloading.", thread_tech_id );
       Thread_Stop_safe ( module );
       return;
     }

    pthread_mutexattr_t param;                                                                /* Creation du mutex de synchro */
    pthread_mutexattr_init( &param );                                                         /* Creation du mutex de synchro */
    pthread_mutexattr_setpshared( &param, PTHREAD_PROCESS_SHARED );
    pthread_mutex_init( &module->synchro, &param );

    if ( module->Thread_run && pthread_create( &module->TID, &attr, (void *)module->Run_thread, module ) )
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': pthread_create failed. Unloading.", thread_tech_id );
       Thread_Stop_safe ( module );
       return;
     }
    pthread_attr_destroy(&attr);                                                                        /* Libération mémoire */
    pthread_rwlock_wrlock ( &Partage->Threads_synchro );
    Partage->Threads = g_slist_append ( Partage->Threads, module );
    pthread_rwlock_unlock ( &Partage->Threads_synchro );
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Thread '%s' of class '%s' loaded with enable='%d'",
              thread_tech_id, thread_classe, module->Thread_run );
  }
/******************************************************************************************************************************/
/* Thread_Stop_by_thread_tech_id: Decharge un seul et unique thread                                                           */
/* Entrée: Le tech_id du thread                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Thread_Stop_by_thread_tech_id ( gchar *thread_tech_id )
  { if (!thread_tech_id)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "thread_tech_id not provided" ); return; }

    struct THREAD *module = Thread_take_module_by_tech_id ( thread_tech_id );/* Sortie de la structure de la liste des Threads */
    if (!module)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s': thread not found, so not stopped.", thread_tech_id ); return; }

    Thread_Stop_safe ( module );                                     /* Arret du module hors liste, a l'issue module is freed */
  }
/******************************************************************************************************************************/
/* Thread_Restart_by_thread_tech_id: Decharge et recharge un thread par thread_tech_id                                        */
/* Entrée: Le tech_id du thread                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Thread_Restart_by_thread_tech_id ( gchar *thread_tech_id )
  { if (!thread_tech_id)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "thread_tech_id not provided" ); return; }

    Thread_Stop_by_thread_tech_id ( thread_tech_id );
    Thread_Start_by_thread_tech_id ( thread_tech_id );
  }
/******************************************************************************************************************************/
/* Thread_Restart_by_classe: Decharge et recharge les threads d'une classe en parametre                                       */
/* Entrée: Le thread_classe a restarter                                                                                       */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Thread_Restart_by_classe ( gchar *thread_classe )
  { if (!thread_classe)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "thread_classe not provided" ); return; }

    GSList *Threads = Thread_List_by_classe ( thread_classe );
    GSList *liste = Threads;
    while (liste)
     { gchar *thread_tech_id = liste->data;
       Thread_Stop_by_thread_tech_id ( thread_tech_id );
       Thread_Start_by_thread_tech_id ( thread_tech_id );
     }
    g_slist_free_full ( Threads, g_free );
  }
/******************************************************************************************************************************/
/* Thread_Start_all: Ouverture de toutes les librairies possibles pour Watchdog                                               */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Thread_Start_all ( void )
  { JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/load", NULL );
    if (!api_result) { Info_new( __func__, Config.log_msrv, LOG_ERR, "%s: API Error for /run/thread LOAD",__func__ ); return; }

    if (Json_get_int ( api_result, "http_code" ) == 200)                                           /* Chargement des modules */
     { JsonArray *array = Json_get_array ( api_result, "threads" );
       Info_new( __func__, Config.log_msrv, LOG_INFO, "Loading %d thread(s)", json_array_get_length(array) );
       GList *Threads = json_array_get_elements ( array );
       GList *threads = Threads;
       while(threads)
        { JsonNode *element = threads->data;
          gchar *thread_tech_id = Json_get_string ( element, "thread_tech_id" );
          if (Json_get_bool ( element, "enable" )) Thread_Start_by_thread_tech_id ( thread_tech_id );
          threads = g_list_next(threads);
        }
       g_list_free(Threads);
     }
    Json_node_unref(api_result);
  }
/******************************************************************************************************************************/
/* Demarrer_dls: Processus D.L.S                                                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_dls ( void )
  { Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Demande de demarrage DLS %d", getpid() );
    if ( pthread_create( &Partage->com_dls.TID, NULL, (void *)Run_dls, NULL ) )
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "pthread_create failed" );
       return(FALSE);
     }
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "thread dls (%p) seems to be running", Partage->com_dls.TID );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Stopper_dls: arret du processus D.L.S                                                                                      */
/* Entré/Sortie: néant                                                                                                        */
/******************************************************************************************************************************/
 void Stopper_dls ( void )
  { Info_new( __func__, Config.log_msrv, LOG_INFO, "Waiting for DLS (%p) to finish", Partage->com_dls.TID );
    Partage->com_dls.Thread_run = FALSE;
    if ( Partage->com_dls.TID ) pthread_join ( Partage->com_dls.TID, NULL );                               /* Attente fin DLS */
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "ok, DLS is down" );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
