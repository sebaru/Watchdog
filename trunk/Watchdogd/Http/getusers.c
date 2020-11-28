/******************************************************************************************************************************/
/* Watchdogd/Http/getusers.c       Gestion des request getusers pour le thread HTTP de watchdog                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    11.07.2020 15:24:31 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getusers.c
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

 #include <string.h>
 #include <unistd.h>
 #include <openssl/rand.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;

/******************************************************************************************************************************/
/* Http_Traiter_mnemos_set: Modifie la config d'un mnemonique                                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_users_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille;

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( ! (Json_has_member ( request, "username" ) ) )
     { if (request) json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar chaine[512], critere[256];
    g_snprintf ( chaine, sizeof(chaine), "UPDATE users SET date_modif=NOW()" );

    if ( Json_has_member ( request, "access_level" ) )
     { g_snprintf( critere, sizeof(critere), ", access_level=%d", Json_get_int ( request, "access_level" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "enable" ) )
     { g_snprintf( critere, sizeof(critere), ", enable=%d", Json_get_bool ( request, "enable" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "notification" ) )
     { g_snprintf( critere, sizeof(critere), ", notification=%d", Json_get_bool ( request, "notification" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "allow_cde" ) )
     { g_snprintf( critere, sizeof(critere), ", allow_cde=%d", Json_get_bool ( request, "allow_cde" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "email" ) )
     { gchar *email = Normaliser_chaine ( Json_get_string ( request, "email" ) );
       g_snprintf( critere, sizeof(critere), ", email='%s'", email );
       g_free(email);
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "xmpp" ) )
     { gchar *xmpp = Normaliser_chaine ( Json_get_string ( request, "xmpp" ) );
       g_snprintf( critere, sizeof(critere), ", xmpp='%s'", xmpp );
       g_free(xmpp);
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "phone" ) )
     { gchar *phone = Normaliser_chaine ( Json_get_string ( request, "phone" ) );
       g_snprintf( critere, sizeof(critere), ", phone='%s'", phone );
       g_free(phone);
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "comment" ) )
     { gchar *comment = Normaliser_chaine ( Json_get_string ( request, "comment" ) );
       g_snprintf( critere, sizeof(critere), ", comment='%s'", comment );
       g_free(comment);
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    gchar *username = Normaliser_chaine ( Json_get_string ( request, "username" ) );

    if ( Json_has_member ( request, "password" ) && !strcmp(username, session->username) )
     { guchar salt[128], salt_bin[17];
       memset ( salt, 0, sizeof(salt) );
       RAND_bytes ( salt_bin, 16 );
       for (gint i=0; i<16; i++)
        { gchar temp[3];
          g_snprintf(temp, sizeof(temp), "%02x", salt_bin[i] );
          g_strlcat( salt, temp, sizeof(salt) );
        }

       guchar hash[EVP_MAX_MD_SIZE*2+1], hash_bin[EVP_MAX_MD_SIZE];
       memset ( hash, 0, sizeof(hash) );
       gint md_len;
       EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                                                                               /* Calcul du SHA1 */
       EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
       EVP_DigestUpdate(mdctx, salt, strlen(salt));
       EVP_DigestUpdate(mdctx, Json_get_string ( request, "password" ), strlen(Json_get_string ( request, "password" )));
       EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
       EVP_MD_CTX_free(mdctx);
       for (gint i=0; i<md_len; i++)
        { gchar temp[3];
          g_snprintf(temp, sizeof(temp), "%02x", hash_bin[i] );
          g_strlcat( hash, temp, sizeof(hash) );
        }

       g_snprintf( critere, sizeof(critere), ", salt='%s', hash='%s'", salt, hash );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    g_snprintf( critere, sizeof(critere), " WHERE username='%s'", username );
    g_strlcat ( chaine, critere, sizeof(chaine) );

    if (strcmp(username, session->username))
     { g_snprintf( critere, sizeof(critere), " AND access_level<%d", session->access_level );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }
    g_free(username);

    if (SQL_Write ( chaine ))
         { soup_message_set_status ( msg, SOUP_STATUS_OK ); }
    else { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" ); }
/*************************************************** Envoi au client **********************************************************/
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_mnemos_set: Modifie la config d'un mnemonique                                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_users_add ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille;
    gchar chaine[256];

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( ! (Json_has_member ( request, "username" ) && Json_has_member ( request, "email" ) &&
            Json_has_member ( request, "password" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    guchar salt[128], salt_bin[17];
    memset ( salt, 0, sizeof(salt) );
    RAND_bytes ( salt_bin, 16 );
    for (gint i=0; i<16; i++)
     { g_snprintf(chaine, sizeof(chaine), "%02x", salt_bin[i] );
       g_strlcat( salt, chaine, sizeof(salt) );
     }

    guchar hash[EVP_MAX_MD_SIZE*2+1], hash_bin[EVP_MAX_MD_SIZE];
    memset ( hash, 0, sizeof(hash) );
    gint md_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                                                                               /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, salt, strlen(salt));
    EVP_DigestUpdate(mdctx, Json_get_string ( request, "password" ), strlen(Json_get_string ( request, "password" )));
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);
    for (gint i=0; i<md_len; i++)
     { g_snprintf(chaine, sizeof(chaine), "%02x", hash_bin[i] );
       g_strlcat( hash, chaine, sizeof(hash) );
     }

    gchar *username = Normaliser_chaine ( Json_get_string ( request, "username" ) );
    gchar *email    = Normaliser_chaine ( Json_get_string ( request, "email" ) );
    g_snprintf ( chaine, sizeof(chaine), "INSERT INTO users SET username='%s', email='%s', salt='%s', hash='%s'",
                 username, email, salt, hash );
    if (SQL_Write ( chaine ))
         { soup_message_set_status ( msg, SOUP_STATUS_OK );
           Audit_log ( session, "User '%s' ('%s') added", username, email );
           Send_mail ( "Bienvenu chez vous !", email, "ceci est un test !" );
         }
    else { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" ); }
    g_free(username);
    g_free(email);
/*************************************************** Envoi au client **********************************************************/
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_mnemos_set: Modifie la config d'un mnemonique                                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_users_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille;

    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( ! (Json_has_member ( request, "username" ) ) )
     { if (request) json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar chaine[256];
    gchar *username = Normaliser_chaine ( Json_get_string ( request, "username" ) );
    g_snprintf ( chaine, sizeof(chaine), "DELETE FROM users WHERE username='%s' AND access_level<%d", username, session->access_level );
    if (SQL_Write ( chaine ))
         { soup_message_set_status ( msg, SOUP_STATUS_OK );
           Audit_log ( session, "User '%s' deleted", username );
         }
    else { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" ); }
    g_free(username);
/*************************************************** Envoi au client **********************************************************/
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_users_kill: Kill une session utilisateur                                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 void Http_traiter_users_kill ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { JsonObject *object;
    GBytes *request;
    JsonNode *Query;
    gchar * data;
    gsize taille;

    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;

    g_object_get ( msg, "request-body-data", &request, NULL );
    if (!request)
     { soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }

    data = g_bytes_unref_to_data ( request, &taille );                     /* Récupération du buffer et ajout d'un \0 d'arret */
    data = g_try_realloc( data, taille + 1 );
    data [taille] = 0;
    Query = json_from_string ( data, NULL );
    if (!Query)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: requete non Json '%s'", __func__, data );
       g_free(data);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Wrong format");
       return;
     }
    g_free(data);

    object = json_node_get_object (Query);
    if (!request)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Object non trouvé", __func__ );
       json_node_unref (Query);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No Object");
       return;
     }

    gchar *target_sid = json_object_get_string_member ( object, "wtd_session" );
    if (!target_sid)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: tech_id non trouvé", __func__ );
       json_node_unref (Query);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No target");
       return;
     }

    GSList *liste = Cfg_http.liste_http_clients;
    while(liste)
     { struct HTTP_CLIENT_SESSION *target = liste->data;
       if ( !strcmp(target->wtd_session, target_sid) )
        { if ( session->access_level > target->access_level || !strcmp(session->username,target->username))
           { Cfg_http.liste_http_clients = g_slist_remove ( Cfg_http.liste_http_clients, target );
             Audit_log ( session, "Session of '%s' killed", target->username );
             g_free(target);
             soup_message_set_status (msg, SOUP_STATUS_OK );
           }
          else soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Droits insuffisants" );
          break;
        }
       liste = g_slist_next ( liste );
     }

    json_node_unref (Query);
    if (!liste) soup_message_set_status_full (msg, SOUP_STATUS_NO_CONTENT, "Session not found" );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getusers_list: Traite une requete sur l'URI users/list                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_users_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf, chaine[256];

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    g_snprintf( chaine, sizeof(chaine), "SELECT id,access_level,username,email,enable,comment,notification,allow_cde,phone,xmpp "
                                        "FROM users WHERE access_level<'%d'", session->access_level );
    SQL_Select_to_JSON ( builder, "users", chaine );
    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
    Audit_log ( session, "Get User session list" );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getusers_list: Traite une requete sur l'URI users/list                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_users_sessions ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_array ( builder, "Sessions" );
    GSList *liste = Cfg_http.liste_http_clients;
    while(liste)
     { struct HTTP_CLIENT_SESSION *sess = liste->data;
       Json_add_object ( builder, NULL );
       Json_add_string ( builder, "username", sess->username );
       Json_add_string ( builder, "host", sess->host );
       Json_add_string ( builder, "wtd_session", sess->wtd_session );
       Json_add_int    ( builder, "access_level", sess->access_level );
       Json_add_int    ( builder, "last_request", sess->last_request );
       Json_end_object ( builder );
       liste = g_slist_next ( liste );
     }
    Json_end_array ( builder );

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getusers_list: Traite une requete sur l'URI users/list                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_users_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data )
  { JsonBuilder *builder;
     GBytes *request_brute;
    gsize taille_buf;
    gsize taille;
    gchar chaine[256];

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }
    if ( ! (Json_has_member ( request, "username" ) ) )
     { if (request) json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    gchar *username = Normaliser_chaine ( Json_get_string ( request, "username" ) );
    json_node_unref(request);

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder)
     { g_snprintf( chaine, sizeof(chaine), "SELECT id,access_level,username,email,enable,comment,notification,phone,xmpp "
                                           "FROM users WHERE username='%s' and access_level<='%d'", username, session->access_level );
       SQL_Select_to_JSON ( builder, NULL, chaine );
       gchar *buf = Json_get_buf ( builder, &taille_buf );
   	   soup_message_set_status (msg, SOUP_STATUS_OK);
       soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
     }
    else
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
    g_free(username);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
