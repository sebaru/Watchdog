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

/******************************************************************************************************************************/
/* Admin_config : fonction appelé par le thread http lors d'une requete POST sur config PROCESS                               */
/* Entrée : la librairie, et le Json recu                                                                                     */
/* Sortie : la base de données est mise à jour                                                                                */
/******************************************************************************************************************************/
 void Admin_config ( struct PROCESS *lib, gpointer msg, JsonNode *request )
  { if ( Json_has_member ( request, "uuid" ) && Json_has_member ( request, "thread_tech_id" ) &&
         Json_has_member ( request, "id" ) && Json_has_member ( request, "enable" ) )
     { SQL_Write_new ( "UPDATE %s SET enable='%d' WHERE id='%d'", lib->name, Json_get_bool(request, "enable"),
                       Json_get_int ( request, "id" ) );
       Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' updated.", __func__,
                 Json_get_string ( request, "uuid" ), Json_get_string ( request, "thread_tech_id" ) );
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }

    if ( ! (Json_has_member ( request, "uuid" ) && Json_has_member ( request, "thread_tech_id" ) &&
            Json_has_member ( request, "hostname" ) && Json_has_member ( request, "description" ) &&
            Json_has_member ( request, "password" ) && Json_has_member ( request, "serial" )
           )
        )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *uuid           = Normaliser_chaine ( Json_get_string( request, "uuid" ) );
    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *hostname       = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *description    = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *password       = Normaliser_chaine ( Json_get_string( request, "password" ) );
    gchar *serial         = Normaliser_chaine ( Json_get_string( request, "serial" ) );

    if (Json_has_member ( request, "id" ))
     { SQL_Write_new ( "UPDATE %s SET uuid='%s', thread_tech_id='%s', description='%s', hostname='%s', password='%s', serial='%s' ",
                       "WHERE id='%d'",
                       lib->name, uuid, thread_tech_id, description, hostname, password, serial,
                       Json_get_int ( request, "id" ) );
       Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' updated.", __func__, uuid, thread_tech_id );
     }
    else
     { SQL_Write_new ( "INSERT INTO %s SET uuid='%s', thread_tech_id='%s', description='%s', hostname='%s', password='%s', serial='%s' ",
                       lib->name, uuid, thread_tech_id, description, hostname, password, serial );
       Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' created.", __func__, uuid, thread_tech_id );
     }

    g_free(uuid);
    g_free(thread_tech_id);
    g_free(hostname);
    g_free(description);
    g_free(password);
    g_free(serial);

    soup_message_set_status (msg, SOUP_STATUS_OK);
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

    if ( ! (Json_has_member ( request, "thread_tech_id" ) && Json_has_member ( request, "acronyme" ) &&
            Json_has_member ( request, "capteur" ) && Json_has_member ( request, "classe" ) &&
            Json_has_member ( request, "hub_id" ) && Json_has_member ( request, "port" )
           )
       )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *thread_tech_id     = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
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
     { SQL_Write_new ( "UPDATE mnemos_DI SET map_thread='PHIDGET', map_thread_tech_id=NULL "
                       "WHERE map_thread_tech_id=CONCAT ( (SELECT thread_tech_id FROM phidget WHERE id=%d), '_P%d') ",
                       hub_id, port
                     );

       SQL_Write_new ( "UPDATE mnemos_DI SET map_thread='PHIDGET', "
                       "map_thread_tech_id=CONCAT ( (SELECT thread_tech_id FROM phidget WHERE id=%d), '_P%d') "
                       "WHERE thread_tech_id='%s' AND acronyme='%s'",
                       hub_id, port, thread_tech_id, acronyme
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
     { SQL_Write_new ( "UPDATE mnemos_DO SET map_thread='PHIDGET', map_thread_tech_id=NULL "
                       "WHERE map_thread_tech_id=CONCAT ( (SELECT thread_tech_id FROM phidget WHERE id=%d), '_P%d') ",
                       hub_id, port
                     );

       SQL_Write_new ( "UPDATE mnemos_DO SET map_thread='PHIDGET', "
                       "map_thread_tech_id=CONCAT ( (SELECT thread_tech_id FROM phidget WHERE id=%d), '_P%d') "
                       "WHERE thread_tech_id='%s' AND acronyme='%s'",
                       hub_id, port, thread_tech_id, acronyme
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

       SQL_Write_new ( "UPDATE mnemos_AI SET map_thread='PHIDGET', map_thread_tech_id=NULL "
                       "WHERE map_thread_tech_id=CONCAT ( (SELECT thread_tech_id FROM phidget WHERE id=%d), '_P%d') ",
                       hub_id, port
                     );

       gchar *unite               = Normaliser_chaine( Json_get_string ( request, "unite" ) );
       gchar *map_question_vocale = Normaliser_chaine( Json_get_string ( request, "map_question_vocale" ) );
       gchar *map_reponse_vocale  = Normaliser_chaine( Json_get_string ( request, "map_reponse_vocale" ) );

       SQL_Write_new ( "UPDATE mnemos_AI SET map_thread='PHIDGET', "
                       "map_thread_tech_id=CONCAT ( (SELECT thread_tech_id FROM phidget WHERE id=%d), '_P%d'), "
                       "min='%d', max='%d', unite='%s', map_question_vocale='%s', map_reponse_vocale='%s' "
                       "WHERE thread_tech_id='%s' AND acronyme='%s'",
                       hub_id, port,
                       Json_get_int ( request, "min" ), Json_get_int ( request, "max" ), unite,
                       map_question_vocale, map_reponse_vocale,
                       thread_tech_id, acronyme
                     );
       g_free(unite);
       g_free(map_question_vocale);
       g_free(map_reponse_vocale);

       if (SQL_Write_new ( "INSERT INTO phidget_AI SET hub_id=%d, port=%d, intervalle=%d, classe='%s', capteur='%s' "
                           "ON DUPLICATE KEY UPDATE intervalle=VALUES(intervalle),"
                           "classe=VALUES(classe),capteur=VALUES(capteur), port=VALUES(port), hub_id=VALUES(hub_id)",
                           hub_id, port, Json_get_int( request, "intervalle" ), phidget_classe, capteur, thread_tech_id, acronyme
                         ))
          { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
     }
    else
     {	soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Classe inconnue");  }
    Dls_recalculer_arbre_comm();/* Calcul de l'arbre de communication car il peut y avoir de nouvelles dependances sur les plugins */
end:
    g_free(thread_tech_id);
    g_free(acronyme);
    g_free(capteur);
    json_node_unref(request);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
