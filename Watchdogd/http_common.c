/******************************************************************************************************************************/
/* Watchdogd/http_common.c        Fonctions communes de gestion des requetes HTTP                                             */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                30.12.2020 22:03:58 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http_common.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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

 #include "watchdogd.h"


/******************************************************************************************************************************/
/* HTTP_New_session: créé une nouvelle session libsoup                                                                        */
/* Entrée: le message                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 SoupSession *HTTP_New_session ( gchar *user_agent )
  { SoupSession *session = soup_session_new ();
    soup_session_set_user_agent   ( session, user_agent );
    soup_session_set_timeout      ( session, 60 );
    soup_session_set_idle_timeout ( session, 60 );
    return(session);
  }
/******************************************************************************************************************************/
/* Http_Add_Agent_signature: signe une requete d'un agent vers l'API Cloud                                                    */
/* Entrée: le message                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Add_Agent_signature ( SoupMessage *msg, gchar *buf, gint buf_size )
  { gchar *origin        = "abls-habitat.fr";
    gchar *domain_uuid   = Json_get_string ( Config.config, "domain_uuid" );
    gchar *domain_secret = Json_get_string ( Config.config, "domain_secret" );
    gchar *agent_uuid    = Json_get_string ( Config.config, "agent_uuid" );
    gchar timestamp[20];
    g_snprintf( timestamp, sizeof(timestamp), "%ld", time(NULL) );

    unsigned char hash_bin[EVP_MAX_MD_SIZE];
    gint md_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                                                                   /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, domain_uuid,   strlen(domain_uuid));
    EVP_DigestUpdate(mdctx, agent_uuid,    strlen(agent_uuid));
    EVP_DigestUpdate(mdctx, domain_secret, strlen(domain_secret));
    if (buf) EVP_DigestUpdate(mdctx, buf,  buf_size);
    EVP_DigestUpdate(mdctx, timestamp,     strlen(timestamp));
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);
    gchar signature[64];
    EVP_EncodeBlock( signature, hash_bin, 32 ); /* 256 bits -> 32 bytes */

    SoupMessageHeaders *headers = soup_message_get_request_headers ( msg );
    soup_message_headers_append ( headers, "Origin",           origin );
    soup_message_headers_append ( headers, "X-ABLS-DOMAIN",    domain_uuid );
    soup_message_headers_append ( headers, "X-ABLS-AGENT",     agent_uuid );
    soup_message_headers_append ( headers, "X-ABLS-TIMESTAMP", timestamp );
    soup_message_headers_append ( headers, "X-ABLS-SIGNATURE", signature );
  }
/******************************************************************************************************************************/
/* Http_Send_json_request_to_API: Envoie une requete sur la connexion et attend la reponse                                     */
/* Entrée: les données envoyer                                                                                                */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Send_json_request_from_agent ( SoupMessage *soup_msg, JsonNode *RootNode )
  {
    if (RootNode)
     { gchar *buffer = Json_node_to_string ( RootNode );
       gint   taille = strlen(buffer);
       Http_Add_Agent_signature ( soup_msg, buffer, taille );
       GBytes *body  = g_bytes_new_take ( buffer, taille );
       soup_message_set_request_body_from_bytes ( soup_msg, "application/json; charset=UTF-8", body );
       g_bytes_unref ( body );
     } else Http_Add_Agent_signature ( soup_msg, NULL, 0 );

    GError *error = NULL;
    SoupSession *session = HTTP_New_session ( "Abls-Habitat Agent" );
    GBytes *response = soup_session_send_and_read ( session, soup_msg, NULL, &error ); /* SYNC */

    gchar *reason_phrase = soup_message_get_reason_phrase(soup_msg);
    gint   status_code   = soup_message_get_status(soup_msg);

    JsonNode *ResponseNode = NULL;
    if (error)
     { gchar *uri = g_uri_to_string(soup_message_get_uri(soup_msg));
       Info_new( __func__, Config.log_msrv, LOG_ERR, "%s: Error '%s'", uri, error->message );
       g_free(uri);
       g_error_free ( error );
     }
    else if (status_code==200)
     { gsize taille;
       gchar *buffer_unsafe = g_bytes_get_data ( response, &taille );
       gchar *buffer_safe   = g_try_malloc0 ( taille + 1 );
       if (buffer_safe)
        { memcpy ( buffer_safe, buffer_unsafe, taille );                                        /* Copy with \0 end of string */
          if (taille) ResponseNode = Json_get_from_string ( buffer_safe );
          g_free(buffer_safe);
        }
     }
    else
     { gchar *uri = g_uri_to_string(soup_message_get_uri(soup_msg));
       Info_new( __func__, Config.log_bus, LOG_ERR, "Error %d for '%s': %s", status_code, uri, reason_phrase );
       g_free(uri);
     }
    g_bytes_unref (response);
    g_object_unref ( session );
    return(ResponseNode);
  }
