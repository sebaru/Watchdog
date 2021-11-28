/******************************************************************************************************************************/
/* Watchdogd/Http/getprocess.c       Gestion des request getprocess pour le thread HTTP de watchdog                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    30.12.2018 01:59:26 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getprocess.c
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

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_list: Traite une requete sur l'URI process/list                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_process_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                  SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     { soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
       return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gchar *name = g_hash_table_lookup ( query, "name" );
    if (name) { Normaliser_as_ascii ( name ); }
/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (name)
     { SQL_Select_to_json_node ( RootNode, "Process", "SELECT * FROM processes WHERE name='%s' ORDER BY host", name ); }
    else SQL_Select_to_json_node ( RootNode, "Process", "SELECT * FROM processes ORDER BY host, name" ); /* Contenu du Status */

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_traiter_process_status: Donne la config d'un process                                                                  */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_traiter_process_config_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                               SoupClientContext *client, gpointer user_data )
  { struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gchar *name = g_hash_table_lookup ( query, "name" );
    if (!name)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres tech_id");
       return;
     }
    Normaliser_as_ascii ( name );

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Select_to_json_node ( RootNode, "config",
                              "SELECT p.*,config.* FROM %s AS config "
                              "INNER JOIN processes AS p ON p.uuid = config.uuid ", name ); /* Contenu du Status */

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_traiter_process_config_delete: Delete une config d'un process                                                         */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_traiter_process_config_delete ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                                  SoupClientContext *client, gpointer user_data )
  { struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    if ( ! (Json_has_member ( request, "uuid" ) && Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *uuid    = Normaliser_chaine ( Json_get_string ( request, "uuid" ) );
    JsonNode *RootNode = Json_node_create();
    if (RootNode)
     { SQL_Select_to_json_node ( RootNode, NULL, "SELECT name FROM processes WHERE uuid = '%s'", uuid );
       gchar *tech_id = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
       SQL_Write_new ( "DELETE FROM %s WHERE tech_id='%s'", Json_get_string( RootNode, "name" ), tech_id );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' deleted.", __func__, uuid, tech_id );

       g_free(tech_id);
       json_node_unref(RootNode);
     }
    g_free(uuid);
/*************************************************** Envoi au client **********************************************************/
    json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_traiter_process_config_post: Ajoute ou modifie une config d'un process                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_traiter_process_config_post ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                                SoupClientContext *client, gpointer user_data )
  { struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    if ( ! (Json_has_member ( request, "uuid" ) && Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    JsonNode *RootNode = Json_node_create();
    if (!RootNode)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Memory Error");
       return;
     }
    gchar *uuid = Normaliser_chaine ( Json_get_string ( request, "uuid" ) );
    SQL_Select_to_json_node ( RootNode, NULL, "SELECT name FROM processes WHERE uuid = '%s'", uuid );
    g_free(uuid);

    GSList *liste = Partage->com_msrv.Librairies;                                        /* Parcours de toutes les librairies */
    struct PROCESS *lib = NULL;
    while(liste)
     { lib = (struct PROCESS *)liste->data;
       if ( ! strcasecmp( Json_get_string ( RootNode, "name" ), lib->name ) ) break;
       liste = g_slist_next(liste);
     }
    json_node_unref(RootNode);

    if (liste && !lib->Admin_config)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "%s: library %s do not have Admin_config.", __func__, lib->name );
       soup_message_set_status_full (msg, SOUP_STATUS_NOT_IMPLEMENTED, "Missing function admin_config" );
     }
    else if (liste && lib->Admin_config)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Admin_config called by '%s' for %s.",
                 __func__, session->username, path );
       lib->Admin_config ( lib, msg, request );
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_NOT_IMPLEMENTED, "Process not found" );

/*************************************************** Envoi au client **********************************************************/
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_traiter_process_status: Donne la config d'un process                                                                  */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_process_config ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  {      if (msg->method == SOUP_METHOD_GET)    Http_traiter_process_config_get    ( server, msg, path, query, client, user_data );
    else if (msg->method == SOUP_METHOD_DELETE) Http_traiter_process_config_delete ( server, msg, path, query, client, user_data );
    else if (msg->method == SOUP_METHOD_POST)   Http_traiter_process_config_post   ( server, msg, path, query, client, user_data );
    else
     { soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
       return;
     }
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_debug: Active ou non le debug d'un process                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_process_debug ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     { soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
       return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "uuid" ) && Json_has_member ( request, "debug" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar   *uuid_src = Json_get_string ( request,"uuid" );
    gboolean debug    = Json_get_bool ( request, "debug" );

    gchar *uuid = Normaliser_chaine ( uuid_src );
    if (!uuid)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Write_new ( "UPDATE processes SET debug=%d WHERE uuid='%s'", debug, uuid );

    JsonNode *RootNode = Json_node_create();
    if (RootNode)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: UUID %s: %s", __func__, uuid, (debug ? "Setting debug ON" : "Setting debug OFF") );
       Json_node_add_string ( RootNode, "zmq_tag", "PROCESS" );
       Json_node_add_string ( RootNode, "action", "DEBUG" );
       Json_node_add_string ( RootNode, "uuid", uuid );
       Json_node_add_bool   ( RootNode, "debug", debug );
       Zmq_Send_json_node( Cfg_http.lib->zmq_to_master, "HTTP", "MSRV", RootNode );
       json_node_unref(RootNode);
     }
    g_free(uuid);
