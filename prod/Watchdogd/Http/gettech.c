/******************************************************************************************************************************/
/* Watchdogd/Http/gettech.c       Gestion des requests sur l'URI /tech du webservice                                          */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    12.08.2020 12:11:19 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * gettech.c
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
 void Http_traiter_tech ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                          SoupClientContext *client, gpointer user_data )
  { struct stat stat_buf;
    gint fd;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!session)
     { soup_message_set_redirect ( msg, 301, "/auth/login" );
       return;
     }

    gchar *prefix = "/tech/";
    if ( (!strcasecmp ( path, "/tech" )) || (!strcasecmp( path, prefix ) ) )
     { soup_message_set_redirect ( msg, 301, "/tech/dashboard" );
       return;
     }

    gchar fichier[80];
    g_snprintf ( fichier, sizeof(fichier), "IHM/tech/%s", g_strcanon ( path + strlen(prefix), "abcdefghijklmnopqrstuvwxyz", '_' ) );

    if (stat ("IHM/tech/header.php", &stat_buf)==-1)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Error Stat" );
       return;
     }
    gint taille_header = stat_buf.st_size;

    if (stat (fichier, &stat_buf)==-1)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Error Stat" );
       return;
     }
    gint taille_fichier = stat_buf.st_size;

    if (stat ("IHM/tech/footer.php", &stat_buf)==-1)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Error Stat" );
       return;
     }
    gint taille_footer = stat_buf.st_size;
    gint taille_result = taille_header + taille_fichier + taille_footer;
    gchar *result = g_try_malloc0 ( taille_result );
    if (!result)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
       return;
     }

    fd = open ("IHM/tech/header.php", O_RDONLY );
    if (fd==-1)
     { g_free(result);
       soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Header Open Error" );
       return;
     }
    read ( fd, &result,taille_header );
    close(fd);
     
    fd = open (fichier, O_RDONLY );
    if (fd==-1)
     { g_free(result);
       soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "File Open Error" );
       return;
     }
    read ( fd, &result + taille_header ,taille_fichier );
    close(fd);

    fd = open (fichier, O_RDONLY );
    if (fd==-1)
     { g_free(result);
       soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Footer Open Error" );
       return;
     }
    read ( fd, &result + taille_header + taille_footer ,taille_footer );
    close(fd);

/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "text/html; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_result );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
