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
 static void Admin_json_phidget_status ( struct PROCESS *Lib, SoupMessage *msg )
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
 static void Admin_json_phidget_hub_list ( struct PROCESS *Lib, SoupMessage *msg )
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
 static void Admin_json_phidget_hub_del ( struct PROCESS *Lib, SoupMessage *msg )
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
 static void Admin_json_phidget_hub_set ( struct PROCESS *Lib, SoupMessage *msg )
  { gboolean retour;

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "hostname" ) && Json_has_member ( request, "description" ) &&
            Json_has_member ( request, "password" ) && Json_has_member ( request, "enable" ) &&
            Json_has_member ( request, "serial" ) && Json_has_member ( request, "tech_id" )
           )
       )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id     = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *hostname    = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *password    = Normaliser_chaine ( Json_get_string( request, "password" ) );

    if (Json_has_member ( request, "id" ))
     { retour = SQL_Write_new ( "UPDATE phidget_hub SET tech_id='%s', description='%s', hostname='%s', password='%s', serial='%d' "
                                "WHERE id='%d'",
                                tech_id, description, hostname, password, Json_get_int ( request, "serial" ), Json_get_int ( request, "id" ) );
     }
    else
     { retour = SQL_Write_new ( "INSERT INTO phidget_hub SET tech_id='%s', description='%s', hostname='%s', password='%s', serial='%d'",
                                tech_id, description, hostname, password, Json_get_int ( request, "serial" ) );
     }
    json_node_unref(request);

    if (retour)
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Lib->Thread_reload = TRUE;
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );

    g_free(tech_id);
    g_free(description);
    g_free(hostname);
    g_free(password);
  }
