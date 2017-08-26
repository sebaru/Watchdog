/******************************************************************************************************************************/
/* Watchdogd/Http/setscenario.c       Gestion des request setscenario pour le thread HTTP de watchdog                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    15.08.2017 19:57:52 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * setscenario.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 
/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"

/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_setscenario: le payload est arrivé, il faut traiter le scenario !                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_completion_setscenario ( struct lws *wsi )
  { unsigned char header[512], *header_cur, *header_end;
    struct HTTP_PER_SESSION_DATA *pss;
    struct SCENARIO_TICK *sce;
    JsonNode *root_node, *node;
    JsonParser *parser;
    JsonObject *object;
    gint retour, taille;
    gchar buf[80];

    pss = lws_wsi_user ( wsi );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %s) HTTP request body completion", __func__, Http_get_session_id(pss->session) );


#ifdef bouh
    if ( pss->session==NULL || pss->session->util==NULL || Tester_groupe_util( pss->session->util, GID_MESSAGE)==FALSE )
     { Http_Send_response_code ( wsi, HTTP_UNAUTHORIZED );
       pss->post_data_length = 0;
       g_free(pss->post_data);
       return(1);
     }
#endif

    header_cur = header;                                                             /* Préparation des headers de la réponse */
    header_end = header + sizeof(header);

    parser = json_parser_new();                                                                    /* Creation du parser JSON */
    if (!parser)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %s) Parser Creation Error", __func__, Http_get_session_id(pss->session) );
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       pss->post_data_length = 0;
       g_free(pss->post_data);
       return(1);
     }

    if ( json_parser_load_from_data ( parser, pss->post_data, pss->post_data_length, NULL ) == FALSE )           /* Parsing ! */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %s) Parsing Error", __func__, Http_get_session_id(pss->session) );
       g_object_unref(parser);
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       pss->post_data_length = 0;
       g_free(pss->post_data);
       return(1);
     }

    root_node = json_parser_get_root (parser);
    if (json_node_get_node_type (root_node) != JSON_NODE_OBJECT)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "%s: (sid %s) Received wrong node_type=%d (%s)", __func__, Http_get_session_id(pss->session),
                 json_node_get_node_type (root_node), json_node_type_name (root_node) );
       g_object_unref(parser);                                                                   /* Libération du parser Json */
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );
       pss->post_data_length = 0;
       g_free(pss->post_data);
       return(1);
     }

    object = json_node_get_object (root_node);
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %s) Received: '%s' with %d members", __func__, Http_get_session_id(pss->session),
              json_node_type_name(root_node), json_object_get_size (object) );

    node = json_object_get_member (object, "num1");
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %s) Received: '%s'", __func__, Http_get_session_id(pss->session),
              json_node_type_name(node) );

    sce = (struct SCENARIO_TICK *)g_malloc0( sizeof(struct SCENARIO_TICK) );
    if (!sce)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %s) Memory Error", __func__, Http_get_session_id(pss->session) );
       g_object_unref(parser);
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       pss->post_data_length = 0;
       g_free(pss->post_data);
       return(1);
     }

    sce->num         = Http_json_get_int (object, "num");                          /* Récupération des valeurs des paramètres */
/*    msg->num         = Http_json_get_int (object, "num");
    msg->type        = Http_json_get_int (object, "type");
    msg->dls_id      = Http_json_get_int (object, "dls_id");
    msg->time_repeat = Http_json_get_int (object, "time_repeat");
    msg->sms         = Http_json_get_int (object, "sms");
    msg->enable      = Http_json_get_int (object, "enable");
    msg->audio       = Http_json_get_int (object, "audio");
    msg->bit_audio   = Http_json_get_int (object, "bit_audio");
    msg->persist     = Http_json_get_int (object, "persist");*/

    g_object_unref(parser);                                                                      /* Libération du parser Json */
    g_free(pss->post_data);
    pss->post_data_length = 0;

/*
    if (msg->id==-1)
     { if ( (msg->id = Ajouter_scenarioDB(msg)) != -1)
        { gchar buf[80];
          g_snprintf( buf, sizeof(buf), "{ \"msg_id\" : %d }", msg->id );
          Http_Send_response_code_with_buffer ( wsi, HTTP_200_OK, HTTP_CONTENT_JSON, buf, strlen(buf) );
        }
       else Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
     }          
    else
     { if ( Modifier_scenarioDB(msg) == TRUE )
        { gchar buf[80];
          g_snprintf( buf, sizeof(buf), "{ \"msg_id\" : %d }", msg->id );
          Http_Send_response_code_with_buffer ( wsi, HTTP_200_OK, HTTP_CONTENT_JSON, buf, strlen(buf) );
        }
       else Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
     }          */
    g_free(sce);
    return(lws_http_transaction_completed(wsi));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
