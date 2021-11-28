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
 struct RADIO_CONFIG Cfg_radio;

 gchar *RADIO[][2] =
  { { "voltage", "http://start-voltage.ice.infomaniak.ch/playlists/start-voltage-high.mp3.m3u" }
  };


/******************************************************************************************************************************/
/* Radio_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                  */
/* Entrée: le pointeur sur la PROCESS                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Radio_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_radio.lib->Thread_debug = FALSE;                                                       /* Settings default parameters */
    Creer_configDB ( Cfg_radio.lib->name, "debug", "false" );

    if ( ! Recuperer_configDB( &db, Cfg_radio.lib->name ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_INFO, "%s: '%s' = %s", __func__, nom, valeur ); /* Print Config */
            if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_radio.lib->Thread_debug = TRUE;  }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Stopper_radio : Stop la diffusion radiophonique en cours                                                                   */
/* Entrée : Néant                                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Stopper_radio ( void )
  { if (Cfg_radio.radio_pid>0)
     { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_DEBUG,
                "%s: Sending kill to radio pid %d", __func__, Cfg_radio.radio_pid );
       kill(Cfg_radio.radio_pid, SIGTERM);
       Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_INFO,
                "%s: Waiting for pid %d termination", __func__, Cfg_radio.radio_pid );
       waitpid(Cfg_radio.radio_pid, NULL, 0);
     }
    Cfg_radio.radio_pid = 0;
  }
/******************************************************************************************************************************/
/* Jouer_wav: Jouer un fichier wav dont le nom est en paramètre                                                               */
/* Entrée : le nom du fichier wav                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static gboolean Jouer_radio ( gchar *radio )
  { Stopper_radio();
    Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_NOTICE, "%s: Starting playing radio %s", __func__, radio );
    Cfg_radio.radio_pid = fork();
    if (Cfg_radio.radio_pid<0)
     { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_ERR,
                "%s: CVLC '%s' fork failed pid=%d", __func__, radio, Cfg_radio.radio_pid );
       return(FALSE);
     }
    else if (!Cfg_radio.radio_pid)
     { execlp( "cvlc", "cvlc", radio, NULL );
       Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_ERR,
                "%s: CVLC '%s' exec failed pid=%d", __func__, radio, Cfg_radio.radio_pid );
       _exit(0);
     }
    else
     { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_DEBUG,
                "%s: CVLC '%s' is playing pid=%d", __func__, radio, Cfg_radio.radio_pid );
     }
    Cfg_radio.nbr_diffusion++;
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du Thread Radio                                                                                  */
/******************************************************************************************************************************/
 void Run_process ( struct PROCESS *lib )
  { 
reload:
    memset( &Cfg_radio, 0, sizeof(Cfg_radio) );                                     /* Mise a zero de la structure de travail */
    Cfg_radio.lib = lib;                                           /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "radio", "USER", lib, WTD_VERSION, "Manage RADIO Module" );
    Radio_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */

    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { sleep(1);
       JsonNode *request;
       while ( (request = Thread_Listen_to_master ( lib ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "PLAY_RADIO" ) )
           { gchar *radio = Json_get_string ( request, "radio" );
             Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_NOTICE, "%s : Diffusing %s", __func__, radio );
             Jouer_radio ( radio );
           }
          else if ( !strcasecmp( zmq_tag, "STOP_RADIO" ) )
           { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_NOTICE, "%s : Stopping radio", __func__ );
             Stopper_radio();
           }
          json_node_unref(request);
        }
     }
    Stopper_radio();

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
