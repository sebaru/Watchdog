/******************************************************************************************************************************/
/* Watchdogd/Http/setmessage.c       Gestion des request setmessage pour le thread HTTP de watchdog                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    22.09.2016 14:18:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * setmessage.c
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
/* Http_Traiter_request_setmessage: Traite une requete sur l'URI message                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gint Http_Traiter_request_setmessage ( struct lws *wsi, struct HTTP_SESSION *session, gchar *remote_name, gchar *remote_ip )
  { struct HTTP_PER_SESSION_DATA *pss;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
             "%s: (sid %.12s) HTTP request from %s(%s)",
              __func__, Http_get_session_id(session), remote_name, remote_ip );

    pss = lws_wsi_user ( wsi );
    g_snprintf( pss->url, sizeof(pss->url), "/ws/setmessage" );
    return(0);               
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_setmessage: le payload est arrivé, il faut traiter le message !                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_completion_setmessage ( struct lws *wsi )
  { unsigned char header[512], *header_cur, *header_end;
    struct HTTP_PER_SESSION_DATA *pss;
    JsonNode *root_node;
    JsonParser *parser;
    gint retour, taille, replace = -1, id = -1, code;

    pss = lws_wsi_user ( wsi );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %.12s) HTTP request body completion", __func__, Http_get_session_id(pss->session) );

    header_cur = header;                                                             /* Préparation des headers de la réponse */
    header_end = header + sizeof(header);

    code = 500;
    parser = json_parser_new();                                                                    /* Creation du parser JSON */
    if (!parser)
     { goto end;
     }

    if ( json_parser_load_from_data ( parser, pss->post_data, pss->post_data_length, NULL ) == FALSE )           /* Parsing ! */
     { g_object_unref(parser);
       goto end;
     }

    root_node = json_parser_get_root (parser);
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %.12s) Received: type_name=%s, node_type=%d", __func__, Http_get_session_id(pss->session),
              json_node_type_name (root_node), json_node_get_node_type (root_node)
            );
    code = 200;

    g_object_unref(parser);                                                                      /* Libération du parser Json */
end:
    retour = lws_add_http_header_status( wsi, code, &header_cur, header_end );
    retour = lws_finalize_http_header ( wsi, &header_cur, header_end );
    *header_cur='\0';                                                                               /* Caractere null d'arret */
    lws_write( wsi, header, header_cur - header, LWS_WRITE_HTTP_HEADERS );

    g_free(pss->post_data);
    pss->post_data_length = 0;
    return(lws_http_transaction_completed(wsi));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
