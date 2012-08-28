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

 #include <bonobo/bonobo-i18n.h>
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
     { Info_new( Config.log, Config.log_all, LOG_INFO,
                 "Start_librairie: thread %s already seems to be running", lib->nom_fichier );
       return(FALSE);
     }
    if ( pthread_create( &lib->TID, NULL, (void *)lib->Run_thread, lib ) )
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
                "Start_librairie: pthread_create failed (%s)",
                lib->nom_fichier );
       return(FALSE);
     }
    pthread_detach( lib->TID );       /* On le détache pour qu'il puisse se terminer sur erreur tout seul */
    Info_new( Config.log, Config.log_all, LOG_NOTICE,
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
    if (lib->Thread_run == FALSE)
     { Info_new( Config.log, Config.log_all, LOG_INFO,
               "Stop_librairie: thread %s already stopped", lib->nom_fichier );
       return(FALSE);
     }

    lib->Thread_run = FALSE;                                         /* On demande au thread de s'arreter */
    while( lib->TID!=0 ) sched_yield();                                             /* Attente fin thread */

    Info_new( Config.log, Config.log_all, LOG_NOTICE,
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
        { Info_new( Config.log, Config.log_all, LOG_INFO,
                   "Charger_librairie_par_fichier: Librairie %s already loaded", nom_fichier );
          return(NULL);
        }
       liste=liste->next;
     }

    lib = (struct LIBRAIRIE *) g_malloc0( sizeof ( struct LIBRAIRIE ) );
    if (!lib) { Info_new( Config.log, Config.log_all, LOG_WARNING,
                          "Charger_librairie_par_fichier: MemoryAlloc failed" );
                return(NULL);
              }

    if (path) { g_snprintf( nom_absolu, sizeof(nom_absolu), "%s/%s", path, nom_fichier );
                lib->dl_handle = dlopen( nom_absolu, RTLD_LAZY );
              }
         else { lib->dl_handle = dlopen( nom_fichier, RTLD_LAZY ); }
    if (!lib->dl_handle)
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
                 "Charger_librairie_par_fichier Candidat: %s rejeté (%s)", nom_fichier, dlerror() );
       g_free(lib);
       return(NULL);
     }

    lib->Run_thread = dlsym( lib->dl_handle, "Run_thread" );                  /* Recherche de la fonction */
    if (!lib->Run_thread)
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
                "Charger_librairie_par_fichier: Candidat %s rejeté sur absence Run_thread", nom_fichier ); 
       dlclose( lib->dl_handle );
       g_free(lib);
       return(NULL);
     }

    lib->Admin_command = dlsym( lib->dl_handle, "Admin_command" );            /* Recherche de la fonction */
    if (!lib->Admin_command)
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
                "Charger_librairie_par_fichier: Candidat %s rejeté sur absence Admin_command", nom_fichier ); 
       dlclose( lib->dl_handle );
       g_free(lib);
       return(NULL);
     }

    g_snprintf( lib->nom_fichier, sizeof(lib->nom_fichier), "%s", nom_fichier );

    Info_new( Config.log, Config.log_all, LOG_INFO, "Charger_librairie_par_fichier: %s loaded", nom_fichier );

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
        { Info_new( Config.log, Config.log_all, LOG_INFO,
                    "Decharger_librairie_par_prompt: trying to unload %s", lib->nom_fichier );

          Stop_librairie(lib);

          pthread_mutex_destroy( &lib->synchro );
          dlclose( lib->dl_handle );
          Partage->com_msrv.Librairies = g_slist_remove( Partage->com_msrv.Librairies, lib );
                                                         /* Destruction de l'entete associé dans la GList */
          Info_new( Config.log, Config.log_all, LOG_NOTICE,
                    "Decharger_librairie_par_prompt: library %s unloaded", lib->nom_fichier );
          g_free( lib );
          return(TRUE);
        }
       liste = liste->next;
     }
   Info_new( Config.log, Config.log_all, LOG_ERR, "Decharger_librairie_par_prompt: library %s not found", prompt );
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
     { Info_new( Config.log, Config.log_all, LOG_ERR, "Charger_librairies: Directory %s Unknown", "/usr/local/lib" );
       return;
     }

    while( (fichier = readdir( repertoire )) )                  /* Pour chacun des fichiers du répertoire */
     { if (!strncmp( fichier->d_name, "libwatchdog-server-", 19 )) /* Chargement unitaire d'une librairie */
        { if ( ! strncmp( fichier->d_name + strlen(fichier->d_name) - 3, ".so", 4 ) )
           { struct LIBRAIRIE *lib;
             lib = Charger_librairie_par_fichier( Config.librairie_dir, fichier->d_name );
             Start_librairie( lib );
           }
        }
     }
    closedir( repertoire );                             /* Fermeture du répertoire a la fin du traitement */

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Charger_librairies: %d Library loaded",
              g_slist_length( Partage->com_msrv.Librairies ) );
  }
