/**********************************************************************************************************/
/* Watchdogd/Watchdogd.c        Démarrage/Arret du systeme Watchdog, gestion des connexions clientes      */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 14 fév 2006 15:56:40 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

 #include <glib.h>
 #include <openssl/ssl.h>
 #include <openssl/err.h>
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
 #include <popt.h>
 #include <pthread.h>
 #include <locale.h>

 #include "watchdogd.h"

 gint Socket_ecoute;                                         /* Socket de connexion (d'écoute) du serveur */

 extern SSL_CTX *Ssl_ctx;                                          /* Contexte de cryptage des connexions */
 struct CONFIG Config;                   /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 struct PARTAGE *Partage;                                    /* Accès aux données partagées des processes */

/**********************************************************************************************************/
/* Exporter : Exporte les données de base Watchdog pour préparer le RELOAD                                */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Exporter ( void )
  { int fd;
    unlink ( FICHIER_EXPORT );
    Partage->taille_partage = sizeof(struct PARTAGE);
    fd = open( FICHIER_EXPORT, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd>0) { write (fd, Partage, sizeof(struct PARTAGE) );
                Info_c( Config.log, DEBUG_CONFIG, "Donnees exportées", FICHIER_EXPORT );
              }
    else      { Info_c( Config.log, DEBUG_CONFIG, "Could not export", strerror(errno) ); }
    close (fd);
  }
/**********************************************************************************************************/
/* Importe : Tente d'importer les données de base Watchdog juste apres le reload                         */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static gint Importer ( void )
  { int fd;
    fd = open( FICHIER_EXPORT, O_RDONLY );
    if (fd>0) { read (fd, Partage, sizeof(struct PARTAGE) );
                Info_c( Config.log, DEBUG_CONFIG, "Import : Donnees importées... Checking size", FICHIER_EXPORT );
                if (Partage->taille_partage != sizeof(struct PARTAGE) )
                 { memset( Partage, 0, sizeof(struct PARTAGE) );
                   Info( Config.log, DEBUG_CONFIG, "Import: Wrong size .. zeroing ..." );
                 }
                else
                 { Info( Config.log, DEBUG_CONFIG, "Import: Size OK" ); }
                close (fd);
                return(1);
              }
    else      { memset( Partage, 0, sizeof(struct PARTAGE) );
                Info( Config.log, DEBUG_CONFIG, "Import: no file .. zeroing ..." );
              }
   return(0);
  }
/**********************************************************************************************************/
/* Charger_config_bit_interne: Chargement des configs bit interne depius la base de données               */
/* Entrée: néant                                                                                          */
/**********************************************************************************************************/
 static void Charger_config_bit_interne( void )
  { Info_new( Config.log, Config.log_all, LOG_INFO, "Chargement des EANA" );
    Charger_eana();
    Info_new( Config.log, Config.log_all, LOG_INFO, "Chargement des EANA fait" );

    Info_new( Config.log, Config.log_all, LOG_INFO, "Chargement des SCENARIO" );
    Charger_scenario();
    Info_new( Config.log, Config.log_all, LOG_INFO, "Chargement des SCENARIO fait" );

    Info_new( Config.log, Config.log_all, LOG_INFO, "Chargement des compteurs horaires" );
    Charger_cpth();
    Info_new( Config.log, Config.log_all, LOG_INFO, "Chargement des compteurs horaires fait" );

    Info_new( Config.log, Config.log_all, LOG_INFO, "Chargement des compteurs impulsion" );
    Charger_cpt_imp();
    Info_new( Config.log, Config.log_all, LOG_INFO, "Chargement des compteurs impulsion fait" );
  }