/******************************************************************************************************************************/
/* Admin_json_phidget_hub_start_stop: Start ou Stop un Hub Phidget                                                            */
/* Entrées: la connexion Websocket, le champ start/stop                                                                       */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_phidget_hub_start_stop ( struct PROCESS *Lib, SoupMessage *msg, gboolean start )
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
 static void Admin_json_phidget_map_list ( struct PROCESS *Lib, GHashTable *query, SoupMessage *msg )
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

    if (! strcasecmp( classe, "DI" ) )
     { if (SQL_Select_to_json_node ( RootNode, "mappings",
                                    "SELECT m.tech_id, m.acronyme, m.libelle, "
                                    "hub.hostname AS hub_hostname, hub.description AS hub_description, "
                                    "di.* FROM phidget_DI AS di "
                                    "INNER JOIN phidget_hub AS hub ON di.hub_id = hub.id "
                                    "INNER JOIN mnemos_DI AS m ON m.map_tech_id = CONCAT ( hub.tech_id, '_P', di.port ) " ) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref ( RootNode );
          return;
        }
     }
    else if (! strcasecmp( classe, "DO" ) )
     { if (SQL_Select_to_json_node ( RootNode, "mappings",
                                    "SELECT m.tech_id, m.acronyme, m.libelle, "
                                    "hub.hostname AS hub_hostname, hub.description AS hub_description, "
                                    "do.* FROM phidget_DO AS do "
                                    "INNER JOIN phidget_hub AS hub ON do.hub_id = hub.id "
                                    "INNER JOIN mnemos_DO AS m ON m.map_tech_id = CONCAT ( hub.tech_id, '_P', do.port ) " ) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref ( RootNode );
          return;
        }
     }
    else if (! strcasecmp( classe, "AI" ) )
     { if (SQL_Select_to_json_node ( RootNode, "mappings",
                                    "SELECT m.tech_id, m.acronyme, m.libelle, m.map_question_vocale, m.map_reponse_vocale, m.min, m.max, m.unite, "
                                    "hub.hostname AS hub_hostname, hub.description AS hub_description, "
                                    "ai.* FROM phidget_AI AS ai "
                                    "INNER JOIN phidget_hub AS hub ON ai.hub_id = hub.id "
                                    "INNER JOIN mnemos_AI AS m ON m.map_tech_id = CONCAT ( hub.tech_id, '_P', ai.port ) " ) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref ( RootNode );
          return;
        }
     }
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
 static void Admin_json_phidget_map_set ( struct PROCESS *Lib, SoupMessage *msg )
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
    gint   hub_id      = Json_get_int( request, "hub_id" );
    gint   port        = Json_get_int( request, "port" );

    gchar *phidget_classe;
         if (!strcasecmp ( capteur, "ADP1000-ORP" ))           phidget_classe="VoltageInput";
    else if (!strcasecmp ( capteur, "ADP1000-PH" ))            phidget_classe="PHSensor";
    else if (!strcasecmp ( capteur, "TMP1200_0-PT100-3850" ))  phidget_classe="TemperatureSensor";
    else if (!strcasecmp ( capteur, "TMP1200_0-PT100-3920" ))  phidget_classe="TemperatureSensor";
    else if (!strcasecmp ( capteur, "AC-CURRENT-10A" ))        phidget_classe="VoltageInput";
    else if (!strcasecmp ( capteur, "AC-CURRENT-25A" ))        phidget_classe="VoltageInput";
    else if (!strcasecmp ( capteur, "AC-CURRENT-50A" ))        phidget_classe="VoltageInput";
    else if (!strcasecmp ( capteur, "AC-CURRENT-100A" ))       phidget_classe="VoltageInput";
    else if (!strcasecmp ( capteur, "TEMP_1124_0" ))           phidget_classe="VoltageRatioInput";
    else if (!strcasecmp ( capteur, "DIGITAL-INPUT" ))         phidget_classe="DigitalInput";
    else if (!strcasecmp ( capteur, "REL2001_0" ))             phidget_classe="DigitalOutput";
    else phidget_classe="Unknown";

    if (! strcasecmp( classe, "DI" ) )
     { SQL_Write_new ( "UPDATE mnemos_DI SET map_thread='PHIDGET', map_tech_id=NULL "
                       "WHERE map_tech_id=CONCAT ( (SELECT tech_id FROM phidget_hub WHERE id=%d), '_P%d') ",
                       hub_id, port
                     );

       SQL_Write_new ( "UPDATE mnemos_DI SET map_thread='PHIDGET', "
                       "map_tech_id=CONCAT ( (SELECT tech_id FROM phidget_hub WHERE id=%d), '_P%d') "
                       "WHERE tech_id='%s' AND acronyme='%s'",
                       hub_id, port, tech_id, acronyme
                     );

       if (SQL_Write_new ( "INSERT INTO phidget_DI SET hub_id=%d, port=%d, classe='%s', capteur='%s' "
                           "ON DUPLICATE KEY UPDATE "
                           "classe=VALUES(classe),capteur=VALUES(capteur), port=VALUES(port), hub_id=VALUES(hub_id)",
                           hub_id, port, phidget_classe, capteur
                         ))
          { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else if (! strcasecmp( classe, "DO" ) )
     { SQL_Write_new ( "UPDATE mnemos_DO SET map_thread='PHIDGET', map_tech_id=NULL "
                       "WHERE map_tech_id=CONCAT ( (SELECT tech_id FROM phidget_hub WHERE id=%d), '_P%d') ",
                       hub_id, port
                     );

       SQL_Write_new ( "UPDATE mnemos_DO SET map_thread='PHIDGET', "
                       "map_tech_id=CONCAT ( (SELECT tech_id FROM phidget_hub WHERE id=%d), '_P%d') "
                       "WHERE tech_id='%s' AND acronyme='%s'",
                       hub_id, port, tech_id, acronyme
                     );

       if (SQL_Write_new ( "INSERT INTO phidget_DO SET hub_id=%d, port=%d, classe='%s', capteur='%s' "
                           "ON DUPLICATE KEY UPDATE "
                           "classe=VALUES(classe),capteur=VALUES(capteur), port=VALUES(port), hub_id=VALUES(hub_id)",
                           hub_id, port, phidget_classe, capteur
                         ))
          { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else if (! strcasecmp( classe, "AI" ) )
     { if ( ! (Json_has_member ( request, "intervalle" ) && Json_has_member ( request, "min" ) &&
               Json_has_member ( request, "max" ) && Json_has_member ( request, "unite" ) &&
               Json_has_member ( request, "map_question_vocale" ) && Json_has_member ( request, "map_reponse_vocale" )
          ) )
        { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
          goto end;
        }

       SQL_Write_new ( "UPDATE mnemos_AI SET map_thread='PHIDGET', map_tech_id=NULL "
                       "WHERE map_tech_id=CONCAT ( (SELECT tech_id FROM phidget_hub WHERE id=%d), '_P%d') ",
                       hub_id, port
                     );

       gchar *unite               = Normaliser_chaine( Json_get_string ( request, "unite" ) );
       gchar *map_question_vocale = Normaliser_chaine( Json_get_string ( request, "map_question_vocale" ) );
       gchar *map_reponse_vocale  = Normaliser_chaine( Json_get_string ( request, "map_reponse_vocale" ) );

       SQL_Write_new ( "UPDATE mnemos_AI SET map_thread='PHIDGET', "
                       "map_tech_id=CONCAT ( (SELECT tech_id FROM phidget_hub WHERE id=%d), '_P%d'), "
                       "min='%d', max='%d', unite='%s', map_question_vocale='%s', map_reponse_vocale='%s' "
                       "WHERE tech_id='%s' AND acronyme='%s'",
                       hub_id, port,
                       Json_get_int ( request, "min" ), Json_get_int ( request, "max" ), unite,
                       map_question_vocale, map_reponse_vocale,
                       tech_id, acronyme
                     );
       g_free(unite);
       g_free(map_question_vocale);
       g_free(map_reponse_vocale);

       if (SQL_Write_new ( "INSERT INTO phidget_AI SET hub_id=%d, port=%d, intervalle=%d, classe='%s', capteur='%s' "
                           "ON DUPLICATE KEY UPDATE intervalle=VALUES(intervalle),"
                           "classe=VALUES(classe),capteur=VALUES(capteur), port=VALUES(port), hub_id=VALUES(hub_id)",
                           hub_id, port, Json_get_int( request, "intervalle" ), phidget_classe, capteur, tech_id, acronyme
                         ))
          { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
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
 static void Admin_json_phidget_map_del ( struct PROCESS *Lib, SoupMessage *msg )
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
 void Admin_json ( struct PROCESS *lib, SoupMessage *msg, const char *path, GHashTable *query, gint access_level )
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
