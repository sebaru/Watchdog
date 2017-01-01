/******************************************************************************************************************************/
/* Watchdogd/Http/getgif.c       Gestion des request getgif pour le thread HTTP de watchdog                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         dim. 05 mai 2013 16:33:43 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Http.h"
/******************************************************************************************************************************/
/* Http_Traiter_request_getgif: Traite une requete sur l'URI getgif                                                           */
/* Entrées: la connexion MHD                                                                                                  */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 gint Http_Traiter_request_getgif ( struct lws *wsi, gchar *remote_name, gchar *remote_ip, gchar *url )
  { gchar nom_fichier[80];
    gint id, mode, retour;

    if ( sscanf ( url, "%d/%d", &id, &mode ) != 2)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Traiter_request_getgif : URL Parsing Error (%s) on file %s" );
       return(1);
     }

    if (mode) { g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%d.gif.%02d", id, mode ); }
         else { g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%d.gif", id ); }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "Http_Traiter_request_getgif : URL Parsing id=%d mode=%d file=%s", id, mode, nom_fichier );

    retour = lws_serve_http_file ( wsi, nom_fichier, "image/gif", NULL, 0);
    if (retour != 0) return(1);                                           /* Si erreur (<0) ou si ok (>0), on ferme la socket */
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
