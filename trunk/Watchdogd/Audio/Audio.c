/******************************************************************************************************************************/
/* Watchdogd/Audio/Audio.c  Gestion des messages audio de Watchdog 2.0                                                        */
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
 #include "Audio.h"
 struct AUDIO_CONFIG Cfg_audio;
/******************************************************************************************************************************/
/* Audio_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                  */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Audio_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_audio.lib->Thread_debug = FALSE;                                                       /* Settings default parameters */
    Cfg_audio.diffusion_enabled = TRUE;
    g_snprintf( Cfg_audio.language, sizeof(Cfg_audio.language), "%s", AUDIO_DEFAUT_LANGUAGE );
    g_snprintf( Cfg_audio.device,   sizeof(Cfg_audio.device), "plughw" );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO, "%s: '%s' = %s", __func__, nom, valeur ); /* Print Config */
            if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
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

reload:
    memset( &Cfg_audio, 0, sizeof(Cfg_audio) );                                     /* Mise a zero de la structure de travail */
    Cfg_audio.lib = lib;                                           /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "W-AUDIO", lib, WTD_VERSION, "Manage Audio System" );
    Audio_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */

    if (Config.instance_is_master)
     { if (Dls_auto_create_plugin( "AUDIO", "Gestion de l'audio diffusion" ) == FALSE)
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_ERR, "%s: DLS Create 'SONO' ERROR\n", __func__ ); }
       Mnemo_auto_create_DI ( FALSE, "AUDIO", "P_ALL", "Profil Audio: All Hps Enabled" );
       Mnemo_auto_create_DI ( FALSE, "AUDIO", "P_NONE", "Profil audio: All Hps disabled" );
     }

    zmq_msg      = Connect_zmq ( ZMQ_SUB, "listen-to-msgs", "inproc", ZMQUEUE_LIVE_MSGS, 0 );
    zmq_from_bus = Connect_zmq ( ZMQ_SUB, "listen-to-bus",  "inproc", ZMQUEUE_LOCAL_BUS, 0 );

    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { struct ZMQ_TARGET *event;
       gchar buffer[256];
       void *payload;

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
          else if ( !strcmp( event->tag, "disable" ) )
           { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE, "%s : Diffusion disabled by master", __func__ );
             Cfg_audio.diffusion_enabled = FALSE;
           }
          else if ( !strcmp( event->tag, "enable" ) )
           { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE, "%s : Diffusion enabled by master", __func__ );
             Cfg_audio.diffusion_enabled = TRUE;
           }
        }

       if ( Recv_zmq ( zmq_msg, &histo_buf, sizeof(struct CMD_TYPE_HISTO) ) != sizeof(struct CMD_TYPE_HISTO) )
        { sleep(1); continue; }

       histo = &histo_buf;
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG,
                "%s : Recu message '%s:%s' (histo->msg.profil_audio=%s, alive=%d)", __func__,
                histo->msg.tech_id, histo->msg.acronyme, histo->msg.profil_audio, histo->alive );

       if ( Cfg_audio.diffusion_enabled == FALSE &&
            ! (histo->msg.type == MSG_ALERTE || histo->msg.type == MSG_DANGER)
          )                                                                     /* Bit positionné quand arret diffusion audio */
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                   "%s : Envoi audio inhibé pour '%s:%s'", __func__, histo->msg.tech_id, histo->msg.acronyme );
          continue;
        }

       if ( histo->alive == 1 && strcasecmp(histo->msg.profil_audio, "P_NONE") )                    /* Si le message apparait */
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO,
                   "%s : Envoi du message audio '%s:%s' (histo->msg.profil_audio=%s)",
                    __func__, histo->msg.tech_id, histo->msg.acronyme, histo->msg.profil_audio);

          if (Config.instance_is_master)
           { Envoyer_commande_dls_data( "AUDIO", histo->msg.profil_audio );  /* Positionnement du profil audio via monostable */
           }

          if (Cfg_audio.last_audio + AUDIO_JINGLE < Partage->top)                              /* Si Pas de message depuis xx */
           { Jouer_wav_by_file("jingle"); }                                                         /* On balance le jingle ! */
          Cfg_audio.last_audio = Partage->top;

          if (strlen(histo->msg.libelle_audio))             /* Si libelle_audio, le jouer, sinon jouer le libelle tout court) */
           { Jouer_google_speech( histo->msg.libelle_audio ); }
          else
           { Jouer_google_speech( histo->msg.libelle ); }

          if (Config.instance_is_master)
           { Envoyer_commande_dls_data( "AUDIO", "P_NONE" );                                 /* Bit de fin d'emission message */
           }

        }
     }
    Close_zmq ( zmq_msg );
    Close_zmq ( zmq_from_bus );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
