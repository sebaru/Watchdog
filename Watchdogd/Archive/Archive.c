/**********************************************************************************************************/
/* Watchdogd/Archive/Archive.c  Gestion des archivages bit_internes Watchdog 2.0                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 08 jui 2006 11:56:48 CEST */
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

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 /*#define DEBUG*/
/**********************************************************************************************************/
/* Ajouter_arch: Ajoute une archive dans la base de données                                               */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                */
/**********************************************************************************************************/
 void Ajouter_arch( gint type, gint num, gint valeur )
  { struct timeval tv;
    struct ARCHDB *arch;
    GList *liste;

    if (Partage->com_arch.taille_arch > 150)
     { Info_n( Config.log, DEBUG_INFO, "ARCH: Ajouter_arch: DROP arch (taille>150) type", type );
       Info_n( Config.log, DEBUG_INFO, "ARCH: Ajouter_arch: DROP arch (taille>150)  num", num );
       return;
     }
    liste = Partage->com_arch.liste_arch;
    pthread_mutex_lock( &Partage->com_arch.synchro );            /* Ajout dans la liste de arch a traiter */
printf(" longueur liste %p %d\n", liste, g_list_length(Partage->com_arch.liste_arch) );
    while (liste)
     {
printf(" début\n" );
       arch = Partage->com_arch.liste_arch->data;                                 /* Recuperation du arch */
printf(" arch %p\n", arch );
       if (arch->type == type && arch->num == num) break;
       liste = liste->next;
     }
    pthread_mutex_unlock( &Partage->com_arch.synchro );
    if (liste) return;                                              /* Si deja dans la base, on le swappe */

    arch = (struct ARCHDB *)g_malloc( sizeof(struct ARCHDB) );
    if (!arch) return;

    gettimeofday( &tv, NULL );                                               /* On prend l'heure actuelle */
    arch->type      = type;
    arch->num       = num;
    arch->valeur    = valeur;
    arch->date_sec  = tv.tv_sec;
    arch->date_usec = tv.tv_usec;

    pthread_mutex_lock( &Partage->com_arch.synchro );            /* Ajout dans la liste de arch a traiter */
    Partage->com_arch.liste_arch = g_list_append( Partage->com_arch.liste_arch, arch );
    Partage->com_arch.taille_arch++;
    pthread_mutex_unlock( &Partage->com_arch.synchro );
  }
/**********************************************************************************************************/
/* Main: Fonction principale du RS485                                                                     */
/**********************************************************************************************************/
 void Run_arch ( void )
  { struct DB *db;
    guint top;
    prctl(PR_SET_NAME, "W-Arch", 0, 0, 0 );

    Info( Config.log, DEBUG_FORK, "ARCH: demarrage" );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_DB, "ARCH: Run_arch: Unable to open database", Config.db_database );
       pthread_exit(GINT_TO_POINTER(-1));
     }

    Partage->com_arch.liste_arch = NULL;                                  /* Initialisation des variables */
    top = Partage->top;                                        /* Initialisation des archivages temporels */
    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { struct ARCHDB *arch;

       if (Partage->com_arch.sigusr1)                                             /* On a recu sigusr1 ?? */
        { Partage->com_arch.sigusr1 = FALSE;
          Info( Config.log, DEBUG_INFO, "ARCH: Run_arch: SIGUSR1" );
        }

       if (!Partage->com_arch.liste_arch)                                 /* Si pas de message, on tourne */
        { sched_yield();
          usleep(10000);
          continue;
        }

       pthread_mutex_lock( &Partage->com_arch.synchro );                                 /* lockage futex */
       arch = Partage->com_arch.liste_arch->data;                                 /* Recuperation du arch */
       Partage->com_arch.liste_arch = g_list_remove ( Partage->com_arch.liste_arch, arch );
#ifdef DEBUG
       Info_n( Config.log, DEBUG_INFO, "ARCH: Run_arch: Reste a traiter",
                                       g_list_length(Partage->com_arch.liste_arch) );
#endif
       Partage->com_arch.taille_arch--;
       pthread_mutex_unlock( &Partage->com_arch.synchro );

       Ajouter_archDB ( Config.log, db, arch );

       g_free(arch);
     }
    Libere_DB_SQL( Config.log, &db );
    Info_n( Config.log, DEBUG_FORK, "ARCH: Run_arch: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
