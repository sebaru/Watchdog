/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_phidget.c        Gestion des responses Admin PHIDGET au serveur watchdog                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       dim. 05 sept. 2010 12:01:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_phidget.c
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

 #include "watchdogd.h"
 #include "Phidget.h"

 extern struct PHIDGET_CONFIG Cfg_phidget;
/******************************************************************************************************************************/
/* Admin_json_phidget_list : fonction appelée pour lister les modules phidget                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_phidget_status ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_node_add_bool ( RootNode, "thread_is_running", Lib->Thread_run );
    if (Lib->Thread_run)                                    /* Warning : Cfg_phidget does not exist if thread is not running ! */
     { Json_node_add_int ( RootNode, "nbr_sensors", g_slist_length(Cfg_phidget.Liste_sensors) ); }

    gchar *buf = Json_node_to_string(RootNode);
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Admin_json_phidget_hub_list: Renvoie la liste des hub Phidget configurés                                                   */
/* Entrées: la connexion Websocket destinataire                                                                               */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_phidget_hub_list ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "hubs", "SELECT * FROM phidget_hub" ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(RootNode);
       return;
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getdlslist: Traite une requete sur l'URI dlslist                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static void Admin_json_phidget_hub_del ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  {
    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    SQL_Write_new( "DELETE FROM phidget_hub WHERE id='%d'", Json_get_int ( request, "id" ) );
    soup_message_set_status (msg, SOUP_STATUS_OK);
    Lib->Thread_reload = TRUE;
  }
/******************************************************************************************************************************/
/* Admin_json_phidget_set: Met à jour une entrée WAGO                                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_phidget_hub_set ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { gboolean retour;

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "hostname" ) && Json_has_member ( request, "description" ) &&
            Json_has_member ( request, "password" ) && Json_has_member ( request, "enable" ) &&
            Json_has_member ( request, "serial" )
           )
       )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *hostname    = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *password    = Normaliser_chaine ( Json_get_string( request, "password" ) );

    if (Json_has_member ( request, "id" ))
     { retour = SQL_Write_new ( "UPDATE phidget_hub SET description='%s', hostname='%s', password='%s', serial='%d' WHERE id='%d'",
                                description, hostname, password, Json_get_int ( request, "serial" ), Json_get_int ( request, "id" ) );
     }
    else
     { retour = SQL_Write_new ( "INSERT INTO phidget_hub SET description='%s', hostname='%s', password='%s', serial='%d'",
                                description, hostname, password, Json_get_int ( request, "serial" ) );
     }
    json_node_unref(request);

    g_free(description);
    g_free(hostname);
    g_free(password);
    if (retour)
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Lib->Thread_reload = TRUE;
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
  }
/******************************************************************************************************************************/
/* Admin_json_phidget_hub_start_stop: Start ou Stop un Hub Phidget                                                            */
/* Entrées: la connexion Websocket, le champ start/stop                                                                       */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_phidget_hub_start_stop ( struct LIBRAIRIE *Lib, SoupMessage *msg, gboolean start )
  { if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "id" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gint id = Json_get_int ( request, "id" );
    json_node_unref(request);


    if (SQL_Write_new ( "UPDATE phidget_hub SET enable='%d' WHERE id='%d'", start, id ) )
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Lib->Thread_reload = TRUE;
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
  }
