/******************************************************************************************************************************/
/* Watchdogd/Radio/Radio.c  Gestion des messages radio de Watchdog 2.0                                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         sam. 09 nov. 2013 13:49:53 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.c
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

 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/wait.h>
 #include <fcntl.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Radio.h"

 gchar *RADIO[][2] =
  { { "voltage", "http://start-voltage.ice.infomaniak.ch/playlists/start-voltage-high.mp3.m3u" }
  };

/******************************************************************************************************************************/
/* Radio_Creer_DB : Creation de la database du process                                                                        */
/* Entrée: le pointeur sur la structure PROCESS                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Radio_Creer_DB ( struct PROCESS *lib )
  {
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'.", __func__, lib->database_version );

    if (lib->database_version==0)
     { SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                       "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "`uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
                       "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                       "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "FOREIGN KEY (`uuid`) REFERENCES `processes` (`uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", lib->name );
       goto end;
     }

end:
    Process_set_database_version ( lib, 1 );
  }
/******************************************************************************************************************************/
/* Stopper_radio : Stop la diffusion radiophonique en cours                                                                   */
/* Entrée : Néant                                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Stopper_radio ( struct SUBPROCESS *module )
  { struct RADIO_VARS *vars = module->vars;
    if (vars->radio_pid>0)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: Sending kill to radio pid %d", __func__, vars->radio_pid );
       kill(vars->radio_pid, SIGTERM);
       Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: Waiting for pid %d termination", __func__, vars->radio_pid );
       waitpid(vars->radio_pid, NULL, 0);
     }
    vars->radio_pid = 0;
  }
/******************************************************************************************************************************/
/* Jouer_wav: Jouer un fichier wav dont le nom est en paramètre                                                               */
/* Entrée : le nom du fichier wav                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static gboolean Jouer_radio ( struct SUBPROCESS *module, gchar *radio )
  { struct RADIO_VARS *vars = module->vars;
    Stopper_radio( module );
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: Starting playing radio %s", __func__, radio );
    vars->radio_pid = fork();
    if (vars->radio_pid<0)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                "%s: CVLC '%s' fork failed pid=%d", __func__, radio, vars->radio_pid );
       return(FALSE);
     }
    else if (!vars->radio_pid)
     { execlp( "cvlc", "cvlc", radio, NULL );
       Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: CVLC '%s' exec failed pid=%d", __func__, radio, vars->radio_pid );
       _exit(0);
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: CVLC '%s' is playing pid=%d", __func__, radio, vars->radio_pid );
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct RADIO_VARS) );
    /*struct RADIO_VARS *vars = module->vars;*/

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );

    if (Dls_auto_create_plugin( tech_id, "Gestion de la radio" ) == FALSE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, tech_id ); }

    SubProcess_send_comm_to_master_new ( module, TRUE );

    while(module->lib->Thread_run == TRUE && module->lib->Thread_reload == FALSE)            /* On tourne tant que necessaire */
     { usleep(100000);
       sched_yield();

       SubProcess_send_comm_to_master_new ( module, module->comm_status );         /* Périodiquement envoie la comm au master */
/******************************************************* Ecoute du master *****************************************************/
       JsonNode *request;
       while ( (request = SubProcess_Listen_to_master_new ( module ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "PLAY_RADIO" ) )
           { gchar *radio = Json_get_string ( request, "radio" );
             Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s : Diffusing %s", __func__, radio );
             Jouer_radio ( module, radio );
           }
          else if ( !strcasecmp( zmq_tag, "STOP_RADIO" ) )
           { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s : Stopping radio", __func__ );
             Stopper_radio( module );
           }
          else
           { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: zmq_tag '%s' not for this thread", __func__, zmq_tag ); }
          json_node_unref(request);
        }
     }
    Stopper_radio( module );
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
    Radio_Creer_DB ( lib );                                                                    /* Création de la DB du thread */
    Thread_init ( "radio", "USER", lib, WTD_VERSION, "Manage RADIO Module" );

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
