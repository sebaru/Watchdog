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

/**********************************************************************************************************/
/* Ajouter_arch: Ajoute une archive dans la base de données                                               */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                */
/**********************************************************************************************************/
 void Ajouter_arch( gint type, gint num, gfloat valeur )
  { struct timeval tv;
    struct ARCHDB *arch;

    if (Partage->com_arch.taille_arch > 150)
     { Info_new( Config.log, Config.log_all, LOG_INFO,
                "Ajouter_arch: DROP arch (taille>150) type=%d, num=%d", type, num );
       return;
     }
    else
     { Info_new( Config.log, Config.log_all, LOG_DEBUG,
                "Ajouter_arch: Add Arch a traiter type=%d, num=%d", type, num );
     }

    arch = (struct ARCHDB *)g_try_malloc0( sizeof(struct ARCHDB) );
    if (!arch) return;

    gettimeofday( &tv, NULL );                                               /* On prend l'heure actuelle */
    arch->type      = type;
    arch->num       = num;
    arch->valeur    = valeur;
    arch->date_sec  = tv.tv_sec;
    arch->date_usec = tv.tv_usec;

    pthread_mutex_lock( &Partage->com_arch.synchro );            /* Ajout dans la liste de arch a traiter */
    Partage->com_arch.liste_arch = g_slist_prepend( Partage->com_arch.liste_arch, arch );
    Partage->com_arch.taille_arch++;
    pthread_mutex_unlock( &Partage->com_arch.synchro );
  }
/**********************************************************************************************************/
/* Main: Fonction principale du RS485                                                                     */
/**********************************************************************************************************/
 void Run_arch ( void )
  { struct DB *db;
    prctl(PR_SET_NAME, "W-Arch", 0, 0, 0 );

    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Starting" );

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Config.log_all, LOG_ERR, 
                "Run_arch: Unable to open database %s", Config.db_database );
       Partage->com_arch.TID = 0;                         /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }

    Partage->com_arch.liste_arch  = NULL;                                 /* Initialisation des variables */
    Partage->com_arch.Thread_run  = TRUE;                                           /* Le thread tourne ! */
    Partage->com_arch.taille_arch = 0;
    while(Partage->com_arch.Thread_run == TRUE)                          /* On tourne tant que necessaire */
     { struct ARCHDB *arch;

       if (Partage->com_arch.Thread_reload)                                         /* On a recu RELOAD ? */
        { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_arch: RELOAD" );
          Partage->com_arch.Thread_reload = FALSE;
        }

       if (Partage->com_arch.Thread_sigusr1)                                      /* On a recu sigusr1 ?? */
        { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_arch: SIGUSR1" );
          pthread_mutex_lock( &Partage->com_arch.synchro );                                 /* lockage futex */
          Info_new( Config.log, Config.log_all, LOG_INFO,
                   "Run_arch: Reste %03d a traiter",
                    g_slist_length(Partage->com_arch.liste_arch) );
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
       Partage->com_arch.liste_arch = g_slist_remove ( Partage->com_arch.liste_arch, arch );
       Info_new( Config.log, Config.log_all, LOG_DEBUG,
                "Run_arch: Reste %03d a traiter",
                 g_slist_length(Partage->com_arch.liste_arch) );
       Partage->com_arch.taille_arch--;
       pthread_mutex_unlock( &Partage->com_arch.synchro );
       Ajouter_archDB ( Config.log, db, arch );
       Info_new( Config.log, Config.log_all, LOG_DEBUG, "Run_arch: archive saved" );
       g_free(arch);
     }
    Libere_DB_SQL( Config.log, &db );
    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_arch: Down (%d)", pthread_self() );
    Partage->com_arch.TID = 0;                            /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
