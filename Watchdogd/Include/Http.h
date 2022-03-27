/******************************************************************************************************************************/
/* Watchdogd/Include/Http.h        Déclaration structure internes des WebServices                                             */
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

 #define HTTP_DEFAUT_FILE_CERT         "https_api_cert.pem"
 #define HTTP_DEFAUT_FILE_KEY          "https_api_key.pem"
 #define HTTP_DEFAUT_TCP_PORT          5560

 struct COM_HTTP                                                                     /* Communication entre le serveur et DLS */
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                             /* TRUE si le thread doit tout logguer */
    SoupServer *socket;
    SoupServer *local_socket;
    GMainLoop *loop;
    GMainContext *loop_context;
    GSList *liste_http_clients;
    GSList *Slaves;                                                               /* Liste des slaves connectés au l'instance */
    gint num_session;
  };

 struct WS_CLIENT_SESSION
  { SoupWebsocketConnection *connexion;
    SoupClientContext *context;
    GSList *Liste_bit_visuels;
    struct HTTP_CLIENT_SESSION *http_session;
  };

 struct HTTP_CLIENT_SESSION
  { gint id;
    gchar username[32];
    gchar appareil[32];
    gchar useragent[128];
    gchar host[32];
    gchar wtd_session[42];
    gint  access_level;
    GSList *Liste_bit_cadrans;
    time_t last_request;
    GSList *liste_ws_clients;
  };

 struct HTTP_WS_SESSION
  { SoupWebsocketConnection *connexion;
    SoupClientContext *context;
  };

/******************************************************************************************************************************/
 struct HTTP_CADRAN
  { gchar tech_id[32];
    gchar acronyme[64];
    gchar unite[32];
    gchar classe[12];
    gpointer dls_data;
    gdouble  valeur;
    gboolean in_range;
    gint last_update;
  };

/*************************************************** Définitions des prototypes ***********************************************/
 extern JsonNode *Http_Msg_to_Json ( SoupMessage *msg );                                                       /* Dans http.c */
 extern JsonNode *Http_Response_Msg_to_Json ( SoupMessage *msg );
 extern gint Http_Msg_status_code ( SoupMessage *msg );
 extern gchar *Http_Msg_reason_phrase ( SoupMessage *msg );
 extern struct HTTP_CLIENT_SESSION *Http_rechercher_session_by_msg ( SoupMessage *msg );
 extern struct HTTP_CLIENT_SESSION *Http_print_request ( SoupServer *server, SoupMessage *msg, const char *path, SoupClientContext *client );
 extern gboolean Http_check_session ( SoupMessage *msg, struct HTTP_CLIENT_SESSION *session, gint min_access_level );
 extern void Run_HTTP ( void );
 extern JsonNode *Http_Post_to_global_API ( gchar *URI, gchar *api_tag, JsonNode *RootNode );
 extern JsonNode *Http_Get_from_global_API ( gchar *URI, gchar *parametres );

 extern void Http_traiter_status  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_run    ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_run_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_start ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                      SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_stop ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_debug ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                      SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_undebug ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_source ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_compil ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_dls_acquitter ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                          SoupClientContext *client, gpointer user_data );

 extern void Http_traiter_archive_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_archive_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_archive_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_archive_table_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                                 SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_archive_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_archive_clear ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                          SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_archive_purge ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                          SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_archive_testdb ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );

 extern void Http_traiter_process_config ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_process_send ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                         SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_process_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                         SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_process_enable ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_process_debug ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                          SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_process_reload ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_instance_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                          SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_instance_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                         SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_instance_reload_icons ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                                  SoupClientContext *client, gpointer user_data);
 extern void Http_traiter_bus     ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_mnemos_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_mnemos_tech_id ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_mnemos_set  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_mnemos_validate ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                            SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_get  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_set  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_save ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_clic ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_show ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_syn_ack ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data);
 extern void Http_traiter_horloge_get  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                         SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_horloge_ticks_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                              SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_horloge_ticks_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                              SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_horloge_ticks_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                               SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_tableau_list  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                          SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_tableau_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_tableau_set  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                         SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_tableau_map_list  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                              SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_tableau_map_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                            SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_tableau_map_set  ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                             SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_archive_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_archive_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_users_list     ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_users_set      ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_users_get      ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_users_add      ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_users_del      ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_users_kill     ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_users_sessions ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                           SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_open_websocket_motifs_CB ( SoupServer *server, SoupWebsocketConnection *connexion, const char *path,
                                                     SoupClientContext *client, gpointer user_data);
 extern void Http_traiter_histo_ack ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                      SoupClientContext *client, gpointer user_data);
 extern void Http_traiter_histo_alive ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data);
 extern void Http_traiter_tech ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_file ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_upload ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_config_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_config_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_config_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_log_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data);
 extern void Http_traiter_install ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data );
 extern void Http_traiter_map ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data );

 extern gboolean Http_Post_to_local_BUS ( struct SUBPROCESS *module, gchar *bus_tag, JsonNode *RootNode );
 extern void Http_Post_to_local_BUS_DI ( struct SUBPROCESS *module, JsonNode *di, gboolean etat );
 extern void Http_Post_to_local_BUS_AI ( struct SUBPROCESS *module, JsonNode *ai, gdouble valeur, gboolean in_range );
 extern void Http_Post_to_local_BUS_CDE ( struct SUBPROCESS *module, gchar *tech_id, gchar *acronyme );
 extern void Http_Post_to_local_BUS_WATCHDOG ( struct SUBPROCESS *module, gchar *tech_id, gchar *acronyme, gint consigne );

 extern void Http_traiter_open_websocket_for_slaves_CB ( SoupServer *server, SoupWebsocketConnection *connexion, const char *path,
                                                         SoupClientContext *client, gpointer user_data);

 extern struct HTTP_CLIENT_SESSION *Http_print_request ( SoupServer *server, SoupMessage *msg, const char *path, SoupClientContext *client );
 extern gboolean Http_check_session ( SoupMessage *msg, struct HTTP_CLIENT_SESSION * session, gint min_access_level );
 extern void Http_Envoyer_les_cadrans ( void );
 extern void Http_Send_json_response ( SoupMessage *msg, JsonNode *RootNode );
 extern void Http_ws_destroy_session ( struct WS_CLIENT_SESSION *client );
 extern void Http_ws_send_to_all ( JsonNode *node );
 extern void Http_Formater_cadran( struct HTTP_CADRAN *cadran );
 extern void Audit_log ( struct HTTP_CLIENT_SESSION *session, gchar *format, ... );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
