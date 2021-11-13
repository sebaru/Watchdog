/******************************************************************************************************************************/
/* Watchdogd/Http/bus.c       Gestion des request bus pour le thread HTTP de watchdog                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    26.12.2017 15:56:43 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * bus.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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

/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_bus: le payload est arrivé, il faut traiter le fichier                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_completion_bus ( struct lws *wsi )
  { const gchar *host, *thread, *tag, *text;
    struct HTTP_PER_SESSION_DATA *pss;
    JsonObject *object;
    JsonNode *Query;

    pss = lws_wsi_user ( wsi );
    pss->post_data [ pss->post_data_length ] = 0;
    Query = json_from_string ( pss->post_data, NULL );
    pss->post_data_length = 0;
    g_free(pss->post_data);

    if (!Query)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: requete non Json", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    object = json_node_get_object (Query);
    if (!object)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Object non trouvé", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    host = json_object_get_string_member ( object, "host" );
    if (!host)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: host non trouvé", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    thread = json_object_get_string_member ( object, "thread" );
    if (!thread)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: thread non trouvé", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    tag = json_object_get_string_member ( object, "tag" );
    if (!tag)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: text non trouvé", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    text = json_object_get_string_member ( object, "text" );
    if (!text)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: tag non trouvé", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
             "%s: HTTP/CLI request for %s:%s:%s:%s", __func__, host, thread, tag, text );

    Send_zmq_with_tag ( Cfg_http.zmq_to_master, NULL, NOM_THREAD, host, thread, tag, (void *)text, strlen(text)+1 );
    return(Http_Send_response_code ( wsi, HTTP_200_OK ));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