/**********************************************************************************************************/
/* Demarrer_onduleur: Thread un process ONDULEUR                                                          */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 static gboolean Creer_config_file_motion ( void )
  { gchar chaine[80];
    struct DB *db;
    gint id;

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Creer_config_file_motion: Connexion DB failed" );
       return(FALSE);
     }                                                                                  /* Si pas d'accès */

    if ( !Recuperer_cameraDB( Config.log, db ) )                      /* Préparation du chargement camera */
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }                                                                         /* Si pas d'enregistrement */

    unlink("motion.conf");                                      /* Création des fichiers de configuration */
    id = open ( "motion.conf", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id<0) { Info_new( Config.log, Config.log_all, LOG_WARNING,
                          "Creer_config_file_motion: creation motion.conf pid file failed %d", id );
                return(FALSE);
              }
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Creer_config_file_motion: creation motion.conf %d", id );
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
        { Libere_DB_SQL( Config.log, &db );
          break;
        }
       g_snprintf(nom_fichier, sizeof(nom_fichier), "Camera/camera%04d.conf", camera->id);

       g_snprintf(chaine, sizeof(chaine), "thread %s/Camera/%s\n", Config.home, nom_fichier);
       write(id, chaine, strlen(chaine));
       
       unlink(nom_fichier);
       id_camera = open ( nom_fichier, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
       if (id<0) { Info_new( Config.log, Config.log_all, LOG_WARNING,
                           "Creer_config_file_motion: creation camera.conf failed %d", id );
                   g_free(camera);
                   close(id);
                   return(FALSE);
                 }
       Info_new( Config.log, Config.log_all, LOG_WARNING, "Creer_config_file_motion: creation thread camera", camera->num );
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
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_motion_detect: Demande de demarrage %d", getpid() );

    if (!Creer_config_file_motion()) return(FALSE);

    g_snprintf(chaine, sizeof(chaine), "%s/Camera/motion.pid", Config.home);
    id = open ( chaine, O_RDONLY, 0 );
    if (id<0) { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_motion_detect: ouverture pid file failed %d", id );
                return(FALSE);
              }

    if (read ( id, chaine, sizeof(chaine) )<0)
              { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_motion_detect: erreur lecture pid file %d", id );
                close(id);
                return(FALSE);
              }
    close(id);
    PID_motion = atoi (chaine);
    kill( PID_motion, SIGHUP );                                                   /* Envoie reload conf a motion */
    Info_new( Config.log, Config.log_all, LOG_WARNING,
           "Demarrer_motion_detect: process motion seems to be running %d", PID_motion );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_sous_serveur: Fork un sous_serveur                                                            */
/* Entrée: l'id du fils                                                                                   */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_sous_serveur ( int id )
  { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_sous_serveur: Demande de demarrage %d", id );
    if (Partage->Sous_serveur[id].Thread_run == TRUE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_sous_serveur: An instance is already running",
               Partage->Sous_serveur[id].pid );
       return(FALSE);
     }
    if ( pthread_create( &Partage->Sous_serveur[id].pid, NULL, (void *)Run_serveur, GINT_TO_POINTER(id) ) )
     { Info_new( Config.log, Config.log_all, LOG_INFO, "Demarrer_sous_serveur: pthread_create failed %s", strerror(errno) );
       return(FALSE);
     }
    else pthread_detach( Partage->Sous_serveur[id].pid );            /* On détache le thread Sous-Serveur */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_sous_serveur %d", id );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_onduleur: Thread un process ONDULEUR                                                          */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_onduleur ( void )
  { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_onduleur: Demande de demarrage %d", getpid() );
    if (Partage->com_onduleur.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_onduleur: An instance is already running %d",
               Partage->com_onduleur.TID );
       return(FALSE);
     }
    if ( pthread_create( &Partage->com_onduleur.TID, NULL, (void *)Run_onduleur, NULL ) )
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_onduleur: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_onduleur.TID ); /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_onduleur: thread onduleur seems to be running %d",
            Partage->com_onduleur.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_dls: Thread un process DLS                                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_dls ( void )
  { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_dls: Demande de demarrage %d", getpid() );
    if (Partage->com_dls.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_dls: An instance is already running %d",
               Partage->com_dls.TID );
       return(FALSE);
     }
    if ( pthread_create( &Partage->com_dls.TID, NULL, (void *)Run_dls, NULL ) )
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_dls: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_dls.TID );      /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_dls: thread dls seems to be running %d",
              Partage->com_dls.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_tellstick: Thread un process TELLSTICK                                                        */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_tellstick ( void )
  { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_tellstick: Demande de demarrage %d", getpid() );

    if (Partage->com_tellstick.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_tellstick: An instance is already running",
               Partage->com_tellstick.TID );
       return(FALSE);
     }

    Partage->com_tellstick.dl_handle = dlopen( "libwatchdog-tellstick.so", RTLD_LAZY );
    if (!Partage->com_tellstick.dl_handle)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "Demarrer_tellstick: dlopen failed (%s)", dlerror() );
       return(FALSE);
     }
                                                              /* Recherche de la fonction 'Run_tellstick' */
    Partage->com_tellstick.Run_tellstick = dlsym( Partage->com_tellstick.dl_handle, "Run_tellstick" );
    if (!Partage->com_tellstick.Run_tellstick)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_tellstick: Run_tellstick does not exist" );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_tellstick.dl_handle = NULL;
       return(FALSE);
     }
                                                          /* Recherche de la fonction 'Ajouter_tellstick' */
    Partage->com_tellstick.Ajouter_tellstick = dlsym( Partage->com_tellstick.dl_handle, "Ajouter_tellstick" );
    if (!Partage->com_tellstick.Ajouter_tellstick)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_tellstick: Ajouter_tellstick does not exist" );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_tellstick.dl_handle = NULL;
       return(FALSE);
     }
                                                       /* Recherche de la fonction 'Admin_tellstick_list' */
    Partage->com_tellstick.Admin_tellstick_list = dlsym( Partage->com_tellstick.dl_handle, "Admin_tellstick_list" );
    if (!Partage->com_tellstick.Admin_tellstick_list)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_tellstick: Admin_tellstick_list does not exist" );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_tellstick.dl_handle = NULL;
       return(FALSE);
     }

    Partage->com_tellstick.Admin_tellstick_learn = dlsym( Partage->com_tellstick.dl_handle, "Admin_tellstick_learn" );
    if (!Partage->com_tellstick.Admin_tellstick_learn)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_tellstick: Admin_tellstick_learn does not exist" );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_tellstick.dl_handle = NULL;
       return(FALSE);
     }

    Partage->com_tellstick.Admin_tellstick_start = dlsym( Partage->com_tellstick.dl_handle, "Admin_tellstick_start" );
    if (!Partage->com_tellstick.Admin_tellstick_start)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_tellstick: Admin_tellstick_start does not exist" );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_tellstick.dl_handle = NULL;
       return(FALSE);
     }

    Partage->com_tellstick.Admin_tellstick_stop = dlsym( Partage->com_tellstick.dl_handle, "Admin_tellstick_stop" );
    if (!Partage->com_tellstick.Admin_tellstick_stop)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_tellstick: Admin_tellstick_stop does not exist" );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_tellstick.dl_handle = NULL;
       return(FALSE);
     }

    if ( pthread_create( &Partage->com_tellstick.TID, NULL, (void *)Partage->com_tellstick.Run_tellstick, NULL ) )
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_tellstick: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_tellstick.TID ); /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_tellstick: thread tellstick seems to be running",
            Partage->com_tellstick.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_lirc: Thread un process LIRC                                                                  */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_lirc ( void )
  { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_lirc: Demande de demarrage %d", getpid() );
    if (Partage->com_lirc.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_lirc: An instance is already running",
               Partage->com_lirc.TID );
       return(FALSE);
     }

    Partage->com_lirc.dl_handle = dlopen( "libwatchdog-lirc.so", RTLD_LAZY );
    if (!Partage->com_lirc.dl_handle)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "Demarrer_lirc: dlopen failed (%s)", dlerror() );
       return(FALSE);
     }
                                                              /* Recherche de la fonction 'Run_tellstick' */
    Partage->com_lirc.Run_lirc = dlsym( Partage->com_lirc.dl_handle, "Run_lirc" );
    if (!Partage->com_lirc.Run_lirc)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_lirc: Run_lirc does not exist" );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_lirc.dl_handle = NULL;
       return(FALSE);
     }

    if ( pthread_create( &Partage->com_lirc.TID, NULL, (void *)Partage->com_lirc.Run_lirc, NULL ) )
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_lirc: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_lirc.TID ); /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_lirc: thread lirc seems to be running %d",
            Partage->com_lirc.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_sms: Thread un process sms                                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_sms ( void )
  { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_sms: Demande de demarrage %d", getpid() );
    if (Partage->com_sms.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_sms: An instance is already running",
               Partage->com_sms.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_sms.TID, NULL, (void *)Run_sms, NULL ))
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_sms: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_sms.TID ); /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_sms: thread sms seems to be running",
            Partage->com_sms.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_audio: Thread un process audio                                                                */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_audio ( void )
  { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_audio: Demande de demarrage %d", getpid() );
    if (Partage->com_audio.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_audio: An instance is already running",
               Partage->com_audio.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_audio.TID, NULL, (void *)Run_audio, NULL ))
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_audio: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_audio.TID ); /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_audio: thread audio seems to be running",
            Partage->com_audio.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_audio: Thread un process admin                                                                */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_admin ( void )
  { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_admin: Demande de demarrage %d", getpid() );
    if (Partage->com_admin.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_admin: An instance is already running",
               Partage->com_admin.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_admin.TID, NULL, (void *)Run_admin, NULL ))
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_admin: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_admin.TID ); /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_admin: thread admin seems to be running",
            Partage->com_admin.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_arch: Thread un process arch                                                                  */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_arch ( void )
  { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_arch: Demande de demarrage %d", getpid() );
    if (Partage->com_arch.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_arch: An instance is already running",
               Partage->com_arch.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_arch.TID, NULL, (void *)Run_arch, NULL ))
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_arch: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_arch.TID ); /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_arch: thread arch seems to be running",
            Partage->com_arch.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_modbus: Thread un process modbus                                                              */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_modbus ( void )
  { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_modbus: Demande de demarrage %d", getpid() );
    if (Partage->com_modbus.Thread_run == TRUE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_modbus: An instance is already running",
               Partage->com_modbus.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_modbus.TID, NULL, (void *)Run_modbus, NULL ))
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_modbus: pthread_create failed" );
       return(FALSE);
     }
    pthread_detach( Partage->com_modbus.TID ); /* On le detache pour qu'il puisse se terminer tout seul */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Demarrer_modbus: thread modbus seems to be running",
            Partage->com_arch.TID );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_empl_libre: recherche un emplacement libre dans la zone partagée Sous_serveur               */
