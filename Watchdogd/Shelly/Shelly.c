/******************************************************************************************************************************/
/* Watchdogd/Shelly/Shelly.c  Gestion des modules SHELLY Watchdgo 4.0                                                         */
/* Projet WatchDog version 4.0       Gestion d'habitat                                                    08.03.2024 23:35:42 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Shelly.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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

 #include <sys/prctl.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Shelly.h"

/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct SHELLY_VARS) );
    struct SHELLY_VARS *vars = module->vars;

    vars->EM10_ACT_POWER  = Mnemo_create_thread_AI ( module, "EM10_ACT_POWER",  "EM10 Puissance active", "W",     ARCHIVE_1_MIN );
    vars->EM10_APRT_POWER = Mnemo_create_thread_AI ( module, "EM10_APRT_POWER", "EM10 Puissance apparente", "VA", ARCHIVE_1_MIN );
    vars->EM10_CURRENT    = Mnemo_create_thread_AI ( module, "EM10_CURRENT",    "EM10 Courant", "A",              ARCHIVE_1_MIN );
    vars->EM10_FREQ       = Mnemo_create_thread_AI ( module, "EM10_FREQ",       "EM10 Fréquence", "HZ",           ARCHIVE_1_MIN );
    vars->EM10_PF         = Mnemo_create_thread_AI ( module, "EM10_PF",         "EM10 Facteur de charge", "",     ARCHIVE_1_MIN );
    vars->EM10_VOLTAGE    = Mnemo_create_thread_AI ( module, "EM10_VOLTAGE",    "EM10 Voltage", "V",              ARCHIVE_1_MIN );
    vars->EM10_ENERGY     = Mnemo_create_thread_AI ( module, "EM10_ENERGY",     "EM10 Energie consommée", "Wh",   ARCHIVE_1_MIN );
    vars->EM10_INJECTION  = Mnemo_create_thread_AI ( module, "EM10_INJECTION",  "EM10 Energie injectée", "Wh",    ARCHIVE_1_MIN );

    vars->EM11_ACT_POWER  = Mnemo_create_thread_AI ( module, "EM11_ACT_POWER",  "EM11 Puissance active", "W",     ARCHIVE_1_MIN );
    vars->EM11_APRT_POWER = Mnemo_create_thread_AI ( module, "EM11_APRT_POWER", "EM11 Puissance apparente", "VA", ARCHIVE_1_MIN );
    vars->EM11_CURRENT    = Mnemo_create_thread_AI ( module, "EM11_CURRENT",    "EM11 Courant", "A",              ARCHIVE_1_MIN );
    vars->EM11_FREQ       = Mnemo_create_thread_AI ( module, "EM11_FREQ",       "EM11 Fréquence", "HZ",           ARCHIVE_1_MIN );
    vars->EM11_PF         = Mnemo_create_thread_AI ( module, "EM11_PF",         "EM11 Facteur de charge", "",     ARCHIVE_1_MIN );
    vars->EM11_VOLTAGE    = Mnemo_create_thread_AI ( module, "EM11_VOLTAGE",    "EM11 Voltage", "V",              ARCHIVE_1_MIN );
    vars->EM11_ENERGY     = Mnemo_create_thread_AI ( module, "EM11_ENERGY",     "EM11 Energie consommée", "Wh",   ARCHIVE_1_MIN );
    vars->EM11_INJECTION  = Mnemo_create_thread_AI ( module, "EM11_INJECTION",  "EM11 Energie injectée", "Wh",    ARCHIVE_1_MIN );

    gchar topic_rpc[256];
    g_snprintf ( topic_rpc, sizeof(topic_rpc), "%s/events/rpc", Json_get_string ( module->config, "string_id" ) );
    MQTT_Subscribe ( module->MQTT_session, topic_rpc );

    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          if (Json_has_member ( request, "topic" ) && !strcmp ( Json_get_string ( request, "topic" ), topic_rpc ))
           { gchar *method = Json_get_string ( request, "method" );
             Info_new( __func__, Config.log_bus, LOG_DEBUG, "MQTT: received '%s'", method );
             if (!strcmp ( method, "NotifyStatus" ))
              { JsonNode *params = Json_get_object_as_node ( request, "params" );
                if (Json_has_member ( params, "em1:0" ) )
                 { JsonNode *em = Json_get_object_as_node ( params, "em1:0" );
                   MQTT_Send_AI ( module, vars->EM10_ACT_POWER,  Json_get_double ( em, "act_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->EM10_APRT_POWER, Json_get_double ( em, "aprt_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->EM10_CURRENT,    Json_get_double ( em, "current" ), TRUE );
                   MQTT_Send_AI ( module, vars->EM10_FREQ,       Json_get_double ( em, "freq" ), TRUE );
                   MQTT_Send_AI ( module, vars->EM10_PF,         Json_get_double ( em, "pf" ), TRUE );
                   MQTT_Send_AI ( module, vars->EM10_VOLTAGE,    Json_get_double ( em, "voltage" ), TRUE );
                 }
                else if (Json_has_member ( params, "em1:1" ) )
                 { JsonNode *em = Json_get_object_as_node ( params, "em1:1" );
                   MQTT_Send_AI ( module, vars->EM11_ACT_POWER,  Json_get_double ( em, "act_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->EM11_APRT_POWER, Json_get_double ( em, "aprt_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->EM11_CURRENT,    Json_get_double ( em, "current" ), TRUE );
                   MQTT_Send_AI ( module, vars->EM11_FREQ,       Json_get_double ( em, "freq" ), TRUE );
                   MQTT_Send_AI ( module, vars->EM11_PF,         Json_get_double ( em, "pf" ), TRUE );
                   MQTT_Send_AI ( module, vars->EM11_VOLTAGE,    Json_get_double ( em, "voltage" ), TRUE );
                 }
              }
             else if (!strcmp ( method, "NotifyEvent" ))
              { JsonNode *params      = Json_get_object_as_node ( request, "params" );
                JsonArray *events     = Json_get_array ( params, "events" );
                JsonNode *first_event = json_array_get_element ( events, 0 );
                gchar *component      = Json_get_string ( first_event, "component" );
                JsonNode *data        = Json_get_object_as_node ( first_event, "data" );
                JsonArray *values     = Json_get_array ( data, "values" );
                           values     = json_array_get_array_element ( values, 0 ); /* Array in array */
                gdouble energie       = json_array_get_double_element ( values, 0 );
                gdouble injection     = json_array_get_double_element ( values, 1 );
                if ( component )
                 { if (!strcmp ( component, "em1data:0" ) )
                    { MQTT_Send_AI ( module, vars->EM10_ENERGY, energie, TRUE );
                      MQTT_Send_AI ( module, vars->EM10_INJECTION, injection, TRUE );
                    }
                   else if (!strcmp ( component, "em1data:1" ) )
                    { MQTT_Send_AI ( module, vars->EM11_ENERGY, energie, TRUE );
                      MQTT_Send_AI ( module, vars->EM11_INJECTION, injection, TRUE );
                    }
                 }
              }
             Thread_send_comm_to_master ( module, TRUE );
           }
          Json_node_unref ( request );
        }
     }
    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
