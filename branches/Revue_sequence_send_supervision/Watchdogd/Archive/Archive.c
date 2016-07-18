/******************************************************************************************************************************/
/* Watchdogd/Archive/Archive.c  Gestion des archivages bit_internes Watchdog 2.0                                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         mer. 09 mai 2012 12:44:56 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <rrd.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */

/******************************************************************************************************************************/
/* Arch_clear_list: efface la liste des archives a prendre en compte                                                          */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                                    */
/******************************************************************************************************************************/
 gint Arch_Clear_list ( void )
  { struct ARCHDB *arch;
    gint save_nbr;
    pthread_mutex_lock( &Partage->com_arch.synchro );                                                        /* lockage futex */
    save_nbr = Partage->com_arch.taille_arch;
    while ( Partage->com_arch.liste_arch )
     { arch = Partage->com_arch.liste_arch->data;                                                     /* Recuperation du arch */
       g_free(arch);                                                                                    /* Libération mémoire */
       Partage->com_arch.liste_arch = g_slist_remove ( Partage->com_arch.liste_arch, arch );
       Partage->com_arch.taille_arch--;
     }
    pthread_mutex_unlock( &Partage->com_arch.synchro );
    return(save_nbr);
 }
/******************************************************************************************************************************/
/* Ajouter_arch: Ajoute une archive dans la base de données                                                                   */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                                    */
/******************************************************************************************************************************/
 void Ajouter_arch( gint type, gint num, gfloat valeur )
  { static gint last_log = 0;
    struct timeval tv;
    struct ARCHDB *arch;

    if (Config.instance_is_master == FALSE) return;                                  /* Les instances Slave n'archivent pas ! */
    else if (Partage->com_arch.taille_arch > 10000)
     { if ( last_log + 60 < Partage->top )
        { Info_new( Config.log, Config.log_arch, LOG_INFO,
                   "Ajouter_arch: DROP arch (taille>10000) type=%d, num=%d", type, num );
          last_log = Partage->top;
        }
       return;
     }
    else if (Partage->com_arch.Thread_run == FALSE)
     { if (Config.instance_is_master == FALSE) return;                                   /* Si administratively DOWN, on sort */
       if ( last_log + 60 < Partage->top )
        { Info_new( Config.log, Config.log_arch, LOG_INFO,
                   "Ajouter_arch: Thread is down. Dropping type=%d, num=%d", type, num );
          last_log = Partage->top;
        }
       return;
     }
    else
     { Info_new( Config.log, Config.log_arch, LOG_DEBUG,
                "Ajouter_arch: Add Arch a traiter type=%d, num=%d", type, num );
     }

    arch = (struct ARCHDB *)g_try_malloc0( sizeof(struct ARCHDB) );
    if (!arch) return;

    gettimeofday( &tv, NULL );                                                                   /* On prend l'heure actuelle */
    arch->type      = type;
    arch->num       = num;
    arch->valeur    = valeur;
    arch->date_sec  = tv.tv_sec;
    arch->date_usec = tv.tv_usec;

    pthread_mutex_lock( &Partage->com_arch.synchro );                                /* Ajout dans la liste de arch a traiter */
    Partage->com_arch.liste_arch = g_slist_append( Partage->com_arch.liste_arch, arch );
    Partage->com_arch.taille_arch++;
    pthread_mutex_unlock( &Partage->com_arch.synchro );
  }