/**********************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme                                         */
/* Entrée: numero du signal à gerer                                                                       */
/**********************************************************************************************************/
 static void Traitement_signaux( int num )
  { char chaine[50];
    if (num == SIGALRM)
     { Partage->top++;
       if (!Partage->top)                         /* Si on passe par zero, on le dit (DEBUG interference) */
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Traitement Signaux: Timer: Partage->top = 0 !!" ); }
       if (!(Partage->top%5))                                          /* Cligno toutes les demi-secondes */
        { SB(5, !B(5)); }
       if (!(Partage->top%3))                                             /* Cligno toutes les 3 dixièmes */
        { SB(6, !B(6)); }
       if (!(Partage->top%10))                                              /* Cligno toutes les secondes */
        { SB(4, !B(4));
          Partage->audit_bit_interne_per_sec_hold += Partage->audit_bit_interne_per_sec;
          Partage->audit_bit_interne_per_sec_hold = Partage->audit_bit_interne_per_sec_hold >> 1;
          Partage->audit_bit_interne_per_sec = 0;

          Partage->audit_tour_dls_per_sec_hold += Partage->audit_tour_dls_per_sec;
          Partage->audit_tour_dls_per_sec_hold = Partage->audit_tour_dls_per_sec_hold >> 1;
          Partage->audit_tour_dls_per_sec = 0;
          if (Partage->audit_tour_dls_per_sec_hold > 100)                       /* Moyennage tour DLS/sec */
           { Partage->com_dls.temps_sched += 50; }
          else if (Partage->audit_tour_dls_per_sec_hold < 50)
           { if (Partage->com_dls.temps_sched) Partage->com_dls.temps_sched -= 10; }
        }

       Partage->top_cdg_plugin_dls++;                                        /* Chien de garde plugin DLS */
       if (Partage->top_cdg_plugin_dls>200)                     /* Si pas de réponse D.L.S en 20 secondes */
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Traitement signaux: CDG plugin DLS !!" );
          Partage->top_cdg_plugin_dls = 0;
        }
       return;
     }

    prctl(PR_GET_NAME, chaine, 0, 0, 0 );
    Info_c( Config.log, DEBUG_INFO, "Traitement_signaux: traité par", chaine );

    switch (num)
     { case SIGQUIT:
       case SIGINT:  Info_new( Config.log, Config.log_all, LOG_INFO, "Recu SIGINT" );
                     Partage->com_msrv.Thread_run = FALSE;   /* On demande l'arret de la boucle programme */
                     break;
       case SIGTERM: Info_new( Config.log, Config.log_all, LOG_INFO, "Recu SIGTERM" );
                     Partage->com_msrv.Thread_run = FALSE;   /* On demande l'arret de la boucle programme */
                     break;
       case SIGCHLD: Info_new( Config.log, Config.log_all, LOG_INFO, "Recu SIGCHLD" );
                     break;
       case SIGPIPE: Info_new( Config.log, Config.log_all, LOG_INFO, "Recu SIGPIPE" ); break;
       case SIGBUS:  Info_new( Config.log, Config.log_all, LOG_INFO, "Recu SIGBUS" ); break;
       case SIGIO:   Info_new( Config.log, Config.log_all, LOG_INFO, "Recu SIGIO" ); break;
       case SIGUSR1: Info_new( Config.log, Config.log_all, LOG_INFO, "Recu SIGUSR1: dumping infos" );
                     Partage->com_msrv.Thread_sigusr1 = TRUE;
                     break;
       case SIGUSR2: Info_new( Config.log, Config.log_all, LOG_INFO, "Recu SIGUSR2: Reloading THREAD in progress" );
                     Partage->com_msrv.Thread_reload = TRUE;
                     break;
       default: Info_n( Config.log, DEBUG_INFO, "Recu signal", num ); break;
     }
  }
