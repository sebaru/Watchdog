/**********************************************************************************************************/
/* Watchdogd/Archive/Archive.c  Gestion des archivages bit_internes Watchdog 2.0                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 09 mai 2012 12:44:56 CEST */
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

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 /*#define DEBUG*/
/**********************************************************************************************************/
/* Ajouter_arch: Ajoute une archive dans la base de données                                               */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                */
/**********************************************************************************************************/
 void Ajouter_arch( gint type, gint num, gfloat valeur )
  { struct timeval tv;
    struct ARCHDB *arch;

    if (Partage->com_arch.taille_arch > 150)
     { Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Ajouter_arch: DROP arch (taille>150) type", type );
       Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Ajouter_arch: DROP arch (taille>150)  num", num );
       return;
     }
    else
     { Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Ajouter_arch: Add Arch a traiter type", type );
       Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Ajouter_arch: Add Arch a traiter  num", num );
     }

    arch = (struct ARCHDB *)g_malloc( sizeof(struct ARCHDB) );
    if (!arch) return;

    Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Ajouter_arch : arch = ", arch );
    gettimeofday( &tv, NULL );                                               /* On prend l'heure actuelle */
    arch->type      = type;
    arch->num       = num;
    arch->valeur    = valeur;
    arch->date_sec  = tv.tv_sec;
    arch->date_usec = tv.tv_usec;

    pthread_mutex_lock( &Partage->com_arch.synchro );            /* Ajout dans la liste de arch a traiter */
    Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Ajouter_arch : Entrée zone protégée arch = ", arch );
    Partage->com_arch.liste_arch = g_list_append( Partage->com_arch.liste_arch, arch );
    Partage->com_arch.taille_arch++;
    Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Ajouter_arch : Sortie zone protégée liste_arch->date = ",
                        Partage->com_arch.liste_arch->data );
    pthread_mutex_unlock( &Partage->com_arch.synchro );
  }
/**********************************************************************************************************/
/* Main: Fonction principale du RS485                                                                     */
/**********************************************************************************************************/
 void Run_arch ( void )
  { struct DB *db;
    prctl(PR_SET_NAME, "W-Arch", 0, 0, 0 );

    Info( Config.log, DEBUG_ARCHIVE, "ARCH: demarrage" );

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_c( Config.log, DEBUG_ARCHIVE, "ARCH: Run_arch: Unable to open database", Config.db_database );
       Partage->com_arch.TID = 0;                         /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }

    Partage->com_arch.liste_arch  = NULL;                                 /* Initialisation des variables */
    Partage->com_arch.Thread_run  = TRUE;                                           /* Le thread tourne ! */
    Partage->com_arch.taille_arch = 0;
    while(Partage->com_arch.Thread_run == TRUE)                          /* On tourne tant que necessaire */
     { struct ARCHDB *arch;

       if (Partage->com_arch.Thread_reload)                                         /* On a recu RELOAD ? */
        { Info( Config.log, DEBUG_ARCHIVE, "ARCH: Run_arch: RELOAD" );
          Partage->com_arch.Thread_reload = FALSE;
        }

       if (Partage->com_arch.Thread_sigusr1)                                      /* On a recu sigusr1 ?? */
        { Info( Config.log, DEBUG_ARCHIVE, "ARCH: Run_arch: SIGUSR1" );
          pthread_mutex_lock( &Partage->com_arch.synchro );                                 /* lockage futex */
          Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Run_arch: Reste a traiter",
                  g_list_length(Partage->com_arch.liste_arch) );
          pthread_mutex_unlock( &Partage->com_arch.synchro );
          Partage->com_arch.Thread_sigusr1 = FALSE;
        }

       if (!Partage->com_arch.liste_arch)                                 /* Si pas de message, on tourne */
        { sched_yield();
          usleep(10000);
          continue;
        }

       pthread_mutex_lock( &Partage->com_arch.synchro );                                 /* lockage futex */
       arch = Partage->com_arch.liste_arch->data;                                 /* Recuperation du arch */
       Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Run_arch : Entrée zone protégée arch = ", arch );
       Partage->com_arch.liste_arch = g_list_remove ( Partage->com_arch.liste_arch, arch );
       Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Run_arch: Reste a traiter",
               g_list_length(Partage->com_arch.liste_arch) );
       Partage->com_arch.taille_arch--;
       Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Run_arch : Sortie zone protégée arch = ", arch );
       pthread_mutex_unlock( &Partage->com_arch.synchro );
       Ajouter_archDB ( Config.log, db, arch );
       Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Run_arch : Avant g_free arch = ", arch );
       g_free(arch);
       Info( Config.log, DEBUG_ARCHIVE, "ARCH: Run_arch: archive saved" );
     }
    Libere_DB_SQL( Config.log, &db );
    Info_n( Config.log, DEBUG_ARCHIVE, "ARCH: Run_arch: Down", pthread_self() );
    Partage->com_arch.TID = 0;                            /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
