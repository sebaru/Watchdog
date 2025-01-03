/******************************************************************************************************************************/
/* Watchdogd/Http/bus_slave.c       Gestion des request BUS depuis le slave                                                   */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                12.03.2022 09:08:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * bus_slave.c
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

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Http_Check_Thread_signature: Vérifie qu'un message est correctement signé par le thread                                    */
/* Entrée: le messages                                                                                                        */
/* Sortie: TRUE si OK                                                                                                         */
/******************************************************************************************************************************/
 gboolean Http_Check_Thread_signature ( gchar *path, SoupServerMessage *msg, gchar **thread_tech_id_p )
  { SoupMessageHeaders *headers = soup_server_message_get_request_headers ( msg );
    if (!headers)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s': No headers provided. Access Denied.", path );
       Http_Send_json_response ( msg, SOUP_STATUS_UNAUTHORIZED, NULL, NULL );
       return(FALSE);
     }

    gchar *origin      = soup_message_headers_get_one ( headers, "Origin" );
    if (!origin)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, Origin Header is missing", path );
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, NULL, NULL );
       return(FALSE);
     }

    gchar *domain_uuid = soup_message_headers_get_one ( headers, "X-ABLS-DOMAIN" );
    if (!domain_uuid)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, X-ABLS-DOMAIN Header is missing", path );
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, NULL, NULL );
       return(FALSE);
     }

    gchar *agent_uuid  = soup_message_headers_get_one ( headers, "X-ABLS-AGENT" );
    if (!agent_uuid)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, X-ABLS-AGENT Header is missing", path );
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, NULL, NULL );
       return(FALSE);
     }

    gchar *thread_tech_id = *thread_tech_id_p = soup_message_headers_get_one ( headers, "X-ABLS-THREAD-TECH-ID" );
    if (!thread_tech_id)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, X-ABLS-THREAD-TECH-ID Header is missing", path );
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, NULL, NULL );
       return(FALSE);
     }

    gchar *timestamp = soup_message_headers_get_one ( headers, "X-ABLS-TIMESTAMP" );
    if (!timestamp)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, X-ABLS-TIMESTAMP Header is missing", path );
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, NULL, NULL );
       return(FALSE);
     }

    gchar *signature   = soup_message_headers_get_one ( headers, "X-ABLS-SIGNATURE" );
    if (!signature)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Bad Request, X-ABLS-SIGNATURE Header is missing", path );
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, NULL, NULL );
       return(FALSE);
     }

    gsize taille_body;
    SoupMessageBody *body = soup_server_message_get_request_body ( msg );
    GBytes *buffer        = soup_message_body_flatten ( body );
    gchar *request_body   = g_bytes_get_data ( buffer, &taille_body );
    gchar *domain_secret  = Json_get_string ( Config.config, "domain_secret" );

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
    g_bytes_unref(buffer);

    if ( strcmp ( signature, local_signature ) )
     { Info_new( __func__, Config.log_bus, LOG_ERR, "'%s' -> Forbidden, Wrong signature", path );
       Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, NULL, NULL );
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

    SoupMessageHeaders *headers = soup_message_get_request_headers ( msg );
    soup_message_headers_append ( headers, "Origin",                origin );
    soup_message_headers_append ( headers, "X-ABLS-DOMAIN",         domain_uuid );
    soup_message_headers_append ( headers, "X-ABLS-AGENT",          agent_uuid );
    soup_message_headers_append ( headers, "X-ABLS-THREAD-TECH-ID", thread_tech_id );
    soup_message_headers_append ( headers, "X-ABLS-TIMESTAMP",      timestamp );
    soup_message_headers_append ( headers, "X-ABLS-SIGNATURE",      signature );
  }
