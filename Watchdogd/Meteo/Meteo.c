/******************************************************************************************************************************/
/* Watchdogd/Meteo/Meteo.c        Gestion de l'méteo pour Watchdog v3.0                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    12.03.2021 20:49:16 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Meteo.c
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

 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <string.h>
 #include <unistd.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Meteo.h"

/******************************************************************************************************************************/
/* Creer_DB: Creer la table associée au Process                                                                               */
/* Entrée: le pointeur sur le PROCESS                                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Meteo_Creer_DB ( struct PROCESS *lib )
  {
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'.", __func__, lib->database_version );

    SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                    "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                    "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                    "`uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                    "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                    "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`token` VARCHAR(65) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`code_insee` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "FOREIGN KEY (`uuid`) REFERENCES `processes` (`uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                    ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", lib->name );

    Process_set_database_version ( lib, 1 );
  }
/******************************************************************************************************************************/
/* Meteo_get_ephemeride: Récupère l'ephemeride auprès de meteoconcept                                                         */
/* Entrée: Niet                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Meteo_get_ephemeride ( struct SUBPROCESS *module )
  { gchar query[256];
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *token      = Json_get_string ( module->config, "token" );
    gchar *code_insee = Json_get_string ( module->config, "code_insee" );
    g_snprintf( query, sizeof(query), "https://api.meteo-concept.com/api/ephemeride/0?token=%s&insee=%s", token, code_insee );

    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
             "%s: Starting getting data for code_insee '%s'", __func__, code_insee );
/********************************************************* Envoi de la requete ************************************************/
    SoupSession *connexion = soup_session_new();
    SoupMessage *soup_msg  = soup_message_new ( "GET", query );
    soup_message_set_request ( soup_msg, "application/json; charset=UTF-8", SOUP_MEMORY_STATIC, NULL, 0 );
    soup_session_send_message (connexion, soup_msg);

    gchar *reason_phrase = Http_Msg_reason_phrase(soup_msg);
    gint   status_code   = Http_Msg_status_code ( soup_msg );

    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: Status %d, reason %s", __func__, status_code, reason_phrase );
    if (status_code!=200)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: Error: %s\n", __func__, reason_phrase );
       SubProcess_send_comm_to_master_new ( module, FALSE );
     }
    else
     { gint heure, minute;
       JsonNode *response   = Http_Response_Msg_to_Json ( soup_msg );
       JsonNode *city       = Json_get_object_as_node ( response, "city" );
       JsonNode *ephemeride = Json_get_object_as_node ( response, "ephemeride" );
       gchar *city_name     = Json_get_string ( city, "name" );
       gchar *sunrise       = Json_get_string ( ephemeride, "sunrise" );
       gchar *sunset        = Json_get_string ( ephemeride, "sunset" );
       if ( sscanf ( sunrise, "%d:%d", &heure, &minute ) == 2)
        { Horloge_del_all_ticks ( thread_tech_id, "SUNRISE" );
          Horloge_add_tick      ( thread_tech_id, "SUNRISE", heure, minute );
          Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                   "%s: %s -> sunrise at %02d:%02d", __func__, city_name, heure, minute );
        }
       if ( sscanf ( sunset, "%d:%d", &heure, &minute ) == 2)
        { Horloge_del_all_ticks ( thread_tech_id, "SUNSET" );
          Horloge_add_tick      ( thread_tech_id, "SUNSET", heure, minute );
          Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                   "%s: %s ->  sunset at %02d:%02d", __func__, city_name, heure, minute );
        }
       Json_node_unref ( response );
       SubProcess_send_comm_to_master_new ( module, TRUE );
     }
    g_object_unref( soup_msg );
    soup_session_abort ( connexion );
  }