/******************************************************************************************************************************/
/* Admin_json_phidget_map_list: Recupère la liste des mapping d'une certaine classe pour le thread phidget                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_phidget_map_list ( struct LIBRAIRIE *Lib, GHashTable *query, SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    gpointer classe = g_hash_table_lookup ( query, "classe" );
    if (!classe)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

/*         if (! strcasecmp( classe, "DI" ) ) target = "mnemos_DI";
    else if (! strcasecmp( classe, "DO" ) ) target = "mnemos_DO";
    else*/ if (! strcasecmp( classe, "AI" ) )
     { if (SQL_Select_to_json_node ( RootNode, "mappings",
                                    "SELECT m.tech_id, m.acronyme, m.libelle, m.map_question_vocale, m.map_reponse_vocale, m.min, m.max, m.unite, "
                                    "hub.hostname AS hub_hostname, hub.description AS hub_description, "
                                    "ai.* FROM phidget_AI AS ai "
                                    "INNER JOIN mnemos_AI AS m ON ai.mnemo_id=m.id "
                                    "INNER JOIN phidget_hub AS hub ON ai.hub_id = hub.id " ) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref ( RootNode );
          return;
        }
     }
/*    else if (! strcasecmp( classe, "AO" ) ) target = "mnemos_AO";*/
    else
     {	soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Wrong class" );
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
/* Admin_json_phidget_map_set: ajoute un mapping dans la base de données Phidget                                              */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_phidget_map_set ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
            Json_has_member ( request, "capteur" ) && Json_has_member ( request, "classe" ) &&
            Json_has_member ( request, "hub_id" ) && Json_has_member ( request, "port" )
           )
       )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id     = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *acronyme    = Normaliser_chaine ( Json_get_string( request, "acronyme" ) );
    gchar *capteur     = Normaliser_chaine ( Json_get_string( request, "capteur" ) );
    gchar *classe      = Json_get_string( request, "classe" );

    gchar *phidget_classe;
         if (!strcasecmp ( capteur, "ADP1000-ORP" )) phidget_classe="VoltageInput";
    else if (!strcasecmp ( capteur, "ADP1000-PH" ))  phidget_classe="PHSensor";
    else if (!strcasecmp ( capteur, "TMP1200_0-PT100-3850" ))  phidget_classe="TemperatureSensor";
    else if (!strcasecmp ( capteur, "TMP1200_0-PT100-3920" ))  phidget_classe="TemperatureSensor";
    else phidget_classe="Unknown";

