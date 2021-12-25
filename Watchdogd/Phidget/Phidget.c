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
/*
 *  #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <errno.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <semaphore.h>
 #include <netinet/in.h>
 #include <netdb.h>*/

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Phidget.h"

/******************************************************************************************************************************/
/* Phidget_Creer_DB : Creation de la database du process                                                                      */
/* Entrée: le pointeur sur la structure PROCESS                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Phidget_Creer_DB ( struct PROCESS *lib )
  {
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'.", __func__, lib->database_version );

    if (lib->database_version==0)
     { SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                       "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`date_create` datetime NOT NULL DEFAULT NOW(),"
                       "`uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
                       "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                       "`enable` TINYINT(1) NOT NULL DEFAULT '0',"
                       "`hostname` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                       "`password` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "`serial` INT(11) UNIQUE NOT NULL DEFAULT '0',"
                       "FOREIGN KEY (`uuid`) REFERENCES `processes` (`uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", lib->name );
       SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s_AI` ("
                       "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`date_create` datetime NOT NULL DEFAULT NOW(),"
                       "`hub_id` int(11) NOT NULL,"
                       "`port` int(11) NOT NULL,"
                       "`classe` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`capteur` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`intervalle` int(11) NOT NULL,"
                       "UNIQUE (hub_id, port, classe),"
                       "FOREIGN KEY (`hub_id`) REFERENCES `phidget` (`id`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;", lib->name );
       SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s_DI` ("
                       "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "`hub_id` int(11) NOT NULL,"
                       "`port` int(11) NOT NULL,"
                       "`classe` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`capteur` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "UNIQUE (hub_id, port, classe),"
                       "FOREIGN KEY (`hub_id`) REFERENCES `phidget` (`id`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;", lib->name );
       SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s_DO` ("
                       "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "`hub_id` int(11) NOT NULL,"
                       "`port` int(11) NOT NULL,"
                       "`classe` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`capteur` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "UNIQUE (hub_id, port, classe),"
                       "FOREIGN KEY (`hub_id`) REFERENCES `phidget` (`id`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;", lib->name );
       goto end;
     }
end:
    Process_set_database_version ( lib, 1 );
  }
/******************************************************************************************************************************/
/* Charger_un_Hub: Charge un Hub dans la librairie                                                                            */
/* Entrée: La structure Json representant le hub                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Phidget_print_error ( struct PHIDGET_ELEMENT *canal )
  { PhidgetReturnCode errorCode;
    size_t errorDetailLen = 256;
    const gchar* errorString;
    gchar errorDetail[errorDetailLen];
    Phidget_getLastError(&errorCode, &errorString, errorDetail, &errorDetailLen);
    Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR,
              "%s: Phidget Error %d for '%s' (%s) : %s - %s", __func__, errorCode, canal->capteur, canal->classe, errorString, errorDetail );
  }
/******************************************************************************************************************************/
/* Phidget_onAIError: Appelé quand une erreur est constatée sur le module Phidget                                             */
/* Entrée: le channel, le contexte, et la description de l'erreur                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onError (PhidgetHandle ph, void *ctx, Phidget_ErrorEventCode code, const char *description)
  { struct PHIDGET_ELEMENT *canal = ctx;

    if ( !strcmp ( canal->classe, "VoltageInput" ) ||
         !strcmp ( canal->classe, "PHSensor" ) ||
         !strcmp ( canal->classe, "TemperatureSensor" ) ||
         !strcmp ( canal->classe, "VoltageRatioInput" ) )
     { if (!canal->dls_ai)
        { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
          return;
        }
       Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: Error for '%s:%s' : '%s' (code %X). Inrange = FALSE;", __func__,
                 canal->dls_ai->tech_id, canal->dls_ai->acronyme, description, code );
       Dls_data_set_AI ( canal->dls_ai->tech_id, canal->dls_ai->acronyme, (gpointer)&canal->dls_ai, 0.0, FALSE );
     }
    else if ( !strcmp ( canal->classe, "DigitalInput" ) )
     { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: Error for '%s:%s' : '%s' (code %X).", __func__,
                 canal->map_tech_id, canal->map_acronyme, description, code );
     }
    else if ( !strcmp ( canal->classe, "DigitalOutput" ) )
     { if (!canal->dls_do)
        { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_DO.", __func__ );
          return;
        }
       Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: Error for '%s:%s' : '%s' (code %X).", __func__,
                 canal->dls_do->tech_id, canal->dls_do->acronyme, description, code );
     }
  }
/******************************************************************************************************************************/
/* Phidget_onPHSensorChange: Appelé quand un module I/O PHSensor a changé de valeur                                           */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onPHSensorChange ( PhidgetPHSensorHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    if (!canal->dls_ai)
     { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
       return;
     }
    Info_new( Config.log, canal->module->lib->Thread_debug, LOG_INFO,
              "%s: '%s':'%s' = %f", __func__, canal->dls_ai->tech_id, canal->dls_ai->acronyme, valeur );
    Dls_data_set_AI ( canal->dls_ai->tech_id, canal->dls_ai->acronyme, (gpointer)&canal->dls_ai, valeur, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onTemperatureSensorChange: Appelé quand un module I/O Temperaute a changé de valeur                                */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onTemperatureSensorChange ( PhidgetTemperatureSensorHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    if (!canal->dls_ai)
     { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
       return;
     }
    Info_new( Config.log, canal->module->lib->Thread_debug, LOG_INFO,
              "%s: '%s':'%s' = %f %s", __func__, canal->dls_ai->tech_id, canal->dls_ai->acronyme, valeur, canal->dls_ai->unite );
    Dls_data_set_AI ( canal->dls_ai->tech_id, canal->dls_ai->acronyme, (gpointer)&canal->dls_ai, valeur, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onVoltableInputChange: Appelé quand un module I/O VoltageInput a changé de valeur                                  */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltageInputChange ( PhidgetVoltageInputHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    if (!canal->dls_ai)
     { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
       return;
     }
    Info_new( Config.log, canal->module->lib->Thread_debug, LOG_INFO,
              "%s: '%s':'%s' = %f %s", __func__, canal->dls_ai->tech_id, canal->dls_ai->acronyme, valeur, canal->dls_ai->unite );
    Dls_data_set_AI ( canal->dls_ai->tech_id, canal->dls_ai->acronyme, (gpointer)&canal->dls_ai, valeur, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onVoltageInputChange: Appelé quand un module I/O VoltageInput a changé de valeur                                   */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltageSensorChange ( PhidgetVoltageInputHandle handle, void *ctx, double valeur,
                                                   Phidget_UnitInfo *sensorUnit )
  { struct PHIDGET_ELEMENT *canal = ctx;
    if (!canal->dls_ai)
     { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
       return;
     }

    Info_new( Config.log, canal->module->lib->Thread_debug, LOG_INFO,
              "%s: '%s':'%s' = %f %s", __func__, canal->dls_ai->tech_id, canal->dls_ai->acronyme, valeur, canal->dls_ai->unite );
    Dls_data_set_AI ( canal->dls_ai->tech_id, canal->dls_ai->acronyme, (gpointer)&canal->dls_ai, valeur, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onVoltageRatoiInputChange: Appelé quand un module I/O RatioInput a changé de valeur                                */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltageRatioSensorChange ( PhidgetVoltageRatioInputHandle ch, void *ctx, double valeur,
                                                        Phidget_UnitInfo *sensorUnit)
  { struct PHIDGET_ELEMENT *canal = ctx;
    if (!canal->dls_ai)
     { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
       return;
     }

    Info_new( Config.log, canal->module->lib->Thread_debug, LOG_INFO,
              "%s: '%s':'%s' = %f %s", __func__, canal->dls_ai->tech_id, canal->dls_ai->acronyme, valeur, canal->dls_ai->unite );
    Dls_data_set_AI ( canal->dls_ai->tech_id, canal->dls_ai->acronyme, (gpointer)&canal->dls_ai, valeur, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onVoltableInputChange: Appelé quand un module I/O VoltageInput a changé de valeur                                  */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onDigitalInputChange ( PhidgetDigitalInputHandle handle, void *ctx, int valeur )
  { struct PHIDGET_ELEMENT *canal = ctx;
    Info_new( Config.log, canal->module->lib->Thread_debug, LOG_INFO,
              "%s: '%s':'%s' = %d", __func__, canal->map_tech_id, canal->map_acronyme, valeur );
    Zmq_Send_DI_to_master_new ( canal->module, canal->map_tech_id, canal->map_acronyme, (valeur !=0 ? TRUE : FALSE) );
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
    gchar *tech_id, *acronyme;

    Phidget_getDeviceSerialNumber(handle, &serial_number);
    Phidget_getDeviceChannelCount(handle, PHIDCHCLASS_NOTHING, &nbr_canaux );
    Phidget_getHubPort( handle, &port );
    Phidget_getChannel( handle, &num_canal );

    if ( !strcmp ( canal->classe, "VoltageInput" ) ||
         !strcmp ( canal->classe, "PHSensor" ) ||
         !strcmp ( canal->classe, "TemperatureSensor" ) ||
         !strcmp ( canal->classe, "VoltageRatioInput" ) )
     { if (!canal->dls_ai)
        { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
          return;
        }
       tech_id  = canal->dls_ai->tech_id;
       acronyme = canal->dls_ai->acronyme;
       Phidget_AnalogAttach ( canal );
     }
    else if ( !strcmp ( canal->classe, "DigitalInput" ) )
     { tech_id  = canal->map_tech_id;
       acronyme = canal->map_acronyme;
     }
    else if ( !strcmp ( canal->classe, "DigitalOutput" ) )
     { if (!canal->dls_do)
        { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_DO.", __func__ );
          return;
        }
       tech_id  = canal->dls_do->tech_id;
       acronyme = canal->dls_do->acronyme;
     }
    else
     { tech_id  = "Unknown";
       acronyme = "Unknown";
     }

    Info_new( Config.log, canal->module->lib->Thread_debug, LOG_NOTICE,
              "%s: '%s:%s' Phidget S/N '%d' Port '%d' classe '%s' (canal '%d') attached. %d channels available.",
              __func__, tech_id, acronyme, serial_number, port, canal->classe, num_canal, nbr_canaux );

    gchar description[64];
    g_snprintf( description, sizeof(description), "Management du module %s", canal->tech_id );

    if (Dls_auto_create_plugin( canal->tech_id, description ) == FALSE)
     { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, canal->tech_id ); }

    g_snprintf( description, sizeof(description), "Statud de la communication du module %s", canal->tech_id );
    Mnemo_auto_create_DI ( FALSE, canal->tech_id, "IO_COMM", description );

    SubProcess_send_comm_to_master_new ( canal->module, TRUE );
  }
/******************************************************************************************************************************/
/* Phidget_onAttachHandler: Appelé quand un canal est détaché                                                                 */
/* Entrée: le channel, le contexte                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onDetachHandler ( PhidgetHandle handle, void *ctx )
  { struct PHIDGET_ELEMENT *canal = ctx;
    int serial_number, nbr_canaux, port, num_canal;
    gchar *tech_id, *acronyme;

    Phidget_getDeviceSerialNumber(handle, &serial_number);
    Phidget_getDeviceChannelCount(handle, PHIDCHCLASS_NOTHING, &nbr_canaux );
    Phidget_getHubPort( handle, &port );
    Phidget_getChannel( handle, &num_canal );

    if ( !strcmp ( canal->classe, "VoltageInput" ) ||
         !strcmp ( canal->classe, "PHSensor" ) ||
         !strcmp ( canal->classe, "TemperatureSensor" ) ||
         !strcmp ( canal->classe, "VoltageRatioInput" ) )
     { if (!canal->dls_ai)
        { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
          return;
        }
       tech_id  = canal->dls_ai->tech_id;
       acronyme = canal->dls_ai->acronyme;
     }
    else if ( !strcmp ( canal->classe, "DigitalInput" ) )
     { tech_id  = canal->map_tech_id;
       acronyme = canal->map_acronyme;
     }
    else if ( !strcmp ( canal->classe, "DigitalOutput" ) )
     { if (!canal->dls_do)
        { Info_new( Config.log, canal->module->lib->Thread_debug, LOG_ERR, "%s: no DLS_DO.", __func__ );
          return;
        }
       tech_id  = canal->dls_do->tech_id;
       acronyme = canal->dls_do->acronyme;
     }
    else
     { tech_id  = "Unknown";
       acronyme = "Unknown";
     }

    Info_new( Config.log, canal->module->lib->Thread_debug, LOG_NOTICE,
              "%s: '%s:%s' Phidget S/N '%d' Port '%d' classe '%s' (canal '%d') detached . %d channels available.",
              __func__, tech_id, acronyme, serial_number, port, canal->classe, num_canal, nbr_canaux );
    SubProcess_send_comm_to_master_new ( canal->module, FALSE );
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
    gchar *hub_tech_id = Json_get_string(module->config, "tech_id");
    gchar *capteur  = Json_get_string(element, "capteur");
    gchar *classe   = Json_get_string(element, "classe");
    gint port       = Json_get_int   (element, "port");
    gchar *hub      = Json_get_string(element, "hub_description");
    gint serial     = Json_get_int   (element, "hub_serial");
    gint intervalle = Json_get_int   (element, "intervalle");

    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
              "%s: Hub %s('%s') (S/N %d), port '%d' capteur '%s'",
              __func__, hub_tech_id, hub, serial, port, capteur );

    struct PHIDGET_ELEMENT *canal = g_try_malloc0 ( sizeof(struct PHIDGET_ELEMENT) );
    if (!canal)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                 "%s: Memory Error on hub %s('%s') (S/N %d), port '%d' capteur '%s'",
                 __func__, hub_tech_id, hub, serial, port, capteur );
       return;
     }

    canal->module = module;                                                                      /* Sauvegarde du module père */
    g_snprintf( canal->capteur,     sizeof(canal->capteur), "%s", capteur );                 /* Sauvegarde du type de capteur */
    g_snprintf( canal->tech_id,     sizeof(canal->tech_id), "%s_P%d", hub_tech_id, port );   /* Sauvegarde du type de capteur */
    g_snprintf( canal->classe,      sizeof(canal->classe), "%s", classe );                   /* Sauvegarde du type de capteur */

    canal->intervalle = intervalle;                                               /* Sauvegarde de l'intervalle d'acquisition */
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    Charger_confDB_AI ( tech_id, acronyme );
    Dls_data_get_AI ( tech_id, acronyme, (gpointer)&canal->dls_ai );                      /* Récupération de l'élément DLS_AI */

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
     { Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
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
    gchar *hub_tech_id = Json_get_string(module->config, "tech_id");
    gchar *capteur  = Json_get_string(element, "capteur");
    gchar *classe   = Json_get_string(element, "classe");
    gchar *hub      = Json_get_string(element, "hub_description");
    gint port       = Json_get_int   (element, "port");
    gint serial     = Json_get_int   (element, "hub_serial");

    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
              "%s: Hub %s('%s') (S/N %d), port '%d' capteur '%s'",
              __func__, hub_tech_id, hub, serial, port, capteur );

    struct PHIDGET_ELEMENT *canal = g_try_malloc0 ( sizeof(struct PHIDGET_ELEMENT) );
    if (!canal)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                 "%s: Memory Error on hub %s('%s') (S/N %d), port '%d' capteur '%s'",
                 __func__, hub_tech_id, hub, serial, port, capteur );
       return;
     }

    g_snprintf( canal->capteur, sizeof(canal->capteur), "%s", capteur );               /* Sauvegarde du type de capteur */
    g_snprintf( canal->tech_id, sizeof(canal->tech_id), "%s_P%d", hub_tech_id, port );
    g_snprintf( canal->classe,  sizeof(canal->classe), "%s", classe );                 /* Sauvegarde du type de capteur */

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
    gchar *hub_tech_id = Json_get_string(module->config, "tech_id");
    gchar *capteur     = Json_get_string(element, "capteur");
    gchar *classe      = Json_get_string(element, "classe");
    gint port          = Json_get_int   (element, "port");
    gchar *hub         = Json_get_string(element, "hub_description");
    gint serial        = Json_get_int   (element, "hub_serial");

    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
              "%s: Hub %s('%s') (S/N %d), port '%d' capteur '%s'",
              __func__, hub_tech_id, hub, serial, port, capteur );

    struct PHIDGET_ELEMENT *canal = g_try_malloc0 ( sizeof(struct PHIDGET_ELEMENT) );
    if (!canal)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                 "%s: Memory Error on hub %s('%s') (S/N %d), port '%d' capteur '%s'",
                 __func__, hub_tech_id, hub, serial, port, capteur );
       return;
     }

    g_snprintf( canal->capteur,     sizeof(canal->capteur), "%s", capteur );                 /* Sauvegarde du type de capteur */
    g_snprintf( canal->tech_id,     sizeof(canal->tech_id), "%s_P%d", hub_tech_id, port );   /* Sauvegarde du type de capteur */
    g_snprintf( canal->classe,      sizeof(canal->classe), "%s", classe );                   /* Sauvegarde du type de capteur */
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    Dls_data_get_DO ( tech_id, acronyme, (gpointer)&canal->dls_do );                      /* Récupération de l'élément DLS_DI */
    if (!canal->dls_do) Dls_data_set_DO ( NULL, tech_id, acronyme, (gpointer)&canal->dls_do, FALSE );      /* Si n'existe pas */

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

    gchar *tech_id     = Json_get_string ( module->config, "tech_id" );
    gchar *hostname    = Json_get_string ( module->config, "hostname" );
    gchar *description = Json_get_string ( module->config, "description" );

    if (Json_get_bool ( module->config, "enable" ) == FALSE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Not Enabled. Stopping SubProcess", __func__, tech_id );
       SubProcess_end ( module );
     }

    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: Chargement du HUB '%s'('%s')", __func__, hostname, description );

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
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: Error Loading AI for %s: '%s'('%s')", __func__,
                 tech_id, hostname, description );
     }

    if (SQL_Select_to_json_node ( module->config, "DI",
                                  "SELECT hub.serial AS hub_serial,hub.description AS hub_description, "
                                  "di.*,m.tech_id,m.acronyme FROM phidget_DI AS di "
                                  "INNER JOIN phidget AS hub ON hub.id=di.hub_id "
                                  "INNER JOIN mnemos_DI AS m ON m.map_tech_id = CONCAT ( hub.tech_id, '_P', di.port ) "
                                  "WHERE hub.enable=1" ) == TRUE)
     { Json_node_foreach_array_element ( module->config, "DI", Charger_un_DI, module ); }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: Error Loading DI for %s: '%s'('%s')", __func__,
                 tech_id, hostname, description );
     }

    if (SQL_Select_to_json_node ( module->config, "DO",
                                  "SELECT hub.serial AS hub_serial,hub.description AS hub_description, "
                                  "do.*,m.tech_id,m.acronyme FROM phidget_DO AS do "
                                  "INNER JOIN phidget AS hub ON hub.id=do.hub_id "
                                  "INNER JOIN mnemos_DO AS m ON m.map_tech_id = CONCAT ( hub.tech_id, '_P', do.port ) "
                                  "WHERE hub.enable=1" ) == TRUE)
     { Json_node_foreach_array_element ( module->config, "DO", Charger_un_DO, module ); }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: Error Loading DO for %s: '%s'('%s')", __func__,
                 tech_id, hostname, description );
     }

    while(module->lib->Thread_run == TRUE && module->lib->Thread_reload == FALSE)            /* On tourne tant que necessaire */
     { usleep(100000);
       sched_yield();

/******************************************************* Ecoute du master *****************************************************/
       JsonNode *request;
       while ( (request = SubProcess_Listen_to_master_new ( module ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "SET_DO" ) )
           { if (!Json_has_member ( request, "tech_id"))
              { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque tech_id", __func__ ); }
             else if (!Json_has_member ( request, "acronyme" ))
              { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque acronyme", __func__ ); }
             else if (!Json_has_member ( request, "etat" ))
              { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque etat", __func__ ); }
             else
              { gchar *tech_id  = Json_get_string ( request, "tech_id" );
                gchar *acronyme = Json_get_string ( request, "acronyme" );
                gboolean etat   = Json_get_bool   ( request, "etat" );

                Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: Recu SET_DO from bus: %s:%s",
                          __func__, tech_id, acronyme );

                GSList *liste = vars->Liste_sensors;
                while (liste)
                 { struct PHIDGET_ELEMENT *canal = liste->data;
                   if ( !strcasecmp ( canal->classe, "DigitalOutput" ) &&
                        !strcasecmp ( canal->dls_do->tech_id, tech_id ) &&
                        !strcasecmp ( canal->dls_do->acronyme, acronyme ) )
                    { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: SET_DO %s:%s=%d", __func__,
                                canal->dls_do->tech_id, canal->dls_do->acronyme, etat );
                      if ( PhidgetDigitalOutput_setState( (PhidgetDigitalOutputHandle)canal->handle, etat ) != EPHIDGET_OK )
                       { Phidget_print_error ( canal ); }
                      break;
                    }
                   liste = g_slist_next(liste);
                 }
              }
           }
          json_node_unref (request);
        }
     }

    g_slist_foreach ( vars->Liste_sensors, (GFunc) g_free, NULL );
    g_slist_free ( vars->Liste_sensors );

    SubProcess_end(module);
  }
/******************************************************************************************************************************/
/* Run_process: Run du Process                                                                                                */
/* Entrée: la structure PROCESS associée                                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_process ( struct PROCESS *lib )
  {
reload:
    Phidget_Creer_DB ( lib );                                                                  /* Création de la DB du thread */
    Thread_init ( "phidget", "I/O", lib, WTD_VERSION, "Manage Phidget System" );

    lib->config = Json_node_create();
    if(lib->config) SQL_Select_to_json_node ( lib->config, "subprocess", "SELECT * FROM phidget WHERE uuid='%s'", lib->uuid );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: %d subprocess to load", __func__, Json_get_int ( lib->config, "nbr_subprocess" ) );

    if (lib->Thread_debug) PhidgetLog_enable(PHIDGET_LOG_INFO, "phidgetlog.log");

    Json_node_foreach_array_element ( lib->config, "subprocess", Process_Load_one_subprocess, lib );   /* Chargement des modules */
    while( lib->Thread_run == TRUE && lib->Thread_reload == FALSE) sleep(1);                 /* On tourne tant que necessaire */
    Process_Unload_all_subprocess ( lib );
    Phidget_resetLibrary();

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }

    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
