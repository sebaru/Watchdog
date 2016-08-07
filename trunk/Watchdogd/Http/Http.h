/**********************************************************************************************************/
/* Watchdogd/HttpMobile/HttpMobile.h        Déclaration structure internes des HttpMobile                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                   mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

 #define NOM_THREAD                    "http"
 #define HTTP_DEFAUT_FILE_CA           "http_cacert.pem"
 #define HTTP_DEFAUT_FILE_CERT         "http_serveursigne.pem"
 #define HTTP_DEFAUT_FILE_KEY          "http_serveurkey.pem"
 #define HTTP_DEFAUT_SSL_CIPHER        "HIGH:NORMAL"
 #define HTTP_DEFAUT_MAX_CONNEXION     16
 #define HTTP_DEFAUT_TCP_PORT          5560

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
    gint tcp_port;                                           /* Port d'écoute TCP (HTTP ou HTTPS, selon le paramètre suivant) */
    gboolean ssl_enable;                                                                             /* TRUE si connexion SSL */
    gchar ssl_cipher_list[128];
    gchar ssl_cert_filepath[80];
    gchar ssl_private_key_filepath[80];
    gchar ssl_ca_filepath[80];
    gboolean authenticate;
 } Cfg_http;

 struct HTTP_SESSION
  { gint     type;
    gchar    sid[65];
    gchar    client_host[80];
    gchar    client_service[20];
    gchar    user_agent[120], origine[80];
    gint     ssl_algo, ssl_proto;
    gchar    *buffer;                                                      /* Le buffer recu dans le corps de la requete HTTP */
    guchar   buffer_size;                                                                        /* La taille utile du buffer */
    struct   CMD_TYPE_UTILISATEUR *util;                                               /* Utilisateur authentifié (via HTTPS) */
    gint     last_top;                                                                         /* Date de la derniere requete */
  };

/*************************************** Définitions des prototypes ***************************************/
 extern gboolean Http_Lire_config ( void );
/* extern gboolean Http_Traiter_request_getsyn ( struct HTTP_SESSION *session, struct MHD_Connection *connection );*/
 extern gboolean Http_Traiter_request_getstatus ( struct lws *wsi );
 extern gint Http_Traiter_request_getgif ( struct lws *wsi, gchar *remote_name, gchar *remote_ip, gchar *url );
 extern gint Http_Traiter_request_getui ( struct lws *wsi, gchar *remote_name, gchar *remote_ip, gchar *url );
 extern gint Http_Traiter_request_getaudio ( struct lws *wsi, gchar *remote_name, gchar *remote_ip, gchar *url );
 /* extern gint Http_Traiter_request_getslash ( struct HTTP_SESSION *session, struct MHD_Connection *connection );
 extern gint Http_Traiter_request_getgif ( struct MHD_Connection *connection );
 extern gboolean Http_Traiter_request_setm ( struct HTTP_SESSION *session, struct MHD_Connection *connection );
 extern void Http_Add_response_header ( struct MHD_Response *response );
 extern void Http_free_liste_satellites ( void );
 extern void Http_Check_satellites_states ( void );
 extern void Http_Traiter_XML_set_internal ( struct HTTP_SESSION *session );*/
#endif
/*--------------------------------------------------------------------------------------------------------*/
