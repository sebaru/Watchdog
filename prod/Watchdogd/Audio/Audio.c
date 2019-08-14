/******************************************************************************************************************************/
/* Watchdogd/Audio/Audio.c  Gestion des messages audio de Watchdog 2.0                                                        */
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
    g_snprintf( Cfg_audio.device,   sizeof(Cfg_audio.device), "plughw" );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO, "%s: '%s' = %s", __func__, nom, valeur ); /* Print Config */
            if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_audio.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_audio.lib->Thread_debug = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "device" ) )
        { g_snprintf( Cfg_audio.device, sizeof(Cfg_audio.device), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "language" ) )
        { g_snprintf( Cfg_audio.language, sizeof(Cfg_audio.language), "%s", valeur ); }
       else
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Jouer_wav: Jouer un fichier wav dont le nom est en paramètre                                                               */
/* Entrée : le nom du fichier wav                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static gboolean Jouer_wav_by_file ( gchar *texte )
  { gint fd_cible, pid;
    gchar fichier[80];

    g_snprintf( fichier, sizeof(fichier), "Son/%s.wav", texte );
    fd_cible = open ( fichier, O_RDONLY, 0 );
    if (fd_cible < 0 && Config.instance_is_master == FALSE)
     { gchar chaine[80];
       g_snprintf(chaine, sizeof(chaine), "wget http://%s:5560/audio/%s -O %s", Config.master_host, texte, fichier );
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                 "%s: '%s' not found trying down from master '%s'", __func__, fichier, chaine );
       system(chaine);
       fd_cible = open ( fichier, O_RDONLY, 0 );
       if (fd_cible < 0)
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                    "%s: '%s' not found (even after download)", __func__, fichier );
          return(FALSE);
        }
     }
    else if (fd_cible < 0 && Config.instance_is_master == TRUE)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR, "%s: '%s' not found", __func__, fichier );
       return(FALSE);
     }
    else close (fd_cible);

    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO, "%s: Envoi d'un wav %s", __func__, fichier );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                "%s: APLAY '%s' fork failed pid=%d", __func__, fichier, pid );
       return(FALSE);
     }
    else if (!pid)
     { execlp( "aplay", "aplay", "--device", Cfg_audio.device, fichier, NULL );
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                "%s: APLAY '%s' exec failed pid=%d", __func__, fichier, pid );
       _exit(0);
     }
    else
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                "%s: APLAY '%s' waiting to finish pid=%d", __func__, fichier, pid );
       waitpid(pid, NULL, 0 );
     }
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "%s: APLAY '%s' finished pid=%d", __func__, fichier, pid );
    Cfg_audio.nbr_diffusion_wav++;
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Jouer_wav: Jouer un fichier wav dont le nom est en paramètre                                                               */
/* Entrée : le nom du fichier wav                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static gboolean Jouer_wav_by_id ( struct CMD_TYPE_MESSAGE *msg )
  { gchar nom_fichier[80];

    g_snprintf( nom_fichier, sizeof(nom_fichier), "%d", msg->num );
    return(Jouer_wav_by_file( nom_fichier ) );
  }
/******************************************************************************************************************************/
/* Jouer_google_speech : Joue un texte avec google_speech et attend la fin de la diffusion                                    */
/* Entrée : le message à jouer                                                                                                */
/* Sortie : True si OK, False sinon                                                                                           */
/******************************************************************************************************************************/
 gboolean Jouer_google_speech ( gchar *libelle_audio )
  { gint pid;

    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE, "%s: Send '%s'", __func__, libelle_audio );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                 "%s: '%s' fork failed pid=%d (%s)", __func__, libelle_audio, pid, strerror(errno) );
       return(FALSE);
     }
    else if (!pid)
     { execlp( "Wtd_play_google.sh", "Wtd_play_google", Cfg_audio.language, libelle_audio, Cfg_audio.device, NULL );
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR,
                "%s: '%s' exec failed pid=%d (%s)", __func__, libelle_audio, pid, strerror( errno ) );
       _exit(0);
     }
    else
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                "%s: '%s' waiting to finish pid=%d", __func__, libelle_audio, pid );
       waitpid(pid, NULL, 0 );
     }
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "%s: Wtd_play_google %s '%s' %s finished pid=%d", __func__,
              Cfg_audio.language, libelle_audio, Cfg_audio.device, pid );
    Cfg_audio.nbr_diffusion_google++;
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du Thread Audio                                                                                  */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct CMD_TYPE_HISTO *histo, histo_buf;
    struct ZMQUEUE *zmq_msg;
    struct ZMQUEUE *zmq_from_bus;
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

    zmq_msg      = Connect_zmq ( ZMQ_SUB, "listen-to-msgs", "inproc", ZMQUEUE_LIVE_MSGS, 0 );
    zmq_from_bus = Connect_zmq ( ZMQ_SUB, "listen-to-bus",  "inproc", ZMQUEUE_LOCAL_BUS, 0 );

    while(Cfg_audio.lib->Thread_run == TRUE)                                                 /* On tourne tant que necessaire */
     { struct ZMQ_TARGET *event;
       gchar buffer[256];
       void *payload;

       if (Cfg_audio.lib->Thread_reload)                                                             /* On a recu reload ?? */
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE, "%s: SIGUSR1", __func__ );
          Audio_Lire_config();
          Cfg_audio.lib->Thread_reload = FALSE;
        }

       if (Cfg_audio.last_audio + 100 < Partage->top)                                /* Au bout de 10 secondes sans diffusion */
        { if (audio_stop == TRUE)                                /* Avons-nous deja envoyé une commande de STOP AUDIO a DLS ? */
           { audio_stop = FALSE;                                         /* Positionné quand il n'y a plus de diffusion audio */
             if (Config.instance_is_master) Envoyer_commande_dls( NUM_BIT_M_AUDIO_END );
           }
        } else audio_stop = TRUE;

       if (Recv_zmq_with_tag ( zmq_from_bus, NOM_THREAD, &buffer, sizeof(buffer), &event, &payload ) > 0) /* Reception d'un paquet master ? */
        { if ( !strcmp( event->tag, "play_wav" ) )
           { gchar fichier[80];
             Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                      "%s : Reception d'un message PLAY_WAV : %s", __func__, (gchar *)payload );
             g_snprintf( fichier, sizeof(fichier), "%s", (gchar *)payload );
             Jouer_wav_by_file ( fichier );
           }
          else if ( !strcmp( event->tag, "play_google" ) )
           { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                      "%s : Reception d'un message PLAY_GOOGLE : %s", __func__, (gchar *)payload );
             Jouer_google_speech ( payload );
           }
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
           { Jouer_wav_by_file("jingle"); }                                                         /* On balance le jingle ! */
          Cfg_audio.last_audio = Partage->top;

          if (Jouer_wav_by_id ( &histo->msg ) == FALSE)                /* Par priorité : wav d'abord, synthèse vocale ensuite */
           { if (strlen(histo->msg.libelle_audio))          /* Si libelle_audio, le jouer, sinon jouer le libelle tout court) */
              { Jouer_google_speech( histo->msg.libelle_audio ); }
             else
              { Jouer_google_speech( histo->msg.libelle ); }
           }
        }
     }
    Close_zmq ( zmq_msg );
    Close_zmq ( zmq_from_bus );
end:
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    Cfg_audio.lib->Thread_run = FALSE;                                                          /* Le thread ne tourne plus ! */
    Cfg_audio.lib->TID = 0;                                                   /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
