/**********************************************************************************************************/
/* Watchdogd/Audio/Audio.c  Gestion des messages audio de Watchdog 2.0                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 11 avr 2009 12:08:12 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Archive.c
 * This file is part of <program name>
 *
 * Copyright (C) 2009 - sebastien
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
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

 #include "Message_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "proto_dls.h"
 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
 static struct DB *Db_watchdog;                                                      /* Database Watchdog */

/**********************************************************************************************************/
/* Ajouter_audio: Ajoute un message audio a prononcer                                                     */
/* Entrées: le numéro du message a prononcer                                                              */
/**********************************************************************************************************/
 void Ajouter_audio( gint id )
  { gint taille;

    pthread_mutex_lock( &Partage->com_audio.synchro );          /* Ajout dans la liste de audio a traiter */
    taille = g_list_length( Partage->com_audio.liste_audio );
    pthread_mutex_unlock( &Partage->com_audio.synchro );

    if (taille > 150)
     { Info_n( Config.log, DEBUG_INFO, "AUDIO: Ajouter_audio: DROP audio (taille>150)", id);
       return;
     }

    pthread_mutex_lock( &Partage->com_audio.synchro );           /* Ajout dans la liste de audio a traiter */
    Partage->com_audio.liste_audio = g_list_append( Partage->com_audio.liste_audio, GINT_TO_POINTER(id) );
    pthread_mutex_unlock( &Partage->com_audio.synchro );
  }
/**********************************************************************************************************/
/* Main: Fonction principale du RS485                                                                     */
/**********************************************************************************************************/
 void Run_audio ( void )
  { guint id;
    struct MSGDB *msg;
    prctl(PR_SET_NAME, "W-Audio", 0, 0, 0 );

    Info( Config.log, DEBUG_FORK, "Audio: demarrage" );

    sleep(5);                                                      /* A l'init, nous attendons 5 secondes */

    Db_watchdog = ConnexionDB( Config.log, Config.db_name,           /* Connexion en tant que user normal */
                               Config.db_admin_username, Config.db_password );

    if (!Db_watchdog)
     { Info_c( Config.log, DEBUG_DB, "AUDIO: Run_audio: Unable to open database (dsn)", Config.db_name );
       pthread_exit(GINT_TO_POINTER(-1));
     }

    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { if (Partage->com_audio.sigusr1)                                             /* On a recu sigusr1 ?? */
        { Partage->com_audio.sigusr1 = FALSE;
          Info( Config.log, DEBUG_INFO, "AUDIO: Run_audio: SIGUSR1" );
        }

       if (!Partage->com_audio.liste_audio)                               /* Si pas de message, on tourne */
        { sched_yield();
          usleep(10000);
          continue;
        }

       pthread_mutex_lock( &Partage->com_audio.synchro );                                /* lockage futex */
       id = GPOINTER_TO_INT(Partage->com_audio.liste_audio->data);              /* Recuperation du audio */
       Partage->com_audio.liste_audio = g_list_remove ( Partage->com_audio.liste_audio, GINT_TO_POINTER(id) );
#ifdef DEBUG
       Info_n( Config.log, DEBUG_INFO, "AUDIO: Run_audio: Reste a traiter",
                                       g_list_length(Partage->com_audio.liste_audio) );
#endif
       pthread_mutex_unlock( &Partage->com_audio.synchro );

       Info_n( Config.log, DEBUG_INFO, "AUDIO : Préparation du message id", id );

       msg = Rechercher_messageDB_par_id( Config.log, Db_watchdog, id );
       if (msg)
        { gchar nom_fichier[128];
          gint fd_cible;

          g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.au", msg->id );
          fd_cible = open ( nom_fichier, O_RDONLY, 0 );
          if (fd_cible>0)
           { Info_c( Config.log, DEBUG_INFO, "AUDIO : le .au existe deja", nom_fichier );
             close(fd_cible);
           }                  /* Si le fichier au existe, on ne le créé pas à nouveau */
          else
           { gint pid;

/***************************************** Création du PHO ************************************************/
             g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.pho", msg->id );
             Info_n( Config.log, DEBUG_INFO, "AUDIO : Lancement de ESPEAK", id );
             pid = fork();
             if (pid<0)
              { Info_n( Config.log, DEBUG_INFO, "AUDIO : Fabrication .pho failed", id ); }
             else if (!pid)                                        /* Création du .au en passant par .pho */
              { gchar texte[80];
                fd_cible = open ( nom_fichier, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
                dup2( fd_cible, 1 );
                g_snprintf( texte, sizeof(texte), "%s", msg->libelle );
                execlp( "espeak", "espeak", "-s", "120", "-v", "mb/mb-fr4", texte, "--stdout", NULL );
                Info_n( Config.log, DEBUG_FORK, "AUDIO: Lancement espeak failed", pid );
                _exit(0);
              }
             Info_n( Config.log, DEBUG_FORK, "AUDIO: waiting for espeak to finish pid", pid );
             wait4(pid, NULL, 0, NULL );
             Info_n( Config.log, DEBUG_FORK, "AUDIO: espeak finished pid", pid );

/****************************************** Création du AU ************************************************/
             Info_n( Config.log, DEBUG_INFO, "AUDIO : Lancement de MBROLA", id );
             pid = fork();
             if (pid<0)
              { Info_n( Config.log, DEBUG_INFO, "AUDIO : Fabrication .au failed", id ); }
             else if (!pid)                                        /* Création du .au en passant par .pho */
              { gchar cible[80];
                g_snprintf( cible, sizeof(nom_fichier), "%d.au", msg->id );
                execlp( "mbrola-linux-i386", "fr4", nom_fichier, cible, NULL );
                Info_n( Config.log, DEBUG_FORK, "AUDIO: Lancement mbrola failed", pid );
                _exit(0);
              }
             Info_n( Config.log, DEBUG_FORK, "AUDIO: waiting for mbrola to finish pid", pid );
             wait4(pid, NULL, 0, NULL );
             Info_n( Config.log, DEBUG_FORK, "AUDIO: mbrola finished pid", pid );

           }
          g_free(msg);
        }
     }
    DeconnexionDB( Config.log, &Db_watchdog );                                  /* Deconnexion de la base */
    Info_n( Config.log, DEBUG_FORK, "AUDIO: Run_audio: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
