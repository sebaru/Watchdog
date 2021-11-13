/**********************************************************************************************************/
/* Watchdogd/Http/getstatus.c       Gestion des request getstatus pour le thread HTTP de watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 15 juin 2013 11:44:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * getstatus.c
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
 gint Http_Traiter_request_getslash ( struct HTTP_SESSION *session, struct MHD_Connection *connection )
  { struct MHD_Response *response;
    gchar nom_fichier[80];
    gint fd;
    struct stat sbuf;

    if ( ((!session) || (!session->util) || (Tester_groupe_util(session->util, GID_TOUTLEMONDE)==FALSE))
       )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Traiter_request_getslash : Error / permission denied" );
       response = MHD_create_response_from_buffer ( strlen (RESPONSE_AUTHENTICATION_NEEDED)+1,
                                                     (void*)RESPONSE_AUTHENTICATION_NEEDED, MHD_RESPMEM_PERSISTENT);
       if (response == NULL) return(MHD_NO);
       Http_Add_response_header ( response );
       MHD_queue_response ( connection, MHD_HTTP_UNAUTHORIZED, response);
       MHD_destroy_response (response);
       return(MHD_YES);
     }
     
    g_snprintf( nom_fichier, sizeof(nom_fichier), "WEB/ui.html" );

    fd = open ( nom_fichier, O_RDONLY);
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Traiter_request_getslash : Error / error %s on file %s for %s",
                 strerror(errno), nom_fichier, session->util->nom );
       if (fd!=-1) close(fd);
       return(MHD_NO);
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "Http_Traiter_request_getslash : Request for / OK for %s", session->util->nom );

    response = MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
    MHD_add_response_header (response, "Content-Type", "text/html");
    Http_Add_response_header ( response );
    MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return(MHD_YES);
  }
/*--------------------------------------------------------------------------------------------------------*/