/**********************************************************************************************************/
/* Sauver_compteur : Envoie les infos Compteurs à la base de données pour sauvegarde !                    */
/* Entrée : Néant                                                                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Sauver_compteur ( struct DB *db )
  { gint cpt;
    for( cpt=0; cpt<NBR_COMPTEUR_H;   cpt++) { Updater_cpthDB( Config.log, db, &Partage->ch[cpt].cpthdb); }     
    for( cpt=0; cpt<NBR_COMPTEUR_IMP; cpt++) { Updater_cpt_impDB( Config.log, db, &Partage->ci[cpt].cpt_impdb); }     
  }
/**********************************************************************************************************/
/* Tatiter_sigusr1 : Print les variable importante dans les lgos                                          */
/* Entrée : Néant                                                                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Traiter_sigusr1 ( void )
  { guint nbr_i, nbr_msg_on, nbr_msg_off, nbr_msg_repeat;
    gchar chaine[256];

    pthread_mutex_lock( &Partage->com_msrv.synchro );
    nbr_i          = g_list_length( Partage->com_msrv.liste_i );
    nbr_msg_off    = g_list_length( Partage->com_msrv.liste_msg_off );     /* Recuperation du numero de i */
    nbr_msg_on     = g_list_length( Partage->com_msrv.liste_msg_on );      /* Recuperation du numero de i */
    nbr_msg_repeat = g_list_length( Partage->com_msrv.liste_msg_repeat );             /* liste des repeat */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    g_snprintf( chaine, sizeof(chaine), "Reste %d I, %d MSG_ON, %d MSG_OFF, %d MSG_REPEAT",
                nbr_i, nbr_msg_on, nbr_msg_off, nbr_msg_repeat );
    Info_new( Config.log, Config.log_all, LOG_INFO, chaine );
  }
/**********************************************************************************************************/
/* Boucle_pere: boucle de controle du pere de tous les serveurs                                           */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void *Boucle_pere ( void )
  { gint cpt_5_minutes, cpt_1_minute, cpt_1_seconde;
    sigset_t sigset;
    struct DB *db;
    gint cpt;

    prctl(PR_SET_NAME, "W-MSRV", 0, 0, 0 );

    Info_new( Config.log, Config.log_all, LOG_INFO, "Boucle_pere: Debut boucle sans fin" );
    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "Boucle_pere: Connexion DB impossible" ); }

    cpt_5_minutes = Partage->top + 3000;
    cpt_1_minute  = Partage->top + 600;
    cpt_1_seconde = Partage->top + 10;

    sigfillset (&sigset);                                     /* Par défaut tous les signaux sont bloqués */
    sigdelset (&sigset, SIGALRM);
    sigdelset (&sigset, SIGPIPE);
    sigdelset (&sigset, SIGHUP);
    sigdelset (&sigset, SIGUSR1);
    sigdelset (&sigset, SIGUSR2);
    sigdelset (&sigset, SIGTERM);
    pthread_sigmask( SIG_SETMASK, &sigset, NULL );

    sleep(1);
    Partage->com_msrv.Thread_run = TRUE;                         /* On dit au maitre que le thread tourne */
    while(Partage->com_msrv.Thread_run == TRUE)                       /* On tourne tant que l'on a besoin */
     { Gerer_arrive_MSGxxx_dls( db );         /* Redistrib des messages DLS vers les clients + Historique */ 
       Gerer_arrive_Ixxx_dls();                             /* Distribution des changements d'etats motif */

       if (Partage->com_msrv.Thread_reload)                                           /* On a recu RELOAD */
        { guint i;
          Info_new( Config.log, Config.log_all, LOG_INFO, "Boucle_pere: RELOAD" );
          Partage->com_modbus.Thread_reload    = TRUE;
          Partage->com_sms.Thread_reload       = TRUE;
          Partage->com_dls.Thread_reload       = TRUE;
          Partage->com_arch.Thread_reload      = TRUE;
          Partage->com_audio.Thread_reload     = TRUE;
          Partage->com_onduleur.Thread_reload  = TRUE;
          Partage->com_lirc.Thread_reload      = TRUE;
          Partage->com_tellstick.Thread_reload = TRUE;
          for (i=0; i<Config.max_serveur; i++)
           { Partage->Sous_serveur[i].Thread_reload = TRUE; }

          Lire_config( NULL );                              /* Lecture sur le fichier /etc/watchdogd.conf */
          Print_config();
          Info_change_log_level ( Config.log, Config.log_level );
          Charger_config_bit_interne();                         /* Rechargement des configs bits internes */
          Partage->com_msrv.Thread_reload      = FALSE;                             /* signal traité. RAZ */
        }

       if (Partage->com_msrv.Thread_sigusr1)                                      /* On a recu sigusr1 ?? */
        { struct LIBRAIRIE *lib;
          GSList *liste;
          guint i;

          Info_new( Config.log, Config.log_all, LOG_INFO, "Boucle_pere: SIGUSR1" );
          Info_n( Config.log, DEBUG_INFO, "Recu SIGUSR1: jeton", Partage->jeton );
          Partage->com_modbus.Thread_sigusr1    = TRUE;
          Partage->com_sms.Thread_sigusr1       = TRUE;
          Partage->com_dls.Thread_sigusr1       = TRUE;
          Partage->com_arch.Thread_sigusr1      = TRUE;
          Partage->com_audio.Thread_sigusr1     = TRUE;
          Partage->com_onduleur.Thread_sigusr1  = TRUE;
          Partage->com_admin.Thread_sigusr1     = TRUE;
          Partage->com_lirc.Thread_sigusr1      = TRUE;
          Partage->com_tellstick.Thread_sigusr1 = TRUE;
          for (i=0; i<Config.max_serveur; i++)
           { Partage->Sous_serveur[i].Thread_sigusr1 = TRUE; }

          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             lib->Thread_sigusr1 = TRUE;
             liste = liste->next;
           }

          Traiter_sigusr1();                     /* Appel de la fonction pour traiter le signal pour MSRV */
          Partage->com_msrv.Thread_sigusr1      = FALSE;
        }

       if (cpt_5_minutes < Partage->top)                                /* Update DB toutes les 5 minutes */
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Boucle_pere: Sauvegarde des CPT" );
          Sauver_compteur( db );
          Exporter();
          cpt_5_minutes = Partage->top + 3000;                         /* Sauvegarde toutes les 5 minutes */
        }

       if (cpt_1_minute < Partage->top)                                   /* Update DB toutes les minutes */
        { for( cpt=0; cpt<NBR_SCENARIO; cpt++)
           { Checker_scenario( cpt ); }
          Gerer_manque_process();                            /* Detection du manque de serveurs en ecoute */
          Gerer_message_repeat(db);
          cpt_1_minute = Partage->top + 600;                             /* Sauvegarde toutes les minutes */
        }

       if (cpt_1_seconde < Partage->top)           /* Toutes les secondes vérification des motion cameras */
        { Camera_check_motion( Config.log, db );
          Asterisk_check_call( Config.log, db );
          Gerer_jeton();                                       /* Don du jeton au serveur le moins chargé */
          cpt_1_seconde = Partage->top + 10;                                        /* Dans une seconde ! */
        }

       if (Partage->com_msrv.reset_motion_detect)
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Boucle_pere: Reset_motion_detect" );
          Demarrer_motion_detect();
          Partage->com_msrv.reset_motion_detect = FALSE;
        }
       usleep(1000);
       sched_yield();
     }

