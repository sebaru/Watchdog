/******************************************************************************************************************************/
/* Watchdogd/thread.c        Gestion des Threads                                                                              */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                      sam 11 avr 2009 12:21:45 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * thread.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
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
 #include <link.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Thread_update_current_context: Met à jour le contexte du thread_tech_id choisi par l'index                                 */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 static void Thread_update_current_context ( struct THREAD *module )
  { module->config = Json_array_get_element_at ( module->config_all, "thread_tech_ids", module->current_thread_tech_id_index );
    module->vars = g_slist_nth_data ( module->vars_all, module->current_thread_tech_id_index );
    module->current_thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
  }

/******************************************************************************************************************************/
/* Thread_send_comm_to_master: Envoi le statut de la comm au master                                                           */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Thread_send_comm_to_master ( struct THREAD *module, gboolean etat )
  { if (module->MQTT_connected == FALSE) return;                                           /* Si pas de connexion, on return; */
    if (module->comm_status != etat || module->comm_next_update <= Partage->top)
     { MQTT_Send_WATCHDOG ( module, "IO_COMM", (etat ? 900 : 0) );

       JsonNode *RootNode = Json_create();
       Json_add_string ( RootNode, "thread_classe",  Json_get_string ( module->config, "thread_classe"  ) );
       Json_add_string ( RootNode, "thread_tech_id", Json_get_string ( module->config, "thread_tech_id" ) );
       Json_add_bool   ( RootNode, "io_comm",        module->comm_status );
       Json_add_bool   ( RootNode, "mqtt_connected", (etat ? module->MQTT_connected : FALSE) );
       MQTT_Send_to_API ( RootNode, "HEARTBEAT" );
       Json_unref ( RootNode );

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
    gchar *thread_tech_id = module->current_thread_tech_id;
    if (module->Thread_run == FALSE) return;                     /* Si le module est en arret, on ne lui donne pas le message */
    JsonNode *response = Json_get_from_string ( msg->payload );
    if (!response)
     { Info_with_prefix( __func__, "mqtt", thread_tech_id, LOG_WARNING,
                         "'%s': MQTT Message Dropped (not JSON) !", thread_tech_id );
       return;
     }
    else { Info_with_prefix( __func__, "mqtt", thread_tech_id, LOG_DEBUG,
                 "'%s': MQTT Message received at %s: %s", thread_tech_id, msg->topic, msg->payload );
       }

    gchar **tokens = g_strsplit ( msg->topic, "/", 3 );
    if (!tokens)
     { Info_with_prefix( __func__, "mqtt", thread_tech_id, LOG_ERR,
                            "'%s': MQTT token split failed at %s: %s", thread_tech_id, msg->topic, msg->payload );
       Json_unref ( response );
       return;
     }

    gint token_num = 0;
    while(tokens[token_num])
     { gchar token_name[16];
       g_snprintf ( token_name, sizeof ( token_name ), "token_lvl%d", token_num );
       Json_add_string ( response, token_name, tokens[token_num] );
       token_num++;
     }
    g_strfreev ( tokens );

    if (!strcasecmp (Json_get_string ( response, "token_lvl0" ), "SET_DEBUG"))
     { gchar *facility = Json_get_string ( response, "facility" );
       gboolean debug = Json_get_bool ( response, "debug" );
       if (!facility || !strlen(facility))
        { Info_with_prefix( __func__, "msrv", thread_tech_id, LOG_ERR,
                            "'%s': SET_DEBUG missing facility", thread_tech_id );
          Json_unref ( response );
          return;
        }

       if (debug) Info_debug_facility   ( thread_tech_id, facility );
             else Info_undebug_facility ( thread_tech_id, facility );

       Info_with_prefix( __func__, "msrv", thread_tech_id, LOG_NOTICE,
                         "'%s': facility '%s' debug set to %d", thread_tech_id, facility, debug );
       Json_unref ( response );
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

/********************************************* Reconnexion au broker MQTT local ***********************************************/
    if (module->MQTT_connected == FALSE && module->MQTT_next_top_connect <= Partage->top )        /* tentative de reconnexion */
     { gchar *thread_tech_id = module->current_thread_tech_id;
      Info_with_prefix( __func__, "mqtt", thread_tech_id, LOG_INFO,
              "'%s': Retrying MQTT connection to '%s'.",
              thread_tech_id, Config.master_hostname );
       mosquitto_reconnect_async(	module->MQTT_session	);
       module->MQTT_next_top_connect = Partage->top + THREAD_MQTT_RECONNECT_DELAY;
     }

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
/*************************************************Passage au tech_id suivant **************************************************/
    module->current_thread_tech_id_index++;
    if (module->current_thread_tech_id_index >= module->nb_thread_tech_ids) module->current_thread_tech_id_index = 0;
    if (module->vars_all) module->vars = g_slist_nth_data ( module->vars_all, module->current_thread_tech_id_index );
    module->config = Json_array_get_element_at ( module->config_all, "thread_tech_ids", module->current_thread_tech_id_index );
    module->current_thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
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
    gchar *thread_tech_id = module->current_thread_tech_id;
    Info_with_prefix( __func__, "mqtt", thread_tech_id, LOG_NOTICE,
                      "'%s': Connection to '%s': return code %d: %s",
                      thread_tech_id, Config.master_hostname, return_code, mosquitto_connack_string( return_code ) );
    if (return_code == 0)
     { module->MQTT_connected = TRUE;
       if (Json_has_member ( module->config, "tech_ids_list" ))    /* Thread de classe: souscription pour chaque tech_id */
        { JsonArray *list = Json_get_array ( module->config, "tech_ids_list" );
          GList *elements = json_array_get_elements ( list );
          GList *elem = elements;
          while (elem)
           { JsonNode *sub_config = elem->data;
             gchar *sub_tech_id = Json_get_string ( sub_config, "thread_tech_id" );
             MQTT_Subscribe ( module->MQTT_session, "SET_AO/%s/#",    sub_tech_id );
             MQTT_Subscribe ( module->MQTT_session, "SET_DO/%s/#",    sub_tech_id );
             MQTT_Subscribe ( module->MQTT_session, "SET_TEST/%s/#",  sub_tech_id );
             MQTT_Subscribe ( module->MQTT_session, "SET_DEBUG/%s/#", sub_tech_id );
             elem = g_list_next ( elem );
           }
          g_list_free ( elements );
        }
       else
        { MQTT_Subscribe ( module->MQTT_session, "SET_AO/%s/#",    thread_tech_id );
          MQTT_Subscribe ( module->MQTT_session, "SET_DO/%s/#",    thread_tech_id );
          MQTT_Subscribe ( module->MQTT_session, "SET_TEST/%s/#",  thread_tech_id );
          MQTT_Subscribe ( module->MQTT_session, "SET_DEBUG/%s/#", thread_tech_id );
        }
     }
  }
/******************************************************************************************************************************/
/* Thread_on_MQTT_disconnect_CB: appelé par la librairie quand le broker est déconnecté                                       */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Thread_MQTT_on_disconnect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { struct THREAD *module = obj;
    gchar *thread_tech_id = module->current_thread_tech_id;
    Info_with_prefix( __func__, "mqtt", thread_tech_id, LOG_NOTICE,
                      "'%s': Disconnected with return code %d: %s. Retry in %ds.", thread_tech_id, return_code, mosquitto_connack_string(return_code), THREAD_MQTT_RECONNECT_DELAY );
    module->MQTT_connected = FALSE;
    module->MQTT_next_top_connect = Partage->top + THREAD_MQTT_RECONNECT_DELAY;
  }

 static void Thread_del_one_slot ( struct THREAD *module, gint id )
  {
     #warning to be updated
    /*Json_array_del_one_element ( module->config_all, "thread_tech_ids", id );*/
    gpointer vars = g_slist_nth_data ( module->vars_all, id );
    if (vars) g_free(vars);
    module->vars_all = g_slist_remove ( module->vars_all, vars );
  }

/******************************************************************************************************************************/
/* Thread_Start_one_tech_id: appelé pour démarrer un tech_id dans le thread                                                   */
/* Entrée: La structure afférente, l'index a demarrer                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Thread_Start_one_tech_id ( struct THREAD *module, guint index )
  { JsonNode *thread_tech_idNode = Json_array_get_element_at ( module->config_all, "thread_tech_ids", index );
    gchar *thread_tech_id = Json_get_string ( thread_tech_idNode, "thread_tech_id" );
    Info_with_prefix( __func__, "msrv", thread_tech_id, LOG_INFO, "Adding memory slot for thread '%s'", thread_tech_id );
#warnning a passer avec hash ? ou en vars{10]}
    gpointer vars = g_try_malloc0 ( sizeof_vars );
    if (!vars) 
     { Info_with_prefix( __func__, "msrv", module->current_thread_tech_id, LOG_ERR, "Memory error for one new slot." ); }
    else module->vars_all = g_slist_append ( module->vars_all, vars );
  }
/******************************************************************************************************************************/
/* Thread_init: appelé par chaque thread, lors de son démarrage                                                               */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Thread_init ( struct THREAD *module, gint sizeof_vars )
  { gchar chaine[128];
    setlocale( LC_ALL, "C" );                                             /* Pour le formatage correct des , . dans les float */

    gchar *thread_classe = Json_get_string ( module->config, "thread_classe" );
    mkdir ( thread_classe, S_IRUSR | S_IWUSR | S_IXUSR );
    g_snprintf( chaine, sizeof(chaine), "W-%s", thread_classe );                                   /* Positionne le nom noyau */
    gchar *upper_name = g_ascii_strup ( chaine, -1 );
    prctl(PR_SET_NAME, upper_name, 0, 0, 0 );
    g_free(upper_name);

    module->current_thread_tech_id_index = 0;
    module->sizeof_vars = sizeof_vars;
    for ( gint i = 0; i<Json_array_get_length ( module->config_all, "thread_tech_ids" ); i++ )
     { Thread_Start_one_tech_id ( module, i ); }
    Thread_update_current_context ( module );

/* ----------------------------------------------------- Ecoute du MQTT local------------------------------------------------ */
    module->MQTT_session = mosquitto_new( thread_classe, TRUE, module );
    if (!module->MQTT_session)
     { Info_with_prefix( __func__, "mqtt", thread_classe, LOG_ERR, "'%s': MQTT session error.", thread_classe ); }
    else
     { mosquitto_message_callback_set    ( module->MQTT_session, Thread_MQTT_on_message_CB );
       /*mosquitto_reconnect_delay_set     ( module->MQTT_session, 10, 60, TRUE );*/
       mosquitto_log_callback_set        ( module->MQTT_session, MQTT_on_log_CB );
       mosquitto_connect_callback_set    ( module->MQTT_session, Thread_MQTT_on_connect_CB );
       mosquitto_disconnect_callback_set ( module->MQTT_session, Thread_MQTT_on_disconnect_CB );
       mosquitto_username_pw_set         ( module->MQTT_session, thread_classe, NULL );
#warning ajouter thread_uuid

       if ( mosquitto_connect( module->MQTT_session, Config.master_hostname, 1883, 60 ) != MOSQ_ERR_SUCCESS )
        { Info_with_prefix( __func__, "mqtt", thread_classe, LOG_ERR,
                       "'%s': MQTT connection to '%s' error. Retry in %ds.",
                       thread_classe, Config.master_hostname, THREAD_MQTT_RECONNECT_DELAY/10 );
          module->MQTT_next_top_connect = Partage->top + THREAD_MQTT_RECONNECT_DELAY;
        }
     }

    if ( mosquitto_loop_start( module->MQTT_session ) != MOSQ_ERR_SUCCESS )
      { Info_with_prefix( __func__, "mqtt", thread_classe, LOG_ERR, "'%s': MQTT loop not started.", thread_classe );
      }

/* ------------------------------------------- Création du plugin dans l'api ------------------------------------------------ */
    JsonNode *RootNode = Json_create();
    if (RootNode)
     { Json_add_string ( RootNode, "tech_id", thread_tech_id );
       Json_add_string ( RootNode, "thread_classe", thread_classe ); 
       Json_add_int    ( RootNode, "syn_id", 1 ); 
       gchar *name = Json_get_string ( module->config, "description" );
       Json_add_string ( RootNode, "name", name );
       Json_add_string ( RootNode, "shortname", name );
       gchar package[128];
       g_snprintf ( package, sizeof(package), "Thread_%s", thread_classe );
       Json_add_string ( RootNode, "package", package );
       if (Dls_auto_create_plugin( RootNode ) == FALSE)
        { Info_with_prefix( __func__, "dls", thread_tech_id, LOG_ERR, "%s: DLS Create ERROR (%s)\n", thread_tech_id, name ); 
        }
       Json_unref ( RootNode );
     }
    else 
     { Info_with_prefix( __func__, "dls", thread_tech_id, LOG_ERR, "%s: Memory error while creating DLS RootNode", thread_tech_id ); }

/* ------------------------------------------------ Création des IOs --------------------------------------------------------- */
    module->IOs = Json_create();
    Json_add_array ( module->IOs, "IOs" );

    module->ai_nbr_tour_par_sec = Mnemo_create_thread_AI ( module, "THREAD_TOUR_PAR_SEC", "Nombre de tour par seconde", "t/s", ARCHIVE_5_MIN );
    Mnemo_create_thread_WATCHDOG ( module, "IO_COMM", "Statut de la communication" );
    Info_with_prefix( __func__, "msrv", thread_tech_id, LOG_NOTICE, "Thread '%s' is UP", thread_tech_id );
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
    g_slist_foreach ( module->MQTT_messages, (GFunc) Json_unref, NULL );
    g_slist_free    ( module->MQTT_messages );   module->MQTT_messages = NULL;
    if (module->vars) { g_slist_free_full (module->vars, g_free);  module->vars   = NULL; }
    Json_unref ( module->IOs );           module->IOs    = NULL;
    Info_with_prefix( __func__, "msrv", module->current_thread_tech_id, LOG_NOTICE,
                         "'%s' is DOWN", module->current_thread_tech_id );
    sleep(1);                       /* le temps d'un appel libsoup a Thread_ws_on_master_connected si Operation was cancelled */
    pthread_exit(0);
  }
/******************************************************************************************************************************/
/* Thread_Stop_safe: Stop un thread                                                                                           */
/* Entrée: Le module, hors de la liste des threads                                                                            */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Thread_Stop_safe ( struct THREAD *module )
  { if (!module) { Info( __func__, "msrv", LOG_ERR, "Module is NULL" ); return; }
    gchar *thread_classe = Json_get_string ( module->config, "thread_classe" );

    module->Thread_run = FALSE;
    Info( __func__, "msrv", LOG_INFO, "'%s': Stopping", thread_classe );
    if (module->TID) pthread_join( module->TID, NULL );                                                /* Attente fin du fils */
    Info( __func__, "msrv", LOG_INFO, "'%s': Stopped", thread_classe );

    if (module->dl_handle) dlclose( module->dl_handle );
    pthread_mutex_destroy( &module->synchro );
    Info( __func__, "msrv", LOG_NOTICE, "'%s': Unloaded and freed", thread_classe );
    if (module->config) Json_unref ( module->config );
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
     { pthread_rwlock_wrlock ( &Partage->Threads_synchro );
       struct THREAD *module = Partage->Threads->data;                                               /* On a trouvé le thread */
       Partage->Threads = g_slist_remove( Partage->Threads, module );
       pthread_rwlock_unlock ( &Partage->Threads_synchro );
       Thread_Stop_safe ( module );
     }
  }
/******************************************************************************************************************************/
/* Thread_Load_library: Chargement d'une librairie                                                                            */
/* Entrée: Le module a charger                                                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Thread_Load_library ( struct THREAD *module )
  { gchar *thread_classe = Json_get_string ( module->config, "thread_classe" );
    gchar nom_fichier[256];
    g_snprintf( nom_fichier,  sizeof(nom_fichier), "libwatchdog-server-%s.so", thread_classe );

    module->dl_handle = dlopen( nom_fichier, RTLD_GLOBAL | RTLD_NOW );
    if (!module->dl_handle)
     { Info( __func__, "msrv", LOG_WARNING, "Thread '%s' dlopen failed (%s) : ", nom_fichier, dlerror() );
       Thread_Stop_safe ( module );
       return;
     }
    struct link_map *map;
    if (dlinfo(module->dl_handle, RTLD_DI_LINKMAP, &map) != 0)
     { Info( __func__, "msrv", LOG_WARNING, "Thread '%s' dl_info failed (%s)", nom_fichier, dlerror() );
       Thread_Stop_safe ( module );
       return;
     }
    else Info( __func__, "msrv", LOG_NOTICE, "Thread '%s' : using file '%s'", thread_classe, map->l_name );

    module->Run_thread = dlsym( module->dl_handle, "Run_thread" );                                /* Recherche de la fonction */
    if (!module->Run_thread)
     { Info( __func__, "msrv", LOG_WARNING, "Thread '%s': File '%s' rejected (Run_thread not found)",
             thread_classe, nom_fichier );
       Thread_Stop_safe ( module );
       return;
     }

    pthread_attr_t attr;                                                       /* Attribut de mutex pour parametrer le module */
    if ( pthread_attr_init(&attr) )                                                 /* Initialisation des attributs du thread */
     { Info( __func__, "msrv", LOG_ERR, "Thread '%s': pthread_attr_init failed. Unloading.", thread_classe );
       Thread_Stop_safe ( module );
       return;
     }

    if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) )                       /* On le laisse joinable au boot */
     { Info( __func__, "msrv", LOG_ERR, "Thread '%s': pthread_setdetachstate failed. Unloading.", thread_classe );
       Thread_Stop_safe ( module );
       return;
     }

    pthread_mutexattr_t param;                                                                /* Creation du mutex de synchro */
    pthread_mutexattr_init( &param );                                                         /* Creation du mutex de synchro */
    pthread_mutexattr_setpshared( &param, PTHREAD_PROCESS_SHARED );
    pthread_mutex_init( &module->synchro, &param );
    module->Thread_run = TRUE;                                                           /* Le thread est runnning by default */

    if ( module->Thread_run && pthread_create( &module->TID, &attr, (void *)module->Run_thread, module ) )
     { Info( __func__, "msrv", LOG_ERR, "Thread '%s': pthread_create failed. Unloading.", thread_classe );
       Thread_Stop_safe ( module );
       return;
     }
    pthread_attr_destroy(&attr);                                                                        /* Libération mémoire */
    pthread_rwlock_wrlock ( &Partage->Threads_synchro );
    Partage->Threads = g_slist_append ( Partage->Threads, module );
    pthread_rwlock_unlock ( &Partage->Threads_synchro );
    Info( __func__, "msrv", LOG_NOTICE, "Thread '%s' loaded", thread_classe );
  }