/******************************************************************************************************************************/
/* Http_Send_json_request_from_thread: Envoie une requete sur la connexion et attend la reponse                               */
/* Entrée: les données envoyer                                                                                                */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Send_json_request_from_thread ( struct THREAD *module, SoupMessage *soup_msg, JsonNode *RootNode )
  {
    if (RootNode)
     { gchar *buffer = Json_node_to_string ( RootNode );
       gint   taille = strlen(buffer);
       Http_Add_Thread_signature ( module, soup_msg, buffer, taille );
       GBytes *body  = g_bytes_new_take ( buffer, taille );
       Info_new( __func__, Config.log_bus, LOG_DEBUG, "Sending %s", buffer );
       soup_message_set_request_body_from_bytes ( soup_msg, "application/json; charset=UTF-8", body );
       g_bytes_unref ( body );
     } else Http_Add_Thread_signature ( module, soup_msg, NULL, 0 );

    GError *error = NULL;
    SoupSession *session = HTTP_New_session ( "Abls-Habitat Thread" );
    GBytes *response = soup_session_send_and_read ( module->Soup_session, soup_msg, NULL, &error ); /* SYNC */

    gchar *reason_phrase = soup_message_get_reason_phrase(soup_msg);
    gint   status_code   = soup_message_get_status(soup_msg);

    JsonNode *ResponseNode = NULL;
    if (error)
     { gchar *uri = g_uri_to_string(soup_message_get_uri(soup_msg));
       Info_new( __func__, Config.log_msrv, LOG_ERR, "%s: Error '%s'", uri, error->message );
       g_free(uri);
       g_error_free ( error );
     }
    else if (status_code==200)
     { gsize taille;
       gchar *buffer_unsafe = g_bytes_get_data ( response, &taille );
       gchar *buffer_safe   = g_try_malloc0 ( taille + 1 );
       if (buffer_safe)
        { memcpy ( buffer_safe, buffer_unsafe, taille );                                        /* Copy with \0 end of string */
          if (taille) ResponseNode = Json_get_from_string ( buffer_safe );
          g_free(buffer_safe);
        }
     }
    else
     { gchar *uri = g_uri_to_string(soup_message_get_uri(soup_msg));
       Info_new( __func__, Config.log_bus, LOG_ERR, "Error %d for '%s': %s", status_code, uri, reason_phrase );
       g_free(uri);
     }
    g_bytes_unref (response);
    g_object_unref ( session );
    return(ResponseNode);
  }
/******************************************************************************************************************************/
/* Http_Send_json_response: Envoie le json en paramètre en prenant le lead dessus                                             */
/* Entrée: le message, le buffer json, le code retour                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Send_json_response ( SoupServerMessage *msg, gint code, gchar *message, JsonNode *RootNode )
  { if (!RootNode) soup_server_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_STATIC, NULL, 0 );
    else
     { gchar *buf = Json_node_to_string ( RootNode );
       soup_server_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
       Json_node_unref ( RootNode );
     }
    soup_server_message_set_status( msg, code, message );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
