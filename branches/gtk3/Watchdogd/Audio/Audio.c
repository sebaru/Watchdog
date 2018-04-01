/******************************************************************************************************************************/
/* Watchdogd/Audio/Audio.c  Gestion des messages audio de Watchdog 2.0                                                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         sam. 09 nov. 2013 13:49:53 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 #include "Audio.h"
/******************************************************************************************************************************/
/* Audio_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                  */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Audio_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_audio.lib->Thread_debug = FALSE;                                                       /* Settings default parameters */
    Cfg_audio.enable            = FALSE; 
    g_snprintf( Cfg_audio.language, sizeof(Cfg_audio.language), "%s", AUDIO_DEFAUT_LANGUAGE );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                "Audio_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO,                                           /* Print Config */
                "Audio_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_audio.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_audio.lib->Thread_debug = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "language" ) )
        { g_snprintf( Cfg_audio.language, sizeof(Cfg_audio.language), "%s", valeur ); }
       else
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE,
                   "Audio_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Jouer_wav: Jouer un fichier wav dont le nom est en paramètre                                                               */
/* Entrée : le nom du fichier wav                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Jouer_wav ( gchar *fichier )
  { gint pid;

    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO, "Jouer_wav: Envoi d'un wav %s", fichier );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                "Jouer_wav: APLAY '%s' fork failed pid=%d", fichier, pid );
     }
    else if (!pid)
     { execlp( "aplay", "aplay", "-R", "1", fichier, NULL );
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                "Jouer_wav: APLAY '%s' exec failed pid=%d", fichier, pid );
       _exit(0);
     }
    else
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                "Jouer_wav: APLAY '%s' waiting to finish pid=%d (%s)", fichier, pid, strerror(errno) );
       waitpid(pid, NULL, 0 );
     }
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
             "Jouer_wav: APLAY '%s' finished pid=%d", fichier, pid );
  }
/******************************************************************************************************************************/
/* Jouer_mp3 : Joue un fichier mp3 et attend la fin de la diffusion                                                           */
/* Entrée : le message à jouer                                                                                                */
/* Sortie : True si OK, False sinon                                                                                           */
/******************************************************************************************************************************/
 gboolean Jouer_mp3 ( struct CMD_TYPE_MESSAGE *msg )
  { gchar nom_fichier[128];
    gint fd_cible, pid;

    g_snprintf( nom_fichier, sizeof(nom_fichier), "Son/%d.mp3", msg->num );
    fd_cible = open ( nom_fichier, O_RDONLY, 0 );
    if (fd_cible < 0) { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                                  "%s: '%s' not found", __func__, nom_fichier );
                        return(FALSE);
                      }
    else close (fd_cible);

    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO, "Jouer_mp3: Send '%s'", nom_fichier );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                 "%s: '%s' fork failed pid=%d (%s)", __func__, nom_fichier, pid, strerror(errno) );
       return(FALSE);
     }
    else if (!pid)
     { execlp( "mpg123", "mpg123", "-q", nom_fichier, NULL );
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                "%s: '%s' exec failed pid=%d (%s)", __func__, nom_fichier, pid, strerror( errno ) );
       _exit(0);
     }
    else
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                "%s: '%s' waiting to finish pid=%d", __func__, nom_fichier, pid );
       waitpid(pid, NULL, 0 );
     }
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "%s: MPG123 '%s' finished pid=%d", __func__, nom_fichier, pid );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Jouer_google_speech : Joue un texte avec google_speech et attend la fin de la diffusion                                    */
/* Entrée : le message à jouer                                                                                                */
/* Sortie : True si OK, False sinon                                                                                           */
/******************************************************************************************************************************/
 gboolean Jouer_google_speech ( gchar *libelle_audio )
  { gint pid;

    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO, "%s: Send '%s'", __func__, libelle_audio );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                 "%s: '%s' fork failed pid=%d (%s)", __func__, libelle_audio, pid, strerror(errno) );
       return(FALSE);
     }
    else if (!pid)
     { execlp( "google_speech", "google_speech", "-v", "debug", "-l", Cfg_audio.language, libelle_audio, NULL );
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                "%s: '%s' exec failed pid=%d (%s)", __func__, libelle_audio, pid, strerror( errno ) );
       _exit(0);
     }
    else
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                "%s: '%s' waiting to finish pid=%d", __func__, libelle_audio, pid );
       waitpid(pid, NULL, 0 );
     }
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
             "%s: google_speech '%s' finished pid=%d", __func__, libelle_audio, pid );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du Thread Audio                                                                                  */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct CMD_TYPE_HISTO *histo, histo_buf;
    struct ZMQUEUE *zmq_msg;
    struct ZMQUEUE *zmq_master;
    struct ZMQUEUE *zmq_admin;
    static gboolean audio_stop = TRUE;

    prctl(PR_SET_NAME, "W-Audio", 0, 0, 0 );
    memset( &Cfg_audio, 0, sizeof(Cfg_audio) );                                     /* Mise a zero de la structure de travail */
    Cfg_audio.lib = lib;                                           /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_audio.lib->TID = pthread_self();                                                    /* Sauvegarde du TID pour le pere */
    Audio_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE,
              "%s: Demarrage %s . . . TID = %p", __func__, VERSION, pthread_self() );
    Cfg_audio.lib->Thread_run = TRUE;                                                                   /* Le thread tourne ! */

    g_snprintf( Cfg_audio.lib->admin_prompt, sizeof(Cfg_audio.lib->admin_prompt), "audio" );
    g_snprintf( Cfg_audio.lib->admin_help,   sizeof(Cfg_audio.lib->admin_help),   "Manage Audio system" );

    if (!Cfg_audio.enable)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread is not enabled in config. Shutting Down %p",
                 pthread_self() );
       goto end;
     }

    zmq_msg = New_zmq ( ZMQ_SUB, "listen-to-msgs" );
    Connect_zmq (zmq_msg, "inproc", ZMQUEUE_LIVE_MSGS, 0 );

    zmq_master = New_zmq ( ZMQ_SUB, "listen-to-MSRV" );
    Connect_zmq (zmq_master, "inproc", ZMQUEUE_LIVE_THREADS, 0 );

    zmq_admin = New_zmq ( ZMQ_REP, "listen-to-admin" );
    Bind_zmq (zmq_admin, "inproc", NOM_THREAD "-admin", 0 );

    while(Cfg_audio.lib->Thread_run == TRUE)                                                 /* On tourne tant que necessaire */
     { gchar buffer[256];
       struct MSRV_EVENT *event;
       void *payload;

       if (Cfg_audio.lib->Thread_sigusr1)                                                             /* On a recu sigusr1 ?? */
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE, "%s: SIGUSR1", __func__ );
          Audio_Lire_config();
          Cfg_audio.lib->Thread_sigusr1 = FALSE;
        }

       if (Cfg_audio.last_audio + 100 < Partage->top)                                /* Au bout de 10 secondes sans diffusion */
        { if (audio_stop == TRUE)                                /* Avons-nous deja envoyé une commande de STOP AUDIO a DLS ? */
           { audio_stop = FALSE;                                         /* Positionné quand il n'y a plus de diffusion audio */
             if (Config.instance_is_master) Envoyer_commande_dls( NUM_BIT_M_AUDIO_END );
           }
        } else audio_stop = TRUE;

       if (Recv_zmq (zmq_admin, &buffer, sizeof(buffer)) > 0)                             /* As-t'on recu un paquet d'admin ? */
        { gchar *response;
          response = Audio_Admin_response ( buffer );
          Send_zmq ( zmq_admin, response, strlen(response) );
          g_free(response);
        }

       if (Recv_zmq_with_tag ( zmq_master, &buffer, sizeof(buffer), &event, &payload ) > 0) /* Reception d'un paquet master ? */
        { if ( strcmp( event->instance, g_get_host_name() ) && strcmp (event->instance, "*") ) break;
          if ( strcmp( event->thread, NOM_THREAD ) && strcmp ( event->thread, "*" ) ) break;

          Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                   "%s : Reception d'un message du master : %s", __func__, (gchar *)payload );
        }

       if ( Recv_zmq ( zmq_msg, &histo_buf, sizeof(struct CMD_TYPE_HISTO) ) != sizeof(struct CMD_TYPE_HISTO) )
        { sleep(1); continue; }

       histo = &histo_buf;
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                "%s : Recu message num=%d (histo->msg.audio=%d, alive=%d)", __func__,
                histo->msg.num, histo->msg.audio, histo->alive );
                   
       if ( M(NUM_BIT_M_AUDIO_INHIB) == 1 &&
           ! (histo->msg.type == MSG_ALERTE || histo->msg.type == MSG_DANGER || histo->msg.type == MSG_ALARME)
          )                                                                     /* Bit positionné quand arret diffusion audio */
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                   "%s : Envoi audio inhibe pour num=%d (histo->msg.audio=%d)", __func__, histo->msg.num, histo->msg.audio );
          continue;
        }
       if ( histo->alive == 1 && histo->msg.audio )                                                 /* Si le message apparait */
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO,
                   "%s : Envoi du message audio %d (histo->msg.audio=%d)", __func__, histo->msg.num, histo->msg.audio );

          if (Config.instance_is_master)
           { Envoyer_commande_dls( histo->msg.bit_audio );                   /* Positionnement du profil audio via monostable */
             Envoyer_commande_dls( NUM_BIT_M_AUDIO_START );                  /* Positionné quand on envoi une diffusion audio */
           }

          if (Cfg_audio.last_audio + AUDIO_JINGLE < Partage->top)                              /* Si Pas de message depuis xx */
           { Jouer_wav("Son/jingle.wav"); }                                                         /* On balance le jingle ! */
          Cfg_audio.last_audio = Partage->top;

          if (Jouer_mp3 ( &histo->msg ) == FALSE)                      /* Par priorité : mp3 d'abord, synthèse vocale ensuite */
           { if (strlen(histo->msg.libelle_audio))          /* Si libelle_audio, le jouer, sinon jouer le libelle tout court) */
              { Jouer_google_speech( histo->msg.libelle_audio ); }
             else
              { Jouer_google_speech( histo->msg.libelle ); }
           }
        }
       else
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                   "%s : Msg Audio non envoye num=%d (histo->msg.audio=%d)", __func__, histo->msg.num, histo->msg.audio );
        }
     }
    Close_zmq ( zmq_msg );
    Close_zmq ( zmq_master );
    Close_zmq ( zmq_admin );
end:
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    Cfg_audio.lib->Thread_run = FALSE;                                                          /* Le thread ne tourne plus ! */
    Cfg_audio.lib->TID = 0;                                                   /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