/******************************************************************************************************************************/
/* Thread_Start_one_thread_classe: Création d'un thread pour une classe spécifique                                            */
/* Entrée: La classe du thread                                                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Thread_Start_one_thread_classe ( gchar *thread_classe )
  { if (!thread_classe)
  { Info( __func__, "msrv", LOG_ERR, "no 'thread_classe' provided" ); return; }

    struct THREAD *found = NULL;
    pthread_rwlock_rdlock ( &Partage->Threads_synchro );
    GSList *liste = Partage->Threads;                                                                 /* Parcours de la liste */
    while(liste)
     { struct THREAD *module = liste->data;
       if (!strcasecmp ( thread_classe, Json_get_string ( module->config, "thread_classe" ) ) )
        { found = module; break; }
       liste = liste->next;
     }
    pthread_rwlock_unlock ( &Partage->Threads_synchro );
    if (found)
     { Info_with_prefix( __func__, "msrv", thread_classe, LOG_ERR,
                         "Thread Class '%s': Cannot start, already running", thread_classe );
       return;
     }

    struct THREAD *module = g_try_malloc0( sizeof(struct THREAD) );
    if (!module)
     { Info_with_prefix( __func__, "msrv", thread_classe, LOG_ERR,
                         "Thread Class '%s': Not Enough Memory", thread_classe );
       return;
     }

    Info_with_prefix( __func__, "msrv", thread_classe, LOG_INFO,
                      "Thread Class '%s': Requesting config from API", thread_classe );
    module->config_all = Http_Get_from_global_API ( "/run/thread/config", "thread_classe=%s", thread_classe );
    if ( ! (module->config_all && Json_get_int ( module->config_all, "http_code" ) == 200) )
     { Info_with_prefix( __func__, "msrv", thread_classe, LOG_ERR,
                         "Thread Class '%s': GET_CONFIG from API Failed. Unloading.", thread_classe );
       Thread_Stop_safe ( module );
       return;
     }

    if (!Json_has_member ( module->config_all, "thread_classe" ) )
     { Info_with_prefix( __func__, "msrv", thread_classe, LOG_ERR,
                         "Thread Class '%s': Missing 'thread_classe' in API response. Unloading.", thread_classe );
       Thread_Stop_safe ( module );
       return;
     }

    if (strcasecmp ( thread_classe, Json_get_string ( module->config_all, "thread_classe" ) ) )
     { Info_with_prefix( __func__, "msrv", thread_classe, LOG_ERR,
                         "Thread Class '%s': Root Class is not the same as API Class. Unloading.", thread_classe );
       Thread_Stop_safe ( module );
       return;
     }
    module->nb_thread_tech_ids = Json_get_int ( module->config_all, "nbr_thread_tech_ids" );
    Info_with_prefix( __func__, "msrv", thread_classe, LOG_NOTICE,
                      "Thread Class '%s': Loading class for '%d' thread_tech_ids",
          thread_classe, module->nb_thread_tech_ids );
    
    Thread_Load_library ( module );
  }
