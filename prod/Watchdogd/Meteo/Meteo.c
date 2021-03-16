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

 struct METEO_CONFIG Cfg_meteo;
/******************************************************************************************************************************/
/* Meteo_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                   */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Meteo_Lire_config ( void )
  { gchar *result;

    Creer_configDB ( NOM_THREAD, "debug", "false" );
    Creer_configDB ( NOM_THREAD, "tech_id", "METEO" );
    Creer_configDB ( NOM_THREAD, "description", "Météo du jour" );
    Creer_configDB ( NOM_THREAD, "code_insee", "Code Insee" );

    result = Recuperer_configDB_by_nom ( NOM_THREAD, "debug" );
    Cfg_meteo.lib->Thread_debug = !g_ascii_strcasecmp(result, "true");
    g_free(result);

    result = Recuperer_configDB_by_nom ( NOM_THREAD, "tech_id" );
    g_snprintf( Cfg_meteo.tech_id, sizeof(Cfg_meteo.tech_id), "%s", result );
    g_free(result);

    result = Recuperer_configDB_by_nom ( NOM_THREAD, "description" );
    g_snprintf( Cfg_meteo.description, sizeof(Cfg_meteo.description), "%s", result );
    g_free(result);

    result = Recuperer_configDB_by_nom ( NOM_THREAD, "token" );
    g_snprintf( Cfg_meteo.token, sizeof(Cfg_meteo.token), "%s", result );
    g_free(result);

    result = Recuperer_configDB_by_nom ( NOM_THREAD, "code_insee" );
    g_snprintf( Cfg_meteo.code_insee, sizeof(Cfg_meteo.code_insee), "%s", result );
    g_free(result);

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Meteo_get_ephemeride: Récupère l'ephemeride auprès de meteoconcept                                                         */
/* Entrée: Niet                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Meteo_get_ephemeride ( void )
  { gchar query[256];
    g_snprintf( query, sizeof(query), "https://api.meteo-concept.com/api/ephemeride/0?token=%s&insee=%s",
                Cfg_meteo.token, Cfg_meteo.code_insee );

/********************************************************* Envoi de la requete ************************************************/
    SoupSession *connexion = soup_session_new();
    SoupMessage *soup_msg = soup_message_new ( "GET", query );
    soup_message_set_request ( soup_msg, "application/json; charset=UTF-8", SOUP_MEMORY_STATIC, NULL, 0 );
    soup_session_send_message (connexion, soup_msg);

    gchar *reason_phrase = Http_Msg_reason_phrase(soup_msg);
    gint   status_code   = Http_Msg_status_code ( soup_msg );

    Info_new( Config.log, Cfg_meteo.lib->Thread_debug, LOG_DEBUG, "%s: Status %d, reason %s", __func__, status_code, reason_phrase );
    if (status_code!=200)
     { Info_new( Config.log, Cfg_meteo.lib->Thread_debug, LOG_ERR, "%s: Error: %s\n", __func__, reason_phrase ); }
    else
     { gint heure, minute;
       JsonNode *response = Http_Response_Msg_to_Json ( soup_msg );
       JsonNode *city = Json_get_object_as_node ( response, "city" );
       JsonNode *ephemeride = Json_get_object_as_node ( response, "ephemeride" );
       gchar *city_name = Json_get_string ( city, "name" );
       gchar *sunrise   = Json_get_string ( ephemeride, "sunrise" );
       gchar *sunset    = Json_get_string ( ephemeride, "sunset" );
       if ( sscanf ( sunrise, "%d:%d", &heure, &minute ) == 2)
        { Horloge_del_all_ticks ( Cfg_meteo.tech_id, "SUNRISE" );
          Horloge_add_tick      ( Cfg_meteo.tech_id, "SUNRISE", heure, minute );
          Info_new( Config.log, Cfg_meteo.lib->Thread_debug, LOG_INFO,
                   "%s: %s -> sunrise at %02d:%02d", __func__, city_name, heure, minute );
        }
       if ( sscanf ( sunset, "%d:%d", &heure, &minute ) == 2)
        { Horloge_del_all_ticks ( Cfg_meteo.tech_id, "SUNSET" );
          Horloge_add_tick      ( Cfg_meteo.tech_id, "SUNSET", heure, minute );
          Info_new( Config.log, Cfg_meteo.lib->Thread_debug, LOG_INFO,
                   "%s: %s ->  sunset at %02d:%02d", __func__, city_name, heure, minute );
        }
       json_node_unref ( response );
       Zmq_Send_WATCHDOG_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, "IO_COMM", METEO_POLLING+100 );
     }
    g_object_unref( soup_msg );
    soup_session_abort ( connexion );
  }
