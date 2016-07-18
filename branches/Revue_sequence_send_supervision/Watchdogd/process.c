/**********************************************************************************************************/
/* Watchdogd/process.c        Gestion des process                                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 11 avr 2009 12:21:45 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * process.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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
 #include <unistd.h>
 #include <stdlib.h>
 #include <dirent.h>
 #include <string.h>
 #include <stdio.h>

 #include <sys/wait.h>
 #include <fcntl.h>
 #include <errno.h>
 #include <openssl/ssl.h>
 #include <dlfcn.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"

 extern gint Socket_ecoute;                                  /* Socket de connexion (d'écoute) du serveur */
 extern SSL_CTX *Ssl_ctx;                                          /* Contexte de cryptage des connexions */

/**********************************************************************************************************/
/* Start_librairie: Demarre le thread en paremetre                                                        */
/* Entrée: La structure associée au thread                                                                */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 gboolean Start_librairie ( struct LIBRAIRIE *lib )
  { pthread_attr_t attr;
    pthread_t tid;
    if (!lib) return(FALSE);
    if (lib->Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "Start_librairie: thread %s already seems to be running", lib->nom_fichier );
       return(FALSE);
     }

    if ( pthread_attr_init(&attr) )                             /* Initialisation des attributs du thread */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "Start_librairie: pthread_attr_init failed (%s)",
                lib->nom_fichier );
       return(FALSE);
     }

    if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) )   /* On le laisse joinable au boot */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "Start_librairie: pthread_setdetachstate failed (%s)",
                lib->nom_fichier );
       return(FALSE);
     }

    if ( pthread_create( &tid, &attr, (void *)lib->Run_thread, lib ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "Start_librairie: pthread_create failed (%s)",
                lib->nom_fichier );
       return(FALSE);
     }
    pthread_attr_destroy(&attr);                                                    /* Libération mémoire */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
             "Start_librairie: Starting thread %s OK (TID=%p)", lib->nom_fichier, tid );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Stop_librairie: Arrete le thread en paremetre                                                          */
/* Entrée: La structure associée au thread                                                                */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 gboolean Stop_librairie ( struct LIBRAIRIE *lib )
  { if (!lib) return(FALSE);
    if ( lib->TID != 0 )
     { Info_new( Config.log, Config.log_msrv, LOG_INFO,
                "Stop_librairie: thread %s, stopping in progress", lib->nom_fichier );

       lib->Thread_run = FALSE;                                      /* On demande au thread de s'arreter */
       while( lib->TID != 0 ) sched_yield();                                        /* Attente fin thread */
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
             "Stop_librairie: thread %s stopped", lib->nom_fichier );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Charger_librarie_par_fichier: Ouverture d'une librairie par son nom                                    */
/* Entrée: Le nom de fichier correspondant                                                                */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 struct LIBRAIRIE *Charger_librairie_par_prompt ( gchar *nom_prompt )
  { pthread_mutexattr_t attr;                                      /* Initialisation des mutex de synchro */
    struct LIBRAIRIE *lib;
    gchar nom_absolu[128];
    GSList *liste;

    liste = Partage->com_msrv.Librairies;
    while (liste)
     { struct LIBRAIRIE *lib;
       lib = (struct LIBRAIRIE *)liste->data;
       if ( ! strcmp( lib->admin_prompt, nom_prompt ) )
        { Info_new( Config.log, Config.log_msrv, LOG_INFO,
                   "Charger_librairie_par_prompt: Librairie %s already loaded", nom_prompt );
          return(NULL);
        }
       liste=liste->next;
     }

    lib = (struct LIBRAIRIE *) g_try_malloc0( sizeof ( struct LIBRAIRIE ) );
    if (!lib) { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                          "Charger_librairie_par_fichier: MemoryAlloc failed" );
                return(NULL);
              }

    g_snprintf( nom_absolu, sizeof(nom_absolu), "%s/libwatchdog-server-%s.so",
                Config.librairie_dir, nom_prompt );

    lib->dl_handle = dlopen( nom_absolu, RTLD_GLOBAL | RTLD_NOW );
    if (!lib->dl_handle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                 "Charger_librairie_par_prompt: Candidat %s failed (%s)", nom_absolu, dlerror() );
       g_free(lib);
       return(NULL);
     }

    lib->Run_thread = dlsym( lib->dl_handle, "Run_thread" );                  /* Recherche de la fonction */
    if (!lib->Run_thread)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Charger_librairie_par_prompt: Candidat %s rejected (Run_thread not found)", nom_absolu ); 
       dlclose( lib->dl_handle );
       g_free(lib);
       return(NULL);
     }

    lib->Admin_command = dlsym( lib->dl_handle, "Admin_command" );            /* Recherche de la fonction */
    if (!lib->Admin_command)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Charger_librairie_par_prompt: Candidat %s rejected (Admin_command not found)", nom_absolu ); 
       dlclose( lib->dl_handle );
       g_free(lib);
       return(NULL);
     }

    g_snprintf( lib->nom_fichier, sizeof(lib->nom_fichier), "%s", nom_absolu );

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_librairie_par_prompt: %s loaded", nom_absolu );

    pthread_mutexattr_init( &attr );                                      /* Creation du mutex de synchro */
    pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_SHARED );
    pthread_mutex_init( &lib->synchro, &attr );

    Partage->com_msrv.Librairies = g_slist_prepend( Partage->com_msrv.Librairies, lib );
    return(lib);
  }
