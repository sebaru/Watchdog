/******************************************************************************************************************************/
/* Watchdogd/process.c        Gestion des process                                                                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 11 avr 2009 12:21:45 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * process.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <dirent.h>
 #include <string.h>
 #include <stdio.h>

 #include <sys/wait.h>
 #include <fcntl.h>
 #include <errno.h>
 #include <dlfcn.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Thread_init: appelé par chaque thread, lors de son démarrage                                                               */
/* Entrée: Le nom du thread, son niveau de log                                                                                */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 void Thread_init ( gchar *pr_name, gchar *classe, struct LIBRAIRIE *lib, gchar *version, gchar *description )
  { gchar chaine[128];
    prctl(PR_SET_NAME, pr_name, 0, 0, 0 );
    lib->TID = pthread_self();                                                              /* Sauvegarde du TID pour le pere */
    lib->Thread_run = TRUE;                                                                             /* Le thread tourne ! */
    time ( &lib->start_time );
    g_snprintf( lib->version,      sizeof(lib->version),      version );
    g_snprintf( lib->admin_help,   sizeof(lib->admin_help),   description );
    Modifier_configDB ( lib->admin_prompt, "thread_version", lib->version );
    g_snprintf( chaine, sizeof(chaine), "INSERT INTO thread_classe SET thread=UPPER('%s'), classe=UPPER('%s') "
                                        "ON DUPLICATE KEY UPDATE classe=VALUES(classe)", lib->admin_prompt, classe );
    SQL_Write ( chaine );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Démarrage du thread '%s' de classe '%s' -> TID = %p", __func__,
              lib->admin_prompt, classe, pthread_self() );
  }
