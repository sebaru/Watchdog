/******************************************************************************************************************************/
/* Watchdogd/Http/getaudio.c       Gestion des request Audio (MP3) pour le thread HTTP de watchdog                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    07.08.2016 04:43:06 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getaudio.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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
/* Http_Traiter_request_getaudio: Traite une requete sur le fichier audio URI                                                 */
/* Entrées: la connexion WebSocket, les remote name/ip et l'URL                                                               */
/* Sortie : code de retour pour libwebsocket                                                                                  */
/******************************************************************************************************************************/
 gint Http_Traiter_request_getaudio ( struct lws *wsi, gchar *remote_name, gchar *remote_ip, gchar *url )
  { gchar fichier[128];
    gint retour;

    if ( strstr( url, ".." ) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : Wrong URL containing '..' (%s)", __func__, url );
       return(1);
     }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s : URL Parsing filename=%s", __func__, url );

    g_snprintf( fichier, sizeof(fichier), "Son/%s.wav", url );

    retour = lws_serve_http_file ( wsi, fichier, "audio/x-wav", NULL, 0);
    if (retour < 0) return(1);                                                          /* Si erreur (<0), on ferme la socket */
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
