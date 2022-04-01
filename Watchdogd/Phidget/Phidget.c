/******************************************************************************************************************************/
/* Watchdogd/Phidget/Phidget.c  Gestion des modules PHIDGET Watchdgo 3.0                                                      */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    18.03.2021 22:02:42 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Phidget.c
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
    gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");
    size_t errorDetailLen = 256;
    const gchar* errorString;
    gchar errorDetail[errorDetailLen];
    Phidget_getLastError(&errorCode, &errorString, errorDetail, &errorDetailLen);
    Info_new( Config.log, canal->module->Thread_debug, LOG_ERR,
              "%s: %s: Phidget Error %d for '%s' (%s) : %s - %s", __func__,
              thread_tech_id, errorCode, canal->capteur, canal->classe, errorString, errorDetail );
  }
/******************************************************************************************************************************/
/* Phidget_onAIError: Appelé quand une erreur est constatée sur le module Phidget                                             */
/* Entrée: le channel, le contexte, et la description de l'erreur                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onError (PhidgetHandle ph, void *ctx, Phidget_ErrorEventCode code, const char *description)
  { struct PHIDGET_ELEMENT *canal = ctx;

    gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");
    if ( !strcmp ( canal->classe, "VoltageInput" ) ||
         !strcmp ( canal->classe, "PHSensor" ) ||
         !strcmp ( canal->classe, "TemperatureSensor" ) ||
         !strcmp ( canal->classe, "VoltageRatioInput" ) )
     { Info_new( Config.log, canal->module->Thread_debug, LOG_ERR,
		         "%s: %s: Error for '%s:%s' : '%s' (code %X). Inrange = FALSE;", __func__, thread_tech_id,
                 canal->map_tech_id, canal->map_acronyme, description, code );
       /*Http_Post_to_local_BUS_AI ( canal->module, canal->map_tech_id, canal->map_acronyme, 0.0, FALSE );*/
     }
    else if ( !strcmp ( canal->classe, "DigitalInput" ) )
     { Info_new( Config.log, canal->module->Thread_debug, LOG_ERR,
		         "%s: %s: Error for '%s:%s' : '%s' (code %X).", __func__, thread_tech_id,
                 canal->map_tech_id, canal->map_acronyme, description, code );
     }
    else if ( !strcmp ( canal->classe, "DigitalOutput" ) )
     { Info_new( Config.log, canal->module->Thread_debug, LOG_ERR,
		         "%s: %s: Error for '%s:%s' : '%s' (code %X).", __func__, thread_tech_id,
                 canal->map_tech_id, canal->map_acronyme, description, code );
     }
  }
/******************************************************************************************************************************/
/* Phidget_onPHSensorChange: Appelé quand un module I/O PHSensor a changé de valeur                                           */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onPHSensorChange ( PhidgetPHSensorHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");
    Info_new( Config.log, canal->module->Thread_debug, LOG_INFO,
              "%s: %s: '%s':'%s' = %f", __func__, thread_tech_id, canal->map_tech_id, canal->map_acronyme, valeur );
    /*Http_Post_to_local_BUS_AI ( module, canal->ai, valeur, TRUE );*/
  }
/******************************************************************************************************************************/
/* Phidget_onTemperatureSensorChange: Appelé quand un module I/O Temperaute a changé de valeur                                */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onTemperatureSensorChange ( PhidgetTemperatureSensorHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");
    Info_new( Config.log, canal->module->Thread_debug, LOG_INFO,
              "%s: %s: '%s':'%s' = %f", __func__, thread_tech_id, canal->map_tech_id, canal->map_acronyme, valeur );
    /*Http_Post_to_local_BUS_AI ( module, canal->ai, valeur, TRUE );*/
  }
