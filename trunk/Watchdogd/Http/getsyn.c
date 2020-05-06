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
  { gsize taille_buf;
    struct DB *db;
    gint syn_id;
    gchar *buf;
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

    if ( Recuperer_motifDB_new ( &db, syn_id ) == FALSE )
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }

    Json_add_array ( builder, "motifs" );
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    while ( db->row )
     { Json_add_object ( builder, NULL );
       Json_add_string ( builder, "tech_id", db->row[0] );
       Json_add_string ( builder, "acronyme", db->row[1] );
       Json_add_string ( builder, "forme", db->row[2] );
       Json_add_string ( builder, "libelle", db->row[3] );
       Json_add_int    ( builder, "access_level", atoi(db->row[4]) );
       Json_add_int    ( builder, "posx", atoi(db->row[5]) );
       Json_add_int    ( builder, "posy", atoi(db->row[6]) );
       Json_add_int    ( builder, "angle", atoi(db->row[7]) );
       Json_add_string ( builder, "def_color", db->row[8] );
       Json_add_string ( builder, "clic_tech_id", db->row[9] );
       Json_add_string ( builder, "clic_acronyme", db->row[10] );
       Json_end_object ( builder );
       Recuperer_ligne_SQL(db);                                                            /* Chargement d'une ligne resultat */
     }
    Liberer_resultat_SQL (db);
    Libere_DB_SQL( &db );
    Json_end_array ( builder );

    Json_add_array ( builder, "passerelles" );
    Json_end_array ( builder );

    Json_add_array ( builder, "liens" );
    Json_end_array ( builder );

    Json_add_array ( builder, "rectangles" );
    Json_end_array ( builder );

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