/**********************************************************************************************************/
/* Decharger_librairie_par_nom: Decharge la librairie dont le nom est en paramètre                        */
/* Entrée: Le nom de la librairie                                                                         */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 gboolean Decharger_librairie_par_prompt ( gchar *prompt )
  { struct LIBRAIRIE *lib;
    GSList *liste;

    liste = Partage->com_msrv.Librairies;
    while(liste)                                                        /* Liberation mémoire des modules */
     { lib = (struct LIBRAIRIE *)liste->data;                     /* Récupération des données de la liste */

       if ( ! strcmp ( lib->admin_prompt, prompt ) )
        { Info_new( Config.log, Config.log_msrv, LOG_INFO,
                    "Decharger_librairie_par_prompt: trying to unload %s", lib->nom_fichier );

          Stop_librairie(lib);
          sleep(5);
          pthread_mutex_destroy( &lib->synchro );
          dlclose( lib->dl_handle );
          Partage->com_msrv.Librairies = g_slist_remove( Partage->com_msrv.Librairies, lib );
                                                         /* Destruction de l'entete associé dans la GList */
          Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                    "Decharger_librairie_par_prompt: library %s unloaded", lib->nom_fichier );
          g_free( lib );
          return(TRUE);
        }
       liste = liste->next;
     }
   Info_new( Config.log, Config.log_msrv, LOG_ERR, "Decharger_librairie_par_prompt: library %s not found", prompt );
   return(FALSE);
  }
/**********************************************************************************************************/
/* Decharger_librairies: Decharge toutes les librairies                                                   */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Decharger_librairies ( void )
  { struct LIBRAIRIE *lib;

    while(Partage->com_msrv.Librairies)                                 /* Liberation mémoire des modules */
     { lib = (struct LIBRAIRIE *)Partage->com_msrv.Librairies->data;
       Decharger_librairie_par_prompt (lib->admin_prompt);
     }
  }
/**********************************************************************************************************/
/* Charger_librairies: Ouverture de toutes les librairies possibles pour Watchdog                         */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Charger_librairies ( void )
  { struct dirent *fichier;
    DIR *repertoire;

    repertoire = opendir ( Config.librairie_dir );                    /* Ouverture du répertoire des librairies */
    if (!repertoire)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "Charger_librairies: Directory %s Unknown", Config.librairie_dir );
       return;
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                "Charger_librairies: Loading Directory %s in progress", Config.librairie_dir );
     }

    while( (fichier = readdir( repertoire )) )                  /* Pour chacun des fichiers du répertoire */
     { gchar prompt[64];
       if (!strncmp( fichier->d_name, "libwatchdog-server-", 19 )) /* Chargement unitaire d'une librairie */
        { if ( ! strncmp( fichier->d_name + strlen(fichier->d_name) - 3, ".so", 4 ) )
           { struct LIBRAIRIE *lib;
             g_snprintf( prompt, strlen(fichier->d_name)-21, "%s", fichier->d_name + 19 );
             lib = Charger_librairie_par_prompt( prompt );
             Start_librairie( lib );
           }
        }
     }
    closedir( repertoire );                             /* Fermeture du répertoire a la fin du traitement */

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_librairies: %d Library loaded",
              g_slist_length( Partage->com_msrv.Librairies ) );
  }
/**********************************************************************************************************/
/* Demarrer_dls: Thread un process DLS                                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_dls ( void )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Demarrer_dls: Demande de demarrage %d", getpid() );
    if (Partage->com_dls.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Demarrer_dls: An instance is already running %d",
               Partage->com_dls.TID );
       return(FALSE);
     }
    memset( &Partage->com_dls, 0, sizeof(Partage->com_dls) );   /* Initialisation des variables du thread */
    if ( pthread_create( &Partage->com_dls.TID, NULL, (void *)Run_dls, NULL ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Demarrer_dls: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_dls.TID );      /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Demarrer_dls: thread dls (%p) seems to be running",
              Partage->com_dls.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_admin: Thread un process admin                                                                */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_admin ( void )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Demarrer_admin: Demande de demarrage %d", getpid() );
    if (Partage->com_admin.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                 "Demarrer_admin: An instance is already running (%d)",
                 Partage->com_admin.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_admin.TID, NULL, (void *)Run_admin, NULL ))
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Demarrer_admin: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_admin.TID ); /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
              "Demarrer_admin: thread admin (%p) seems to be running",
              Partage->com_admin.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_arch: Thread un process arch                                                                  */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_arch ( void )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Demarrer_arch: Demande de demarrage %d", getpid() );
    if (Partage->com_arch.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Demarrer_arch: An instance is already running",
                 Partage->com_arch.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_arch.TID, NULL, (void *)Run_arch, NULL ))
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Demarrer_arch: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_arch.TID ); /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Demarrer_arch: thread arch (%p) seems to be running",
              Partage->com_arch.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Stopper_fils: arret de tous les fils Watchdog                                                          */
/* Entrée/Sortie: flag = TRUE si on demande aussi l'arret du thread Admin                                 */
/**********************************************************************************************************/
 void Stopper_fils ( gint flag )
  { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: Debut stopper_fils" );

    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: Waiting for DLS (%p) to finish", Partage->com_dls.TID );
    Partage->com_dls.Thread_run = FALSE;
    while ( Partage->com_dls.TID != 0 ) sched_yield();                                 /* Attente fin DLS */
    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: ok, DLS is down" );

    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: Waiting for ARCH (%p) to finish", Partage->com_arch.TID );
    Partage->com_arch.Thread_run = FALSE;
    while ( Partage->com_arch.TID != 0 ) sched_yield();                            /* Attente fin Archive */
    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: ok, ARCH is down" );

    if (flag)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: Waiting for ADMIN (%p) to finish", Partage->com_admin.TID );
       Partage->com_admin.Thread_run = FALSE;
       while ( Partage->com_admin.TID != 0 ) sched_yield();                          /* Attente fin ADMIN */
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: ok, ADMIN is down" );
     }

    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: Fin stopper_fils" );
  }
/*--------------------------------------------------------------------------------------------------------*/
