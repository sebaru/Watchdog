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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

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
            Json_has_member ( request, "log_zmq" ) && Json_has_member ( request, "log_trad" ) )
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

    Config.log_db = Json_get_bool ( request, "log_db" );
    Modifier_configDB ( "msrv", "log_db", (Config.log_db ? "true" : "false") );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: LogDB set to '%d'", __func__, Config.log_db );

    Config.log_zmq = Json_get_bool ( request, "log_zmq" );
    Modifier_configDB ( "msrv", "log_zmq", (Config.log_zmq ? "true" : "false") );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: LogZMQ set to '%d'", __func__, Config.log_zmq );

    Config.log_trad = Json_get_bool ( request, "log_trad" );
    Modifier_configDB ( "msrv", "log_trad", (Config.log_trad ? "true" : "false") );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: LogTrad set to '%d'", __func__, Config.log_trad );

    json_node_unref(request);
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_traiter_instance_reload_icons: Met a jour la base d'icones selon base_icones.sql                                      */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_instance_reload_icons ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data)
  { struct stat stat_buf;
    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    if ( !Config.instance_is_master )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Instance is not master. Ignoring.", __func__ );
       return;
     }

    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Reloading Icons DB", __func__ );
    gchar *DB_ICONS = "/usr/local/share/Watchdog/base_icones.sql";

    if (stat ( DB_ICONS, &stat_buf)==-1)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Stat DB Icones Error" );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Stat DB Icones Error", __func__ );
       return;
     }

    gchar *db_icones = g_try_malloc0 ( stat_buf.st_size+1 );
    if (!db_icones)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory DB Icones Error" );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Memory DB Icones Error", __func__ );
       return;
     }

    gint fd = open ( DB_ICONS, O_RDONLY );
    if (!fd)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Open DB Icones Error" );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Open DB Icones Error", __func__ );
       g_free(db_icones);
       return;
     }
    if (read ( fd, db_icones, stat_buf.st_size ) != stat_buf.st_size)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Read DB Icones Error" );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Read DB Icones Error", __func__ );
       g_free(db_icones);
       return;
     }
    close(fd);

    SQL_Write_new ( db_icones );
    g_free(db_icones);
    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Icons DB Loaded.", __func__ );

	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
