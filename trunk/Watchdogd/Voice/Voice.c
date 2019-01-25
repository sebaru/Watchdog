/******************************************************************************************************************************/
/* Watchdogd/Voice/Voice.c        Gestion des ordres vocaux de Watchdog v2.0                                                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    29.12.2018 22:06:26 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Voice.c
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
 #include <string.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/wait.h>
 #include <signal.h>
 #include <fcntl.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Voice.h"

/******************************************************************************************************************************/
/* Voice_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                  */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Voice_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_voice.lib->Thread_debug = FALSE;                                                       /* Settings default parameters */
    Cfg_voice.enable            = FALSE; 
    g_snprintf( Cfg_voice.audio_device,  sizeof(Cfg_voice.audio_device),  "default" );
    g_snprintf( Cfg_voice.key_words,     sizeof(Cfg_voice.key_words),     "dis moi jolie maison" );
    g_snprintf( Cfg_voice.gain_control,  sizeof(Cfg_voice.gain_control),  "none" );
    g_snprintf( Cfg_voice.vad_threshold, sizeof(Cfg_voice.vad_threshold), "3.0" );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ))                            /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_INFO,                                           /* Print Config */
                "%s: '%s' = %s", __func__, nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_voice.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "audio_device" ) )
        { g_snprintf( Cfg_voice.audio_device, sizeof(Cfg_voice.audio_device), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "key_words" ) )
        { g_snprintf( Cfg_voice.key_words, sizeof(Cfg_voice.key_words), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "gain_control" ) )
        { g_snprintf( Cfg_voice.gain_control, sizeof(Cfg_voice.gain_control), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "vad_threshold" ) )
        { g_snprintf( Cfg_voice.vad_threshold, sizeof(Cfg_voice.vad_threshold), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_voice.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Voice_Make_pulseaudio_file : Création du fichier de connexion pulseaudio                                                   */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Voice_Make_pulseaudio_file ( void )
  { gchar chaine[128];
    gchar file[128];
    gint id_fichier;

    g_snprintf( file, sizeof(file), ".pulse/client.conf" );
    unlink(file);
    id_fichier = open( file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id_fichier<0 || lockf( id_fichier, F_TLOCK, 0 ) )
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_WARNING, "%s: Open file '%s' for write failed (%s)", __func__,
                 file, strerror(errno) );
       close(id_fichier);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "default-server=/run/user/%d/pulse/native", getuid() );
    if (write( id_fichier, chaine, strlen(chaine) )<0)
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR, "%s: Write to file '%s' failed (%s)", __func__, 
                 file, strerror(errno) );
       close(id_fichier);
       return;
     }
    close(id_fichier);
  }
/******************************************************************************************************************************/
/* Voice_Make_jsgf_grammaire : Lit tous les mnemos du thread et les places dans la grammaire                                  */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Voice_Make_jsgf_grammaire ( void )
  { gchar *debut="#JSGF V1.0 UTF-8;\n\ngrammar watchdog.fr;\n\n<evenement> = ";
    struct CMD_TYPE_MNEMO_BASE *mnemo;
    gchar *file="wtd.gram";
    gchar chaine[128];
    gint id_fichier;
    struct DB *db;

    if ( ! Recuperer_mnemo_baseDB_by_thread ( &db, NOM_THREAD ) )
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR,
                 "%s: Error searching Database for '%s'", __func__, NOM_THREAD );
       return;
     }

    unlink(file);
    id_fichier = open( file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id_fichier<0 || lockf( id_fichier, F_TLOCK, 0 ) )
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_WARNING, "%s: Open file '%s' for write failed (%s)", __func__,
                 file, strerror(errno) );
       Libere_DB_SQL ( &db );
       close(id_fichier);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "#JSGF V1.0 UTF-8;\n\ngrammar watchdog.fr;\n\n" );
    if (write( id_fichier, chaine, strlen(chaine) )<0)
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR, "%s: Write to file '%s' failed (%s)", __func__, 
                 file, strerror(errno) );
       Libere_DB_SQL ( &db );
       close(id_fichier);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "<debut> = %s;\n\n", Cfg_voice.key_words );
    write( id_fichier, chaine, strlen(chaine) );

    g_snprintf(chaine, sizeof(chaine), "<evenement> = %s", QUELLE_VERSION );
    write( id_fichier, chaine, strlen(chaine) );

    while ( (mnemo = Recuperer_mnemo_baseDB_suite( &db )) != NULL)
     { write( id_fichier, "\n |", 3 );
       write( id_fichier, mnemo->ev_text, strlen(mnemo->ev_text) );
       g_free(mnemo);
     }
    g_snprintf(chaine, sizeof(chaine), ";\n public <phrase>= <debut> <evenement>;\n" );
    write( id_fichier, chaine, strlen(chaine) );
    close(id_fichier);
  }
