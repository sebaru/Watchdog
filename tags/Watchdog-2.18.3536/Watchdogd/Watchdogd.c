/******************************************************************************************************************************/
/* Watchdogd/Watchdogd.c        Démarrage/Arret du systeme Watchdog, gestion des connexions clientes                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           mar 14 fév 2006 15:56:40 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Watchdogd.c
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

 #include <sys/prctl.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdio.h>  /* Pour printf */
 #include <stdlib.h> /* Pour exit */
 #include <fcntl.h>
 #include <signal.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <sys/file.h>
 #include <sys/types.h>
 #include <grp.h>
 #include <popt.h>
 #include <pthread.h>
 #include <locale.h>
 #include <pwd.h>
 #include <gcrypt.h>                                                      /* Pour assurer le multithreading avec IMSG et HTTP */
 #include <curl/curl.h>                                                          /* Pour le multithreading avec Master et SMS */

 #include "watchdogd.h"

 struct CONFIG Config;                                       /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 struct PARTAGE *Partage;                                                        /* Accès aux données partagées des processes */

/******************************************************************************************************************************/
/* Exporter : Exporte les données de base Watchdog pour préparer le RELOAD                                                    */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Exporter ( void )
  { int fd;
    unlink ( FICHIER_EXPORT );
    Partage->taille_partage = sizeof(struct PARTAGE);
    g_snprintf( Partage->version, sizeof(Partage->version), "%s", VERSION );
    fd = open( FICHIER_EXPORT, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );

    if ( fd < 0 )                                                                                   /* Traitement des erreurs */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "Exporter: Open Error on %s. Could not export (%s)",
                 FICHIER_EXPORT, strerror(errno) );
       return;
     }

    if ( write (fd, Partage->version, sizeof(Partage->version)) != sizeof(Partage->version) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Exporter: Version Export to %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( write (fd, &Partage->top, sizeof(Partage->top)) != sizeof(Partage->top) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Exporter: Top Export to %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( write (fd, Partage->m, sizeof(Partage->m)) != sizeof(Partage->m) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Exporter: Monostable Export to %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( write (fd, Partage->b, sizeof(Partage->b)) != sizeof(Partage->b) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Exporter: Bistable Export to %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( write (fd, Partage->g, sizeof(Partage->g)) != sizeof(Partage->g) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Exporter: Messages Export to %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( write (fd, Partage->i, sizeof(Partage->i)) != sizeof(Partage->i) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Exporter: Icones Export to %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( write (fd, Partage->e, sizeof(Partage->e)) != sizeof(Partage->e) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Exporter: Input Export to %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( write (fd, Partage->a, sizeof(Partage->a)) != sizeof(Partage->a) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Exporter: Output Export to %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    close (fd);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Exporter: Export successfull" );
  }
/******************************************************************************************************************************/
/* Importe : Tente d'importer les données de base Watchdog juste apres le reload                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Importer ( void )
  { int fd;

    fd = open( FICHIER_EXPORT, O_RDONLY );                                                            /* Ouverture du fichier */
    if ( fd <= 0 )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "Importer : Open Errort %s (%s).", FICHIER_EXPORT, strerror(errno) );
       return(FALSE);
     }

    if ( read (fd, Partage->version, sizeof(Partage->version)) != sizeof(Partage->version) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Importer: Version Import from %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
       close(fd);
       return(FALSE);
     }

    if ( strncmp (Partage->version, VERSION, sizeof(Partage->version)) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "Importer : Wrong version number on %s (Import Version %s but Watchdog v%s)",
                 FICHIER_EXPORT, Partage->version, VERSION );
       close(fd);
       return(FALSE);
     }

    if ( read (fd, &Partage->top, sizeof(Partage->top)) != sizeof(Partage->top) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Importer: Top Import from %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( read (fd, Partage->m, sizeof(Partage->m)) != sizeof(Partage->m) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Importer: Monostable Import from %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( read (fd, Partage->b, sizeof(Partage->b)) != sizeof(Partage->b) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Importer: Bistable Import from %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( read (fd, Partage->g, sizeof(Partage->g)) != sizeof(Partage->g) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Importer: Messages Import from %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( read (fd, Partage->i, sizeof(Partage->i)) != sizeof(Partage->i) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Importer: Icones Import from %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( read (fd, Partage->e, sizeof(Partage->e)) != sizeof(Partage->e) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Importer: Input Import from %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    if ( read (fd, Partage->a, sizeof(Partage->a)) != sizeof(Partage->a) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Importer: Output Import from %s failed (%s)",
                 FICHIER_EXPORT, strerror(errno) );
     }
    close(fd);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Importer: Size OK, import OK" );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Charger_config_bit_interne: Chargement des configs bit interne depuis la base de données                                   */
/* Entrée: néant                                                                                                              */
/******************************************************************************************************************************/
 void Charger_config_bit_interne( void )
  { Charger_analogInput();
    Charger_cpth();
    Charger_cpt_imp();
    Charger_tempo();
    Charger_messages();
    Charger_registre();
  }
/******************************************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme                                                             */
/* Entrée: numero du signal à gerer                                                                                           */
/******************************************************************************************************************************/
 static void Traitement_signaux( int num )
  { char chaine[50];
    if (num == SIGALRM)
     { Partage->top++;
       if (!Partage->top)                                             /* Si on passe par zero, on le dit (DEBUG interference) */
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Timer: Partage->top = 0 !!", __func__ ); }
       if (!(Partage->top%5))                                                              /* Cligno toutes les demi-secondes */
        { SB_SYS(5, !B(5)); }
       if (!(Partage->top%3))                                                                 /* Cligno toutes les 3 dixièmes */
        { SB_SYS(6, !B(6)); }
       if (!(Partage->top%10))                                                                  /* Cligno toutes les secondes */
        { SB_SYS(4, !B(4));
          Partage->audit_bit_interne_per_sec_hold += Partage->audit_bit_interne_per_sec;
          Partage->audit_bit_interne_per_sec_hold = Partage->audit_bit_interne_per_sec_hold >> 1;
          Partage->audit_bit_interne_per_sec = 0;
          SEA ( NUM_EA_SYS_BITS_PER_SEC, Partage->audit_bit_interne_per_sec_hold );                             /* historique */

          Partage->audit_tour_dls_per_sec_hold += Partage->audit_tour_dls_per_sec;
          Partage->audit_tour_dls_per_sec_hold = Partage->audit_tour_dls_per_sec_hold >> 1;
          Partage->audit_tour_dls_per_sec = 0;
          SEA ( NUM_EA_SYS_TOUR_DLS_PER_SEC, Partage->audit_tour_dls_per_sec_hold );                            /* historique */
          if (Partage->audit_tour_dls_per_sec_hold > 100)                                           /* Moyennage tour DLS/sec */
           { Partage->com_dls.temps_sched += 50; }
          else if (Partage->audit_tour_dls_per_sec_hold < 80)
           { if (Partage->com_dls.temps_sched) Partage->com_dls.temps_sched -= 10; }
          SEA ( NUM_EA_SYS_DLS_WAIT, Partage->com_dls.temps_sched );                                            /* historique */
        }

       Partage->top_cdg_plugin_dls++;                                                            /* Chien de garde plugin DLS */
       if (Partage->top_cdg_plugin_dls>200)                                         /* Si pas de réponse D.L.S en 20 secondes */
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: CDG plugin DLS !!", __func__ );
          Partage->top_cdg_plugin_dls = 0;
        }
       return;
     }

    prctl(PR_GET_NAME, chaine, 0, 0, 0 );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: handled by %s", __func__, chaine );

    switch (num)
     { case SIGQUIT:
       case SIGINT:  Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGINT" );
                     Partage->com_msrv.Thread_run = FALSE;                       /* On demande l'arret de la boucle programme */
                     break;
       case SIGTERM: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGTERM" );
                     Partage->com_msrv.Thread_run = FALSE;                       /* On demande l'arret de la boucle programme */
                     break;
       case SIGABRT: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGABRT" );
                     break;
       case SIGCHLD: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGCHLD" );
                     break;
       case SIGPIPE: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGPIPE" ); break;
       case SIGBUS:  Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGBUS" ); break;
       case SIGIO:   Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGIO" ); break;
       case SIGUSR1: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGUSR1: dumping infos" );
                     Partage->com_msrv.Thread_sigusr1 = TRUE;
                     break;
       case SIGUSR2: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGUSR2: Reloading THREAD in progress" );
                     Partage->com_msrv.Thread_reload = TRUE;
                     break;
       default: Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Recu signal", num ); break;
     }
  }
/******************************************************************************************************************************/
/* Sauver_compteur : Envoie les infos Compteurs à la base de données pour sauvegarde !                                        */
/* Entrée : Néant                                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Sauver_compteur ( void )
  { if (Config.instance_is_master == FALSE) return;                                /* Seul le master sauvegarde les compteurs */
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Saving CPT", __func__ );
    Updater_cpthDB();                                                                     /* Sauvegarde des compteurs Horaire */
    Updater_cpt_impDB();                                                              /* Sauvegarde des compteurs d'impulsion */
  }
/******************************************************************************************************************************/
/* Tatiter_sigusr1 : Print les variable importante dans les lgos                                                              */
/* Entrée : Néant                                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Traiter_sigusr1 ( void )
  { guint nbr_i, nbr_msg, nbr_msg_repeat;
    gchar chaine[256];

    pthread_mutex_lock( &Partage->com_msrv.synchro );
    nbr_i          = g_slist_length( Partage->com_msrv.liste_i );
    nbr_msg        = g_slist_length( Partage->com_msrv.liste_msg );                            /* Recuperation du numero de i */
    nbr_msg_repeat = g_slist_length( Partage->com_msrv.liste_msg_repeat );                                /* liste des repeat */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    g_snprintf( chaine, sizeof(chaine), "%s: Reste %d I, %d MSG, %d MSG_REPEAT", __func__, 
                nbr_i, nbr_msg, nbr_msg_repeat );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, chaine );
  }
/******************************************************************************************************************************/
/* Boucle_pere: boucle de controle du pere de tous les serveurs                                                               */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void *Boucle_pere ( void )
  { gint cpt_5_minutes, cpt_1_minute;
    struct CMD_TYPE_HISTO histo;
    struct ZMQUEUE *zmq_from_slave, *zmq_from_master, *zmq_from_threads;

    prctl(PR_SET_NAME, "W-MSRV", 0, 0, 0 );

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Debut boucle sans fin", __func__ );

/************************************************* Socket ZMQ interne *********************************************************/
    Partage->com_msrv.zmq_msg = New_zmq ( ZMQ_PUB, "pub-int-msgs" );
    Bind_zmq ( Partage->com_msrv.zmq_msg, "inproc", ZMQUEUE_LIVE_MSGS, 0 );

    Partage->com_msrv.zmq_motif = New_zmq ( ZMQ_PUB, "pub-int-motifs" );
    Bind_zmq ( Partage->com_msrv.zmq_motif, "inproc", ZMQUEUE_LIVE_MOTIFS, 0 );

    Partage->com_msrv.zmq_to_threads = New_zmq ( ZMQ_PUB, "pub-to-threads" );
    Bind_zmq ( Partage->com_msrv.zmq_to_threads, "inproc", ZMQUEUE_LIVE_THREADS, 0 );

    zmq_from_threads = New_zmq ( ZMQ_SUB, "listen-to-threads" );
    Bind_zmq ( zmq_from_threads, "inproc", ZMQUEUE_LIVE_MASTER, 0 );

/***************************************** Socket pour une instance master ****************************************************/
    if (Config.instance_is_master == TRUE)
     { Partage->com_msrv.zmq_to_slave = New_zmq ( ZMQ_PUB, "pub-to-slave" );
       Bind_zmq ( Partage->com_msrv.zmq_to_slave, "tcp", "*", 5555 );
       zmq_from_slave = New_zmq ( ZMQ_SUB, "listen-to-slave" );
       Bind_zmq ( zmq_from_slave, "tcp", "*", 5556 );
     }
/***************************************** Socket de subscription au master ***************************************************/
    else                                                                                     /* Connexion au master si besoin */
     { Partage->com_msrv.zmq_to_master = New_zmq ( ZMQ_PUB, "pub-to-master" );
       Connect_zmq ( Partage->com_msrv.zmq_to_master, "tcp", Config.master_host, 5556 );
       zmq_from_master = New_zmq ( ZMQ_SUB, "listen-to-master" );
       Connect_zmq ( zmq_from_master, "tcp", Config.master_host, 5555 );
     }

    cpt_5_minutes = Partage->top + 3000;
    cpt_1_minute  = Partage->top + 600;

    sleep(1);
    Partage->com_msrv.Thread_run = TRUE;                                             /* On dit au maitre que le thread tourne */
    while(Partage->com_msrv.Thread_run == TRUE)                                           /* On tourne tant que l'on a besoin */
     { Gerer_arrive_MSGxxx_dls();                                 /* Redistrib des messages DLS vers les clients + Historique */ 
       Gerer_arrive_Ixxx_dls();                                                 /* Distribution des changements d'etats motif */
       Gerer_arrive_Axxx_dls();                                           /* Distribution des changements d'etats sorties TOR */
       Gerer_arrive_Events();                                       /* Gestion des evenements entre Thread, DLS, et satellite */

       if (Config.instance_is_master == TRUE)                                     /* Instance is master : listening to slaves */
        { struct MSRV_EVENT *event;
          gchar buffer[2048];
          void *payload;
          gint byte;
          if ( (byte=Recv_zmq_with_tag( zmq_from_slave, &buffer, sizeof(buffer), &event, &payload )) > 0 )
           { switch(event->tag)
              { case TAG_ZMQ_SET_BIT:
                 { struct ZMQ_SET_BIT *bit;
                   bit = (struct ZMQ_SET_BIT *)payload;
                   if (bit->type == MNEMO_MONOSTABLE) { Envoyer_commande_dls ( bit->num ); }
                   break;
                 }
                default:
                 { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: receive wrong tag number '%d' for ZMQ '%s'",
                             __func__, event->tag, zmq_from_slave->name );
                 }
              }
           }
        }
       else                                                                         /* Instance is slave, listening to master */
        { struct MSRV_EVENT *event;
          gchar buffer[2048];
          void *payload;
          gint byte;
          if ( (byte=Recv_zmq_with_tag( zmq_from_master, &buffer, sizeof(buffer), &event, &payload )) > 0 )
           { switch(event->tag)
              { case TAG_ZMQ_TO_HISTO:
                 { if (Send_zmq( Partage->com_msrv.zmq_msg, payload, sizeof(struct CMD_TYPE_HISTO)) == -1)
                    { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Send to ZMQ '%s' socket failed (%s)",
                                __func__, Partage->com_msrv.zmq_msg->name, zmq_strerror(errno) );
                    }
                   break;
                 }
                case TAG_ZMQ_TO_THREADS:
                 { if (Send_zmq( Partage->com_msrv.zmq_to_threads, buffer, byte ) == -1)
                    { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Send to ZMQ '%s' socket failed (%s)",
                                __func__, Partage->com_msrv.zmq_to_threads->name, zmq_strerror(errno) );
                    }
                   break;
                 }
                default:
                 { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: receive wrong tag number '%d' for ZMQ '%s'",
                             __func__, event->tag, zmq_from_master->name );
                 }
              }
           }
                                                                      /* Si reception depuis un thread, report vers le master */
          if ( (byte=Recv_zmq( zmq_from_threads, &buffer, sizeof(buffer) )) > 0 )
           { Send_zmq ( Partage->com_msrv.zmq_to_master, buffer, byte ); }
        }

       if (Partage->com_msrv.Thread_reload)                                                               /* On a recu RELOAD */
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: RELOAD", __func__ );
          Partage->com_dls.Thread_reload       = TRUE;
          Partage->com_arch.Thread_reload      = TRUE;

          Lire_config( NULL );                                                  /* Lecture sur le fichier /etc/watchdogd.conf */
          Print_config();
          Info_change_log_level ( Config.log, Config.log_level );
          Charger_config_bit_interne();                                             /* Rechargement des configs bits internes */
          Partage->com_msrv.Thread_reload      = FALSE;                                                 /* signal traité. RAZ */
        }

       if (Partage->com_msrv.Thread_sigusr1)                                                          /* On a recu sigusr1 ?? */
        { struct LIBRAIRIE *lib;
          GSList *liste;

          Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: SIGUSR1", __func__ );
          Partage->com_dls.Thread_sigusr1       = TRUE;
          Partage->com_arch.Thread_sigusr1      = TRUE;
          Partage->com_admin.Thread_sigusr1     = TRUE;

          liste = Partage->com_msrv.Librairies;                                          /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             lib->Thread_sigusr1 = TRUE;
             liste = liste->next;
           }

          Traiter_sigusr1();                                         /* Appel de la fonction pour traiter le signal pour MSRV */
          Partage->com_msrv.Thread_sigusr1      = FALSE;
        }

       if (cpt_5_minutes < Partage->top)                                                    /* Update DB toutes les 5 minutes */
        { Sauver_compteur();
          Exporter();
          cpt_5_minutes = Partage->top + 3000;                                             /* Sauvegarde toutes les 5 minutes */
        }

       if (cpt_1_minute < Partage->top)                                                       /* Update DB toutes les minutes */
        { Gerer_histo_repeat();
          Print_SQL_status();                                                             /* Print SQL status for debugging ! */
          cpt_1_minute = Partage->top + 600;                                                 /* Sauvegarde toutes les minutes */
        }

       usleep(1000);
       sched_yield();
     }