/* Entrée: rien                                                                                           */
/* Sortie: la place libre, ou -1 si erreur                                                                */
/**********************************************************************************************************/
 static gint Rechercher_empl_libre ( void )
  { gint i;
    for (i=0; i<Config.max_serveur; i++)      /* Recherche d'un emplacement libre pour le nouveau serveur */
     { if (Partage->Sous_serveur[i].Thread_run == FALSE) return(i); }
    return(-1);
  }
/**********************************************************************************************************/
/* Rechercher_serveur_inactif: Recherche un serveur actuellement sans connexion cliente                   */
/* Entrée: rien                                                                                           */
/* Sortie: l'id du serveur inactif, ou -1 si il n'y en a pas                                              */
/**********************************************************************************************************/
 static gint Rechercher_serveur_inactif ( void )
  { gint i;
    for (i=0; i<Config.max_serveur; i++)                                /* Recherche d'un serveur inactif */
     { if (Partage->Sous_serveur[i].Thread_run == TRUE &&
           Partage->Sous_serveur[i].nb_client == 0) return(i);
     }
    return(-1);
  }
/**********************************************************************************************************/
/* Rechercher_moins_occupe: Recherche un serveur legerement chargé                                        */
/* Entrée: rien                                                                                           */
/* Sortie: l'id du serveur ou -1 si il n'y en a pas                                                       */
/**********************************************************************************************************/
 static gint Rechercher_moins_occupe ( void )
  { gint i, choix;
    choix = -1;                                                     /* On début, nous n'avons rien choisi */
    for (i=0; i<Config.max_serveur; i++)            /* Recherche d'un serveur moins chargé que les autres */
     { if (Partage->Sous_serveur[i].Thread_run == TRUE)
        { if (choix==-1) choix = i;                                                   /* Premier choix !! */
          else { if (Partage->Sous_serveur[i].nb_client < Partage->Sous_serveur[choix].nb_client)
                  { choix = i; }
               }
        }
     }
    return(choix);
  }
