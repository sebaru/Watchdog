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

 static gint      PID_motion   = 0;                                            /* Le PID de motion detect */

 extern gint Socket_ecoute;                                  /* Socket de connexion (d'écoute) du serveur */
 extern SSL_CTX *Ssl_ctx;                                          /* Contexte de cryptage des connexions */

/**********************************************************************************************************/
/* Start_librairie: Demarre le thread en paremetre                                                        */
/* Entrée: La structure associée au thread                                                                */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 gboolean Start_librairie ( struct LIBRAIRIE *lib )
  { if (!lib) return(FALSE);
    if (lib->Thread_run == TRUE)
     { Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "Start_librairie: thread %s already seems to be running", lib->nom_fichier );
       return(FALSE);
     }
    if ( pthread_create( &lib->TID, NULL, (void *)lib->Run_thread, lib ) )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Start_librairie: pthread_create failed (%s)",
                lib->nom_fichier );
       return(FALSE);
     }
    pthread_detach( lib->TID );       /* On le détache pour qu'il puisse se terminer sur erreur tout seul */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
             "Start_librairie: Starting thread %s OK", lib->nom_fichier );
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
     }

    lib->Thread_run = FALSE;                                         /* On demande au thread de s'arreter */
    while( lib->TID!=0 ) sched_yield();                                             /* Attente fin thread */

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
             "Stop_librairie: thread %s stopped", lib->nom_fichier );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Charger_librarie_par_fichier: Ouverture d'une librairie par son nom                                    */
/* Entrée: Le nom de fichier correspondant                                                                */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 struct LIBRAIRIE *Charger_librairie_par_fichier ( gchar *path, gchar *nom_fichier )
  { pthread_mutexattr_t attr;                                      /* Initialisation des mutex de synchro */
    struct LIBRAIRIE *lib;
    gchar nom_absolu[128];
    GSList *liste;

    liste = Partage->com_msrv.Librairies;
    while (liste)
     { struct LIBRAIRIE *lib;
       lib = (struct LIBRAIRIE *)liste->data;
       if ( ! strcmp( lib->nom_fichier, nom_fichier ) )
        { Info_new( Config.log, Config.log_msrv, LOG_INFO,
                   "Charger_librairie_par_fichier: Librairie %s already loaded", nom_fichier );
          return(NULL);
        }
       liste=liste->next;
     }

    lib = (struct LIBRAIRIE *) g_try_malloc0( sizeof ( struct LIBRAIRIE ) );
    if (!lib) { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                          "Charger_librairie_par_fichier: MemoryAlloc failed" );
                return(NULL);
              }

    if (path) { g_snprintf( nom_absolu, sizeof(nom_absolu), "%s/%s", path, nom_fichier );
                lib->dl_handle = dlopen( nom_absolu, RTLD_GLOBAL | RTLD_NOW );
              }
         else { lib->dl_handle = dlopen( nom_fichier, RTLD_GLOBAL | RTLD_NOW ); }
    if (!lib->dl_handle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                 "Charger_librairie_par_fichier Candidat: %s failed (%s)", nom_fichier, dlerror() );
       g_free(lib);
       return(NULL);
     }

    lib->Run_thread = dlsym( lib->dl_handle, "Run_thread" );                  /* Recherche de la fonction */
    if (!lib->Run_thread)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Charger_librairie_par_fichier: Candidat %s rejeté sur absence Run_thread", nom_fichier ); 
       dlclose( lib->dl_handle );
       g_free(lib);
       return(NULL);
     }

    lib->Admin_command = dlsym( lib->dl_handle, "Admin_command" );            /* Recherche de la fonction */
    if (!lib->Admin_command)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Charger_librairie_par_fichier: Candidat %s rejeté sur absence Admin_command", nom_fichier ); 
       dlclose( lib->dl_handle );
       g_free(lib);
       return(NULL);
     }

    g_snprintf( lib->nom_fichier, sizeof(lib->nom_fichier), "%s", nom_fichier );

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_librairie_par_fichier: %s loaded", nom_fichier );

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
     { if (!strncmp( fichier->d_name, "libwatchdog-server-", 19 )) /* Chargement unitaire d'une librairie */
        { if ( ! strncmp( fichier->d_name + strlen(fichier->d_name) - 3, ".so", 4 ) )
           { struct LIBRAIRIE *lib;
             lib = Charger_librairie_par_fichier( Config.librairie_dir, fichier->d_name );
#ifdef bouh
             Start_librairie( lib );
#endif
           }
        }
     }
    closedir( repertoire );                             /* Fermeture du répertoire a la fin du traitement */

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_librairies: %d Library loaded",
              g_slist_length( Partage->com_msrv.Librairies ) );
  }
/**********************************************************************************************************/
/* Demarrer_config_file_motion                                                                            */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 static gboolean Creer_config_file_motion ( void )
  { gchar chaine[80];
    struct DB *db;
    gint id;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Creer_config_file_motion: Connexion DB failed" );
       return(FALSE);
     }                                                                                  /* Si pas d'accès */

    if ( !Recuperer_cameraDB( Config.log, db ) )                      /* Préparation du chargement camera */
     { Libere_DB_SQL( &db );
       return(FALSE);
     }                                                                         /* Si pas d'enregistrement */

    unlink("motion.conf");                                      /* Création des fichiers de configuration */
    id = open ( "motion.conf", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id<0) { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                          "Creer_config_file_motion: creation motion.conf pid file failed %d", id );
                return(FALSE);
              }
    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Creer_config_file_motion: creation motion.conf %d", id );
