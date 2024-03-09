/******************************************************************************************************************************/
/* Watchdogd/Include/Http.h        Déclaration structure internes des WebServices                                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http.h
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

#ifndef _HTTP_H_
 #define _HTTP_H_

 #define HTTP_DEFAUT_FILE_CERT         "https_bus_cert.pem"
 #define HTTP_DEFAUT_FILE_KEY          "https_bus_key.pem"
 #define HTTP_DEFAUT_TCP_PORT          5559

 struct COM_HTTP                                                                     /* Communication entre le serveur et DLS */
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                             /* TRUE si le thread doit tout logguer */
    SoupServer *local_socket;
    GMainLoop *loop;
    GMainContext *loop_context;
    GSList *liste_http_clients;
    GSList *Slaves;                                                               /* Liste des slaves connectés au l'instance */
    gint num_session;
  };

 struct HTTP_WS_SESSION
  { SoupWebsocketConnection *connexion;
  };

/*************************************************** Définitions des prototypes ***********************************************/

 extern void Run_HTTP ( void );

 extern void Http_traiter_status  ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data );
 extern void Http_traiter_dls_status ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data );
 extern void Http_traiter_dls_run    ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data );
 extern void Http_traiter_dls_run_set ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request );
 extern void Http_traiter_dls_run_acquitter ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request );

 extern void Http_traiter_set_di_post ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request );
 extern void Http_traiter_set_ai_post ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request );
 extern void Http_traiter_set_cde_post ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request );
 extern void Http_traiter_set_watchdog_post ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request );
 extern void Http_traiter_get_io ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query );

 extern JsonNode *Http_Msg_to_Json ( SoupServerMessage *msg );
 extern JsonNode *Http_Get_from_local_BUS ( struct THREAD *module, gchar *uri );
 extern gboolean Http_Post_to_local_BUS ( struct THREAD *module, gchar *uri, JsonNode *RootNode );
 extern void Http_Add_Thread_signature ( struct THREAD *module, SoupMessage *msg, gchar *buf, gint buf_size );
 extern gboolean Http_Check_Thread_signature ( gchar *path, SoupServerMessage *msg, gchar **thread_tech_id_p );
 extern void Http_Post_to_local_BUS_CDE ( struct THREAD *module, gchar *tech_id, gchar *acronyme );
 extern void Http_Post_thread_WATCHDOG_to_local_BUS ( struct THREAD *module, gchar *acronyme, gint consigne );

 extern void Http_traiter_open_websocket_for_slaves_CB ( SoupServer *server, SoupServerMessage *msg, const char* path,
                                                         SoupWebsocketConnection* connection, gpointer user_data );

 extern void Http_ws_send_json_to_slave ( struct HTTP_WS_SESSION *slave, JsonNode *node );
 extern void Http_Send_to_slaves ( gchar *tag, JsonNode *RootNode );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
