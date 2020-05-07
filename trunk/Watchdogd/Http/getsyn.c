/******************************************************************************************************************************/
/* Watchdogd/Http/getsyn.c       Gestion des requests sur l'URI /syn du webservice                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                 06.05.2020 10:57:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getsyn.c
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
 #include <libxml/xmlwriter.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar *buf, chaine[256];
    gsize taille_buf;
    gint syn_id;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    Http_print_request ( server, msg, path, client );

    if ( ! g_str_has_prefix ( path, "/syn/get/" ) )
     { soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }

    if (!strlen (path+9))
     { soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }
    syn_id = atoi(path+9);

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns WHERE id=%d", syn_id );
    if (Select_SQL_to_JSON ( builder, NULL, chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_motifs WHERE syn_id=%d", syn_id );
    if (Select_SQL_to_JSON ( builder, "motifs", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_pass WHERE syn_id=%d", syn_id );
    if (Select_SQL_to_JSON ( builder, "passerelles", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_liens WHERE syn_id=%d", syn_id );
    if (Select_SQL_to_JSON ( builder, "liens", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_rectangles WHERE syn_id=%d", syn_id );
    if (Select_SQL_to_JSON ( builder, "rectangles", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
