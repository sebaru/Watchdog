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
  { gchar *buf;
    gsize taille_buf;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Select_to_JSON ( builder, "instances", "SELECT DISTINCT(instance_id),valeur AS instance_is_master FROM config WHERE nom='instance_is_master'" );

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_traiter_instance_reset: Reset une instance                                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_instance_reset ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille;
    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( Config.instance_is_master && Json_has_member ( request, "instance" ) &&
         strcasecmp ( Json_get_string(request,"instance"), "MASTER" ) &&
         strcasecmp ( Json_get_string(request,"instance"), g_get_host_name() ) )
     { Http_redirect_to_slave ( msg, Json_get_string(request,"instance") );
       json_node_unref(request);
       return;
     }

    Partage->com_msrv.Thread_run = FALSE;
    Audit_log ( session, "Instance is restarting" );
    json_node_unref(request);
    soup_message_set_status ( msg, SOUP_STATUS_OK );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