/******************************************************************************************************************************/
/* Http_Get_from_local_BUS: Envoie un message GET vers le master                                                              */
/* Entrée: le module source, l'URI cible                                                                                      */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 JsonNode *Http_Get_from_local_BUS ( struct THREAD *module, gchar *uri )
  { if (!module) return(NULL);

    gchar query[256];
    g_snprintf( query, sizeof(query), "https://%s:5559/%s", Config.master_hostname, uri );
/********************************************************* Envoi de la requete ************************************************/
    SoupMessage *soup_msg  = soup_message_new ( "GET", query );
    if (!soup_msg)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "MSG Error Sending to %s", query );
       goto end;
     }
    /*g_object_set ( soup_msg, "http-version", SOUP_HTTP_1_0, NULL );*/
    g_signal_connect ( G_OBJECT(soup_msg), "accept-certificate", G_CALLBACK(Http_Accept_certificate), module );
    JsonNode *response = Http_Send_json_request_from_thread ( module, soup_msg, NULL); /* SYNC */

    gchar *reason_phrase = soup_message_get_reason_phrase(soup_msg);
    gint   status_code   = soup_message_get_status(soup_msg);

    Info_new( __func__, Config.log_bus, LOG_DEBUG, "Status %d, reason %s", status_code, reason_phrase );
    if (status_code!=200)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "Error %d for '%s': %s\n", status_code, query, reason_phrase ); }
    g_object_unref( soup_msg );
end:
    return(response);
  }
/******************************************************************************************************************************/
/* Http_traiter_get_io: Donne les IO du thread appelant                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_get_io ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query )
  { gchar *thread_tech_id;
    if (!Http_Check_Thread_signature ( path, msg, &thread_tech_id )) return;
    if (!thread_tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "thread_tech_id missing", NULL ); return; }

    struct DLS_PLUGIN *plugin = Dls_get_plugin_by_tech_id ( thread_tech_id );
    if (!plugin) { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Plugin not found", NULL ); return; }
    JsonNode *Response = Json_node_create();

    JsonArray *DI_array = Json_node_add_array ( Response, "DI" );
    GSList *liste = plugin->Dls_data_DI;
    while (liste)
     { struct DLS_DI *DI = liste->data;
       JsonNode *element = Json_node_create();
       Dls_DI_to_json ( element, DI );
       if (MSRV_Map_to_thread ( element ) && Json_has_member ( element, "thread_tech_id" ) && Json_has_member ( element, "thread_acronyme" ) )
        { Json_array_add_element ( DI_array, element );
        } else Json_node_unref ( element );
       liste = g_slist_next ( liste );
     }

    JsonArray *DO_array = Json_node_add_array ( Response, "DO" );
    liste = plugin->Dls_data_DO;
    while (liste)
     { struct DLS_DO *DO = liste->data;
       JsonNode *element = Json_node_create();
       Dls_DO_to_json ( element, DO );
       if (MSRV_Map_to_thread ( element ) && Json_has_member ( element, "thread_tech_id" ) && Json_has_member ( element, "thread_acronyme" ) )
        { Json_array_add_element ( DO_array, element );
        } else Json_node_unref ( element );
       liste = g_slist_next ( liste );
     }

    JsonArray *AI_array = Json_node_add_array ( Response, "AI" );
    liste = plugin->Dls_data_AI;
    while (liste)
     { struct DLS_AI *AI = liste->data;
       JsonNode *element = Json_node_create();
       Dls_AI_to_json ( element, AI );
       if (MSRV_Map_to_thread ( element ) && Json_has_member ( element, "thread_tech_id" ) && Json_has_member ( element, "thread_acronyme" ) )
        { Json_array_add_element ( AI_array, element );
        } else Json_node_unref ( element );
       liste = g_slist_next ( liste );
     }

    JsonArray *AO_array = Json_node_add_array ( Response, "AO" );
    liste = plugin->Dls_data_AO;
    while (liste)
     { struct DLS_AO *AO = liste->data;
       JsonNode *element = Json_node_create();
       Dls_AO_to_json ( element, AO );
       if (MSRV_Map_to_thread ( element ) && Json_has_member ( element, "thread_tech_id" ) && Json_has_member ( element, "thread_acronyme" ) )
        { Json_array_add_element ( AO_array, element );
        } else Json_node_unref ( element );
       liste = g_slist_next ( liste );
     }

    Info_new( __func__, Config.log_bus, LOG_INFO,  "GET_IO done for '%s'", thread_tech_id );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "There are Outputs", Response );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
