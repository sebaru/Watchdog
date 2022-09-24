/******************************************************************************************************************************/
/* Watchdogd/api_sync.c        Interconnexion avec l'API                                                                      */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    10.06.2022 10:04:24 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * api_sync.c
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
 #include <string.h>
 #include <unistd.h>
 #include <time.h>
 #include <sys/prctl.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* API_handle_API_messages: Traite les messages recue de l'API                                                               */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void API_handle_API_messages ( void )
  {
    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    JsonNode *request = Partage->com_msrv.API_ws_messages->data;
    Partage->com_msrv.API_ws_messages = g_slist_remove ( Partage->com_msrv.API_ws_messages, request );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );

    gchar *agent_tag = Json_get_string ( request, "agent_tag" );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: receive agent_tag '%s' !", __func__, agent_tag );

         if ( !strcasecmp( agent_tag, "RESET") )
     { Partage->com_msrv.Thread_run = FALSE;
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: RESET: Stopping in progress", __func__ );
     }
    else if ( !strcasecmp( agent_tag, "UPGRADE") )
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: UPGRADE: Upgrading in progress", __func__ );
       gint pid = getpid();
       gint new_pid = fork();
       if (new_pid<0)
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s_Fils: UPGRADE: erreur Fork", __func__ ); }
       else if (!new_pid)
        { system("cd SRC; ./autogen.sh; sudo make install; " );
          Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s_Fils: UPGRADE: done. Restarting.", __func__ );
          kill (pid, SIGTERM);                                                                          /* Stop old processes */
          exit(0);
        }
     }
    else if ( !strcasecmp( agent_tag, "THREAD_START") ) { Thread_Start_one_thread ( NULL, 0, request, NULL ); }
    else if ( !strcasecmp( agent_tag, "THREAD_STOP") )  { Thread_Stop_one_thread ( request ); }
    else if ( !strcasecmp( agent_tag, "THREAD_SEND") )  { Thread_Push_API_message ( request ); }
    else if ( !strcasecmp( agent_tag, "AGENT_SET") )
     { if ( !( Json_has_member ( request, "log_bus" ) && Json_has_member ( request, "log_level" ) &&
               Json_has_member ( request, "log_msrv" ) && Json_has_member ( request, "headless" )
             )
          )
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: AGENT_SET: wrong parameters", __func__ );
          goto end;
        }
       Config.log_bus    = Json_get_bool ( request, "log_bus" );
       Config.log_msrv   = Json_get_bool ( request, "log_msrv" );
       gboolean headless = Json_get_bool ( request, "headless" );
       Info_change_log_level ( Config.log, Json_get_int ( request, "log_level" ) );
       Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: AGENT_SET: log_msrv=%d, bus=%d, log_level=%d, headless=%d", __func__,
                 Config.log_msrv, Config.log_bus, Json_get_int ( request, "log_level" ), headless );
       if (Config.headless != headless)
        { Partage->com_msrv.Thread_run = FALSE;
          Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: AGENT_SET: headless has changed, rebooting", __func__ );
        }
     }
    else if ( !strcasecmp( agent_tag, "REMAP") && Config.instance_is_master == TRUE) MSRV_Remap();
    else if ( !strcasecmp( agent_tag, "DLS_COMPIL") && Config.instance_is_master == TRUE)
     { if ( !( Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "codec" ) ) )
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DLS_COMPIL: wrong parameters", __func__ );
          goto end;
        }
       gchar *plugin_tech_id = Json_get_string ( request, "tech_id" );
       Dls_Save_CodeC_to_disk ( plugin_tech_id, Json_get_string ( request, "codec" ) );
       Dls_Reseter_un_plugin ( plugin_tech_id );
     }

end:
    Json_node_unref(request);
  }
/******************************************************************************************************************************/
/* API_on_API_message_CB: Appelé par libsoup lorsque l'on recoit un message sur la websocket connectée à l'API               */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void API_on_API_message_CB ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: WebSocket Message received !", __func__ );
    gsize taille;

    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!request)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: WebSocket Message Dropped (not JSON) !", __func__ );
       return;
     }

    if (!Json_has_member ( request, "agent_tag" ))
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: WebSocket Message Dropped (no 'agent_tag') !", __func__ );
       Json_node_unref(request);
       return;
     }

    pthread_mutex_lock ( &Partage->com_msrv.synchro );                                       /* Ajout dans la liste a traiter */
    Partage->com_msrv.API_ws_messages = g_slist_append ( Partage->com_msrv.API_ws_messages, request );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* API_ws_on_master_close_CB: Traite une deconnexion sur la websocket API                                                   */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void API_on_API_close_CB ( SoupWebsocketConnection *connexion, gpointer user_data )
  { g_object_unref(Partage->com_msrv.API_websocket);
    Partage->com_msrv.API_websocket = NULL;
    Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: WebSocket Close. Reboot Needed!", __func__ );
    /* Partage->com_msrv.Thread_run = FALSE; */
  }
 static void API_on_API_error_CB ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: WebSocket Error received %p!", __func__, self );
  }
