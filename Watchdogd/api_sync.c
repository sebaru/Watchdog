/******************************************************************************************************************************/
/* Watchdogd/api_sync.c        Interconnexion avec l'API                                                                      */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    10.06.2022 10:04:24 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * api_sync.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien LEFEVRE
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
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Post_to_global_API ( gchar *URI, JsonNode *RootNode )
  { gboolean unref_RootNode = FALSE;
    gchar query[256];

    g_snprintf( query, sizeof(query), "https://%s%s", Json_get_string ( Config.config, "api_url" ), URI );
/********************************************************* Envoi de la requete ************************************************/
    SoupMessage *soup_msg  = soup_message_new ( "POST", query );
    if (!soup_msg)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Wrong URI Sending to API %s", query );
       return(NULL);
     }

    if (!RootNode) { RootNode = Json_node_create(); unref_RootNode = TRUE; }
    Json_node_add_int ( RootNode, "request_time", time(NULL) );

    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Sending to API %s", query );

    JsonNode *ResponseNode = Http_Send_json_request_from_agent ( soup_msg, RootNode );

    if (unref_RootNode) Json_node_unref(RootNode);

    gchar *reason_phrase = soup_message_get_reason_phrase(soup_msg);
    gint   status_code   = soup_message_get_status(soup_msg);
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "%s Status %d, reason %s", URI, status_code, reason_phrase );

    if (status_code!=200)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "%s Error %d for '%s': %s\n", URI, status_code, query, reason_phrase ); }
    g_object_unref( soup_msg );
    return(ResponseNode);
 }
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Get_from_global_API ( gchar *URI, gchar *format, ... )
  { gchar query[512];
    va_list ap;

    if (format)
     { gchar parametres[128];
       va_start( ap, format );
       g_vsnprintf ( parametres, sizeof(parametres), format, ap );
       va_end ( ap );
       g_snprintf( query, sizeof(query), "https://%s/%s?%s", Json_get_string ( Config.config, "api_url"), URI, parametres );
     }
    else g_snprintf( query, sizeof(query), "https://%s/%s", Json_get_string ( Config.config, "api_url"), URI );
/********************************************************* Envoi de la requete ************************************************/
    SoupMessage *soup_msg  = soup_message_new ( "GET", query );
    if (!soup_msg)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Wrong URI Reading from API %s", query );
       return(NULL);
     }

    JsonNode *ResponseNode = Http_Send_json_request_from_agent ( soup_msg, NULL );

    gchar *reason_phrase = soup_message_get_reason_phrase(soup_msg);
    gint   status_code   = soup_message_get_status(soup_msg);

    gchar nom_fichier[256];
    g_snprintf ( nom_fichier, sizeof(nom_fichier), "cache-%s", query );
    g_strcanon ( nom_fichier+6, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWYYZ", '_' );

    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "%s Status %d, reason %s", URI, status_code, reason_phrase );
    if (status_code!=200)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "%s Error %d for '%s': %s\n", URI, status_code, query, reason_phrase );
       Json_node_unref ( ResponseNode );
       ResponseNode = Json_read_from_file ( nom_fichier );
       if (ResponseNode) Info_new( __func__, Config.log_msrv, LOG_WARNING, "Using cache for %s", query );
     }
    else
     { if (Json_has_member ( ResponseNode, "api_cache" ) && Json_get_bool ( ResponseNode, "api_cache" ) )
        { Json_write_to_file ( nom_fichier, ResponseNode ); }
     }
    g_object_unref( soup_msg );
    return(ResponseNode);
 }
/******************************************************************************************************************************/
/* API_on_API_message_CB: Appelé par libsoup lorsque l'on recoit un message sur la websocket connectée à l'API               */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void API_on_API_message_CB ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { Info_new( __func__, Config.log_msrv, LOG_INFO, "WebSocket Message received !" );
    gsize taille;

    gchar *buffer = g_bytes_get_data ( message_brut, &taille );
    JsonNode *request = Json_get_from_string ( buffer );
    if (!request)
     { if (taille) buffer[taille-1] = 0;
       Info_new( __func__, Config.log_msrv, LOG_WARNING, "WebSocket Message Dropped (not JSON) %s !", buffer );
       return;
     }

    if (!Json_has_member ( request, "agent_tag" ))
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "WebSocket Message Dropped (no 'agent_tag') !" );
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
  { Partage->com_msrv.API_websocket = NULL;
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "WebSocket Close. Reboot Needed!" );
    /* Partage->com_msrv.Thread_run = FALSE; */
  }
 static void API_on_API_error_CB ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { Info_new( __func__, Config.log_msrv, LOG_ERR, "WebSocket Error received %p!", self );
  }
