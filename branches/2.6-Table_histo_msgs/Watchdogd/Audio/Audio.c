/**********************************************************************************************************/
/* Watchdogd/Audio/Audio.c  Gestion des messages audio de Watchdog 2.0                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                     sam. 09 nov. 2013 13:49:53 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 #include "Audio.h"
/**********************************************************************************************************/
/* Audio_Lire_config : Lit la config Watchdog et rempli la structure mémoire                              */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 gboolean Audio_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_audio.lib->Thread_debug = FALSE;                                   /* Settings default parameters */
    Cfg_audio.enable            = FALSE; 

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                      /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                "Audio_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )       /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO,                         /* Print Config */
                "Audio_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_audio.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_audio.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE,
                   "Audio_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_audio: Ajoute un message audio a prononcer                                                     */
/* Entrées: le numéro du message a prononcer                                                              */
/**********************************************************************************************************/
 void Audio_Gerer_histo( struct CMD_TYPE_HISTO *histo )
  { gint taille;

    if ( ! histo->msg.bit_voc ) { g_free(histo); return; }               /* Si flag = 0; on return direct */

    pthread_mutex_lock( &Cfg_audio.lib->synchro );              /* Ajout dans la liste de audio a traiter */
    taille = g_slist_length( Cfg_audio.Liste_histos );
    pthread_mutex_unlock( &Cfg_audio.lib->synchro );


    if (taille > 150)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                 "Ajouter_audio: DROP audio %d (taille = %d > 150)", histo->msg.num, taille);
       g_free(histo);
       return;
     }
    else if (Cfg_audio.lib->Thread_run == FALSE)
     { Info_new( Config.log, Config.log_arch, LOG_INFO,
                "Ajouter_audio: Thread is down. Dropping msg %d", histo->msg.num );
       g_free(histo);
       return;
     }

    pthread_mutex_lock( &Cfg_audio.lib->synchro );           /* Ajout dans la liste de audio a traiter */
    Cfg_audio.Liste_histos = g_slist_append( Cfg_audio.Liste_histos, histo );
    pthread_mutex_unlock( &Cfg_audio.lib->synchro );
  }
/**********************************************************************************************************/
/* Jouer_wav: Jouer un fichier wav dont le nom est en paramètre                                           */
/* Entrée : le nom du fichier wav                                                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Jouer_wav ( gchar *fichier )
  { gint pid;

    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO, "Jouer_wav: Envoi d'un wav %s", fichier );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING, "Jouer_wav: APLAY fork failed pid=%d", pid ); }
    else if (!pid)
     { execlp( "aplay", "aplay", "-R", "1", fichier, NULL );
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING, "Jouer_wav: Lancement APLAY failed pid=%d", pid );
       _exit(0);
     }
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "Jouer_wav: waiting for APLAY to finish pid=%d", pid );
    waitpid(pid, NULL, 0 );
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "Jouer_wav: APLAY finished pid=%d", pid );
  }
/**********************************************************************************************************/
/* Jouer_mp3 : Joue un fichier mp3 et attend la fin de la diffusion                                       */
/* Entrée : le message à jouer                                                                            */
/* Sortie : True si OK, False sinon                                                                       */
/**********************************************************************************************************/
 gboolean Jouer_mp3 ( struct CMD_TYPE_MESSAGE *msg )
  { gchar nom_fichier[128];
    gint fd_cible, pid;

    g_snprintf( nom_fichier, sizeof(nom_fichier), "Son/%d.mp3", msg->num );
    fd_cible = open ( nom_fichier, O_RDONLY, 0 );
    if (fd_cible < 0) { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                                  "Jouer_mp3: fichier %s non trouve", nom_fichier );
                        return(FALSE);
                      }
    else close (fd_cible);

    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO, "Jouer_mp3: Envoi du mp3 %s", nom_fichier );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                 "Jouer_mp3: MPG123 fork failed pid=%d (%s)", pid, strerror(errno) );
     }
    else if (!pid)
     { execlp( "mpg123", "mpg123", "-vvvv", nom_fichier, NULL );
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING,
                "Jouer_mp3: Lancement MPG123 failed (%s)", strerror( errno ) );
       _exit(0);
     }
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "Jouer_mp3: waiting for MPG123 to finish pid=%d", pid );
    waitpid(pid, NULL, 0 );
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "Jouer_mp3: MPG123 finished pid=%d", pid );

    return(TRUE);
  }
