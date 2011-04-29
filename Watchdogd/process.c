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
 #include <bonobo/bonobo-i18n.h>
 #include <sys/wait.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <errno.h>
 #include <openssl/ssl.h>
 #include <string.h>
 #include <dlfcn.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"

 static gint      PID_motion   = 0;                                            /* Le PID de motion detect */

 extern gint Socket_ecoute;                                  /* Socket de connexion (d'écoute) du serveur */
 extern SSL_CTX *Ssl_ctx;                                          /* Contexte de cryptage des connexions */

/**********************************************************************************************************/
/* Demarrer_onduleur: Thread un process ONDULEUR                                                          */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 static gboolean Creer_config_file_motion ( void )
  { gchar chaine[80];
    struct DB *db;
    gint id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info( Config.log, DEBUG_INFO, "MSRV: Creer_config_file_motion: Connexion DB failed" );
       return(FALSE);
     }                                                                                  /* Si pas d'accès */

    if ( !Recuperer_cameraDB( Config.log, db ) )                      /* Préparation du chargement camera */
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }                                                                         /* Si pas d'enregistrement */

    unlink("motion.conf");                                      /* Création des fichiers de configuration */
    id = open ( "motion.conf", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id<0) { Info_n( Config.log, DEBUG_INFO,
                        "MSRV: Creer_config_file_motion: creation motion.conf pid file failed", id );
                return(FALSE);
              }
    Info_n( Config.log, DEBUG_INFO, "MSRV: Creer_config_file_motion: creation motion.conf", id );
