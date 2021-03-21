/******************************************************************************************************************************/
/* Watchdogd/Http/gettableau.c       Gestion des requests sur l'URI /tableau du webservice                                    */
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
/* Http_Traiter_get_tableau: Fourni une list JSON des elements d'un tableau                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_tableau_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }

    if ( SQL_Select_to_json_node ( RootNode, "tableaux",
                                  "SELECT tableau.*,syns.page FROM tableau "
                                  "INNER JOIN syns ON tableau.syn_id = syns.id "
                                  "WHERE access_level<=%d", session->access_level ) == FALSE )
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(RootNode);
       return;
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_tableau: Fourni une list JSON des elements d'un tableau                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_tableau_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { gchar chaine[256];

    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint tableau_id = Json_get_int ( request, "id" );
    json_node_unref(request);

    g_snprintf( chaine, sizeof(chaine), "DELETE tableau FROM tableau "
                                        "INNER JOIN syns ON tableau.syn_id = syns.id "
                                        "WHERE tableau.id=%d AND syns.access_level<='%d'",
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
/* Http_Traiter_get_tableau: Fourni une list JSON des elements d'un tableau                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_tableau_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { gchar requete[256];

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "titre" ) && Json_has_member ( request, "syn_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint syn_id  = Json_get_int ( request, "syn_id" );
    gchar *titre = Normaliser_chaine ( Json_get_string( request, "titre" ) );

    if ( Json_has_member ( request, "id" ) )                                                                       /* Edition */
     { g_snprintf( requete, sizeof(requete),
                  "UPDATE tableau INNER JOIN syns ON tableau.syn_id = syns.id "
                  "SET titre='%s', syn_id='%d' WHERE tableau.id='%d' AND access_level<='%d'",
                   titre, syn_id, Json_get_int(request,"id"), session->access_level );
       Audit_log ( session, "tableau '%s'(%d) modifié", titre, Json_get_int(request,"id") );
     }
    else
     { g_snprintf( requete, sizeof(requete),
                  "INSERT INTO tableau SET titre='%s', syn_id='%d'",
                   titre, syn_id );
       Audit_log ( session, "tableau '%s' created", titre );
     }
    if (SQL_Write (requete)) { soup_message_set_status (msg, SOUP_STATUS_OK); }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );

    g_free(titre);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_tableau: Fourni une list JSON des elements d'un tableau                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_tableau_map_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gchar *tableau_id_string = g_hash_table_lookup ( query, "tableau_id" );
    if (!tableau_id_string)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres tableau_id");
       return;
     }
    gint tableau_id = atoi(tableau_id_string);

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "tableau_map",
                                 "SELECT tm.*,dico.libelle,dico.unite FROM tableau_map AS tm "
                                 "INNER JOIN tableau AS t ON t.id=tm.tableau_id "
                                 "INNER JOIN syns ON t.syn_id = syns.id "
                                 "LEFT JOIN dictionnaire AS dico ON tm.tech_id=dico.tech_id AND tm.acronyme=dico.acronyme "
                                 "WHERE syns.access_level<=%d AND t.id='%d'", session->access_level, tableau_id ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       json_node_unref ( RootNode );
       return;
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_tableau: Fourni une list JSON des elements d'un tableau                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_tableau_map_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data )
  { gchar chaine[256];

    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint tableau_id = Json_get_int ( request, "id" );
    json_node_unref(request);

    g_snprintf( chaine, sizeof(chaine), "DELETE tm FROM tableau_map AS tm "
                                        "INNER JOIN tableau AS t ON t.id=tm.tableau_id "
                                        "INNER JOIN syns ON t.syn_id = syns.id "
                                        "WHERE tm.id=%d AND syns.access_level<='%d'", tableau_id, session->access_level );
    if (SQL_Write (chaine)==FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Delete Error");
       return;
     }

/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    Audit_log ( session, "tableau '%d' deleted", tableau_id );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_tableau: Fourni une list JSON des elements d'un tableau                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_tableau_map_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data )
  { gchar requete[256];

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
            Json_has_member ( request, "color") ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *tech_id  = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *acronyme = Normaliser_chaine ( Json_get_string( request, "acronyme" ) );
    gchar *color    = Normaliser_chaine ( Json_get_string( request, "color" ) );

    if ( Json_has_member ( request, "id" ) )                                                                       /* Edition */
     { gint id = Json_get_int(request,"id");
       g_snprintf( requete, sizeof(requete),
                  "UPDATE tableau_map AS tm "
                  "INNER JOIN tableau AS t ON t.id=tm.tableau_id "
                  "INNER JOIN syns ON t.syn_id = syns.id "
                  " SET tech_id='%s', acronyme='%s', color='%s' WHERE tm.id='%d' AND syns.access_level<='%d'",
                   tech_id, acronyme, color, id, session->access_level );
       Audit_log ( session, "tableau_map '%d' changed to '%s:%s", id, tech_id, acronyme );
       if (SQL_Write (requete)) { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Update Error" );
     }
    else if ( Json_has_member ( request, "tableau_id" ) )
     { gint tableau_id = Json_get_int ( request, "tableau_id" );
       g_snprintf( requete, sizeof(requete),
                  "INSERT INTO tableau_map "
                  " SET tableau_id='%d', tech_id='%s', acronyme='%s', color='%s' ",
                   tableau_id, tech_id, acronyme, color );
       Audit_log ( session, "tableau_map: '%s:%s added to tableau '%d'", tech_id, acronyme, tableau_id );
       if (SQL_Write (requete)) { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Insert Error" );
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Request Error" );

    g_free(tech_id);
    g_free(acronyme);
    g_free(color);
    json_node_unref(request);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
