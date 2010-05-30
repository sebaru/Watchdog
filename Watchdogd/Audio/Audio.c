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
     { Info_n( Config.log, DEBUG_INFO, "AUDIO: Ajouter_audio: DROP audio (taille>150)", num);
       return;
     }

    pthread_mutex_lock( &Partage->com_audio.synchro );           /* Ajout dans la liste de audio a traiter */
    Partage->com_audio.liste_audio = g_list_append( Partage->com_audio.liste_audio, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_audio.synchro );
  }
/**********************************************************************************************************/
/* Main: Fonction principale du RS485                                                                     */
/**********************************************************************************************************/
 void Run_audio ( void )
  { struct CMD_TYPE_MESSAGE *msg;
    struct DB *db;
    guint num;
    prctl(PR_SET_NAME, "W-Audio", 0, 0, 0 );

    Info( Config.log, DEBUG_FORK, "Audio: demarrage" );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_DB, "AUDIO: Run_audio: Unable to open database", Config.db_database );
       pthread_exit(GINT_TO_POINTER(-1));
     }

    Partage->com_audio.liste_audio = NULL;                      /* Initialisation des variables du thread */
    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { if (Partage->com_audio.sigusr1)                                            /* On a recu sigusr1 ?? */
        { Partage->com_audio.sigusr1 = FALSE;
          Info( Config.log, DEBUG_INFO, "AUDIO: Run_audio: SIGUSR1" );
        }

       if (!Partage->com_audio.liste_audio)                               /* Si pas de message, on tourne */
        { sched_yield();
          usleep(10000);
          continue;
        }

       pthread_mutex_lock( &Partage->com_audio.synchro );                                /* lockage futex */
       num = GPOINTER_TO_INT(Partage->com_audio.liste_audio->data);              /* Recuperation du audio */
       Partage->com_audio.liste_audio = g_list_remove ( Partage->com_audio.liste_audio,
                                                        GINT_TO_POINTER(num) );
#ifdef DEBUG
       Info_n( Config.log, DEBUG_INFO, "AUDIO: Run_audio: Reste a traiter",
                                       g_list_length(Partage->com_audio.liste_audio) );
#endif
       pthread_mutex_unlock( &Partage->com_audio.synchro );

       Info_n( Config.log, DEBUG_INFO, "AUDIO : Préparation du message id", num );

       msg = Rechercher_messageDB( Config.log, db, num );
       if (msg)
        { gchar nom_fichier[128], cible[128];
          gint fd_cible, pid;

          Envoyer_commande_dls( msg->bit_voc );          /* Positionnement du profil audio via monostable */

          g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.pho", msg->num );
          g_snprintf( cible,       sizeof(cible),       "%d.au",  msg->num );
          fd_cible = open ( cible, O_RDONLY, 0 );
          if (fd_cible>0)
           { Info_c( Config.log, DEBUG_INFO, "AUDIO : le .au existe deja", cible );
             close(fd_cible);
           }                  /* Si le fichier au existe, on ne le créé pas à nouveau */
          else
           { 
/***************************************** Création du PHO ************************************************/
             Info_n( Config.log, DEBUG_INFO, "AUDIO : Lancement de ESPEAK", num );
             pid = fork();
             if (pid<0)
              { Info_n( Config.log, DEBUG_INFO, "AUDIO : Fabrication .pho failed", num ); }
             else if (!pid)                                        /* Création du .au en passant par .pho */
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
                execlp( "espeak", "espeak", "-s", chaine2, "-v", chaine, texte, NULL );
                Info_n( Config.log, DEBUG_FORK, "AUDIO: Lancement espeak failed", pid );
                _exit(0);
              }
             Info_n( Config.log, DEBUG_FORK, "AUDIO: waiting for espeak to finish pid", pid );
             wait4(pid, NULL, 0, NULL );
             Info_n( Config.log, DEBUG_FORK, "AUDIO: espeak finished pid", pid );

/****************************************** Création du AU ************************************************/
             Info_n( Config.log, DEBUG_INFO, "AUDIO : Lancement de MBROLA", num );
             pid = fork();
             if (pid<0)
              { Info_n( Config.log, DEBUG_INFO, "AUDIO : Fabrication .au failed", num ); }
             else if (!pid)                                        /* Création du .au en passant par .pho */
              { gchar chaine[30];
                switch (msg->type_voc)
                 { case 0: g_snprintf( chaine, sizeof(chaine), "fr1" ); break;
                   case 1: g_snprintf( chaine, sizeof(chaine), "fr2" ); break;
                   case 2: g_snprintf( chaine, sizeof(chaine), "fr6" ); break;
                   default:
                   case 3: g_snprintf( chaine, sizeof(chaine), "fr4" ); break;
                 }
                execlp( "mbrola-linux-i386", "mbrola-linux-i386", chaine, nom_fichier, cible, NULL );
                Info_n( Config.log, DEBUG_FORK, "AUDIO: Lancement mbrola failed", pid );
                _exit(0);
              }
             Info_n( Config.log, DEBUG_FORK, "AUDIO: waiting for mbrola to finish pid", pid );
             wait4(pid, NULL, 0, NULL );
             Info_n( Config.log, DEBUG_FORK, "AUDIO: mbrola finished pid", pid );

           }
/****************************************** Lancement de l'audio ******************************************/
          Info_n( Config.log, DEBUG_INFO, "AUDIO : Lancement de APLAY", num );
          pid = fork();
          if (pid<0)
           { Info_n( Config.log, DEBUG_INFO, "AUDIO : Lancement APLAY failed", num ); }
          else if (!pid)
           { execlp( "aplay", "aplay", "-R", "1", cible, NULL );
             Info_n( Config.log, DEBUG_FORK, "AUDIO: Lancement APLAY failed", pid );
             _exit(0);
           }
          Info_n( Config.log, DEBUG_FORK, "AUDIO: waiting for APLAY to finish pid", pid );
          wait4(pid, NULL, 0, NULL );
          Info_n( Config.log, DEBUG_FORK, "AUDIO: APLAY finished pid", pid );

          g_free(msg);
        }
     }
    Libere_DB_SQL( Config.log, &db );
    Info_n( Config.log, DEBUG_FORK, "AUDIO: Run_audio: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
