/******************************************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.c  Gestion des ups Watchdog                                                                    */
/* Projet Abls-Habitat version 4.6       Gestion d'habitat                                     mar. 10 nov. 2009 15:56:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Onduleur.c
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

 #include <stdio.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
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

    MQTT_Send_AI ( module, vars->Load, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Realpower, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Battery_charge, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Input_voltage, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Battery_runtime, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Battery_voltage, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Input_hz, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Output_current, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Output_hz, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Output_voltage, 0.0, FALSE );

    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s disconnected (host='%s')", thread_tech_id, host );
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

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *host           = Json_get_string ( module->config, "host" );
    gchar *name           = Json_get_string ( module->config, "name" );
    gchar *admin_username = Json_get_string ( module->config, "admin_username" );
    gchar *admin_password = Json_get_string ( module->config, "admin_password" );

    if ( (connexion = upscli_connect( &vars->upsconn, host, UPS_PORT_TCP, UPSCLI_CONN_TRYSSL)) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: connexion refused by ups (host '%s' -> %s)", thread_tech_id, host,
                 (char *)upscli_strerror(&vars->upsconn) );
       return(FALSE);
     }

    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s connected (host='%s')", thread_tech_id, host );
/********************************************************* UPSDESC ************************************************************/
    g_snprintf( buffer, sizeof(buffer), "GET UPSDESC %s\n", name );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: Sending GET UPSDESC failed (%s)", thread_tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                   "%s: Reading GET UPSDESC failed (%s)", thread_tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { gchar description[128];
          g_snprintf( description, sizeof(description), "%s", buffer + strlen(name) + 10 );
          description [ strlen(description) - 1 ] = 0; /* supprime les " du début/fin */
          Info_new( __func__, module->Thread_debug, LOG_DEBUG, "%s: Reading GET UPSDESC %s", thread_tech_id, description );
        }
     }
/**************************************************** USERNAME ****************************************************************/
    g_snprintf( buffer, sizeof(buffer), "USERNAME %s\n", admin_username );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: Sending USERNAME failed %s", thread_tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                   "%s: Reading USERNAME failed %s", thread_tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { Info_new( __func__, module->Thread_debug, LOG_DEBUG,
                   "%s: Reading USERNAME %s", thread_tech_id, buffer );
        }
     }

/******************************************************* PASSWORD *************************************************************/
    g_snprintf( buffer, sizeof(buffer), "PASSWORD %s\n", admin_password );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: Sending PASSWORD failed %s", thread_tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                   "%s: Reading PASSWORD failed %s", thread_tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { Info_new( __func__, module->Thread_debug, LOG_DEBUG,
                   "%s: Reading PASSWORD %s", thread_tech_id, buffer );
        }
     }

    vars->date_next_connexion = 0;
    vars->started = TRUE;
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s connected (host='%s')", thread_tech_id, host );
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
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending '%s'", thread_tech_id, buffer );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "%s: Sending INSTCMD failed with error '%s' for '%s'", thread_tech_id,
                 (char *)upscli_strerror(&vars->upsconn), buffer );
       Deconnecter_UPS ( module );
       return;
     }

    if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: Reading INSTCMD result failed (%s) error %s", thread_tech_id,
                 nom_cmd, (char *)upscli_strerror(&vars->upsconn) );
       Deconnecter_UPS ( module );
       return;
     }
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending '%s' OK", thread_tech_id, nom_cmd );
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
    if (!vars->started) return(NULL);

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *name           = Json_get_string ( module->config, "name" );

    g_snprintf( buffer, sizeof(buffer), "GET VAR %s %s\n", name, nom_var );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING, "%s: Sending GET VAR failed (%s) error=%s", thread_tech_id,
                 buffer, (char *)upscli_strerror(&vars->upsconn) );
       Deconnecter_UPS ( module );
       return(NULL);
     }

    retour_read = upscli_readline( &vars->upsconn, buffer, sizeof(buffer) );
    Info_new( __func__, module->Thread_debug, LOG_DEBUG,
             "%s: Reading GET VAR %s ReadLine result = %d, upscli_upserror = %d, buffer = %s", thread_tech_id,
              nom_var, retour_read, upscli_upserror(&vars->upsconn), buffer );
    if ( retour_read == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING, "%s: Reading GET VAR result failed (%s) error=%s", thread_tech_id,
                 nom_var, (char *)upscli_strerror(&vars->upsconn) );
       Deconnecter_UPS ( module );
       return(NULL);
     }

    if ( ! strncmp ( buffer, "VAR", 3 ) )                                    /* si Réponse numérique de la part du UPS daemon */
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG,
                "%s: Reading GET VAR %s OK = %s", thread_tech_id, nom_var, buffer );
       return(buffer + 6 + strlen(name) + strlen(nom_var));
     }

    if ( ! strncmp ( buffer, "ERR", 3 ) )                                            /* Detection des erreurs type DATA-STALE */
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                "%s: Reading GET VAR %s ERROR = %s", thread_tech_id, nom_var, buffer );
     }

    return(NULL);                                                  /* VAR NOT SUPPORTED / DRIVER NOT CONNECTED are not errors */
  }
