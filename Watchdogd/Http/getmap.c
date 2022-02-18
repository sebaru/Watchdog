/******************************************************************************************************************************/
/* Watchdogd/Http/getmap.c.c        Gestion des Mapping Watchdog                                                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    04.10.2020 20:48:08 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getmap.c.c
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

/******************************************************************************************************************************/
/* Http_traiter_map_get: récupère une liste de mapping (en dehors du mapping de thread)                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_traiter_map_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gchar *thread_tech_id_src = g_hash_table_lookup ( query, "thread_tech_id" );
    if (!thread_tech_id_src)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres classe");
       return;
     }

    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    gchar *thread_tech_id  = Normaliser_chaine ( thread_tech_id_src );
    SQL_Select_to_json_node ( RootNode, "mappings",
                              "SELECT * FROM mappings AS map "
                              "WHERE thread_tech_id='%s' ", thread_tech_id
                            );
    g_free(thread_tech_id);

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_traiter_map_set: ajoute un mapping dans la base de données                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_traiter_map_post ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data )
  { struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;
                                                            /* Création d'une entrée, notamment pour le mapping _COMMAND_TEXT */
    if ( Json_has_member ( request, "thread_tech_id" ) && Json_has_member ( request, "thread_acronyme" ) &&
      ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ))
       )
     { gchar *thread_tech_id  = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
       gchar *thread_acronyme = Normaliser_chaine ( Json_get_string( request, "thread_acronyme" ) );
       SQL_Write_new ( "INSERT INTO mappings SET thread_tech_id = UPPER('%s'), thread_acronyme=UPPER('%s')",
                       thread_tech_id, thread_acronyme );
       g_free(thread_tech_id);
       g_free(thread_acronyme);
       json_node_unref(request);
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
            Json_has_member ( request, "thread_tech_id" ) && Json_has_member ( request, "thread_acronyme" )
           )
       )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *thread_tech_id  = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *thread_acronyme = Normaliser_chaine ( Json_get_string( request, "thread_acronyme" ) );
    gchar *tech_id         = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *acronyme        = Normaliser_chaine ( Json_get_string( request, "acronyme" ) );

    SQL_Write_new ( "UPDATE mappings SET tech_id = NULL, acronyme = NULL "
                    "WHERE tech_id = '%s' AND acronyme = '%s'", tech_id, acronyme );

    SQL_Write_new ( "INSERT INTO mappings SET "
                    "thread_tech_id = UPPER('%s'), thread_acronyme = UPPER('%s'), "
                    "tech_id = UPPER('%s'), acronyme = '%s' "
                    "ON DUPLICATE KEY UPDATE tech_id = '%s', acronyme = '%s'",
                    thread_tech_id, thread_acronyme, tech_id, acronyme, tech_id, acronyme );

    g_free(tech_id);
    g_free(acronyme);
    g_free(thread_tech_id);
    g_free(thread_acronyme);

    MSRV_Remap();
    Dls_recalculer_arbre_comm();/* Calcul de l'arbre de communication car il peut y avoir de nouvelles dependances sur les plugins */

    json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_traiter_map: Gère la mapping entre les thread et les bits DLS                                                         */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_map ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                         SoupClientContext *client, gpointer user_data )
  { if (msg->method == SOUP_METHOD_POST) return ( Http_traiter_map_post ( server, msg, path, query, client, user_data ) );
    if (msg->method == SOUP_METHOD_GET ) return ( Http_traiter_map_get  ( server, msg, path, query, client, user_data ) );
    else soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
