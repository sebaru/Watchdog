/******************************************************************************************************************************/
/* Watchdogd/Http/cli.c       Gestion des request cli pour le thread HTTP de watchdog                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    26.12.2017 15:56:43 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * cli.c
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

 static const char *PARAM_CLI[] =
  { "user", "id", "host", "commande" };
 enum
  { PARAM_CLI_USER,
    PARAM_CLI_ID,
    PARAM_CLI_HOST,
    PARAM_CLI_COMMANDE,
    NBR_PARAM_CLI
  };
/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_cli: le payload est arrivé, il faut traiter le fichier                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_completion_cli ( struct lws *wsi )
  { const gchar *ev_host, *ev_thread, *ev_text;
    struct HTTP_PER_SESSION_DATA *pss;
    JsonObject *object;
    JsonNode *Query;
    gint retour;

    pss = lws_wsi_user ( wsi );
    pss->post_data [ pss->post_data_length ] = 0;
    Query = json_from_string ( pss->post_data, NULL );

    if (!Query)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: requete non Json", __func__ );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       goto end;
     }

    object = json_node_get_object (Query);
    if (!object)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Object non trouvé", __func__ );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       goto end;
     }

    ev_host = json_object_get_string_member ( object, "ev_host" );
    if (!ev_host)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: ev_host non trouvé", __func__ );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       goto end;
     }

    ev_thread = json_object_get_string_member ( object, "ev_thread" );
    if (!ev_thread)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: ev_thread non trouvé", __func__ );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       goto end;
     }

    ev_text = json_object_get_string_member ( object, "ev_text" );
    if (!ev_text)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: ev_text non trouvé", __func__ );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       goto end;
     }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
             "%s: HTTP/CLI request for %s:%s:%s", __func__, ev_host, ev_thread, ev_text );
             
    Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave,   TAG_ZMQ_CLI, ev_host, ev_thread,
                        (void *)ev_text, pss->post_data_length+1 );
    Send_zmq_with_tag ( Partage->com_msrv.zmq_to_threads, TAG_ZMQ_CLI, ev_host, ev_thread,
                        (void *)ev_text, pss->post_data_length+1 );
    Http_Send_response_code ( wsi, HTTP_200_OK );

end:
    pss->post_data_length = 0;
    g_free(pss->post_data);
    return(1);                                                                                         /* si erreur, on coupe */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
