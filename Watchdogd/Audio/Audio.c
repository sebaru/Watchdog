/**********************************************************************************************************/
/* Watchdogd/Audio/Audio.c  Gestion des messages audio de Watchdog 2.0                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 11 avr 2009 12:08:12 CEST */
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
 
 #include <glib.h>
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/wait.h>
 #include <fcntl.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
/**********************************************************************************************************/
/* Ajouter_audio: Ajoute un message audio a prononcer                                                     */
/* Entrées: le numéro du message a prononcer                                                              */
/**********************************************************************************************************/
 void Ajouter_audio( gint num )
  { gint taille;

    pthread_mutex_lock( &Partage->com_audio.synchro );          /* Ajout dans la liste de audio a traiter */
    taille = g_list_length( Partage->com_audio.liste_audio );
    pthread_mutex_unlock( &Partage->com_audio.synchro );

    if (taille > 150)
     { Info_new( Config.log, FALSE, LOG_WARNING, "Ajouter_audio: DROP audio (taille>150) %d", num);
       return;
     }

    pthread_mutex_lock( &Partage->com_audio.synchro );           /* Ajout dans la liste de audio a traiter */
    Partage->com_audio.liste_audio = g_list_append( Partage->com_audio.liste_audio, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_audio.synchro );
  }
/**********************************************************************************************************/
/* Jouer_wav: Jouer un fichier wav dont le nom est en paramètre                                           */
/* Entrée : le nom du fichier wav                                                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Jouer_wav ( gchar *fichier )
  { gint pid;

    Info_new( Config.log, FALSE, LOG_INFO, "Jouer_wav: Envoi d'un wav %s", fichier );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, FALSE, LOG_WARNING, "Jouer_wav: APLAY fork failed pid=%d", pid ); }
    else if (!pid)
     { execlp( "aplay", "aplay", "-R", "1", fichier, NULL );
       Info_new( Config.log, FALSE, LOG_WARNING, "Jouer_wav: Lancement APLAY failed pid=%d", pid );
       _exit(0);
     }
    Info_new( Config.log, FALSE, LOG_DEBUG, "Jouer_wav: waiting for APLAY to finish pid=%d", pid );
    wait4(pid, NULL, 0, NULL );
    Info_new( Config.log, FALSE, LOG_DEBUG, "Jouer_wav: APLAY finished pid=%d", pid );
  }
/**********************************************************************************************************/
/* Jouer_mp3 : Joue un fichier mp3 et attend la fin de la diffusion                                       */
/* Entrée : le message à jouer                                                                            */
/* Sortie : True si OK, False sinon                                                                       */
/**********************************************************************************************************/
 static gboolean Jouer_mp3 ( struct CMD_TYPE_MESSAGE *msg )
  { gchar nom_fichier[128];
    gint fd_cible, pid;

    g_snprintf( nom_fichier, sizeof(nom_fichier), "Son/%d.mp3", msg->num );
    fd_cible = open ( nom_fichier, O_RDONLY, 0 );
    if (fd_cible < 0) { Info_new( Config.log, FALSE, LOG_WARNING,
                                  "Jouer_mp3: fichier %s non trouve", nom_fichier );
                        return(FALSE);
                      }
    else close (fd_cible);

    Info_new( Config.log, FALSE, LOG_INFO, "Jouer_mp3: Envoi du mp3 %s", nom_fichier );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, FALSE, LOG_WARNING, "Jouer_mp3: MPG123 fork failed pid=%d", pid ); }
    else if (!pid)
     { execlp( "mpg123", "mpg123", "-q", nom_fichier, NULL );
       Info_new( Config.log, FALSE, LOG_WARNING, "Jouer_mp3: Lancement MPG123 failed pid=%d", pid );
       _exit(0);
     }
    Info_new( Config.log, FALSE, LOG_DEBUG, "Jouer_mp3: waiting for MPG123 to finish pid=%d", pid );
    wait4(pid, NULL, 0, NULL );
    Info_new( Config.log, FALSE, LOG_DEBUG, "Jouer_mp3: MPG123 finished pid=%d", pid );

    return(TRUE);
  }
/**********************************************************************************************************/
/* Jouer_espeak : Joue un message via synthèse vocale et attend la fin de la diffusion                    */
/* Entrée : le message à jouer                                                                            */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Jouer_espeak ( struct CMD_TYPE_MESSAGE *msg )
  { gchar nom_fichier[128], cible[128];
    gint fd_cible, pid, num;

    g_snprintf( nom_fichier, sizeof(nom_fichier), "Son/%d.pho", msg->num );
    unlink( nom_fichier );                                            /* Destruction des anciens fichiers */
    g_snprintf( cible,       sizeof(cible),       "Son/%d.au",  msg->num );
    unlink( cible );                                                  /* Destruction des anciens fichiers */
/***************************************** Création du PHO ************************************************/
    num = msg->num;                                /* Attention, on fork donc plus de mémoire partagée !! */
    Info_new( Config.log, FALSE, LOG_INFO, "Lancement de ESPEAK %d", num );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, FALSE, LOG_WARNING, "Fork Fabrication .pho failed pid=%d", pid ); }
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
       Info_new( Config.log, FALSE, LOG_WARNING, "Lancement espeak failed pid=%d", pid );
       _exit(0);
     }
    Info_new( Config.log, FALSE, LOG_DEBUG, "waiting for espeak to finish pid=%d", pid );
    wait4(pid, NULL, 0, NULL );
    Info_new( Config.log, FALSE, LOG_DEBUG, "espeak finished pid=%d", pid );

