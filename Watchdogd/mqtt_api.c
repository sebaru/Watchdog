/******************************************************************************************************************************/
/* Watchdogd/mqtt_api.c        Fonctions communes de gestion des requetes HTTP                                             */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                30.12.2020 22:03:58 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mqtt_api.c
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

 #include <glib.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* MQTT_on_log_CB: Affiche un log de la librairie MQTT                                                                        */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_on_log_CB( struct mosquitto *mosq, void *obj, int level, const char *message )
  { gint info_level;
    switch(level)
     { default:
       case MOSQ_LOG_INFO:    info_level = LOG_INFO;    break;
       case MOSQ_LOG_NOTICE:  info_level = LOG_NOTICE;  break;
       case MOSQ_LOG_WARNING: info_level = LOG_WARNING; break;
       case MOSQ_LOG_ERR:     info_level = LOG_ERR;     break;
       case MOSQ_LOG_DEBUG:   info_level = LOG_DEBUG;   break;
     }
    Info_new( __func__, Config.log_msrv, info_level, "%s", message );
  }
/******************************************************************************************************************************/
/* MQTT_on_connect_CB: appelé par la librairie quand le broker est connecté                                                   */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_on_connect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Connected with return code %d: %s",
              return_code, mosquitto_connack_string( return_code ) );
    if (return_code == 0) Partage->com_msrv.MQTT_connected = TRUE ;
  }
/******************************************************************************************************************************/
/* MQTT_on_disconnect_CB: appelé par la librairie quand le broker est déconnecté                                              */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_on_disconnect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Disconnected with return code %d: %s",
              return_code, mosquitto_connack_string( return_code ) );
    Partage->com_msrv.MQTT_connected = FALSE;
  }
/******************************************************************************************************************************/
/* MQTT_Subscribe: souscrit à un topic                                                                                        */
/* Entrée: la structure MQTT, le topic                                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Subscribe ( struct mosquitto *mqtt_session, gchar *topic )
  { if ( mosquitto_subscribe( mqtt_session, NULL, topic, 1 ) != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, Config.log_bus, LOG_ERR, "Subscribe to topic '%s' FAILED", topic ); }
    else
     { Info_new( __func__, Config.log_bus, LOG_INFO, "Subscribe to topic '%s' OK", topic ); }
  }
/******************************************************************************************************************************/
/* MQTT_on_API_message_CB: Appelé par mosquitto lorsque l'on recoit un message MQTT de la part de l'API                       */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_on_mqtt_api_message_CB ( struct mosquitto *MQTT_session, void *obj, const struct mosquitto_message *msg )
  { gchar **tokens = g_strsplit ( msg->topic, "/", 3 );
    if (!tokens) return;
    if (!tokens[0]) goto end; /* Normalement le domain_uuid  */
    if (!tokens[1]) goto end; /* Normalement le agent_uuid, ou agents ou master */
    if (!tokens[2]) goto end; /* Normalement le tag/topic  */
    gchar *topic = tokens[2];

    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "MQTT Message received from API: %s", topic );

/*-------------------------------------------------- Message without payload -------------------------------------------------*/
         if ( !strcasecmp( topic, "RESET") )
     { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "RESET: Stopping in progress" );
       Partage->com_msrv.Thread_run = FALSE;
       goto end;
     }
    else if ( !strcasecmp( topic, "UPGRADE") )
     { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "UPGRADE: Upgrading in progress" );
       MSRV_Agent_upgrade_to ( WTD_BRANCHE );
       goto end;
     }
    else if ( Config.instance_is_master && !strcasecmp( topic, "REMAP") )
     { MSRV_Remap();
       MQTT_Send_to_topic ( Partage->com_msrv.MQTT_local_session, "threads", "SYNC_IO", NULL );/* Synchronisation des IO depuis les threads */
     }
    else if ( Config.instance_is_master && !strcasecmp( topic, "RELOAD_HORLOGE_TICK") ) Dls_Load_horloge_ticks();

