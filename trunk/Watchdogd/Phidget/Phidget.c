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

 struct PHIDGET_CONFIG Cfg_phidget;
/******************************************************************************************************************************/
/* Phidget_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gboolean Phidget_Lire_config ( void )
  { gchar *result;
    Creer_configDB ( NOM_THREAD, "debug", "false" );
    result = Recuperer_configDB_by_nom ( NOM_THREAD, "debug" );
    Cfg_phidget.lib->Thread_debug = !g_ascii_strcasecmp(result, "true");
    g_free(result);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Phidget_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                 */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Phidget_Creer_DB ( void )
  { gint database_version;

    gchar *database_version_string = Recuperer_configDB_by_nom( NOM_THREAD, "database_version" );
    if (database_version_string)
     { database_version = atoi( database_version_string );
       g_free(database_version_string);
     } else database_version=0;

    Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'. Thread_Version '%s'.", __func__, database_version, WTD_VERSION );

    if (database_version==0)
     { SQL_Write ( "CREATE TABLE IF NOT EXISTS `phidget_hub` ("
                   "`id` int(11) NOT NULL AUTO_INCREMENT,"
                   "`date_create` datetime NOT NULL DEFAULT NOW(),"
                   "`enable` tinyint(1) NOT NULL DEFAULT '1',"
                   "`hostname` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                   "`password` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                   "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                   "`serial` INT(11) NOT NULL DEFAULT '0',"
                   "PRIMARY KEY (`id`),"
                   "UNIQUE (hostname),"
                   "UNIQUE (serial)"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;" );
       SQL_Write ( "CREATE TABLE IF NOT EXISTS `phidget_AI` ("
                   "`id` int(11) NOT NULL AUTO_INCREMENT,"
                   "`date_create` datetime NOT NULL DEFAULT NOW(),"
                   "`hub_id` int(11) NOT NULL,"
                   "`mnemo_id` int(11) NULL,"
                   "`classe` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                   "`capteur` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                   "`intervalle` int(11) NOT NULL,"
                   "`port` int(11) NOT NULL,"
                   "PRIMARY KEY (`id`),"
                   "UNIQUE (hub_id, port, classe),"
                   "UNIQUE (mnemo_id),"
                   "FOREIGN KEY (`hub_id`) REFERENCES `phidget_hub` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,"
                   "FOREIGN KEY (`mnemo_id`) REFERENCES `mnemos_AI` (`id`) ON DELETE SET NULL ON UPDATE CASCADE"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;" );
       goto end;
     }

end:
    database_version = 1;
    Modifier_configDB_int ( NOM_THREAD, "database_version", database_version );
  }
/******************************************************************************************************************************/
/* Charger_un_Hub: Charge un Hub dans la librairie                                                                            */
/* Entrée: La structure Json representant le hub                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Phidget_print_error ( void )
  {	PhidgetReturnCode errorCode;
    size_t errorDetailLen = 256;
    const gchar* errorString;
    gchar errorDetail[errorDetailLen];
    Phidget_getLastError(&errorCode, &errorString, errorDetail, &errorDetailLen);
    Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_ERR,
              "%s: Phidget Error %d : %s - %s", __func__, errorCode, errorString, errorDetail );
 	}
/******************************************************************************************************************************/
/* Charger_un_Hub: Charge un Hub dans la librairie                                                                            */
/* Entrée: La structure Json representant le hub                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_Hub (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
              "%s: Chargement du HUB '%s'('%s')", __func__,
              Json_get_string(element, "hostname"), Json_get_string(element, "description") );
    PhidgetNet_addServer( Json_get_string(element, "hostname"),
                          Json_get_string(element, "hostname"), 5661,
                          Json_get_string(element, "password"), 0);
  }
/******************************************************************************************************************************/
/* Charger_tous_Hub: Requete la DB pour charger les hub phidget                                                               */
/* Entrée: rien                                                                                                               */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Charger_tous_Hub ( void  )
  { JsonNode *RootNode = Json_node_create ();
    if (!RootNode) return(FALSE);

    if (SQL_Select_to_json_node ( RootNode, "hubs", "SELECT * FROM phidget_hub WHERE enable=1" ) == FALSE)
     { json_node_unref(RootNode);
       return(FALSE);
     }

    Json_node_foreach_array_element ( RootNode, "hubs", Charger_un_Hub, NULL );
    json_node_unref(RootNode);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Phidget_onPHSensorChange: Appelé quand un module I/O PHSensor a changé de valeur                                           */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onPHSensorChange ( PhidgetPHSensorHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ANALOGINPUT *canal = ctx;
    if (!canal->dls_ai)
     { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
       return;
     }
	   Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
              "%s: '%s':'%s' = %lf", __func__, canal->dls_ai->tech_id, canal->dls_ai->acronyme, valeur );
    Dls_data_set_AI ( canal->dls_ai->tech_id, canal->dls_ai->acronyme, (gpointer)&canal->dls_ai, valeur, TRUE );
  }

/******************************************************************************************************************************/
/* Phidget_onVoltableInputError: Appelé quand une erreur est constatée sur le module Phidget                                  */
/* Entrée: le channel, le contexte, et la description de l'erreur                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltableInputError (PhidgetHandle ph, void *ctx, Phidget_ErrorEventCode code, const char* description)
  { struct PHIDGET_ANALOGINPUT *canal = ctx;
    if (!canal->dls_ai)
     { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
       return;
     }
    Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_ERR, "%s: Error for '%s:%s' : '%s' (code %X). Inrange = FALSE;", __func__,
              canal->dls_ai->tech_id, canal->dls_ai->acronyme, description, code );
    canal->dls_ai->inrange = FALSE;
  }
/******************************************************************************************************************************/
/* Phidget_onORPChange: Appelé quand un module I/O ORP a changé de valeur                                                     */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltableInputChange ( PhidgetVoltageInputHandle handle, void *ctx, double valeur )
  { struct PHIDGET_ANALOGINPUT *canal = ctx;
    if (!canal->dls_ai)
     { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_ERR, "%s: no DLS_AI.", __func__ );
       return;
     }
    Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
              "%s: '%s':'%s' = %lf", __func__, canal->dls_ai->tech_id, canal->dls_ai->acronyme, valeur );
    Dls_data_set_AI ( canal->dls_ai->tech_id, canal->dls_ai->acronyme, (gpointer)&canal->dls_ai, valeur, TRUE );
  }
