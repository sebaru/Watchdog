/******************************************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.c  Gestion des upss UPS Watchdgo 2.0                                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mar. 10 nov. 2009 15:56:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Onduleur.c
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
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <upsclient.h>
 #include <locale.h>

/*********************************************************** Headers **********************************************************/
 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Onduleur.h"

/******************************************************************************************************************************/
/* Deconnecter: Deconnexion du ups                                                                                         */
/* Entrée: un id                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Deconnecter_UPS ( struct THREAD *module )
  { struct UPS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *host    = Json_get_string ( module->config, "host" );

    if (vars->started == TRUE)
     { upscli_disconnect( &vars->upsconn );
       vars->started = FALSE;
     }

    Http_Post_to_local_BUS_AI ( module, vars->Load, 0.0, FALSE );
    Http_Post_to_local_BUS_AI ( module, vars->Realpower, 0.0, FALSE );
    Http_Post_to_local_BUS_AI ( module, vars->Battery_charge, 0.0, FALSE );
    Http_Post_to_local_BUS_AI ( module, vars->Input_voltage, 0.0, FALSE );
    Http_Post_to_local_BUS_AI ( module, vars->Battery_runtime, 0.0, FALSE );
    Http_Post_to_local_BUS_AI ( module, vars->Battery_voltage, 0.0, FALSE );
    Http_Post_to_local_BUS_AI ( module, vars->Input_hz, 0.0, FALSE );
    Http_Post_to_local_BUS_AI ( module, vars->Output_current, 0.0, FALSE );
    Http_Post_to_local_BUS_AI ( module, vars->Output_hz, 0.0, FALSE );
    Http_Post_to_local_BUS_AI ( module, vars->Output_voltage, 0.0, FALSE );

    Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: %s disconnected (host='%s')", __func__, thread_tech_id, host );
    Thread_send_comm_to_master ( module, FALSE );
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 static gboolean Connecter_ups ( struct THREAD *module )
  { struct UPS_VARS *vars = module->vars;
    gchar buffer[80];
    int connexion;

    gchar *thread_tech_id        = Json_get_string ( module->config, "thread_tech_id" );
    gchar *host           = Json_get_string ( module->config, "host" );
    gchar *name           = Json_get_string ( module->config, "name" );
    gchar *admin_username = Json_get_string ( module->config, "admin_username" );
    gchar *admin_password = Json_get_string ( module->config, "admin_password" );

    if ( (connexion = upscli_connect( &vars->upsconn, host, UPS_PORT_TCP, UPSCLI_CONN_TRYSSL)) == -1 )
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                "%s: %s: connexion refused by ups (host '%s' -> %s)", __func__, thread_tech_id, host,
                 (char *)upscli_strerror(&vars->upsconn) );
       return(FALSE);
     }

    Info_new( Config.log, module->Thread_debug, LOG_NOTICE,
              "%s: %s connected (host='%s')", __func__, thread_tech_id, host );
/********************************************************* UPSDESC ************************************************************/
    g_snprintf( buffer, sizeof(buffer), "GET UPSDESC %s\n", name );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                "%s: %s: Sending GET UPSDESC failed (%s)", __func__, thread_tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading GET UPSDESC failed (%s)", __func__, thread_tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { gchar description[128];
          g_snprintf( description, sizeof(description), "%s", buffer + strlen(name) + 10 );
          description [ strlen(description) - 1 ] = 0; /* supprime les " du début/fin */
          Info_new( Config.log, module->Thread_debug, LOG_DEBUG, "%s: %s: Reading GET UPSDESC %s", __func__, thread_tech_id, description );
#warning a modifier en envoie API
          /*SQL_Write_new ( "UPDATE %s SET description='%s' WHERE thread_tech_id='%s'",
                          module->lib->name, description, thread_tech_id );*/
        }
     }
/**************************************************** USERNAME ****************************************************************/
    g_snprintf( buffer, sizeof(buffer), "USERNAME %s\n", admin_username );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                "%s: %s: Sending USERNAME failed %s", __func__, thread_tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading USERNAME failed %s", __func__, thread_tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { Info_new( Config.log, module->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading USERNAME %s", __func__, thread_tech_id, buffer );
        }
     }

