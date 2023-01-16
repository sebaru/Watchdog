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
/* Http_Get_from_local_BUS: Envoie un message GET vers le master                                                              */
/* Entrée: le module source, l'URI cible                                                                                      */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 JsonNode *Http_Get_from_local_BUS ( struct THREAD *module, gchar *uri )
  { if (!module) return(NULL);

    gchar query[256];
    g_snprintf( query, sizeof(query), "https://%s:5559/%s", Config.master_hostname, uri );
/********************************************************* Envoi de la requete ************************************************/
    SoupMessage *soup_msg  = soup_message_new ( "GET", query );
    if (!soup_msg)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "MSG Error Sending to %s", query );
       goto end;
     }
    g_object_set ( soup_msg, "http-version", SOUP_HTTP_1_0, NULL );
    JsonNode *response = Http_Send_json_request_from_thread ( module, soup_msg, NULL); /* SYNC */

    gchar *reason_phrase = soup_message_get_reason_phrase(soup_msg);
    gint   status_code   = soup_message_get_status(soup_msg);

    Info_new( __func__, Config.log_bus, LOG_DEBUG, "Status %d, reason %s", status_code, reason_phrase );
    if (status_code!=200)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "Error %d for '%s': %s\n", status_code, query, reason_phrase ); }
    g_object_unref( soup_msg );
end:
    return(response);
  }
/******************************************************************************************************************************/
/* Http_Post_to_local_BUS: Envoie un message a l'API local                                                                    */
/* Entrée: la socket, le tag, le message, sa longueur                                                                         */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Http_Post_to_local_BUS ( struct THREAD *module, gchar *uri, JsonNode *RootNode )
  { gchar query[256];
    gboolean retour = FALSE;

    if (!module) return(FALSE);
    if (!RootNode) return(FALSE);

    Json_node_add_string ( RootNode, "thread_tech_id", Json_get_string ( module->config, "thread_tech_id" ) );

    g_snprintf( query, sizeof(query), "https://%s:5559/%s", Config.master_hostname, uri );
/********************************************************* Envoi de la requete ************************************************/
    SoupMessage *soup_msg  = soup_message_new ( "POST", query );
    if (!soup_msg)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "MSG Error Sending to %s", query );
       return(FALSE);
     }
    g_object_set ( soup_msg, "http-version", SOUP_HTTP_1_0, NULL );
    g_signal_connect ( G_OBJECT(soup_msg), "accept-certificate", G_CALLBACK(Http_Accept_certificate), module );

    JsonNode *response = Http_Send_json_request_from_thread ( module, soup_msg, RootNode ); /* SYNC */
    Json_node_unref( response );

    gchar *reason_phrase = soup_message_get_reason_phrase(soup_msg);
    gint   status_code   = soup_message_get_status(soup_msg);

    Info_new( __func__, Config.log_bus, LOG_DEBUG, "Status %d, reason %s", status_code, reason_phrase );
    if (status_code!=200)
         { Info_new( __func__, Config.log_bus, LOG_ERR, "Error %d for '%s': %s\n", status_code, query, reason_phrase ); }
    else { retour = TRUE; }
    g_object_unref( soup_msg );
    return(retour);
  }
