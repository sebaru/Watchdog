/******************************************************************************************************************************/
/* Watchdogd/Http/gettableau.c       Gestion des requests sur l'URI /tableau du webservice                                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    12.12.2020 10:11:53 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * gettableau.c
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
/* Http_Traiter_get_tableau: Fourni une list JSON des elements d'un tableauoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_tableau_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { gchar *buf, chaine[256];
    gsize taille_buf;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * FROM tableau WHERE access_level<=%d", session->access_level);
    if (SQL_Select_to_JSON ( builder, "tableaux", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_tableau: Fourni une list JSON des elements d'un tableauoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_tableau_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { gchar chaine[256];
    GBytes *request_brute;
    gsize taille;

    if (msg->method != SOUP_METHOD_DELETE)
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

    if ( ! (Json_has_member ( request, "id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint tableau_id = Json_get_int ( request, "id" );
    json_node_unref(request);

    g_snprintf( chaine, sizeof(chaine), "DELETE FROM tableau WHERE id=%d AND access_level<='%d'",
                tableau_id, session->access_level );
    if (SQL_Write (chaine)==FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Delete Error");
       return;
     }

/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    Audit_log ( session, "tableau '%d' deleted", tableau_id );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_tableau: Fourni une list JSON des elements d'un tableauoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_tableau_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gchar requete[256];
    gsize taille;

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( ! (Json_has_member ( request, "titre" ) && Json_has_member ( request, "access_level" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *titre       = Normaliser_chaine ( Json_get_string( request, "titre" ) );
    gint  access_level = Json_get_int ( request, "access_level" );
    if (access_level>session->access_level) access_level = session->access_level;

    if ( Json_has_member ( request, "id" ) )                                                                       /* Edition */
     { g_snprintf( requete, sizeof(requete),
                  "UPDATE tableau SET titre='%s', access_level='%d' WHERE id='%d' AND access_level<'%d'",
                   titre, access_level, Json_get_int(request,"id"), session->access_level );
       Audit_log ( session, "tableau '%s'(%d) created", titre, Json_get_int(request,"id") );
     }
    else
     {
       g_snprintf( requete, sizeof(requete),
                  "INSERT INTO tableau SET titre=CONCAT('%s','-',id), access_level='%d'",
                   titre, access_level );
       Audit_log ( session, "tableau '%s' created", titre );
     }
    if (SQL_Write (requete)) { soup_message_set_status (msg, SOUP_STATUS_OK); }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );

    g_free(titre);
    json_node_unref(request);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
