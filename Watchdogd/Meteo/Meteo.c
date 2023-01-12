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
/* Meteo_get_ephemeride: Récupère l'ephemeride auprès de meteoconcept                                                         */
/* Entrée: Niet                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Meteo_get_ephemeride ( struct THREAD *module )
  { struct METEO_VARS *vars = module->vars;
    gchar query[256];
    gchar *token      = Json_get_string ( module->config, "token" );
    gchar *code_insee = Json_get_string ( module->config, "code_insee" );
    g_snprintf( query, sizeof(query), "https://api.meteo-concept.com/api/ephemeride/0?token=%s&insee=%s", token, code_insee );

    Info_new( __func__, module->Thread_debug, LOG_DEBUG,
             "Starting getting data for code_insee '%s'", code_insee );
/********************************************************* Envoi de la requete ************************************************/
    SoupSession *connexion = soup_session_new();
    SoupMessage *soup_msg  = soup_message_new ( "GET", query );
    JsonNode *response = Http_Send_json_request_from_thread ( module, connexion, soup_msg, NULL );
    gchar *reason_phrase = soup_message_get_reason_phrase(soup_msg);
    gint   status_code   = soup_message_get_status ( soup_msg );

    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Status %d, reason %s", status_code, reason_phrase );
    if (status_code!=200) Thread_send_comm_to_master ( module, FALSE );
    else
     { gint heure, minute;
       JsonNode *city       = Json_get_object_as_node ( response, "city" );
       JsonNode *ephemeride = Json_get_object_as_node ( response, "ephemeride" );
       gchar *city_name     = Json_get_string ( city, "name" );
       gchar *sunrise       = Json_get_string ( ephemeride, "sunrise" );
       gchar *sunset        = Json_get_string ( ephemeride, "sunset" );
       if ( sscanf ( sunrise, "%d:%d", &heure, &minute ) == 2)
        { Mnemo_delete_thread_HORLOGE_tick ( module, vars->sunrise );
          Mnemo_create_thread_HORLOGE_tick ( module, vars->sunrise, heure, minute );
          Info_new( __func__, module->Thread_debug, LOG_INFO,
                   "%s -> sunrise at %02d:%02d", city_name, heure, minute );
        }
       if ( sscanf ( sunset, "%d:%d", &heure, &minute ) == 2)
        { Mnemo_delete_thread_HORLOGE_tick ( module, vars->sunset );
          Mnemo_create_thread_HORLOGE_tick ( module, vars->sunset, heure, minute );
          Info_new( __func__, module->Thread_debug, LOG_INFO,
                   "%s ->  sunset at %02d:%02d", city_name, heure, minute );
        }
       Thread_send_comm_to_master ( module, TRUE );
     }
    Json_node_unref ( response );
    g_object_unref( soup_msg );
    soup_session_abort ( connexion );
  }
/******************************************************************************************************************************/
/* Meteo_update_forecast: Met a jour le forecast auprès de meteoconcept                                                       */
/* Entrée: Nier                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Meteo_update_forecast ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct THREAD *module = user_data;
    struct METEO_VARS *vars = module->vars;

    gint day       = Json_get_int ( element, "day" );
    gint temp_min  = Json_get_int ( element, "tmin" );
    gint temp_max  = Json_get_int ( element, "tmax" );
    Info_new( __func__, module->Thread_debug, LOG_DEBUG,
              "day %02d -> temp_min=%02d, temp_max=%02d", day, temp_min, temp_max );

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
 static void Meteo_get_forecast ( struct THREAD *module )
  { gchar query[256];
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *token      = Json_get_string ( module->config, "token" );
    gchar *code_insee = Json_get_string ( module->config, "code_insee" );
    g_snprintf( query, sizeof(query), "https://api.meteo-concept.com/api/forecast/daily?token=%s&insee=%s", token, code_insee );

    Info_new( __func__, module->Thread_debug, LOG_DEBUG,
             "%s: Starting getting data for code_insee '%s'", thread_tech_id, code_insee );
/********************************************************* Envoi de la requete ************************************************/
    SoupSession *connexion = soup_session_new();
    SoupMessage *soup_msg  = soup_message_new ( "GET", query );
    JsonNode *response = Http_Send_json_request_from_thread ( module, connexion, soup_msg, NULL );

    gchar *reason_phrase = soup_message_get_reason_phrase(soup_msg);
    gint   status_code   = soup_message_get_status ( soup_msg );

    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "%s: Status %d, reason %s", thread_tech_id, status_code, reason_phrase );
    if (status_code==200) Json_node_foreach_array_element ( response, "forecast", Meteo_update_forecast, module );
    Json_node_unref ( response );
    g_object_unref( soup_msg );
    soup_session_abort ( connexion );
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct METEO_VARS) );
    struct METEO_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    vars->sunrise = Mnemo_create_thread_HORLOGE ( module, "SUNRISE", "Horloge du levé du soleil" );
    vars->sunset  = Mnemo_create_thread_HORLOGE ( module, "SUNSET",  "Horloge du couché du soleil" );

    for (gint cpt=0; cpt<14; cpt++)
     { gchar acronyme[64];
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_TEMP_MIN", cpt );
       vars->Temp_min[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Température minimum", "°C", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_TEMP_MAX", cpt );
       vars->Temp_max[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Température maximum", "°C", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_PLUIE", cpt );
       vars->Proba_pluie[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Probabilité de pluie (0-100%)", "%", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_GEL", cpt );
       vars->Proba_gel[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Probabilité de gel (0-100%)", "%", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_BROUILLARD", cpt );
       vars->Proba_brouillard[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Probabilité de brouillard (0-100%)", "%", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_VENT_70", cpt );
       vars->Proba_vent_70[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Probabilité de vent > 70km/h  (0-100%)", "%", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_VENT_100", cpt );
       vars->Proba_vent_100[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Probabilité de vent > 100km/h (0-100%)", "%", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_RAFALE_VENT_SI_ORAGE", cpt );
       vars->Proba_vent_orage[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Vitesse des rafales de vent si orage", "km/h", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_VENT_A_10M", cpt );
       vars->Vent_10m[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Vent moyen à 10 mètres", "km/h", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_DIRECTION_VENT", cpt );
       vars->Direction_vent[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Direction du vent", "°", ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_RAFALE_VENT", cpt );
       vars->Rafale_vent[cpt] = Mnemo_create_thread_AI ( module, acronyme,  "Vitesse des rafales de vent", "km/h", ARCHIVE_1_HEURE );
     }

    Meteo_get_ephemeride( module );
    Meteo_get_forecast( module );
    while( module->Thread_run == TRUE )                                                      /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->WS_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->WS_messages->data;
          module->WS_messages = g_slist_remove ( module->WS_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *tag = Json_get_string ( request, "tag" );

          Info_new( __func__, module->Thread_debug, LOG_DEBUG, "%s: tag '%s' not for this thread", thread_tech_id, tag );
          Json_node_unref(request);
        }
/****************************************************** Connexion ! ***********************************************************/
       if (Partage->top - vars->last_request >= METEO_POLLING)
        { Meteo_get_ephemeride( module );
          Meteo_get_forecast( module );
          vars->last_request = Partage->top;
        }
     }

    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
