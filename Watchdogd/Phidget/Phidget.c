/******************************************************************************************************************************/
/* Watchdogd/Phidget/Phidget.c  Gestion des modules PHIDGET Watchdgo 3.0                                                      */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                                18.03.2021 22:02:42 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Phidget.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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

 #include <stdio.h>
 #include <fcntl.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Phidget.h"

/******************************************************************************************************************************/
/* Charger_un_Hub: Charge un Hub dans la librairie                                                                            */
/* Entrée: La structure Json representant le hub                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Phidget_print_error ( struct PHIDGET_ELEMENT *canal )
  { PhidgetReturnCode errorCode;
    /*gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");*/
    gchar *capteur = Json_get_string(canal->element, "capteur");
    gchar *classe  = Json_get_string(canal->element, "classe");
    size_t errorDetailLen = 256;
    const gchar* errorString;
    gchar errorDetail[errorDetailLen];
    Phidget_getLastError(&errorCode, &errorString, errorDetail, &errorDetailLen);
    Info_new( __func__, canal->module->Thread_debug, LOG_ERR, "Phidget Error %d for '%s' (%s) : %s - %s",
              errorCode, capteur, classe, errorString, errorDetail );
  }
/******************************************************************************************************************************/
/* Phidget_onAIError: Appelé quand une erreur est constatée sur le module Phidget                                             */
/* Entrée: le channel, le contexte, et la description de l'erreur                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onError (PhidgetHandle ph, void *ctx, Phidget_ErrorEventCode code, const char *description)
  { struct PHIDGET_ELEMENT *canal = ctx;

    gchar *thread_tech_id  = Json_get_string(canal->module->config, "thread_tech_id");
    gchar *thread_acronyme = Json_get_string(canal->element, "thread_acronyme");
    gchar *classe          = Json_get_string(canal->element, "classe");

    if ( !strcmp ( classe, "AI" ) )
     { Info_new( __func__, canal->module->Thread_debug, LOG_ERR,
		         "Error for '%s:%s' : '%s' (code %X). Inrange = FALSE;",
           thread_tech_id, thread_acronyme, description, code );
       MQTT_Send_AI ( canal->module, canal->element, 0.0, FALSE );
     }
    else
     { Info_new( __func__, canal->module->Thread_debug, LOG_ERR,
		               "Error for '%s:%s' : '%s' (code %X).", thread_tech_id,
                 thread_tech_id, thread_acronyme, description, code );
     }
  }
/******************************************************************************************************************************/
/* Phidget_onPHSensorChange: Appelé quand un module I/O PHSensor a changé de valeur                                           */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onPHSensorChange ( PhidgetPHSensorHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id  = Json_get_string(canal->module->config, "thread_tech_id");
    gchar *thread_acronyme = Json_get_string(canal->element, "thread_acronyme");
    Info_new( __func__, canal->module->Thread_debug, LOG_INFO, "'%s:%s' = %f", thread_tech_id, thread_acronyme, valeur );
    MQTT_Send_AI ( canal->module, canal->element, valeur, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onTemperatureSensorChange: Appelé quand un module I/O Temperaute a changé de valeur                                */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onTemperatureSensorChange ( PhidgetTemperatureSensorHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id  = Json_get_string(canal->module->config, "thread_tech_id");
    gchar *thread_acronyme = Json_get_string(canal->element, "thread_acronyme");
    Info_new( __func__, canal->module->Thread_debug, LOG_INFO, "'%s:%s' = %f", thread_tech_id, thread_acronyme, valeur );
    MQTT_Send_AI ( canal->module, canal->element, valeur, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onVoltableInputChange: Appelé quand un module I/O VoltageInput a changé de valeur                                  */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltageInputChange ( PhidgetVoltageInputHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id  = Json_get_string(canal->module->config, "thread_tech_id");
    gchar *thread_acronyme = Json_get_string(canal->element, "thread_acronyme");
    Info_new( __func__, canal->module->Thread_debug, LOG_INFO, "'%s:%s' = %f", thread_tech_id, thread_acronyme, valeur );
    MQTT_Send_AI ( canal->module, canal->element, valeur, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onVoltageInputChange: Appelé quand un module I/O VoltageInput a changé de valeur                                   */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltageSensorChange ( PhidgetVoltageInputHandle handle, void *ctx, double valeur,
                                                   Phidget_UnitInfo *sensorUnit )
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id  = Json_get_string(canal->module->config, "thread_tech_id");
    gchar *thread_acronyme = Json_get_string(canal->element, "thread_acronyme");
    Info_new( __func__, canal->module->Thread_debug, LOG_INFO, "'%s:%s' = %f", thread_tech_id, thread_acronyme, valeur );
    MQTT_Send_AI ( canal->module, canal->element, valeur, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onVoltageRatoiInputChange: Appelé quand un module I/O RatioInput a changé de valeur                                */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltageRatioSensorChange ( PhidgetVoltageRatioInputHandle ch, void *ctx, double valeur,
                                                        Phidget_UnitInfo *sensorUnit)
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id  = Json_get_string(canal->module->config, "thread_tech_id");
    gchar *thread_acronyme = Json_get_string(canal->element, "thread_acronyme");
    Info_new( __func__, canal->module->Thread_debug, LOG_INFO, "'%s:%s' = %f", thread_tech_id, thread_acronyme, valeur );
    MQTT_Send_AI ( canal->module, canal->element, valeur, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onVoltableInputChange: Appelé quand un module I/O VoltageInput a changé de valeur                                  */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onDigitalInputChange ( PhidgetDigitalInputHandle handle, void *ctx, int valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id  = Json_get_string(canal->module->config, "thread_tech_id");
    gchar *thread_acronyme = Json_get_string(canal->element, "thread_acronyme");
    Info_new( __func__, canal->module->Thread_debug, LOG_INFO, "'%s:%s' = %d", thread_tech_id, thread_acronyme, valeur );
    MQTT_Send_DI ( canal->module, canal->element, (valeur ? TRUE : FALSE) );
  }
/******************************************************************************************************************************/
/* Phidget_AnalogAttach: Appelé quand un canal analogique est en cours d'attachement                                          */
/* Entrée: le canal                                                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Phidget_AnalogAttach ( struct PHIDGET_ELEMENT *canal )
  { gint intervalle = Json_get_int(canal->element, "intervalle");
    gchar *capteur  = Json_get_string(canal->element, "capteur");
    if (intervalle)
     { if (Phidget_setDataInterval( canal->handle, intervalle ) != EPHIDGET_OK) Phidget_print_error(canal); }

    if (!strcasecmp(capteur, "ADP1000-PH"))
     { /**/
     }
    else if (!strcasecmp(capteur, "ADP1000-ORP"))
     { if ( PhidgetVoltageInput_setVoltageRange( (PhidgetVoltageInputHandle)canal->handle, VOLTAGE_RANGE_2V ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(capteur, "TMP1200_0-PT100-3850"))
     { if ( PhidgetTemperatureSensor_setRTDType( (PhidgetTemperatureSensorHandle)canal->handle, RTD_TYPE_PT100_3850 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
       if ( PhidgetTemperatureSensor_setRTDWireSetup( (PhidgetTemperatureSensorHandle)canal->handle, RTD_WIRE_SETUP_2WIRE ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(capteur, "TMP1200_0-PT100-3920"))
     { if ( PhidgetTemperatureSensor_setRTDType( (PhidgetTemperatureSensorHandle)canal->handle, RTD_TYPE_PT100_3920 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
       if ( PhidgetTemperatureSensor_setRTDWireSetup( (PhidgetTemperatureSensorHandle)canal->handle, RTD_WIRE_SETUP_2WIRE ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(capteur, "AC-CURRENT-10A"))
     { if ( PhidgetVoltageInput_setSensorType ( (PhidgetVoltageInputHandle)canal->handle, SENSOR_TYPE_3500 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(capteur, "AC-CURRENT-25A"))
     { if ( PhidgetVoltageInput_setSensorType ( (PhidgetVoltageInputHandle)canal->handle, SENSOR_TYPE_3501 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(capteur, "AC-CURRENT-50A"))
     { if ( PhidgetVoltageInput_setSensorType ( (PhidgetVoltageInputHandle)canal->handle, SENSOR_TYPE_3502 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(capteur, "AC-CURRENT-100A"))
     { if ( PhidgetVoltageInput_setSensorType ( (PhidgetVoltageInputHandle)canal->handle, SENSOR_TYPE_3503 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(capteur, "TEMP_1124_0"))
     { if ( PhidgetVoltageRatioInput_setSensorType ( (PhidgetVoltageRatioInputHandle)canal->handle, SENSOR_TYPE_1124 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
  }
/***************************************************************************************************************************/
/* Phidget_onAttachHandler: Appelé quand un canal estmodule I/O VoltageRatio a changé de valeur                               */
/* Entrée: le channel, le contexte                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onAttachHandler ( PhidgetHandle handle, void *ctx )
  { struct PHIDGET_ELEMENT *canal = ctx;
    int serial_number, nbr_canaux, port, num_canal;

    gchar *thread_tech_id  = Json_get_string(canal->module->config, "thread_tech_id");
    gchar *thread_acronyme = Json_get_string(canal->element, "thread_acronyme");
    gchar *classe          = Json_get_string(canal->element, "classe");
    gchar *capteur         = Json_get_string(canal->element, "capteur");
    Phidget_getDeviceSerialNumber(handle, &serial_number);
    Phidget_getDeviceChannelCount(handle, PHIDCHCLASS_NOTHING, &nbr_canaux );
    Phidget_getHubPort( handle, &port );
    Phidget_getChannel( handle, &num_canal );

    if ( !strcmp ( classe, "AI" ) ) { Phidget_AnalogAttach ( canal ); }

    Info_new( __func__, canal->module->Thread_debug, LOG_NOTICE,
              "'%s:%s' Phidget S/N '%d' Port '%d' capteur '%s' (canal '%d') attached. %d channels available.",
              thread_tech_id, thread_acronyme, serial_number, port, capteur, num_canal, nbr_canaux );

    canal->attached = TRUE;
  }
/******************************************************************************************************************************/
/* Phidget_onAttachHandler: Appelé quand un canal est détaché                                                                 */
/* Entrée: le channel, le contexte                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onDetachHandler ( PhidgetHandle handle, void *ctx )
  { struct PHIDGET_ELEMENT *canal = ctx;
    int serial_number, nbr_canaux, port, num_canal;

    gchar *thread_tech_id  = Json_get_string(canal->module->config, "thread_tech_id");
    gchar *thread_acronyme = Json_get_string(canal->element, "thread_acronyme");
    gchar *classe          = Json_get_string(canal->element, "classe");
    Phidget_getDeviceSerialNumber(handle, &serial_number);
    Phidget_getDeviceChannelCount(handle, PHIDCHCLASS_NOTHING, &nbr_canaux );
    Phidget_getHubPort( handle, &port );
    Phidget_getChannel( handle, &num_canal );

    Info_new( __func__, canal->module->Thread_debug, LOG_NOTICE,
              "'%s:%s' Phidget S/N '%d' Port '%d' classe '%s' (canal '%d') detached . %d channels available.",
              thread_tech_id, thread_acronyme, serial_number, port, classe, num_canal, nbr_canaux );
    canal->attached = FALSE;
  }
/******************************************************************************************************************************/
/* Charger_un_IO: Charge une IO dans la librairie                                                                             */
/* Entrée: La structure Json representant l'i/o                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Phidget_set_config ( struct PHIDGET_ELEMENT *canal, gint serial, gint port, gboolean is_hub_port )
  { if (Phidget_setDeviceSerialNumber( (PhidgetHandle)canal->handle, serial ) != EPHIDGET_OK)
     { Phidget_print_error(canal);
       return;
     }

    if (Phidget_setIsHubPortDevice( (PhidgetHandle)canal->handle, is_hub_port ) != EPHIDGET_OK)
     { Phidget_print_error(canal);
       return;
     }

    if (Phidget_setHubPort( (PhidgetHandle)canal->handle, port ) != EPHIDGET_OK)
     { Phidget_print_error(canal);
       return;
     }

    if (Phidget_setIsRemote( (PhidgetHandle)canal->handle, 1 ) != EPHIDGET_OK)
     { Phidget_print_error(canal);
       return;
     }
  }
/******************************************************************************************************************************/
/* Charger_un_AI: Charge une IO dans la librairie                                                                             */
/* Entrée: La structure Json representant l'i/o                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_IO (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct THREAD *module = user_data;
    struct PHIDGET_VARS *vars = module->vars;
    gint serial    = Json_get_int   ( module->config, "serial" );
    gchar *capteur = Json_get_string( element, "capteur" );
    gint port      = Json_get_int   ( element, "port" );

    Info_new( __func__, module->Thread_debug, LOG_INFO, "Loading S/N %d, port '%d', capteur '%s'", serial, port, capteur );

    struct PHIDGET_ELEMENT *canal = g_try_malloc0 ( sizeof(struct PHIDGET_ELEMENT) );
    if (!canal)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Memory Error on S/N %d, port '%d' capteur '%s'", serial, port, capteur );
       return;
     }

    canal->module  = module;                                                                     /* Sauvegarde du module père */
    canal->element = element;

    if (!strcasecmp(capteur, "ADP1000-PH"))
     { if ( PhidgetPHSensor_create( (PhidgetPHSensorHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetPHSensor_setOnPHChangeHandler( (PhidgetPHSensorHandle)canal->handle, Phidget_onPHSensorChange, canal ) ) goto error;
       Phidget_set_config ( canal, serial, port, FALSE );
     }
    else if (!strcasecmp(capteur, "ADP1000-ORP"))
     { if ( PhidgetVoltageInput_create( (PhidgetVoltageInputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetVoltageInput_setOnVoltageChangeHandler( (PhidgetVoltageInputHandle)canal->handle,
                                                            Phidget_onVoltageInputChange, canal ) != EPHIDGET_OK ) goto error;
       Phidget_set_config ( canal, serial, port, FALSE );
     }
    else if (!strcasecmp(capteur, "TMP1200_0-PT100-3850"))
     { if ( PhidgetTemperatureSensor_create( (PhidgetTemperatureSensorHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetTemperatureSensor_setOnTemperatureChangeHandler( (PhidgetTemperatureSensorHandle)canal->handle,
                                                                     Phidget_onTemperatureSensorChange, canal ) != EPHIDGET_OK ) goto error;
       Phidget_set_config ( canal, serial, port, FALSE );
     }
    else if (!strcasecmp(capteur, "TMP1200_0-PT100-3920"))
     { if ( PhidgetTemperatureSensor_create( (PhidgetTemperatureSensorHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetTemperatureSensor_setOnTemperatureChangeHandler( (PhidgetTemperatureSensorHandle)canal->handle,
                                                                     Phidget_onTemperatureSensorChange, canal ) != EPHIDGET_OK ) goto error;
       Phidget_set_config ( canal, serial, port, FALSE );
     }
    else if (!strcasecmp(capteur, "AC-CURRENT-10A"))
     { if ( PhidgetVoltageInput_create( (PhidgetVoltageInputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetVoltageInput_setOnSensorChangeHandler( (PhidgetVoltageInputHandle)canal->handle,
                                                           Phidget_onVoltageSensorChange, canal ) != EPHIDGET_OK ) goto error;
       Phidget_set_config ( canal, serial, port, TRUE );
     }
    else if (!strcasecmp(capteur, "AC-CURRENT-25A"))
     { if ( PhidgetVoltageInput_create( (PhidgetVoltageInputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetVoltageInput_setOnSensorChangeHandler( (PhidgetVoltageInputHandle)canal->handle,
                                                           Phidget_onVoltageSensorChange, canal ) != EPHIDGET_OK ) goto error;
       Phidget_set_config ( canal, serial, port, TRUE );
     }
    else if (!strcasecmp(capteur, "AC-CURRENT-50A"))
     { if ( PhidgetVoltageInput_create( (PhidgetVoltageInputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetVoltageInput_setOnSensorChangeHandler( (PhidgetVoltageInputHandle)canal->handle,
                                                           Phidget_onVoltageSensorChange, canal ) != EPHIDGET_OK ) goto error;
       if ( PhidgetVoltageInput_setVoltageRange( (PhidgetVoltageInputHandle)canal->handle, VOLTAGE_RANGE_5V) != EPHIDGET_OK ) goto error;
       Phidget_set_config ( canal, serial, port, TRUE );
     }
    else if (!strcasecmp(capteur, "AC-CURRENT-100A"))
     { if ( PhidgetVoltageInput_create( (PhidgetVoltageInputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetVoltageInput_setOnSensorChangeHandler( (PhidgetVoltageInputHandle)canal->handle,
                                                           Phidget_onVoltageSensorChange, canal ) != EPHIDGET_OK ) goto error;
       Phidget_set_config ( canal, serial, port, TRUE );
     }
    else if (!strcasecmp(capteur, "TEMP_1124_0"))
     { if ( PhidgetVoltageRatioInput_create( (PhidgetVoltageRatioInputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetVoltageRatioInput_setOnSensorChangeHandler( (PhidgetVoltageRatioInputHandle)canal->handle,
                                                                Phidget_onVoltageRatioSensorChange, canal ) != EPHIDGET_OK ) goto error;
       Phidget_set_config ( canal, serial, port, TRUE );
     }
    else if (!strcasecmp(capteur, "DIGITAL-INPUT"))
     { if ( PhidgetDigitalInput_create( (PhidgetDigitalInputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetDigitalInput_setOnStateChangeHandler( (PhidgetDigitalInputHandle)canal->handle, Phidget_onDigitalInputChange, canal ) ) goto error;
       Phidget_set_config ( canal, serial, port, TRUE );
     }
    else if (!strcasecmp(capteur, "REL2001_0"))
     { if ( PhidgetDigitalOutput_create( (PhidgetDigitalOutputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       Phidget_set_config ( canal, serial, port, TRUE );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_INFO,
                 "capteur phidget inconnue on S/N %d, port '%d' capteur '%s'", serial, port, capteur );
       goto error;
     }

    if ( Phidget_setOnErrorHandler( canal->handle, Phidget_onError, canal ) ) goto error;
    Phidget_setOnAttachHandler((PhidgetHandle)canal->handle, Phidget_onAttachHandler, canal);
    Phidget_setOnDetachHandler((PhidgetHandle)canal->handle, Phidget_onDetachHandler, canal);
    if (Phidget_open ((PhidgetHandle)canal->handle) != EPHIDGET_OK) goto error;
    vars->Liste_sensors = g_slist_prepend ( vars->Liste_sensors, canal );
    return;
error:
    Phidget_print_error(canal);
    g_free(canal);
  }
/******************************************************************************************************************************/
/* Decharger_un_IO: Décharge une IO dans la librairie                                                                         */
/* Entrée: Le canal representant l'i/o                                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Decharger_un_IO ( struct PHIDGET_ELEMENT *canal )
  { Phidget_close ( (PhidgetHandle)canal->handle );
    gchar *capteur = Json_get_string( canal->element, "capteur" );

         if (!strcasecmp(capteur, "ADP1000-PH"))           PhidgetPHSensor_delete         ( (PhidgetPHSensorHandle *)&canal->handle );
    else if (!strcasecmp(capteur, "ADP1000-ORP"))          PhidgetVoltageInput_delete     ( (PhidgetVoltageInputHandle *)&canal->handle );
    else if (!strcasecmp(capteur, "TMP1200_0-PT100-3850")) PhidgetTemperatureSensor_delete( (PhidgetTemperatureSensorHandle *)&canal->handle );
    else if (!strcasecmp(capteur, "TMP1200_0-PT100-3920")) PhidgetTemperatureSensor_create( (PhidgetTemperatureSensorHandle *)&canal->handle );
    else if (!strcasecmp(capteur, "AC-CURRENT-10A"))       PhidgetVoltageInput_create     ( (PhidgetVoltageInputHandle *)&canal->handle );
    else if (!strcasecmp(capteur, "AC-CURRENT-25A"))       PhidgetVoltageInput_create     ( (PhidgetVoltageInputHandle *)&canal->handle );
    else if (!strcasecmp(capteur, "AC-CURRENT-50A"))       PhidgetVoltageInput_create     ( (PhidgetVoltageInputHandle *)&canal->handle );
    else if (!strcasecmp(capteur, "AC-CURRENT-100A"))      PhidgetVoltageInput_create     ( (PhidgetVoltageInputHandle *)&canal->handle );
    else if (!strcasecmp(capteur, "TEMP_1124_0"))          PhidgetVoltageRatioInput_create( (PhidgetVoltageRatioInputHandle *)&canal->handle );
    else if (!strcasecmp(capteur, "DIGITAL-INPUT"))        PhidgetDigitalInput_create     ( (PhidgetDigitalInputHandle *)&canal->handle );
    else if (!strcasecmp(capteur, "REL2001_0"))            PhidgetDigitalOutput_create    ( (PhidgetDigitalOutputHandle *)&canal->handle );
  }
/******************************************************************************************************************************/
/* Phidget_SET_DO: Met a jour une sortie TOR en fonction du jsonnode en parametre                                             */
/* Entrée: le module et le buffer Josn                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Phidget_SET_DO ( struct THREAD *module, JsonNode *msg )
  { struct PHIDGET_VARS *vars = module->vars;
    gchar *thread_tech_id      = Json_get_string ( module->config, "thread_tech_id" );
    gchar *msg_thread_tech_id  = Json_get_string ( msg, "thread_tech_id" );
    gchar *msg_thread_acronyme = Json_get_string ( msg, "thread_acronyme" );
    gchar *msg_tech_id         = Json_get_string ( msg, "tech_id" );
    gchar *msg_acronyme        = Json_get_string ( msg, "acronyme" );

    if (!msg_thread_tech_id)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Requete mal formée manque msg_thread_tech_id" );
       return;
     }

    if (!msg_thread_acronyme)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Requete mal formée manque msg_thread_acronyme" );
       return;
     }

    if (strcasecmp (msg_thread_tech_id, thread_tech_id))
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Pas pour nous" );
       return;
     }

    if (!Json_has_member ( msg, "etat" ))
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Requete mal formée manque etat" );
       return;
     }

    gboolean etat = Json_get_bool ( msg, "etat" );
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "SET_DO '%s:%s'/'%s:%s'=%d",
              msg_thread_tech_id, msg_thread_acronyme, msg_tech_id, msg_acronyme, etat );

    GSList *liste = vars->Liste_sensors;
    while (liste)
     { struct PHIDGET_ELEMENT *canal = liste->data;
       gchar *thread_acronyme = Json_get_string(canal->element, "thread_acronyme");
       gchar *classe          = Json_get_string(canal->element, "classe");

       if ( !strcasecmp ( classe, "DO" ) &&
            !strcasecmp ( thread_acronyme, msg_thread_acronyme ) )
        { if ( PhidgetDigitalOutput_setState( (PhidgetDigitalOutputHandle)canal->handle, etat ) != EPHIDGET_OK )
           { Phidget_print_error ( canal ); }
          break;
        }
       liste = g_slist_next(liste);
     }
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct PHIDGET_VARS) );
    struct PHIDGET_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname    = Json_get_string ( module->config, "hostname" );
    gchar *description = Json_get_string ( module->config, "description" );

    Info_new( __func__, module->Thread_debug, LOG_INFO, "%s: Loading %s('%s')", thread_tech_id, hostname, description );

    PhidgetReturnCode result = PhidgetNet_addServer( hostname, hostname, 5661, Json_get_string(module->config, "password"), 0);
	   if (result != EPHIDGET_OK)
		   { const gchar *error;
       Phidget_getErrorDescription ( result, &error );
       Info_new( __func__, module->Thread_debug, LOG_ERR, "PhidgetNet_addServer failed: '%s'", error );
       goto connect_failed;
     }

    JsonNode *RootNode = Json_node_create ();                                                     /* Envoi de la conf a l'API */
    if (RootNode)
     { Json_node_add_string ( RootNode, "thread_tech_id", thread_tech_id );
       JsonNode *API_result = Http_Post_to_global_API ( "/run/phidget/add/io", RootNode );
       Json_node_unref ( API_result );
       Json_node_unref ( RootNode );
     }
/* Chargement des I/O */
    Json_node_foreach_array_element ( module->config, "IO", Charger_un_IO, module );

    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/************************************************* Calcul de la comm **********************************************************/
       GSList *elements = vars->Liste_sensors;
       while ( elements )                                             /* Si tous les sensors sont attached, alors comm = TRUE */
        { struct PHIDGET_ELEMENT *element = elements->data;
          if(element->attached == FALSE) break;
          elements = g_slist_next ( elements );
        }
       Thread_send_comm_to_master ( module, (elements ? FALSE : TRUE) );
/****************************************************** Ecoute du master ******************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *tag = Json_get_string ( request, "tag" );

          if ( !strcasecmp( tag, "SET_DO" ) ) { Phidget_SET_DO ( module, request ); }
          Json_node_unref (request);
        }
     }

    PhidgetNet_removeServer( hostname );                                                /* Arrete la connexion au hub phidget */
    g_slist_foreach ( vars->Liste_sensors, (GFunc) Decharger_un_IO, NULL );
    /*Phidget_finalize(0); non thread_safe apres. */
    g_slist_foreach ( vars->Liste_sensors, (GFunc) g_free, NULL );
    g_slist_free ( vars->Liste_sensors );
connect_failed:
    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