/**********************************************************************************************************/
/* Nb_clients: Nombre de clients connectés au système                                                     */
/* Entrée: rien                                                                                           */
/* Sortie: un entier !!                                                                                   */
/**********************************************************************************************************/
 static gint Nb_clients ( void )
  { gint i, nb;
    nb = 0;                                                         /* On début, nous n'avons rien choisi */
    for (i=0; i<Config.max_serveur; i++)                                /* Recherche de tous les serveurs */
     { if (Partage->Sous_serveur[i].Thread_run == TRUE)
        { nb += Partage->Sous_serveur[i].nb_client; }
     }
    return(nb);
  }

/**********************************************************************************************************/
/* Gerer_jeton: Donne le jeton au process le moins chargé.                                                */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_jeton ( void )
  { gint i;
/*************************************** Calcul du nouveau jeton ******************************************/
    if (Partage->jeton == -1 && Nb_clients() < Config.max_client )             /* Calcul du nouveau jeton */
     { i = Rechercher_serveur_inactif();                           /* A la recherche d'un serveur inactif */
       if (i!=-1)                                             /* Si c'est le cas, on lui assigne le jeton */
        { Partage->jeton = i;
          Info_new( Config.log, Config.log_all, LOG_INFO, "Gerer_jeton: serveur sans client trouve id=%d" );
        }
       else                  /* Tous nos serveurs sont utilisés, il faut donc soit créer un autre serveur */
        {                                            /* soit donner la connexion à un serveur deja occupé */
          Info_new( Config.log, Config.log_all, LOG_DEBUG, "Gerer_jeton: Recherche d'un emplacement libre" );
          i = Rechercher_empl_libre();
          if (i != -1)
           { Info_new( Config.log, Config.log_all, LOG_NOTICE,
                       "Gerer_jeton: Creation d'un nouveau ssrv id = %d", i );
             if (Demarrer_sous_serveur(i))
              { Partage->jeton = i;                                              /* On lui donne le jeton */
              }
             else
              { Info_new( Config.log, Config.log_all, LOG_WARNING, "Gerer_jeton: creation nouveau ssrv failed" ); }
           }
          else         /* On ne peut plus creer de sous serveur, on attribue la connexion au moins occupé */
           { Info_new( Config.log, Config.log_all, LOG_DEBUG, "Gerer_jeton: Recherche du ssrv le moins chargé" );
             i = Rechercher_moins_occupe();
             if (i != -1)
              { Partage->jeton = i;                                              /* On lui donne le jeton */
              }
             else
              { Info_new( Config.log, Config.log_all, LOG_WARNING, "Gerer_jeton: rechercher_moins_occupe failed" );
              }
           }
        }
       Info_new( Config.log, Config.log_all, LOG_INFO, "Gerer_jeton: jeton to server %d", i );
     }
  }
