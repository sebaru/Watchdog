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
/* Envoi_sms_smsbox: Envoi un sms par SMSBOX                                                                                  */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Meteo_get_meteo_concept ( void )
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
       Info_new( Config.log, Cfg_meteo.lib->Thread_debug, LOG_DEBUG,
                 "%s: a %s -> sunrise=%s, sunset=%s", __func__, city_name, sunrise, sunset );
       if ( sscanf ( sunrise, "%d:%d", &heure, &minute ) == 2)
        {
        }
       if ( sscanf ( sunset, "%d:%d", &heure, &minute ) == 2)
        {
        }
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

    if (Dls_auto_create_plugin( Cfg_meteo.tech_id, "Gestion de l'méteo" ) == FALSE)
     { Info_new( Config.log, Cfg_meteo.lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, Cfg_meteo.tech_id ); }

    Mnemo_auto_create_WATCHDOG ( FALSE, Cfg_meteo.tech_id, "IO_COMM", "Statut de la communication avec l'api meteo concept" );
    Mnemo_auto_create_HORLOGE  ( Cfg_meteo.tech_id, "SUNRISE", "Horloge du levé du soleil" );
    Mnemo_auto_create_HORLOGE  ( Cfg_meteo.tech_id, "SUNSET",  "Horloge du couché du soleil" );

    zmq_from_bus                 = Zmq_Connect ( ZMQ_SUB, "listen-to-bus",  "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    Cfg_meteo.zmq_to_master = Zmq_Connect ( ZMQ_PUB, "pub-to-master",  "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { gchar buffer[1024];
       usleep(10000);
       sched_yield();

/****************************************************** Test connexion ! ******************************************************/
       if (Cfg_meteo.test_api)
        { Meteo_get_meteo_concept();
          Cfg_meteo.test_api = FALSE;
        }

/****************************************************** Test connexion ! ******************************************************/
       if (Partage->top - Cfg_meteo.last_request >= 36000)
        { Meteo_get_meteo_concept();
          Cfg_meteo.last_request = Partage->top;
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