#ifdef bouh
    g_snprintf(chaine, sizeof(chaine), "control_port 8080\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "control_localhost off\n");
    write(id, chaine, strlen(chaine));
#endif
    g_snprintf(chaine, sizeof(chaine), "process_id_file %s/motion.pid\n", Config.home);
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
    g_snprintf(chaine, sizeof(chaine), "target_dir %s\n", Config.home);
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
       g_snprintf(nom_fichier, sizeof(nom_fichier), "camera%04d.conf", camera->id);

       g_snprintf(chaine, sizeof(chaine), "thread %s/%s\n", Config.home, nom_fichier);
       write(id, chaine, strlen(chaine));
       
       unlink(nom_fichier);
       id_camera = open ( nom_fichier, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
       if (id<0) { Info_n( Config.log, DEBUG_INFO,
                           "MSRV: Creer_config_file_motion: creation camera.conf failed", id );
                   g_free(camera);
                   close(id);
                   return(FALSE);
                 }
       Info_n( Config.log, DEBUG_INFO, "MSRV: Creer_config_file_motion: creation thread camera", camera->num );
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
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_motion_detect: Demande de demarrage"), getpid() );

    if (!Creer_config_file_motion()) return(FALSE);

    g_snprintf(chaine, sizeof(chaine), "%s/motion.pid", Config.home);
    id = open ( chaine, O_RDONLY, 0 );
    if (id<0) { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_motion_detect: ouverture pid file failed", id );
                return(FALSE);
              }

    if (read ( id, chaine, sizeof(chaine) )<0)
              { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_motion_detect: erreur lecture pid file", id );
                close(id);
                return(FALSE);
              }
    close(id);
    PID_motion = atoi (chaine);
    kill( PID_motion, SIGHUP );                                                   /* Envoie reload conf a motion */
    Info_n( Config.log, DEBUG_INFO,
           "MSRV: Demarrer_motion_detect: process motion seems to be running", PID_motion );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_sous_serveur: Fork un sous_serveur                                                            */
/* Entrée: l'id du fils                                                                                   */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 static gboolean Demarrer_sous_serveur ( int id )
  { static int nbr_thread = 0;
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_sous_serveur: Demande de demarrage"), id );
    if (Partage->Sous_serveur[id].Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_sous_serveur: An instance is already running"),
               Partage->Sous_serveur[id].pid );
       return(FALSE);
     }
    if ( pthread_create( &Partage->Sous_serveur[id].pid, NULL, (void *)Run_serveur, GINT_TO_POINTER(id) ) )
     { Info_c( Config.log, DEBUG_INFO, _("MSRV: Demarrer_sous_serveur: pthread_create failed"), strerror(errno) );
       return(FALSE);
     }
    else nbr_thread++;
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_sous_serveur: nbr_thread"), nbr_thread );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_onduleur: Thread un process ONDULEUR                                                          */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_onduleur ( void )
  { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_onduleur: Demande de demarrage"), getpid() );
    if (Partage->com_onduleur.Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_onduleur: An instance is already running"),
               Partage->com_onduleur.TID );
       return(FALSE);
     }
    if ( pthread_create( &Partage->com_onduleur.TID, NULL, (void *)Run_onduleur, NULL ) )
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_onduleur: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_onduleur: thread onduleur seems to be running",
                   Partage->com_onduleur.TID );
         }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_dls: Thread un process DLS                                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_dls ( void )
  { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_dls: Demande de demarrage"), getpid() );
    if (Partage->com_dls.Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_dls: An instance is already running"),
               Partage->com_dls.TID );
       return(FALSE);
     }
    if ( pthread_create( &Partage->com_dls.TID, NULL, (void *)Run_dls, NULL ) )
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_dls: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_dls: thread dls seems to be running", Partage->com_dls.TID ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_tellstick: Thread un process TELLSTICK                                                        */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_tellstick ( void )
  { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_tellstick: Demande de demarrage"), getpid() );

    if (Partage->com_tellstick.Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_tellstick: An instance is already running"),
               Partage->com_tellstick.TID );
       return(FALSE);
     }

    Partage->com_tellstick.dl_handle = dlopen( "libwatchdog-tellstick.so", RTLD_LAZY );
    if (!Partage->com_tellstick.dl_handle)
     { Info_c( Config.log, DEBUG_INFO, _("MSRV: Demarrer_tellstick: dlopen failed"), dlerror() );
       return(FALSE);
     }
                                                              /* Recherche de la fonction 'Run_tellstick' */
    Partage->com_tellstick.Run_tellstick = dlsym( Partage->com_tellstick.dl_handle, "Run_tellstick" );
    if (!Partage->com_tellstick.Run_tellstick)
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_tellstick: Run_tellstick does not exist") );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_tellstick.dl_handle = NULL;
       return(FALSE);
     }
                                                          /* Recherche de la fonction 'Ajouter_tellstick' */
    Partage->com_tellstick.Ajouter_tellstick = dlsym( Partage->com_tellstick.dl_handle, "Ajouter_tellstick" );
    if (!Partage->com_tellstick.Ajouter_tellstick)
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_tellstick: Ajouter_tellstick does not exist") );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_tellstick.dl_handle = NULL;
       return(FALSE);
     }
                                                       /* Recherche de la fonction 'Admin_tellstick_list' */
    Partage->com_tellstick.Admin_tellstick_list = dlsym( Partage->com_tellstick.dl_handle, "Admin_tellstick_list" );
    if (!Partage->com_tellstick.Admin_tellstick_list)
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_tellstick: Admin_tellstick_list does not exist") );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_tellstick.dl_handle = NULL;
       return(FALSE);
     }

    Partage->com_tellstick.Admin_tellstick_learn = dlsym( Partage->com_tellstick.dl_handle, "Admin_tellstick_learn" );
    if (!Partage->com_tellstick.Admin_tellstick_learn)
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_tellstick: Admin_tellstick_learn does not exist") );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_tellstick.dl_handle = NULL;
       return(FALSE);
     }

    if ( pthread_create( &Partage->com_tellstick.TID, NULL, (void *)Partage->com_tellstick.Run_tellstick, NULL ) )
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_tellstick: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_tellstick: thread tellstick seems to be running", Partage->com_tellstick.TID ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_tellstick: Thread un process TELLSTICK                                                        */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_lirc ( void )
  { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_lirc: Demande de demarrage"), getpid() );
    if (Partage->com_lirc.Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_lirc: An instance is already running"),
               Partage->com_lirc.TID );
       return(FALSE);
     }

    Partage->com_lirc.dl_handle = dlopen( "libwatchdog-lirc.so", RTLD_LAZY );
    if (!Partage->com_lirc.dl_handle)
     { Info_c( Config.log, DEBUG_INFO, _("MSRV: Demarrer_lirc: dlopen failed"), dlerror() );
       return(FALSE);
     }
                                                              /* Recherche de la fonction 'Run_tellstick' */
    Partage->com_lirc.Run_lirc = dlsym( Partage->com_lirc.dl_handle, "Run_lirc" );
    if (!Partage->com_lirc.Run_lirc)
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_lirc: Run_lirc does not exist") );
       dlclose( Partage->com_tellstick.dl_handle );
       Partage->com_lirc.dl_handle = NULL;
       return(FALSE);
     }

    if ( pthread_create( &Partage->com_lirc.TID, NULL, (void *)Partage->com_lirc.Run_lirc, NULL ) )
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_lirc: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_lirc: thread lirc seems to be running", Partage->com_lirc.TID ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_dls: Thread un process rs485                                                                  */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_rs485 ( void )
  { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_rs485: Demande de demarrage"), getpid() );
    if (Partage->com_rs485.Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_rs485: An instance is already running"),
               Partage->com_rs485.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_rs485.TID, NULL, (void *)Run_rs485, NULL ))
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_rs485: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_rs485: thread rs485 seems to be running",
                   Partage->com_rs485.TID ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_sms: Thread un process sms                                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_sms ( void )
  { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_sms: Demande de demarrage"), getpid() );
    if (Partage->com_sms.Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_sms: An instance is already running"),
               Partage->com_sms.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_sms.TID, NULL, (void *)Run_sms, NULL ))
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_sms: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_sms: thread sms seems to be running", Partage->com_sms.TID ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_audio: Thread un process audio                                                                */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_audio ( void )
  { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_audio: Demande de demarrage"), getpid() );
    if (Partage->com_audio.Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_audio: An instance is already running"),
               Partage->com_audio.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_audio.TID, NULL, (void *)Run_audio, NULL ))
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_audio: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_audio: thread audio seems to be running",
                   Partage->com_audio.TID ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_audio: Thread un process admin                                                                */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_admin ( void )
  { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_admin: Demande de demarrage"), getpid() );
    if (Partage->com_admin.Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_admin: An instance is already running"),
               Partage->com_admin.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_admin.TID, NULL, (void *)Run_admin, NULL ))
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_admin: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_admin: thread admin seems to be running",
                   Partage->com_admin.TID ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_arch: Thread un process arch                                                                  */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_arch ( void )
  { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_arch: Demande de demarrage"), getpid() );
    if (Partage->com_arch.Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_arch: An instance is already running"),
               Partage->com_arch.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_arch.TID, NULL, (void *)Run_arch, NULL ))
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_arch: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_arch: thread arch seems to be running", Partage->com_arch.TID ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_modbus: Thread un process modbus                                                              */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_modbus ( void )
  { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_modbus: Demande de demarrage"), getpid() );
    if (Partage->com_modbus.Thread_run == TRUE)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Demarrer_modbus: An instance is already running"),
               Partage->com_modbus.TID );
       return(FALSE);
     }
    if (pthread_create( &Partage->com_modbus.TID, NULL, (void *)Run_modbus, NULL ))
     { Info( Config.log, DEBUG_INFO, _("MSRV: Demarrer_modbus: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_INFO, "MSRV: Demarrer_modbus: thread modbus seems to be running",
                   Partage->com_arch.TID ); }
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
     { if (Partage->Sous_serveur[i].pid == -1) return(i); }
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
     { if (Partage->Sous_serveur[i].pid != -1 &&
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
     { if (Partage->Sous_serveur[i].pid != -1)
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
     { if (Partage->Sous_serveur[i].pid != -1)
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
    if (Partage->jeton == -1 && Nb_clients() < Config.max_client &&            /* Calcul du nouveau jeton */
        Ssl_ctx && Socket_ecoute )                                 /* Si on ecoute le reseau, et ssl okay */
     { i = Rechercher_serveur_inactif();                           /* A la recherche d'un serveur inactif */
       if (i!=-1)                                             /* Si c'est le cas, on lui assigne le jeton */
        { Partage->jeton = i;
          Info( Config.log, DEBUG_INFO, _("MSRV: Gerer_jeton: serveur sans client trouvé") );
        }
       else                  /* Tous nos serveurs sont utilisés, il faut donc soit créer un autre serveur */
        {                                            /* soit donner la connexion à un serveur deja occupé */
          Info( Config.log, DEBUG_INFO, _("MSRV: Gerer_jeton: Recherche d'un emplacement libre") );
          i = Rechercher_empl_libre();
          if (i != -1)
           { Info_n( Config.log, DEBUG_INFO,
                     _("MSRV: Gerer_jeton: Creation d'un nouveau ssrv"), i );
             if (Demarrer_sous_serveur(i))
              { Partage->jeton = i;                                              /* On lui donne le jeton */
              }
             else
              { Info( Config.log, DEBUG_INFO, _("MSRV: Gerer_jeton: creation nouveau ssrv failed") ); }
           }
          else         /* On ne peut plus creer de sous serveur, on attribue la connexion au moins occupé */
           { Info( Config.log, DEBUG_INFO, _("MSRV: Gerer_jeton: Recherche du ssrv le moins chargé") );
             i = Rechercher_moins_occupe();
             if (i != -1)
              { Partage->jeton = i;                                              /* On lui donne le jeton */
              }
             else
              { Info( Config.log, DEBUG_INFO, _("MSRV: Gerer_jeton: rechercher_moins_occupe failed") );
              }
           }
        }

       Info_n( Config.log, DEBUG_INFO, _("MSRV:     Gerer_jeton: jeton to server"), i );
       
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
     { if (Partage->Sous_serveur[i].pid != -1) nbr_ssrv++;
     }

    while (nbr_ssrv < Config.min_serveur)                     /* Si nous avons trop peu de serveur online */
     { i = Rechercher_empl_libre();
       if (i==-1) break;
       Info_n( Config.log, DEBUG_INFO, _("MSRV: Gerer_manque_process: Too few servers, we create a new one"), i );
       if (!Demarrer_sous_serveur(i)) break;
       nbr_ssrv++;
     }
  }
/**********************************************************************************************************/
/* Stopper_fils: arret de tous les fils Watchdog                                                          */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Stopper_fils ( gint flag )
  { gint i;
    Info( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Debut stopper_fils") );

    for (i=0; i<Config.max_serveur; i++)                   /* Arret de tous les fils en cours d'execution */
     { if (Partage->Sous_serveur[i].pid != -1)                             /* Attente de la fin du fils i */
        { Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for SSRV pid to finish"),
                                          Partage->Sous_serveur[i].pid );
          pthread_join( Partage->Sous_serveur[i].pid, NULL );
          Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, SSRV pid down"),
                                          Partage->Sous_serveur[i].pid );
        }
     }

    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for DLS to finish"), Partage->com_dls.TID );
    if (Partage->com_dls.Thread_run == TRUE)
     { Partage->com_dls.Thread_run = FALSE;
       pthread_join( Partage->com_dls.TID, NULL );                                     /* Attente fin DLS */
     }
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, DLS is down"), Partage->com_dls.TID );

    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for ONDULEUR to finish"), Partage->com_onduleur.TID );
    if (Partage->com_onduleur.Thread_run == TRUE)
     { Partage->com_onduleur.Thread_run = FALSE;
       pthread_join( Partage->com_onduleur.TID, NULL );                           /* Attente fin ONDULEUR */
     }
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, ONDULEUR is down"), Partage->com_onduleur.TID );

    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for RS485 to finish"), Partage->com_rs485.TID );
    if (Partage->com_rs485.Thread_run == TRUE)
     { Partage->com_rs485.Thread_run = FALSE;
       pthread_join( Partage->com_rs485.TID, NULL );                                 /* Attente fin RS485 */
     }
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, RS485 is down"), Partage->com_rs485.TID );

    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for TELLSTICK to finish"), Partage->com_tellstick.TID );
    if (Partage->com_tellstick.Thread_run == TRUE)
     { Partage->com_tellstick.Thread_run = FALSE;
       pthread_join( Partage->com_tellstick.TID, NULL );                         /* Attente fin TELLSTICK */
     }
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, TELLSTICK is down"), Partage->com_tellstick.TID );

    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for LIRC to finish"), Partage->com_lirc.TID );
    if (Partage->com_lirc.Thread_run == TRUE)
     { Partage->com_lirc.Thread_run = FALSE;
       pthread_join( Partage->com_lirc.TID, NULL );                                   /* Attente fin LIRC */
     }
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, LIRC is down"), Partage->com_lirc.TID );

    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for MODBUS to finish"), Partage->com_modbus.TID );
    if (Partage->com_modbus.Thread_run == TRUE)
     { Partage->com_modbus.Thread_run = FALSE;
       pthread_join( Partage->com_modbus.TID, NULL );                               /* Attente fin MODBUS */
     }
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, MODBUS is down"), Partage->com_modbus.TID );

    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for SMS to finish"), Partage->com_sms.TID );
    if (Partage->com_sms.Thread_run == TRUE)
     { Partage->com_sms.Thread_run = FALSE;
       pthread_join( Partage->com_sms.TID, NULL );                                     /* Attente fin SMS */
     }
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, SMS is down"), Partage->com_sms.TID );

    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for ARCH to finish"), Partage->com_arch.TID );
    if (Partage->com_arch.Thread_run == TRUE)
     { Partage->com_arch.Thread_run = FALSE;
       pthread_join( Partage->com_arch.TID, NULL );                                   /* Attente fin ARCH */
     }
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, ARCH is down"), Partage->com_arch.TID );

    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for AUDIO to finish"), Partage->com_audio.TID );
    if (Partage->com_audio.Thread_run == TRUE)
     { Partage->com_audio.Thread_run = FALSE;
       pthread_join( Partage->com_audio.TID, NULL );                                 /* Attente fin AUDIO */
     }
    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, AUDIO is down"), Partage->com_audio.TID );

    Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: keep MOTION running"), PID_motion );

    if (flag)
     { Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Waiting for ADMIN to finish"), Partage->com_admin.TID );
        if (Partage->com_admin.Thread_run == TRUE)
         { Partage->com_admin.Thread_run = FALSE;
           pthread_join( Partage->com_admin.TID, NULL );                                     /* Attente fin ADMIN */
         }
       Info_n( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: ok, ADMIN is down"), Partage->com_admin.TID );
     }

    Info( Config.log, DEBUG_INFO, _("MSRV: Stopper_fils: Fin stopper_fils") );
  }
/*--------------------------------------------------------------------------------------------------------*/
