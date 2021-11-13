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
/* Http_Traiter_request_getgif: Traite une requete sur l'URI getgif                                       */
/* Entrées: la connexion MHD                                                                              */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 gint Http_Traiter_request_getgif ( struct MHD_Connection *connection )
  { const gchar *id_char, *mode_char;
    struct MHD_Response *response;
    gchar nom_fichier[80];
    gint id, mode, fd;
    struct stat sbuf;

    id_char   = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "id" );
    mode_char = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "mode" );

    if (!id_char)   { id = 1; }
               else { id = atoi(id_char); }
    if (!mode_char) { mode = 0; }
               else { mode = atoi(mode_char); }

    if (mode) { g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%d.gif.%02d", id, mode ); }
         else { g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%d.gif", id ); }

    fd = open ( nom_fichier, O_RDONLY);
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Traiter_request_getgif : Error /getgif error %s on file %s", strerror(errno), nom_fichier );
       if (fd!=-1) close(fd);
       response = MHD_create_response_from_buffer ( strlen (RESPONSE_INTERNAL_ERROR)+1,
                                                       RESPONSE_INTERNAL_ERROR, MHD_RESPMEM_PERSISTENT);
       if (response == NULL) return(MHD_NO);
       Http_Add_response_header ( response );
       MHD_queue_response ( connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
       MHD_destroy_response (response);
       return(MHD_YES);
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "Http_Traiter_request_getgif : Request for /getgif id %d mode %d file %s OK",
              id, mode, nom_fichier );
       
    response = MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
    MHD_add_response_header (response, "Content-Type", "image/gif");
    Http_Add_response_header ( response );
    MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return(MHD_YES);
  }
/*--------------------------------------------------------------------------------------------------------*/
