/**********************************************************************************************************/
/* Watchdogd/Http/getgif.c       Gestion des request getgif pour le thread HTTP de watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 05 mai 2013 16:33:43 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * getgif.c
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
 
 #include <string.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <microhttpd.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Http.h"
/**********************************************************************************************************/
/* Traiter_dynxml: Traite une requete sur l'URI dynxml                                                    */
/* Entrées: la connexion MHD                                                                              */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 gboolean Http_Traiter_request_getgif ( struct MHD_Connection *connection )
  { struct MHD_Response *response;
    const gchar *gif_char, *mode_char, *x_titanium;
    gchar nom_fichier[80];
    gint gif, mode, fd;
    struct stat sbuf;

    gif_char  = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "gif" );
    mode_char = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "mode" );

    if (!gif_char)  { gif = 1; }
               else { gif = atoi(gif_char); }
    if (!mode_char) { mode = 0; }
               else { mode = atoi(mode_char); }

    if (mode) { g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%d.gif.%02d", gif, mode ); }
         else { g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%d.gif", gif ); }

    fd = open ( nom_fichier, O_RDONLY);
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "Http_request : Error /gif error %s on file %s", strerror(errno), nom_fichier );
       if (fd!=-1) close(fd);
       return(FALSE);
     }

    response = MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
    MHD_add_response_header (response, "Content-Type", "image/gif");
    x_titanium = MHD_lookup_connection_value (connection, MHD_HEADER_KIND, "X-Titanium-Id");
    if (!x_titanium) x_titanium = "unknown";
    MHD_add_response_header ( response, "Access-Control-Allow-Origin", "*" );
    MHD_add_response_header ( response, "X-Titanium-Id", x_titanium);
    MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Http_Traiter_request_gifile: Traite une requete sur l'URI gifile                                       */
/* Entrées: la connexion MHD                                                                              */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 gboolean Http_Traiter_request_gifile ( struct MHD_Connection *connection )
  { struct MHD_Response *response;
    struct stat sbuf;
    gint fd;

    fd = open ("anna.jpg", O_RDONLY);
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "Http_request : Error /gifile %s", strerror(errno) );
       if (fd!=-1) close(fd);
       return(FALSE);
     }
    response = MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
    MHD_add_response_header (response, "Content-Type", "image/jpg");
    MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