/*    if (! strcasecmp( classe, "DI" ) )
     { SQL_Write_new ( "UPDATE mnemos_DI SET map_thread=NULL, map_tech_id=NULL, map_tag=NULL "
                       " WHERE map_tech_id='%s' AND map_tag='%d';", NOM_THREAD, io_id );

       g_snprintf( requete, sizeof(requete),
                   "UPDATE mnemos_DI SET map_thread='%s', map_tech_id='%s', map_tag='%d' "
                   " WHERE tech_id='%s' AND acronyme='%s';", NOM_THREAD, NOM_THREAD, io_id, tech_id, acronyme );

       if (SQL_Write_new ("UPDATE mnemos_DI SET map_thread='%s', map_tech_id='%s', map_tag='%d' "
                          " WHERE tech_id='%s' AND acronyme='%s';", NOM_THREAD, NOM_THREAD, io_id, tech_id, acronyme))
          { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else*/ if (! strcasecmp( classe, "AI" ) )
     { if ( ! (Json_has_member ( request, "intervalle" ) && Json_has_member ( request, "min" ) &&
               Json_has_member ( request, "max" ) && Json_has_member ( request, "unite" ) &&
               Json_has_member ( request, "map_question_vocale" ) && Json_has_member ( request, "map_reponse_vocale" )
          ) )
        { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
          goto end;
        }

       gchar *unite               = Normaliser_chaine( Json_get_string ( request, "unite" ) );
       gchar *map_question_vocale = Normaliser_chaine( Json_get_string ( request, "map_question_vocale" ) );
       gchar *map_reponse_vocale  = Normaliser_chaine( Json_get_string ( request, "map_reponse_vocale" ) );

       SQL_Write_new ( "UPDATE mnemos_AI SET map_thread='PHIDGET', map_tech_id='PHIDGET', "
                       "min='%d', max='%d', unite='%s', map_question_vocale='%s', map_reponse_vocale='%s' "
                       "WHERE tech_id='%s' AND acronyme='%s'",
                       Json_get_int ( request, "min" ), Json_get_int ( request, "max" ), unite,
                       map_question_vocale, map_reponse_vocale,
                       tech_id, acronyme
                     );
       g_free(unite);
       g_free(map_question_vocale);
       g_free(map_reponse_vocale);

       SQL_Write_new ("UPDATE phidget_AI SET mnemo_id=NULL "
                      "WHERE mnemo_id=(SELECT id FROM mnemos_AI WHERE tech_id='%s' AND acronyme='%s')",
                       tech_id, acronyme
                     );

       if (SQL_Write_new ( "INSERT INTO phidget_AI SET hub_id=%d, port=%d, intervalle=%d, classe='%s', capteur='%s',"
                           "mnemo_id=(SELECT id FROM mnemos_AI WHERE tech_id='%s' AND acronyme='%s') "
                           "ON DUPLICATE KEY UPDATE mnemo_id=VALUES(mnemo_id), intervalle=VALUES(intervalle)",
                           Json_get_int( request, "hub_id"), Json_get_int( request, "port"), Json_get_int( request, "intervalle" ),
                           phidget_classe, capteur,
                           tech_id, acronyme
                         ))
          { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
/*
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
*/
    else
     {	soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Classe inconnue");  }
    Dls_recalculer_arbre_comm();/* Calcul de l'arbre de communication car il peut y avoir de nouvelles dependances sur les plugins */
end:
    g_free(tech_id);
    g_free(acronyme);
    g_free(capteur);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Admin_json_phidget_map_del: supprime un mapping dans la base de données Phidget                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_phidget_map_del ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "classe" ) && Json_has_member ( request, "id" )) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *classe      = Json_get_string( request, "classe" );

    if (! strcasecmp( classe, "DI" ) )
     { if (SQL_Write_new ( "DELETE FROM phidget_DI WHERE id='%d'", Json_get_int ( request, "id" ) ))
          { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else if (! strcasecmp( classe, "AI" ) )
     { if (SQL_Write_new ( "DELETE FROM phidget_AI WHERE id='%d'", Json_get_int ( request, "id" ) ))
          { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else if (! strcasecmp( classe, "DO" ) )
     { if (SQL_Write_new ( "DELETE FROM phidget_DO WHERE id='%d'", Json_get_int ( request, "id" ) ))
          { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else if (! strcasecmp( classe, "AO" ) )
     { if (SQL_Write_new ( "DELETE FROM phidget_AO WHERE id='%d'", Json_get_int ( request, "id" ) ))
          { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else
     {	soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Classe inconnue");  }
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 void Admin_json ( struct LIBRAIRIE *lib, SoupMessage *msg, const char *path, GHashTable *query, gint access_level )
  { if (access_level < 6)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }
         if (!strcasecmp(path, "/status"))    { Admin_json_phidget_status ( lib, msg ); }
    else if (!strcasecmp(path, "/hub/list"))  { Admin_json_phidget_hub_list ( lib, msg ); }
    else if (!strcasecmp(path, "/hub/del"))   { Admin_json_phidget_hub_del ( lib, msg ); }
    else if (!strcasecmp(path, "/hub/set"))   { Admin_json_phidget_hub_set ( lib, msg ); }
    else if (!strcasecmp(path, "/hub/start")) { Admin_json_phidget_hub_start_stop ( lib, msg, TRUE ); }
    else if (!strcasecmp(path, "/hub/stop"))  { Admin_json_phidget_hub_start_stop ( lib, msg, FALSE ); }
    else if (!strcasecmp(path, "/map/list"))  { Admin_json_phidget_map_list ( lib, query, msg ); }
    else if (!strcasecmp(path, "/map/set"))   { Admin_json_phidget_map_set ( lib, msg ); }
    else if (!strcasecmp(path, "/map/del"))   { Admin_json_phidget_map_del ( lib, msg ); }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