/******************************************************************************************************************************/
/* Voice_Jouer_mp3 : Joue un fichier mp3 et attend la fin de la diffusion                                                     */
/* Entrée : le message à jouer                                                                                                */
/* Sortie : True si OK, False sinon                                                                                           */
/******************************************************************************************************************************/
 static gboolean Voice_Jouer_mp3 ( gchar *texte )
  { gchar nom_fichier[128];
    gint fd_cible, pid;

    g_snprintf( nom_fichier, sizeof(nom_fichier), "Son/%s.mp3", texte );
    fd_cible = open ( nom_fichier, O_RDONLY, 0 );
    if (fd_cible < 0) { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_WARNING,
                                  "%s: '%s' not found", __func__, nom_fichier );
                        return(FALSE);
                      }
    else close (fd_cible);

    Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_INFO, "Voice_Jouer_mp3: Send '%s'", nom_fichier );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR,
                 "%s: '%s' fork failed pid=%d (%s)", __func__, nom_fichier, pid, strerror(errno) );
       return(FALSE);
     }
    else if (!pid)
     { execlp( "mpg123", "mpg123", "-o", "pulse", "-q", nom_fichier, NULL );
       Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR,
                "%s: '%s' exec failed pid=%d (%s)", __func__, nom_fichier, pid, strerror( errno ) );
       _exit(0);
     }
    else
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_DEBUG,
                "%s: '%s' waiting to finish pid=%d", __func__, nom_fichier, pid );
       waitpid(pid, NULL, 0 );
     }
    Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_DEBUG, "%s: MPG123 '%s' finished pid=%d", __func__, nom_fichier, pid );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Voice_Jouer_google_speech : Joue un texte avec google_speech et attend la fin de la diffusion                              */
/* Entrée : le message à jouer                                                                                                */
/* Sortie : True si OK, False sinon                                                                                           */
/******************************************************************************************************************************/
 static gboolean Voice_Jouer_google_speech ( gchar *libelle_audio )
  { gint pid;

    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR,
                 "%s: '%s' fork failed pid=%d (%s)", __func__, libelle_audio, pid, strerror(errno) );
       return(FALSE);
     }
    else if (!pid)
     { execlp( "google_speech", "google_speech", "-v", "debug", "-l", "fr", libelle_audio, NULL );
       Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR,
                "%s: '%s' exec failed pid=%d (%s)", __func__, libelle_audio, pid, strerror( errno ) );
       _exit(0);
     }
    else
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_DEBUG,
                "%s: '%s' waiting to finish pid=%d", __func__, libelle_audio, pid );
       waitpid(pid, NULL, 0 );
     }
    Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_DEBUG,
             "%s: google_speech '%s' finished pid=%d", __func__, libelle_audio, pid );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un client et un utilisateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct CMD_TYPE_HISTO *histo, histo_buf;
    gchar commande_vocale[256], *evenement;
    gint pipefd[2], pidpocket;
    struct timeval tv;

