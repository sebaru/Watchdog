/******************************************************************************************************************************/
/* Watchdogd/Http/delmessage.c       Gestion des request delmessage pour le thread HTTP de watchdog                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    19.03.2017 11:37:58 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * delmessage.c
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
/* Http_Traiter_request_delmessage: Traite une requete sur l'URI message                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gint Http_Traiter_request_delmessage ( struct lws *wsi, struct HTTP_SESSION *session )
  { struct HTTP_PER_SESSION_DATA *pss;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %.12s) HTTP request",
              __func__, Http_get_session_id(session) );

    pss = lws_wsi_user ( wsi );
    g_snprintf( pss->url, sizeof(pss->url), "/ws/delmessage" );
    return(0);               
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_delmessage: le payload est arrivé, il faut traiter le message !                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_completion_delmessage ( struct lws *wsi )
  { unsigned char header[512], *header_cur, *header_end;
    struct HTTP_PER_SESSION_DATA *pss;
    JsonNode *root_node;
    JsonParser *parser;
    gint retour, taille, code;

    pss = lws_wsi_user ( wsi );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %.12s) Processing...", __func__, Http_get_session_id(pss->session) );


    if ( pss->session==NULL || pss->session->util==NULL || Tester_groupe_util( pss->session->util, GID_MESSAGE)==FALSE)
     { Http_Send_error_code ( wsi, 401 );
       return(TRUE);
     }

    header_cur = header;                                                             /* Préparation des headers de la réponse */
    header_end = header + sizeof(header);

    code = 400;                                                                                                /* Bad Request */
    parser = json_parser_new();                                                                    /* Creation du parser JSON */
    if (!parser)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %.12s) Parser Creation Error", __func__, Http_get_session_id(pss->session) );
       goto end;
     }

    if ( json_parser_load_from_data ( parser, pss->post_data, pss->post_data_length, NULL ) == FALSE )           /* Parsing ! */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %.12s) Parsing Error", __func__, Http_get_session_id(pss->session) );
       g_object_unref(parser);
       goto end;
     }

    root_node = json_parser_get_root (parser);
    if (json_node_get_node_type (root_node) == JSON_NODE_OBJECT)
     { struct CMD_TYPE_MESSAGE msg;
       JsonObject *object;
       JsonNode *node;

       object = json_node_get_object (root_node);
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "%s: (sid %.12s) Received: object with %d members", __func__, Http_get_session_id(pss->session),
                json_object_get_size (object) );

       msg.id = Http_json_get_int (object, "id");
       if (msg.id!=-1 && Retirer_messageDB( &msg ) == TRUE ) code = 200;
     }
    else
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "%s: (sid %.12s) Received wrong node_type=%d (%s)", __func__, Http_get_session_id(pss->session),
                 json_node_get_node_type (root_node), json_node_type_name (root_node) );
     }
    g_object_unref(parser);                                                                      /* Libération du parser Json */
end:
    g_free(pss->post_data);
    retour = lws_add_http_header_status( wsi, code, &header_cur, header_end );
    retour = lws_finalize_http_header ( wsi, &header_cur, header_end );
    *header_cur='\0';                                                                               /* Caractere null d'arret */
    lws_write( wsi, header, header_cur - header, LWS_WRITE_HTTP_HEADERS );
    pss->post_data_length = 0;
    return(1);                                         /* on clos direct la connexion -- lws_http_transaction_completed(wsi));*/
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
