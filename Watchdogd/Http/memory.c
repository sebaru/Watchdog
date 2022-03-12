/******************************************************************************************************************************/
/* Watchdogd/Http/memory.c       Gestion des request memory pour le thread HTTP de watchdog                                   */
/* Projet WatchDog version 4.0       Gestion d'habitat                                                    12.03.2022 09:08:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * memory.c
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

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Http_Traiter_install: Traite l'installation du système                                                                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 static void Http_memory_post ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Parsing Request Failed" );
       return;
     }

    if ( ! ( Json_has_member ( request, "bus_tag" ) && Json_has_member ( request, "domain_uuid" ) &&
             Json_has_member ( request, "thread_tech_id" )
           )
       )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *domain_uuid    = Json_get_string ( request, "domain_uuid" );
    if ( strcasecmp( domain_uuid, Json_get_string ( Config.config, "domain_uuid" )  ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Wrong Domain UUID. Dropping.");
       json_node_unref(request);
       return;
     }

    gchar *thread_tech_id = Json_get_string ( request, "thread_tech_id" );
    gchar *bus_tag = Json_get_string ( request, "bus_tag" );
    if ( !strcasecmp( bus_tag, "SET_WATCHDOG") )
     { if (! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
              Json_has_member ( request, "consigne" ) ) )
        { Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: SET_WATCHDOG: wrong parameters from '%s'", __func__, thread_tech_id );
          soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
          json_node_unref(request);
          return;
        }

       Info_new( Config.log, Config.log_bus, LOG_INFO,
                 "%s: SET_WATCHDOG from '%s': '%s:%s'=%d", __func__, thread_tech_id,
                 Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ),
                 Json_get_int ( request, "consigne" ) );
       Dls_data_set_WATCHDOG ( NULL, Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ), NULL,
                               Json_get_int ( request, "consigne" ) );
     }
/************************************ Positionne une valeur d'une Entrée Analogique *******************************************/
    else if ( !strcasecmp( bus_tag, "SET_AI") )
     { if (! (Json_has_member ( request, "thread_acronyme" ) &&
              Json_has_member ( request, "valeur" ) && Json_has_member ( request, "in_range" ) &&
              Json_has_member ( request, "libelle" ) && Json_has_member ( request, "unite" )
             )
          )
        { Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: SET_AI: wrong parameters from '%s'", __func__, thread_tech_id );
          soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
          json_node_unref(request);
          return;
        }

       gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
       gchar *tech_id         = thread_tech_id;
       gchar *acronyme        = thread_acronyme;

       JsonNode *map = g_tree_lookup ( Partage->Maps_from_thread, request );
       if (map)
        { tech_id  = Json_get_string ( map, "tech_id" );
          acronyme = Json_get_string ( map, "acronyme" );
        }
       Info_new( Config.log, Config.log_bus, LOG_INFO,
                 "%s: SET_AI from '%s': '%s:%s'/'%s:%s'=%f %s (range=%d)", __func__,
                 thread_tech_id, thread_tech_id, thread_acronyme, tech_id, acronyme,
                 Json_get_double ( request, "valeur" ), Json_get_string ( request, "unite" ), Json_get_bool ( request, "in_range" ) );
       struct DLS_AI *ai = NULL;
       Dls_data_set_AI ( tech_id, acronyme, (gpointer)&ai,
                         Json_get_double ( request, "valeur" ), Json_get_bool ( request, "in_range" ) );
       if (Json_get_bool ( request, "first_send" ) == TRUE )
        { g_snprintf ( ai->libelle, sizeof(ai->libelle), "%s", Json_get_string ( request, "libelle" ) );
          g_snprintf ( ai->unite,   sizeof(ai->unite),   "%s", Json_get_string ( request, "unite" ) );
          ai->archivage = Json_get_int ( request, "archivage" );
          gchar *libelle = Normaliser_chaine ( ai->libelle );
          SQL_Write_new ( "INSERT INTO mappings SET classe='AI', "
                          "thread_tech_id = '%s', thread_acronyme = '%s', tech_id = '%s', acronyme = '%s', libelle='%s' "
                          "ON DUPLICATE KEY UPDATE classe=VALUE(classe), libelle=VALUE(libelle) ",
                          thread_tech_id, thread_acronyme, tech_id, acronyme, libelle );
          g_free(libelle);
        }
     }
/************************************ Réaction sur SET_CDE ********************************************************************/
    else if ( !strcasecmp( bus_tag, "SET_CDE") )
     { if (! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) ) )
        { Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: SET_CDE: wrong parameters from '%s'", __func__, thread_tech_id );
          soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
          json_node_unref(request);
          return;
        }
       Info_new( Config.log, Config.log_bus, LOG_INFO,
                 "%s: SET_CDE from '%s': '%s:%s'=1", __func__, thread_tech_id,
                 Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ) );
       Envoyer_commande_dls_data ( Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ) );
     }
/************************************ Réaction sur SET_DI *********************************************************************/
    else if ( !strcasecmp( bus_tag, "SET_DI") )
     { if (! (Json_has_member ( request, "thread_acronyme" ) &&
              Json_has_member ( request, "etat" )&& Json_has_member ( request, "libelle" )
             )
          )
        { Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: SET_DI: wrong parameters from '%s'", __func__, thread_tech_id );
          soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
          json_node_unref(request);
          return;
        }

       gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
       gchar *tech_id         = thread_tech_id;
       gchar *acronyme        = thread_acronyme;

       JsonNode *map = g_tree_lookup ( Partage->Maps_from_thread, request );
       if (map)
        { tech_id  = Json_get_string ( map, "tech_id" );
          acronyme = Json_get_string ( map, "acronyme" );
        }
       Info_new( Config.log, Config.log_bus, LOG_INFO,
                 "%s: SET_DI from '%s': '%s:%s/'%s:%s'=%d", __func__,
                 thread_tech_id, thread_tech_id, thread_acronyme, tech_id, acronyme,
                 Json_get_bool ( request, "etat" ) );
       struct DLS_DI *di = NULL;
       Dls_data_set_DI ( NULL, tech_id, acronyme, (gpointer)&di, Json_get_bool ( request, "etat" ) );
       if (Json_get_bool ( request, "first_send" ) == TRUE )
        { g_snprintf ( di->libelle, sizeof(di->libelle), "%s", Json_get_string ( request, "libelle" ) );
          gchar *libelle = Normaliser_chaine ( di->libelle );
          SQL_Write_new ( "INSERT INTO mappings SET classe='DI', "
                          "thread_tech_id = '%s', thread_acronyme = '%s', tech_id = '%s', acronyme = '%s', libelle='%s' "
                          "ON DUPLICATE KEY UPDATE classe=VALUE(classe), libelle=VALUE(libelle) ",
                          thread_tech_id, thread_acronyme, tech_id, acronyme, libelle );
          g_free(libelle);
        }
     }

    json_node_unref(request);
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_traiter_memory: Traite la gestion des bits memoire                                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_memory ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  {      if (msg->method == SOUP_METHOD_POST) return (Http_memory_post(server, msg, path, query, client, user_data));
    /*else if (msg->method == SOUP_METHOD_GET)  return (Http_memory_get (server, msg, path, query, client, user_data));*/
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
