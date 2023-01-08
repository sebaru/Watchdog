/******************************************************************************************************************************/
/* Watchdogd/http_common.c        Fonctions communes de gestion des requetes HTTP                                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    30.12.2020 22:03:58 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http_common.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Msg_to_Json ( SoupMessage *msg )
  { GBytes *request_brute;
    gsize taille;
    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if (!request) { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Not a JSON request"); }
    return(request);
  }
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Response_Msg_to_Json ( SoupMessage *msg )
  { GBytes *reponse_brute;
    gsize taille;
    g_object_get ( msg, "response-body-data", &reponse_brute, NULL );
    JsonNode *reponse = Json_get_from_string ( g_bytes_get_data ( reponse_brute, &taille ) );
    return(reponse);
  }
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 gint Http_Msg_status_code ( SoupMessage *msg )
  { gint status;
    g_object_get ( msg, "status-code", &status, NULL );
    return(status);
  }
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 gchar *Http_Msg_reason_phrase ( SoupMessage *msg )
  { gchar *phrase;
    g_object_get ( msg, "reason-phrase", &phrase, NULL );
    return(phrase);
  }
/******************************************************************************************************************************/
/* Http_Check_Thread_signature: Vérifie qu'un message est correctement signé par le thread                                    */
/* Entrée: le messages                                                                                                        */
/* Sortie: TRUE si OK                                                                                                         */
/******************************************************************************************************************************/
 gboolean Http_Check_Thread_signature ( gchar *path, SoupMessage *msg, gchar **thread_tech_id_p )
  { SoupMessageHeaders *headers;
    g_object_get ( msg, "request-headers", &headers, NULL );
    if (!headers)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s': No headers provided. Access Denied.", path );
       soup_message_set_status ( msg, SOUP_STATUS_UNAUTHORIZED );
       return(FALSE);
     }

    gchar *origin      = soup_message_headers_get_one ( headers, "Origin" );
    if (!origin)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, Origin Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    gchar *domain_uuid = soup_message_headers_get_one ( headers, "X-ABLS-DOMAIN" );
    if (!domain_uuid)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, X-ABLS-DOMAIN Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    gchar *agent_uuid  = soup_message_headers_get_one ( headers, "X-ABLS-AGENT" );
    if (!agent_uuid)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, X-ABLS-AGENT Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    gchar *thread_tech_id = *thread_tech_id_p = soup_message_headers_get_one ( headers, "X-ABLS-THREAD-TECH-ID" );
    if (!thread_tech_id)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, X-ABLS-THREAD-TECH-ID Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    gchar *timestamp = soup_message_headers_get_one ( headers, "X-ABLS-TIMESTAMP" );
    if (!timestamp)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, X-ABLS-TIMESTAMP Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    gchar *signature   = soup_message_headers_get_one ( headers, "X-ABLS-SIGNATURE" );
    if (!signature)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, X-ABLS-SIGNATURE Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    GBytes *gbytes_body;
    gsize taille_body;
    g_object_get ( msg, "request-body-data", &gbytes_body, NULL );
    gchar *request_body  = g_bytes_get_data ( gbytes_body, &taille_body );
    gchar *domain_secret = Json_get_string ( Config.config, "domain_secret" );

    unsigned char hash_bin[EVP_MAX_MD_SIZE];
    gint md_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                                                                   /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, domain_uuid,    strlen(domain_uuid));
    EVP_DigestUpdate(mdctx, agent_uuid,     strlen(agent_uuid));
    EVP_DigestUpdate(mdctx, domain_secret,  strlen(domain_secret));
    EVP_DigestUpdate(mdctx, thread_tech_id, strlen(thread_tech_id));
    EVP_DigestUpdate(mdctx, request_body,   taille_body);
    EVP_DigestUpdate(mdctx, timestamp,      strlen(timestamp));
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);
    gchar local_signature[64];
    EVP_EncodeBlock( local_signature, hash_bin, 32 ); /* 256 bits -> 32 bytes */

    gint retour = strcmp ( signature, local_signature );
    g_bytes_unref(gbytes_body);
    if (retour)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Forbidden, Wrong signature", path );
       soup_message_set_status ( msg, SOUP_STATUS_FORBIDDEN );
       return(FALSE);
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Http_Add_Thread_signature: signe une requete d'un thread vers le master                                                    */
/* Entrée: le message                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Add_Thread_signature ( struct THREAD *module, SoupMessage *msg, gchar *buf, gint buf_size )
  { gchar *origin         = "abls-habitat.fr";
    gchar *domain_uuid    = Json_get_string ( Config.config,  "domain_uuid" );
    gchar *domain_secret  = Json_get_string ( Config.config,  "domain_secret" );
    gchar *agent_uuid     = Json_get_string ( Config.config,  "agent_uuid" );
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar timestamp[20];
    g_snprintf( timestamp, sizeof(timestamp), "%ld", time(NULL) );

    unsigned char hash_bin[EVP_MAX_MD_SIZE];
    gint md_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                                                                   /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, domain_uuid,    strlen(domain_uuid));
    EVP_DigestUpdate(mdctx, agent_uuid,     strlen(agent_uuid));
    EVP_DigestUpdate(mdctx, domain_secret,  strlen(domain_secret));
    EVP_DigestUpdate(mdctx, thread_tech_id, strlen(thread_tech_id));
    if (buf) EVP_DigestUpdate(mdctx, buf,   buf_size);
    EVP_DigestUpdate(mdctx, timestamp,      strlen(timestamp));
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);
    gchar signature[64];
    EVP_EncodeBlock( signature, hash_bin, 32 ); /* 256 bits -> 32 bytes */

    SoupMessageHeaders *headers;
    g_object_get ( msg, "request-headers", &headers, NULL );
    soup_message_headers_append ( headers, "Origin",                origin );
    soup_message_headers_append ( headers, "X-ABLS-DOMAIN",         domain_uuid );
    soup_message_headers_append ( headers, "X-ABLS-AGENT",          agent_uuid );
    soup_message_headers_append ( headers, "X-ABLS-THREAD-TECH-ID", thread_tech_id );
    soup_message_headers_append ( headers, "X-ABLS-TIMESTAMP",      timestamp );
    soup_message_headers_append ( headers, "X-ABLS-SIGNATURE",      signature );
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

    SoupMessageHeaders *headers;
    g_object_get ( msg, "request-headers", &headers, NULL );
    soup_message_headers_append ( headers, "Origin",           origin );
    soup_message_headers_append ( headers, "X-ABLS-DOMAIN",    domain_uuid );
    soup_message_headers_append ( headers, "X-ABLS-AGENT",     agent_uuid );
    soup_message_headers_append ( headers, "X-ABLS-TIMESTAMP", timestamp );
    soup_message_headers_append ( headers, "X-ABLS-SIGNATURE", signature );
  }
/******************************************************************************************************************************/
/* Http_Send_json_response: Envoie le json en paramètre en prenant le lead dessus                                             */
/* Entrée: le messages, le buffer json                                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Send_json_response ( SoupMessage *msg, gint code, gchar *message, JsonNode *RootNode )
  { if (!RootNode)
     { if ( (RootNode = Json_node_create() ) == NULL ) return; }

    gchar *buf = Json_node_to_string ( RootNode );
    Json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status_full (msg, code, message);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
