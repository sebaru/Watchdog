/******************************************************************************************************************************/
/* Watchdogd/Http/getinstance.c       Gestion des request getinstance pour le thread HTTP de watchdog                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    26.07.2020 21:23:28 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getinstance.c
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
/* Http_Traiter_instance_list: Fourni une list JSON des instances Watchdog dans le domaine                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_instance_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Select_to_json_node ( RootNode, "instances",
                             "SELECT DISTINCT(instance_id),valeur AS instance_is_master FROM config WHERE nom='instance_is_master'" );

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_traiter_log: Répond aux requetes sur l'URI log                                                                        */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_instance_loglevel ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data)
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( Config.instance_is_master && Json_has_member ( request, "instance" ) &&
         strcasecmp ( Json_get_string(request,"instance"), "MASTER" ) &&
         strcasecmp ( Json_get_string(request,"instance"), g_get_host_name() ) )
     { Http_redirect_to_slave ( msg, Json_get_string(request,"instance") );
       json_node_unref(request);
       return;
     }

    if ( ! (Json_has_member ( request, "log_level" ) && Json_has_member ( request, "log_db" ) &&
            Json_has_member ( request, "log_zmq" ))
       )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint log_target = Json_get_int ( request, "log_level" );
    if (log_target<3 || log_target>7)
     { json_node_unref(request);
	      soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais niveau de log");
       return;
     }

    Info_change_log_level ( Config.log, log_target );
    Modifier_configDB_int ( "msrv", "log_level", log_target );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: LogLevel set to '%d'", __func__, log_target );

    Config.log_db = Json_get_int ( request, "log_db" );
    Modifier_configDB_int ( "msrv", "log_db", Config.log_db );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: LogDB set to '%d'", __func__, Config.log_db );

    Config.log_zmq = Json_get_int ( request, "log_zmq" );
    Modifier_configDB_int ( "msrv", "log_zmq", Config.log_zmq );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: LogZMQ set to '%d'", __func__, Config.log_zmq );


    json_node_unref(request);
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
