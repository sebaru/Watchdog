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

 #define USER_TAILLE_SALT  128
 #define USER_TAILLE_HASH  (EVP_MAX_MD_SIZE*2+1)

/******************************************************************************************************************************/
/* Http_user_generate_salt: Génère un salt dans le buffer en parametre                                                        */
/* Entrées: le buffer                                                                                                         */
/* Sortie : le buffer, avec le salt                                                                                           */
/******************************************************************************************************************************/
 static gchar *Http_user_generate_salt ( void )
  { guchar salt_bin[17];
    gchar *salt = g_try_malloc0 ( USER_TAILLE_SALT );
    if (!salt) return(NULL);
    RAND_bytes ( salt_bin, 16 );
    for (gint i=0; i<16; i++)
     { gchar chaine[3];
       g_snprintf(chaine, sizeof(chaine), "%02x", salt_bin[i] );
       g_strlcat( salt, chaine, USER_TAILLE_SALT );
     }
    return(salt);
  }
/******************************************************************************************************************************/
/* Http_user_generate_hash: Génère un hash dans le buffer en parametre, s'appuyant sur le salt                                */
/* Entrées: le salt, le password                                                                                              */
/* Sortie : le buffer                                                                                                         */
/******************************************************************************************************************************/
 static gchar *Http_user_generate_hash ( gchar *salt, gchar *password )
  { guchar hash_bin[EVP_MAX_MD_SIZE];
    gchar *hash = g_try_malloc0 ( USER_TAILLE_HASH );

    gint md_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                                                                               /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, salt, strlen(salt));
    EVP_DigestUpdate(mdctx, password, strlen(password) );
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);
    for (gint i=0; i<md_len; i++)
     { gchar chaine[3];
       g_snprintf(chaine, sizeof(chaine), "%02x", hash_bin[i] );
       g_strlcat( hash, chaine, USER_TAILLE_HASH );
     }
    return(hash);
  }
/******************************************************************************************************************************/
/* Http_Traiter_mnemos_set: Modifie la config d'un mnemonique                                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_users_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "username" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar chaine[512], critere[256];
    g_snprintf ( chaine, sizeof(chaine), "UPDATE users SET date_modif=NOW()" );

    if ( Json_has_member ( request, "access_level" ) )
     { gint level = Json_get_int ( request, "access_level" );
       if (level<1) level=1;
       g_snprintf( critere, sizeof(critere), ", access_level=%d", Json_get_int ( request, "access_level" ) );
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
     { gchar *salt = Http_user_generate_salt();
       if (salt)
        { gchar *hash = Http_user_generate_hash( salt, Json_get_string ( request, "password" ) );

          if (hash)
           { g_snprintf( critere, sizeof(critere), ", salt='%s', hash='%s'", salt, hash );
             g_strlcat ( chaine, critere, sizeof(chaine) );
             g_free(hash);
           }
          g_free(salt);
        }
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
  { gchar chaine[256];

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "username" ) && Json_has_member ( request, "email" ) &&
            Json_has_member ( request, "password" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *salt = Http_user_generate_salt ();
    if (!salt)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Salt Error");
       return;
     }

    gchar *hash = Http_user_generate_hash ( salt, Json_get_string ( request, "password" ) );
    if (!hash)
     { json_node_unref(request);
       g_free(salt);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Hash Error");
       return;
     }

    gchar *username = Normaliser_chaine ( Json_get_string ( request, "username" ) );
    gchar *email    = Normaliser_chaine ( Json_get_string ( request, "email" ) );
    g_snprintf ( chaine, sizeof(chaine), "INSERT INTO users SET access_level=1, username='%s', email='%s', salt='%s', hash='%s'",
                 username, email, salt, hash );
    g_free(salt);
    g_free(hash);
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
  { if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "username" ) ) )
     { json_node_unref(request);
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
  {
    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint target_id = Json_get_int( request, "id" );

    GSList *liste = Partage->com_http.liste_http_clients;
    while(liste)
     { struct HTTP_CLIENT_SESSION *target = liste->data;
       if ( target->id == target_id )
        { if ( session->access_level>target->access_level || !strcmp(session->username,target->username) )
           { Partage->com_http.liste_http_clients = g_slist_remove ( Partage->com_http.liste_http_clients, target );
             Audit_log ( session, "Session '%d' for '%s' on '%s' killed", target->id, target->username, target->appareil );
             g_free(target);
             soup_message_set_status (msg, SOUP_STATUS_OK );
           }
          else soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Droits insuffisants" );
          break;
        }
       liste = g_slist_next ( liste );
     }

    json_node_unref (request);
    if (!liste) soup_message_set_status_full (msg, SOUP_STATUS_NO_CONTENT, "Session not found" );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getusers_list: Traite une requete sur l'URI users/list                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_users_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Select_to_json_node ( RootNode, "users",
                             "SELECT id,access_level,username,email,enable,comment,notification,allow_cde,phone,xmpp "
                             "FROM users WHERE access_level<'%d' OR (access_level='%d' AND username='%s')",
                             session->access_level, session->access_level, session->username );

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getusers_list: Traite une requete sur l'URI users/list                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_users_sessions ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;
/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    JsonArray *sessions = Json_node_add_array ( RootNode, "Sessions" );
    GSList *liste = Partage->com_http.liste_http_clients;
    while(liste)
     { struct HTTP_CLIENT_SESSION *sess = liste->data;
       if ( (sess->access_level < session->access_level) ||
            (!strcmp( sess->username, session->username)) )
        { JsonNode *session_node = Json_node_create();
          if (session_node)
           { Json_node_add_string ( session_node, "username", sess->username );
             Json_node_add_string ( session_node, "appareil", sess->appareil );
             Json_node_add_string ( session_node, "useragent", sess->useragent );
             Json_node_add_string ( session_node, "host", sess->host );
             Json_node_add_int    ( session_node, "id", sess->id );
             Json_node_add_int    ( session_node, "access_level", sess->access_level );
             Json_node_add_int    ( session_node, "last_request", sess->last_request );
             Json_array_add_element ( sessions, session_node );
           }
        }
       liste = g_slist_next ( liste );
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getusers_list: Traite une requete sur l'URI users/list                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_users_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "username" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    gchar *username = Normaliser_chaine ( Json_get_string ( request, "username" ) );
    json_node_unref(request);

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Select_to_json_node ( RootNode, NULL,
                             "SELECT id,access_level,username,email,enable,comment,notification,phone,xmpp "
                             "FROM users WHERE username='%s' and access_level<'%d'", username, session->access_level );
    g_free(username);

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
