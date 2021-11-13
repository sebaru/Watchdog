/******************************************************************************************************************************/
/* Watchdogd/Http/bus.c       Gestion des request bus pour le thread HTTP de watchdog                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    26.12.2017 15:56:43 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * bus.c
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

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;
/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_bus: le payload est arrivé, il faut traiter le fichier                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 void Http_traiter_bus ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                         SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    /*Zmq_Send_json_node ( Cfg_http.zmq_to_master, NOM_THREAD, host, thread, tag, (void *)text, strlen(text)+1 );*/
    json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