/******************************************************************************************************************************/
/* API_ws_on_API_connected: Termine la creation de la connexion websocket API API et raccorde le signal handler             */
/* Entrée: les variables traditionnelles de libsous                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void API_on_API_connected ( GObject *source_object, GAsyncResult *res, gpointer user_data )
  { GError *error = NULL;
    Partage->com_msrv.API_websocket = soup_session_websocket_connect_finish ( Partage->com_msrv.API_session, res, &error );
    if (!Partage->com_msrv.API_websocket)                                                    /* No limit on incoming packet ! */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: WebSocket error: %s.", __func__, error->message );
       g_error_free (error);
       return;
     }
    g_object_set ( G_OBJECT(Partage->com_msrv.API_websocket), "max-incoming-payload-size", G_GINT64_CONSTANT(256000), NULL );
    g_object_set ( G_OBJECT(Partage->com_msrv.API_websocket), "keepalive-interval", G_GINT64_CONSTANT(30), NULL );
    g_signal_connect ( Partage->com_msrv.API_websocket, "message", G_CALLBACK(API_on_API_message_CB), NULL );
    g_signal_connect ( Partage->com_msrv.API_websocket, "closed",  G_CALLBACK(API_on_API_close_CB), NULL );
    g_signal_connect ( Partage->com_msrv.API_websocket, "error",   G_CALLBACK(API_on_API_error_CB), NULL );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: WebSocket to API connected", __func__ );
  }
/******************************************************************************************************************************/
/* API_ws_init: appelé pour démarrer le websocket vers l'API                                                                 */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void API_ws_init ( void )
  { static gchar *protocols[] = { "live-agent", NULL };
    gchar chaine[256];
    g_snprintf(chaine, sizeof(chaine), "wss://%s/websocket", Json_get_string ( Config.config, "api_url" ) );
    SoupMessage *query = soup_message_new ( "GET", chaine );
    Http_Add_Agent_signature ( query, NULL, 0 );

    GCancellable *cancel = g_cancellable_new();
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Starting WebSocket connect to %s", __func__, chaine );
    soup_session_websocket_connect_async ( Partage->com_msrv.API_session, query,
                                           NULL, protocols, cancel, API_on_API_connected, NULL );
    g_object_unref(query);
    g_object_unref(cancel);
  }
/******************************************************************************************************************************/
/* API_ws_end: appelé pour stopper la websocket vers l'API                                                                   */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void API_ws_end ( void )
  { if ( Partage->com_msrv.API_websocket &&
         soup_websocket_connection_get_state ( Partage->com_msrv.API_websocket ) == SOUP_WEBSOCKET_STATE_OPEN )
     { soup_websocket_connection_close ( Partage->com_msrv.API_websocket, 0, "Thanks, Bye !" ); }
    Partage->com_msrv.API_websocket = NULL;
    if (Partage->com_msrv.API_ws_messages)
     { g_slist_foreach ( Partage->com_msrv.API_ws_messages, (GFunc) json_node_unref, NULL );
       g_slist_free ( Partage->com_msrv.API_ws_messages );
       Partage->com_msrv.API_ws_messages = NULL;
     }
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du thread                                                                                        */
/******************************************************************************************************************************/
 void Run_api_sync ( void )
  { prctl(PR_SET_NAME, "W-APISYNC", 0, 0, 0 );

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );

/***************************************** WebSocket Connect to API ************************************************************/
    API_ws_init();
    gint cpt_1_minute = Partage->top + 600;
    while(Partage->com_msrv.Thread_run == TRUE)                                              /* On tourne tant que necessaire */
     {
/*---------------------------------------------- Report des visuels ----------------------------------------------------------*/
       if (Partage->com_msrv.liste_visuel) API_Send_visuels ();                                /* Traitement des I dynamiques */
/*---------------------------------------------- Report des messages ---------------------------------------------------------*/
       if (Partage->com_msrv.liste_msg) API_Send_MSGS();
       if (Partage->com_msrv.API_ws_messages) API_handle_API_messages();

       if (cpt_1_minute < Partage->top)                                                       /* Update DB toutes les minutes */
        { if (Partage->com_msrv.API_websocket == NULL) API_ws_init();                 /* Si websocket closed, try to restart */
          cpt_1_minute += 600;                                                               /* Sauvegarde toutes les minutes */
        }

       usleep(1000);
       sched_yield();
     }
    API_ws_end();

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Down (%p)", __func__, pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
