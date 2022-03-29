/******************************************************************************************************************************/
/* Watchdogd/Http/bus_slave.c       Gestion des request BUS depuis le slave                                                   */
/* Projet WatchDog version 4.0       Gestion d'habitat                                                    12.03.2022 09:08:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * bus_slave.c
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
/* Http_Post_to_local_BUS: Envoie un message a l'API local                                                                    */
/* Entrée: la socket, le tag, le message, sa longueur                                                                         */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 JsonNode *Http_Post_to_local_BUS ( struct SUBPROCESS *module, gchar *bus_tag, JsonNode *RootNode )
  { gchar query[256];
    JsonNode *retour = NULL;
    gboolean free_root_node = FALSE;

    if (!module) return(NULL);
    if (!RootNode) RootNode = Json_node_create();

    Json_node_add_string ( RootNode, "thread_tech_id", Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_add_string ( RootNode, "bus_tag", bus_tag );

    g_snprintf( query, sizeof(query), "https://%s:5559/bus", Config.master_hostname );
/********************************************************* Envoi de la requete ************************************************/
    SoupSession *connexion = soup_session_new();
    g_object_set ( G_OBJECT(connexion), "ssl-strict", FALSE, NULL );
    SoupMessage *soup_msg  = soup_message_new ( "POST", query );
    if (!soup_msg)
     { Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: MSG Error Sending to %s", __func__, query );
       goto end;
     }
    Json_node_add_string ( RootNode, "domain_uuid", Json_get_string ( Config.config, "domain_uuid" ) );
    Json_node_add_int ( RootNode, "request_time", time(NULL) );

    gchar *buf = Json_node_to_string ( RootNode );
    Info_new( Config.log, Config.log_bus, LOG_DEBUG,
             "%s: Sending to %s: %s", __func__, query, buf );
    soup_message_set_request ( soup_msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
    /* Async soup_session_queue_message (client->connexion, msg, callback, client);*/
    soup_session_send_message (connexion, soup_msg); /* SYNC */

    gchar *reason_phrase = Http_Msg_reason_phrase(soup_msg);
    gint   status_code   = Http_Msg_status_code(soup_msg);

    Info_new( Config.log, Config.log_bus, LOG_DEBUG, "%s: Status %d, reason %s", __func__, status_code, reason_phrase );
    if (status_code!=200)
     { Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: Error %d for '%s': %s\n", __func__, status_code, query, reason_phrase ); }
    else { retour = Http_Response_Msg_to_Json ( soup_msg ); }
    g_object_unref( soup_msg );
end:
    soup_session_abort ( connexion );
    if (free_root_node) Json_node_unref ( RootNode );
    return(retour);
  }
/******************************************************************************************************************************/
/* Http_Post_to_local_BUS_DI: Envoie le bit DI au master                                                                      */
/* Entrée: la structure SUBPROCESS, le json associé, l'etat attentu                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Post_to_local_BUS_DI ( struct SUBPROCESS *module, JsonNode *di, gboolean etat )
  { if (!module) return;
    gboolean update = FALSE;
    if (!Json_has_member ( di, "etat" )) { Json_node_add_bool ( di, "first_send", TRUE ); update = TRUE; }
    else
     { Json_node_add_bool ( di, "first_send", FALSE );
       gboolean old_etat = Json_get_bool ( di, "etat" );
       if ( old_etat != etat ) update = TRUE;
     }
    if (update)
     { Json_node_add_bool ( di, "etat", etat );
       Http_Post_to_local_BUS ( module, "SET_DI", di );
     }
  }
/******************************************************************************************************************************/
/* Http_Post_to_local_BUS_AI: Envoie le bit AI au master                                                                      */
/* Entrée: la structure SUBPROCESS, le json associé, l'etat attentu                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Post_to_local_BUS_AI ( struct SUBPROCESS *module, JsonNode *ai, gdouble valeur, gboolean in_range )
  { if (!module) return;
    gboolean update = FALSE;
    if (!Json_has_member ( ai, "valeur" )) { Json_node_add_bool ( ai, "first_send", TRUE ); update = TRUE; }
    else
     { Json_node_add_bool ( ai, "first_send", FALSE );
       gdouble  old_valeur   = Json_get_double ( ai, "valeur" );
       gboolean old_in_range = Json_get_bool   ( ai, "in_range" );
       if ( old_valeur != valeur || old_in_range != in_range ) update = TRUE;
     }
    if (update)
     { Json_node_add_double ( ai, "valeur", valeur );
       Json_node_add_bool   ( ai, "in_range", in_range );
       Http_Post_to_local_BUS ( module, "SET_AI", ai );
     }
  }
/******************************************************************************************************************************/
/* Http_Post_to_local_BUS_CDE: Envoie le bit DI CDE au master                                                                 */
/* Entrée: la structure SUBPROCESS, le tech_id, l'acronyme, l'etat attentu                                                    */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Post_to_local_BUS_CDE ( struct SUBPROCESS *module, gchar *tech_id, gchar *acronyme )
  { if (!module) return;
    JsonNode *body = Json_node_create ();
    if(!body) return;
    Json_node_add_string ( body, "tech_id",  tech_id );
    Json_node_add_string ( body, "acronyme", acronyme );
    Http_Post_to_local_BUS ( module, "SET_CDE", body );
    Json_node_unref(body);
  }
/******************************************************************************************************************************/
/* Http_Post_to_local_BUS_WATCHDOG: Envoie le bit WATCHDOG au master selon le status                                          */
/* Entrée: la structure SUBPROCESS, le tech_id, l'acronyme, l'etat attentu                                                    */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Post_to_local_BUS_WATCHDOG ( struct SUBPROCESS *module, gchar *tech_id, gchar *acronyme, gint consigne )
  { if (!module) return;
    JsonNode *body = Json_node_create ();
    if(!body) return;
    Json_node_add_string ( body, "tech_id",  tech_id ); /* target */
    Json_node_add_string ( body, "acronyme", acronyme );
    Json_node_add_int    ( body, "consigne", consigne );
    Http_Post_to_local_BUS ( module, "SET_WATCHDOG", body );
    Json_node_unref(body);
  }
/******************************************************************************************************************************/
/* Http_Traiter_install: Traite l'installation du système                                                                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 static void Http_traiter_bus_post ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data )
  { JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Parsing Request Failed" );
       return;
     }

    if ( !Json_has_member ( request, "bus_tag" ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "bus_tag missing");
       Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: '%s': bus_tag missing", __func__ );
       Json_node_unref(request);
       return;
     }
    gchar *bus_tag = Json_get_string ( request, "bus_tag" );

    if ( !Json_has_member ( request, "thread_tech_id" ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "thread_tech_id missing");
       Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: thread_tech_id missing for bus_tag %s", __func__, bus_tag );
       Json_node_unref(request);
       return;
     }
    gchar *thread_tech_id = Json_get_string ( request, "thread_tech_id" );


    if ( !Json_has_member ( request, "domain_uuid" ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "domain_uuid missing");
       Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: domain_uuid missing for thread_tech_id %s", __func__, thread_tech_id );
       Json_node_unref(request);
       return;
     }
    gchar *domain_uuid    = Json_get_string ( request, "domain_uuid" );

    if ( strcasecmp( domain_uuid, Json_get_string ( Config.config, "domain_uuid" )  ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Wrong Domain UUID.");
       Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: domain_uuid mismatch", __func__ );
       Json_node_unref(request);
       return;
     }

/************************************ Positionne un watchdog ******************************************************************/
    if ( !strcasecmp( bus_tag, "SET_WATCHDOG") )
     { if (! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
              Json_has_member ( request, "consigne" ) ) )
        { Info_new( Config.log, Config.log_bus, LOG_ERR, "%s: SET_WATCHDOG: wrong parameters from '%s'", __func__, thread_tech_id );
          soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
          Json_node_unref(request);
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
          Json_node_unref(request);
          return;
        }

       gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
       gchar *tech_id         = thread_tech_id;
       gchar *acronyme        = thread_acronyme;

       JsonNode *map = g_tree_lookup ( Partage->Maps_from_thread, request );
       if (map && Json_has_member ( map, "tech_id" ) && Json_has_member ( map, "acronyme" ) )
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
          Json_node_unref(request);
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
          Json_node_unref(request);
          return;
        }

       gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
       gchar *tech_id         = thread_tech_id;
       gchar *acronyme        = thread_acronyme;

       JsonNode *map = g_tree_lookup ( Partage->Maps_from_thread, request );
       if (map && Json_has_member ( map, "tech_id" ) && Json_has_member ( map, "acronyme" ) )
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
/************************************ Réaction sur GET_DO *********************************************************************/
    else if ( !strcasecmp( bus_tag, "GET_DO") )
     { JsonNode *Response = Json_node_create();
       JsonArray *output_array = Json_node_add_array ( Response, "douts" );
       GSList *liste = Partage->Dls_data_DO;
       while (liste)
        { struct DLS_DO *dout = liste->data;
          JsonNode *element = Json_node_create();
          Json_node_add_string( element, "tech_id",  dout->tech_id );
          Json_node_add_string( element, "acronyme", dout->acronyme );
          JsonNode *map = g_tree_lookup ( Partage->Maps_to_thread, element );
          if (map && Json_has_member ( map, "thread_tech_id" ) && Json_has_member ( map, "thread_acronyme" ) )
           { gchar *local_thread_tech_id  = Json_get_string ( map, "thread_tech_id" );
             if (!strcasecmp ( local_thread_tech_id, thread_tech_id ) )
              { Json_node_add_string ( element, "thread_tech_id",  thread_tech_id );
                Json_node_add_string ( element, "thread_acronyme", Json_get_string ( map, "thread_acronyme" ) );
                Json_node_add_bool   ( element, "etat", dout->etat );
                Json_array_add_element ( output_array, element );
              }
             else Json_node_unref ( element );
             Json_node_unref ( map );
           } else Json_node_unref ( element );
          liste = g_slist_next ( liste );
        }
       Http_Send_json_response ( msg, Response );
     }
    else
     { soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
       Json_node_unref(request);
       return;
     }
    Json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_traiter_bus: Traite la gestion des bits memoire                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_bus ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  {      if (msg->method == SOUP_METHOD_POST) return (Http_traiter_bus_post(server, msg, path, query, client, user_data));
    /*else if (msg->method == SOUP_METHOD_GET)  return (Http_memory_get (server, msg, path, query, client, user_data));*/
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