/*************************************************** Envoi au client **********************************************************/
    json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_start_stop: Traite une requete sur l'URI process/stop|start                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_process_start ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     { soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
       return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "uuid" ) && Json_has_member ( request, "status" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar   *uuid_src = Json_get_string ( request,"uuid" );
    gboolean status   = Json_get_bool ( request, "status" );

    gchar *uuid = Normaliser_chaine ( uuid_src );
    if (!uuid)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Write_new ( "UPDATE processes SET enable=%d WHERE uuid='%s'", status, uuid );

    JsonNode *RootNode = Json_node_create();
    if (RootNode)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: UUID %s: %s", __func__, uuid, (status ? "Enabling" : "Disabling") );
       Json_node_add_string ( RootNode, "zmq_tag", "PROCESS" );
       Json_node_add_string ( RootNode, "action", "RELOAD" );
       Json_node_add_string ( RootNode, "uuid", uuid );
       Zmq_Send_json_node( Cfg_http.lib->zmq_to_master, "HTTP", "MSRV", RootNode );
       json_node_unref(RootNode);
     }
    g_free(uuid);
/*************************************************** Envoi au client **********************************************************/
    json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_reload: Traite une requete sur l'URI process/reload                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_process_reload ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     { soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
       return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! Json_has_member ( request, "uuid" ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar   *uuid_src = Json_get_string ( request,"uuid" );
    gchar *uuid = Normaliser_chaine ( uuid_src );
    if (!uuid)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
/*************************************************** WS Reload library ********************************************************/
    JsonNode *RootNode = Json_node_create();
    if (RootNode)
     { SQL_Select_to_json_node ( RootNode, NULL, "SELECT uuid, host, name FROM processes WHERE uuid='%s'", uuid );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Reloading start for UUID %s: %s:%s", __func__,
                 uuid, Json_get_string ( RootNode, "host" ), Json_get_string ( RootNode, "name" ) );
       Json_node_add_string ( RootNode, "zmq_tag", "PROCESS" );
       Json_node_add_string ( RootNode, "action", "RELOAD" );
       Zmq_Send_json_node( Cfg_http.lib->zmq_to_master, "HTTP", "MSRV", RootNode );
       json_node_unref(RootNode);
     }
    g_free(uuid);
/*************************************************** Envoi au client **********************************************************/
    json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
