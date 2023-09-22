/******************************************************************************************************************************/
/* Watchdogd/Http/Http.c        Gestion des connexions HTTP WebService de watchdog                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Http.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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

 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <string.h>
 #include <crypt.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"
 #include "Http.h"

#warning a migrer coté API
#ifdef bouh
    gpointer search_string = g_hash_table_lookup ( query, "search[value]" );
    if (!search_string) { search = g_strdup (""); }
                   else { search = Normaliser_chaine ( search_string ); }
    gchar *draw_string = g_hash_table_lookup ( query, "draw" );
    if (draw_string) Json_node_add_int ( RootNode, "draw", atoi(draw_string) );
                else Json_node_add_int ( RootNode, "draw", 1 );
    gchar *start_string = g_hash_table_lookup ( query, "start" );
    if (start_string) start = atoi(start_string);
                 else start = 200;
    gchar *length_string = g_hash_table_lookup ( query, "length" );
    if (length_string) length = atoi(length_string);
                  else length = 200;


    "SELECT COUNT(*) AS recordsTotal FROM dictionnaire LIMIT %d", length )==FALSE)
                                  "SELECT COUNT(*) AS recordsFiltered FROM dictionnaire "
                                  "WHERE tech_id LIKE '%%%s%%' OR acronyme LIKE '%%%s%%' OR libelle LIKE '%%%s%%' "
                                  "LIMIT %d OFFSET %d",
                                  search, search, search, length, start )==FALSE)
    if (SQL_Select_to_json_node ( RootNode, "data",
                                  "SELECT * FROM dictionnaire "
                                  "WHERE tech_id LIKE '%%%s%%' OR acronyme LIKE '%%%s%%' OR libelle LIKE '%%%s%%' "
                                  "LIMIT %d OFFSET %d",
                                  search, search, search, length, start )==FALSE)

#endif
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Msg_to_Json ( SoupServerMessage *msg )
  { gsize taille;

    SoupMessageBody *body = soup_server_message_get_request_body ( msg );
    GBytes *buffer        = soup_message_body_flatten ( body );                                    /* Add \0 to end of buffer */
    JsonNode *request     = Json_get_from_string ( g_bytes_get_data ( buffer, &taille ) );
    g_bytes_unref(buffer);
    return(request);
  }
/******************************************************************************************************************************/
/* HTTP_Handle_request: Repond aux requests reçues                                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void HTTP_Handle_request_CB ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data )
  { SoupMessageHeaders *headers = soup_server_message_get_response_headers ( msg );
    soup_message_headers_append ( headers, "Cache-Control", "no-store, must-revalidate" );
    soup_message_headers_append ( headers, "Access-Control-Allow-Origin", "*" );
    soup_message_headers_append ( headers, "Access-Control-Allow-Methods", "*" );
    soup_message_headers_append ( headers, "Access-Control-Allow-Headers", "content-type, authorization, X-ABLS-DOMAIN" );
/*---------------------------------------------------- OPTIONS ---------------------------------------------------------------*/
    if (soup_server_message_get_method(msg) == SOUP_METHOD_OPTIONS)
     { soup_message_headers_append ( headers, "Access-Control-Max-Age", "86400" );
       Http_Send_json_response (msg, SOUP_STATUS_OK, NULL, NULL );
       return;
     }