/****************************************** Création du AU ************************************************/
    Info_new( Config.log, FALSE, LOG_INFO, "Lancement de MBROLA %d", num );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, FALSE, LOG_WARNING, "Fabrication .au failed pid=%d", pid ); }
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
       Info_new( Config.log, FALSE, LOG_WARNING, "Lancement mbrola failed pid=%d", pid );
       _exit(0);
     }
    Info_new( Config.log, FALSE, LOG_DEBUG, "waiting for mbrola to finish pid", pid );
    wait4(pid, NULL, 0, NULL );
    Info_new( Config.log, FALSE, LOG_DEBUG, "mbrola finished pid", pid );
/****************************************** Lancement de l'audio ******************************************/
    Jouer_wav(cible);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du RS485                                                                     */
/**********************************************************************************************************/
 void Run_audio ( void )
  { struct CMD_TYPE_MESSAGE *msg;
    struct DB *db;
    guint num;
    prctl(PR_SET_NAME, "W-Audio", 0, 0, 0 );

    Info_new( Config.log, FALSE, LOG_NOTICE,
              "Run_audio: Demarrage . . . TID = %d", pthread_self() );

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, FALSE, LOG_CRIT, "Run_audio: Unable to open database %s", Config.db_database );
       Partage->com_audio.TID = 0;                        /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }

    Partage->com_audio.Thread_run = TRUE;                                           /* Le thread tourne ! */
    while(Partage->com_audio.Thread_run == TRUE)                         /* On tourne tant que necessaire */
     {
       if (Partage->com_audio.Thread_reload)                                      /* On a recu sigusr1 ?? */
        { Info_new( Config.log, FALSE, LOG_NOTICE, "Run_audio: RELOAD" );
          Partage->com_audio.Thread_reload = FALSE;
        }

       if (Partage->com_audio.Thread_sigusr1)                                     /* On a recu sigusr1 ?? */
        { Info_new( Config.log, FALSE, LOG_NOTICE, "Run_audio: SIGUSR1" );
          pthread_mutex_lock( &Partage->com_audio.synchro );                             /* lockage futex */
          Info_new( Config.log, FALSE, LOG_NOTICE,
                    "Run_audio: Reste a traiter %d",
                    g_list_length(Partage->com_audio.liste_audio) );
          pthread_mutex_unlock( &Partage->com_audio.synchro );
          Partage->com_audio.Thread_sigusr1 = FALSE;
        }

       if (!Partage->com_audio.liste_audio)                               /* Si pas de message, on tourne */
        { sched_yield();
          sleep(1);
          Envoyer_commande_dls( NUM_BIT_M_AUDIO_END );/* Positionné quand il n'y a plus de diffusion audio*/
          continue;
        }

       pthread_mutex_lock( &Partage->com_audio.synchro );                                /* lockage futex */
       num = GPOINTER_TO_INT(Partage->com_audio.liste_audio->data);              /* Recuperation du audio */
       Partage->com_audio.liste_audio = g_list_remove ( Partage->com_audio.liste_audio,
                                                        GINT_TO_POINTER(num) );
       pthread_mutex_unlock( &Partage->com_audio.synchro );

       Info_new( Config.log, FALSE, LOG_INFO, "Préparation du message id %d", num );

       msg = Rechercher_messageDB( Config.log, db, num );
       if (msg)
        { 
          Envoyer_commande_dls( msg->bit_voc );          /* Positionnement du profil audio via monostable */

          Envoyer_commande_dls( NUM_BIT_M_AUDIO_START ); /* Positionné quand on envoi une diffusion audio */

          if (Partage->com_audio.last_audio + AUDIO_JINGLE < Partage->top) /* Si Pas de message depuis xx */
           { Jouer_wav("Son/jingle.wav"); }                                     /* On balance le jingle ! */
          Partage->com_audio.last_audio = Partage->top;

          if ( ! Jouer_mp3 ( msg ) )               /* Par priorité : mp3 d'abord, synthèse vocale ensuite */
           { Jouer_espeak ( msg ); }

          g_free(msg);
        }
     }
    Libere_DB_SQL( Config.log, &db );
    Info_new( Config.log, FALSE, LOG_NOTICE,
              "Run_audio: Down . . . TID = %d", pthread_self() );
    Partage->com_audio.TID = 0;                           /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
