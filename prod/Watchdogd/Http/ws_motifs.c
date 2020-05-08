/******************************************************************************************************************************/
/* Watchdogd/Http/ws_motifs.c        Gestion des echanges des elements visuels de watchdog                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    06.05.2020 09:53:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ws_motifs.c
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

 static void Http_ws_motifs_on_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Close Connexion received !", __func__ );
    g_object_unref(connexion);
    Cfg_http.liste_ws_motifs_clients = g_slist_remove ( Cfg_http.liste_ws_motifs_clients, connexion );
  }

 static void Http_ws_motifs_on_message ( SoupWebsocketConnection *connexion, gint type, GBytes *message, gpointer user_data )
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Message received !", __func__ );
  }

 static void Http_ws_motifs_on_error ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Error received %p!", __func__, self );
  }

/******************************************************************************************************************************/
/* Http_traiter_websocket: Traite une requete websocket                                                                       */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_open_websocket_motifs_CB ( SoupServer *server, SoupWebsocketConnection *connexion, const char *path,
                                              SoupClientContext *client, gpointer user_data)
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Opened %p state %d!", __func__, connexion,
              soup_websocket_connection_get_state (connexion) );
    g_signal_connect ( connexion, "message", G_CALLBACK(Http_ws_motifs_on_message), NULL);
    g_signal_connect ( connexion, "closed",  G_CALLBACK(Http_ws_motifs_on_closed), NULL);
    g_signal_connect ( connexion, "error",   G_CALLBACK(Http_ws_motifs_on_error), NULL);
    /*soup_websocket_connection_send_text ( connexion, "Welcome on Watchdog WebSocket !" );*/
    Cfg_http.liste_ws_motifs_clients = g_slist_prepend ( Cfg_http.liste_ws_motifs_clients, connexion );
    g_object_ref(connexion);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