/**************************** Terminaison: Deconnexion DB et kill des serveurs ****************************/ 
    Sauver_compteur( db );                                             /* Dernière sauvegarde avant arret */
    Info_new( Config.log, Config.log_all, LOG_INFO, "Boucle_pere: fin boucle sans fin" );
    Libere_DB_SQL( Config.log, &db );
    pthread_exit( NULL );
  }
/**********************************************************************************************************/
/* Lire_ligne_commande: Parse la ligne de commande pour d'eventuels parametres                            */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 static gboolean Lire_ligne_commande( int argc, char *argv[] )
  { gint help, port, log_level, max_client, fg, initrsa, single, compil;
    gchar *home, *file;
    gint nbr_bytes;
    gchar *chaine;
    FILE *fd;
    struct poptOption Options[]= 
     { { "port", 'p',       POPT_ARG_INT,
         &port,             0, "Port to listen to", "PORT" },
       { "foreground", 'f', POPT_ARG_NONE,
         &fg,               0, "Run in foreground", NULL },
       { "initrsa",    'r', POPT_ARG_NONE,
         &initrsa,          0, "RSA initialisation", NULL },
       { "debug",      'd', POPT_ARG_INT,
         &log_level,      0, "Debug level", "LEVEL" },
       { "max_client", 'm', POPT_ARG_INT,
         &max_client,       0, "Maximum of connexions allowed", "MAX" },
       { "home",       'H', POPT_ARG_STRING,
         &home,             0, "Home directory", "HOME" },
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

    home = NULL;
    file = NULL;
    port           = -1;
    max_client     = -1;
    log_level    = -1;
    initrsa        = 0;
    fg             = 0;
    help           = 0;
    compil         = 0;
    single         = 0;

    context = poptGetContext( NULL, argc, (const char **)argv, Options, POPT_CONTEXT_ARG_OPTS );
    while ( (rc = poptGetNextOpt( context )) != -1)                      /* Parse de la ligne de commande */
     { switch (rc)
        { case POPT_ERROR_BADOPT: printf( "Option %s unknown\n", poptBadOption(context, 0) );
                                  help=1; break;
          default: printf("Erreur de parsing ligne de commande\n");
        }
     }

    if (help)                                                             /* Affichage de l'aide en ligne */
     { poptPrintHelp(context, stdout, 0);
       poptFreeContext(context);
       exit(EXIT_OK);
     }
    poptFreeContext( context );                                                     /* Liberation memoire */

    Lire_config( file );                                    /* Lecture sur le fichier /etc/watchdogd.conf */

    if (single)          Config.single      = TRUE;                        /* Demarrage en mode single ?? */
    if (port!=-1)        Config.port        = port;                    /* Priorite à la ligne de commande */
    if (log_level!=-1)   Config.log_level   = log_level;
    if (max_client!=-1)  Config.max_client  = max_client;
    if (compil)          Config.compil      = 1;               /* Compilation de tous les plugins D.L.S ? */
    if (home)            g_snprintf( Config.home, sizeof(Config.home), "%s", home );

    if (chdir(Config.home))                                         /* Positionnement à la racine du home */
     { printf( "Chdir %s failed\n", Config.home ); exit(EXIT_ERREUR); }

    if (initrsa)
     { RSA *rsa;

       nbr_bytes = Config.taille_clef_rsa>>3;
       chaine = g_malloc0( nbr_bytes );
       if (!chaine)
        { printf( " Not enough memory\n" );
          exit(EXIT_OK);
        }

       rsa = RSA_generate_key( Config.taille_clef_rsa, 65537, NULL, NULL );
       if (!rsa)
        { printf( " Could not generate RSA keys: %s\n", ERR_error_string( ERR_get_error(), NULL ) );
        }
       else 
        { printf( " RSA keys generation OK\n" );
          fd = fopen( FICHIER_CLEF_SEC_RSA, "w+" );
          if (!fd)
           { printf( "Could not open file %s\n", FICHIER_CLEF_SEC_RSA ); }
          else
           { gint ok;
             ok = PEM_write_RSAPrivateKey( fd, rsa, NULL, NULL, 0, NULL, NULL );
             if (!ok) printf( "Could not write into %s\n", FICHIER_CLEF_SEC_RSA );
             fclose(fd);
           }

          fd = fopen( FICHIER_CLEF_PUB_RSA, "w+" );
          if (!fd)
           { printf( "Could not open file %s\n", FICHIER_CLEF_PUB_RSA ); }
          else
           { gint ok;
             ok = PEM_write_RSAPublicKey( fd, rsa );
             if (!ok) printf( "Could not write into %s\n", FICHIER_CLEF_PUB_RSA );
             fclose(fd);
           }
        }
       g_free(chaine);
       exit(EXIT_OK);
     }
     
    Config.rsa = NULL;
    fd = fopen( FICHIER_CLEF_PUB_RSA, "r" );
    if (!fd)
     { printf( "Could not open file %s\n", FICHIER_CLEF_PUB_RSA ); exit(EXIT_OK); }
    Config.rsa = PEM_read_RSAPublicKey( fd, NULL, NULL, NULL );
    if (!Config.rsa) printf("Unable to load %s\n", FICHIER_CLEF_PUB_RSA );
    fclose(fd); 

    fd = fopen( FICHIER_CLEF_SEC_RSA, "r" );
    if (!fd)
     { printf( "Could not open file %s\n", FICHIER_CLEF_SEC_RSA ); exit(EXIT_OK); }
    Config.rsa = PEM_read_RSAPrivateKey( fd, &Config.rsa, NULL, NULL );
    if (!Config.rsa) printf("Unable to load %s\n", FICHIER_CLEF_SEC_RSA );
    fclose(fd); 

    return(fg);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du serveur watchdog                                                          */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 int main ( int argc, char *argv[], char *envp[] )
  { struct sigaction sig;
    gchar strpid[12];
    gint fd_lock, i;
    pthread_t TID;
    gint import=0;
    gboolean fg;

    umask(022);                                                          /* Masque de creation de fichier */
    fg = Lire_ligne_commande( argc, argv );                   /* Lecture du fichier conf et des arguments */
    printf(" Going to background : %s\n", (fg ? "FALSE" : "TRUE") );

    if (fg == FALSE)                                                   /* On tourne en tant que daemon ?? */
     { gint pid;
       pid = fork();
       if (pid<0) { printf("Fork 1 failed\n"); exit(EXIT_ERREUR); }                       /* On daemonize */
       if (pid>0) exit(EXIT_OK);                                                       /* On kill le père */
      
       setsid();                                                             /* Indépendance du processus */
      
       pid = fork();
       if (pid<0) { printf("Fork 2 failed\n"); exit(EXIT_ERREUR); }     /* Evite des pb (linuxmag 44 p78) */
       if (pid>0) exit(EXIT_OK);                                                       /* On kill le père */

       setsid();                                                             /* Indépendance du processus */
    }
                                                                  /* Verification de l'unicité du process */
    fd_lock = open( VERROU_SERVEUR, O_RDWR | O_CREAT | O_SYNC, 0640 );
    if (fd_lock<0)
     { printf( "Lock file creation failed: %s/%s\n", Config.home, VERROU_SERVEUR );
       exit(EXIT_ERREUR);
     }
    if (flock( fd_lock, LOCK_EX | LOCK_NB )<0)                     /* Creation d'un verrou sur le fichier */
     { printf( "Cannot lock %s/%s. Probably another daemon is running : %s\n",
                Config.home, VERROU_SERVEUR, strerror(errno) );
       close(fd_lock);
       exit(EXIT_ERREUR);
     }
    fcntl(fd_lock, F_SETFD, FD_CLOEXEC );                                       /* Set close on exec flag */
    g_snprintf( strpid, sizeof(strpid), "%d\n", getpid() );            /* Enregistrement du pid au cas ou */
    write( fd_lock, strpid, strlen(strpid) );

    g_thread_init(NULL);   /* Initialisation de la glib en environnement multi-threadé (obsolete en 2.32) */
    Config.log = Info_init( "Watchdogd", Config.log_level );                       /* Init msgs d'erreurs */

    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Start" );
    Print_config();

    Socket_ecoute = Activer_ecoute();                             /* Initialisation de l'écoute via TCPIP */
    if ( Socket_ecoute<0 )            
     { Info_new( Config.log, Config.log_all, LOG_CRIT, "Network down, foreign connexions disabled" );
       return(EXIT_OK);
     }

    setlocale( LC_ALL, "C" );                        /* Pour le formattage correct des , . dans les float */
    Partage = NULL;                                                                     /* Initialisation */
    Partage = Shm_init();                                        /* Initialisation de la mémoire partagée */
    if (!Partage)
     { Info_new( Config.log, Config.log_all, LOG_CRIT, "Shared memory failed to allocate" ); }
    else
     { pthread_mutexattr_t attr;                                   /* Initialisation des mutex de synchro */
       memset( Partage, 0, sizeof(struct PARTAGE) );                             /* RAZ des bits internes */
       import = Importer();                         /* Tente d'importer les données juste après un reload */

       memset( &Partage->com_msrv,     0, sizeof(Partage->com_msrv) );
       memset( &Partage->com_modbus,   0, sizeof(Partage->com_modbus) );
       memset( &Partage->com_sms,      0, sizeof(Partage->com_sms) );
       memset( &Partage->com_dls,      0, sizeof(Partage->com_dls) );
       memset( &Partage->com_arch,     0, sizeof(Partage->com_arch) );
       memset( &Partage->com_audio,    0, sizeof(Partage->com_audio) );
       memset( &Partage->com_onduleur, 0, sizeof(Partage->com_onduleur) );
       memset( &Partage->com_admin,    0, sizeof(Partage->com_admin) );
       memset( &Partage->com_lirc,     0, sizeof(Partage->com_lirc) );
       memset( &Partage->com_tellstick,0, sizeof(Partage->com_tellstick) );

       Partage->jeton            = -1;                           /* Initialisation de la mémoire partagée */
       
       pthread_mutexattr_init( &attr );
       pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_SHARED );
       pthread_mutex_init( &Partage->com_sms.synchro, &attr );
       pthread_mutex_init( &Partage->com_msrv.synchro, &attr );
       pthread_mutex_init( &Partage->com_dls.synchro, &attr );
       pthread_mutex_init( &Partage->com_arch.synchro, &attr );
       pthread_mutex_init( &Partage->com_audio.synchro, &attr );
       pthread_mutex_init( &Partage->com_onduleur.synchro, &attr );
       pthread_mutex_init( &Partage->com_admin.synchro, &attr );
       pthread_mutex_init( &Partage->com_tellstick.synchro, &attr );
       pthread_mutex_init( &Partage->com_lirc.synchro, &attr );
       pthread_mutex_init( &Partage->com_modbus.synchro, &attr );
 
       Partage->Sous_serveur = &Partage->ss_serveur;                 /* Initialisation du pointeur global */
       for (i=0; i<Config.max_serveur; i++)
        { Partage->Sous_serveur[i].Thread_run = FALSE;
          Partage->Sous_serveur[i].Thread_sigusr1 = FALSE;
          Partage->Sous_serveur[i].Thread_reload = FALSE;
          Partage->Sous_serveur[i].pid = -1;
          Partage->Sous_serveur[i].nb_client = -1;
          pthread_mutex_init( &Partage->Sous_serveur[i].synchro, &attr );
        }

       sig.sa_handler = Traitement_signaux;                     /* Gestionnaire de traitement des signaux */
       sig.sa_flags = SA_RESTART;     /* Voir Linux mag de novembre 2002 pour le flag anti cut read/write */
       sigfillset (&sig.sa_mask);                             /* Par défaut tous les signaux sont bloqués */
       sigaction( SIGPIPE, &sig, NULL );
       sigaction( SIGHUP,  &sig, NULL );                                         /* Reinitialisation soft */
       sigaction( SIGINT,  &sig, NULL );                                         /* Reinitialisation soft */
       sigaction( SIGALRM, &sig, NULL );                                         /* Reinitialisation soft */
       sigaction( SIGUSR1, &sig, NULL );                               /* Reinitialisation DLS uniquement */
       sigaction( SIGUSR2, &sig, NULL );                               /* Reinitialisation DLS uniquement */
       sigaction( SIGIO, &sig, NULL );                                 /* Reinitialisation DLS uniquement */
       sigaction( SIGTERM, &sig, NULL );

#ifdef bouh
       sigfillset (&sigset);                                  /* Par défaut tous les signaux sont bloqués */
       pthread_sigmask( SIG_SETMASK, &sigset, NULL );
#endif
       if (!import)
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Clear Histo" );
          Clear_histoDB ();                                            /* Clear de la table histo au boot */
          Info_new( Config.log, Config.log_all, LOG_INFO, "Clear Histo fait" );
        } else Info_new( Config.log, Config.log_all, LOG_INFO, "Import => pas de clear histo" );

       Charger_config_bit_interne ();       /* Chargement des configurations des bit interne depuis la DB */

       if (Config.ssl_crypt) { Ssl_ctx = Init_ssl();                                /* Initialisation SSL */
                               if (!Ssl_ctx)
                                { Info_new( Config.log, Config.log_all, LOG_ERR, "Init ssl failed" ); }
                             }
                        else { Ssl_ctx = NULL; }

       if (Config.single == FALSE)                                             /* Si demarrage des thread */
        { if (!Demarrer_arch())                                            /* Demarrage gestion Archivage */
           { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Pb ARCH" ); }

          if (!Demarrer_modbus())                                      /* Demarrage gestion module MODBUS */
           { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Pb MODBUS" ); }

          if (!Demarrer_sms())                                                        /* Démarrage S.M.S. */
           { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Pb SMS" ); }

          if (!Demarrer_audio())                                                   /* Démarrage A.U.D.I.O */
           { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Pb AUDIO" ); }

          if (!Demarrer_dls())                                                        /* Démarrage D.L.S. */
           { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Pb DLS" ); }

          if (!Demarrer_onduleur())                                                 /* Démarrage Onduleur */
           { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Pb ONDULEUR" ); }

          if (!Demarrer_tellstick())                                               /* Démarrage Tellstick */
           { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Pb TELLSTICK" ); }

          if (!Demarrer_lirc())                                                         /* Démarrage Lirc */
           { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Pb LIRC" ); }

          if (!Demarrer_motion_detect())                              /* Démarrage Detection de mouvement */
           { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Pb MOTION_DETECT" ); }

          Charger_librairies();                           /* Chargement de toutes les librairies Watchdog */
        }

       if (!Demarrer_admin())                                                          /* Démarrage ADMIN */
        { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Pb Admin -> Arret" ); }

       pthread_create( &TID, NULL, (void *)Boucle_pere, NULL );
       pthread_join( TID, NULL );                                   /* Attente fin de la boucle pere MSRV */
       Stopper_fils(TRUE);                                             /* Arret de tous les fils watchdog */
       Decharger_librairies();
       if (Config.ssl_crypt) SSL_CTX_free( Ssl_ctx );                               /* Libération mémoire */
     }

    pthread_mutex_destroy( &Partage->com_modbus.synchro );
    pthread_mutex_destroy( &Partage->com_sms.synchro );
    pthread_mutex_destroy( &Partage->com_msrv.synchro );
    pthread_mutex_destroy( &Partage->com_dls.synchro );
    pthread_mutex_destroy( &Partage->com_arch.synchro );
    pthread_mutex_destroy( &Partage->com_audio.synchro );
    pthread_mutex_destroy( &Partage->com_admin.synchro );
    pthread_mutex_destroy( &Partage->com_tellstick.synchro );
    pthread_mutex_destroy( &Partage->com_lirc.synchro );
    for (i=0; i<Config.max_serveur; i++)
     { pthread_mutex_destroy( &Partage->Sous_serveur[i].synchro ); }

    close(fd_lock);                       /* Fermeture du FileDescriptor correspondant au fichier de lock */

    if (Socket_ecoute>0) close(Socket_ecoute);
    if (Config.rsa) RSA_free( Config.rsa );

    if (Partage->com_msrv.Thread_clear_reboot == FALSE) Exporter();       /* Tente d'exporter les données */
    else { Info_new( Config.log, Config.log_all, LOG_NOTICE, "CLEAR-REBOOT : Erasing export file %s", FICHIER_EXPORT );
           unlink ( FICHIER_EXPORT );
         }
    if (Partage->com_msrv.Thread_reboot == TRUE)
     { gint pid;
       Info_new( Config.log, Config.log_all, LOG_NOTICE, "Rebooting ..." );
       pid = fork();
       if (pid<0) { Info_new( Config.log, Config.log_all, LOG_CRIT, "Fork Failed on reboot" );
                    printf("Fork 1 failed\n"); exit(EXIT_ERREUR); }                       /* On daemonize */
       if (pid>0)
        { Shm_stop( Partage );                                             /* Libération mémoire partagée */
          exit(EXIT_OK);                                                               /* On kill le père */
        }
       Info_new( Config.log, Config.log_all, LOG_NOTICE, "Rebooting in progress cmd=", argv[0] );
       sleep(5);
       execvp ( argv[0], argv );
       Info_new( Config.log, Config.log_all, LOG_CRIT, "Rebooting ERROR (%s) !", strerror(errno) );
       exit(EXIT_ERREUR);
     }

    Shm_stop( Partage );                                                   /* Libération mémoire partagée */
    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Stopped" );
    return(EXIT_OK);
  }
/*--------------------------------------------------------------------------------------------------------*/
