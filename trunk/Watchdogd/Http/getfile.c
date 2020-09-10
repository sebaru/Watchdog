/******************************************************************************************************************************/
/* Watchdogd/Http/getfile.c       Gestion des requests sur des ressources fichiers                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    10.09.2020 08:31:51 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getfile.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;

/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_file ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                          SoupClientContext *client, gpointer user_data )
  { struct stat stat_buf;
    gchar fichier[80];
    gint fd;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    /*struct HTTP_CLIENT_SESSION *session = */Http_print_request ( server, msg, path, client );

    g_snprintf ( fichier, sizeof(fichier), "IHM%s", g_strcanon ( path, "abcdefghijklmnopqrstuvwxyz_", '_' ) );

    if (stat (fichier, &stat_buf)==-1)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Error Stat" );
       return;
     }
    gint taille_fichier = stat_buf.st_size;

    gchar *result = g_try_malloc0 ( taille_fichier );
    if (!result)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
       return;
     }

    fd = open (fichier, O_RDONLY );
    if (fd==-1)
     { g_free(result);
       soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "File Open Error" );
       return;
     }
    read ( fd, &result, taille_fichier );
    close(fd);

/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    if ( !strncasecmp (path, "/js/", strlen("/js/") ) )
     { soup_message_set_response ( msg, "text/javascript; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_fichier ); }
    else if ( !strncasecmp (path, "/svg/", strlen("/svg/") ) )
     { soup_message_set_response ( msg, "image/svg+xml; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_fichier ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
