/******************************************************************************************************************************/
/* Watchdogd/Http/install.c       Gestion des request install pour le thread HTTP de watchdog                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    20.09.2020 00:44:17 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * install.c
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

/******************************************************************************************************************************/
/* Http_Traiter_install: Traite l'installation du système                                                                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_install ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar fichier[80];
    struct stat stat_buf;

    if (msg->method == SOUP_METHOD_GET)
     { SoupMessageHeaders *headers;
       g_object_get ( G_OBJECT(msg), "response_headers", &headers, NULL );

       gchar fichier[128];
       g_snprintf ( fichier, sizeof(fichier), "%s/install.html", WTD_PKGDATADIR );

       if (stat (fichier, &stat_buf)==-1)
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : File '%s' not found", __func__, fichier );
          soup_message_set_status_full ( msg, SOUP_STATUS_NOT_FOUND, "File not found" );
          return;
        }

       gint   taille_result = stat_buf.st_size;
       gchar *result        = g_try_malloc ( taille_result );
       if (!result)
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : File '%s' malloc error", __func__, fichier );
          soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
          return;
        }

       gint fd = open ( fichier, O_RDONLY );
       if (fd==-1)
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : File '%s' open error '%s'", __func__, fichier, strerror(errno) );
          g_free(result);
          soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "File Open Error" );
          return;
        }
       read ( fd, result, taille_result );
       close(fd);

       soup_message_headers_append ( headers, "cache-control", "private, max-age=86400" );
       soup_message_set_response ( msg, "text/html; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_result );
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }

    if (msg->method != SOUP_METHOD_POST)
     { soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
       return;
     }

    g_snprintf ( fichier, sizeof(fichier), "/etc/abls-habitat-agent.conf" );
    if (stat (fichier, &stat_buf)!=-1)                   /* Si pas d'erreur et fichier présent, c'est que c'est deja installé */
     { JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Json_node_add_bool( RootNode, "success", FALSE );
          Json_node_add_string( RootNode, "message", "Agent already installed" );
          Http_Send_json_response ( msg, RootNode );
        }
       else soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
       return;
     }

    if (getuid()!=0)
     { JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Json_node_add_bool( RootNode, "success", FALSE );
          Json_node_add_string( RootNode, "message", "Agent is not Running as ROOT" );
          Http_Send_json_response ( msg, RootNode );
        }
       else soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
       return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request)
     { JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Json_node_add_bool( RootNode, "success", FALSE );
          Json_node_add_string( RootNode, "message", "Parsing Request Failed" );
          Http_Send_json_response ( msg, RootNode );
        }
       else soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
       return;
     }

    if ( ! ( Json_has_member ( request, "domain_uuid" ) && Json_has_member ( request, "domain_secret" )
          && Json_has_member ( request, "api_url" )
           )
       )
     { JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Json_node_add_bool( RootNode, "success", FALSE );
          Json_node_add_string( RootNode, "message", "Mauvais parametres" );
          Http_Send_json_response ( msg, RootNode );
        }
       else soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
       Json_node_unref(request);
       return;
     }
    gchar *domain_uuid   = Json_get_string ( request, "domain_uuid" );
    gchar *domain_secret = Json_get_string ( request, "domain_secret" );
    gchar *api_url       = Json_get_string ( request, "api_url" );
    if ( g_str_has_prefix ( api_url, "https://" ) ) api_url+=8;
    if (strlen(api_url)==0) api_url = "api.abls-habitat.fr";
/******************************************* Création fichier de config *******************************************************/
    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Creating config file '%s'", __func__, fichier );
    JsonNode *RootNode = Json_node_create ();
    if (RootNode)
     { Json_node_add_string( RootNode, "domain_uuid", domain_uuid );
       Json_node_add_string( RootNode, "domain_secret", domain_secret );
       Json_node_add_string( RootNode, "api_url", api_url );
       Json_node_add_string( RootNode, "product", "agent" );
       Json_node_add_string( RootNode, "vendor", "abls-habitat.fr" );
       time_t t = time(NULL);
       struct tm *temps = localtime( &t );
       if (temps)
        { gchar date[64];
          strftime( date, sizeof(date), "%F %T", temps );
          Json_node_add_string( RootNode, "install_time", date );
        }
       Json_write_to_file ( "/etc/abls-habitat-agent.conf", RootNode );
       Json_node_unref(RootNode);
     }
    else { Info_new( Config.log, TRUE, LOG_ERR, "%s: Writing config failed: Memory Error.", __func__ ); }

    Json_node_unref(request);

    RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
    Json_node_add_string ( RootNode, "status", "installed" );
    Http_Send_json_response ( msg, RootNode );
    Partage->com_msrv.Thread_run = FALSE;                                                    /* On reboot toute la baraque !! */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