/**********************************************************************************************************/
/* Jouer_espeak : Joue un message via synthèse vocale et attend la fin de la diffusion                    */
/* Entrée : le message à jouer                                                                            */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void Jouer_espeak ( struct CMD_TYPE_MESSAGE *msg )
  { gchar nom_fichier[128], cible[128];
    gint fd_cible, pid, num;

    g_snprintf( nom_fichier, sizeof(nom_fichier), "Son/%d.pho", msg->num );
    unlink( nom_fichier );                                            /* Destruction des anciens fichiers */
    g_snprintf( cible,       sizeof(cible),       "Son/%d.au",  msg->num );
    unlink( cible );                                                  /* Destruction des anciens fichiers */
/***************************************** Création du PHO ************************************************/
    num = msg->num;                                /* Attention, on fork donc plus de mémoire partagée !! */
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO, "Lancement de ESPEAK %d", num );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING, "Fork Fabrication .pho failed pid=%d", pid ); }
    else if (!pid)                                                 /* Création du .au en passant par .pho */
     { gchar texte[80], chaine[30], chaine2[30];
       switch (msg->type_voc)
        { case 0: g_snprintf( chaine, sizeof(chaine), "mb/mb-fr1" ); break;
          case 1: g_snprintf( chaine, sizeof(chaine), "mb/mb-fr4" ); break;
          case 2: g_snprintf( chaine, sizeof(chaine), "mb/mb-fr1" ); break;
          default:
          case 3: g_snprintf( chaine, sizeof(chaine), "mb/mb-fr4" ); break;
        }
       g_snprintf( chaine2, sizeof(chaine2), "%d", msg->vitesse_voc );
       fd_cible = open ( nom_fichier, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
       dup2( fd_cible, 1 );
       g_snprintf( texte, sizeof(texte), "%s", msg->libelle_audio );
       execlp( "espeak", "espeak", "-q", "-s", chaine2, "-v", chaine, texte, NULL );
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING, "Lancement espeak failed pid=%d", pid );
       _exit(0);
     }
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "waiting for espeak to finish pid=%d", pid );
    waitpid(pid, NULL, 0 );
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "espeak finished pid=%d", pid );

