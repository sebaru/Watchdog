/******************************************************************************************************************************/
/* Watchdogd/Http/gethorloge.c       Gestion des requests sur l'URI /horloge du webservice                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    26.12.2020 15:01:11 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * gethorloge.c
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
/* Http_Traiter_get_horloge: Fourni une list JSON des elements d'un horloge                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_horloge_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;

    gpointer tech_id_string = g_hash_table_lookup ( query, "tech_id" );
    if (!tech_id_string)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    gchar *tech_id = Normaliser_chaine ( tech_id_string );
    if (!tech_id)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Memory Error");
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       g_free(tech_id);
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "horloges",
                                 "SELECT * FROM mnemos_HORLOGE WHERE tech_id='%s' AND access_level<=%d",
                                  tech_id, session->access_level )==FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref ( RootNode );
       g_free(tech_id);
       return;
     }
    g_free(tech_id);

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_horloge: Fourni une list JSON des elements d'un horloge                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_horloge_ticks_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                        SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;

    gpointer horloge_id_string = g_hash_table_lookup ( query, "horloge_id" );
    if ( !horloge_id_string )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    gint horloge_id = atoi (horloge_id_string);

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "horloge_ticks",
                                 "SELECT t.* FROM mnemos_HORLOGE_ticks as t INNER JOIN mnemos_HORLOGE as h"
                                 " ON t.horloge_id = h.id WHERE h.id=%d AND access_level<=%d",
                                 horloge_id, session->access_level )==FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref ( RootNode );
       return;
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_horloge: Fourni une list JSON des elements d'un horloge                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_horloge_ticks_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data )
  { gchar chaine[256];

    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint tick_id = Json_get_int ( request, "id" );
    json_node_unref(request);

    g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_HORLOGE_ticks WHERE id=%d/* AND access_level<='%d'*/",
                tick_id, session->access_level );
    if (SQL_Write (chaine)==FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Delete Error");
       return;
     }

/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    Audit_log ( session, "horloge_tick '%d' deleted", tick_id );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_horloge: Fourni une list JSON des elements d'un horlogeoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_horloge_ticks_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data )
  { if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    gchar chaine[512], critere[256];
    if ( Json_has_member ( request, "id" ) )                                                                       /* Edition */
     { g_snprintf( chaine, sizeof(chaine), "UPDATE mnemos_HORLOGE_ticks AS t INNER JOIN mnemos_HORLOGE AS h ON t.horloge_id=h.id "
                                           "SET date_modif=NOW()" );
     }
    else if ( ! (Json_has_member ( request, "horloge_id" ) ) )                                      /* Ajout dans une horloge */
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    else
     { g_snprintf( chaine, sizeof(chaine), "INSERT INTO mnemos_HORLOGE_ticks "
                                           "SET horloge_id=%d", Json_get_int ( request, "horloge_id" ) );
     }

    if ( Json_has_member ( request, "heure" ) && Json_has_member ( request, "minute" ) )
     { g_snprintf( critere, sizeof(critere), ", heure=%d, minute=%d",
                   Json_get_int ( request, "heure" ), Json_get_int ( request, "minute" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "lundi" ) )
     { g_snprintf( critere, sizeof(critere), ", lundi=%d", Json_get_bool ( request, "lundi" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "mardi" ) )
     { g_snprintf( critere, sizeof(critere), ", mardi=%d", Json_get_bool ( request, "mardi" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "mercredi" ) )
     { g_snprintf( critere, sizeof(critere), ", mercredi=%d", Json_get_bool ( request, "mercredi" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "jeudi" ) )
     { g_snprintf( critere, sizeof(critere), ", jeudi=%d", Json_get_bool ( request, "jeudi" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "vendredi" ) )
     { g_snprintf( critere, sizeof(critere), ", vendredi=%d", Json_get_bool ( request, "vendredi" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "samedi" ) )
     { g_snprintf( critere, sizeof(critere), ", samedi=%d", Json_get_bool ( request, "samedi" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "dimanche" ) )
     { g_snprintf( critere, sizeof(critere), ", dimanche=%d", Json_get_bool ( request, "dimanche" ) );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if ( Json_has_member ( request, "id" ) )                                                                       /* Edition */
     { g_snprintf( critere, sizeof(critere), " WHERE access_level <= %d", session->access_level );
       g_strlcat ( chaine, critere, sizeof(chaine) );
       g_snprintf( critere, sizeof(critere), " AND t.id=%d", Json_get_int ( request, "id") );
       g_strlcat ( chaine, critere, sizeof(chaine) );
     }

    if (SQL_Write (chaine)) { soup_message_set_status (msg, SOUP_STATUS_OK); }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
    json_node_unref(request);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