/*-------------------------------------------------- Message without payload -------------------------------------------------*/
    JsonNode *request = Json_get_from_string ( msg->payload );
    if (!request)
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "MQTT Message from API dropped: not JSON" );
       goto end;
     }

         if ( !strcasecmp( topic, "THREAD_STOP") )    { Thread_Stop_one_thread ( request ); }
    else if ( !strcasecmp( topic, "THREAD_RESTART") ) { Thread_Stop_one_thread ( request );
                                                        Thread_Start_one_thread ( NULL, 0, request, NULL );
                                                      }
    else if ( !strcasecmp( topic, "THREAD_SEND") )    { Thread_Push_API_message ( request ); }
    else if ( !strcasecmp( topic, "THREAD_DEBUG") )   { Thread_Set_debug ( request ); }
    else if ( !strcasecmp( topic, "AGENT_SET") )
     { if ( !( Json_has_member ( request, "log_bus" ) && Json_has_member ( request, "log_level" ) &&
               Json_has_member ( request, "log_dls" ) &&
               Json_has_member ( request, "log_msrv" ) && Json_has_member ( request, "headless" )
             )
          )
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "AGENT_SET: wrong parameters" );
          goto end;
        }
       Config.log_bus    = Json_get_bool ( request, "log_bus" );
       Config.log_msrv   = Json_get_bool ( request, "log_msrv" );
       Config.log_dls    = Json_get_bool ( request, "log_dls" );
       gboolean headless = Json_get_bool ( request, "headless" );
       gint log_level    = Json_get_int  ( request, "log_level" );
       gchar *branche    = Json_get_string ( request, "branche" );
       Info_change_log_level ( log_level );
       Info_new( __func__, TRUE, LOG_NOTICE, "AGENT_SET: log_msrv=%d, log_bus=%d, log_dls=%d, log_level=%d, headless=%d",
                 Config.log_msrv, Config.log_bus, Config.log_dls, log_level, headless );
       if (Config.headless != headless)
        { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "AGENT_SET: headless has changed, rebooting" );
          Partage->com_msrv.Thread_run = FALSE;
        }
       if (strcmp ( WTD_BRANCHE, branche ))
        { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "AGENT_SET: branche has changed, upgrading and rebooting" );
          MSRV_Agent_upgrade_to ( branche );
        }
     }

    if (Config.instance_is_master == FALSE) goto end_request;

    if ( !strcasecmp( topic, "SYN_CLIC") )
     { if ( !Json_has_member ( request, "tech_id" ) )
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "SYN_CLIC: tech_id is missing" ); goto end_request; }
       if ( !Json_has_member ( request, "acronyme" ) )
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "SYN_CLIC: acronyme is missing" ); goto end_request; }
       gchar *tech_id  = Json_get_string ( request, "tech_id" );
       gchar *acronyme = Json_get_string ( request, "acronyme" );
       struct DLS_DI *bit = Dls_data_lookup_DI ( tech_id, acronyme );
       if (!bit) Info_new( __func__, Config.log_msrv, LOG_ERR, "SYN_CLIC: '%s:%s' not found. Dropping.", tech_id, acronyme );
       else Dls_data_set_DI_pulse ( NULL, bit );
     }
    else if ( !strcasecmp( topic, "DLS_ACQUIT") )
     { if ( !Json_has_member ( request, "tech_id" ) )
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "DLS_ACQUIT: tech_id is missing" );
          goto end_request;
        }
       gchar *plugin_tech_id = Json_get_string ( request, "tech_id" );
       Dls_Acquitter_plugin ( plugin_tech_id );
     }
    else if ( !strcasecmp( topic, "DLS_RELOAD") )
     { if ( !Json_has_member ( request, "tech_id" ) )
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "DLS_RELOAD: tech_id is missing" );
          goto end_request;
        }
       gchar *target_tech_id = Json_get_string ( request, "tech_id" );
       struct DLS_PLUGIN *found = Dls_get_plugin_by_tech_id ( target_tech_id );
       if (found) Dls_Save_Data_to_API ( found );     /* Si trouvé, on sauve les valeurs des bits internes avant rechargement */
       struct DLS_PLUGIN *dls = Dls_Importer_un_plugin ( target_tech_id );
       if (dls) Info_new( __func__, Config.log_dls, LOG_NOTICE, "'%s': imported", target_tech_id );
           else Info_new( __func__, Config.log_dls, LOG_ERR, "'%s': error when importing", target_tech_id );
       Dls_Load_horloge_ticks();
     }
    else if ( !strcasecmp( topic, "DLS_RESTART") )
     { if ( !Json_has_member ( request, "tech_id" ) )
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "DLS_RESTART: tech_id is missing" );
          goto end_request;
        }
       gchar *target_tech_id = Json_get_string ( request, "tech_id" );
       struct DLS_PLUGIN *plugin = Dls_get_plugin_by_tech_id ( target_tech_id );
       if (plugin)
        { pthread_mutex_lock( &Partage->com_dls.synchro );                    /* On stoppe DLS pour éviter l'usage concurrent */
          Dls_Reseter_all_bit_interne ( plugin );
          plugin->vars.resetted = TRUE;                                            /* au chargement, le bit de start vaut 1 ! */
          pthread_mutex_unlock( &Partage->com_dls.synchro );
          Info_new( __func__, Config.log_dls, LOG_NOTICE, "'%s': resetted", target_tech_id );
        }
       else Info_new( __func__, Config.log_dls, LOG_ERR, "'%s': error when resetting: plugin not found.", target_tech_id );
     }
    else if ( !strcasecmp( topic, "DLS_SET") )
     { if ( ! Json_has_member ( request, "tech_id" )  )
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "DLS_SET: wrong parameters" );
          goto end_request;
        }
       gchar *plugin_tech_id = Json_get_string ( request, "tech_id" );
       if (Json_has_member ( request, "debug"  )) Dls_Debug_plugin   ( plugin_tech_id, Json_get_bool ( request, "debug" ) );
       if (Json_has_member ( request, "enable" )) Dls_Activer_plugin ( plugin_tech_id, Json_get_bool ( request, "enable" ) );
     }