/******************************************************************************************************************************/
/* Ajouter_archRRA: Ajout d'un enregistrement dans les fichiers RRAs                                                          */
/* Entrée: une structure d'archivage                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Ajouter_archRRD ( struct ARCHDB *arch )
  { gchar fichier[80], update[80];
    gchar *params[11];
    struct stat sbuf;
    gint result;

    g_snprintf( fichier, sizeof(fichier), "RRA/%02d-%04d.rrd", arch->type, arch->num );
    params[1] = fichier;

    if ( stat ( fichier, &sbuf ) == -1)                                                           /* Test présence du fichier */
     { Info_new( Config.log, Config.log_arch, LOG_WARNING,
                "Ajouter_archRRD: Unable to Stat file %s (%s). Trying to create RRD file", fichier, strerror(errno) );
       params[0] = "create";
       params[2] = "--start";
       params[3] = "-1y";
       params[4] = "--step";
       params[5] = "10";
       params[6] = "DS:val:GAUGE:1000:U:U";
       params[7] = "RRA:MIN:0.5:3:630720"; /* 6*60*24*365 / 5 */
       params[8] = "RRA:MAX:0.5:3:630720";
       params[9] = "RRA:AVERAGE:0.5:3:630720";
       params[10] = "RRA:LAST:0.5:1:630720";
       rrd_clear_error();
       result = rrd_create( 11, params );
       if (result)       
        { Info_new( Config.log, Config.log_arch, LOG_ERR,
                    "Ajouter_archRRD: Error %d creating RRD (%s). Dropping ARCH", result, rrd_get_error() );
        }
       else 
        { Info_new( Config.log, Config.log_arch, LOG_INFO,
                    "Ajouter_archRRD: Creation of RRD file %s OK", fichier );
        }
       return;
     }
    params[0] = "rrdupdate";
    g_snprintf( update,  sizeof(update), "%d:%f", arch->date_sec, arch->valeur );
    params[2] = update;
    rrd_clear_error();
    result = rrd_update( 3, params );
    /*if (result)
     { Info_new( Config.log, Config.log_arch, LOG_WARNING,
                "Ajouter_archRRD: RRD error %d (%s)", result, rrd_get_error() );
     }*/
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du thread                                                                                        */
/******************************************************************************************************************************/
 void Run_arch ( void )
  { struct DB *db;
    prctl(PR_SET_NAME, "W-Arch", 0, 0, 0 );

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "Starting" );

    Partage->com_arch.liste_arch  = NULL;                                                     /* Initialisation des variables */
    Partage->com_arch.Thread_run  = TRUE;                                                               /* Le thread tourne ! */
    Partage->com_arch.taille_arch = 0;
    Info_new( Config.log, Config.log_arch, LOG_NOTICE,
              "Run_arch: Demarrage . . . TID = %p", pthread_self() );

    while(Partage->com_arch.Thread_run == TRUE)                                              /* On tourne tant que necessaire */
     { struct ARCHDB *arch;

       if (Partage->com_arch.Thread_reload)                                                             /* On a recu RELOAD ? */
        { Info_new( Config.log, Config.log_arch, LOG_NOTICE, "Run_arch: RELOAD" );
          Partage->com_arch.Thread_reload = FALSE;
        }

       if (Partage->com_arch.Thread_sigusr1)                                                          /* On a recu sigusr1 ?? */
        { Info_new( Config.log, Config.log_arch, LOG_NOTICE, "Run_arch: SIGUSR1" );
          pthread_mutex_lock( &Partage->com_arch.synchro );                                                  /* lockage futex */
          Info_new( Config.log, Config.log_arch, LOG_INFO,
                   "Run_arch: Reste %03d a traiter",
                    g_slist_length(Partage->com_arch.liste_arch) );
          pthread_mutex_unlock( &Partage->com_arch.synchro );
          Partage->com_arch.Thread_sigusr1 = FALSE;
        }

       if (!Partage->com_arch.liste_arch)                                                     /* Si pas de message, on tourne */
        { sched_yield();
          usleep(10000);
          continue;
        }

       db = Init_DB_SQL();       
       if (!db)
        { Info_new( Config.log, Config.log_arch, LOG_ERR, 
                   "Run_arch: Unable to open database %s", Config.db_database );
          continue;
        }

       while (Partage->com_arch.liste_arch)
        { pthread_mutex_lock( &Partage->com_arch.synchro );                                                  /* lockage futex */
          arch = Partage->com_arch.liste_arch->data;                                                  /* Recuperation du arch */
          Partage->com_arch.liste_arch = g_slist_remove ( Partage->com_arch.liste_arch, arch );
          Partage->com_arch.taille_arch--;
          Info_new( Config.log, Config.log_arch, LOG_DEBUG,
                   "Run_arch: Reste %03d a traiter", Partage->com_arch.taille_arch );
          pthread_mutex_unlock( &Partage->com_arch.synchro );
          Ajouter_archRRD( arch );
          Ajouter_archDB ( db, arch );
          g_free(arch);
          Info_new( Config.log, Config.log_arch, LOG_DEBUG, "Run_arch: archive saved" );
        }
       Libere_DB_SQL( &db );
       SEA ( NUM_EA_SYS_ARCHREQUEST, Partage->com_arch.taille_arch );                                      /* pour historique */
     }

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "Run_arch: Down (%p)", pthread_self() );
    Partage->com_arch.Thread_run  = FALSE;                                                              /* Le thread tourne ! */
    Partage->com_arch.TID = 0;                                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