/**********************************************************************************************************/
/* Gerer_manque_process: Gestion des ss en cas de manque à l'écoute                                       */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_manque_process ( void )
  { gint i, nbr_ssrv;

    nbr_ssrv=0;
    for (i=0; i<Config.max_serveur; i++)                                /* Recherche de tous les serveurs */
     { if (Partage->Sous_serveur[i].Thread_run == TRUE) nbr_ssrv++;
     }

    if (nbr_ssrv >= Config.min_serveur) return;               /* Si nous avons trop peu de serveur online */

    i = Rechercher_empl_libre();
    if (i!=-1)
     { Info_new( Config.log, Config.log_all, LOG_NOTICE,
                 "Gerer_manque_process: Too few servers, we create a new one (id=%d)", i );
       Demarrer_sous_serveur(i);
     }
  }
/**********************************************************************************************************/
/* Stopper_fils: arret de tous les fils Watchdog                                                          */
/* Entrée/Sortie: flag = TRUE si on demande aussi l'arret du thread Admin                                 */
/**********************************************************************************************************/
 void Stopper_fils ( gint flag )
  { gint i;
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Debut stopper_fils" );

    for (i=0; i<Config.max_serveur; i++)                   /* Arret de tous les fils en cours d'execution */
     { if (Partage->Sous_serveur[i].Thread_run == TRUE)                    /* Attente de la fin du fils i */
        { Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Waiting for SSRV (%d) to finish",
                                          Partage->Sous_serveur[i].pid );
          Partage->Sous_serveur[i].Thread_run = FALSE;             /* Attention, les thread sont detach ! */
          while (Partage->Sous_serveur[i].pid) sched_yield();
          Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: ok, SSRV (%d) down", i );
        }
     }

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Waiting for DLS (%d) to finish", Partage->com_dls.TID );
    Partage->com_dls.Thread_run = FALSE;
    while ( Partage->com_dls.TID != 0 ) sched_yield();                                 /* Attente fin DLS */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: ok, DLS is down" );

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Waiting for ONDULEUR (%d) to finish", Partage->com_onduleur.TID );
    Partage->com_onduleur.Thread_run = FALSE;
    while ( Partage->com_onduleur.TID != 0 ) sched_yield();                       /* Attente fin ONDULEUR */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: ok, ONDULEUR is down" );

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Waiting for TELLSTICK (%d) to finish", Partage->com_tellstick.TID );
    Partage->com_tellstick.Thread_run = FALSE;
    while ( Partage->com_tellstick.TID != 0 ) sched_yield();                       /* Attente fin ONDULEUR */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: ok, TELLSTICK is down" );

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Waiting for LIRC (%d) to finish", Partage->com_lirc.TID );
    Partage->com_lirc.Thread_run = FALSE;
    while ( Partage->com_lirc.TID != 0 ) sched_yield();                       /* Attente fin ONDULEUR */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: ok, LIRC is down" );

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Waiting for MODBUS (%d) to finish", Partage->com_modbus.TID );
    Partage->com_modbus.Thread_run = FALSE;
    while ( Partage->com_modbus.TID != 0 ) sched_yield();                       /* Attente fin ONDULEUR */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: ok, MODBUS is down" );

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Waiting for SMS (%d) to finish", Partage->com_sms.TID );
    Partage->com_sms.Thread_run = FALSE;
    while ( Partage->com_sms.TID != 0 ) sched_yield();                       /* Attente fin ONDULEUR */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: ok, SMS is down" );

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Waiting for ARCH (%d) to finish", Partage->com_arch.TID );
    Partage->com_arch.Thread_run = FALSE;
    while ( Partage->com_arch.TID != 0 ) sched_yield();                       /* Attente fin ONDULEUR */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: ok, ARCH is down" );

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Waiting for AUDIO (%d) to finish", Partage->com_audio.TID );
    Partage->com_audio.Thread_run = FALSE;
    while ( Partage->com_audio.TID != 0 ) sched_yield();                       /* Attente fin ONDULEUR */
    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: ok, AUDIO is down" );

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: keep MOTION (%d) running", PID_motion );

    if (flag)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Waiting for ADMIN (%d) to finish", Partage->com_admin.TID );
       Partage->com_admin.Thread_run = FALSE;
       while ( Partage->com_admin.TID != 0 ) sched_yield();                       /* Attente fin ONDULEUR */
       Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: ok, ADMIN is down" );
     }

    Info_new( Config.log, Config.log_all, LOG_WARNING, "Stopper_fils: Fin stopper_fils" );
  }
/*--------------------------------------------------------------------------------------------------------*/