/******************************************************* PASSWORD *************************************************************/
    g_snprintf( buffer, sizeof(buffer), "PASSWORD %s\n", admin_password );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                "%s: %s: Sending PASSWORD failed %s", __func__, thread_tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading PASSWORD failed %s", __func__, thread_tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { Info_new( Config.log, module->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading PASSWORD %s", __func__, thread_tech_id, buffer );
        }
     }

    vars->date_next_connexion = 0;
    vars->started = TRUE;
    Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: %s up and running (host='%s')", __func__, thread_tech_id, host );
    Thread_send_comm_to_master ( module, TRUE );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Onduleur_set_instcmd: Envoi d'une instant commande à l'ups                                                                 */
/* Entrée : l'ups, le nom de la commande                                                                                      */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 static void Onduleur_set_instcmd ( struct THREAD *module, gchar *nom_cmd )
  { struct UPS_VARS *vars = module->vars;
    gchar buffer[80];

    if (vars->started != TRUE) return;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *name    = Json_get_string ( module->config, "name" );

    g_snprintf( buffer, sizeof(buffer), "INSTCMD %s %s\n", name, nom_cmd );
    Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: %s: Sending '%s'", __func__, thread_tech_id, buffer );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                 "%s: %s: Sending INSTCMD failed with error '%s' for '%s'", __func__, thread_tech_id,
                 (char *)upscli_strerror(&vars->upsconn), buffer );
       Deconnecter_UPS ( module );
       return;
     }

    if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                "%s: %s: Reading INSTCMD result failed (%s) error %s", __func__, thread_tech_id,
                 nom_cmd, (char *)upscli_strerror(&vars->upsconn) );
       Deconnecter_UPS ( module );
       return;
     }
    Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: %s: Sending '%s' OK", __func__, thread_tech_id, nom_cmd );
  }
/******************************************************************************************************************************/
/* Onduleur_get_var: Recupere une valeur de la variable en parametre                                                          */
/* Entrée : l'ups, le nom de variable, la variable a renseigner                                                               */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 static gchar *Onduleur_get_var ( struct THREAD *module, gchar *nom_var )
  { struct UPS_VARS *vars = module->vars;
    static gchar buffer[80];
    gint retour_read;

    gchar *thread_tech_id        = Json_get_string ( module->config, "thread_tech_id" );
    gchar *name           = Json_get_string ( module->config, "name" );

    g_snprintf( buffer, sizeof(buffer), "GET VAR %s %s\n", name, nom_var );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                "%s: %s: Sending GET VAR failed (%s) error=%s", __func__, thread_tech_id,
                buffer, (char *)upscli_strerror(&vars->upsconn) );
       Deconnecter_UPS ( module );
       return(NULL);
     }

    retour_read = upscli_readline( &vars->upsconn, buffer, sizeof(buffer) );
    Info_new( Config.log, module->Thread_debug, LOG_DEBUG,
             "%s: %s: Reading GET VAR %s ReadLine result = %d, upscli_upserror = %d, buffer = %s", __func__, thread_tech_id,
              nom_var, retour_read, upscli_upserror(&vars->upsconn), buffer );
    if ( retour_read == -1 )
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                "%s: %s: Reading GET VAR result failed (%s) error=%s", __func__, thread_tech_id,
                 nom_var, (char *)upscli_strerror(&vars->upsconn) );
       return(NULL);
     }

    if ( ! strncmp ( buffer, "VAR", 3 ) )
     { Info_new( Config.log, module->Thread_debug, LOG_DEBUG,
                "%s: %s: Reading GET VAR %s OK = %s", __func__, thread_tech_id, nom_var, buffer );
       return(buffer + 6 + strlen(name) + strlen(nom_var));
     }

    if ( ! strcmp ( buffer, "ERR VAR-NOT-SUPPORTED" ) )
     { return(NULL);                                                         /* Variable not supported... is not an error ... */
     }

    Info_new( Config.log, module->Thread_debug, LOG_WARNING,
             "%s: %s: Reading GET VAR %s Failed : error %s (buffer %s)", __func__, thread_tech_id,
              nom_var, (char *)upscli_strerror(&vars->upsconn), buffer );
    Deconnecter_UPS ( module );
    vars->date_next_connexion = Partage->top + UPS_RETRY;
    return(NULL);
  }