/******************************************************************************************************************************/
/* Meteo_update_forecast: Met a jour le forecast auprès de meteoconcept                                                       */
/* Entrée: Nier                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Meteo_update_forecast ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct SUBPROCESS *module = user_data;
    struct METEO_VARS *vars = module->vars;

    gint day       = Json_get_int ( element, "day" );
    gint temp_min  = Json_get_int ( element, "tmin" );
    gint temp_max  = Json_get_int ( element, "tmax" );
    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
              "%s: day %02d -> temp_min=%02d, temp_max=%02d", __func__, day, temp_min, temp_max );

    Http_Post_to_local_BUS_AI ( module, vars->Temp_min[day],         1.0*Json_get_int ( element, "tmin" ), TRUE );
    Http_Post_to_local_BUS_AI ( module, vars->Temp_max[day],         1.0*Json_get_int ( element, "tmax" ), TRUE );
    Http_Post_to_local_BUS_AI ( module, vars->Proba_pluie[day],      1.0*Json_get_int ( element, "probarain" ), TRUE );
    Http_Post_to_local_BUS_AI ( module, vars->Proba_gel[day],        1.0*Json_get_int ( element, "probafrost" ), TRUE );
    Http_Post_to_local_BUS_AI ( module, vars->Proba_brouillard[day], 1.0*Json_get_int ( element, "probafog" ), TRUE );
    Http_Post_to_local_BUS_AI ( module, vars->Proba_vent_70[day],    1.0*Json_get_int ( element, "probawind70" ), TRUE );
    Http_Post_to_local_BUS_AI ( module, vars->Proba_vent_100[day],   1.0*Json_get_int ( element, "probawind100" ), TRUE );
    Http_Post_to_local_BUS_AI ( module, vars->Proba_vent_orage[day], 1.0*Json_get_int ( element, "gustx" ), TRUE );
    Http_Post_to_local_BUS_AI ( module, vars->Vent_10m[day],         1.0*Json_get_int ( element, "wind10m" ), TRUE );
    Http_Post_to_local_BUS_AI ( module, vars->Direction_vent[day],   1.0*Json_get_int ( element, "dirwind10m" ), TRUE );
    Http_Post_to_local_BUS_AI ( module, vars->Rafale_vent[day],      1.0*Json_get_int ( element, "gust10m" ), TRUE );
  }
/******************************************************************************************************************************/
/* Meteo_get_forecast: Récupère le forecast auprès de meteoconcept                                                            */
/* Entrée: Nier                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Meteo_get_forecast ( struct SUBPROCESS *module )
  { gchar query[256];
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *token      = Json_get_string ( module->config, "token" );
    gchar *code_insee = Json_get_string ( module->config, "code_insee" );
    g_snprintf( query, sizeof(query), "https://api.meteo-concept.com/api/forecast/daily?token=%s&insee=%s", token, code_insee );

    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
             "%s: %s: Starting getting data for code_insee '%s'", __func__, thread_tech_id, code_insee );
/********************************************************* Envoi de la requete ************************************************/
    SoupSession *connexion = soup_session_new();
    SoupMessage *soup_msg  = soup_message_new ( "GET", query );
    soup_message_set_request ( soup_msg, "application/json; charset=UTF-8", SOUP_MEMORY_STATIC, NULL, 0 );
    soup_session_send_message (connexion, soup_msg);

    gchar *reason_phrase = Http_Msg_reason_phrase(soup_msg);
    gint   status_code   = Http_Msg_status_code ( soup_msg );

    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: %s: Status %d, reason %s", __func__, thread_tech_id, status_code, reason_phrase );
    if (status_code!=200)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: Error: %s\n", __func__, thread_tech_id, reason_phrase ); }
    else
     { JsonNode *response = Http_Response_Msg_to_Json ( soup_msg );
       Json_node_foreach_array_element ( response, "forecast", Meteo_update_forecast, module );
       Json_node_unref ( response );
     }
    g_object_unref( soup_msg );
    soup_session_abort ( connexion );
  }
