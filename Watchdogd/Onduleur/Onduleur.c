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
/* Creer_DB: Creer la table associée au Process                                                                               */
/* Entrée: le pointeur sur le PROCESS                                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Ups_Creer_DB ( struct PROCESS *lib )
  { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'.", __func__, lib->database_version );

    if (lib->database_version==0)
     { SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `ups` ("
                       "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`date_create` datetime NOT NULL DEFAULT NOW(),"
                       "`uuid` varchar(37) COLLATE utf8_unicode_ci NOT NULL,"
                       "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                       "`host` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                       "`name` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                       "`admin_username` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                       "`admin_password` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                       "FOREIGN KEY (`uuid`) REFERENCES `processes` (`uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                       "PRIMARY KEY (`id`) "
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;" );
       goto end;
     }

end:
    Process_set_database_version ( lib, 1 );
  }
/******************************************************************************************************************************/
/* Deconnecter: Deconnexion du ups                                                                                         */
/* Entrée: un id                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Deconnecter_UPS ( struct SUBPROCESS *module )
  { struct UPS_VARS *vars = module->vars;

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );
    gchar *host    = Json_get_string ( module->config, "host" );

    if (vars->started == TRUE)
     { upscli_disconnect( &vars->upsconn );
       vars->started = FALSE;
     }

    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE,
              "%s: %s disconnected (host='%s')", __func__, tech_id, host );
    SubProcess_send_comm_to_master_new ( module, FALSE );
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 static gboolean Connecter_ups ( struct SUBPROCESS *module )
  { struct UPS_VARS *vars = module->vars;
    gchar buffer[80];
    int connexion;

    gchar *tech_id        = Json_get_string ( module->config, "tech_id" );
    gchar *host           = Json_get_string ( module->config, "host" );
    gchar *name           = Json_get_string ( module->config, "name" );
    gchar *admin_username = Json_get_string ( module->config, "admin_username" );
    gchar *admin_password = Json_get_string ( module->config, "admin_password" );

    if ( (connexion = upscli_connect( &vars->upsconn, host, UPS_PORT_TCP, UPSCLI_CONN_TRYSSL)) == -1 )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                "%s: %s: connexion refused by ups (host '%s' -> %s)", __func__, tech_id, host,
                 (char *)upscli_strerror(&vars->upsconn) );
       return(FALSE);
     }

    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE,
              "%s: %s connected (host='%s')", __func__, tech_id, host );
/********************************************************* UPSDESC ************************************************************/
    g_snprintf( buffer, sizeof(buffer), "GET UPSDESC %s\n", name );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending GET UPSDESC failed (%s)", __func__, tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading GET UPSDESC failed (%s)", __func__, tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { gchar description[128];
	         g_snprintf( description, sizeof(description), "%s", buffer + strlen(name) + 9 );
          Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading GET UPSDESC %s", __func__, tech_id, description );
          SQL_Write_new ( "UPDATE %s SET description='%s' WHERE tech_id='%s'",
                          module->lib->name, description, tech_id );
        }
     }
/**************************************************** USERNAME ****************************************************************/
    g_snprintf( buffer, sizeof(buffer), "USERNAME %s\n", admin_username );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending USERNAME failed %s", __func__, tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading USERNAME failed %s", __func__, tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading USERNAME %s", __func__, tech_id, buffer );
        }
     }

/******************************************************* PASSWORD *************************************************************/
    g_snprintf( buffer, sizeof(buffer), "PASSWORD %s\n", admin_password );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending PASSWORD failed %s", __func__, tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading PASSWORD failed %s", __func__, tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading PASSWORD %s", __func__, tech_id, buffer );
        }
     }

