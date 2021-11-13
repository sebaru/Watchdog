/******************************************************************************************************************************/
/* Watchdogd/HttpMobile/Http.h        Déclaration structure internes des WebServices                                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http.h
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

#ifndef _HTTP_H_
 #define _HTTP_H_
 #include <libsoup/soup.h>
 #include <json-glib/json-glib.h>
 #include <uuid/uuid.h>

 #define NOM_THREAD                    "http"
 #define HTTP_DEFAUT_FILE_CA           "http_cacert.pem"
 #define HTTP_DEFAUT_FILE_CERT         "http_serveursigne.pem"
 #define HTTP_DEFAUT_FILE_KEY          "http_serveurkey.pem"
 #define HTTP_DEFAUT_TCP_PORT          5560
 #define HTTP_FORBIDDEN_ERROR          "Session inexistante ou droits insuffisants"
 struct WS_CLIENT_SESSION
  { SoupWebsocketConnection *connexion;
    SoupClientContext *context;
    GSList *Liste_bit_cadrans;
    GSList *Liste_bit_motifs;
  };

 struct HTTP_CLIENT_SESSION
  { gchar username[32];
    gchar wtd_session[42];
    gint  access_level;
    gint  last_request;
  };

 struct HTTP_CONFIG
  { struct LIBRAIRIE *lib;

    struct ZMQUEUE *zmq_from_bus;                                                                       /* Envoi vers le msrv */
    struct ZMQUEUE *zmq_to_master;                                                                      /* Envoi vers le msrv */
    gint tcp_port;                                           /* Port d'écoute TCP (HTTP ou HTTPS, selon le paramètre suivant) */
    gboolean ssl_enable;                                                                             /* TRUE si connexion SSL */
    gchar ssl_cert_filepath[80];
    gchar ssl_private_key_filepath[80];
    GSList *liste_ws_motifs_clients;
    GSList *liste_ws_msgs_clients;
    GSList *liste_http_clients;
    gint wtd_session_expiry;
 };

/*************************************************** Définitions des prototypes ***********************************************/
 extern gboolean Http_Lire_config ( void );
 extern void Http_traiter_status  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls     ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_process ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_instance_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                          SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_bus     ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_memory  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_mnemos_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_show ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_get  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_set  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_update_motifs ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                              SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_clic ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_archive_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_users_list     ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_users_kill     ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_users_sessions ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_msgs_send_histo_to_all ( struct CMD_TYPE_HISTO *histo );
 extern void Http_msgs_send_pulse_to_all ( void );
 extern void Http_traiter_open_websocket_msgs_CB ( SoupServer *server, SoupWebsocketConnection *connexion, const char *path,
                                                   SoupClientContext *client, gpointer user_data);
 extern void Http_traiter_open_websocket_motifs_CB ( SoupServer *server, SoupWebsocketConnection *connexion, const char *path,
                                                     SoupClientContext *client, gpointer user_data);
 extern void Http_traiter_histo_ack ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                      SoupClientContext *client, gpointer user_data);
 extern void Http_traiter_histo_alive ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data);
 extern void Http_Memory_get_all ( SoupMessage *msg, gchar *tech_id );
 extern struct HTTP_CLIENT_SESSION *Http_print_request ( SoupServer *server, SoupMessage *msg, const char *path, SoupClientContext *client );
 extern void Http_Envoyer_les_cadrans ( void );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
