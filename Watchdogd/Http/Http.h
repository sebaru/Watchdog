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
 #include <microhttpd.h>

 #define HTTP_DEFAUT_FILE_CA           "http_cacert.pem"
 #define HTTP_DEFAUT_FILE_SERVER       "http_serveursigne.pem"
 #define HTTP_DEFAUT_FILE_KEY          "http_serveurkey.pem"
 #define HTTP_DEFAUT_MAX_CONNEXION     100

 struct HTTP_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean slave_enable;
    gint nbr_max_connexion;
    gboolean http_enable;                         /* True si la config indique que le thread doit tourner */
    gint http_port;
    gboolean https_enable;                        /* True si la config indique que le thread doit tourner */
    gint https_port;
    gchar https_file_cert[80];
    gchar *ssl_cert;
    gchar https_file_key[80];
    gchar *ssl_key;
    gchar https_file_ca[80];
    gchar *ssl_ca;
    struct MHD_Daemon *http_server;
    struct MHD_Daemon *https_server;
 } Cfg_http;

/*************************************** Définitions des prototypes ***************************************/
 extern gboolean Http_Traiter_request_getsyn ( struct MHD_Connection *connection );
 extern gboolean Http_Traiter_request_set_internal ( struct MHD_Connection *connection );
#endif
/*--------------------------------------------------------------------------------------------------------*/
