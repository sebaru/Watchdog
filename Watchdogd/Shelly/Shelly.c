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

    gchar *string_id = Json_get_string ( module->config, "string_id" );
    if (!string_id)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "ERROR: No string_id, stopping thread" ); goto end; }
    else if (g_str_has_prefix ( string_id, SHELLY_PRO_EM_50 ))                                          /* Monophasé 2 canaux */
     { vars->EM10_ACT_POWER  = Mnemo_create_thread_AI ( module, "EM10_ACT_POWER",  "EM10 Puissance active", "W",     ARCHIVE_1_MIN );
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
     }
    else if (g_str_has_prefix ( string_id, SHELLY_PRO_3_EM ))                                                     /* Triphasé */
     { vars->U1          = Mnemo_create_thread_AI ( module, "U1",           "Voltage Phase 1", "V", ARCHIVE_1_MIN );
       vars->U2          = Mnemo_create_thread_AI ( module, "U2",           "Voltage Phase 2", "V", ARCHIVE_1_MIN );
       vars->U3          = Mnemo_create_thread_AI ( module, "U3",           "Voltage Phase 3", "V", ARCHIVE_1_MIN );
       vars->I1          = Mnemo_create_thread_AI ( module, "I1",           "Courant Phase 1", "A", ARCHIVE_1_MIN );
       vars->I2          = Mnemo_create_thread_AI ( module, "I2",           "Courant Phase 2", "A", ARCHIVE_1_MIN );
       vars->I3          = Mnemo_create_thread_AI ( module, "I3",           "Courant Phase 3", "A", ARCHIVE_1_MIN );
       vars->I_TOTAL     = Mnemo_create_thread_AI ( module, "I_TOTAL",      "Courant Total", "A", ARCHIVE_1_MIN );
       vars->ACT_TOTAL   = Mnemo_create_thread_AI ( module, "ACT_TOTAL",    "Puissance Active Totale", "W", ARCHIVE_1_MIN );
       vars->ACT_POWER1  = Mnemo_create_thread_AI ( module, "ACT_POWER1",   "Puissance Active Phase 1", "W", ARCHIVE_1_MIN );
       vars->ACT_POWER2  = Mnemo_create_thread_AI ( module, "ACT_POWER2",   "Puissance Active Phase 2", "W", ARCHIVE_1_MIN );
       vars->ACT_POWER3  = Mnemo_create_thread_AI ( module, "ACT_POWER3",   "Puissance Active Phase 3", "W", ARCHIVE_1_MIN );
       vars->APRT_TOTAL  = Mnemo_create_thread_AI ( module, "APRT_TOTAL",   "Puissance Apparente Totale", "VA", ARCHIVE_1_MIN );
       vars->APRT_POWER1 = Mnemo_create_thread_AI ( module, "APRT_POWER1",  "Puissance Apparente Phase 1", "VA", ARCHIVE_1_MIN );
       vars->APRT_POWER2 = Mnemo_create_thread_AI ( module, "APRT_POWER2",  "Puissance Apparente Phase 2", "VA", ARCHIVE_1_MIN );
       vars->APRT_POWER3 = Mnemo_create_thread_AI ( module, "APRT_POWER3",  "Puissance Apparente Phase 3", "VA", ARCHIVE_1_MIN );
       vars->FREQ1       = Mnemo_create_thread_AI ( module, "FREQ1",        "Fréquence Phase 1", "HZ", ARCHIVE_1_MIN );
       vars->FREQ2       = Mnemo_create_thread_AI ( module, "FREQ2",        "Fréquence Phase 2", "HZ", ARCHIVE_1_MIN );
       vars->FREQ3       = Mnemo_create_thread_AI ( module, "FREQ3",        "Fréquence Phase 3", "HZ", ARCHIVE_1_MIN );
       vars->PF1         = Mnemo_create_thread_AI ( module, "PF1",          "Facteur de charge Phase 1", "cos", ARCHIVE_1_MIN );
       vars->PF2         = Mnemo_create_thread_AI ( module, "PF2",          "Facteur de charge Phase 2", "cos", ARCHIVE_1_MIN );
       vars->PF3         = Mnemo_create_thread_AI ( module, "PF3",          "Facteur de charge Phase 3", "cos", ARCHIVE_1_MIN );
       vars->ENERGY1     = Mnemo_create_thread_AI ( module, "ENERGY1",      "Energie consommée Phase 1", "Wh",   ARCHIVE_1_MIN );
       vars->ENERGY2     = Mnemo_create_thread_AI ( module, "ENERGY2",      "Energie consommée Phase 2", "Wh",   ARCHIVE_1_MIN );
       vars->ENERGY3     = Mnemo_create_thread_AI ( module, "ENERGY3",      "Energie consommée Phase 3", "Wh",   ARCHIVE_1_MIN );
       vars->INJECTION1  = Mnemo_create_thread_AI ( module, "INJECTION1",   "Energie injectée Phase 1", "Wh",    ARCHIVE_1_MIN );
       vars->INJECTION2  = Mnemo_create_thread_AI ( module, "INJECTION2",   "Energie injectée Phase 2", "Wh",    ARCHIVE_1_MIN );
       vars->INJECTION3  = Mnemo_create_thread_AI ( module, "INJECTION3",   "Energie injectée Phase 3", "Wh",    ARCHIVE_1_MIN );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Shelly type '%s' not recognized", string_id ); goto end; }

    gchar topic_rpc[256];
    g_snprintf ( topic_rpc, sizeof(topic_rpc), "%s/events/rpc", string_id );
    MQTT_Subscribe ( module->MQTT_session, topic_rpc );

    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          if (Json_has_member ( request, "topic" ) && !strcmp ( Json_get_string ( request, "topic" ), topic_rpc ) &&
              Json_has_member ( request, "method" )
             )
           { gchar *method = Json_get_string ( request, "method" );
             Info_new( __func__, Config.log_bus, LOG_DEBUG, "MQTT: received '%s'", method );
/*---------------------------------------------------- Notify Status ---------------------------------------------------------*/
             if (!strcmp ( method, "NotifyStatus" ) && Json_has_member ( request, "params" ) )
              { JsonNode *params = Json_get_object_as_node ( request, "params" );

                if (g_str_has_prefix ( string_id, SHELLY_PRO_EM_50 ))                                  /* Monophasé, 2 canaux */
                 { if (Json_has_member ( params, "em1:0" ) )
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
                else if (g_str_has_prefix ( string_id, SHELLY_PRO_3_EM ) && Json_has_member ( params, "em:0" ) )
                 { JsonNode *em = Json_get_object_as_node ( params, "em:0" );
                   MQTT_Send_AI ( module, vars->EM11_ACT_POWER, Json_get_double ( em, "act_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->U1            , Json_get_double ( em, "a_voltage" ), TRUE );
                   MQTT_Send_AI ( module, vars->U2            , Json_get_double ( em, "b_voltage" ), TRUE );
                   MQTT_Send_AI ( module, vars->U3            , Json_get_double ( em, "c_voltage" ), TRUE );
                   MQTT_Send_AI ( module, vars->I1            , Json_get_double ( em, "a_current" ), TRUE );
                   MQTT_Send_AI ( module, vars->I2            , Json_get_double ( em, "b_current" ), TRUE );
                   MQTT_Send_AI ( module, vars->I3            , Json_get_double ( em, "c_current" ), TRUE );
                   MQTT_Send_AI ( module, vars->I_TOTAL       , Json_get_double ( em, "total_current" ), TRUE );
                   MQTT_Send_AI ( module, vars->ACT_TOTAL     , Json_get_double ( em, "total_act_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->ACT_POWER1    , Json_get_double ( em, "a_act_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->ACT_POWER2    , Json_get_double ( em, "b_act_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->ACT_POWER3    , Json_get_double ( em, "c_act_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->APRT_TOTAL    , Json_get_double ( em, "total_aprt_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->APRT_POWER1   , Json_get_double ( em, "a_aprt_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->APRT_POWER2   , Json_get_double ( em, "b_aprt_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->APRT_POWER3   , Json_get_double ( em, "c_aprt_power" ), TRUE );
                   MQTT_Send_AI ( module, vars->FREQ1         , Json_get_double ( em, "a_freq" ), TRUE );
                   MQTT_Send_AI ( module, vars->FREQ2         , Json_get_double ( em, "b_freq" ), TRUE );
                   MQTT_Send_AI ( module, vars->FREQ3         , Json_get_double ( em, "c_freq" ), TRUE );
                   MQTT_Send_AI ( module, vars->PF1           , Json_get_double ( em, "a_pf" ), TRUE );
                   MQTT_Send_AI ( module, vars->PF2           , Json_get_double ( em, "b_pf" ), TRUE );
                   MQTT_Send_AI ( module, vars->PF3           , Json_get_double ( em, "c_pf" ), TRUE );
                 }
              }
/*---------------------------------------------------- Notify Event  ---------------------------------------------------------*/
             else if (!strcmp ( method, "NotifyEvent" ))
              { if (g_str_has_prefix ( string_id, SHELLY_PRO_EM_50 ) )
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
                else if (g_str_has_prefix ( string_id, SHELLY_PRO_3_EM ) )
                 { JsonNode *params      = Json_get_object_as_node ( request, "params" );
                   JsonArray *events     = Json_get_array ( params, "events" );
                   JsonNode *first_event = json_array_get_element ( events, 0 );
                   gchar *component      = Json_get_string ( first_event, "component" );
                   JsonNode *data        = Json_get_object_as_node ( first_event, "data" );
                   JsonArray *values     = Json_get_array ( data, "values" );
                              values     = json_array_get_array_element ( values, 0 ); /* Array in array */
                   gdouble energie1      = json_array_get_double_element ( values, 0 );
                   gdouble energie2      = json_array_get_double_element ( values, 16 );
                   gdouble energie3      = json_array_get_double_element ( values, 32 );
                   gdouble injection1    = json_array_get_double_element ( values, 2 );
                   gdouble injection2    = json_array_get_double_element ( values, 18 );
                   gdouble injection3    = json_array_get_double_element ( values, 34 );
                   if ( component && !strcmp ( component, "emdata:0" ) )
                    { MQTT_Send_AI ( module, vars->ENERGY1, energie1, TRUE );
                      MQTT_Send_AI ( module, vars->ENERGY2, energie2, TRUE );
                      MQTT_Send_AI ( module, vars->ENERGY3, energie3, TRUE );
                      MQTT_Send_AI ( module, vars->INJECTION1, injection1, TRUE );
                      MQTT_Send_AI ( module, vars->INJECTION2, injection2, TRUE );
                      MQTT_Send_AI ( module, vars->INJECTION3, injection3, TRUE );
                    }
                 }
              }
             Thread_send_comm_to_master ( module, TRUE );
           }
          Json_node_unref ( request );
        }
     }
end:
    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