/******************************************************************************************************************************/
/* API_ws_on_API_connected: Termine la creation de la connexion websocket API API et raccorde le signal handler               */
/* Entrée: les variables traditionnelles de libsous                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void API_on_API_connected ( GObject *source_object, GAsyncResult *res, gpointer user_data )
  { GError *error = NULL;
    Partage->com_msrv.API_websocket = soup_session_websocket_connect_finish ( Partage->API_Sync_session, res, &error );
    if (!Partage->com_msrv.API_websocket)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "WebSocket error: %s.", error->message );
       g_error_free (error);
       return;
     }
    soup_websocket_connection_set_keepalive_interval ( Partage->com_msrv.API_websocket, 30 );
    soup_websocket_connection_set_max_incoming_payload_size ( Partage->com_msrv.API_websocket, 2048000 );/* No limit on incoming packet ! */

    g_signal_connect ( Partage->com_msrv.API_websocket, "message", G_CALLBACK(API_on_API_message_CB), NULL );
    g_signal_connect ( Partage->com_msrv.API_websocket, "closed",  G_CALLBACK(API_on_API_close_CB), NULL );
    g_signal_connect ( Partage->com_msrv.API_websocket, "error",   G_CALLBACK(API_on_API_error_CB), NULL );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "WebSocket to API connected" );
  }
/******************************************************************************************************************************/
/* API_ws_init: appelé pour démarrer le websocket vers l'API                                                                 */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void API_ws_init ( void )
  { static gchar *protocols[] = { "live-agent", NULL };
    gchar chaine[256];
    g_snprintf(chaine, sizeof(chaine), "wss://%s/run/websocket", Json_get_string ( Config.config, "api_url" ) );
    SoupMessage *soup_msg = soup_message_new ( "GET", chaine );
    Http_Add_Agent_signature ( soup_msg, NULL, 0 );

    Info_new( __func__, Config.log_msrv, LOG_INFO, "Starting WebSocket connect to %s", chaine );
    soup_session_websocket_connect_async ( Partage->API_Sync_session, soup_msg,
                                           NULL, protocols, 0, NULL, API_on_API_connected, NULL );
    g_object_unref(soup_msg);
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

    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Demarrage . . . TID = %p", pthread_self() );
/***************************************** WebSocket Connect to API ************************************************************/
    Partage->API_Sync_session = HTTP_New_session( "Abls-habitat SYNC Agent" );
    API_ws_init();
    gint cpt_1_minute = Partage->top + 600;
    while(Partage->com_msrv.Thread_run == TRUE)                                              /* On tourne tant que necessaire */
     {
/*---------------------------------------------- Report des visuels ----------------------------------------------------------*/
       if (Partage->com_msrv.liste_visuel) API_Send_visuels ();                                /* Traitement des I dynamiques */
/*---------------------------------------------- Report des messages ---------------------------------------------------------*/
       if (Partage->com_msrv.liste_msg) API_Send_MSGS();
/*---------------------------------------------- Report des abonnements ------------------------------------------------------*/
       if (Partage->abonnements) API_Send_Abonnements();

       if (cpt_1_minute < Partage->top)                                                       /* Update DB toutes les minutes */
        { if (Partage->com_msrv.API_websocket == NULL) API_ws_init();                  /* Si websocket closed, try to restart */
          cpt_1_minute += 600;                                                               /* Sauvegarde toutes les minutes */
        }
       usleep(1000);
       sched_yield();
       if (!Partage->com_msrv.API_websocket) continue;                                   /* Si pas de connexion API, attendre */

/*------------------------------------- Report de tout ce qui a été préparé à l'API ------------------------------------------*/
       if (Partage->liste_json_to_ws_api)
        { JsonNode *RootNode = Partage->liste_json_to_ws_api->data;
          Partage->liste_json_to_ws_api = g_slist_remove ( Partage->liste_json_to_ws_api, RootNode );
          Partage->liste_json_to_ws_api_size--;
          gchar *buf = Json_node_to_string ( RootNode );
          soup_websocket_connection_send_text ( Partage->com_msrv.API_websocket, buf );
          g_free(buf);
          Json_node_unref ( RootNode );
        }
     }
    API_ws_end();
    g_object_unref ( Partage->API_Sync_session );
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Down (%p)", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