/************************************************** PREPARE LES AI ************************************************************/
    if (!vars->nbr_connexion)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                "%s: %s: Initialise le DLS et charge les AI ", __func__, tech_id );
       if (Dls_auto_create_plugin( tech_id, "Gestion de l'onduleur" ) == FALSE)
        { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", tech_id ); }

       Mnemo_auto_create_DI ( FALSE, tech_id, "OUTLET_1_STATUS", "Statut de la prise n°1" );
       Mnemo_auto_create_DI ( FALSE, tech_id, "OUTLET_2_STATUS", "Statut de la prise n°2" );
       Mnemo_auto_create_DI ( FALSE, tech_id, "UPS_ONLINE", "UPS Online" );
       Mnemo_auto_create_DI ( FALSE, tech_id, "UPS_CHARGING", "UPS en charge" );
       Mnemo_auto_create_DI ( FALSE, tech_id, "UPS_ON_BATT",  "UPS sur batterie" );
       Mnemo_auto_create_DI ( FALSE, tech_id, "UPS_REPLACE_BATT",  "Batteries UPS a changer" );
       Mnemo_auto_create_DI ( FALSE, tech_id, "UPS_ALARM",  "UPS en alarme !" );

       Mnemo_auto_create_AI ( FALSE, tech_id, "LOAD", "Charge onduleur", "%" );
       Zmq_Send_AI_to_master ( module->lib, tech_id, "LOAD", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, tech_id, "REALPOWER", "Charge onduleur", "W" );
       Zmq_Send_AI_to_master ( module->lib, tech_id, "REALPOWER", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, tech_id, "BATTERY_CHARGE", "Charge batterie", "%" );
       Zmq_Send_AI_to_master ( module->lib, tech_id, "BATTERY_CHARGE", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, tech_id, "INPUT_VOLTAGE", "Tension d'entrée", "V" );
       Zmq_Send_AI_to_master ( module->lib, tech_id, "INPUT_VOLTAGE", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, tech_id, "BATTERY_RUNTIME", "Durée de batterie restante", "s" );
       Zmq_Send_AI_to_master ( module->lib, tech_id, "BATTERY_RUNTIME", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, tech_id, "BATTERY_VOLTAGE", "Tension batterie", "V" );
       Zmq_Send_AI_to_master ( module->lib, tech_id, "BATTERY_VOLTAGE", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, tech_id, "INPUT_HZ", "Fréquence d'entrée", "HZ" );
       Zmq_Send_AI_to_master ( module->lib, tech_id, "INPUT_HZ", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, tech_id, "OUTPUT_CURRENT", "Courant de sortie", "A" );
       Zmq_Send_AI_to_master ( module->lib, tech_id, "OUTPUT_CURRENT", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, tech_id, "OUTPUT_HZ", "Fréquence de sortie", "HZ" );
       Zmq_Send_AI_to_master ( module->lib, tech_id, "OUTPUT_HZ", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, tech_id, "OUTPUT_VOLTAGE", "Tension de sortie", "V" );
       Zmq_Send_AI_to_master ( module->lib, tech_id, "OUTPUT_VOLTAGE", 0.0, FALSE );

       Mnemo_auto_create_DO ( FALSE, tech_id, "LOAD_OFF", "Coupe la sortie ondulée" );
       Mnemo_auto_create_DO ( FALSE, tech_id, "LOAD_ON", "Active la sortie ondulée" );
       Mnemo_auto_create_DO ( FALSE, tech_id, "OUTLET_1_OFF", "Désactive la prise n°1" );
       Mnemo_auto_create_DO ( FALSE, tech_id, "OUTLET_1_ON", "Active la prise n°1" );
       Mnemo_auto_create_DO ( FALSE, tech_id, "OUTLET_2_OFF", "Désactive la prise n°2" );
       Mnemo_auto_create_DO ( FALSE, tech_id, "OUTLET_2_ON", "Active la prise n°2" );
       Mnemo_auto_create_DO ( FALSE, tech_id, "START_DEEP_BAT", "Active un test de décharge profond" );
       Mnemo_auto_create_DO ( FALSE, tech_id, "START_QUICK_BAT", "Active un test de décharge léger" );
       Mnemo_auto_create_DO ( FALSE, tech_id, "STOP_TEST_BAT", "Stop le test de décharge batterie" );
     }

    vars->date_next_connexion = 0;
    vars->started = TRUE;
    vars->nbr_connexion++;
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE,
              "%s: %s up and running (host='%s')", __func__, tech_id, host );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Onduleur_set_instcmd: Envoi d'une instant commande à l'ups                                                                 */
/* Entrée : l'ups, le nom de la commande                                                                                      */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 static void Onduleur_set_instcmd ( struct SUBPROCESS *module, gchar *nom_cmd )
  { struct UPS_VARS *vars = module->vars;
	   gchar buffer[80];

    if (vars->started != TRUE) return;

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );
    gchar *name    = Json_get_string ( module->config, "name" );

    g_snprintf( buffer, sizeof(buffer), "INSTCMD %s %s\n", name, nom_cmd );
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: %s: Sending '%s'", __func__, tech_id, buffer );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                 "%s: %s: Sending INSTCMD failed with error '%s' for '%s'", __func__, tech_id,
                 (char *)upscli_strerror(&vars->upsconn), buffer );
       Deconnecter_UPS ( module );
       return;
     }

    if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                "%s: %s: Reading INSTCMD result failed (%s) error %s", __func__, tech_id,
                 nom_cmd, (char *)upscli_strerror(&vars->upsconn) );
       Deconnecter_UPS ( module );
       return;
     }
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: %s: Sending '%s' OK", __func__, tech_id, nom_cmd );
  }
