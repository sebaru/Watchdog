/******************************************************************************************************************************/
/* Watchdogd/Meteo/Meteo.c        Gestion de l'méteo pour Watchdog v3.0                                        */
/* Projet Abls-Habitat version 4.5       Gestion d'habitat                                                12.03.2021 20:49:16 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Meteo.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Start getting data for code_insee '%s'", code_insee );
/********************************************************* Envoi de la requete ************************************************/
    JsonNode *response = Http_Request ( query, NULL, NULL );
    gint http_code     = Json_get_int ( response, "http_code" );

    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Status %d", http_code );
    if (http_code!=200) Thread_send_comm_to_master ( module, FALSE );
    else
     { gint heure, minute;
       JsonNode *city       = Json_get_object_as_node ( response, "city" );
       JsonNode *ephemeride = Json_get_object_as_node ( response, "ephemeride" );
       gchar *city_name     = Json_get_string ( city, "name" );
       gchar *sunrise       = Json_get_string ( ephemeride, "sunrise" );
       gchar *sunset        = Json_get_string ( ephemeride, "sunset" );
       if ( sscanf ( sunrise, "%d:%d", &heure, &minute ) == 2)
        { Info_new( __func__, module->Thread_debug, LOG_INFO, "%s -> sunrise at %02d:%02d", city_name, heure, minute );
          Mnemo_delete_thread_HORLOGE_tick ( module, vars->sunrise );
          Mnemo_create_thread_HORLOGE_tick ( module, vars->sunrise, heure, minute );
        }
       if ( sscanf ( sunset, "%d:%d", &heure, &minute ) == 2)
        { Info_new( __func__, module->Thread_debug, LOG_INFO, "%s ->  sunset at %02d:%02d", city_name, heure, minute );
          Mnemo_delete_thread_HORLOGE_tick ( module, vars->sunset );
          Mnemo_create_thread_HORLOGE_tick ( module, vars->sunset, heure, minute );
        }
       Thread_send_comm_to_master ( module, TRUE );
     }
    Json_node_unref ( response );
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

    MQTT_Send_AI ( module, vars->Weather[day],          1.0*Json_get_int ( element, "weather" ), TRUE );
    MQTT_Send_AI ( module, vars->Temp_min[day],         1.0*Json_get_int ( element, "tmin" ), TRUE );
    MQTT_Send_AI ( module, vars->Temp_max[day],         1.0*Json_get_int ( element, "tmax" ), TRUE );
    MQTT_Send_AI ( module, vars->Proba_pluie[day],      1.0*Json_get_int ( element, "probarain" ), TRUE );
    MQTT_Send_AI ( module, vars->Proba_gel[day],        1.0*Json_get_int ( element, "probafrost" ), TRUE );
    MQTT_Send_AI ( module, vars->Proba_brouillard[day], 1.0*Json_get_int ( element, "probafog" ), TRUE );
    MQTT_Send_AI ( module, vars->Proba_vent_70[day],    1.0*Json_get_int ( element, "probawind70" ), TRUE );
    MQTT_Send_AI ( module, vars->Proba_vent_100[day],   1.0*Json_get_int ( element, "probawind100" ), TRUE );
    MQTT_Send_AI ( module, vars->Proba_vent_orage[day], 1.0*Json_get_int ( element, "gustx" ), TRUE );
    MQTT_Send_AI ( module, vars->Vent_10m[day],         1.0*Json_get_int ( element, "wind10m" ), TRUE );
    MQTT_Send_AI ( module, vars->Direction_vent[day],   1.0*Json_get_int ( element, "dirwind10m" ), TRUE );
    MQTT_Send_AI ( module, vars->Rafale_vent[day],      1.0*Json_get_int ( element, "gust10m" ), TRUE );
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
    JsonNode *response = Http_Request ( query, NULL, NULL );
    gint http_code     = Json_get_int ( response, "http_code" );
    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Status %d", http_code );

    if (http_code==200) { Json_node_foreach_array_element ( response, "forecast", Meteo_update_forecast, module ); }
    Json_node_unref ( response );
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct METEO_VARS) );
    struct METEO_VARS *vars = module->vars;

    /*gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );*/

    vars->sunrise = Mnemo_create_thread_HORLOGE ( module, "SUNRISE", "Horloge du levé du soleil" );
    vars->sunset  = Mnemo_create_thread_HORLOGE ( module, "SUNSET",  "Horloge du couché du soleil" );

    for (gint cpt=0; cpt<14; cpt++)
     { gchar acronyme[64];
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_WEATHER", cpt );
       vars->Weather[cpt] = Mnemo_create_thread_AI ( module, acronyme, "Météo prévisionnelle", "code", ARCHIVE_1_HEURE );
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
    gint polling_consigne = 0;
    while( module->Thread_run == TRUE )                                                      /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *token_lvl0 = Json_get_string ( request, "token_lvl0" );

          Info_new( __func__, module->Thread_debug, LOG_DEBUG, "token_lvl0 '%s' not for this thread", token_lvl0 );
          Json_node_unref(request);
        }
/****************************************************** Connexion ! ***********************************************************/
       if (Partage->top - vars->last_request >= polling_consigne)
        { Meteo_get_ephemeride( module );
          Meteo_get_forecast( module );
          vars->last_request = Partage->top;
          if (!module->comm_status) polling_consigne = 600; else polling_consigne = METEO_POLLING;       /* Polling adaptatif */
        }
     }

    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