/******************************************************************************************************************************/
/* Interroger_ups: Interrogation d'un ups                                                                                     */
/* Entrée: identifiants des ups                                                                                               */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                                               */
/******************************************************************************************************************************/
 static void Interroger_ups( struct THREAD *module )
  { struct UPS_VARS *vars = module->vars;
    gchar *reponse;

    if ( (reponse = Onduleur_get_var ( module, "ups.load" )) != NULL )
     { MQTT_Send_AI ( module, vars->Load, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "ups.realpower" )) != NULL )
     { MQTT_Send_AI ( module, vars->Realpower, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.charge" )) != NULL )
     { MQTT_Send_AI ( module, vars->Battery_charge, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "input.voltage" )) != NULL )
     { MQTT_Send_AI ( module, vars->Input_voltage, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.runtime" )) != NULL )
     { MQTT_Send_AI ( module, vars->Battery_runtime, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.voltage" )) != NULL )
     { MQTT_Send_AI ( module, vars->Battery_voltage, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "input.frequency" )) != NULL )
     { MQTT_Send_AI ( module, vars->Input_hz, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.current" )) != NULL )
     { MQTT_Send_AI ( module, vars->Output_current, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.frequency" )) != NULL )
     { MQTT_Send_AI ( module, vars->Output_hz, atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.voltage" )) != NULL )
     { MQTT_Send_AI ( module, vars->Output_voltage, atof(reponse+1), TRUE ); }

/*---------------------------------------------- Récupération des entrées TOR de l'UPS ---------------------------------------*/
    if ( (reponse = Onduleur_get_var ( module, "outlet.1.status" )) != NULL )
     { MQTT_Send_DI ( module, vars->Outlet_1_status, !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( module, "outlet.2.status" )) != NULL )
     { MQTT_Send_DI ( module, vars->Outlet_2_status, !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( module, "ups.status" )) != NULL )
     { MQTT_Send_DI ( module, vars->Ups_online,       (g_strrstr(reponse, "OL")?TRUE:FALSE) );
       MQTT_Send_DI ( module, vars->Ups_charging,     (g_strrstr(reponse, "DISCHRG")?FALSE:TRUE) );
       MQTT_Send_DI ( module, vars->Ups_on_batt,      (g_strrstr(reponse, "OB")?TRUE:FALSE) );
       MQTT_Send_DI ( module, vars->Ups_replace_batt, (g_strrstr(reponse, "RB")?TRUE:FALSE) );
       MQTT_Send_DI ( module, vars->Ups_alarm,        (g_strrstr(reponse, "ALARM")?TRUE:FALSE) );
       Thread_send_comm_to_master ( module, TRUE );
     }
    else Thread_send_comm_to_master ( module, FALSE );
  }
/******************************************************************************************************************************/
/* Modbus_SET_DO: Met a jour une sortie TOR en fonction du jsonnode en parametre                                              */
/* Entrée: le module et le buffer Josn                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Ups_SET_DO ( struct THREAD *module, JsonNode *msg )
  { gchar *thread_tech_id      = Json_get_string ( module->config, "thread_tech_id" );
    gchar *msg_thread_tech_id  = Json_get_string ( msg, "token_lvl1" );
    gchar *msg_thread_acronyme = Json_get_string ( msg, "token_lvl2" );
    gchar *msg_tech_id         = Json_get_string ( msg, "tech_id" );
    gchar *msg_acronyme        = Json_get_string ( msg, "acronyme" );

    if (!msg_thread_tech_id)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': requete mal formée manque msg_thread_tech_id", thread_tech_id ); }
    else if (!msg_thread_acronyme)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': requete mal formée manque msg_thread_acronyme", thread_tech_id ); }
    else if (strcasecmp (msg_thread_tech_id, thread_tech_id))
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "'%s': Pas pour nous", thread_tech_id ); }
    else if (!Json_has_member ( msg, "etat" ))
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': requete mal formée manque etat", thread_tech_id ); }
    else
     { gboolean etat = Json_get_bool ( msg, "etat" );
       pthread_mutex_lock ( &module->synchro );
       Info_new( __func__, module->Thread_debug, LOG_NOTICE, "'%s': SET_DO '%s:%s'/'%s:%s'=%d",
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
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct UPS_VARS) );
    struct UPS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    vars->Outlet_1_status = Mnemo_create_thread_DI ( module, "OUTLET_1_STATUS", "Statut de la prise n°1" );
    vars->Outlet_2_status = Mnemo_create_thread_DI ( module, "OUTLET_2_STATUS", "Statut de la prise n°2" );
    vars->Ups_online      = Mnemo_create_thread_DI ( module, "UPS_ONLINE", "UPS Online" );
    vars->Ups_charging    = Mnemo_create_thread_DI ( module, "UPS_CHARGING", "UPS en charge" );
    vars->Ups_on_batt     = Mnemo_create_thread_DI ( module, "UPS_ON_BATT",  "UPS sur batterie" );
    vars->Ups_replace_batt= Mnemo_create_thread_DI ( module, "UPS_REPLACE_BATT",  "Batteries UPS à changer" );
    vars->Ups_alarm       = Mnemo_create_thread_DI ( module, "UPS_ALARM",  "UPS en alarme !" );

    vars->Load            = Mnemo_create_thread_AI ( module, "LOAD", "Charge onduleur", "%", ARCHIVE_1_MIN );
    vars->Realpower       = Mnemo_create_thread_AI ( module, "REALPOWER", "Charge onduleur", "W", ARCHIVE_1_MIN );
    vars->Battery_charge  = Mnemo_create_thread_AI ( module, "BATTERY_CHARGE", "Charge batterie", "%", ARCHIVE_1_MIN );
    vars->Input_voltage   = Mnemo_create_thread_AI ( module, "INPUT_VOLTAGE", "Tension d'entrée", "V", ARCHIVE_5_MIN );
    vars->Battery_runtime = Mnemo_create_thread_AI ( module, "BATTERY_RUNTIME", "Durée de batterie restante", "s", ARCHIVE_1_MIN );
    vars->Battery_voltage = Mnemo_create_thread_AI ( module, "BATTERY_VOLTAGE", "Tension batterie", "V", ARCHIVE_1_MIN );
    vars->Input_hz        = Mnemo_create_thread_AI ( module, "INPUT_HZ", "Fréquence d'entrée", "HZ", ARCHIVE_5_MIN );
    vars->Output_current  = Mnemo_create_thread_AI ( module, "OUTPUT_CURRENT", "Courant de sortie", "A", ARCHIVE_1_MIN );
    vars->Output_hz       = Mnemo_create_thread_AI ( module, "OUTPUT_HZ", "Fréquence de sortie", "HZ", ARCHIVE_5_MIN );
    vars->Output_voltage  = Mnemo_create_thread_AI ( module, "OUTPUT_VOLTAGE", "Tension de sortie", "V", ARCHIVE_5_MIN );

    Mnemo_create_thread_DO ( module, "LOAD_OFF",        "Coupe la sortie ondulée", TRUE );
    Mnemo_create_thread_DO ( module, "LOAD_ON",         "Active la sortie ondulée", TRUE );
    Mnemo_create_thread_DO ( module, "OUTLET_1_OFF",    "Désactive la prise n°1", TRUE );
    Mnemo_create_thread_DO ( module, "OUTLET_1_ON",     "Active la prise n°1", TRUE );
    Mnemo_create_thread_DO ( module, "OUTLET_2_OFF",    "Désactive la prise n°2", TRUE );
    Mnemo_create_thread_DO ( module, "OUTLET_2_ON",     "Active la prise n°2", TRUE );
    Mnemo_create_thread_DO ( module, "START_DEEP_BAT",  "Active un test de décharge profond", TRUE );
    Mnemo_create_thread_DO ( module, "START_QUICK_BAT", "Active un test de décharge léger", TRUE );
    Mnemo_create_thread_DO ( module, "STOP_TEST_BAT",   "Stop le test de décharge batterie", TRUE );

    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          if (Json_has_member ( request, "token_lvl0" ))
           { gchar *token_lvl0 = Json_get_string ( request, "token_lvl0" );
             if (!strcasecmp (token_lvl0, "SET_DO") ) Ups_SET_DO ( module, request );
           }
          Json_node_unref ( request );
        }
/********************************************* Début de l'interrogation du ups ************************************************/
       if ( Partage->top >= vars->date_next_connexion )                               /* Si attente retente, on change de ups */
        { if ( ! vars->started )                                                                 /* Communication OK ou non ? */
           { if ( ! Connecter_ups( module ) )                                                 /* Demande de connexion a l'ups */
              { Info_new( __func__, module->Thread_debug, LOG_WARNING, "%s: Module DOWN", thread_tech_id );
                Deconnecter_UPS ( module );                                               /* Sur erreur, on deconnecte le ups */
                vars->date_next_connexion = Partage->top + UPS_RETRY;
              }
           }
          else
           { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "%s: Interrogation ups", thread_tech_id );
             Interroger_ups ( module );
             vars->date_next_connexion = Partage->top + UPS_POLLING;                         /* Update toutes les xx secondes */
          }
        }
     }

    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
