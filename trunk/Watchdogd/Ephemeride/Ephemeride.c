/******************************************************************************************************************************/
/* Watchdogd/Ephemeride/Ephemeride.c        Gestion de l'éphéméride pour Watchdog v3.0                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    12.03.2021 20:49:16 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Ephemeride.c
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
 #include "Ephemeride.h"

 struct EPHEMERIDE_CONFIG Cfg_ephemeride;
/******************************************************************************************************************************/
/* Ephemeride_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                   */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Ephemeride_Lire_config ( void )
  { gchar *result;

    Creer_configDB ( NOM_THREAD, "debug", "false" );
    Creer_configDB ( NOM_THREAD, "tech_id", "EPHEMERIDE" );
    Creer_configDB ( NOM_THREAD, "description", "Ephemeride du jour" );
    Creer_configDB ( NOM_THREAD, "code_insee", "Code Insee" );

    result = Recuperer_configDB_by_nom ( NOM_THREAD, "debug" );
    Cfg_ephemeride.lib->Thread_debug = !g_ascii_strcasecmp(result, "true");
    g_free(result);

    result = Recuperer_configDB_by_nom ( NOM_THREAD, "tech_id" );
    g_snprintf( Cfg_ephemeride.tech_id, sizeof(Cfg_ephemeride.tech_id), "%s", result );
    g_free(result);

    result = Recuperer_configDB_by_nom ( NOM_THREAD, "description" );
    g_snprintf( Cfg_ephemeride.description, sizeof(Cfg_ephemeride.description), "%s", result );
    g_free(result);

    result = Recuperer_configDB_by_nom ( NOM_THREAD, "token" );
    g_snprintf( Cfg_ephemeride.token, sizeof(Cfg_ephemeride.token), "%s", result );
    g_free(result);

    result = Recuperer_configDB_by_nom ( NOM_THREAD, "code_insee" );
    g_snprintf( Cfg_ephemeride.code_insee, sizeof(Cfg_ephemeride.code_insee), "%s", result );
    g_free(result);

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Envoi_sms_smsbox: Envoi un sms par SMSBOX                                                                                  */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Ephemeride_get_meteo_concept ( void )
  { gchar clair[512], hash_string[48], signature[48], query[128];
    unsigned char hash_bin[EVP_MAX_MD_SIZE];
    EVP_MD_CTX *mdctx;
    gsize taille_buf;
    int md_len;

    g_snprintf( query, sizeof(query), "https://api.meteo-concept.com/api/ephemeride/0?token=%s&insee=%s",
                Cfg_ephemeride.token, Cfg_ephemeride.code_insee );

/********************************************************* Envoi de la requete ************************************************/
    SoupSession *connexion = soup_session_new();
    SoupMessage *soup_msg = soup_message_new ( "GET", query );
    soup_message_set_request ( soup_msg, "application/json; charset=UTF-8", SOUP_MEMORY_STATIC, NULL, 0 );
    /*SoupMessageHeaders *headers;
    g_object_get ( G_OBJECT(soup_msg), "request_headers", &headers, NULL );
    soup_message_headers_append ( headers, "X-Ovh-Application", Cfg_ephemeride.ovh_application_key );
    soup_message_headers_append ( headers, "X-Ovh-Consumer",    Cfg_ephemeride.ovh_consumer_key );
    soup_message_headers_append ( headers, "X-Ovh-Signature",   signature );
    soup_message_headers_append ( headers, "X-Ovh-Timestamp",   timestamp );*/
    soup_session_send_message (connexion, soup_msg);

    GBytes *response_brute;
    gchar *reason_phrase;
    gint status_code;

    g_object_get ( soup_msg, "status-code", &status_code, "reason-phrase", &reason_phrase, "response-body-data", &response_brute, NULL );
    Info_new( Config.log, Cfg_ephemeride.lib->Thread_debug, LOG_DEBUG, "%s: Status %d, reason %s", __func__, status_code, reason_phrase );
    if (status_code!=200)
     { gsize taille;
       gchar *error = g_bytes_get_data ( response_brute, &taille );
       Info_new( Config.log, Cfg_ephemeride.lib->Thread_debug, LOG_ERR, "%s: Error: %s\n", __func__, error );
       g_free(error);
     }
    else
     { gsize taille;
       gchar *buf = g_bytes_get_data ( response_brute, &taille );
       JsonNode *response = Json_get_from_string ( buf );
       g_free(buf);
       /*Info_new( Config.log, Cfg_ephemeride.lib->Thread_debug, LOG_ERR, "%s: Error: %s\n", __func__, error );*/
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
    memset( &Cfg_ephemeride, 0, sizeof(Cfg_ephemeride) );                                       /* Mise a zero de la structure de travail */
    Cfg_ephemeride.lib = lib;                                            /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "W-EPHEMERIDE", "EXTAPI", lib, WTD_VERSION, "Manage Ephemeride system (meteo concept)" );
    Ephemeride_Lire_config ();                                                    /* Lecture de la configuration logiciel du thread */

    if (Dls_auto_create_plugin( Cfg_ephemeride.tech_id, "Gestion de l'éphéméride" ) == FALSE)
     { Info_new( Config.log, Cfg_ephemeride.lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, Cfg_ephemeride.tech_id ); }

    Mnemo_auto_create_WATCHDOG ( FALSE, Cfg_ephemeride.tech_id, "IO_COMM", "Statut de la communication avec l'api meteo concept" );

    zmq_from_bus                 = Zmq_Connect ( ZMQ_SUB, "listen-to-bus",  "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    Cfg_ephemeride.zmq_to_master = Zmq_Connect ( ZMQ_PUB, "pub-to-master",  "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { gchar buffer[1024];
       usleep(10000);
       sched_yield();

/****************************************************** Test connexion ! ******************************************************/
       if (Cfg_ephemeride.test_api)
        { Ephemeride_get_meteo_concept();
          Cfg_ephemeride.test_api = FALSE;
        }

/****************************************************** Test connexion ! ******************************************************/
       if (Partage->top - Cfg_ephemeride.last_request >= 36000)
        { Ephemeride_get_meteo_concept();
          Cfg_ephemeride.last_request = Partage->top;
        }

/********************************************************* Envoi de SMS *******************************************************/
       JsonNode *request;
       while ( (request=Recv_zmq_with_json( zmq_from_bus, NOM_THREAD, (gchar *)&buffer, sizeof(buffer) )) != NULL)
        { /*gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );*/
          json_node_unref(request);
        }
     }
    Zmq_Close ( zmq_from_bus );
    Zmq_Close ( Cfg_ephemeride.zmq_to_master );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
