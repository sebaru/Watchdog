/******************************************************************************************************************************/
/* Watchdogd/Http/Http.c        Gestion des connexions HTTP WebService de watchdog                                            */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                   mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Http.c
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
#warning Delete get_io
       else if (!strcasecmp ( path, "/get_io" ))     Http_traiter_get_io     ( server, msg, path, query );
       else { Http_Send_json_response (msg, SOUP_STATUS_NOT_IMPLEMENTED, NULL, NULL ); return; }
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
    GTlsCertificate *cert = g_tls_certificate_new_from_files (HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY, &error);
    if (error)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Failed to load SSL Certificate '%s' and '%s'. Error '%s'",
                 HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY, error->message  );
       g_error_free(error);
       goto end;
     }
    Partage->com_http.local_socket = soup_server_new( "server-header", "Watchdogd API SSL Server", "tls-certificate", cert, NULL );
    g_object_unref (cert);

    if (!Partage->com_http.local_socket)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "SoupServer new Failed !" );
       goto end;
     }
    Info_new( __func__, Config.log_msrv, LOG_INFO, "SSL Loaded with '%s' and '%s'", HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY );

    soup_server_add_handler ( Partage->com_http.local_socket, "/" , HTTP_Handle_request_CB, NULL, NULL );

    if (!soup_server_listen_all (Partage->com_http.local_socket, HTTP_DEFAUT_TCP_PORT, SOUP_SERVER_LISTEN_HTTPS, &error))
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
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