/******************************************************************************************************************************/
/* Interroger_ups: Interrogation d'un ups                                                                                     */
/* Entrée: identifiants des upss ups                                                                                       */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                                               */
/******************************************************************************************************************************/
 static gboolean Interroger_ups( struct THREAD *module )
  { struct UPS_VARS *vars = module->vars;
    gchar *reponse;

    if ( (reponse = Onduleur_get_var ( module, "ups.load" )) != NULL )
     { Http_Post_to_local_BUS_AI ( module, vars->Load, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "ups.realpower" )) != NULL )
     { Http_Post_to_local_BUS_AI ( module, vars->Realpower, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.charge" )) != NULL )
     { Http_Post_to_local_BUS_AI ( module, vars->Battery_charge, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "input.voltage" )) != NULL )
     { Http_Post_to_local_BUS_AI ( module, vars->Input_voltage, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.runtime" )) != NULL )
     { Http_Post_to_local_BUS_AI ( module, vars->Battery_runtime, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.voltage" )) != NULL )
     { Http_Post_to_local_BUS_AI ( module, vars->Battery_voltage, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "input.frequency" )) != NULL )
     { Http_Post_to_local_BUS_AI ( module, vars->Input_hz, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.current" )) != NULL )
     { Http_Post_to_local_BUS_AI ( module, vars->Output_current, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.frequency" )) != NULL )
     { Http_Post_to_local_BUS_AI ( module, vars->Output_hz, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.voltage" )) != NULL )
     { Http_Post_to_local_BUS_AI ( module, vars->Output_voltage, atof(reponse+1), TRUE ); }

/*---------------------------------------------- Récupération des entrées TOR de l'UPS ---------------------------------------*/
    if ( (reponse = Onduleur_get_var ( module, "outlet.1.status" )) != NULL )
     { Http_Post_to_local_BUS_DI ( module, vars->Outlet_1_status, !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( module, "outlet.2.status" )) != NULL )
     { Http_Post_to_local_BUS_DI ( module, vars->Outlet_2_status, !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( module, "ups.status" )) != NULL )
     { Http_Post_to_local_BUS_DI ( module, vars->Ups_online,       (g_strrstr(reponse, "OL")?TRUE:FALSE) );
       Http_Post_to_local_BUS_DI ( module, vars->Ups_charging,     (g_strrstr(reponse, "DISCHRG")?FALSE:TRUE) );
       Http_Post_to_local_BUS_DI ( module, vars->Ups_on_batt,      (g_strrstr(reponse, "OB")?TRUE:FALSE) );
       Http_Post_to_local_BUS_DI ( module, vars->Ups_replace_batt, (g_strrstr(reponse, "RB")?TRUE:FALSE) );
       Http_Post_to_local_BUS_DI ( module, vars->Ups_alarm,        (g_strrstr(reponse, "ALARM")?TRUE:FALSE) );
     }

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Modbus_SET_DO: Met a jour une sortie TOR en fonction du jsonnode en parametre                                              */
/* Entrée: le module et le buffer Josn                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Ups_SET_DO ( struct THREAD *module, JsonNode *msg )
  { gchar *thread_tech_id      = Json_get_string ( module->config, "thread_tech_id" );
    gchar *msg_thread_tech_id  = Json_get_string ( msg, "thread_tech_id" );
    gchar *msg_thread_acronyme = Json_get_string ( msg, "thread_acronyme" );
    gchar *msg_tech_id         = Json_get_string ( msg, "tech_id" );
    gchar *msg_acronyme        = Json_get_string ( msg, "acronyme" );

    if (!msg_thread_tech_id)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': requete mal formée manque msg_thread_tech_id", __func__, thread_tech_id ); }
    else if (!msg_thread_acronyme)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': requete mal formée manque msg_thread_acronyme", __func__, thread_tech_id ); }
    else if (strcasecmp (msg_thread_tech_id, thread_tech_id))
     { Info_new( Config.log, module->Thread_debug, LOG_DEBUG, "%s: '%s': Pas pour nous", __func__, thread_tech_id ); }
    else if (!Json_has_member ( msg, "etat" ))
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': requete mal formée manque etat", __func__, thread_tech_id ); }
    else
     { gboolean etat = Json_get_bool ( msg, "etat" );
       pthread_mutex_lock ( &module->synchro );
       Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: '%s': SET_DO '%s:%s'/'%s:%s'=%d", __func__,
                 thread_tech_id, msg_thread_tech_id, msg_thread_acronyme, msg_tech_id, msg_acronyme, etat );
       if (etat)
        { if (!strcasecmp(msg_thread_acronyme, "LOAD_OFF"))        Onduleur_set_instcmd ( module, "load.off" );
          if (!strcasecmp(msg_thread_acronyme, "LOAD_ON"))         Onduleur_set_instcmd ( module, "load.on" );
          if (!strcasecmp(msg_thread_acronyme, "OUTLET_1_OFF"))    Onduleur_set_instcmd ( module, "outlet.1.load.off" );
          if (!strcasecmp(msg_thread_acronyme, "OUTLET_1_ON"))     Onduleur_set_instcmd ( module, "outlet.1.load.on" );
          if (!strcasecmp(msg_thread_acronyme, "OUTLET_2_OFF"))    Onduleur_set_instcmd ( module, "outlet.2.load.off" );
          if (!strcasecmp(msg_thread_acronyme, "OUTLET_2_ON"))     Onduleur_set_instcmd ( module, "outlet.2.load.on" );
          if (!strcasecmp(msg_thread_acronyme, "START_DEEP_BAT"))  Onduleur_set_instcmd ( module, "test.battery.start.deep" );
          if (!strcasecmp(msg_thread_acronyme, "START_QUICK_BAT")) Onduleur_set_instcmd ( module, "test.battery.start.quick" );
          if (!strcasecmp(msg_thread_acronyme, "STOP_TEST_BAT"))   Onduleur_set_instcmd ( module, "test.battery.stop" );
        }
       pthread_mutex_unlock ( &module->synchro );
     }
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct UPS_VARS) );
    struct UPS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (Json_get_bool ( module->config, "enable" ) == FALSE)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': Not Enabled. Stopping Thread", __func__, thread_tech_id );
       Thread_end ( module );
     }

    vars->Outlet_1_status = Mnemo_create_thread_DI ( module, "OUTLET_1_STATUS", "Statut de la prise n°1" );
    vars->Outlet_2_status = Mnemo_create_thread_DI ( module, "OUTLET_2_STATUS", "Statut de la prise n°2" );
    vars->Ups_online      = Mnemo_create_thread_DI ( module, "UPS_ONLINE", "UPS Online" );
    vars->Ups_charging    = Mnemo_create_thread_DI ( module, "UPS_CHARGING", "UPS en charge" );
    vars->Ups_on_batt     = Mnemo_create_thread_DI ( module, "UPS_ON_BATT",  "UPS sur batterie" );
    vars->Ups_replace_batt= Mnemo_create_thread_DI ( module, "UPS_REPLACE_BATT",  "Batteries UPS a changer" );
    vars->Ups_alarm       = Mnemo_create_thread_DI ( module, "UPS_ALARM",  "UPS en alarme !" );

    vars->Load            = Mnemo_create_thread_AI ( module, "LOAD", "Charge onduleur", "%", ARCHIVE_1_MIN );
    vars->Realpower       = Mnemo_create_thread_AI ( module, "REALPOWER", "Charge onduleur", "W", ARCHIVE_1_MIN );
    vars->Battery_charge  = Mnemo_create_thread_AI ( module, "BATTERY_CHARGE", "Charge batterie", "%", ARCHIVE_1_MIN );
    vars->Input_voltage   = Mnemo_create_thread_AI ( module, "INPUT_VOLTAGE", "Tension d'entrée", "V", ARCHIVE_1_MIN );
    vars->Battery_runtime = Mnemo_create_thread_AI ( module, "BATTERY_RUNTIME", "Durée de batterie restante", "s", ARCHIVE_1_MIN );
    vars->Battery_voltage = Mnemo_create_thread_AI ( module, "BATTERY_VOLTAGE", "Tension batterie", "V", ARCHIVE_1_MIN );
    vars->Input_hz        = Mnemo_create_thread_AI ( module, "INPUT_HZ", "Fréquence d'entrée", "HZ", ARCHIVE_1_MIN );
    vars->Output_current  = Mnemo_create_thread_AI ( module, "OUTPUT_CURRENT", "Courant de sortie", "A", ARCHIVE_1_MIN );
    vars->Output_hz       = Mnemo_create_thread_AI ( module, "OUTPUT_HZ", "Fréquence de sortie", "HZ", ARCHIVE_1_MIN );
    vars->Output_voltage  = Mnemo_create_thread_AI ( module, "OUTPUT_VOLTAGE", "Tension de sortie", "V", ARCHIVE_1_MIN );

    Mnemo_auto_create_DO ( FALSE, thread_tech_id, "LOAD_OFF", "Coupe la sortie ondulée" );
    Mnemo_auto_create_DO ( FALSE, thread_tech_id, "LOAD_ON", "Active la sortie ondulée" );
    Mnemo_auto_create_DO ( FALSE, thread_tech_id, "OUTLET_1_OFF", "Désactive la prise n°1" );
    Mnemo_auto_create_DO ( FALSE, thread_tech_id, "OUTLET_1_ON", "Active la prise n°1" );
    Mnemo_auto_create_DO ( FALSE, thread_tech_id, "OUTLET_2_OFF", "Désactive la prise n°2" );
    Mnemo_auto_create_DO ( FALSE, thread_tech_id, "OUTLET_2_ON", "Active la prise n°2" );
    Mnemo_auto_create_DO ( FALSE, thread_tech_id, "START_DEEP_BAT", "Active un test de décharge profond" );
    Mnemo_auto_create_DO ( FALSE, thread_tech_id, "START_QUICK_BAT", "Active un test de décharge léger" );
    Mnemo_auto_create_DO ( FALSE, thread_tech_id, "STOP_TEST_BAT", "Stop le test de décharge batterie" );

    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->WS_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->WS_messages->data;
          module->WS_messages = g_slist_remove ( module->WS_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *bus_tag = Json_get_string ( request, "bus_tag" );
          if ( !strcasecmp (bus_tag, "SET_DO") ) Ups_SET_DO ( module, request );
          Json_node_unref ( request );
        }
/********************************************* Début de l'interrogation du ups ************************************************/
       if ( Partage->top >= vars->date_next_connexion )                               /* Si attente retente, on change de ups */
        { if ( ! vars->started )                                                                 /* Communication OK ou non ? */
           { if ( ! Connecter_ups( module ) )                                                 /* Demande de connexion a l'ups */
              { Info_new( Config.log, module->Thread_debug, LOG_WARNING, "%s: %s: Module DOWN", __func__, thread_tech_id );
                Deconnecter_UPS ( module );                                               /* Sur erreur, on deconnecte le ups */
                vars->date_next_connexion = Partage->top + UPS_RETRY;
              }
           }
          else
           { Info_new( Config.log, module->Thread_debug, LOG_DEBUG, "%s: %s: Interrogation ups", __func__, thread_tech_id );
             if ( Interroger_ups ( module ) == FALSE )
              { Deconnecter_UPS ( module );
                vars->date_next_connexion = Partage->top + UPS_RETRY;                            /* On retente dans longtemps */
              }
             else vars->date_next_connexion = Partage->top + UPS_POLLING;                    /* Update toutes les xx secondes */
          }
        }
     }

    Json_node_unref ( vars->Load );
    Json_node_unref ( vars->Realpower );
    Json_node_unref ( vars->Battery_charge );
    Json_node_unref ( vars->Input_voltage );
    Json_node_unref ( vars->Battery_runtime );
    Json_node_unref ( vars->Battery_voltage );
    Json_node_unref ( vars->Input_hz );
    Json_node_unref ( vars->Output_current );
    Json_node_unref ( vars->Output_hz );
    Json_node_unref ( vars->Output_voltage );

    Json_node_unref ( vars->Outlet_1_status ),
    Json_node_unref ( vars->Outlet_2_status );
    Json_node_unref ( vars->Ups_online );
    Json_node_unref ( vars->Ups_charging );
    Json_node_unref ( vars->Ups_on_batt );
    Json_node_unref ( vars->Ups_replace_batt );
    Json_node_unref ( vars->Ups_alarm );

    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