/******************************************************************************************************************************/
/* Http_Post_thread_DI_to_local_BUS: Envoie le bit DI au master                                                               */
/* Entrée: la structure THREAD, le json associé, l'etat attentu                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Post_thread_DI_to_local_BUS ( struct THREAD *module, JsonNode *thread_di, gboolean etat )
  { if (!module) return;
    gboolean update = FALSE;
    if (!Json_has_member ( thread_di, "etat" )) { update = TRUE; }
    else
     { gboolean old_etat = Json_get_bool ( thread_di, "etat" );
       if ( old_etat != etat ) update = TRUE;
     }
    if (update)
     { Json_node_add_bool ( thread_di, "etat", etat );
       if (Config.instance_is_master == TRUE) Dls_data_set_DI_from_thread_di ( thread_di );
       else Http_Post_to_local_BUS ( module, "SET_DI", thread_di );
     }
  }
/******************************************************************************************************************************/
/* Http_Post_thread_AI_to_local_BUS: Envoie le bit AI au master                                                               */
/* Entrée: la structure THREAD, le json associé, l'etat attentu                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Post_thread_AI_to_local_BUS ( struct THREAD *module, JsonNode *thread_ai, gdouble valeur, gboolean in_range )
  { if (!module) return;
    gboolean update = FALSE;
    if (!Json_has_member ( thread_ai, "valeur" )) { update = TRUE; }
    else
     { gdouble  old_valeur   = Json_get_double ( thread_ai, "valeur" );
       gboolean old_in_range = Json_get_bool   ( thread_ai, "in_range" );
       if ( old_valeur != valeur || old_in_range != in_range ) update = TRUE;
     }
    if (update)
     { Json_node_add_double ( thread_ai, "valeur", valeur );
       Json_node_add_bool   ( thread_ai, "in_range", in_range );
       if (Config.instance_is_master == TRUE) Dls_data_set_AI_from_thread_ai ( thread_ai );
       else Http_Post_to_local_BUS ( module, "SET_AI", thread_ai );
     }
  }
/******************************************************************************************************************************/
/* Http_Post_to_local_BUS_CDE: Envoie le bit DI CDE au master                                                                 */
/* Entrée: la structure THREAD, le tech_id, l'acronyme, l'etat attentu                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Post_to_local_BUS_CDE ( struct THREAD *module, gchar *tech_id, gchar *acronyme )
  { if (!module) return;
    JsonNode *body = Json_node_create ();
    if(!body) return;
    Json_node_add_string ( body, "tech_id",  tech_id );
    Json_node_add_string ( body, "acronyme", acronyme );
    Http_Post_to_local_BUS ( module, "SET_CDE", body );
    Json_node_unref(body);
  }
/******************************************************************************************************************************/
/* Http_Post_WATCHDOG_to_local_BUS: Envoie le bit WATCHDOG au master selon le status                                          */
/* Entrée: la structure THREAD, le tech_id, l'acronyme, la consigne                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Post_thread_WATCHDOG_to_local_BUS ( struct THREAD *module, gchar *thread_acronyme, gint consigne )
  { if (!module) return;
    JsonNode *thread_watchdog = Json_node_create ();
    if(!thread_watchdog) return;
    Json_node_add_string ( thread_watchdog, "thread_acronyme", thread_acronyme );
    Json_node_add_int    ( thread_watchdog, "consigne", consigne );
    if (Config.instance_is_master == TRUE) Dls_data_set_WATCHDOG_from_thread_watchdog ( thread_watchdog );
    else Http_Post_to_local_BUS ( module, "SET_WATCHDOG", thread_watchdog );
    Json_node_unref(thread_watchdog);
  }
/******************************************************************************************************************************/
/* Http_traiter_get_do: Donne les DO au thread appelant                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_get_do ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query )
  { gchar *thread_tech_id;
    if (!Http_Check_Thread_signature ( path, msg, &thread_tech_id )) return;
    if (!thread_tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "thread_tech_id missing", NULL ); return; }

    JsonNode *Response = Json_node_create();
    JsonArray *output_array = Json_node_add_array ( Response, "douts" );
    struct DLS_PLUGIN *plugin = Dls_get_plugin_by_tech_id ( thread_tech_id );
    if (plugin)
     { GSList *liste = plugin->Dls_data_DO;
       while (liste)
        { struct DLS_DO *dout = liste->data;
          JsonNode *element = Json_node_create();
          Dls_DO_to_json ( element, dout );
          if (MSRV_Map_to_thread ( element ) && Json_has_member ( element, "thread_tech_id" ) && Json_has_member ( element, "thread_acronyme" ) )
           { Json_array_add_element ( output_array, element );
           } else Json_node_unref ( element );
          liste = g_slist_next ( liste );
        }
       Info_new( __func__, Config.log_bus, LOG_INFO,  "GET_DO done for '%s'", thread_tech_id );
     }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "There are DO", Response );
  }
/******************************************************************************************************************************/
/* Http_traiter_set_watchdog_post: Positionne un Watchdog dans DLS                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_set_watchdog_post ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request )
  { gchar *thread_tech_id;
    if (!Http_Check_Thread_signature ( path, msg, &thread_tech_id )) return;
    if (!thread_tech_id)
     { Http_Send_json_response (msg, SOUP_STATUS_BAD_REQUEST, "thread_tech_id missing", NULL);
       Info_new( __func__, Config.log_bus, LOG_ERR, "thread_tech_id missing for path %s", path );
       return;
     }

    if ( Dls_data_set_WATCHDOG_from_thread_watchdog ( request ) == FALSE )
     { Info_new( __func__, Config.log_bus, LOG_ERR, "SET_WATCHDOG: wrong parameters from '%s'", thread_tech_id );
       Http_Send_json_response (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres", NULL);
       return;
     }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "WATCHDOG set", NULL );
  }
/******************************************************************************************************************************/
/* Http_traiter_set_cde_post: Positionne une Commande dans DLS                                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_set_cde_post ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request )
  { gchar *thread_tech_id;
    if (!Http_Check_Thread_signature ( path, msg, &thread_tech_id )) return;
    if (!thread_tech_id)
     { Http_Send_json_response (msg, SOUP_STATUS_BAD_REQUEST, "thread_tech_id missing", NULL);
       Info_new( __func__, Config.log_bus, LOG_ERR, "thread_tech_id missing for path %s", path );
       return;
     }

    if (! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) ) )
     { Info_new( __func__, Config.log_bus, LOG_ERR, "SET_CDE: wrong parameters from '%s'", thread_tech_id );
       Http_Send_json_response (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres", NULL);
       return;
     }
    Info_new( __func__, Config.log_bus, LOG_INFO,
              "SET_CDE from '%s': '%s:%s'=1", thread_tech_id,
              Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ) );
    Envoyer_commande_dls_data ( Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "CDE set", NULL );
  }
/******************************************************************************************************************************/
/* Http_traiter_set_ai_post: Positionne une AI dans DLS                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_set_ai_post ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request )
  { gchar *thread_tech_id;
    if (!Http_Check_Thread_signature ( path, msg, &thread_tech_id )) return;

    if ( Dls_data_set_AI_from_thread_ai ( request ) == FALSE )
     { Info_new( __func__, Config.log_bus, LOG_ERR, "SET_AI: wrong parameters from '%s'", thread_tech_id );
       Http_Send_json_response (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres", NULL);
       return;
     }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "AI set", NULL );
  }
/******************************************************************************************************************************/
/* Http_traiter_set_di_post: Positionne une DI dans DLS                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_set_di_post ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request )
  { gchar *thread_tech_id;
    if (!Http_Check_Thread_signature ( path, msg, &thread_tech_id )) return;

    if ( Dls_data_set_DI_from_thread_di ( request ) == FALSE )
     { Info_new( __func__, Config.log_bus, LOG_ERR, "SET_DI: wrong parameters from '%s'", thread_tech_id );
       Http_Send_json_response (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres", NULL);
       return;
     }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "DI set", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
