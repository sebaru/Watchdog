/******************************************************************************************************************************/
/* Watchdogd/Http/ws_histo_msgs.c        Gestion des echanges des élements d'historiques/messages via webconnect de watchdog  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    01.05.2020 09:53:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ws_histo_msgs.c
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
 #include <sys/time.h>
/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;

/******************************************************************************************************************************/
/* Http_traiter_log: Répond aux requetes sur l'URI log                                                                        */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_histo_alive ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data)
  { gchar critere[32];
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    Http_print_request ( server, msg, path, client );
    gpointer syn_id_src = g_hash_table_lookup ( query, "syn_id" );
    if (syn_id_src) g_snprintf( critere, sizeof(critere), "AND syn.id=%d", atoi(syn_id_src) );
    else bzero ( critere, sizeof(critere) );

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "enregs",
                                  "SELECT histo.*, histo.alive, histo.libelle, msg.typologie, dls.syn_id,"
                                  "parent_syn.page as syn_parent_page, syn.page as syn_page,"
                                  "dls.shortname as dls_shortname, msg.tech_id, msg.acronyme"
                                  " FROM histo_msgs as histo"
                                  " INNER JOIN msgs as msg ON msg.id = histo.id_msg"
                                  " INNER JOIN dls as dls ON dls.tech_id = msg.tech_id"
                                  " INNER JOIN syns as syn ON syn.id = dls.syn_id"
                                  " INNER JOIN syns as parent_syn ON parent_syn.id = syn.parent_id"
                                  " WHERE alive = 1 %s ORDER BY histo.date_create DESC", critere ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
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
/* Http_traiter_log: Répond aux requetes sur l'URI log                                                                        */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_histo_ack ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data)
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    gchar *tech_id  = Json_get_string( request, "tech_id" );
    gchar *acronyme = Json_get_string( request, "acronyme" );

    if (tech_id && acronyme)
     { gchar chaine[80], date_fixe[80];
       struct timeval tv;
       gettimeofday( &tv, NULL );
       struct tm *temps = localtime( (time_t *)&tv.tv_sec );
       strftime( chaine, sizeof(chaine), "%F %T", temps );
       gchar *temp_date_fixe = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
       g_snprintf( date_fixe, sizeof(date_fixe), "%s", temp_date_fixe );
       g_free( temp_date_fixe );

       Acquitter_histo_msgsDB ( tech_id, acronyme, session->username, date_fixe );
       Dls_acquitter_plugin(tech_id);

       JsonNode *RootNode = Json_node_create ();
       if (RootNode == NULL)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: JSon RootNode creation failed", __func__ );
          soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
        }
       else
        { Json_node_add_string ( RootNode, "zmq_tag", "DLS_HISTO" );
          Json_node_add_string ( RootNode, "tech_id", tech_id );
          Json_node_add_string ( RootNode, "acronyme", acronyme );
          Json_node_add_string ( RootNode, "nom_ack", session->username );
          Json_node_add_string ( RootNode, "date_fixe", date_fixe );
          Http_ws_send_to_all ( RootNode );
          json_node_unref(RootNode);
          soup_message_set_status (msg, SOUP_STATUS_OK);
        }
     }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
    json_node_unref(request);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
