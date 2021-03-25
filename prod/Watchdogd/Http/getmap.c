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
  { gchar *target;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gpointer classe = g_hash_table_lookup ( query, "classe" );
    if (!classe)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *thread = Normaliser_as_ascii ( g_hash_table_lookup ( query, "thread" ) );
    if (!thread)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

         if (! strcasecmp( classe, "DI" ) ) target = "mnemos_DI";
    else if (! strcasecmp( classe, "DO" ) ) target = "mnemos_DO";
    else if (! strcasecmp( classe, "AI" ) ) target = "mnemos_AI";
    else if (! strcasecmp( classe, "AO" ) ) target = "mnemos_AO";
    else
     {	soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Wrong class" );
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "mappings", "SELECT * FROM %s AS m WHERE map_thread='%s'", target, thread ) == FALSE)
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
/* Http_traiter_map_del: supprime un mapping dans la base de données                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_map_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar requete[256], *target;

    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "classe" ) &&
            Json_has_member ( request, "map_tech_id" ) && Json_has_member ( request, "map_tag" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

         if (! strcasecmp( Json_get_string( request, "classe" ), "DI" ) ) target = "mnemos_DI";
    else if (! strcasecmp( Json_get_string( request, "classe" ), "DO" ) ) target = "mnemos_DO";
    else if (! strcasecmp( Json_get_string( request, "classe" ), "AI" ) ) target = "mnemos_AI";
    else if (! strcasecmp( Json_get_string( request, "classe" ), "AO" ) ) target = "mnemos_AO";
    else
     {	soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvaise classe");
		     return;
     }

    gchar *tech_id = Normaliser_chaine ( Json_get_string( request, "map_tech_id" ) );
    gchar *tag     = Normaliser_chaine ( Json_get_string( request, "map_tag" ) );
    json_node_unref(request);
    g_snprintf( requete, sizeof(requete), "UPDATE %s SET map_thread = NULL, map_tech_id = NULL, map_tag = NULL "
                                          "WHERE map_tech_id='%s' AND map_tag='%s'", target, tech_id, tag );
    g_free(tech_id);
    g_free(tag);

    if (SQL_Write (requete)) soup_message_set_status (msg, SOUP_STATUS_OK);
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
  }
/******************************************************************************************************************************/
/* Http_traiter_map_set: ajoute un mapping dans la base de données                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_map_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar requete[512];

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
            Json_has_member ( request, "map_tech_id" ) && Json_has_member ( request, "map_tag" ) &&
            Json_has_member ( request, "classe" ) && Json_has_member ( request, "thread" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *map_tech_id = Normaliser_as_ascii ( Json_get_string( request, "map_tech_id" ) );
    gchar *map_tag     = Normaliser_chaine   ( Json_get_string( request, "map_tag" ) );
    gchar *tech_id     = Normaliser_as_ascii ( Json_get_string( request, "tech_id" ) );
    gchar *acronyme    = Normaliser_as_ascii ( Json_get_string( request, "acronyme" ) );
    gchar *thread      = Normaliser_as_ascii ( Json_get_string( request, "thread" ) );
    gchar *classe      = Json_get_string( request, "classe" );

    if (! strcasecmp( classe, "DI" ) )
     { g_snprintf( requete, sizeof(requete),
                   "UPDATE mnemos_DI SET map_thread=NULL, map_tech_id=NULL, map_tag=NULL "
                   " WHERE map_tech_id='%s' AND map_tag='%s';", map_tech_id, map_tag );

       SQL_Write (requete);

       g_snprintf( requete, sizeof(requete),
                   "UPDATE mnemos_DI SET map_thread='%s', map_tech_id='%s', map_tag='%s' "
                   " WHERE tech_id='%s' AND acronyme='%s';", thread, map_tech_id, map_tag, tech_id, acronyme );

       if (SQL_Write (requete)) { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else if (! strcasecmp( classe, "AI" ) )
     { if ( ! (Json_has_member ( request, "type" ) && Json_has_member ( request, "min" ) &&
               Json_has_member ( request, "max" ) && Json_has_member ( request, "unite" ) &&
               Json_has_member ( request, "map_question_vocale" ) && Json_has_member ( request, "map_reponse_vocale" )
          ) )
        { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
          goto end;
        }

       g_snprintf( requete, sizeof(requete),
                   "UPDATE mnemos_AI SET map_thread=NULL, map_tech_id=NULL, map_tag=NULL "
                   " WHERE map_tech_id='%s' AND map_tag='%s';", map_tech_id, map_tag );

       SQL_Write (requete);

       gchar *unite               = Normaliser_chaine( Json_get_string ( request, "unite" ) );
       gchar *map_question_vocale = Normaliser_chaine( Json_get_string ( request, "map_question_vocale" ) );
       gchar *map_reponse_vocale  = Normaliser_chaine( Json_get_string ( request, "map_reponse_vocale" ) );
       g_snprintf( requete, sizeof(requete),
                   "UPDATE mnemos_AI SET map_thread='%s', map_tech_id='%s', map_tag='%s',"
                   " type='%d', min='%d', max='%d', unite='%s', map_question_vocale='%s', map_reponse_vocale='%s'"
                   " WHERE tech_id='%s' AND acronyme='%s';",
                   thread, map_tech_id, map_tag, Json_get_int ( request, "type" ),
                   Json_get_int ( request, "min" ), Json_get_int ( request, "max" ),
                   unite, map_question_vocale, map_reponse_vocale,
                   tech_id, acronyme );
       g_free(unite);
       g_free(map_question_vocale);
       g_free(map_reponse_vocale);
       if (SQL_Write (requete)) { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else if (! strcasecmp( classe, "DO" ) )
     { g_snprintf( requete, sizeof(requete),
                   "UPDATE mnemos_DO SET map_thread=NULL, map_tech_id=NULL, map_tag=NULL "
                   " WHERE map_tech_id='%s' AND map_tag='%s';", map_tech_id, map_tag );

       SQL_Write (requete);

       g_snprintf( requete, sizeof(requete),
                   "UPDATE mnemos_DO SET map_thread='%s', map_tech_id='%s', map_tag='%s' "
                   " WHERE tech_id='%s' AND acronyme='%s';", thread, map_tech_id, map_tag, tech_id, acronyme );

       if (SQL_Write (requete)) soup_message_set_status (msg, SOUP_STATUS_OK);
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else if (! strcasecmp( classe, "AO" ) )
     { if ( ! (Json_has_member ( request, "type" ) && Json_has_member ( request, "min" ) &&
               Json_has_member ( request, "max" ) && Json_has_member ( request, "unite" ) &&
               Json_has_member ( request, "map_question_vocale" ) && Json_has_member ( request, "map_reponse_vocale" )
          ) )
        { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
          goto end;
        }

       g_snprintf( requete, sizeof(requete),
                   "UPDATE mnemos_AO SET map_thread=NULL, map_tech_id=NULL, map_tag=NULL "
                   " WHERE map_tech_id='%s' AND map_tag='%s';", map_tech_id, map_tag );

       SQL_Write (requete);

       gchar *unite               = Normaliser_chaine( Json_get_string ( request, "unite" ) );
       gchar *map_question_vocale = Normaliser_chaine( Json_get_string ( request, "map_question_vocale" ) );
       gchar *map_reponse_vocale  = Normaliser_chaine( Json_get_string ( request, "map_reponse_vocale" ) );
       g_snprintf( requete, sizeof(requete),
                   "UPDATE mnemos_AO SET map_thread='%s', map_tech_id='%s', map_tag='%s',"
                   " type='%d', min='%d', max='%d', unite='%s', map_question_vocale='%s', map_reponse_vocale='%s'"
                   " WHERE tech_id='%s' AND acronyme='%s';",
                   thread, map_tech_id, map_tag, Json_get_int ( request, "type" ),
                   Json_get_int ( request, "min" ), Json_get_int ( request, "max" ),
                   unite, map_question_vocale, map_reponse_vocale,
                   tech_id, acronyme );
       g_free(unite);
       g_free(map_question_vocale);
       g_free(map_reponse_vocale);
       if (SQL_Write (requete)) { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else
     {	soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Classe inconnue");  }
    Dls_recalculer_arbre_comm();/* Calcul de l'arbre de communication car il peut y avoir de nouvelles dependances sur les plugins */
end:
    g_free(map_tag);
    json_node_unref(request);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
