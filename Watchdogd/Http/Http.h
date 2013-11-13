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

 #define NOM_THREAD                    "http"
 #define HTTP_DEFAUT_FILE_CA           "http_cacert.pem"
 #define HTTP_DEFAUT_FILE_CERT         "http_serveursigne.pem"
 #define HTTP_DEFAUT_FILE_KEY          "http_serveurkey.pem"
 #define HTTP_DEFAUT_MAX_CONNEXION     100

 struct HTTP_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean satellite_enable;
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
    GSList *Liste_XML_docs;                      /* Une liste de document XML recu de la part des clients */
    GSList *Liste_satellites;                                                    /* Listes des satellites */
 } Cfg_http;

 enum HTTP_CONNEXION_TYPE
  { HTTP_CONNEXION_SET_INTERNAL = 1
  };

 struct HTTP_CONNEXION_INFO
  { gint     type;
    gboolean request_processed;                 /* TRUE si la requete est complete et doit etre processée */
    gchar   *buffer;                                   /* Le buffer recu dans le corps de la requete HTTP */
    gint     buffer_size;                                                    /* La taille utile du buffer */
  };

 struct SATELLITE_INFOS                          /* Informations liées aux connexions avec les Satellites */
  { gchar instance_id[80];
    gint  bit_state;
    guint last_top;
  };
/*************************************** Définitions des prototypes ***************************************/
 extern gboolean Http_Lire_config ( void );
 extern gboolean Http_Traiter_request_getsyn ( struct MHD_Connection *connection );
 extern gboolean Http_Traiter_request_getstatus ( struct MHD_Connection *connection );
 extern gboolean Http_Traiter_request_getgif ( struct MHD_Connection *connection );
 extern gboolean Http_Traiter_request_gifile ( struct MHD_Connection *connection );
 extern gboolean Http_Traiter_request_set_internal ( struct MHD_Connection *connection,
                                                     const char *upload_data, 
                                                     size_t *upload_data_size, void **con_cls );
 extern void Http_free_liste_satellites ( void );
 extern void Http_Check_satellites_states ( void );
 extern void Http_Traiter_XML_set_internal ( struct HTTP_CONNEXION_INFO *infos );
#endif
/*--------------------------------------------------------------------------------------------------------*/
