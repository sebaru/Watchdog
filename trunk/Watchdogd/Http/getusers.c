/******************************************************************************************************************************/
/* Watchdogd/Http/getusers.c       Gestion des request getusers pour le thread HTTP de watchdog                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    11.07.2020 15:24:31 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getusers.c
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

 #include <string.h>
 #include <unistd.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;
/******************************************************************************************************************************/
/* Http_Traiter_request_getusers_list: Traite une requete sur l'URI users/list                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_users_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf, chaine[256];

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );

    if ( ! (session && session->access_level >= 1) )
     { soup_message_set_status (msg, SOUP_STATUS_FORBIDDEN);
       return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    g_snprintf( chaine, sizeof(chaine), "SELECT * FROM users WHERE access_level<='%d'", session->access_level );
    SQL_Select_to_JSON ( builder, "Users", chaine );
    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getusers_list: Traite une requete sur l'URI users/list                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_users_sessions ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );

    if ( ! (session && session->access_level >= 6) )
     { soup_message_set_status (msg, SOUP_STATUS_FORBIDDEN);
       return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_array ( builder, "Sessions" );
    GSList *liste = Cfg_http.liste_http_clients;
    while(liste)
     { struct HTTP_CLIENT_SESSION *sess = liste->data;
       Json_add_string ( builder, "username", sess->username );
       Json_add_string ( builder, "wtd_session", sess->wtd_session );
       Json_add_int    ( builder, "access_level", sess->access_level );
       Json_add_int    ( builder, "last_request", sess->last_request );
       liste = g_slist_next ( liste );
     }
    Json_end_array ( builder );

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
