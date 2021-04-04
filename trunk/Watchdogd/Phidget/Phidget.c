/******************************************************************************************************************************/
/* Watchdogd/Phidget/Phidget.c  Gestion des modules PHIDGET Watchdgo 3.0                                                       */
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
/* Phidget_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                 */
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
                   "PRIMARY KEY (`id`)"
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
    database_version = 2;
    Modifier_configDB_int ( NOM_THREAD, "database_version", database_version );
  }
/******************************************************************************************************************************/
/* Charger_un_Hub: Charge un Hub dans la librairie                                                                            */
/* Entrée: La structure Json representant le hub                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Phidget_print_error ( void )
  {	PhidgetReturnCode errorCode;
    size_t errorDetailLen = 100;
    const gchar* errorString;
    gchar errorDetail[256];
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
  {
    Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
                "%s: Chargement du HUB '%s'('%s')", __func__, Json_get_string(element, "hostname"), Json_get_string(element, "description") );
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
/* Phidget_onVoltageRatioChange: Appelé quand un module I/O VoltageRatio a changé de valeur                                   */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onVoltageRatioChange ( PhidgetVoltageRatioInputHandle canal, void *ctx, double voltageRatio )
  {
	   Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
              "%s: Changement de valeur : %lf", __func__, voltageRatio );
  }
/******************************************************************************************************************************/
/* Phidget_onPHSensorChange: Appelé quand un module I/O PHSensor a changé de valeur                                           */
/* Entrée: le channel, le contexte, et la nouvelle valeur                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onPHSensorChange ( PhidgetPHSensorHandle canal, void *ctx, double PH )
  {
	   Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
              "%s: Changement de valeur : %lf", __func__, PH );
  }/***************************************************************************************************************************/
/* Phidget_onAttachHandler: Appelé quand un canal estmodule I/O VoltageRatio a changé de valeur                               */
/* Entrée: le channel, le contexte                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onAttachHandler ( PhidgetHandle ph, void *ctx )
  {
    //You can access the Phidget that fired the event by using the first parameter
    //of the event handler
    int deviceSerialNumber;
    Phidget_getDeviceSerialNumber(ph, &deviceSerialNumber);
	   Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
              "%s: Phidget S/N %d attached", __func__, deviceSerialNumber );
  }
/******************************************************************************************************************************/
/* Phidget_onAttachHandler: Appelé quand un canal estmodule I/O VoltageRatio a changé de valeur                               */
/* Entrée: le channel, le contexte                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void CCONV Phidget_onDetachHandler ( PhidgetHandle ph, void *ctx )
  {
    //You can access the Phidget that fired the event by using the first parameter
    //of the event handler
    int deviceSerialNumber;
    Phidget_getDeviceSerialNumber(ph, &deviceSerialNumber);
	   Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
              "%s: Phidget S/N %d detached", __func__, deviceSerialNumber );
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
/* Charger_un_IO: Charge une IO dans la librairie                                                                             */
/* Entrée: La structure Json representant l'i/o                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_AI (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { gchar *classe = Json_get_string(element, "classe");
    gint port     = Json_get_int   (element, "port");
    gint serial   = Json_get_int   (element, "serial");
    Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
                "%s: Chargement d'une AI '%s' port %d on S/N %d", __func__, classe, port, serial );

    if (!strcasecmp(classe, "VoltageRatioInput"))
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
    else if (!strcasecmp(classe, "PHSensor"))
     { PhidgetPHSensorHandle handle;
      	PhidgetPHSensor_create(&handle);
   	   PhidgetPHSensor_setOnPHChangeHandler(handle, Phidget_onPHSensorChange, NULL);
       Phidget_set_config ( (PhidgetHandle)handle, serial, port, FALSE );
       Phidget_setOnAttachHandler((PhidgetHandle)handle, Phidget_onAttachHandler, NULL);
       Phidget_setOnDetachHandler((PhidgetHandle)handle, Phidget_onDetachHandler, NULL);
	      //Open your Phidgets and wait for attachment
   	   if (Phidget_open ((PhidgetHandle)handle) != EPHIDGET_OK)
        {	Phidget_print_error();
          return;
        }
     }
    else
     { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_INFO,
                 "%s: classe phidget '%s' inconnue pour port %d on S/N %d", __func__, classe, port, serial );
       return;
     }
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
                                  "SELECT hub.serial,ai.* FROM phidget_AI AS ai "
                                  "INNER JOIN phidget_hub AS hub ON hub.id=ai.hub_id WHERE hub.enable=1" ) == FALSE)
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
end:
    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
