/******************************************************************************************************************************/
/* Watchdogd/Http/getui.c       Gestion des request User Interface pour le thread HTTP de watchdog                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         dim. 05 mai 2013 16:33:43 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getui.c
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

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Http.h"
/******************************************************************************************************************************/
/* Http_Traiter_request_getui: Traite une requete sur le fichier URI                                                          */
/* Entrées: la connexion WebSocket, les remote name/ip et l'URL                                                               */
/* Sortie : code de retour pour libwebsocket                                                                                  */
/******************************************************************************************************************************/
 gint Http_Traiter_request_getui ( struct lws *wsi, gchar *remote_name, gchar *remote_ip, gchar *url )
  { gchar *mime_type, fichier[128];
    gint taille, retour;

    if ( strstr( url, ".." ) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Traiter_request_getui : Wrong URL containing '..' (%s)", url );
       return(1);
     }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "Http_Traiter_request_getui : URL Parsing filename=%s", url );

    taille = strlen(url);
	   if (taille < 5)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Traiter_request_getui : Wrong URL without suffix (%s)", url );
       return(1);
     }
    mime_type = "text/plain";                                                                                   /* Par défaut */
   	if (!strcmp(&url[taille - 4], ".ico")) mime_type = "image/x-icon";
   	if (!strcmp(&url[taille - 4], ".png")) mime_type = "image/png";
   	if (!strcmp(&url[taille - 5], ".html")) mime_type = "text/html";
   	if (!strcmp(&url[taille - 4], ".css")) mime_type = "text/css";

    g_snprintf( fichier, sizeof(fichier), "WEB/%s", url );

    retour = lws_serve_http_file ( wsi, fichier, mime_type, NULL, 0);
    if (retour != 0) return(1);                                           /* Si erreur (<0) ou si ok (>0), on ferme la socket */
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