/*------------------------------------------------------ GET -----------------------------------------------------------------*/

    if (soup_server_message_get_method(msg) == SOUP_METHOD_GET)
     {      if (!strcasecmp ( path, "/" ))           Http_traiter_status     ( server, msg, path, query, user_data );
       else if (!strcasecmp ( path, "/status" ))     Http_traiter_status     ( server, msg, path, query, user_data );
       else if (!strcasecmp ( path, "/dls/status" )) Http_traiter_dls_status ( server, msg, path, query, user_data );
       else if (!strcasecmp ( path, "/dls/run" ))    Http_traiter_dls_run    ( server, msg, path, query, user_data );
       else if (!strcasecmp ( path, "/get_io" ))     Http_traiter_get_io     ( server, msg, path, query );
       else { Http_Send_json_response (msg, SOUP_STATUS_NOT_IMPLEMENTED, NULL, NULL ); return; }
     }
    else if (soup_server_message_get_method(msg) == SOUP_METHOD_POST)
     { JsonNode *request = Http_Msg_to_Json ( msg );
       if (!request)
        { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Parsing Request Failed", NULL );
          return;
        }
            if (!strcasecmp ( path, "/dls/run/set" ))      Http_traiter_dls_run_set       ( server, msg, path, request );
       else if (!strcasecmp ( path, "/dls/run/acquitter")) Http_traiter_dls_run_acquitter ( server, msg, path, request );
       else if (!strcasecmp ( path, "/set_di"))            Http_traiter_set_di_post       ( server, msg, path, request );
       else if (!strcasecmp ( path, "/set_ai"))            Http_traiter_set_ai_post       ( server, msg, path, request );
       else if (!strcasecmp ( path, "/set_cde"))           Http_traiter_set_cde_post      ( server, msg, path, request );
       else if (!strcasecmp ( path, "/set_watchdog"))      Http_traiter_set_watchdog_post ( server, msg, path, request );
       else Http_Send_json_response (msg, SOUP_STATUS_NOT_FOUND, "Not found", NULL );
       Json_node_unref ( request );
     }
    else { Http_Send_json_response (msg, SOUP_STATUS_NOT_IMPLEMENTED, "Method not implemented", NULL ); return; }
  }
/******************************************************************************************************************************/
/* Run_HTTP: Thread principal                                                                                                 */
/* Entrée: une structure PROCESS                                                                                              */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_HTTP ( void )
  { GError *error = NULL;

    prctl(PR_SET_NAME, "W-HTTP", 0, 0, 0 );
    Partage->com_http.Thread_run = TRUE;                                                                /* Le thread tourne ! */
    Info_new( __func__, Partage->com_http.Thread_debug, LOG_NOTICE, "Demarrage . . . TID = %p", pthread_self() );
/********************************************* New API ************************************************************************/
    if (Config.bus_is_ssl)
     { GTlsCertificate *cert = g_tls_certificate_new_from_files (HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY, &error);
       if (error)
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "Failed to load SSL Certificate '%s' and '%s'. Error '%s'",
                    HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY, error->message  );
          g_error_free(error);
          goto end;
        }
       Partage->com_http.local_socket = soup_server_new( "server-header", "Watchdogd API SSL Server", "tls-certificate", cert, NULL );
       g_object_unref (cert);
     }
    else Partage->com_http.local_socket = soup_server_new( "server-header", "Watchdogd API Server", NULL );

    if (!Partage->com_http.local_socket)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "SoupServer new Failed !" );
       goto end;
     }
    Info_new( __func__, Config.log_msrv, LOG_INFO, "SSL Loaded with '%s' and '%s'", HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY );

    soup_server_add_handler ( Partage->com_http.local_socket, "/" , HTTP_Handle_request_CB, NULL, NULL );

    if (Config.instance_is_master)
     { static gchar *protocols[] = { "live-bus", NULL };
       soup_server_add_websocket_handler ( Partage->com_http.local_socket, "/ws_bus" , NULL, protocols, Http_traiter_open_websocket_for_slaves_CB, NULL, NULL );
     }

    if (!soup_server_listen_all (Partage->com_http.local_socket, HTTP_DEFAUT_TCP_PORT, (Config.bus_is_ssl ? SOUP_SERVER_LISTEN_HTTPS : 0), &error))
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "SoupServer Listen Failed '%s' !", error->message );
       g_error_free(error);
       goto end_socket;
     }
    Info_new( __func__, Config.log_msrv, LOG_INFO, "HTTP SoupServer SSL Listen OK on port %d !", HTTP_DEFAUT_TCP_PORT );

    Partage->com_http.loop = g_main_loop_new (NULL, TRUE);
    while(Partage->com_http.Thread_run == TRUE)
     { sched_yield();
       usleep(1000);
       g_main_context_iteration ( g_main_loop_get_context ( Partage->com_http.loop ), FALSE );
     }

end_socket:
    if (Partage->com_http.local_socket)
     { soup_server_disconnect ( Partage->com_http.local_socket );
       g_object_unref(Partage->com_http.local_socket);
     }

    if (Partage->com_http.loop)
     { g_main_loop_quit ( Partage->com_http.loop );
       g_main_loop_unref( Partage->com_http.loop );
     }

end:
    Info_new( __func__, Partage->com_http.Thread_debug, LOG_NOTICE, "HTTP Down (%p)", pthread_self() );
    Partage->com_http.TID = 0;                                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
