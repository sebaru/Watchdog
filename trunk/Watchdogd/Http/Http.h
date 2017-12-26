/******************************************************************************************************************************/
/* Watchdogd/HttpMobile/HttpMobile.h        Déclaration structure internes des WebServices                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * httpmobile.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 #include <libwebsockets.h>
 #include <libxml/xmlwriter.h>
 #include <json-glib/json-glib.h>

 #define NOM_THREAD                    "http"
 #define HTTP_DEFAUT_FILE_CA           "http_cacert.pem"
 #define HTTP_DEFAUT_FILE_CERT         "http_serveursigne.pem"
 #define HTTP_DEFAUT_FILE_KEY          "http_serveurkey.pem"
 #define HTTP_DEFAUT_SSL_CIPHER        "HIGH:NORMAL"
 #define HTTP_DEFAUT_MAX_CONNEXION     16
 #define HTTP_DEFAUT_TCP_PORT          5560
 #define HTTP_DEFAUT_MAX_UPLOAD_BYTES  10240000
 #define HTTP_DEFAUT_LWS_DEBUG_LEVEL   0

 #define HTTP_200_OK                   200
 #define HTTP_BAD_REQUEST              400
 #define HTTP_UNAUTHORIZED             401
 #define HTTP_BAD_METHOD               405
 #define HTTP_SERVER_ERROR             100

 #define HTTP_CONTENT_JSON             "application/json"
 #define HTTP_CONTENT_XML              "application/xml"
 
 enum WS_PROTO
  {	/* always first */
    WS_PROTO_HTTP = 0,
    WS_PROTO_STATUS,
    /* always last */
    WS_NBR_PROTO
  };

 struct HTTP_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean Thread_reload;                                              /* TRUE si le thread doit recharger sa configuration */

    struct lws_context_creation_info ws_info;                                             /* Paramétrage du context WebSocket */
    struct lws_context *ws_context;                                                                      /* Context WebSocket */
    
    gint nbr_max_connexion;                                           /* Nombre maximum de connexion autorisées simultanément */
    gint max_upload_bytes;                                                                   /* Taille max du fichier uploadé */
    gint lws_debug_level;                                                              /* Niveau de debug de la librairie LWS */
    gint tcp_port;                                           /* Port d'écoute TCP (HTTP ou HTTPS, selon le paramètre suivant) */
    gboolean ssl_enable;                                                                             /* TRUE si connexion SSL */
    gchar ssl_cipher_list[128];
    gchar ssl_cert_filepath[80];
    gchar ssl_private_key_filepath[80];
    gchar ssl_ca_filepath[80];
    gboolean authenticate;
    GSList *Liste_sessions;
 } Cfg_http;

 struct HTTP_PER_SESSION_DATA
  { gchar url[80];
    struct lws_spa *spa;
    struct HTTP_SESSION *session;
    gchar *post_data;
    gint post_data_length;
  };

 struct HTTP_SESSION
  { /*gint     type;*/
    gchar    sid[2*EVP_MAX_MD_SIZE+1];
    gchar    sid_string[120];
    gchar    remote_name[80];
    gchar    remote_ip[20];
    gchar    user_agent[120];
    gint     is_ssl;
    struct   CMD_TYPE_UTILISATEUR *util;                                               /* Utilisateur authentifié (via HTTPS) */
    gint     last_top;                                                                         /* Date de la derniere requete */
  };

/*************************************************** Définitions des prototypes ***********************************************/
 extern gboolean Http_Lire_config ( void );
 extern gint Http_json_get_int ( JsonObject *object, gchar *name );
 extern void Http_Send_response_code ( struct lws *wsi, gint code );
 extern void Http_Send_response_code_with_buffer ( struct lws *wsi, gint code, gchar *content_type, gchar *buffer, gint taille_buf );
 extern gint Http_CB_file_upload( struct lws *wsi, char *buffer, int taille );
 extern gboolean Http_Traiter_request_getsyn ( struct lws *wsi, struct HTTP_SESSION *session );
 extern gboolean Http_Traiter_request_getstatus ( struct lws *wsi );
 extern gint Http_Traiter_request_getgif ( struct lws *wsi, gchar *remote_name, gchar *remote_ip, gchar *url );
 extern gint Http_Traiter_request_getsvg ( struct lws *wsi, struct HTTP_SESSION *session );
 extern gint Http_Traiter_request_getaudio ( struct lws *wsi, gchar *remote_name, gchar *remote_ip, gchar *url );
 extern gint Http_Traiter_request_body_cli ( struct lws *wsi, void *data, size_t taille );
 extern gint Http_Traiter_request_body_completion_cli ( struct lws *wsi );
 extern gint Http_Traiter_request_body_postfile ( struct lws *wsi, void *data, size_t taille );
 extern gint Http_Traiter_request_body_completion_postfile ( struct lws *wsi );
 extern gint Http_Traiter_request_getui ( struct lws *wsi, gchar *remote_name, gchar *remote_ip, gchar *url );
 extern gint Http_Traiter_request_setm ( struct lws *wsi );
  
 extern struct HTTP_SESSION *Http_get_session ( struct lws *wsi, gchar *remote_name, gchar *remote_ip );
 extern gchar *Http_get_session_id ( struct HTTP_SESSION *session );
 extern void Http_Check_sessions ( void );
 extern void Http_Liberer_session ( struct HTTP_SESSION *session );
 extern void Http_Close_session ( struct lws *wsi, struct HTTP_SESSION *session );

 extern gint Http_Traiter_request_login ( struct HTTP_SESSION *session, struct lws *wsi, gchar *remote_name, gchar *remote_ip );
 extern gint Http_Traiter_request_body_login ( struct lws *wsi, void *data, size_t taille );
 extern gint Http_Traiter_request_body_completion_login ( struct lws *wsi, gchar *remote_name, gchar *remote_ip );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