/******************************************************************************************************************************/
/* Meteo_get_forecast: Récupère le forecast auprès de meteoconcept                                                            */
/* Entrée: Nier                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Meteo_update_forecast ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { gint day = Json_get_int ( element, "day" );
    gint temp_min = Json_get_int ( element, "tmin" );
    gint temp_max = Json_get_int ( element, "tmax" );
    Info_new( Config.log, Cfg_meteo.lib->Thread_debug, LOG_DEBUG,
              "%s: day %02d -> temp_min=%02d, temp_max=%02d", __func__, day, temp_min, temp_max );
    gchar acronyme[64];
    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_TEMP_MIN", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "tmin" ), TRUE );

    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_TEMP_MAX", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "tmax" ), TRUE );

    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_PLUIE", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "probarain" ), TRUE );

    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_GEL", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "probafrost" ), TRUE );

    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_BROUILLARD", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "probafog" ), TRUE );

    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_VENT_70", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "probawind70" ), TRUE );

    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_VENT_100", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "probawind100" ), TRUE );

    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_RAFALE_VENT_SI_ORAGE", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "gustx" ), TRUE );

    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_VENT_A_10M", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "wind10m" ), TRUE );

    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_DIRECTION_VENT", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "dirwind10m" ), TRUE );

    g_snprintf( acronyme, sizeof(acronyme), "DAY%d_RAFALE_VENT", day );
    Zmq_Send_AI_to_master ( Cfg_meteo.zmq_to_master, NOM_THREAD, Cfg_meteo.tech_id, acronyme, 1.0*Json_get_int ( element, "gust10m" ), TRUE );
  }
/******************************************************************************************************************************/
/* Meteo_get_forecast: Récupère le forecast auprès de meteoconcept                                                            */
/* Entrée: Nier                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Meteo_get_forecast ( void )
  { gchar query[256];
    g_snprintf( query, sizeof(query), "https://api.meteo-concept.com/api/forecast/daily?token=%s&insee=%s",
                Cfg_meteo.token, Cfg_meteo.code_insee );

/********************************************************* Envoi de la requete ************************************************/
    SoupSession *connexion = soup_session_new();
    SoupMessage *soup_msg = soup_message_new ( "GET", query );
    soup_message_set_request ( soup_msg, "application/json; charset=UTF-8", SOUP_MEMORY_STATIC, NULL, 0 );
    soup_session_send_message (connexion, soup_msg);

    gchar *reason_phrase = Http_Msg_reason_phrase(soup_msg);
    gint   status_code   = Http_Msg_status_code ( soup_msg );

    Info_new( Config.log, Cfg_meteo.lib->Thread_debug, LOG_DEBUG, "%s: Status %d, reason %s", __func__, status_code, reason_phrase );
    if (status_code!=200)
     { Info_new( Config.log, Cfg_meteo.lib->Thread_debug, LOG_ERR, "%s: Error: %s\n", __func__, reason_phrase ); }
    else
     { JsonNode *response = Http_Response_Msg_to_Json ( soup_msg );
       Json_node_foreach_array_element ( response, "forecast", Meteo_update_forecast, NULL );
       json_node_unref ( response );
     }
    g_object_unref( soup_msg );
    soup_session_abort ( connexion );
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un client et un utilisateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct ZMQUEUE *zmq_from_bus;
reload:
    memset( &Cfg_meteo, 0, sizeof(Cfg_meteo) );                                       /* Mise a zero de la structure de travail */
    Cfg_meteo.lib = lib;                                            /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "W-METEO", "EXTAPI", lib, WTD_VERSION, "Manage Meteo system (meteo concept)" );
    Meteo_Lire_config ();                                                    /* Lecture de la configuration logiciel du thread */

    if (Dls_auto_create_plugin( Cfg_meteo.tech_id, "Gestion de la météo" ) == FALSE)
     { Info_new( Config.log, Cfg_meteo.lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, Cfg_meteo.tech_id ); }

    Mnemo_auto_create_WATCHDOG ( FALSE, Cfg_meteo.tech_id, "IO_COMM", "Statut de la communication avec l'api meteo concept" );
    Mnemo_auto_create_HORLOGE  ( FALSE, Cfg_meteo.tech_id, "SUNRISE", "Horloge du levé du soleil" );
    Mnemo_auto_create_HORLOGE  ( FALSE, Cfg_meteo.tech_id, "SUNSET",  "Horloge du couché du soleil" );
    for (gint cpt=0; cpt<=13; cpt++)
     { gchar acronyme[64];
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_TEMP_MIN", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme, "Température minimum", "°C" );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_TEMP_MAX", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme, "Température maximum", "°C" );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_PLUIE", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme, "Probabilité de pluie (0-100%)", "%" );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_GEL", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme, "Probabilité de gel (0-100%)", "%" );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_BROUILLARD", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme, "Probabilité de brouillard (0-100%)", "%" );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_VENT_70", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme, "Probabilité de vent > 70km/h  (0-100%)", "%" );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_VENT_100", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme, "Probabilité de vent > 100km/h (0-100%)", "%" );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_RAFALE_VENT_SI_ORAGE", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme, "Vitesse des rafales de vent si orage", "km/h" );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_VENT_A_10M", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme, "Vent moyen à 10 mètres", "km/h" );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_DIRECTION_VENT", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme, "Direction du vent", "°" );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_RAFALE_VENT", cpt );
       Mnemo_auto_create_AI ( FALSE, Cfg_meteo.tech_id, acronyme,  "Vitesse des rafales de vent", "km/h" );
     }

    zmq_from_bus            = Zmq_Connect ( ZMQ_SUB, "listen-to-bus",  "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    Cfg_meteo.zmq_to_master = Zmq_Connect ( ZMQ_PUB, "pub-to-master",  "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

    Meteo_get_ephemeride();
    Meteo_get_forecast();
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { gchar buffer[1024];
       usleep(10000);
       sched_yield();

/****************************************************** Test connexion ! ******************************************************/
       if (Partage->top - Cfg_meteo.last_request >= METEO_POLLING || Cfg_meteo.test_api)
        { Meteo_get_ephemeride();
          Meteo_get_forecast();
          Cfg_meteo.last_request = Partage->top;
          Cfg_meteo.test_api = FALSE;
        }

/********************************************************* Envoi de SMS *******************************************************/
       JsonNode *request;
       while ( (request=Recv_zmq_with_json( zmq_from_bus, NOM_THREAD, (gchar *)&buffer, sizeof(buffer) )) != NULL)
        { /*gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );*/
          json_node_unref(request);
        }
     }
    Zmq_Close ( zmq_from_bus );
    Zmq_Close ( Cfg_meteo.zmq_to_master );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
