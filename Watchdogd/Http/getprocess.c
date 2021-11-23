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
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Select_to_json_node ( RootNode, "Process", "SELECT * FROM processes" );                          /* Contenu du Status */

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_debug: Active ou non le debug d'un process                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_process_debug ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
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
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_start_stop: Traite une requete sur l'URI process/stop|start                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_process_start ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
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
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_reload: Traite une requete sur l'URI process/reload                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_process_reload ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
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
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess: Traite une requete sur l'URI process                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_process ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  {
    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;


    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: Searching for CLI commande %s", __func__, path );
    path = path + strlen("/api/process/");

/****************************************** WS get Running config library *****************************************************/
    if (!strncasecmp ( path, "archive/", strlen("archive/")))
     { Admin_arch_json ( msg, path+strlen("archive/"), query, session->access_level );
       Audit_log ( session, "Processus 'Archive': %s", path );
     }
    else
     { GSList *liste;
       if (msg->method == SOUP_METHOD_GET)                                               /* Test si Slave redirect necessaire */
        { gpointer instance = g_hash_table_lookup ( query, "instance" );
          if (!instance) instance="MASTER";
          if (!strcasecmp(instance, "null")) instance="MASTER";
          if ( Config.instance_is_master && strcasecmp ( instance, "MASTER" ) && strcasecmp ( instance, g_get_host_name() ) )
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s : Redirecting %s vers %s", __func__, path, instance );
             Http_redirect_to_slave ( msg, instance );
             return;
           }
        }
       else if (msg->method == SOUP_METHOD_PUT || msg->method == SOUP_METHOD_POST || msg->method == SOUP_METHOD_DELETE)
        { GBytes *request_brute;
          gsize taille;
          g_object_get ( msg, "request-body-data", &request_brute, NULL );
          JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
          if ( !request)
           { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
             return;
           }

          if ( Config.instance_is_master && Json_has_member ( request, "instance" ) &&
               strcasecmp ( Json_get_string(request,"instance"), "MASTER" ) &&
               strcasecmp ( Json_get_string(request,"instance"), g_get_host_name() ) )
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                       "%s : Redirecting %s vers %s", __func__, path, Json_get_string(request,"instance") );
             Http_redirect_to_slave ( msg, Json_get_string(request,"instance") );
             //soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
             json_node_unref(request);
             return;
           }
        }
       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       while(liste)
        { struct LIBRAIRIE *lib = liste->data;
          if ( ! strncasecmp( path, lib->name, strlen(lib->name) ) )
           { if (!lib->Admin_json)
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                          "%s: library %s do not have Admin_json.", __func__, lib->name );
                soup_message_set_status_full (msg, SOUP_STATUS_NOT_IMPLEMENTED, "Missing function admin_json" );
                return;
              }
             else
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Admin_json called by '%s' for %s.",
                          __func__, session->username, path );
                lib->Admin_json ( lib, msg, path+strlen(lib->name), query, session->access_level );
                Audit_log ( session, "Processus '%s': %s", lib->name, path );
                return;
              }
            }
           liste = g_slist_next(liste);
        }
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Unknown Thread" );
       return;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