/******************************************************************************************************************************/
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct METEO_VARS) );
    struct METEO_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    Mnemo_auto_create_HORLOGE ( FALSE, thread_tech_id, "SUNRISE", "Horloge du levé du soleil" );
    Mnemo_auto_create_HORLOGE ( FALSE, thread_tech_id, "SUNSET",  "Horloge du couché du soleil" );
    for (gint cpt=0; cpt<14; cpt++)
     { gchar acronyme[64];
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_TEMP_MIN", cpt );
       vars->Temp_min[cpt] = Mnemo_create_subprocess_AI ( module, acronyme, "Température minimum", "°C", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_TEMP_MAX", cpt );
       vars->Temp_max[cpt] = Mnemo_create_subprocess_AI ( module, acronyme, "Température maximum", "°C", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_PLUIE", cpt );
       vars->Proba_pluie[cpt] = Mnemo_create_subprocess_AI ( module, acronyme, "Probabilité de pluie (0-100%)", "%", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_GEL", cpt );
       vars->Proba_gel[cpt] = Mnemo_create_subprocess_AI ( module, acronyme, "Probabilité de gel (0-100%)", "%", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_BROUILLARD", cpt );
       vars->Proba_brouillard[cpt] = Mnemo_create_subprocess_AI ( module, acronyme, "Probabilité de brouillard (0-100%)", "%", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_VENT_70", cpt );
       vars->Proba_vent_70[cpt] = Mnemo_create_subprocess_AI ( module, acronyme, "Probabilité de vent > 70km/h  (0-100%)", "%", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_VENT_100", cpt );
       vars->Proba_vent_100[cpt] = Mnemo_create_subprocess_AI ( module, acronyme, "Probabilité de vent > 100km/h (0-100%)", "%", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_RAFALE_VENT_SI_ORAGE", cpt );
       vars->Proba_vent_orage[cpt] = Mnemo_create_subprocess_AI ( module, acronyme, "Vitesse des rafales de vent si orage", "km/h", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_VENT_A_10M", cpt );
       vars->Vent_10m[cpt] = Mnemo_create_subprocess_AI ( module, acronyme, "Vent moyen à 10 mètres", "km/h", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_DIRECTION_VENT", cpt );
       vars->Direction_vent[cpt] = Mnemo_create_subprocess_AI ( module, acronyme, "Direction du vent", "°", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_RAFALE_VENT", cpt );
       vars->Rafale_vent[cpt] = Mnemo_create_subprocess_AI ( module, acronyme,  "Vitesse des rafales de vent", "km/h", ARCHIVE_1_HEURE );
     }

    Meteo_get_ephemeride( module );
    Meteo_get_forecast( module );
    while( module->lib->Thread_run == TRUE && module->lib->Thread_reload == FALSE)           /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       SubProcess_send_comm_to_master_new ( module, module->comm_status );         /* Périodiquement envoie la comm au master */
/****************************************************** Connexion ! ***********************************************************/
       if (Partage->top - vars->last_request >= METEO_POLLING)
        { Meteo_get_ephemeride( module );
          Meteo_get_forecast( module );
          vars->last_request = Partage->top;
        }
/********************************************************* Ecoute du master ***************************************************/
       JsonNode *request;
       while ( (request = SubProcess_Listen_to_master_new ( module ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "test" ) ) vars->last_request = Partage->top - METEO_POLLING;
          Json_node_unref(request);
        }
     }

    for (gint cpt=0; cpt<14; cpt++)
     { Json_node_unref ( vars->Temp_min[cpt] );
       Json_node_unref ( vars->Temp_max[cpt] );
       Json_node_unref ( vars->Proba_pluie[cpt] );
       Json_node_unref ( vars->Proba_gel[cpt] );
       Json_node_unref ( vars->Proba_brouillard[cpt] );
       Json_node_unref ( vars->Proba_vent_70[cpt] );
       Json_node_unref ( vars->Proba_vent_100[cpt] );
       Json_node_unref ( vars->Proba_vent_orage[cpt] );
       Json_node_unref ( vars->Vent_10m[cpt] );
       Json_node_unref ( vars->Direction_vent[cpt] );
       Json_node_unref ( vars->Rafale_vent[cpt] );
     }

    SubProcess_end(module);
  }
/******************************************************************************************************************************/
/* Run_process: Run du Process                                                                                                */
/* Entrée: la structure PROCESS associée                                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_process ( struct PROCESS *lib )
  {
reload:
    Meteo_Creer_DB ( lib );                                                                    /* Création de la DB du thread */
    Thread_init ( "meteo", "EXTAPI", lib, WTD_VERSION, "Manage Meteo system (meteo concept)" );

    lib->config = Json_node_create();
    if(lib->config) SQL_Select_to_json_node ( lib->config, "subprocess", "SELECT * FROM %s WHERE uuid='%s'", lib->name, lib->uuid );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: %d subprocess to load", __func__, Json_get_int ( lib->config, "nbr_subprocess" ) );

    Json_node_foreach_array_element ( lib->config, "subprocess", Process_Load_one_subprocess, lib );   /* Chargement des modules */
    while( lib->Thread_run == TRUE && lib->Thread_reload == FALSE) sleep(1);                 /* On tourne tant que necessaire */
    Process_Unload_all_subprocess ( lib );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }

    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