/******************************************************************************************************************************/
/* Phidget_onVoltableInputChange: Appelé quand un module I/O VoltageInput a changé de valeur                                  */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltageInputChange ( PhidgetVoltageInputHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");
    Info_new( Config.log, canal->module->Thread_debug, LOG_INFO,
              "%s: %s: '%s':'%s' = %f", __func__, thread_tech_id, canal->map_tech_id, canal->map_acronyme, valeur );
    /*Http_Post_to_local_BUS_AI ( module, canal->ai, valeur, TRUE );*/
  }
/******************************************************************************************************************************/
/* Phidget_onVoltageInputChange: Appelé quand un module I/O VoltageInput a changé de valeur                                   */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltageSensorChange ( PhidgetVoltageInputHandle handle, void *ctx, double valeur,
                                                   Phidget_UnitInfo *sensorUnit )
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");
    Info_new( Config.log, canal->module->Thread_debug, LOG_INFO,
              "%s: %s: '%s':'%s' = %f", __func__, thread_tech_id, canal->map_tech_id, canal->map_acronyme, valeur );
    /*Http_Post_to_local_BUS_AI ( module, canal->ai, valeur, TRUE );*/
  }
/******************************************************************************************************************************/
/* Phidget_onVoltageRatoiInputChange: Appelé quand un module I/O RatioInput a changé de valeur                                */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltageRatioSensorChange ( PhidgetVoltageRatioInputHandle ch, void *ctx, double valeur,
                                                        Phidget_UnitInfo *sensorUnit)
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");
    Info_new( Config.log, canal->module->Thread_debug, LOG_INFO,
              "%s: %s: '%s':'%s' = %f", __func__, thread_tech_id, canal->map_tech_id, canal->map_acronyme, valeur );
    /*Http_Post_to_local_BUS_AI ( module, canal->ai, valeur, TRUE );*/
  }
/******************************************************************************************************************************/
/* Phidget_onVoltableInputChange: Appelé quand un module I/O VoltageInput a changé de valeur                                  */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onDigitalInputChange ( PhidgetDigitalInputHandle handle, void *ctx, int valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");
    Info_new( Config.log, canal->module->Thread_debug, LOG_INFO,
              "%s: %s: '%s':'%s' = %d", __func__, thread_tech_id, canal->map_tech_id, canal->map_acronyme, valeur );
    /*Http_Post_to_local_BUS_DI( canal->module, canal->map_tech_id, canal->map_acronyme, (valeur !=0 ? TRUE : FALSE) );*/
  }
