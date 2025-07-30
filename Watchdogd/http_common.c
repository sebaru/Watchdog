/******************************************************************************************************************************/
/* Watchdogd/http_common.c        Fonctions communes de gestion des requetes HTTP                                             */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                30.12.2020 22:03:58 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http_common.c
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

 #include <glib.h>
 #include <stdio.h>
 #include <curl/curl.h>

 #include "watchdogd.h"

 struct HTTP_BUFFER
  { gchar *body;
    size_t size;
  };

/******************************************************************************************************************************/
/* Http_Init: Initialise la librairie HTTP                                                                                    */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Init ( void )
  { curl_global_init(CURL_GLOBAL_DEFAULT);
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "lib cURL initialized" );
  }
/******************************************************************************************************************************/
/* Http_End: Désactive la librairie HTTP                                                                                      */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_End ( void )
  { curl_global_cleanup();
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "lib cURL un-initialized" );
  }
/******************************************************************************************************************************/
/* Http_Write_CB: Fonction d'écriture du callback                                                                             */
/* Entrée: le contenu, la taille, le nombre de bytes, de user data                                                            */
/* Sortie: la nouvelle taille                                                                                                 */
/******************************************************************************************************************************/
 static size_t Http_Write_CB ( void *contents, size_t size, size_t nmemb, void *userp )
  { struct HTTP_BUFFER *buffer = userp;
    size_t chunksize = size * nmemb;

    char *ptr = g_try_realloc( buffer->body, buffer->size + chunksize + 1 );
    if(!ptr)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Realloc failed" ); return(0); }

    buffer->body = ptr;
    memcpy( buffer->body + buffer->size, contents, chunksize );
    buffer->size += chunksize;
    buffer->body[buffer->size] = 0;
    return(chunksize);
  }
/******************************************************************************************************************************/
/* Http_Post: Réalise une requete POST vers un site distant                                                                   */
/* Entrée: L'url, le payload                                                                                                  */
/* Sortie: la reponse json                                                                                                    */
/******************************************************************************************************************************/
 JsonNode *Http_Request ( gchar *url, JsonNode *json_payload )
  { struct curl_slist *headers = NULL;                                                /* Set the appropriate headers for JSON */
    JsonNode *ResponseNode = NULL;
    gchar *payload = NULL;
    gint http_code = 0;                                                                                     /* Code de retour */

    if (!url) return(NULL);
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Request to %s is starting", url );
/*------------------------------------------------ Init du cURL --------------------------------------------------------------*/
    CURL *curl = curl_easy_init();
    if(!curl) { Info_new( __func__, Config.log_msrv, LOG_ERR, "Request to %s: Curl_easy_init failed", url ); return(NULL); }

/*------------------------------------------------ Préparation du payload ----------------------------------------------------*/
    if (json_payload)
     { payload = Json_node_to_string ( json_payload );
       if (!payload) { Info_new( __func__, Config.log_msrv, LOG_ERR, "Request to %s: Json to string failed", url ); goto end; }
     }

/*------------------------------------------------ Préparation du cURL -------------------------------------------------------*/
    struct HTTP_BUFFER *buffer = g_try_malloc0( sizeof(struct HTTP_BUFFER) );
    if (!buffer) { Info_new( __func__, Config.log_msrv, LOG_ERR, "Request to %s: Malloc buffer failed", url ); goto end; }

    curl_easy_setopt( curl, CURLOPT_URL, url );
    if (payload)
     { curl_easy_setopt( curl, CURLOPT_POST, 1L);
       curl_easy_setopt( curl, CURLOPT_POSTFIELDS, payload);
     }
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, Http_Write_CB );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, buffer );

    gchar *domain_uuid   = Json_get_string ( Config.config, "domain_uuid" );
    gchar *domain_secret = Json_get_string ( Config.config, "domain_secret" );
    gchar *agent_uuid    = Json_get_string ( Config.config, "agent_uuid" );

/*------------------------------------------------ Préparation des headers ---------------------------------------------------*/
    gchar timestamp[20];
    g_snprintf( timestamp, sizeof(timestamp), "%ld", time(NULL) );

    headers = curl_slist_append( headers, "Content-Type: application/json" );
    gchar chaine[256];
    g_snprintf( chaine, sizeof(chaine), "Origin: %s", "abls-habitat.fr" );
    headers = curl_slist_append( headers, chaine );

    g_snprintf( chaine, sizeof(chaine), "X-ABLS-DOMAIN: %s", domain_uuid );
    headers = curl_slist_append( headers, chaine );

    g_snprintf( chaine, sizeof(chaine), "X-ABLS-AGENT: %s", agent_uuid );
    headers = curl_slist_append( headers, chaine );

    g_snprintf( chaine, sizeof(chaine), "X-ABLS-TIMESTAMP: %s", timestamp );
    headers = curl_slist_append( headers, chaine );

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

