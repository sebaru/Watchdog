/******************************************************************************************************************************/
/* Watchdogd/Http/ws.c        Gestion des echanges des elements visuels de watchdog                                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    06.05.2020 09:53:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ws.c
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
/* Envoi_au_serveur: Envoi une requete web au serveur Watchdogd                                                               */
/* Entrée: des infos sur le paquet à envoyer                                                                                  */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Http_ws_send_to_client ( struct WS_CLIENT_SESSION *client, JsonNode *node )
  { gchar *buf = Json_node_to_string ( node );
    soup_websocket_connection_send_text ( client->connexion, buf );
    g_free(buf);
  }
/******************************************************************************************************************************/
/* Http_ws_send_to_all: Envoi d'un buffer a tous les clients connectés à la websocket                                         */
/* Entrée: Le buffer                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_ws_send_to_all ( JsonNode *node )
  { gchar *buf = Json_node_to_string ( node );
    pthread_mutex_lock( &Cfg_http.lib->synchro );
    GSList *sessions = Cfg_http.liste_http_clients;
    while ( sessions )
     { struct HTTP_CLIENT_SESSION *session = sessions->data;
       GSList *liste_ws = session->liste_ws_clients;
       while (liste_ws)
        { struct WS_CLIENT_SESSION *client = liste_ws->data;
          soup_websocket_connection_send_text ( client->connexion, buf );
          liste_ws = g_slist_next(liste_ws);
        }
       sessions = g_slist_next ( sessions );
     }
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
    g_free(buf);
  }
/******************************************************************************************************************************/
/* Envoyer_un_cadran: Envoi un update cadran au client                                                                        */
/* Entrée: une reference sur la session en cours, et le cadran a envoyer                                                      */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void HTTP_CADRAN_to_json ( JsonNode *node, struct HTTP_CADRAN *http_cadran )
  { Json_node_add_string ( node, "tech_id",  http_cadran->tech_id );
    Json_node_add_string ( node, "acronyme", http_cadran->acronyme );
    Json_node_add_string ( node, "classe",   http_cadran->classe );
    Json_node_add_bool   ( node, "in_range", http_cadran->in_range );
    Json_node_add_double ( node, "valeur",   http_cadran->valeur );
    Json_node_add_string ( node, "unite",    http_cadran->unite );
  }
/******************************************************************************************************************************/
/* Http_ws_on_message: Appelé par libsoup lorsque l'on recoit un message sur la websocket                              */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_ws_on_message ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { struct WS_CLIENT_SESSION *client = user_data;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Message received !", __func__ );
    gsize taille;

    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: WebSocket Message Dropped (not JSON) !", __func__ );
       return;
     }

    if (!Json_has_member ( response, "zmq_tag" ))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: WebSocket Message Dropped (no 'zmq_tag') !", __func__ );
       json_node_unref(response);
       return;
     }

    gchar *zmq_tag = Json_get_string( response, "zmq_tag" );

    if(!strcasecmp(zmq_tag,"CONNECT"))
     { if ( ! (Json_has_member( response, "wtd_session") ))
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: WebSocket without wtd_session !", __func__ ); }
       else
        { gchar *wtd_session = Json_get_string ( response, "wtd_session");
          GSList *liste = Cfg_http.liste_http_clients;
          while ( liste )                                                      /* Recherche de la session HTTP correspondante */
           { struct HTTP_CLIENT_SESSION *http_session = liste->data;
             if (!strcmp(http_session->wtd_session, wtd_session))
              { client->http_session = http_session;
                pthread_mutex_lock( &Cfg_http.lib->synchro );
                http_session->liste_ws_clients = g_slist_prepend ( http_session->liste_ws_clients, client );
                pthread_mutex_unlock( &Cfg_http.lib->synchro );
                Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: session found for '%s' !", __func__, http_session->username );
                break;
              }
             liste = g_slist_next ( liste );
           }
        }
     }

    if (!client->http_session)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: Not authorized !", __func__ ); }
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Http_Envoyer_les_cadrans: Envoi les cadrans aux clients                                                                    */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_Envoyer_les_cadrans ( void )
  { pthread_mutex_lock( &Cfg_http.lib->synchro );
    GSList *sessions = Cfg_http.liste_http_clients;
    while ( sessions )
     { struct HTTP_CLIENT_SESSION *session = sessions->data;
       GSList *cadrans = session->Liste_bit_cadrans;
       while ( cadrans )
        { struct HTTP_CADRAN *cadran = cadrans->data;
          if (cadran->last_update + 10 <= Partage->top)
           { GSList *clients = session->liste_ws_clients;
             JsonNode *RootNode = Json_node_create();
             if (RootNode)
              { Http_Formater_cadran ( cadran );
                Json_node_add_string ( RootNode, "zmq_tag", "DLS_CADRAN" );
                HTTP_CADRAN_to_json ( RootNode, cadran );
                while (clients)
                 { struct WS_CLIENT_SESSION *client = clients->data;
                   Http_ws_send_to_client ( client, RootNode );
                   clients = g_slist_next(clients);
                 }
                json_node_unref( RootNode );
              }
             cadran->last_update = Partage->top;
           }
          cadrans = g_slist_next(cadrans);
        }
       sessions = g_slist_next(sessions);
     }
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
  }
/******************************************************************************************************************************/
/* Http_ws_destroy_session: Supprime une session WS                                                                           */
/* Entrée: la WS                                                                                                              */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_ws_destroy_session ( struct WS_CLIENT_SESSION *client )
  { if (client->http_session)
     { struct HTTP_CLIENT_SESSION *session = client->http_session;
       pthread_mutex_lock( &Cfg_http.lib->synchro );
       session->liste_ws_clients = g_slist_remove ( session->liste_ws_clients, client );
       pthread_mutex_unlock( &Cfg_http.lib->synchro );
       g_slist_free ( client->Liste_bit_visuels );
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Session closed !", __func__ );
    g_object_unref(client->connexion);
    g_free(client);
  }
/******************************************************************************************************************************/
/* Http_ws_on_closed: Traite une deconnexion                                                                                  */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_ws_on_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { struct WS_CLIENT_SESSION *client = user_data;
    Http_ws_destroy_session ( client );
  }
 static void Http_ws_on_error ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Error received %p!", __func__, self );
  }
/******************************************************************************************************************************/
/* Http_traiter_websocket: Traite une requete websocket                                                                       */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_open_websocket_motifs_CB ( SoupServer *server, SoupWebsocketConnection *connexion, const char *path,
                                              SoupClientContext *context, gpointer user_data)
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Opened %p state %d!", __func__, connexion,
              soup_websocket_connection_get_state (connexion) );
    struct WS_CLIENT_SESSION *client = g_try_malloc0( sizeof(struct WS_CLIENT_SESSION) );
    if(!client)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: WebSocket Memory error. Closing !", __func__ );
       return;
     }
    client->connexion = connexion;
    client->context   = context;
    g_signal_connect ( connexion, "message", G_CALLBACK(Http_ws_on_message), client );
    g_signal_connect ( connexion, "closed",  G_CALLBACK(Http_ws_on_closed), client );
    g_signal_connect ( connexion, "error",   G_CALLBACK(Http_ws_on_error), client );
    /*soup_websocket_connection_send_text ( connexion, "Welcome on Watchdog WebSocket !" );*/
    g_object_ref(connexion);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/