/******************************************************************************************************************************/
/* Thread_init: appelé par chaque thread, lors de son démarrage                                                               */
/* Entrée: Le nom du thread, son niveau de log                                                                                */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 void Thread_end ( struct LIBRAIRIE *lib )
  { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", lib->admin_prompt, pthread_self() );
    lib->Thread_run = FALSE;                                                                    /* Le thread ne tourne plus ! */
    lib->TID = 0;                                                             /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/******************************************************************************************************************************/
/* Start_librairie: Demarre le thread en paremetre                                                                            */
/* Entrée: La structure associée au thread                                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Start_librairie ( struct LIBRAIRIE *lib )
  { pthread_attr_t attr;
    pthread_t tid;
    if (!lib) return(FALSE);
    if (lib->Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: thread %s already seems to be running", __func__, lib->nom_fichier );
       return(FALSE);
     }

    if ( pthread_attr_init(&attr) )                                                 /* Initialisation des attributs du thread */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: pthread_attr_init failed (%s)", __func__, lib->nom_fichier );
       return(FALSE);
     }

    if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) )                       /* On le laisse joinable au boot */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: pthread_setdetachstate failed (%s)", __func__, lib->nom_fichier );
       return(FALSE);
     }

    if ( pthread_create( &tid, &attr, (void *)lib->Run_thread, lib ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: pthread_create failed (%s)", __func__, lib->nom_fichier );
       return(FALSE);
     }
    pthread_attr_destroy(&attr);                                                                        /* LibÃération mÃémoire */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Starting thread %s OK (TID=%p)", __func__, lib->nom_fichier, tid );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Stop_librairie: Arrete le thread en paremetre                                                                              */
/* EntrÃée: La structure associÃée au thread                                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Stop_librairie ( struct LIBRAIRIE *lib )
  { if (!lib) return(FALSE);
    if ( lib->TID != 0 )
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: thread %s, stopping in progress", __func__, lib->nom_fichier );
       lib->Thread_run = FALSE;                                                          /* On demande au thread de s'arreter */
       while( lib->TID != 0 ) sched_yield();                                                            /* Attente fin thread */
       sleep(1);
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: thread %s stopped", __func__, lib->nom_fichier );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Charger_librarie_par_fichier: Ouverture d'une librairie par son nom                                                        */
/* EntrÃée: Le nom de fichier correspondant                                                                                    */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 struct LIBRAIRIE *Charger_librairie_par_prompt ( gchar *prompt )
  { pthread_mutexattr_t attr;                                                          /* Initialisation des mutex de synchro */
    struct LIBRAIRIE *lib;
    gchar nom_absolu[128];
    GSList *liste;

    liste = Partage->com_msrv.Librairies;
    while (liste)
     { struct LIBRAIRIE *lib;
       lib = (struct LIBRAIRIE *)liste->data;
       if ( ! strcmp( lib->admin_prompt, prompt ) )
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Librairie %s already loaded", __func__, prompt );
          return(NULL);
        }
       liste=liste->next;
     }

    lib = (struct LIBRAIRIE *) g_try_malloc0( sizeof ( struct LIBRAIRIE ) );
    if (!lib) { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: MemoryAlloc failed", __func__ );
                return(NULL);
              }

    g_snprintf( nom_absolu, sizeof(nom_absolu), "%s/libwatchdog-server-%s.so", Config.librairie_dir, prompt );

    lib->dl_handle = dlopen( nom_absolu, RTLD_GLOBAL | RTLD_NOW );
    if (!lib->dl_handle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Candidat %s failed (%s)", __func__, nom_absolu, dlerror() );
       g_free(lib);
       return(NULL);
     }

    lib->Run_thread = dlsym( lib->dl_handle, "Run_thread" );                                      /* Recherche de la fonction */
    if (!lib->Run_thread)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Candidat %s rejected (Run_thread not found)", __func__, nom_absolu );
       dlclose( lib->dl_handle );
       g_free(lib);
       return(NULL);
     }

    lib->Admin_json = dlsym( lib->dl_handle, "Admin_json" );                                      /* Recherche de la fonction */
    g_snprintf( lib->admin_prompt, sizeof(lib->admin_prompt), "%s", prompt );
    g_snprintf( lib->nom_fichier,  sizeof(lib->nom_fichier),  "%s", nom_absolu );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: %s loaded", __func__, nom_absolu );

    pthread_mutexattr_init( &attr );                                                          /* Creation du mutex de synchro */
    pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_SHARED );
    pthread_mutex_init( &lib->synchro, &attr );

    if ( !strcasecmp( prompt, "http" ) ) Start_librairie( lib );
    else
     { gchar *enable = Recuperer_configDB_by_nom ( prompt, "enable" );
       if ( enable && !strcasecmp ( enable, "true" ) ) { Start_librairie( lib ); g_free(enable); }
       else { Info_new( Config.log, Config.log_msrv, LOG_INFO,
                        "%s: Librairie '%s' is not enabled : Loaded but not started", __func__, prompt );
            }
     }

    Partage->com_msrv.Librairies = g_slist_prepend( Partage->com_msrv.Librairies, lib );
    return(lib);
  }
/******************************************************************************************************************************/
/* Decharger_librairie_par_nom: Decharge la librairie dont le nom est en paramètre                                            */
/* Entrée: Le nom de la librairie                                                                                             */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 gboolean Decharger_librairie_par_prompt ( gchar *prompt )
  { struct LIBRAIRIE *lib;
    GSList *liste;

    liste = Partage->com_msrv.Librairies;
    while(liste)                                                                            /* Liberation mÃémoire des modules */
     { lib = (struct LIBRAIRIE *)liste->data;                                         /* RÃécupÃération des donnÃées de la liste */

       if ( ! strcmp ( lib->admin_prompt, prompt ) )
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: trying to unload %s", __func__, lib->nom_fichier );

          Stop_librairie(lib);
          pthread_mutex_destroy( &lib->synchro );
          dlclose( lib->dl_handle );
          Partage->com_msrv.Librairies = g_slist_remove( Partage->com_msrv.Librairies, lib );
                                                                             /* Destruction de l'entete associÃé dans la GList */
          Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: library %s unloaded", __func__, lib->nom_fichier );
          g_free( lib );
          return(TRUE);
        }
       liste = liste->next;
     }
   Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: library %s not found", __func__, prompt );
   return(FALSE);
  }