/****************************************** Création du AU ************************************************/
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO, "Lancement de MBROLA %d", num );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING, "Fabrication .au failed pid=%d", pid ); }
    else if (!pid)                                                 /* Création du .au en passant par .pho */
     { gchar chaine[30];
       switch (msg->type_voc)
        { case 0: g_snprintf( chaine, sizeof(chaine), "fr1" ); break;
          case 1: g_snprintf( chaine, sizeof(chaine), "fr2" ); break;
          case 2: g_snprintf( chaine, sizeof(chaine), "fr6" ); break;
          default:
          case 3: g_snprintf( chaine, sizeof(chaine), "fr4" ); break;
        }
       execlp( "mbrola-linux-i386", "mbrola-linux-i386", chaine, nom_fichier, cible, NULL );
       Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_WARNING, "Lancement mbrola failed pid=%d", pid );
       _exit(0);
     }
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "waiting for mbrola to finish pid", pid );
    waitpid(pid, NULL, 0 );
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_DEBUG, "mbrola finished pid", pid );
/****************************************** Lancement de l'audio ******************************************/
    Jouer_wav(cible);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du RS485                                                                     */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct CMD_TYPE_HISTO *histo;
    static gboolean audio_stop = TRUE;

    prctl(PR_SET_NAME, "W-Audio", 0, 0, 0 );
    memset( &Cfg_audio, 0, sizeof(Cfg_audio) );                 /* Mise a zero de la structure de travail */
    Cfg_audio.lib = lib;                       /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_audio.lib->TID = pthread_self();                                /* Sauvegarde du TID pour le pere */
    Audio_Lire_config ();                               /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );
    Cfg_audio.lib->Thread_run = TRUE;                                               /* Le thread tourne ! */

    g_snprintf( Cfg_audio.lib->admin_prompt, sizeof(Cfg_audio.lib->admin_prompt), "audio" );
    g_snprintf( Cfg_audio.lib->admin_help,   sizeof(Cfg_audio.lib->admin_help),   "Manage Audio system" );

    if (!Cfg_audio.enable)
     { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread is not enabled in config. Shutting Down %p",
                 pthread_self() );
       goto end;
     }

    Abonner_distribution_histo ( Audio_Gerer_histo );          /* Abonnement de la diffusion des messages */
    while(Cfg_audio.lib->Thread_run == TRUE)                             /* On tourne tant que necessaire */
     {
       if (Cfg_audio.lib->Thread_sigusr1)                                         /* On a recu sigusr1 ?? */
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE, "Run_audio: SIGUSR1" );
          pthread_mutex_lock( &Cfg_audio.lib->synchro );                                 /* lockage futex */
          Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE,
                    "Run_audio: Reste %03d a traiter",
                    g_slist_length(Cfg_audio.Liste_histos) );
          pthread_mutex_unlock( &Cfg_audio.lib->synchro );
          Cfg_audio.lib->Thread_sigusr1 = FALSE;
        }

       if (!Cfg_audio.Liste_histos)                                        /* Si pas de message, on tourne */
        { if (Cfg_audio.last_audio + 100 < Partage->top)         /* Au bout de 10 secondes sans diffusion */
           { if (audio_stop == TRUE)        /* Avons-nous deja envoyé une commande de STOP AUDIO a DLS ?? */
              { audio_stop = FALSE;
                Envoyer_commande_dls( NUM_BIT_M_AUDIO_END );/* Positionné quand il n'y a plus de diffusion audio*/
              }
           } else audio_stop = TRUE;
          sched_yield();
          sleep(1);
          continue;
        }

       pthread_mutex_lock( &Cfg_audio.lib->synchro );                                    /* lockage futex */
       histo = Cfg_audio.Liste_histos->data;                                     /* Recuperation du audio */
       Cfg_audio.Liste_histos = g_slist_remove ( Cfg_audio.Liste_histos, histo );
       pthread_mutex_unlock( &Cfg_audio.lib->synchro );

       if ( histo->alive == 1 &&                                                /* Si le message apparait */
            (M(NUM_BIT_M_AUDIO_INHIB) == 0 || histo->msg.type == MSG_ALERTE
                                           || histo->msg.type == MSG_DANGER 
                                           || histo->msg.type == MSG_ALARME
            )
          )                                                 /* Bit positionné quand arret diffusion audio */
        { Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_INFO,
                   "Run_thread : Envoi du message audio %d", histo->msg.num );

          Envoyer_commande_dls( histo->msg.bit_voc );    /* Positionnement du profil audio via monostable */
          Envoyer_commande_dls( NUM_BIT_M_AUDIO_START ); /* Positionné quand on envoi une diffusion audio */

          if (Cfg_audio.last_audio + AUDIO_JINGLE < Partage->top)          /* Si Pas de message depuis xx */
           { Jouer_wav("Son/jingle.wav"); }                                     /* On balance le jingle ! */
          Cfg_audio.last_audio = Partage->top;

          if ( ! Jouer_mp3 ( &histo->msg ) )       /* Par priorité : mp3 d'abord, synthèse vocale ensuite */
           { Jouer_espeak ( &histo->msg ); }
        }
       g_free(histo);
     }
    Desabonner_distribution_histo ( Audio_Gerer_histo );    /* Desabonnement de la diffusion des messages */

end:
    Info_new( Config.log, Cfg_audio.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_audio.lib->Thread_run = FALSE;                                      /* Le thread ne tourne plus ! */
    Cfg_audio.lib->TID = 0;                               /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