end_request:
    Json_node_unref ( request );
end:
    g_strfreev( tokens );                                                                      /* Libération des tokens topic */
  }
/******************************************************************************************************************************/
/* MQTT_Send_to_API: Envoie le node au broker API                                                                             */
/* Entrée: le topic, le node                                                                                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_to_API ( JsonNode *node, gchar *topic, ... )
  { gchar topic_inter[256], topic_full[256];
    va_list ap;
    if (!topic) return;

    g_snprintf( topic_inter, sizeof(topic_inter), "%s/%s", Json_get_string ( Config.config, "domain_uuid" ), topic );
    va_start( ap, topic );
    g_vsnprintf ( topic_full, sizeof(topic_full), topic_inter, ap );
    va_end ( ap );

    gboolean free_node=FALSE;
    if (!node) { node = Json_node_create(); free_node = TRUE; }
    gchar *buffer = Json_node_to_string ( node );
    mosquitto_publish( Partage->com_msrv.MQTT_API_session, NULL, topic_full, strlen(buffer), buffer, 2, TRUE );
    g_free(buffer);
    if (free_node) Json_node_unref(node);
  }
/******************************************************************************************************************************/
/* MQTT_Start_MQTT_API: Appelé pour démarrer les interactions MQTT du master avec l'API                                       */
/* Entrée: Néant                                                                                                              */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean MQTT_Start_MQTT_API ( void )
  { gint retour;
    gchar *agent_uuid    = Json_get_string ( Config.config, "agent_uuid" );
    gchar *domain_uuid   = Json_get_string ( Config.config, "domain_uuid" );

    Partage->com_msrv.MQTT_API_session = mosquitto_new( agent_uuid, FALSE, NULL );
    if (!Partage->com_msrv.MQTT_API_session)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "MQTT_API session error." ); return(FALSE); }

    mosquitto_log_callback_set        ( Partage->com_msrv.MQTT_API_session, MQTT_on_log_CB );
    mosquitto_connect_callback_set    ( Partage->com_msrv.MQTT_API_session, MQTT_on_connect_CB );
    mosquitto_disconnect_callback_set ( Partage->com_msrv.MQTT_API_session, MQTT_on_disconnect_CB );
    mosquitto_message_callback_set    ( Partage->com_msrv.MQTT_API_session, MQTT_on_mqtt_api_message_CB );
    mosquitto_reconnect_delay_set     ( Partage->com_msrv.MQTT_API_session, 10, 60, TRUE );

    if (Config.mqtt_over_ssl)
     { mosquitto_tls_set( Partage->com_msrv.MQTT_API_session, NULL, "/etc/ssl/certs", NULL, NULL, NULL ); }

    gchar mqtt_username[128];
    g_snprintf( mqtt_username, sizeof(mqtt_username), "%s-agent", domain_uuid );
    mosquitto_username_pw_set( Partage->com_msrv.MQTT_API_session, mqtt_username, Config.mqtt_password );
    retour = mosquitto_connect( Partage->com_msrv.MQTT_API_session, Config.mqtt_hostname, Config.mqtt_port, 60 );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "MQTT_API connection to '%s' error: %s",
                 Config.mqtt_hostname, mosquitto_strerror ( retour ) );
       return(FALSE);
     }

    gchar topic[256];
    g_snprintf ( topic, sizeof(topic), "%s/%s/#", domain_uuid, agent_uuid );
    MQTT_Subscribe ( Partage->com_msrv.MQTT_API_session, topic );
    g_snprintf ( topic, sizeof(topic), "%s/agents/#", domain_uuid );
    MQTT_Subscribe ( Partage->com_msrv.MQTT_API_session, topic );

    if (Config.instance_is_master)                                                                          /* Démarrage MQTT */
     { g_snprintf ( topic, sizeof(topic), "%s/master/#", domain_uuid );
       MQTT_Subscribe ( Partage->com_msrv.MQTT_API_session, topic );
     }

    retour = mosquitto_loop_start( Partage->com_msrv.MQTT_API_session );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "MQTT loop not started: ", mosquitto_strerror ( retour ) );
       return(FALSE);
     }
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Connected as %s with id %s on %s.",
              mqtt_username, agent_uuid, Config.mqtt_hostname );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* MQTT_Stop_MQTT_API: Appelé pour stopper les interactions MQTT du master avec l'API                                         */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Stop_MQTT_API ( void )
  { mosquitto_disconnect( Partage->com_msrv.MQTT_API_session );
    mosquitto_loop_stop( Partage->com_msrv.MQTT_API_session, FALSE );
    mosquitto_destroy( Partage->com_msrv.MQTT_API_session );
    Partage->com_msrv.MQTT_API_session = NULL;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