/*---------------------------------------------- Calcul de la signature ------------------------------------------------------*/
    unsigned char hash_bin[EVP_MAX_MD_SIZE];
    gint md_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                                                                   /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, domain_uuid,   strlen(domain_uuid));
    EVP_DigestUpdate(mdctx, agent_uuid,    strlen(agent_uuid));
    EVP_DigestUpdate(mdctx, domain_secret, strlen(domain_secret));
    if (payload) EVP_DigestUpdate(mdctx, payload, strlen(payload));
    EVP_DigestUpdate(mdctx, timestamp,     strlen(timestamp));
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);
    gchar signature[64];
    EVP_EncodeBlock( signature, hash_bin, 32 ); /* 256 bits -> 32 bytes */

    g_snprintf( chaine, sizeof(chaine), "X-ABLS-SIGNATURE: %s", signature );
    headers = curl_slist_append( headers, chaine );

/*------------------------------------------- Réalisation de la requete ------------------------------------------------------*/
    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);                               /* Recupération du code retour */
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Request to %s: HttpCode = %d", url, http_code );

    if( res != CURLE_OK )
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Request to %s: Curl_easy_perform failed: %s", url, curl_easy_strerror(res) ); }
    else if (http_code == 200)
     { if (buffer->body)
        { ResponseNode = Json_get_from_string ( buffer->body );
          if (!ResponseNode) { Info_new( __func__, Config.log_msrv, LOG_ERR, "Request to %s: Response is not json", url ); }
        }
       else Info_new( __func__, Config.log_msrv, LOG_ERR, "No body received" );
     }
    else Info_new( __func__, Config.log_msrv, LOG_ERR, "Request to %s: HttpCode = %d", url, http_code );

    if (!ResponseNode) ResponseNode = Json_node_create ();                       /* Si pas de body en response, on en créé un */
    if (ResponseNode)
     { Json_node_add_int ( ResponseNode, "http_code", http_code ); }
    else Info_new( __func__, Config.log_msrv, LOG_ERR, "Request to %s: ResponseNode Error", url );

end:
    if (headers) curl_slist_free_all(headers);                                                         /* Cleanup the headers */
    if (buffer)  g_free(buffer);
    if (payload) g_free(payload);
    curl_easy_cleanup(curl);
    return(ResponseNode);
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
/* Http_Send_json_request_from_agent: Envoie une requete sur la connexion et attend la reponse                                */
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
       Info_new( __func__, Config.log_msrv, LOG_ERR, "Error %d for '%s': %s", status_code, uri, reason_phrase );
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
       Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Sending %s", buffer );
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
       Info_new( __func__, Config.log_msrv, LOG_ERR, "Error %d for '%s': %s", status_code, uri, reason_phrase );
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
    if (!RootNode) { RootNode = Json_node_create(); unref_RootNode = TRUE; }
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Sending to API %s", query );

    JsonNode *ResponseNode = Http_Request ( query, RootNode );
    if (unref_RootNode) Json_node_unref(RootNode);

    gint http_code = Json_get_int ( ResponseNode, "http_code" );
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "%s: Status %d", query, http_code );

    if (http_code!=200)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "%s Error %d", query, http_code ); }
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
    JsonNode *ResponseNode = Http_Request ( query, NULL );
    if (!ResponseNode) { Info_new( __func__, Config.log_msrv, LOG_ERR, "Error with Http_Get %s", query ); return(NULL); }
    gint http_code = Json_get_int ( ResponseNode, "http_code" );
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "%s Status %d for '%s'", URI, http_code, query );

    gchar nom_fichier[256];
    g_snprintf ( nom_fichier, sizeof(nom_fichier), "cache-%s", query );
    g_strcanon ( nom_fichier+6, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWYYZ", '_' );

    if (http_code!=200)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "%s Error %d for '%s'", URI, http_code, query );
       Json_node_unref ( ResponseNode );
       ResponseNode = Json_read_from_file ( nom_fichier );
       if (ResponseNode) Info_new( __func__, Config.log_msrv, LOG_WARNING, "Using cache for %s", query );
     }
    else
     { if (Json_has_member ( ResponseNode, "api_cache" ) && Json_get_bool ( ResponseNode, "api_cache" ) )
        { Json_write_to_file ( nom_fichier, ResponseNode ); }
     }
    return(ResponseNode);
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
