
/******************************************************************************************************************************/
/* Watchdogd/Http/histo_msgs.c        Gestion des echanges des elements d'historiques/messages de watchdog                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    01.05.2020 09:53:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * histo_msgs.c
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
 void Http_traiter_histo_ack ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data)
  { GBytes *request_brute;
    gsize taille;
    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    Http_print_request ( server, msg, path, client );

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

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

       gchar *username = soup_client_context_get_auth_user (client);
       if (!username) username = "unknown";

       Acquitter_histo_msgsDB ( tech_id, acronyme, username, date_fixe );

       JsonBuilder *builder = Json_create ();
       if (builder == NULL)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: JSon builder creation failed", __func__ );
          soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
        }
       else
        { gsize taille_buf;
          Json_add_string ( builder, "zmq_type", "update_histo" );
          Json_add_string ( builder, "tech_id", tech_id );
          Json_add_string ( builder, "acronyme", acronyme );
          Json_add_string ( builder, "nom_ack", username );
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
 void Http_msgs_send_histo_to_all ( struct CMD_TYPE_HISTO *histo )
  { gsize taille_buf;
    JsonBuilder *builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: JSon builder creation failed", __func__ );
       return;
     }
    Json_add_string( builder, "zmq_type", "update_histo" );
    Histo_msg_print_to_JSON ( builder, histo );
    gchar *buf = Json_get_buf ( builder, &taille_buf );
    Http_msgs_send_to_all ( buf );
  }
/******************************************************************************************************************************/
/* Recuperer_histo_msgsDB_alive: Recupération de l'ensemble des messages encore Alive dans le BDD                             */
/* Entrée: La base de données de travail                                                                                      */
/* Sortie: False si probleme                                                                                                  */
/******************************************************************************************************************************/
 void Http_msgs_send_pulse_to_all ( void )
  { gsize taille_buf;
    JsonBuilder *builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: JSon builder creation failed", __func__ );
       return;
     }
    Json_add_string( builder, "zmq_type", "pulse" );
    gchar *buf = Json_get_buf ( builder, &taille_buf );
    Http_msgs_send_to_all ( buf );
  }
/******************************************************************************************************************************/
/* Recuperer_histo_msgsDB_alive: Recupération de l'ensemble des messages encore Alive dans le BDD                             */
/* Entrée: La base de données de travail                                                                                      */
/* Sortie: False si probleme                                                                                                  */
/******************************************************************************************************************************/
 static void Http_ws_msgs_send_histo_alive ( SoupWebsocketConnection *connexion )
  { struct CMD_TYPE_HISTO *histo;
    gsize taille_buf;
    struct DB *db;
    gchar *buf;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonBuilder *builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }

    if ( ! Recuperer_histo_msgsDB_alive( &db ) )                                                     /* Si pas de histos (??) */
     { goto end; }

    Json_add_string( builder, "zmq_type", "load_histo_alive" );
    Json_add_int   ( builder, "nbr_enreg", db->nbr_result );
    Json_add_array ( builder, "enregs" );
    while ( (histo = Recuperer_histo_msgsDB_suite( &db )) != NULL)
     { json_builder_begin_object ( builder );
       Histo_msg_print_to_JSON ( builder, histo );
       json_builder_end_object (builder);
     }
    Json_end_array( builder );
end:
    buf = Json_get_buf (builder, &taille_buf);
    soup_websocket_connection_send_text ( connexion, buf );
    g_free(buf);
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
    g_signal_connect ( connexion, "closed",  G_CALLBACK(Http_ws_msgs_on_closed), NULL);
    Cfg_http.liste_ws_msgs_clients = g_slist_prepend ( Cfg_http.liste_ws_msgs_clients, connexion );
    Http_ws_msgs_send_histo_alive ( connexion );
    g_object_ref(connexion);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
