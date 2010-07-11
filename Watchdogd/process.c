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

/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"

 static pthread_t TID_sms      = 0;                                 /* Le tid du SMS en cours d'execution */
 static pthread_t TID_dls      = 0;                                 /* Le tid du DLS en cours d'execution */
 static pthread_t TID_rs485    = 0;                               /* Le tid du rs485 en cours d'execution */
 static pthread_t TID_arch     = 0;                                /* Le tid du ARCH en cours d'execution */
 static pthread_t TID_modbus   = 0;                              /* Le tid du MODBUS en cours d'execution */
 static pthread_t TID_audio    = 0;                              /* Le tid du AUDIO  en cours d'execution */
 static pthread_t TID_onduleur = 0;                              /* Le tid du AUDIO  en cours d'execution */
 static pthread_t TID_admin    = 0;                              /* Le tid du ADMIN  en cours d'execution */
 static gint      PID_motion   = 0;                                            /* Le PID de motion detect */

 extern gint Socket_ecoute;                                  /* Socket de connexion (d'écoute) du serveur */
 extern SSL_CTX *Ssl_ctx;                                          /* Contexte de cryptage des connexions */

/**********************************************************************************************************/
/* Demarrer_onduleur: Thread un process ONDULEUR                                                          */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_motion_detect ( void )
  { gchar chaine[80];
    struct DB *db;
    gint id;
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_motion_detect: Demande de demarrage"), getpid() );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info( Config.log, DEBUG_INFO, "MSRV: Demarrer_motion_detect: Connexion DB failed" );
       return(FALSE);
     }                                                                                  /* Si pas d'accès */

    if ( !Recuperer_cameraDB( Config.log, db ) )                      /* Préparation du chargement camera */
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }                                                                         /* Si pas d'enregistrement */

    unlink("motion.conf");                                      /* Création des fichiers de configuration */
    unlink("camera*.conf");
    id = open ( "motion.conf", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_motion_detect: creation motion.conf", id );
    g_snprintf(chaine, sizeof(chaine), "daemon off\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "framerate 25\n");
    write(id, chaine, strlen(chaine));
    g_snprintf(chaine, sizeof(chaine), "netcam_http 1.1\n");
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
       g_snprintf(nom_fichier, sizeof(nom_fichier), "camera%04d.conf", camera->id_mnemo);

       g_snprintf(chaine, sizeof(chaine), "thread %s\n", nom_fichier);
       write(id, chaine, strlen(chaine));
       
       id_camera = open ( nom_fichier, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
       Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_motion_detect: creation thread camera", camera->num );
       g_snprintf(chaine, sizeof(chaine), "netcam_url %s\n", camera->location);
       write(id_camera, chaine, strlen(chaine));
       g_snprintf(chaine, sizeof(chaine), "sql_query insert into cameras_motion (id_mnemo) values (%d)\n",
                  camera->id_mnemo);
       write(id_camera, chaine, strlen(chaine));
       g_snprintf(chaine, sizeof(chaine), "text_left CAM%04d %s\n", camera->num, camera->objet);
       write(id_camera, chaine, strlen(chaine));
       g_snprintf(chaine, sizeof(chaine), "movie_filename CAM%04d-%%Y%%m%%d%%H%%M%%S\n", camera->num);
       write(id_camera, chaine, strlen(chaine));

       close(id_camera);
     }
    close(id);
          
    PID_motion = fork();
    if (PID_motion<0)
     { Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_motion_detect: fork failed", PID_motion );
       return(FALSE);
     }
    else if (!PID_motion)                                                        /* Demarrage du "motion" */
     { execlp( "motion", "motion", "-c", "motion.conf", NULL );
       Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_motion_detect: Lancement motion failed", PID_motion );
       _exit(0);
     }
    Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_motion_detect: process motion seems to be running",
            PID_motion );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_sous_serveur: Fork un sous_serveur                                                            */
/* Entrée: l'id du fils                                                                                   */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 static gboolean Demarrer_sous_serveur ( int id )
  { static int nbr_thread = 0;
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_sous_serveur: Demande de demarrage"), id );
    if ( pthread_create( &Partage->Sous_serveur[id].pid, NULL, (void *)Run_serveur, GINT_TO_POINTER(id) ) )
     { Info_c( Config.log, DEBUG_FORK, _("MSRV: Demarrer_sous_serveur: pthread_create failed"), strerror(errno) );
       return(FALSE);
     }
    else nbr_thread++;
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_sous_serveur: nbr_thread"), nbr_thread );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_onduleur: Thread un process ONDULEUR                                                          */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_onduleur ( void )
  { Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_onduleur: Demande de demarrage"), getpid() );
    if ( pthread_create( &TID_onduleur, NULL, (void *)Run_onduleur, NULL ) )
     { Info( Config.log, DEBUG_FORK, _("MSRV: Demarrer_onduleur: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_onduleur: thread onduleur seems to be running",
                   TID_onduleur );
         }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_dls: Thread un process DLS                                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_dls ( void )
  { Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_dls: Demande de demarrage"), getpid() );
    if ( pthread_create( &TID_dls, NULL, (void *)Run_dls, NULL ) )
     { Info( Config.log, DEBUG_FORK, _("MSRV: Demarrer_dls: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_dls: thread dls seems to be running", TID_dls ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_dls: Thread un process rs485                                                                  */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_rs485 ( void )
  { Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_rs485: Demande de demarrage"), getpid() );
    if (pthread_create( &TID_rs485, NULL, (void *)Run_rs485, NULL ))
     { Info( Config.log, DEBUG_FORK, _("MSRV: Demarrer_rs485: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_rs485: thread rs485 seems to be running",
                   TID_rs485 ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_sms: Thread un process sms                                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_sms ( void )
  { Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_sms: Demande de demarrage"), getpid() );
    if (pthread_create( &TID_sms, NULL, (void *)Run_sms, NULL ))
     { Info( Config.log, DEBUG_FORK, _("MSRV: Demarrer_sms: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_sms: thread sms seems to be running", TID_sms ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_audio: Thread un process sms                                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_audio ( void )
  { Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_audio: Demande de demarrage"), getpid() );
    if (pthread_create( &TID_audio, NULL, (void *)Run_audio, NULL ))
     { Info( Config.log, DEBUG_FORK, _("MSRV: Demarrer_audio: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_audio: thread audio seems to be running",
                   TID_audio ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_audio: Thread un process sms                                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_admin ( void )
  { Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_admin: Demande de demarrage"), getpid() );
    if (pthread_create( &TID_admin, NULL, (void *)Run_admin, NULL ))
     { Info( Config.log, DEBUG_FORK, _("MSRV: Demarrer_admin: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_admin: thread admin seems to be running",
                   TID_admin ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_arch: Thread un process sms                                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_arch ( void )
  { Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_arch: Demande de demarrage"), getpid() );
    if (pthread_create( &TID_arch, NULL, (void *)Run_arch, NULL ))
     { Info( Config.log, DEBUG_FORK, _("MSRV: Demarrer_arch: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_arch: thread arch seems to be running", TID_arch ); }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Demarrer_modbus: Thread un process modbus                                                              */
/* Entrée: rien                                                                                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Demarrer_modbus ( void )
  { Info_n( Config.log, DEBUG_FORK, _("MSRV: Demarrer_modbus: Demande de demarrage"), getpid() );
    if (pthread_create( &TID_modbus, NULL, (void *)Run_modbus, NULL ))
     { Info( Config.log, DEBUG_FORK, _("MSRV: Demarrer_modbus: pthread_create failed") );
       return(FALSE);
     }
    else { Info_n( Config.log, DEBUG_FORK, "MSRV: Demarrer_modbus: thread modbus seems to be running",
                   TID_arch ); }
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
          Info( Config.log, DEBUG_FORK, _("MSRV: Gerer_jeton: serveur sans client trouvé") );
        }
       else                  /* Tous nos serveurs sont utilisés, il faut donc soit créer un autre serveur */
        {                                            /* soit donner la connexion à un serveur deja occupé */
          Info( Config.log, DEBUG_FORK, _("MSRV: Gerer_jeton: Recherche d'un emplacement libre") );
          i = Rechercher_empl_libre();
          if (i != -1)
           { Info_n( Config.log, DEBUG_FORK,
                     _("MSRV: Gerer_jeton: Creation d'un nouveau ssrv"), i );
             if (Demarrer_sous_serveur(i))
              { Partage->jeton = i;                                              /* On lui donne le jeton */
              }
             else
              { Info( Config.log, DEBUG_FORK, _("MSRV: Gerer_jeton: creation nouveau ssrv failed") ); }
           }
          else         /* On ne peut plus creer de sous serveur, on attribue la connexion au moins occupé */
           { Info( Config.log, DEBUG_FORK, _("MSRV: Gerer_jeton: Recherche du ssrv le moins chargé") );
             i = Rechercher_moins_occupe();
             if (i != -1)
              { Partage->jeton = i;                                              /* On lui donne le jeton */
              }
             else
              { Info( Config.log, DEBUG_FORK, _("MSRV: Gerer_jeton: rechercher_moins_occupe failed") );
              }
           }
        }

       Info_n( Config.log, DEBUG_FORK, _("MSRV:     Gerer_jeton: jeton to server"), i );
       
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
       Info_n( Config.log, DEBUG_FORK, _("MSRV: Gerer_manque_process: Too few servers, we create a new one"), i );
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
    Info( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Debut stopper_fils") );

    for (i=0; i<Config.max_serveur; i++)                   /* Arret de tous les fils en cours d'execution */
     { if (Partage->Sous_serveur[i].pid != -1)                             /* Attente de la fin du fils i */
        { Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Waiting for SSRV pid to finish"),
                                          Partage->Sous_serveur[i].pid );
          pthread_join( Partage->Sous_serveur[i].pid, NULL );
          Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: ok, SSRV pid down"),
                                          Partage->Sous_serveur[i].pid );
        }
     }

    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Waiting for DLS to finish"), TID_dls );
    if (TID_dls) { pthread_join( TID_dls, NULL ); }                                    /* Attente fin DLS */
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: ok, DLS is down"), TID_dls );

    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Waiting for ONDULEUR to finish"), TID_onduleur );
    if (TID_onduleur) { pthread_join( TID_onduleur, NULL ); }                     /* Attente fin ONDULEUR */
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: ok, ONDULEUR is down"), TID_onduleur );

    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Waiting for RS485 to finish"), TID_rs485 );
    if (TID_rs485) { pthread_join( TID_rs485, NULL ); }                              /* Attente fin RS485 */
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: ok, RS485 is down"), TID_rs485 );

    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Waiting for MODBUS to finish"), TID_modbus );
    if (TID_modbus) { pthread_join( TID_modbus, NULL ); }                           /* Attente fin MODBUS */
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: ok, MODBUS is down"), TID_rs485 );

    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Waiting for SMS to finish"), TID_sms );
    if (TID_sms) { pthread_join( TID_sms, NULL ); }                                    /* Attente fin SMS */
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: ok, SMS is down"), TID_sms );

    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Waiting for ARCH to finish"), TID_arch );
    if (TID_arch) { pthread_join( TID_arch, NULL ); }                                 /* Attente fin ARCH */
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: ok, ARCH is down"), TID_arch );

    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Waiting for AUDIO to finish"), TID_audio );
    if (TID_audio) { pthread_join( TID_audio, NULL ); }                              /* Attente fin AUDIO */
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: ok, AUDIO is down"), TID_audio );

    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Waiting for MOTION to finish"), PID_motion );
    if (PID_motion) { kill( PID_motion, SIGTERM );                                  /* Attente fin MOTION */
                      wait4 (PID_motion, NULL, 0, NULL );
                    }
    Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: ok, MOTION is down"), PID_motion );

    if (flag)
     { Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Waiting for ADMIN to finish"), TID_admin );
       if (TID_admin) { pthread_join( TID_admin, NULL ); }                           /* Attente fin ADMIN */
       Info_n( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: ok, ADMIN is down"), TID_admin );
     }

    Info( Config.log, DEBUG_FORK, _("MSRV: Stopper_fils: Fin stopper_fils") );
  }
/*--------------------------------------------------------------------------------------------------------*/
