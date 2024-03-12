/******************************************************************************************************************************/
/* Watchdogd/http_common.c        Fonctions communes de gestion des requetes HTTP                                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    30.12.2020 22:03:58 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http_common.c
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

 #include <glib.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Mqtt_Send_to_topic: Envoie le node au broker                                                                               */
/* Entrée: la structure MQTT, le topic, le node                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_to_topic ( struct mosquitto *mqtt_session, gchar *topic, gchar *tag, JsonNode *node )
  { gboolean free_node=FALSE;
    if (! (mqtt_session && topic && tag) ) return;
    if (!node) { node = Json_node_create(); free_node = TRUE; }
    Json_node_add_string ( node, "tag", tag );
    gchar *buffer = Json_node_to_string ( node );
    mosquitto_publish(	mqtt_session, NULL, topic, strlen(buffer), buffer, 0, FALSE );
    g_free(buffer);
    if (free_node) Json_node_unref(node);
  }
/******************************************************************************************************************************/
/* Mqtt_Send_AI: Envoie le bit AI au master                                                                                   */
/* Entrée: la structure MQTT, l'AI, la valeur et le range                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_AI ( struct THREAD *module, JsonNode *thread_ai, gdouble valeur, gboolean in_range )
  { if (! (module && thread_ai)) return;
    Json_node_add_double ( thread_ai, "valeur", valeur );
    Json_node_add_bool   ( thread_ai, "in_range", in_range );
    MQTT_Send_to_topic ( module->MQTT_session, "agent/master", "SET_AI", thread_ai );
  }
/******************************************************************************************************************************/
/* MQTT_Send_DI: Envoie le bit DI au master                                                                                   */
/* Entrée: la structure MQTT, la DI, la valeur                                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_DI ( struct THREAD *module, JsonNode *thread_di, gboolean etat )
  { if (! (module && thread_di)) return;
    Json_node_add_bool ( thread_di, "etat", etat );
    MQTT_Send_to_topic ( module->MQTT_session, "agent/master", "SET_DI", thread_di );
  }
/******************************************************************************************************************************/
/* MQTT_Send_DI: Envoie le bit DI au master, au format pulse                                                                  */
/* Entrée: la structure MQTT, la DI, la valeur                                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_DI_pulse ( struct THREAD *module, gchar *tech_id, gchar *acronyme )
  { if (! (module && tech_id && acronyme)) return;
    JsonNode *thread_di = Json_node_create();
    Json_node_add_string ( thread_di, "thread_tech_id", Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_add_string ( thread_di, "tech_id", tech_id );
    Json_node_add_string ( thread_di, "acronyme", acronyme );
    MQTT_Send_to_topic ( module->MQTT_session, "agent/master", "SET_DI_PULSE", thread_di );
    Json_node_unref ( thread_di );
  }
/******************************************************************************************************************************/
/* MQTT_Send_WATCHDOG: Envoie le WATCHDOG au master                                                                           */
/* Entrée: la structure MQTT, le watchdog, la consigne                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_WATCHDOG ( struct THREAD *module, gchar *thread_acronyme, gint consigne )
  { if (! (module && thread_acronyme)) return;
    JsonNode *thread_watchdog = Json_node_create ();
    if(!thread_watchdog) return;
    Json_node_add_string ( thread_watchdog, "thread_tech_id", Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_add_string ( thread_watchdog, "thread_acronyme", thread_acronyme );
    Json_node_add_int    ( thread_watchdog, "consigne", consigne );
    MQTT_Send_to_topic ( module->MQTT_session, "agent/master", "SET_WATCHDOG", thread_watchdog );
    Json_node_unref(thread_watchdog);
  }
/******************************************************************************************************************************/
/* MQTT_Subscribe: souscrit à un topic                                                                                        */
/* Entrée: la structure MQTT, le topic                                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Subscribe ( struct mosquitto *mqtt_session, gchar *topic )
  { if ( mosquitto_subscribe(	mqtt_session, NULL, topic, 0 ) != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, Config.log_bus, LOG_ERR, "Subscribe to topic '%s' FAILED", topic ); }
    else
     { Info_new( __func__, Config.log_bus, LOG_INFO, "Subscribe to topic '%s' OK", topic ); }
  }
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