/******************************************************************************************************************************/
/* Phidget_AnalogAttach: Appelé quand un canal analogique est en cours d'attachement                                          */
/* Entrée: le canal                                                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Phidget_AnalogAttach ( struct PHIDGET_ELEMENT *canal )
  { if (canal->intervalle)
     { if (Phidget_setDataInterval( canal->handle, canal->intervalle ) != EPHIDGET_OK) Phidget_print_error(canal); }

    if (!strcasecmp(canal->capteur, "ADP1000-PH"))
     { /**/
     }
    else if (!strcasecmp(canal->capteur, "ADP1000-ORP"))
     { if ( PhidgetVoltageInput_setVoltageRange( (PhidgetVoltageInputHandle)canal->handle, VOLTAGE_RANGE_2V ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(canal->capteur, "TMP1200_0-PT100-3850"))
     { if ( PhidgetTemperatureSensor_setRTDType( (PhidgetTemperatureSensorHandle)canal->handle, RTD_TYPE_PT100_3850 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
       if ( PhidgetTemperatureSensor_setRTDWireSetup( (PhidgetTemperatureSensorHandle)canal->handle, RTD_WIRE_SETUP_2WIRE ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(canal->capteur, "TMP1200_0-PT100-3920"))
     { if ( PhidgetTemperatureSensor_setRTDType( (PhidgetTemperatureSensorHandle)canal->handle, RTD_TYPE_PT100_3920 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
       if ( PhidgetTemperatureSensor_setRTDWireSetup( (PhidgetTemperatureSensorHandle)canal->handle, RTD_WIRE_SETUP_2WIRE ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(canal->capteur, "AC-CURRENT-10A"))
     { if ( PhidgetVoltageInput_setSensorType ( (PhidgetVoltageInputHandle)canal->handle, SENSOR_TYPE_3500 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(canal->capteur, "AC-CURRENT-25A"))
     { if ( PhidgetVoltageInput_setSensorType ( (PhidgetVoltageInputHandle)canal->handle, SENSOR_TYPE_3501 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(canal->capteur, "AC-CURRENT-50A"))
     { if ( PhidgetVoltageInput_setSensorType ( (PhidgetVoltageInputHandle)canal->handle, SENSOR_TYPE_3502 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(canal->capteur, "AC-CURRENT-100A"))
     { if ( PhidgetVoltageInput_setSensorType ( (PhidgetVoltageInputHandle)canal->handle, SENSOR_TYPE_3503 ) != EPHIDGET_OK )
        { Phidget_print_error(canal); }
     }
    else if (!strcasecmp(canal->capteur, "TEMP_1124_0"))
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

    gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");
    Phidget_getDeviceSerialNumber(handle, &serial_number);
    Phidget_getDeviceChannelCount(handle, PHIDCHCLASS_NOTHING, &nbr_canaux );
    Phidget_getHubPort( handle, &port );
    Phidget_getChannel( handle, &num_canal );

    if ( !strcmp ( canal->classe, "VoltageInput" ) ||
         !strcmp ( canal->classe, "PHSensor" ) ||
         !strcmp ( canal->classe, "TemperatureSensor" ) ||
         !strcmp ( canal->classe, "VoltageRatioInput" ) )
     { Phidget_AnalogAttach ( canal ); }

    Info_new( Config.log, canal->module->Thread_debug, LOG_NOTICE,
              "%s: %s: '%s:%s' Phidget S/N '%d' Port '%d' classe '%s' (canal '%d') attached. %d channels available.",
              __func__, thread_tech_id, canal->map_tech_id, canal->map_acronyme, serial_number, port, canal->classe, num_canal, nbr_canaux );

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

    gchar *thread_tech_id = Json_get_string(canal->module->config, "thread_tech_id");
    Phidget_getDeviceSerialNumber(handle, &serial_number);
    Phidget_getDeviceChannelCount(handle, PHIDCHCLASS_NOTHING, &nbr_canaux );
    Phidget_getHubPort( handle, &port );
    Phidget_getChannel( handle, &num_canal );

    Info_new( Config.log, canal->module->Thread_debug, LOG_NOTICE,
              "%s: %s: '%s:%s' Phidget S/N '%d' Port '%d' classe '%s' (canal '%d') detached . %d channels available.",
              __func__, thread_tech_id, canal->map_tech_id, canal->map_acronyme, serial_number, port, canal->classe, num_canal, nbr_canaux );
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
 static void Charger_un_AI (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct SUBPROCESS *module = user_data;
    struct PHIDGET_VARS *vars = module->vars;
    gchar *thread_tech_id = Json_get_string(module->config, "thread_tech_id");
    gchar *capteur  = Json_get_string(element, "capteur");
    gchar *classe   = Json_get_string(element, "classe");
    gint port       = Json_get_int   (element, "port");
    gchar *hub      = Json_get_string(element, "hub_description");
    gint serial     = Json_get_int   (element, "hub_serial");
    gint intervalle = Json_get_int   (element, "intervalle");

    Info_new( Config.log, module->Thread_debug, LOG_INFO,
              "%s: %s('%s'): S/N %d, port '%d' capteur '%s'",
              __func__, thread_tech_id, hub, serial, port, capteur );

    struct PHIDGET_ELEMENT *canal = g_try_malloc0 ( sizeof(struct PHIDGET_ELEMENT) );
    if (!canal)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR,
                 "%s: %s('%s'): Memory Error (S/N %d), port '%d' capteur '%s'",
                 __func__, thread_tech_id, hub, serial, port, capteur );
       return;
     }

    canal->module = module;                                                                      /* Sauvegarde du module père */
    g_snprintf( canal->capteur,      sizeof(canal->capteur), "%s", capteur );                /* Sauvegarde du type de capteur */
    g_snprintf( canal->tech_id,      sizeof(canal->tech_id), "%s_P%d", thread_tech_id, port );
    g_snprintf( canal->classe,       sizeof(canal->classe), "%s", classe );                  /* Sauvegarde du type de capteur */
    g_snprintf( canal->map_tech_id,  sizeof(canal->map_tech_id),  "%s", Json_get_string ( element, "tech_id" ) );
    g_snprintf( canal->map_acronyme, sizeof(canal->map_acronyme), "%s", Json_get_string ( element, "acronyme" ) );

    canal->intervalle = intervalle;                                               /* Sauvegarde de l'intervalle d'acquisition */

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
    else
     { Info_new( Config.log, module->Thread_debug, LOG_INFO,
                 "%s: classe phidget inconnue on hub '%s'(S/N %d), port '%d' capteur '%s'", __func__, hub, serial, port, capteur );
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
/* Charger_un_AI: Charge une IO dans la librairie                                                                             */
/* Entrée: La structure Json representant l'i/o                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_DI (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct SUBPROCESS *module = user_data;
    struct PHIDGET_VARS *vars = module->vars;
    gchar *thread_tech_id = Json_get_string(module->config, "thread_tech_id");
    gchar *capteur  = Json_get_string(element, "capteur");
    gchar *classe   = Json_get_string(element, "classe");
    gchar *hub      = Json_get_string(element, "hub_description");
    gint port       = Json_get_int   (element, "port");
    gint serial     = Json_get_int   (element, "hub_serial");

    Info_new( Config.log, module->Thread_debug, LOG_INFO,
              "%s: Hub %s('%s') (S/N %d), port '%d' capteur '%s'",
              __func__, thread_tech_id, hub, serial, port, capteur );

    struct PHIDGET_ELEMENT *canal = g_try_malloc0 ( sizeof(struct PHIDGET_ELEMENT) );
    if (!canal)
     { Info_new( Config.log, module->Thread_debug, LOG_INFO,
                 "%s: Memory Error on hub %s('%s') (S/N %d), port '%d' capteur '%s'",
                 __func__, thread_tech_id, hub, serial, port, capteur );
       return;
     }

    canal->module = module;                                                                      /* Sauvegarde du module père */
    g_snprintf( canal->capteur,      sizeof(canal->capteur), "%s", capteur );                /* Sauvegarde du type de capteur */
    g_snprintf( canal->tech_id,      sizeof(canal->tech_id), "%s_P%d", thread_tech_id, port );
    g_snprintf( canal->classe,       sizeof(canal->classe), "%s", classe );                  /* Sauvegarde du type de capteur */
    g_snprintf( canal->map_tech_id,  sizeof(canal->map_tech_id),  "%s", Json_get_string ( element, "tech_id" ) );
    g_snprintf( canal->map_acronyme, sizeof(canal->map_acronyme), "%s", Json_get_string ( element, "acronyme" ) );

    if (!strcasecmp(capteur, "DIGITAL-INPUT"))
     { if ( PhidgetDigitalInput_create( (PhidgetDigitalInputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetDigitalInput_setOnStateChangeHandler( (PhidgetDigitalInputHandle)canal->handle, Phidget_onDigitalInputChange, canal ) ) goto error;
       Phidget_set_config ( canal, serial, port, TRUE );
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
/* Charger_un_AI: Charge une IO dans la librairie                                                                             */
/* Entrée: La structure Json representant l'i/o                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_DO (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct SUBPROCESS *module = user_data;
    struct PHIDGET_VARS *vars = module->vars;
    gchar *thread_tech_id = Json_get_string(module->config, "thread_tech_id");
    gchar *capteur     = Json_get_string(element, "capteur");
    gchar *classe      = Json_get_string(element, "classe");
    gint port          = Json_get_int   (element, "port");
    gchar *hub         = Json_get_string(element, "hub_description");
    gint serial        = Json_get_int   (element, "hub_serial");

    Info_new( Config.log, module->Thread_debug, LOG_INFO,
              "%s: Hub %s('%s') (S/N %d), port '%d' capteur '%s'",
              __func__, thread_tech_id, hub, serial, port, capteur );

    struct PHIDGET_ELEMENT *canal = g_try_malloc0 ( sizeof(struct PHIDGET_ELEMENT) );
    if (!canal)
     { Info_new( Config.log, module->Thread_debug, LOG_INFO,
                 "%s: Memory Error on hub %s('%s') (S/N %d), port '%d' capteur '%s'",
                 __func__, thread_tech_id, hub, serial, port, capteur );
       return;
     }

    canal->module = module;                                                                      /* Sauvegarde du module père */
    g_snprintf( canal->capteur,      sizeof(canal->capteur), "%s", capteur );                /* Sauvegarde du type de capteur */
    g_snprintf( canal->tech_id,      sizeof(canal->tech_id), "%s_P%d", thread_tech_id, port );
    g_snprintf( canal->classe,       sizeof(canal->classe), "%s", classe );                  /* Sauvegarde du type de capteur */
    g_snprintf( canal->map_tech_id,  sizeof(canal->map_tech_id),  "%s", Json_get_string ( element, "tech_id" ) );
    g_snprintf( canal->map_acronyme, sizeof(canal->map_acronyme), "%s", Json_get_string ( element, "acronyme" ) );

    if (!strcasecmp(capteur, "REL2001_0"))
     { if ( PhidgetDigitalOutput_create( (PhidgetDigitalOutputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       Phidget_set_config ( canal, serial, port, TRUE );
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
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct PHIDGET_VARS) );
    struct PHIDGET_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname    = Json_get_string ( module->config, "hostname" );
    gchar *description = Json_get_string ( module->config, "description" );

    if (Json_get_bool ( module->config, "enable" ) == FALSE)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: Not Enabled. Stopping SubProcess", __func__, thread_tech_id );
       SubProcess_end ( module );
     }

    Info_new( Config.log, module->Thread_debug, LOG_INFO, "%s: %s: Loading %s('%s')", __func__, thread_tech_id, hostname, description );

    PhidgetNet_addServer( hostname, hostname, 5661, Json_get_string(module->config, "password"), 0);
/* Chargement des I/O */
    if (SQL_Select_to_json_node ( module->config, "AI",
                                  "SELECT hub.serial AS hub_serial,hub.description AS hub_description, "
                                  "ai.*,m.tech_id,m.acronyme FROM phidget_AI AS ai "
                                  "INNER JOIN phidget AS hub ON hub.id=ai.hub_id "
                                  "INNER JOIN mnemos_AI AS m ON m.map_tech_id = CONCAT ( hub.tech_id, '_P', ai.port ) "
                                  "WHERE hub.enable=1" ) == TRUE)
     { Json_node_foreach_array_element ( module->config, "AI", Charger_un_AI, module ); }
    else
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: Error Loading AI: '%s'('%s')", __func__,
                 thread_tech_id, hostname, description );
     }

    if (SQL_Select_to_json_node ( module->config, "DI",
                                  "SELECT hub.serial AS hub_serial,hub.description AS hub_description, "
                                  "di.*,m.tech_id,m.acronyme FROM phidget_DI AS di "
                                  "INNER JOIN phidget AS hub ON hub.thread_tech_id=di.thread_tech_id "
                                  "INNER JOIN mappings AS m ON m.thread_tech_id  = hub.thread_tech_id "
                                  "                        AND m.thread_acronyme = di.thread_acronyme "
                                  "WHERE hub.enable=1" ) == TRUE)
     { Json_node_foreach_array_element ( module->config, "DI", Charger_un_DI, module ); }
    else
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: Error Loading DI: '%s'('%s')", __func__,
                 thread_tech_id, hostname, description );
     }

    if (SQL_Select_to_json_node ( module->config, "DO",
                                  "SELECT hub.serial AS hub_serial,hub.description AS hub_description, "
                                  "do.*,m.tech_id,m.acronyme FROM phidget_DO AS do "
                                  "INNER JOIN phidget AS hub ON hub.id=do.hub_id "
                                  "INNER JOIN mnemos_DO AS m ON m.map_tech_id = CONCAT ( hub.tech_id, '_P', do.port ) "
                                  "WHERE hub.enable=1" ) == TRUE)
     { Json_node_foreach_array_element ( module->config, "DO", Charger_un_DO, module ); }
    else
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: Error Loading DO: '%s'('%s')", __func__,
                 thread_tech_id, hostname, description );
     }

    gboolean synthese_comm = FALSE;                                                /* Synthese de la comm de tous les sensors */
    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { SubProcess_loop ( module );                                       /* Loop sur process pour mettre a jour la telemetrie */
/************************************************* Calcul de la comm **********************************************************/
       GSList *elements = vars->Liste_sensors;
       module->comm_status = TRUE;
       while ( elements )                                             /* Si tous les sensors sont attached, alors comm = TRUE */
        { struct PHIDGET_ELEMENT *element = elements->data;
          module->comm_status &= element->attached;
          elements = g_slist_next ( elements );
        }
/****************************************************** Ecoute du master ******************************************************/
       while ( module->Master_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->Master_messages->data;
          module->Master_messages = g_slist_remove ( module->Master_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *bus_tag = Json_get_string ( request, "bus_tag" );

          if ( !strcasecmp( bus_tag, "SET_DO" ) )
           { if (!Json_has_member ( request, "tech_id"))
              { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: requete mal formée manque tech_id", __func__ ); }
             else if (!Json_has_member ( request, "acronyme" ))
              { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: requete mal formée manque acronyme", __func__ ); }
             else if (!Json_has_member ( request, "etat" ))
              { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: requete mal formée manque etat", __func__ ); }
             else
              { gchar *tech_id  = Json_get_string ( request, "tech_id" );
                gchar *acronyme = Json_get_string ( request, "acronyme" );
                gboolean etat   = Json_get_bool   ( request, "etat" );

                Info_new( Config.log, module->Thread_debug, LOG_DEBUG, "%s: %s: Recu SET_DO from bus: %s:%s=%d",
                          __func__, thread_tech_id, tech_id, acronyme, etat );

                GSList *liste = vars->Liste_sensors;
                while (liste)
                 { struct PHIDGET_ELEMENT *canal = liste->data;
                   if ( !strcasecmp ( canal->classe, "DigitalOutput" ) &&
                        !strcasecmp ( canal->map_tech_id, tech_id ) &&
                        !strcasecmp ( canal->map_acronyme, acronyme ) )
                    { Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: %s: SET_DO %s:%s=%d", __func__,
                                thread_tech_id, canal->map_tech_id, canal->map_acronyme, etat );
                      if ( PhidgetDigitalOutput_setState( (PhidgetDigitalOutputHandle)canal->handle, etat ) != EPHIDGET_OK )
                       { Phidget_print_error ( canal ); }
                      break;
                    }
                   liste = g_slist_next(liste);
                 }
              }
           }
          Json_node_unref (request);
        }
     }

    g_slist_foreach ( vars->Liste_sensors, (GFunc) g_free, NULL );
    g_slist_free ( vars->Liste_sensors );

    SubProcess_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
