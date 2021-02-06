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
 static void Http_ws_msgs_on_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Close Connexion received !", __func__ );
    g_object_unref(connexion);
    Cfg_http.liste_ws_msgs_clients = g_slist_remove ( Cfg_http.liste_ws_msgs_clients, connexion );
  }
 static void Http_ws_msgs_on_error  ( SoupWebsocketConnection *connexion, GError *error, gpointer user_data )
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Error '%s' Connexion received !",
              __func__, error->message );
    g_error_free (error);
  }
/******************************************************************************************************************************/
/* Http_ws_send_to_all: Envoi d'un buffer a tous les clients connectés à la websocket                                         */
/* Entrée: Le buffer                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_ws_send_to_all ( JsonNode *node )
  { gsize taille_buf;
    gchar *buf = Json_node_get_buf ( node, &taille_buf );
    GSList *liste = Cfg_http.liste_ws_msgs_clients;
    while (liste)
     { SoupWebsocketConnection *connexion = liste->data;
       soup_websocket_connection_send_text ( connexion, buf );
       liste = g_slist_next(liste);
     }
    g_free(buf);
  }
/******************************************************************************************************************************/
/* Http_msgs_send_to_all: Envoi d'un buffer a tous les clients connectés à la websocket                                       */
/* Entrée: Le buffer                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_msgs_send_to_all ( gchar *buf )
  { GSList *liste = Cfg_http.liste_ws_msgs_clients;
    while (liste)
     { SoupWebsocketConnection *connexion = liste->data;
       soup_websocket_connection_send_text ( connexion, buf );
       liste = g_slist_next(liste);
     }
    g_free(buf);
  }
/******************************************************************************************************************************/
/* Http_traiter_log: Répond aux requetes sur l'URI log                                                                        */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_histo_alive ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data)
  { gchar chaine[512];
    gsize taille_buf;
    gint syn_id;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    Http_print_request ( server, msg, path, client );
    gpointer syn_id_src = g_hash_table_lookup ( query, "syn_id" );
    if (syn_id_src) syn_id = atoi(syn_id_src);
    else syn_id = 1;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonBuilder *builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }

    g_snprintf( chaine, sizeof(chaine),
                "SELECT histo.*, histo.alive, msg.libelle, msg.typologie, dls.syn_id,"
                "parent_syn.page as syn_parent_page, syn.page as syn_page,"
                "dls.shortname as dls_shortname, msg.tech_id, msg.acronyme"
                " FROM histo_msgs as histo"
                " INNER JOIN msgs as msg ON msg.id = histo.id_msg"
                " INNER JOIN dls as dls ON dls.tech_id = msg.tech_id"
                " INNER JOIN syns as syn ON syn.id = dls.syn_id"
                " INNER JOIN syns as parent_syn ON parent_syn.id = syn.parent_id"
                " WHERE alive = 1 AND syn.id=%d ORDER BY histo.date_create DESC", syn_id );
    if (SQL_Select_to_JSON ( builder, "enregs", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    gchar *buf = Json_get_buf (builder, &taille_buf);
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
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

       JsonBuilder *builder = Json_create ();
       if (builder == NULL)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: JSon builder creation failed", __func__ );
          soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
        }
       else
        { gsize taille_buf;
          Json_add_string ( builder, "zmq_tag", "DLS_HISTO" );
          Json_add_string ( builder, "tech_id", tech_id );
          Json_add_string ( builder, "acronyme", acronyme );
          Json_add_string ( builder, "nom_ack", session->username );
          Json_add_string ( builder, "date_fixe", date_fixe );
          gchar *buf = Json_get_buf ( builder, &taille_buf );
          Http_msgs_send_to_all ( buf );
          soup_message_set_status (msg, SOUP_STATUS_OK);
        }
     }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Recuperer_histo_msgsDB_alive: Recupération de l'ensemble des messages encore Alive dans le BDD                             */
/* Entrée: La base de données de travail                                                                                      */
/* Sortie: False si probleme                                                                                                  */
/******************************************************************************************************************************/
 void Http_ws_send_pulse_to_all ( void )
  { gsize taille_buf;
    JsonBuilder *builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: JSon builder creation failed", __func__ );
       return;
     }
    Json_add_string( builder, "zmq_tag", "pulse" );
    gchar *buf = Json_get_buf ( builder, &taille_buf );
    Http_msgs_send_to_all ( buf );
  }
/******************************************************************************************************************************/
/* Http_traiter_websocket: Traite une requete websocket                                                                       */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_open_websocket_msgs_CB ( SoupServer *server, SoupWebsocketConnection *connexion, const char *path,
                                            SoupClientContext *client, gpointer user_data)
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: MSGS WebSocket Opened %p state %d!", __func__, connexion,
              soup_websocket_connection_get_state (connexion) );
    g_signal_connect ( connexion, "closed", G_CALLBACK(Http_ws_msgs_on_closed), NULL );
    g_signal_connect ( connexion, "error",  G_CALLBACK(Http_ws_msgs_on_error),  NULL );
    Cfg_http.liste_ws_msgs_clients = g_slist_prepend ( Cfg_http.liste_ws_msgs_clients, connexion );
    g_object_ref(connexion);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