reload:
    prctl(PR_SET_NAME, "W-VOICE", 0, 0, 0 );
    memset( &Cfg_voice, 0, sizeof(Cfg_voice) );                                     /* Mise a zero de la structure de travail */
    Cfg_voice.lib = lib;                                           /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_voice.lib->TID = pthread_self();                                                    /* Sauvegarde du TID pour le pere */
    Voice_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_NOTICE,
              "%s: Demarrage %s . . . TID = %p (thread %s)", __func__, VERSION, pthread_self(), NOM_THREAD );
    Cfg_voice.lib->Thread_run = TRUE;                                                                   /* Le thread tourne ! */

    g_snprintf( Cfg_voice.lib->admin_prompt, sizeof(Cfg_voice.lib->admin_prompt), NOM_THREAD );
    g_snprintf( Cfg_voice.lib->admin_help,   sizeof(Cfg_voice.lib->admin_help),   "Manage VOICE system" );

    if (!Cfg_voice.enable)
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_NOTICE,
                "%s: Thread is not enabled in config. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }
    Voice_Make_pulseaudio_file();
    Voice_Make_jsgf_grammaire();
/********************************************* Création du process de reconnaissance vocale ***********************************/
    pipe(pipefd);                                                           /* Création de 2 File Descriptor : 0=Read 1=Write */

    pidpocket = fork();
    if (pidpocket<0)
     { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR, "%s_Fils: Erreur Fork PocketSphinx. Aborting" );
       goto end;
     }
    else if (!pidpocket)
     { gint cpt_fd;
       close(pipefd[0]);                                                               /* this descriptor is no longer needed */
       for (cpt_fd = getdtablesize(); cpt_fd>2; cpt_fd--)                            /* Fermeture de tous les FD avant execve */
        { if (cpt_fd!=pipefd[1]) close(cpt_fd); }
       Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_DEBUG, "%s: Starting PocketSphinx (pid %d) !", __func__, getpid() );
       dup2(pipefd[1], 1);  /* Send stdout to the pipe (back to father !) */
       dup2(pipefd[1], 2);  /* Send stderr to the pipe (back to father !) */
       execlp( "pocketsphinx_continuous", "pocketsphinx_continuous", "-adcdev", Cfg_voice.audio_device,
               "-inmic", "yes", "-agc", Cfg_voice.gain_control, "-logfn", "pocket.log",
               "-vad_threshold", Cfg_voice.vad_threshold,
               "-dict", "fr.dict", "-jsgf", "wtd.gram", "-hmm", "cmusphinx-fr-5.2", NULL );
       Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR, "%s_Fils: lancement PocketSphinx failed", __func__ );
       _exit(0);
     }

    close(pipefd[1]);  // close the write end of the pipe in the parent

    if (Config.instance_is_master==FALSE)                                                          /* si l'instance est Slave */
     { Cfg_voice.zmq_to_master = New_zmq ( ZMQ_PUB, "pub-to-master" );
       Connect_zmq ( Cfg_voice.zmq_to_master, "inproc", ZMQUEUE_LIVE_MASTER, 0 );
     }

    while ( Cfg_voice.lib->Thread_run == TRUE )
     { gchar mute[128];
       struct DB *db;
       gint retour;
       fd_set fd;

       if (Cfg_voice.lib->Thread_reload)                                                      /* A-t'on recu un signal USR1 ? */
        { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_INFO, "%s: RELOADING", __func__ );
          break;
        }

       tv.tv_sec=5;
       tv.tv_usec=0;
       FD_ZERO(&fd);
       FD_SET( pipefd[0], &fd );
       retour = select( pipefd[0]+1, &fd, NULL, NULL, &tv );
       if (retour==-1) break;
       if (retour==0)
        { sched_yield();
          usleep(1000);
          continue;
        }
       retour = read(pipefd[0], commande_vocale, sizeof(commande_vocale));
       if (retour<=0)
        { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_WARNING,
                    "%s: recu error (ret=%d) restarting in 5s", __func__, retour );
          sleep(5);
          Cfg_voice.lib->Thread_reload = TRUE;
          continue;
        }
       commande_vocale[retour-1]=0;                                                                 /* Caractere NULL d'arret */
       evenement = commande_vocale + strlen(Cfg_voice.key_words) + 1;

       Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_NOTICE, "%s: recu = '%s'. Searching...", __func__, evenement );

       /*g_snprintf( mute, sizeof(mute), "pactl set-source-mute %s 1", Cfg_voice.audio_device );
       system(mute);*/

       if (!strcmp( QUELLE_VERSION, evenement ))
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), "Ma version est la %s", PACKAGE_VERSION );
          Voice_Jouer_google_speech ( chaine );
          continue;
        }

       if ( ! Recuperer_mnemo_baseDB_by_event_text ( &db, NOM_THREAD, evenement ) )
        { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR,
                    "%s: Error searching Database for '%s'", __func__, evenement );
          continue;
        }
          
       if ( db->nbr_result == 0 )                                                           /* Si pas d'enregistrement trouvé */
        { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_WARNING,
                    "%s: No match found for '%s'", __func__, evenement );
          Libere_DB_SQL ( &db );
          Voice_Jouer_mp3 ( "Je_ne_sais_pas_faire" );
        }
       else if (db->nbr_result > 1)
        { Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_WARNING,
                    "%s: Too many event for '%s'", __func__, evenement );
          Libere_DB_SQL ( &db );
          Voice_Jouer_mp3 ( "C_est_ambigue" );
        }
       else
        { struct CMD_TYPE_MNEMO_BASE *mnemo;
          while ( (mnemo = Recuperer_mnemo_baseDB_suite( &db )) != NULL)
           { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Match found for '%s' Type %d Num %d - %s", __func__,
                       commande_vocale, mnemo->type, mnemo->num, mnemo->libelle );
             Voice_Jouer_mp3 ( "C_est_parti" );
             if (Config.instance_is_master==TRUE)                                                 /* si l'instance est Maitre */
              { switch( mnemo->type )
                 { case MNEMO_MONOSTABLE:
                        Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_NOTICE,
                                  "%s: '%s' -> Mise à un du bit '%s:%s'", __func__,
                                  evenement, mnemo->dls_tech_id, mnemo->acronyme );
                        Envoyer_commande_dls_data ( mnemo->dls_tech_id, mnemo->acronyme );
                        break;
                   default:
                        Info_new( Config.log, Config.log_msrv, LOG_ERR,
                                  "%s: '%s' -> Error, type of mnemo not handled", __func__, evenement );
                 }
              }
             else /* Envoi au master via thread HTTP */
              { if (mnemo->type == MNEMO_MONOSTABLE)
                 { struct ZMQ_SET_BIT bit;
                   bit.type = mnemo->type;
                   bit.num = mnemo->num;
                   g_snprintf( bit.dls_tech_id, sizeof(bit.dls_tech_id), "%s", mnemo->dls_tech_id );
                   g_snprintf( bit.acronyme, sizeof(bit.acronyme), "%s", mnemo->acronyme );
                   Send_zmq_with_tag ( Cfg_voice.zmq_to_master, TAG_ZMQ_SET_BIT, NULL, NOM_THREAD, "*", "*",
                                       &bit, sizeof(struct ZMQ_SET_BIT) );
                 }
                else Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_ERR,
                              "%s: '%s' -> Error, type of mnemo not handled", __func__, evenement );
              }
             g_free(mnemo);
           }
        }
     }
    Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_INFO, "%s: Sending kill to pocketsphinx", __func__ );
    kill(pidpocket, SIGKILL);
    Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_INFO, "%s: Waiting for termination", __func__ );
    waitpid(pidpocket, NULL, 0);
    close(pipefd[0]);                                                                       /* Fermeture du pipe en reception */
end:
    Close_zmq ( Cfg_voice.zmq_to_master );

    Info_new( Config.log, Cfg_voice.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    if (Cfg_voice.lib->Thread_run && Cfg_voice.lib->Thread_reload)
     { Cfg_voice.lib->Thread_reload = FALSE;
       goto reload;
     }

    Cfg_voice.lib->Thread_run = FALSE;
    Cfg_voice.lib->TID = 0;                                                     /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
