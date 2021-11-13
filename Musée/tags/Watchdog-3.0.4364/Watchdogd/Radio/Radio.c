/******************************************************************************************************************************/
/* Watchdogd/Radio/Radio.c  Gestion des messages radio de Watchdog 2.0                                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         sam. 09 nov. 2013 13:49:53 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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
/******************************************************************************************************************************/
/* Radio_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                  */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Radio_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_radio.lib->Thread_debug = FALSE;                                                       /* Settings default parameters */
    Cfg_radio.enable            = FALSE;

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_INFO, "%s: '%s' = %s", __func__, nom, valeur ); /* Print Config */
            if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_radio.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_radio.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
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
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct ZMQUEUE *zmq_from_bus;
    gchar radio[128];

    prctl(PR_SET_NAME, "W-RADIO", 0, 0, 0 );
    memset( &Cfg_radio, 0, sizeof(Cfg_radio) );                                     /* Mise a zero de la structure de travail */
    Cfg_radio.lib = lib;                                           /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_radio.lib->TID = pthread_self();                                                    /* Sauvegarde du TID pour le pere */
    Radio_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_NOTICE,
              "%s: Demarrage %s . . . TID = %p", __func__, VERSION, pthread_self() );
    Cfg_radio.lib->Thread_run = TRUE;                                                                   /* Le thread tourne ! */

    g_snprintf( Cfg_radio.lib->admin_prompt, sizeof(Cfg_radio.lib->admin_prompt), "radio" );
    g_snprintf( Cfg_radio.lib->admin_help,   sizeof(Cfg_radio.lib->admin_help),   "Manage Radio system" );

    if (!Cfg_radio.enable)
     { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_NOTICE,
                "%s: Thread is not enabled in config. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    zmq_from_bus = Connect_zmq ( ZMQ_SUB, "listen-to-bus", "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    g_snprintf( radio, sizeof(radio), "%s",                                                               /* Radio par défaut */
                "http://start-voltage.ice.infomaniak.ch/playlists/start-voltage-high.mp3.m3u" );
    while ( Cfg_radio.lib->Thread_run == TRUE)                                               /* On tourne tant que necessaire */
     { struct ZMQ_TARGET *event;
       gchar buffer[256];
       void *payload;

       if (Cfg_radio.lib->Thread_reload)                                                             /* On a recu reload ?? */
        { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_NOTICE, "%s: SIGUSR1", __func__ );
          Radio_Lire_config();
          Cfg_radio.lib->Thread_reload = FALSE;
        }

       if (Recv_zmq_with_tag ( zmq_from_bus, NOM_THREAD, &buffer, sizeof(buffer), &event, &payload ) > 0) /* Reception d'un paquet master ? */
        { if ( !strcmp( event->tag, "play_radio" ) )
           { if (strlen(payload)) { g_snprintf(radio, sizeof(radio), "%s", (gchar *) payload); }
             Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_DEBUG,
                       "%s : Reception d'un message PLAY RADIO : %s", __func__, radio );
             Jouer_radio ( radio );
           } else
          if ( !strcmp( event->tag, "stop_radio" ) )
           { Stopper_radio(); }
        }

       sleep(1);
     }
    Close_zmq ( zmq_from_bus );
    Stopper_radio();
end:
    Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    Cfg_radio.lib->Thread_run = FALSE;                                                          /* Le thread ne tourne plus ! */
    Cfg_radio.lib->TID = 0;                                                   /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
