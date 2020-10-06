/******************************************************************************************************************************/
/* Watchdogd/Http/getconfig.c       Gestion des request getconfig pour le thread HTTP de watchdog                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    26.07.2020 21:23:28 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getconfig.c
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
/* Http_Traiter_config_list: Fourni une list JSON des configs Watchdog dans le domaine                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_config_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gchar requete[256];
    gsize taille;
    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No Request");
       return;
     }

    if ( ! (Json_has_member ( request, "id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    g_snprintf( requete, sizeof(requete), "DELETE FROM config WHERE id=%d", Json_get_int ( request, "id" ) );
    Audit_log ( session, "Config id %d deleted", Json_get_int ( request, "id" ) );
    json_node_unref(request);

    if (SQL_Write ( requete ))
     { soup_message_set_status (msg, SOUP_STATUS_OK);
     }
    else
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error"); }
  }
/******************************************************************************************************************************/
/* Http_Modifier_Config: Modifie une entrée de la table de configuration                                                      */
/* Entrées: l'element Json a positionner                                                                                      */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Modifier_config ( JsonObject *object, const gchar *member_name, JsonNode *member_node, gpointer user_data )
  { gchar *conditions = user_data;
    gchar requete[256];
    gchar *valeur = Normaliser_chaine ( json_object_get_string_member ( object, member_name ) );
    gchar *param  = Normaliser_chaine ( member_name );
    g_snprintf( requete, sizeof(requete), "UPDATE config set valeur='%s' WHERE %s AND nom='%s'", valeur, conditions, param );
    SQL_Write ( requete );
    g_free(valeur);
    g_free(param);
  }
/******************************************************************************************************************************/
/* Http_traiter_config_set: Modifie une liste d'entrée dans la table config                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_config_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gchar critere[256];
    gsize taille;
    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No Request");
       return;
     }

    if ( ! (Json_has_member ( request, "instance" ) && Json_has_member ( request, "thread" ) && Json_has_member ( request, "parametres" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *instance;
    if (!strcasecmp(Json_get_string( request, "instance" ), "MASTER")) { instance = g_strdup(g_get_host_name()); }
    else instance = Normaliser_chaine ( Json_get_string( request, "instance" ) );

    gchar *thread   = Normaliser_chaine ( Json_get_string( request, "thread" ) );
    g_snprintf( critere, sizeof(critere), " instance_id='%s' AND nom_thread='%s' ",instance, thread );
    g_free(instance);
    g_free(thread);
    json_object_foreach_member ( Json_get_object(request, "parametres"), Http_Modifier_config, critere );

    Audit_log ( session, "Config for '%s':'%s' changed", Json_get_string( request, "instance" ), Json_get_string( request, "thread" ) );
    json_node_unref(request);
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_Traiter_config_list: Fourni une list JSON des configs Watchdog dans le domaine                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_config_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { gchar *buf, requete[256], critere[128];
    gsize taille_buf;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;


    gchar *instance = Normaliser_as_ascii ( g_hash_table_lookup ( query, "instance" ) );
    gchar *thread   = Normaliser_as_ascii ( g_hash_table_lookup ( query, "thread" ) );
    gchar *param    = Normaliser_as_ascii ( g_hash_table_lookup ( query, "param" ) );
    if (!instance && !thread && !param)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    if (!instance || !strcasecmp(instance, "MASTER")) { instance = g_get_host_name(); }

    g_snprintf( requete, sizeof(requete), "SELECT * FROM config WHERE 1=1 ");
    if (instance)
     { g_snprintf( critere, sizeof(critere), " AND instance_id LIKE '%s'", instance );
       g_strlcat ( requete, critere, sizeof(requete) );
     }

    if (thread)
     { g_snprintf( critere, sizeof(critere), " AND nom_thread LIKE '%s'", thread );
       g_strlcat ( requete, critere, sizeof(requete) );
     }

    if (param)
     { g_snprintf( critere, sizeof(critere), " AND nom LIKE '%s'", param );
       g_strlcat ( requete, critere, sizeof(requete) );
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Select_to_JSON ( builder, "configs", requete );

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
