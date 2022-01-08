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
 extern struct HTTP_CONFIG Cfg_http;

/******************************************************************************************************************************/
/* Admin_json_map_list: Recupère la liste des mapping d'une certaine classe pour un thread donné                              */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_map_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                              SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gchar *thread = g_hash_table_lookup ( query, "thread" );
    if (!thread)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres thread");
       return;
     }
    Normaliser_as_ascii ( thread );

    gpointer classe = g_hash_table_lookup ( query, "classe" );
    if (!classe)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres classe");
       return;
     }
    Normaliser_as_ascii ( classe );

/************************************************ Préparation du buffer JSON **************************************************/
    GSList *liste = Partage->com_msrv.Librairies;                                        /* Parcours de toutes les librairies */
    while(liste)
     { struct PROCESS *lib = liste->data;
       if ( ! strcasecmp( thread, lib->name ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste)
     { soup_message_set_status_full (msg, SOUP_STATUS_NOT_FOUND, "Not a thread" );
       return;
     }

    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Select_to_json_node ( RootNode, "mappings", "SELECT * FROM %s_%s ", thread, classe );  /* Contenu de la table details */

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_traiter_map_set: ajoute un mapping dans la base de données                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_map_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

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

    gchar old_thread_tech_id[32];
    bzero ( old_thread_tech_id, sizeof(old_thread_tech_id) );

    JsonNode *OldNode = Json_node_create ();
    if (OldNode)
     { SQL_Select_to_json_node ( OldNode, NULL,
                                 "SELECT thread_tech_id FROM mappings "
                                 "WHERE mappings.tech_id = '%s' AND mappings.acronyme = '%s'", tech_id, acronyme );
       if (Json_has_member ( OldNode, "thread_tech_id" ))
        { g_snprintf ( old_thread_tech_id, sizeof(old_thread_tech_id), "%s", Json_get_string ( OldNode, "thread_tech_id" ) ); }
       json_node_unref ( OldNode );
     }

    SQL_Write_new ( "UPDATE mappings SET tech_id = NULL, acronyme = NULL "
                    "WHERE tech_id = '%s' AND acronyme = '%s'", tech_id, acronyme );

    SQL_Write_new ( "UPDATE mappings SET tech_id = '%s', acronyme = '%s' "
                    "WHERE thread_tech_id = '%s' AND thread_acronyme = '%s'", tech_id, acronyme, thread_tech_id, thread_acronyme );

    if ( strcasecmp ( old_thread_tech_id, thread_tech_id ) )
     { JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Json_node_add_string ( RootNode, "thread_tech_id", old_thread_tech_id );
          Json_node_add_string ( RootNode, "zmq_tag", "SUBPROCESS_REMAP" );
          Zmq_Send_json_node( Cfg_http.lib->zmq_to_master, "HTTP", "*", RootNode );
        }
       json_node_unref ( RootNode );
     }

    JsonNode *RootNode = Json_node_create ();
    if (RootNode)
     { Json_node_add_string ( RootNode, "thread_tech_id", thread_tech_id );
       Json_node_add_string ( RootNode, "zmq_tag", "SUBPROCESS_REMAP" );
       Zmq_Send_json_node( Cfg_http.lib->zmq_to_master, "HTTP", "*", RootNode );
     }
    json_node_unref ( RootNode );

    Dls_recalculer_arbre_comm();/* Calcul de l'arbre de communication car il peut y avoir de nouvelles dependances sur les plugins */

    g_free(tech_id);
    g_free(acronyme);
    g_free(thread_tech_id);
    g_free(thread_acronyme);
    json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