/*********************************** Terminaison: Deconnexion DB et kill des serveurs *****************************************/ 
    Sauver_compteur();                                                                     /* Dernière sauvegarde avant arret */
    Close_zmq ( Partage->com_msrv.zmq_msg );
    Close_zmq ( Partage->com_msrv.zmq_motif );
    Close_zmq ( Partage->com_msrv.zmq_to_threads );
    Close_zmq ( zmq_from_threads );
    if (Config.instance_is_master == TRUE)
     { Close_zmq( Partage->com_msrv.zmq_to_slave );
       Close_zmq( zmq_from_slave );
     }
    else
     { Close_zmq( Partage->com_msrv.zmq_to_master );
       Close_zmq( zmq_from_master );
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: fin boucle sans fin", __func__ );
    pthread_exit( NULL );
  }
/******************************************************************************************************************************/
/* Lire_ligne_commande: Parse la ligne de commande pour d'eventuels parametres                                                */
/* Entrée: argc, argv                                                                                                         */
/* Sortie: -1 si erreur, 0 si ok                                                                                              */
/******************************************************************************************************************************/
 static gboolean Lire_ligne_commande( int argc, char *argv[] )
  { gint help = 0, log_level = -1, fg = 0, single = 0, compil = 0, version = 0;
    gchar *home = NULL, *file= NULL, *run_as = NULL;
    struct passwd *pwd, *old;
    struct poptOption Options[]= 
     { { "foreground", 'f', POPT_ARG_NONE,
         &fg,               0, "Run in foreground", NULL },
       { "version",    'v', POPT_ARG_NONE,
         &version,          0, "Display Version Number", NULL },
       { "debug",      'd', POPT_ARG_INT,
         &log_level,      0, "Debug level", "LEVEL" },
       { "home",       'H', POPT_ARG_STRING,
         &home,             0, "Home directory", "HOME" },
       { "run_as",     'u', POPT_ARG_STRING,
         &run_as,           0, "Run as user", "USER" },
       { "conffile",   'c', POPT_ARG_STRING,
         &file,             0, "Configuration file", "FILE" },
       { "compil",     'C', POPT_ARG_NONE,
         &compil,           0, "Compilation des plugins DLS au demarrage", NULL },
       { "help",       'h', POPT_ARG_NONE,
         &help,             0, "Help", NULL },
       { "single",     's', POPT_ARG_NONE,
         &single,           0, "Don't start thread", NULL },
       POPT_TABLEEND
     };
    poptContext context;
    int rc;

    context = poptGetContext( NULL, argc, (const char **)argv, Options, POPT_CONTEXT_ARG_OPTS );
    while ( (rc = poptGetNextOpt( context )) != -1)                                          /* Parse de la ligne de commande */
     { switch (rc)
        { case POPT_ERROR_BADOPT: printf( "Option %s is unknown\n", poptBadOption(context, 0) );
                                  help=1; break;
          default: printf("Parsing error\n");
        }
     }

    if (help)                                                                                 /* Affichage de l'aide en ligne */
     { poptPrintHelp(context, stdout, 0);
       poptFreeContext(context);
       exit(EXIT_OK);
     }
    poptFreeContext( context );                                                                         /* Liberation memoire */

    if (version)                                                                            /* Affichage du numéro de version */
     { printf(" Watchdogd - Version %s\n", VERSION );
       exit(EXIT_OK);
     }

    Lire_config( file );                                                        /* Lecture sur le fichier /etc/watchdogd.conf */

    if (single)          Config.single      = TRUE;                                            /* Demarrage en mode single ?? */
    if (log_level!=-1)   Config.log_level   = log_level;
    if (compil)          Config.compil      = 1;                                   /* Compilation de tous les plugins D.L.S ? */
    if (home)            g_snprintf( Config.home,   sizeof(Config.home),   "%s", home );
    if (run_as)          g_snprintf( Config.run_as, sizeof(Config.run_as), "%s", run_as );

    pwd = getpwnam ( Config.run_as );
    if (!pwd)
     { printf("Error, user '%s' not found in /etc/passwd (%s).. Could not set run_as user\n",
              Config.run_as, strerror(errno) );
       exit(EXIT_ERREUR);
     }
    else printf("User '%s' (uid %d) found.\n", Config.run_as, pwd->pw_uid);

    old = getpwuid ( getuid() );
    if (!old)
     { printf("Error, actual user '%d' not found in /etc/passwd (%s).. Could not set run_as user\n",
              getuid(), strerror(errno) );
       exit(EXIT_ERREUR);
     }
    
    if (old->pw_uid != pwd->pw_uid)                                                      /* Besoin de changer d'utilisateur ? */
     { printf("Dropping privileges '%s' (%d) -> '%s' (%d).\n", old->pw_name, old->pw_uid, pwd->pw_name, pwd->pw_uid );
       if (initgroups ( Config.run_as, pwd->pw_gid )==-1)                                           /* On drop les privilèges */
        { printf("Error, cannot Initgroups for user '%s' (%s)\n",
                 Config.run_as, strerror(errno) );
          exit(EXIT_ERREUR);
        }

       if (setgid ( pwd->pw_gid )==-1)                                                              /* On drop les privilèges */
        { printf("Error, cannot setGID for user '%s' (%s)\n",
                 Config.run_as, strerror(errno) );
          exit(EXIT_ERREUR);
        }

       if (setuid ( pwd->pw_uid )==-1)                                                              /* On drop les privilèges */
        { printf("Error, cannot setUID for user '%s' (%s)\n",
                 Config.run_as, strerror(errno) );
          exit(EXIT_ERREUR);
        }
     }
       
    if (chdir(Config.home))                                                             /* Positionnement à la racine du home */
     { printf( "Chdir %s failed\n", Config.home ); exit(EXIT_ERREUR); }
    else
     { printf( "Chdir %s successfull. PID=%d\n", Config.home, getpid() ); }

    return(fg);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du serveur watchdog                                                                              */
/* Entrée: argc, argv                                                                                                         */
/* Sortie: -1 si erreur, 0 si ok                                                                                              */
/******************************************************************************************************************************/
 int main ( int argc, char *argv[], char *envp[] )
  { struct itimerval timer;
    struct sigaction sig;
    gchar strpid[12];
    gint fd_lock;
    pthread_t TID;
    gint import=0;
    gboolean fg;

    umask(022);                                                                              /* Masque de creation de fichier */
    fg = Lire_ligne_commande( argc, argv );                                       /* Lecture du fichier conf et des arguments */

    if (fg == FALSE)                                                                       /* On tourne en tant que daemon ?? */
     { gint pid;
       pid = fork();
       if (pid<0) { printf("Fork 1 failed\n"); exit(EXIT_ERREUR); }                                           /* On daemonize */
       if (pid>0) exit(EXIT_OK);                                                                           /* On kill le père */
      
       pid = fork();
       if (pid<0) { printf("Fork 2 failed\n"); exit(EXIT_ERREUR); }                         /* Evite des pb (linuxmag 44 p78) */
       if (pid>0) exit(EXIT_OK);                                                                           /* On kill le père */

       setsid();                                                                                 /* Indépendance du processus */
     }
                                                                                      /* Verification de l'unicité du process */
    fd_lock = open( VERROU_SERVEUR, O_RDWR | O_CREAT | O_SYNC, 0640 );
    if (fd_lock<0)
     { printf( "Lock file creation failed: %s/%s\n", Config.home, VERROU_SERVEUR );
       exit(EXIT_ERREUR);
     }
    if (flock( fd_lock, LOCK_EX | LOCK_NB )<0)                                         /* Creation d'un verrou sur le fichier */
     { printf( "Cannot lock %s/%s. Probably another daemon is running : %s\n",
                Config.home, VERROU_SERVEUR, strerror(errno) );
       close(fd_lock);
       exit(EXIT_ERREUR);
     }
    fcntl(fd_lock, F_SETFD, FD_CLOEXEC );                                                           /* Set close on exec flag */
    g_snprintf( strpid, sizeof(strpid), "%d\n", getpid() );                                /* Enregistrement du pid au cas ou */
    if (write( fd_lock, strpid, strlen(strpid) )<0)
     { printf( "Cannot write PID on %s/%s (%s)\n", Config.home, VERROU_SERVEUR, strerror(errno) ); }

    Config.log = Info_init( "Watchdogd", Config.log_level );                                           /* Init msgs d'erreurs */

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Start v%s", VERSION );
    Print_config();

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    gcry_check_version(NULL);                                                        /* Initialisation de la librairie GCRYPT */
    curl_global_init (CURL_GLOBAL_ALL);                                                 /* Initialisation de la libraire CURL */
    Partage = NULL;                                                                                         /* Initialisation */
    Partage = Shm_init();                                                            /* Initialisation de la mémoire partagée */
    if (!Partage)
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "Shared memory failed to allocate" ); }
    else
     { pthread_mutexattr_t attr;                                                       /* Initialisation des mutex de synchro */
       memset( Partage, 0, sizeof(struct PARTAGE) );                                                 /* RAZ des bits internes */
       import = Importer();                                             /* Tente d'importer les données juste après un reload */
       time ( &Partage->start_time );
       pthread_mutexattr_init( &attr );
       pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_SHARED );
       pthread_mutex_init( &Partage->com_msrv.synchro, &attr );
       pthread_mutex_init( &Partage->com_dls.synchro, &attr );
       pthread_mutex_init( &Partage->com_dls.synchro_traduction, &attr );
       pthread_mutex_init( &Partage->com_arch.synchro, &attr );
       pthread_mutex_init( &Partage->com_admin.synchro, &attr );
       pthread_mutex_init( &Partage->com_db.synchro, &attr );

       sigfillset (&sig.sa_mask);                             /* Par défaut tous les signaux sont bloqués */
       pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );

       if (!import)
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "Clear Histo" );
          Clear_histoDB ();                                            /* Clear de la table histo au boot */
          Info_new( Config.log, Config.log_msrv, LOG_INFO, "Clear Histo done" );
        } else Info_new( Config.log, Config.log_msrv, LOG_INFO, "Import => pas de clear histo" );

       Update_database_schema();                                /* Update du schéma de Database si besoin */
       Charger_config_bit_interne ();     /* Chargement des configurations des bits internes depuis la DB */

       Partage->zmq_ctx = zmq_ctx_new ();                                                   /* Initialisation du context d'echange ZMQ */
       if (!Partage->zmq_ctx)
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Init ZMQ Context Failed (%s)", __func__, zmq_strerror(errno) ); }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Init ZMQ Context OK", __func__ ); }

       if (Config.single == FALSE)                                                                 /* Si demarrage des thread */
        { if (!Config.instance_is_master)
           { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Arch Thread is administratively DOWN (instance is not Master)" ); }
          else if (!Demarrer_arch())                                            /* Demarrage gestion Archivage */
           { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Pb ARCH" ); }

          if (!Config.instance_is_master)
           { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "D.L.S Thread is administratively DOWN (instance is not Master)" ); }
          else if (!Demarrer_dls())                                                        /* Démarrage D.L.S. */
           { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Pb DLS" ); }

          Charger_librairies();                           /* Chargement de toutes les librairies Watchdog */
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "NOT starting threads (single mode=true)" ); }

       if (!Demarrer_admin())                                                          /* Démarrage ADMIN */
        { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Pb Admin -> Arret" ); }

       if ( pthread_create( &TID, NULL, (void *)Boucle_pere, NULL ) )
        { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                   "Demarrage boucle sans fin pthread_create failed %s", strerror(errno) );
        }

                                                         /********** Mise en place de la gestion des signaux ******************/
       sig.sa_handler = Traitement_signaux;                                         /* Gestionnaire de traitement des signaux */
       sig.sa_flags = SA_RESTART;                         /* Voir Linux mag de novembre 2002 pour le flag anti cut read/write */
       sigaction( SIGALRM, &sig, NULL );                                                             /* Reinitialisation soft */
       sigaction( SIGUSR1, &sig, NULL );                                                   /* Reinitialisation DLS uniquement */
       sigaction( SIGUSR2, &sig, NULL );                                                   /* Reinitialisation DLS uniquement */
       sigaction( SIGINT,  &sig, NULL );                                                             /* Reinitialisation soft */
       sigaction( SIGTERM, &sig, NULL );
       sigaction( SIGABRT, &sig, NULL );
       sigaction( SIGPIPE, &sig, NULL );                                               /* Pour prevenir un segfault du client */
       sigfillset (&sig.sa_mask);                                                 /* Par défaut tous les signaux sont bloqués */
       sigdelset ( &sig.sa_mask, SIGALRM );
       sigdelset ( &sig.sa_mask, SIGUSR1 );
       sigdelset ( &sig.sa_mask, SIGUSR2 );
       sigdelset ( &sig.sa_mask, SIGINT  );
       sigdelset ( &sig.sa_mask, SIGTERM );
       sigdelset ( &sig.sa_mask, SIGABRT );
       sigdelset ( &sig.sa_mask, SIGPIPE );
       pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );

       timer.it_value.tv_sec = timer.it_interval.tv_sec = 0;                                    /* Tous les 100 millisecondes */
       timer.it_value.tv_usec = timer.it_interval.tv_usec = 100000;                                 /* = 10 fois par secondes */
       setitimer( ITIMER_REAL, &timer, NULL );                                                             /* Active le timer */

       pthread_join( TID, NULL );                                                       /* Attente fin de la boucle pere MSRV */
       Decharger_librairies();                                                /* Déchargement de toutes les librairies filles */
       Stopper_fils(TRUE);                                                                 /* Arret de tous les fils watchdog */
       zmq_ctx_term( Partage->zmq_ctx );
       zmq_ctx_destroy( Partage->zmq_ctx );
     }

    pthread_mutex_destroy( &Partage->com_msrv.synchro );
    pthread_mutex_destroy( &Partage->com_dls.synchro );
    pthread_mutex_destroy( &Partage->com_dls.synchro_traduction );
    pthread_mutex_destroy( &Partage->com_arch.synchro );
    pthread_mutex_destroy( &Partage->com_admin.synchro );
    pthread_mutex_destroy( &Partage->com_db.synchro );

    close(fd_lock);                                           /* Fermeture du FileDescriptor correspondant au fichier de lock */

    if (Partage->com_msrv.Thread_clear_reboot == FALSE) Exporter();                           /* Tente d'exporter les données */
    else { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "CLEAR-REBOOT : Erasing export file %s", FICHIER_EXPORT );
           unlink ( FICHIER_EXPORT );
         }

    if (Partage->com_msrv.Thread_reboot == TRUE)                                         /* Devons-nous rebooter le process ? */
     { gint pid;
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Rebooting in progress cmd = %s", argv[0] );
       pid = fork();
       if (pid<0) { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "Fork Failed on reboot" );
                    printf("Fork 1 failed\n"); exit(EXIT_ERREUR); }                                           /* On daemonize */
       if (pid==0)
        { sleep(5);
          execvp ( argv[0], argv );
          Info_new( Config.log, Config.log_msrv, LOG_CRIT, "Rebooting ERROR (%s) !", strerror(errno) );
          exit(EXIT_ERREUR);
        }
     }

    sigfillset (&sig.sa_mask);                                                    /* Par défaut tous les signaux sont bloqués */
    pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );
    curl_global_cleanup();
    Shm_stop( Partage );                                                                       /* Libération mémoire partagée */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Stopped" );
    return(EXIT_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
