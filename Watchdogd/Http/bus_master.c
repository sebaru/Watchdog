/******************************************************************************************************************************/
/* Watchdogd/Http/bus_master.c        Gestion des request BUS depuis le master                                                */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    06.05.2020 09:53:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * bus_master.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Envoi_au_serveur: Envoi une requete web au serveur Watchdogd                                                               */
/* Entrée: des infos sur le paquet à envoyer                                                                                  */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Http_ws_send_json_to_slave ( struct HTTP_WS_SESSION *slave, JsonNode *RootNode )
  { gchar *buffer = Json_node_to_string ( RootNode );
    soup_websocket_connection_send_text ( slave->connexion, buffer );
    g_free(buffer);
  }
/******************************************************************************************************************************/
/* Http_Send_to_slaves: Envoi un tag aux slaves                                                                               */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_Send_to_slaves ( gchar *tag, JsonNode *RootNode )
  { gboolean unref_RootNode = FALSE;
    if (!RootNode) { RootNode = Json_node_create (); unref_RootNode = TRUE; }
    Json_node_add_string ( RootNode, "tag", tag );
    gchar *buffer = Json_node_to_string ( RootNode );
    if (unref_RootNode) Json_node_unref (RootNode);

    pthread_mutex_lock( &Partage->com_http.synchro );
    GSList *liste = Partage->com_http.Slaves;
    while ( liste )
     { struct HTTP_WS_SESSION *slave = liste->data;
       soup_websocket_connection_send_text ( slave->connexion, buffer );
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock( &Partage->com_http.synchro );
    g_free(buffer);
  }
/******************************************************************************************************************************/
/* Http_ws_on_slave_message: Appelé par libsoup lorsque l'on recoit un message sur la websocket slave                         */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_ws_on_slave_message ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { /*struct HTTP_WS_SESSION *slave = user_data;*/
    Info_new( __func__, Config.log_msrv, LOG_INFO, "WebSocket Message received !" );
    gsize taille;
    gchar *buffer = g_bytes_get_data ( message_brut, &taille );
    JsonNode *response = Json_get_from_string ( buffer );
    if (!response)
     { if (taille) buffer[taille-1] = 0;
       Info_new( __func__, Config.log_msrv, LOG_WARNING, "WebSocket Message Dropped (not JSON): %s !", buffer );
       return;
     }

    if (!Json_has_member ( response, "tag" ))
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "WebSocket Message Dropped (no 'tag') !" );
       Json_node_unref(response);
       return;
     }

    Info_new( __func__, Config.log_msrv, LOG_INFO, "receive tag '%s'  !", Json_get_string ( response, "tag" ) );

    Json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Http_ws_destroy_slave_session: Supprime une session slave                                                                  */
/* Entrée: la WS                                                                                                              */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_ws_destroy_slave_session ( struct HTTP_WS_SESSION *slave )
  { pthread_mutex_lock( &Partage->com_http.synchro );
    Partage->com_http.Slaves = g_slist_remove ( Partage->com_http.Slaves, slave );
    pthread_mutex_unlock( &Partage->com_http.synchro );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "WebSocket Session closed !" );
    g_object_unref(slave->connexion);
    g_free(slave);
  }
/******************************************************************************************************************************/
/* Http_ws_on_slave_closed: Traite une deconnexion slave                                                                      */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_ws_on_slave_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { struct HTTP_WS_SESSION *slave = user_data;
    Http_ws_destroy_slave_session ( slave );
  }
 static void Http_ws_on_slave_error ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { Info_new( __func__, Config.log_msrv, LOG_INFO, "WebSocket Error received %p!", self );
  }
/******************************************************************************************************************************/
/* Http_traiter_open_websocket_slaves_CB: Traite une requete websocket depuis les slaves                                      */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_open_websocket_for_slaves_CB ( SoupServer *server, SoupServerMessage *msg, const char* path,
                                                  SoupWebsocketConnection* connexion, gpointer user_data )
  { Info_new( __func__, Config.log_msrv, LOG_INFO, "WebSocket Opened %p state %d!", connexion,
              soup_websocket_connection_get_state (connexion) );
    struct HTTP_WS_SESSION *slave = g_try_malloc0( sizeof(struct HTTP_WS_SESSION) );
    if(!slave)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "WebSocket Memory error. Closing !" );
       return;
     }
    slave->connexion = connexion;
    g_signal_connect ( connexion, "message", G_CALLBACK(Http_ws_on_slave_message), slave );
    g_signal_connect ( connexion, "closed",  G_CALLBACK(Http_ws_on_slave_closed), slave );
    g_signal_connect ( connexion, "error",   G_CALLBACK(Http_ws_on_slave_error), slave );
    /*soup_websocket_connection_send_text ( connexion, "Welcome on Watchdog WebSocket !" );*/
    pthread_mutex_lock( &Partage->com_http.synchro );
    Partage->com_http.Slaves = g_slist_prepend ( Partage->com_http.Slaves, slave );
    pthread_mutex_unlock( &Partage->com_http.synchro );
    g_object_ref(connexion);
    Info_new( __func__, Config.log_msrv, LOG_INFO, "WebSocket Listening" );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