#ifdef bouh
    g_snprintf(chaine, sizeof(chaine), "control_port 8080\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "control_localhost off\n");
    write(id, chaine, strlen(chaine));
#endif
    g_snprintf(chaine, sizeof(chaine), "process_id_file %s/Camera/motion.pid\n", Config.home);
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "daemon on\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "framerate 25\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "netcam_http 1.1\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "despeckle EedDl\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "output_normal off\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "ffmpeg_cap_new on\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "ffmpeg_bps 500000\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "ffmpeg_video_codec mpeg4\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "text_right %%Y-%%m-%%d\\n%%T-%%q\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "target_dir %s/Camera\n", Config.home);
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "mysql_db %s\n", Config.db_database);
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "mysql_host %s\n", Config.db_host);
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "mysql_user %s\n", Config.db_username);
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "mysql_password %s\n", Config.db_password);
    write(id, chaine, strlen(chaine));

    for ( ; ; )
     { struct CMD_TYPE_CAMERA *camera;
       gchar nom_fichier[80];
       gint id_camera;
       camera = Recuperer_cameraDB_suite( Config.log, db );
       if (!camera)
        { Libere_DB_SQL( &db );
          break;
        }
       g_snprintf(nom_fichier, sizeof(nom_fichier), "Camera/camera%04d.conf", camera->id);

       g_snprintf(chaine, sizeof(chaine), "thread %s/Camera/%s\n", Config.home, nom_fichier);
       write(id, chaine, strlen(chaine));
       
       unlink(nom_fichier);
       id_camera = open ( nom_fichier, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
       if (id<0) { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                           "Creer_config_file_motion: creation camera.conf failed %d", id );
                   g_free(camera);
                   close(id);
                   return(FALSE);
                 }
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Creer_config_file_motion: creation thread camera", camera->num );
       g_snprintf(chaine, sizeof(chaine), "netcam_url %s\n", camera->location);
       write(id_camera, chaine, strlen(chaine));
       g_snprintf(chaine, sizeof(chaine), "sql_query insert into cameras_motion (id) values (%d)\n",
                  camera->id);
       write(id_camera, chaine, strlen(chaine));
       g_snprintf(chaine, sizeof(chaine), "text_left CAM%04d %s\n", camera->num, camera->objet);
       write(id_camera, chaine, strlen(chaine));
       g_snprintf(chaine, sizeof(chaine), "movie_filename CAM%04d-%%Y%%m%%d%%H%%M%%S\n", camera->num);
       write(id_camera, chaine, strlen(chaine));

       close(id_camera);
       g_free(camera);
     }
    close(id);
    return(TRUE);          
  }
/**********************************************************************************************************/
/* Demarrer_onduleur: Thread un process ONDULEUR                                                          */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_motion_detect ( void )
  { gchar chaine[80];
    gint id;
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Demarrer_motion_detect: Demande de demarrage %d", getpid() );

    if (!Creer_config_file_motion()) return(FALSE);

    g_snprintf(chaine, sizeof(chaine), "%s/Camera/motion.pid", Config.home);
    id = open ( chaine, O_RDONLY, 0 );
    if (id<0) { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Demarrer_motion_detect: ouverture pid file failed %d", id );
                return(FALSE);
              }

    if (read ( id, chaine, sizeof(chaine) )<0)
              { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Demarrer_motion_detect: erreur lecture pid file %d", id );
                close(id);
                return(FALSE);
              }
    close(id);
    PID_motion = atoi (chaine);
    kill( PID_motion, SIGHUP );                                                   /* Envoie reload conf a motion */
    Info_new( Config.log, Config.log_msrv, LOG_WARNING,
           "Demarrer_motion_detect: process motion seems to be running %d", PID_motion );
    return(TRUE);
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
    if ( pthread_create( &Partage->com_dls.TID, NULL, (void *)Run_dls, NULL ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Demarrer_dls: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_dls.TID );      /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Demarrer_dls: thread dls (%d) seems to be running",
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
              "Demarrer_admin: thread admin (%d) seems to be running",
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
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Demarrer_arch: thread arch (%d) seems to be running",
            Partage->com_arch.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Stopper_fils: arret de tous les fils Watchdog                                                          */
/* Entrée/Sortie: flag = TRUE si on demande aussi l'arret du thread Admin                                 */
/**********************************************************************************************************/
 void Stopper_fils ( gint flag )
  { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: Debut stopper_fils" );

    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: Waiting for DLS (%d) to finish", Partage->com_dls.TID );
    Partage->com_dls.Thread_run = FALSE;
    while ( Partage->com_dls.TID != 0 ) sched_yield();                                 /* Attente fin DLS */
    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: ok, DLS is down" );

    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: Waiting for ARCH (%d) to finish", Partage->com_arch.TID );
    Partage->com_arch.Thread_run = FALSE;
    while ( Partage->com_arch.TID != 0 ) sched_yield();                       /* Attente fin ONDULEUR */
    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: ok, ARCH is down" );

    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: keep MOTION (%d) running", PID_motion );

    if (flag)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: Waiting for ADMIN (%d) to finish", Partage->com_admin.TID );
       Partage->com_admin.Thread_run = FALSE;
       while ( Partage->com_admin.TID != 0 ) sched_yield();                       /* Attente fin ONDULEUR */
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: ok, ADMIN is down" );
     }

    Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Stopper_fils: Fin stopper_fils" );
  }
/*--------------------------------------------------------------------------------------------------------*/
