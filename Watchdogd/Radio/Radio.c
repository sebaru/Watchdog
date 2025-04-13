/******************************************************************************************************************************/
/* Watchdogd/Radio/Radio.c  Gestion des messages radio de Watchdog 2.0                                                        */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                     sam. 09 nov. 2013 13:49:53 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.c
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
/* Stopper_radio : Stop la diffusion radiophonique en cours                                                                   */
/* Entrée : Néant                                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Stopper_radio ( struct THREAD *module )
  { struct RADIO_VARS *vars = module->vars;
    if (vars->radio_pid>0)
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Sending kill to radio pid %d", vars->radio_pid );
       kill(vars->radio_pid, SIGTERM);
       Info_new( __func__, module->Thread_debug, LOG_INFO, "Waiting for pid %d termination", vars->radio_pid );
       waitpid(vars->radio_pid, NULL, 0);
     }
    vars->radio_pid = 0;
  }
/******************************************************************************************************************************/
/* Jouer_wav: Jouer un fichier wav dont le nom est en paramètre                                                               */
/* Entrée : le nom du fichier wav                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static gboolean Jouer_radio ( struct THREAD *module, gchar *radio )
  { struct RADIO_VARS *vars = module->vars;
    Stopper_radio( module );
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Starting playing radio %s", radio );
    vars->radio_pid = fork();
    if (vars->radio_pid<0)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                "CVLC '%s' fork failed pid=%d", radio, vars->radio_pid );
       return(FALSE);
     }
    else if (!vars->radio_pid)
     { execlp( "cvlc", "cvlc", radio, NULL );
       Info_new( __func__, module->Thread_debug, LOG_ERR, "CVLC '%s' exec failed pid=%d", radio, vars->radio_pid );
       _exit(0);
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "CVLC '%s' is playing pid=%d", radio, vars->radio_pid );
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct RADIO_VARS) );
    /*struct RADIO_VARS *vars = module->vars;*/

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );

    Thread_send_comm_to_master ( module, TRUE );

    while(module->Thread_run == TRUE)                                                   /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *tag = Json_get_string ( request, "tag" );
          if ( !strcasecmp( tag, "PLAY_RADIO" ) )
           { gchar *radio = Json_get_string ( request, "radio" );
             Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Diffusing %s", tech_id, radio );
             Jouer_radio ( module, radio );
           }
          else if ( !strcasecmp( tag, "STOP_RADIO" ) )
           { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Stopping radio", tech_id );
             Stopper_radio( module );
           }
          else
           { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "%s: tag '%s' not for this thread", tech_id, tag ); }
          Json_node_unref(request);
        }
     }
    Stopper_radio( module );
    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
