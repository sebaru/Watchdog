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

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */

/******************************************************************************************************************************/
/* Arch_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                   */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Arch_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    g_snprintf( Partage->com_arch.archdb_database, sizeof(Partage->com_arch.archdb_database), "%s", Config.db_database );
    g_snprintf( Partage->com_arch.archdb_username, sizeof(Partage->com_arch.archdb_username), "%s", Config.db_username );
    g_snprintf( Partage->com_arch.archdb_password, sizeof(Partage->com_arch.archdb_password), "%s", Config.db_password );
    g_snprintf( Partage->com_arch.archdb_host, sizeof(Partage->com_arch.archdb_host), "%s", Config.db_host );
    Partage->com_arch.archdb_port = Config.db_port;
    Partage->com_arch.duree_retention = ARCHIVE_DEFAUT_RETENTION;

    if ( ! Recuperer_configDB( &db, "arch" ) )                                              /* Connexion a la base de données */
     { Info_new( Config.log, Config.log_arch, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     {      if ( ! g_ascii_strcasecmp ( nom, "database" ) )
        { g_snprintf( Partage->com_arch.archdb_database, sizeof(Partage->com_arch.archdb_database), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "username" ) )
        { g_snprintf( Partage->com_arch.archdb_username, sizeof(Partage->com_arch.archdb_username), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "password" ) )
        { g_snprintf( Partage->com_arch.archdb_password, sizeof(Partage->com_arch.archdb_password), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "host" ) )
        { g_snprintf( Partage->com_arch.archdb_host, sizeof(Partage->com_arch.archdb_host), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "port" ) )
        { Partage->com_arch.archdb_port = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "days" ) )
        { Partage->com_arch.duree_retention = atoi(valeur);  }
       else
        { Info_new( Config.log, Config.log_arch, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Arch_clear_list: efface la liste des archives a prendre en compte                                                          */
/* Entrées: néant                                                                                                             */
/* Sortie : le nombre d'archive detruites                                                                                     */
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
    Info_new( Config.log, Config.log_arch, LOG_DEBUG,
             "%s: Clear %05d archive(s)", __func__, save_nbr );
    return(save_nbr);
 }
/******************************************************************************************************************************/
/* Ajouter_arch: Ajoute une archive dans la base de données                                                                   */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                                    */
/******************************************************************************************************************************/
 void Ajouter_arch( gint type, gint num, gfloat valeur )
  { static gint last_log = 0, taille_buf = 500000;
    struct timeval tv;
    struct ARCHDB *arch;

    if (Config.instance_is_master == FALSE) return;                                  /* Les instances Slave n'archivent pas ! */
    else if (Partage->com_arch.taille_arch > taille_buf)
     { if ( last_log + 600 < Partage->top )
        { Info_new( Config.log, Config.log_arch, LOG_INFO,
                   "Ajouter_arch: DROP arch (taille>%d) type=%d, num=%d", taille_buf, type, num );
          last_log = Partage->top;
        }
       return;
     }
    else if (Partage->com_arch.Thread_run == FALSE)                                      /* Si administratively DOWN, on sort */
     { if ( last_log + 600 < Partage->top )
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
    Partage->com_arch.liste_arch = g_slist_prepend( Partage->com_arch.liste_arch, arch );
    Partage->com_arch.taille_arch++;
    pthread_mutex_unlock( &Partage->com_arch.synchro );
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du thread                                                                                        */
/******************************************************************************************************************************/
 void Run_arch ( void )
  { struct DB *db;
    gint top, last_update;
    prctl(PR_SET_NAME, "W-Arch", 0, 0, 0 );

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "Starting" );

    Arch_Lire_config ();                                                                       /* Lecture des données en base */
    Partage->com_arch.liste_arch  = NULL;                                                     /* Initialisation des variables */
    Partage->com_arch.Thread_run  = TRUE;                                                               /* Le thread tourne ! */
    Partage->com_arch.taille_arch = 0;
    Info_new( Config.log, Config.log_arch, LOG_NOTICE,
              "Run_arch: Demarrage . . . TID = %p", pthread_self() );

    last_update = Partage->top;
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
          Arch_Lire_config();
        }

       if ( (Partage->top - last_update) >= 864000 )                                                     /* Une fois par jour */
        { pthread_t tid;
          if (pthread_create( &tid, NULL, (void *)Arch_Update_SQL_Partitions_thread, NULL ))
           { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s: pthread_create failed for Update SQL Partitions", __func__ ); }
          else
           { pthread_detach( tid ); }                                /* On le detache pour qu'il puisse se terminer tout seul */
          last_update=Partage->top;
        }

       if (!Partage->com_arch.liste_arch)                                                     /* Si pas de message, on tourne */
        { sched_yield();
          sleep(5);
          continue;
        }

       db = Init_ArchDB_SQL();       
       if (!db)
        { Info_new( Config.log, Config.log_arch, LOG_ERR, 
                   "%s: Unable to open database %s/%s/%s", __func__,
                    Partage->com_arch.archdb_host, Partage->com_arch.archdb_username, Partage->com_arch.archdb_database );
          sleep(10);
          continue;
        }
       Info_new( Config.log, Config.log_arch, LOG_DEBUG,
                "%s: Traitement de %05d archive(s)", __func__, Partage->com_arch.taille_arch );
       top = Partage->top;
       while (Partage->com_arch.liste_arch && Partage->com_arch.Thread_run == TRUE)
        { pthread_mutex_lock( &Partage->com_arch.synchro );                                                  /* lockage futex */
          arch = Partage->com_arch.liste_arch->data;                                                  /* Recuperation du arch */
          Partage->com_arch.liste_arch = g_slist_remove ( Partage->com_arch.liste_arch, arch );
          Partage->com_arch.taille_arch--;
          pthread_mutex_unlock( &Partage->com_arch.synchro );
          Ajouter_archDB ( db, arch );
          g_free(arch);
          Info_new( Config.log, Config.log_arch, LOG_DEBUG,
                   "%s: Reste %d archives a traiter", __func__, Partage->com_arch.taille_arch );
        }
       Info_new( Config.log, Config.log_arch, LOG_DEBUG,
                "%s: Traitement en %06.1fs", __func__, (Partage->top-top)/10.0 );
       Libere_DB_SQL( &db );
       SEA ( NUM_EA_SYS_ARCHREQUEST, Partage->com_arch.taille_arch );                                      /* pour historique */
     }

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Cleaning Arch List before stop", __func__);
    Arch_Clear_list();                                              /* Suppression des enregistrements restants dans la liste */

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Down (%p)", __func__, pthread_self() );
    Partage->com_arch.Thread_run  = FALSE;                                                              /* Le thread tourne ! */
    Partage->com_arch.TID = 0;                                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
