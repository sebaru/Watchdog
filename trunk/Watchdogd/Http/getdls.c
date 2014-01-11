/**********************************************************************************************************/
/* Watchdogd/Http/getid.c       Gestion des request getid pour le thread HTTP de watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 11 janv. 2014 22:17:02 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * getid.c
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
 gboolean Http_Traiter_request_getid ( struct MHD_Connection *connection )
  { struct MHD_Response *response;
    const gchar *id_char;
    gchar nom_fichier[80];
    gint id, fd;
    struct stat sbuf;

    id_char  = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "id" );

    if (!id_char)  { id = 1; }
              else { id = atoi(id_char); }

    g_snprintf( nom_fichier, sizeof(nom_fichier), "Dls/%d.dls", id );

    fd = open ( nom_fichier, O_RDONLY);
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "Http_request : Error /getdls error %s on file %s", strerror(errno), nom_fichier );
       if (fd!=-1) close(fd);
       return(FALSE);
     }

    response = MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
    Http_Add_titanium_response_header ( connection, response );
    MHD_add_response_header (response, "Content-Type", "text/plain");
    MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