/******************************************************************************************************************************/
/* Decharger_librairies: Decharge toutes les librairies                                                                       */
/* EntrÃée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Decharger_librairies ( void )
  { struct LIBRAIRIE *lib;
    GSList *liste;

    liste = Partage->com_msrv.Librairies;                 /* Envoie une commande d'arret pour toutes les librairies d'un coup */
    while(liste)
     { lib = (struct LIBRAIRIE *)liste->data;
       lib->Thread_run = FALSE;                                                          /* On demande au thread de s'arreter */
       liste = liste->next;
     }

    while(Partage->com_msrv.Librairies)                                                     /* Liberation mémoire des modules */
     { lib = (struct LIBRAIRIE *)Partage->com_msrv.Librairies->data;
       Decharger_librairie_par_prompt (lib->admin_prompt);
     }
  }
/******************************************************************************************************************************/
/* Charger_librairies: Ouverture de toutes les librairies possibles pour Watchdog                                             */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_librairies ( void )
  { struct dirent *fichier;
    DIR *repertoire;

    repertoire = opendir ( Config.librairie_dir );                                  /* Ouverture du répertoire des librairies */
    if (!repertoire)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Directory %s Unknown", __func__, Config.librairie_dir );
       return;
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Loading Directory %s in progress", __func__, Config.librairie_dir );
     }

    while( (fichier = readdir( repertoire )) )                                      /* Pour chacun des fichiers du répertoire */
     { gchar prompt[64];
       if (!strncmp( fichier->d_name, "libwatchdog-server-", 19 ))                     /* Chargement unitaire d'une librairie */
        { if ( ! strncmp( fichier->d_name + strlen(fichier->d_name) - 3, ".so", 4 ) )
           { g_snprintf( prompt, strlen(fichier->d_name)-21, "%s", fichier->d_name + 19 );
             Charger_librairie_par_prompt( prompt );
           }
        }
     }
    closedir( repertoire );                                                 /* Fermeture du rÃ©pertoire a la fin du traitement */

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: %d Library loaded", __func__, g_slist_length( Partage->com_msrv.Librairies ) );
  }
/******************************************************************************************************************************/
/* Demarrer_dls: Thread un process DLS                                                                                        */
/* EntrÃ©e: rien                                                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_dls ( void )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Demande de demarrage %d", __func__, getpid() );
    if (Partage->com_dls.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: An instance is already running %d",__func__, Partage->com_dls.TID );
       return(FALSE);
     }
    memset( &Partage->com_dls, 0, sizeof(Partage->com_dls) );                       /* Initialisation des variables du thread */
    if ( pthread_create( &Partage->com_dls.TID, NULL, (void *)Run_dls, NULL ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: pthread_create failed", __func__ );
       return(FALSE);
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: thread dls (%p) seems to be running", __func__, Partage->com_dls.TID );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Demarrer_arch: Thread un process arch                                                                                      */
/* EntrÃée: rien                                                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Demarrer_arch ( void )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Demande de demarrage %d", __func__, getpid() );
    if (Partage->com_arch.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: An instance is already running", __func__, Partage->com_arch.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_arch.TID, NULL, (void *)Run_arch, NULL ))
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: pthread_create failed", __func__ );
       return(FALSE);
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: thread arch (%p) seems to be running", __func__, Partage->com_arch.TID );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Stopper_fils: arret de tous les fils Watchdog                                                                              */
/* Entré/Sortie: néant                                                                                                        */
/******************************************************************************************************************************/
 void Stopper_fils ( void )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Debut stopper_fils", __func__ );

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Waiting for DLS (%p) to finish", __func__, Partage->com_dls.TID );
    Partage->com_dls.Thread_run = FALSE;
    while ( Partage->com_dls.TID != 0 ) sched_yield();                                                     /* Attente fin DLS */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: ok, DLS is down", __func__ );

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Waiting for ARCH (%p) to finish", __func__, Partage->com_arch.TID );
    Partage->com_arch.Thread_run = FALSE;
    while ( Partage->com_arch.TID != 0 ) sched_yield();                                                    /* Attente fin DLS */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: ok, ARCH is down", __func__ );

    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Fin stopper_fils", __func__ );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