/******************************************************************************************************************************/
/* Thread_Start_one_classe_by_array: Demarre une classe complete depuis un element du tableau de l'API                        */
/******************************************************************************************************************************/
 static void Thread_Start_one_thread_classe_by_array ( JsonArray *array, guint index, JsonNode *class_node, gpointer user_data )
  { gchar *thread_classe = Json_get_string ( class_node, "thread_classe" );

    if (!thread_classe)
     { Info( __func__, "msrv", LOG_ERR, "Missing 'thread_classe' in API response" );
       return;
     }

    Thread_Start_one_thread_classe ( thread_classe );
  }
/******************************************************************************************************************************/
/* Thread_Start_all: Ouverture de toutes les librairies possibles pour Watchdog                                               */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Thread_Start_all ( void )
  { JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/load", NULL );
    if (!api_result) { Info( __func__, "msrv", LOG_ERR, "%s: API Error for /run/thread LOAD",__func__ ); return; }

    if (Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "msrv", LOG_ERR, "%s: API Error for /run/thread LOAD: http_code=%d",__func__, Json_get_int ( api_result, "http_code" ) ); }
    else
     { Json_to_log ( "API /run/thread/load result", "config", api_result );                        /* Print API result to log */
       Json_foreach_array_element ( api_result, "threads", Thread_Start_one_thread_classe_by_array, NULL );
     }
    Json_unref(api_result);
  }
/******************************************************************************************************************************/
/* Demarrer_dls: Processus D.L.S                                                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_dls ( void )
  { Info( __func__, "msrv", LOG_DEBUG, "Demande de demarrage DLS %d", getpid() );
    if ( pthread_create( &Partage->com_dls.TID, NULL, (void *)Run_dls, NULL ) )
     { Info( __func__, "msrv", LOG_ERR, "pthread_create failed" );
       return(FALSE);
     }
    Info( __func__, "msrv", LOG_NOTICE, "thread dls (%p) seems to be running", Partage->com_dls.TID );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Stopper_dls: arret du processus D.L.S                                                                                      */
/* Entré/Sortie: néant                                                                                                        */
/******************************************************************************************************************************/
 void Stopper_dls ( void )
  { Info( __func__, "msrv", LOG_INFO, "Waiting for DLS (%p) to finish", Partage->com_dls.TID );
    Partage->com_dls.Thread_run = FALSE;
    if ( Partage->com_dls.TID ) pthread_join ( Partage->com_dls.TID, NULL );                               /* Attente fin DLS */
    Info( __func__, "msrv", LOG_NOTICE, "ok, DLS is down" );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