/***************************************************************************************************************************/
/* Phidget_onAttachHandler: Appelé quand un canal estmodule I/O VoltageRatio a changé de valeur                               */
/* Entrée: le channel, le contexte                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onAnalogInputAttachHandler ( PhidgetHandle handle, void *ctx )
  { struct PHIDGET_ANALOGINPUT *canal = ctx;
	   int serial_number, nbr_canaux, port, num_canal;
    const char *classe;

    Phidget_getDeviceSerialNumber(handle, &serial_number);
    Phidget_getDeviceChannelCount(handle, PHIDCHCLASS_NOTHING, &nbr_canaux );
    Phidget_getHubPort(handle, &port );
    Phidget_getChannel( handle, &num_canal );
    Phidget_getChannelClassName( handle, &classe );

	   Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_NOTICE,
              "%s: Phidget S/N '%d' Port '%d' classe '%s' (canal '%d') attached with intervalle %d (sec). %d channels available.",
              __func__, serial_number, port, classe, num_canal, canal->intervalle, nbr_canaux );
    if (canal->intervalle)
     { if (Phidget_setDataInterval( handle, canal->intervalle*1000 ) != EPHIDGET_OK)	Phidget_print_error(); }

    if (!strcasecmp(canal->capteur, "ADP1000-ORP"))
     { if ( PhidgetVoltageInput_setVoltageRange( (PhidgetVoltageInputHandle)canal->handle, VOLTAGE_RANGE_2V ) != EPHIDGET_OK )
        { Phidget_print_error(); }
     }
  }
/******************************************************************************************************************************/
/* Phidget_onAttachHandler: Appelé quand un canal estmodule I/O VoltageRatio a changé de valeur                               */
/* Entrée: le channel, le contexte                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onAnalogInputDetachHandler ( PhidgetHandle handle, void *ctx )
  { struct PHIDGET_ANALOGINPUT *canal = ctx;
	   //You can access the Phidget that fired the event by using the first parameter
    //of the event handler
    int serial_number, nbr_canaux, port, num_canal;
    const char *classe;

    Phidget_getDeviceSerialNumber(handle, &serial_number);
    Phidget_getDeviceChannelCount(handle, PHIDCHCLASS_NOTHING, &nbr_canaux );
    Phidget_getHubPort(handle, &port );
    Phidget_getChannel( handle, &num_canal );
    Phidget_getChannelClassName( handle, &classe );
    Dls_data_set_AI ( canal->dls_ai->tech_id, canal->dls_ai->acronyme, (gpointer)&canal->dls_ai, 0.0, FALSE );
	   Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_NOTICE,
              "%s: Phidget S/N '%d' Port '%d' classe '%s' (canal '%d') detached . %d channels available.",
              __func__, serial_number, port, classe, num_canal, nbr_canaux );
  }
/******************************************************************************************************************************/
/* Charger_un_IO: Charge une IO dans la librairie                                                                             */
/* Entrée: La structure Json representant l'i/o                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Phidget_set_config ( PhidgetHandle handle, gint serial, gint port, gboolean is_hub_port )
   { if (Phidget_setDeviceSerialNumber((PhidgetHandle)handle, serial) != EPHIDGET_OK)
      {	Phidget_print_error();
        return;
    	 }
   	 if (Phidget_setIsHubPortDevice((PhidgetHandle)handle, is_hub_port) != EPHIDGET_OK)
      {	Phidget_print_error();
        return;
    	 }

   	 if (Phidget_setHubPort((PhidgetHandle)handle, port) != EPHIDGET_OK)
      {	Phidget_print_error();
        return;
    	 }

   	 if (Phidget_setIsRemote((PhidgetHandle)handle, 1) != EPHIDGET_OK)
      {	Phidget_print_error();
        return;
    	 }
   }
/******************************************************************************************************************************/
/* Charger_un_AI: Charge une IO dans la librairie                                                                             */
/* Entrée: La structure Json representant l'i/o                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_AI (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { gchar *capteur  = Json_get_string(element, "capteur");
    gint port       = Json_get_int   (element, "port");
    gchar *hub      = Json_get_string(element, "hub_description");
    gint serial     = Json_get_int   (element, "hub_serial");
    gint intervalle = Json_get_int   (element, "intervalle");
    Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
                "%s: Hub '%s' (S/N %d), port '%d' capteur '%s'", __func__, hub, serial, port, capteur );

    struct PHIDGET_ANALOGINPUT *canal = g_try_malloc0 ( sizeof(struct PHIDGET_ANALOGINPUT) );
    if (!canal)
     { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
                 "%s: Memory Error on hub '%s' (S/N %d), port '%d' capteur '%s'", __func__, hub, serial, port, capteur );
       return;
     }

    g_snprintf( canal->capteur, sizeof(canal->capteur), "%s", capteur );                     /* Sauvegarde du type de capteur */
    canal->intervalle = intervalle;                                               /* Sauvegarde de l'intervalle d'acquisition */
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    Charger_confDB_AI ( tech_id, acronyme );
    Dls_data_get_AI ( tech_id, acronyme, (gpointer)&canal->dls_ai );                      /* Récupération de l'élément DLS_AI */

    if (!strcasecmp(capteur, "test"))
     { /*PhidgetVoltageRatioInputHandle handle;
      	PhidgetVoltageRatioInput_create(&handle);
   	   /*PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(handle, Phidget_onVoltageRatioChange, NULL);*/
   	   /*PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(handle, Phidget_onVoltageRatioChange, NULL);*/
       /*Phidget_set_config ( (PhidgetHandle)handle, serial, port, TRUE );
       Phidget_setOnAttachHandler((PhidgetHandle)handle, Phidget_onAttachHandler, NULL);
       Phidget_setOnDetachHandler((PhidgetHandle)handle, Phidget_onDetachHandler, NULL);
	      //Open your Phidgets and wait for attachment
   	   if (Phidget_open ((PhidgetHandle)handle) != EPHIDGET_OK)
        {	Phidget_print_error();
          return;
        }*/
     }
    /* else if (!strcasecmp(classe, "VoltageInput"))
     { PhidgetVoltageRatioInputHandle handle;
      	PhidgetVoltageRatioInput_create(&handle);
   	   PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(handle, Phidget_onVoltageRatioChange, NULL);
       Phidget_set_config ( (PhidgetHandle)handle, serial, port, TRUE );
       Phidget_setOnAttachHandler((PhidgetHandle)handle, Phidget_onAttachHandler, NULL);
       Phidget_setOnDetachHandler((PhidgetHandle)handle, Phidget_onDetachHandler, NULL);
	      //Open your Phidgets and wait for attachment
   	   if (Phidget_open ((PhidgetHandle)handle) != EPHIDGET_OK)
        {	Phidget_print_error();
          return;
        }
     }*/
    else if (!strcasecmp(capteur, "ADP1000-PH"))
     { if ( PhidgetPHSensor_create( (PhidgetPHSensorHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
       if ( PhidgetPHSensor_setOnPHChangeHandler( (PhidgetPHSensorHandle)canal->handle, Phidget_onPHSensorChange, canal ) ) goto error;
       if ( Phidget_setOnErrorHandler( canal->handle, Phidget_onVoltableInputError, canal ) ) goto error;
     }
    else if (!strcasecmp(capteur, "ADP1000-ORP"))
     { if ( PhidgetVoltageInput_create( (PhidgetVoltageInputHandle *)&canal->handle ) != EPHIDGET_OK ) goto error;
   	   if ( PhidgetVoltageInput_setOnVoltageChangeHandler( (PhidgetVoltageInputHandle)canal->handle,
                                                            Phidget_onVoltableInputChange, canal ) != EPHIDGET_OK ) goto error;
       if ( Phidget_setOnErrorHandler( canal->handle, Phidget_onVoltableInputError, canal ) != EPHIDGET_OK ) goto error;
     }
    else
     { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
                 "%s: classe phidget inconnue on hub '%s'(S/N %d), port '%d' capteur '%s'", __func__, hub, serial, port, capteur );
       goto error;
     }

    Phidget_set_config ( (PhidgetHandle)canal->handle, serial, port, FALSE );
    Phidget_setOnAttachHandler((PhidgetHandle)canal->handle, Phidget_onAnalogInputAttachHandler, canal);
    Phidget_setOnDetachHandler((PhidgetHandle)canal->handle, Phidget_onAnalogInputDetachHandler, canal);
    if (Phidget_open ((PhidgetHandle)canal->handle) != EPHIDGET_OK) goto error;
    Cfg_phidget.Liste_sensors = g_slist_prepend ( Cfg_phidget.Liste_sensors, canal );
    return;
error:
  	 Phidget_print_error();
    g_free(canal);
  }
/******************************************************************************************************************************/
/* Charger_tous_IO: Charge toutes les I/O Phidget                                                                             */
/* Entrée: rien                                                                                                               */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Charger_tous_IO ( void  )
  { JsonNode *RootNode = Json_node_create ();
    if (!RootNode) return(FALSE);

    if (SQL_Select_to_json_node ( RootNode, "AI",
                                  "SELECT hub.serial AS hub_serial,hub.description AS hub_description, "
                                  "ai.*,m.tech_id,m.acronyme FROM phidget_AI AS ai "
                                  "INNER JOIN mnemos_AI AS m ON ai.mnemo_id=m.id "
                                  "INNER JOIN phidget_hub AS hub ON hub.id=ai.hub_id "
                                  "WHERE hub.enable=1" ) == FALSE)
     { json_node_unref(RootNode);
       return(FALSE);
     }

    Json_node_foreach_array_element ( RootNode, "AI", Charger_un_AI, NULL );
    json_node_unref(RootNode);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du PHIDGET                                                                                        */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  {

reload:
    memset( &Cfg_phidget, 0, sizeof(Cfg_phidget) );                                 /* Mise a zero de la structure de travail */
    Cfg_phidget.lib = lib;                                         /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "W-PHIDGET", "I/O", lib, WTD_VERSION, "Manage Phidget System" );
    Phidget_Lire_config ();                                                 /* Lecture de la configuration logiciel du thread */
    if (Config.instance_is_master==FALSE)
     { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_NOTICE,
                "%s: Instance is not Master. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }
    Phidget_Creer_DB();

    if (Cfg_phidget.lib->Thread_debug) PhidgetLog_enable(PHIDGET_LOG_INFO, "phidgetlog.log");

    if ( Charger_tous_Hub() == FALSE )                                                      /* Chargement des modules phidget */
     { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_ERR, "%s: Error while loading HUB PHIDGET -> stop", __func__ );
       Cfg_phidget.lib->Thread_run = FALSE;                                                     /* Le thread ne tourne plus ! */
     }

    if ( Charger_tous_IO() == FALSE )                                                      /* Chargement des modules phidget */
     { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_ERR, "%s: Error while loading IO PHIDGET -> stop", __func__ );
       Cfg_phidget.lib->Thread_run = FALSE;                                                     /* Le thread ne tourne plus ! */
     }

    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { usleep(100000);
     }

    Phidget_resetLibrary();
    g_slist_foreach ( Cfg_phidget.Liste_sensors, (GFunc) g_free, NULL );
    g_slist_free ( Cfg_phidget.Liste_sensors );

end:
    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