/******************************************************************************************************************************/
/* Onduleur_get_var: Recupere une valeur de la variable en parametre                                                          */
/* Entrée : l'ups, le nom de variable, la variable a renseigner                                                               */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 static gchar *Onduleur_get_var ( struct SUBPROCESS *module, gchar *nom_var )
  { struct UPS_VARS *vars = module->vars;
	   static gchar buffer[80];
    gint retour_read;

    gchar *tech_id        = Json_get_string ( module->config, "tech_id" );
    gchar *name           = Json_get_string ( module->config, "name" );

    g_snprintf( buffer, sizeof(buffer), "GET VAR %s %s\n", name, nom_var );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending GET VAR failed (%s) error=%s", __func__, tech_id,
                buffer, (char *)upscli_strerror(&vars->upsconn) );
       Deconnecter_UPS ( module );
       return(NULL);
     }

    retour_read = upscli_readline( &vars->upsconn, buffer, sizeof(buffer) );
    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
             "%s: %s: Reading GET VAR %s ReadLine result = %d, upscli_upserror = %d, buffer = %s", __func__, tech_id,
              nom_var, retour_read, upscli_upserror(&vars->upsconn), buffer );
    if ( retour_read == -1 )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                "%s: %s: Reading GET VAR result failed (%s) error=%s", __func__, tech_id,
                 nom_var, (char *)upscli_strerror(&vars->upsconn) );
       return(NULL);
     }

    if ( ! strncmp ( buffer, "VAR", 3 ) )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
                "%s: %s: Reading GET VAR %s OK = %s", __func__, tech_id, nom_var, buffer );
       return(buffer + 6 + strlen(name) + strlen(nom_var));
     }

    if ( ! strcmp ( buffer, "ERR VAR-NOT-SUPPORTED" ) )
     { return(NULL);                                                         /* Variable not supported... is not an error ... */
     }

    Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
             "%s: %s: Reading GET VAR %s Failed : error %s (buffer %s)", __func__, tech_id,
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
 static gboolean Interroger_ups( struct SUBPROCESS *module )
  { gchar *reponse;

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );

    if ( (reponse = Onduleur_get_var ( module, "ups.load" )) != NULL )
     { Zmq_Send_AI_to_master ( module->lib, tech_id, "LOAD", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "ups.realpower" )) != NULL )
     { Zmq_Send_AI_to_master ( module->lib, tech_id, "REALPOWER", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.charge" )) != NULL )
     { Zmq_Send_AI_to_master ( module->lib, tech_id, "BATTERY_CHARGE", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "input.voltage" )) != NULL )
     { Zmq_Send_AI_to_master ( module->lib, tech_id, "INPUT_VOLTAGE", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.runtime" )) != NULL )
     { Zmq_Send_AI_to_master ( module->lib, tech_id, "BATTERY_RUNTIME", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.voltage" )) != NULL )
     { Zmq_Send_AI_to_master ( module->lib, tech_id, "BATTERY_VOLTAGE", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "input.frequency" )) != NULL )
     { Zmq_Send_AI_to_master ( module->lib, tech_id, "INPUT_HZ", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.current" )) != NULL )
     { Zmq_Send_AI_to_master ( module->lib, tech_id, "OUTPUT_CURRENT", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.frequency" )) != NULL )
     { Zmq_Send_AI_to_master ( module->lib, tech_id, "OUTPUT_HZ", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.voltage" )) != NULL )
     { Zmq_Send_AI_to_master ( module->lib, tech_id, "OUTPUT_VOLTAGE", atof(reponse+1), TRUE ); }

/*---------------------------------------------- Récupération des entrées TOR de l'UPS ---------------------------------------*/
    if ( (reponse = Onduleur_get_var ( module, "outlet.1.status" )) != NULL )
     { Zmq_Send_DI_to_master ( module->lib, tech_id, "OUTLET_1_STATUS", !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( module, "outlet.2.status" )) != NULL )
     { Zmq_Send_DI_to_master ( module->lib, tech_id, "OUTLET_2_STATUS", !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( module, "ups.status" )) != NULL )
     { Zmq_Send_DI_to_master ( module->lib, tech_id, "UPS_ONLINE",       (g_strrstr(reponse, "OL")?TRUE:FALSE) );
       Zmq_Send_DI_to_master ( module->lib, tech_id, "UPS_CHARGING",     (g_strrstr(reponse, "DISCHRG")?FALSE:TRUE) );
       Zmq_Send_DI_to_master ( module->lib, tech_id, "UPS_ON_BATT",      (g_strrstr(reponse, "OB")?TRUE:FALSE) );
       Zmq_Send_DI_to_master ( module->lib, tech_id, "UPS_REPLACE_BATT", (g_strrstr(reponse, "RB")?TRUE:FALSE) );
       Zmq_Send_DI_to_master ( module->lib, tech_id, "UPS_ALARM",        (g_strrstr(reponse, "ALARM")?TRUE:FALSE) );
     }
    Zmq_Send_DI_to_master ( module->lib, tech_id, "IO_COMM", TRUE );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct UPS_VARS) );
    struct UPS_VARS *vars = module->vars;

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );

    if (Dls_auto_create_plugin( tech_id, "Gestion de l'onduleur" ) == FALSE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, tech_id ); }

    while(module->lib->Thread_run == TRUE && module->lib->Thread_reload == FALSE)            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

/********************************************* Début de l'interrogation du ups ************************************************/
       if ( Partage->top >= vars->date_next_connexion )                               /* Si attente retente, on change de ups */
        { if ( ! vars->started )                                                                 /* Communication OK ou non ? */
           { if ( ! Connecter_ups( module ) )                                                 /* Demande de connexion a l'ups */
              { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: %s: Module DOWN", __func__, tech_id );
                Deconnecter_UPS ( module );                                               /* Sur erreur, on deconnecte le ups */
                vars->date_next_connexion = Partage->top + UPS_RETRY;
              }
           }
          else
           { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: %s: Interrogation ups", __func__, tech_id );
             if ( Interroger_ups ( module ) == FALSE )
              { Deconnecter_UPS ( module );
                vars->date_next_connexion = Partage->top + UPS_RETRY;                            /* On retente dans longtemps */
              }
             else vars->date_next_connexion = Partage->top + UPS_POLLING;                    /* Update toutes les xx secondes */
          }
        }
/******************************************************************************************************************************/
       JsonNode *request;
       while ( (request = SubProcess_Listen_to_master_new ( module ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "SET_DO" ) )
           { gchar *tech_id, *acronyme;
             tech_id  = Json_get_string ( request, "tech_id" );
             acronyme = Json_get_string ( request, "acronyme" );
             if (!tech_id)
              { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque tech_id", __func__ ); }
             else if (!acronyme)
              { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque acronyme", __func__ ); }
             else
              { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: Recu SET_DO from bus: %s:%s", __func__, tech_id, acronyme );

                if (!strcasecmp(acronyme, "LOAD_OFF"))        Onduleur_set_instcmd ( module, "load.off" );
                if (!strcasecmp(acronyme, "LOAD_ON"))         Onduleur_set_instcmd ( module, "load.on" );
                if (!strcasecmp(acronyme, "OUTLET_1_OFF"))    Onduleur_set_instcmd ( module, "outlet.1.load.off" );
                if (!strcasecmp(acronyme, "OUTLET_1_ON"))     Onduleur_set_instcmd ( module, "outlet.1.load.on" );
                if (!strcasecmp(acronyme, "OUTLET_2_OFF"))    Onduleur_set_instcmd ( module, "outlet.2.load.off" );
                if (!strcasecmp(acronyme, "OUTLET_2_ON"))     Onduleur_set_instcmd ( module, "outlet.2.load.on" );
                if (!strcasecmp(acronyme, "START_DEEP_BAT"))  Onduleur_set_instcmd ( module, "test.battery.start.deep" );
                if (!strcasecmp(acronyme, "START_QUICK_BAT")) Onduleur_set_instcmd ( module, "test.battery.start.quick" );
                if (!strcasecmp(acronyme, "STOP_TEST_BAT"))   Onduleur_set_instcmd ( module, "test.battery.stop" );
              }
           }
          json_node_unref (request);
        }
     }
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
    Ups_Creer_DB ( lib );                                                                     /* Création de la DB du thread */
    Thread_init ( "ups", "I/O", lib, WTD_VERSION, "Manage UPS Module" );

    lib->config = Json_node_create();
    if(lib->config) SQL_Select_to_json_node ( lib->config, "subprocess", "SELECT * FROM %s WHERE uuid='%s'", lib->name, lib->uuid );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: %d subprocess to load", __func__, Json_get_int ( lib->config, "nbr_subprocess" ) );

    Json_node_foreach_array_element ( lib->config, "subprocess", Process_Load_one_subprocess, lib );   /* Chargement des modules */
    while( lib->Thread_run == TRUE && lib->Thread_reload == FALSE) sleep(1);                 /* On tourne tant que necessaire */
    Process_Unload_all_subprocess ( lib );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }

    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
