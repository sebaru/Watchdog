/**********************************************************************************************************/
/* Watchdogd/Watchdogd.c        Démarrage/Arret du systeme Watchdog, gestion des connexions clientes      */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 14 fév 2006 15:56:40 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Watchdogd.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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
 #include <string.h>
 #include <stdio.h>  /* Pour printf */
 #include <stdlib.h> /* Pour exit */
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/time.h>
 #include <signal.h>
 #include <sys/stat.h>
 #include <popt.h>
 #include <pthread.h>

 #include "Erreur.h"
 #include "Config.h"
 #include "watchdogd.h"
 #include "proto_srv.h"
 #include "Module_dls.h"
 #include "Histo_DB.h"
 #include "EntreeANA_DB.h"
 #include "Cpth_DB.h"
 #include "ValANA_DB.h"

 gint Socket_ecoute;                                         /* Socket de connexion (d'écoute) du serveur */

 extern SSL_CTX *Ssl_ctx;                                          /* Contexte de cryptage des connexions */
 struct CONFIG Config;                   /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 struct PARTAGE *Partage;                                    /* Accès aux données partagées des processes */
/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme                                         */
/* Entrée: numero du signal à gerer                                                                       */
/**********************************************************************************************************/
 static void Traitement_signaux( int num )
  { switch (num)
     { case SIGQUIT:
       case SIGINT:  Info_n( Config.log, DEBUG_INFO, "SIGINT Caught", Partage->Arret );
                     Partage->Arret = FIN;                   /* On demande l'arret de la boucle programme */
                     Info_n( Config.log, DEBUG_INFO, "Arret=fin", Partage->Arret );
                     break;
       case SIGTERM: Info_n( Config.log, DEBUG_INFO, "SIGTERM Caught", Partage->Arret );
                                                                           /* Si arret demandé du serveur */
                     Partage->Arret = FIN;                   /* On demande l'arret de la boucle programme */
                     Info_n( Config.log, DEBUG_INFO, "Arret=fin", Partage->Arret );
                     break;
       case SIGCHLD: Info( Config.log, DEBUG_INFO, "SIGCHLD Caught" ); /* Si arret demandé du serveur */
                     break;
       case SIGALRM: Partage->top++;
                     if (!Partage->top) /* Si on passe par zero, on le dit (DEBUG interference) */
                      { Info( Config.log, DEBUG_INFO, "Traitement Signaux: Timer: Partage->top = 0 !!" ); }
                     if (!(Partage->top%10))                                /* Cligno toutes les secondes */
                      { SB(4, !B(4));
                        Partage->audit_bit_interne_per_sec_hold += Partage->audit_bit_interne_per_sec;
                        Partage->audit_bit_interne_per_sec_hold = Partage->audit_bit_interne_per_sec_hold >> 1;
                        Partage->audit_bit_interne_per_sec = 0;
                      }

                     Partage->top_cdg_plugin_dls++;                          /* Chien de garde plugin DLS */
                     if (Partage->top_cdg_plugin_dls>50)
                      { Info( Config.log, DEBUG_INFO, "Traitement signaux: CDG plugin DLS !!" );
                        Partage->top_cdg_plugin_dls = 0;
                      }
                     break;
       case SIGPIPE: Info( Config.log, DEBUG_INFO, "Recu SIGPIPE" ); break;
       case SIGBUS:  Info( Config.log, DEBUG_INFO, "Recu SIGBUS" ); break;
       case SIGIO:   Info( Config.log, DEBUG_INFO, "Recu SIGIO" ); break;
       case SIGUSR1: { int i;
                       Info( Config.log, DEBUG_INFO, "Recu SIGUSR1: dumping infos" );
                       Info_n( Config.log, DEBUG_INFO, "Recu SIGUSR1: jeton", Partage->jeton );
                       for (i=0; i<Config.max_serveur; i++)
                        { Info_n( Config.log, DEBUG_INFO, "Recu SIGUSR1: SSRV  id", i );
                          Info_n( Config.log, DEBUG_INFO, "Recu SIGUSR1: SSRV pid", Partage->Sous_serveur[i].pid );
                          Info_n( Config.log, DEBUG_INFO, "Recu SIGUSR1: SSRV nbr_client", Partage->Sous_serveur[i].nb_client );
                          if (Partage->Sous_serveur[i].pid) Partage->Sous_serveur[i].sigusr1 = TRUE;
                        }
                       Partage->com_ssrv_dls.sigusr1 = TRUE;
                       Partage->com_rs485.reload   = TRUE;
                       Partage->com_msrv_sms.sigusr1 = TRUE;
                       Partage->com_modbus.reload = TRUE;
                       Partage->com_arch.sigusr1 = TRUE;          
                       Partage->com_audio.sigusr1 = TRUE;          
                       Partage->com_admin.sigusr1 = TRUE;          
                     }
                     break;
       case SIGUSR2: Info( Config.log, DEBUG_INFO, "Recu SIGUSR2: Reloading THREAD in progress" );
                     Partage->Arret = RELOAD;
                     break;
       default: Info_n( Config.log, DEBUG_INFO, "Recu signal", num ); break;
     }
  }
/**********************************************************************************************************/
/* Boucle_pere: boucle de controle du pere de tous les serveurs                                           */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void *Boucle_pere ( void )
  { gint cpth_prochain_save_db;
    gint scenario_test_date;
    struct DB *db;
    gint cpt;

    prctl(PR_SET_NAME, "W-MSRV", 0, 0, 0 );
    Info( Config.log, DEBUG_INFO, "MSRV: Boucle_pere: Debut boucle sans fin" );
    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info( Config.log, DEBUG_INFO, "MSRV: Boucle_pere: Connexion DB impossible" ); }

    cpth_prochain_save_db = Partage->top + 3000;
    scenario_test_date = Partage->top + 100;

    sleep(1);
    while( Partage->Arret < FIN )
     { Gerer_jeton();                                          /* Don du jeton au serveur le moins chargé */
       Gerer_manque_process();                               /* Detection du manque de serveurs en ecoute */
/*     Gerer_fifo_admin();                                       /* Gestion de l'interface d'admin locale */

       Gerer_arrive_MSGxxx_dls( db );         /* Redistrib des messages DLS vers les clients + Historique */ 
       Gerer_arrive_Ixxx_dls();                             /* Distribution des changements d'etats motif */

       if (cpth_prochain_save_db < Partage->top)                        /* Update DB toutes les 5 minutes */
        { Info( Config.log, DEBUG_INFO, "MSRV: Boucle_pere: Sauvegarde des CPTH" );
          for( cpt=0; cpt<NBR_COMPTEUR_H; cpt++)
           { Updater_cpthDB( Config.log, db, &Partage->ch[cpt].cpthdb); }     
          cpth_prochain_save_db = Partage->top + 3000;                 /* Sauvegarde toutes les 5 minutes */
        }

       if (scenario_test_date < Partage->top)                             /* Update DB toutes les minutes */
        { for( cpt=0; cpt<NBR_SCENARIO; cpt++)
           { Checker_scenario( cpt ); }
          scenario_test_date = Partage->top + 600;                       /* Sauvegarde toutes les minutes */
        }

       usleep(1000);
       sched_yield();
     }

/**************************** Terminaison: Deconnexion DB et kill des serveurs ****************************/ 
    Info( Config.log, DEBUG_INFO, "MSRV: Boucle_pere: fin boucle sans fin" );
    Libere_DB_SQL( Config.log, &db );
    pthread_exit( NULL );
  }
/**********************************************************************************************************/
/* Lire_ligne_commande: Parse la ligne de commande pour d'eventuels parametres                            */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 static gboolean Lire_ligne_commande( int argc, char *argv[] )
  { gint help, port, debug_level, max_client, fg, initdb, initrsa;
    gchar *home, *file;
    gint nbr_bytes;
    gchar *chaine;
    FILE *fd;
    struct poptOption Options[]= 
     { { "port", 'p',       POPT_ARG_INT,
         &port,             0, "Port to listen to", "PORT" },
       { "foreground", 'f', POPT_ARG_NONE,
         &fg,               0, "Run in foreground", NULL },
       { "initdb",     'i', POPT_ARG_NONE,
         &initdb,           0, "Database initialisation", NULL },
       { "initrsa",    'r', POPT_ARG_NONE,
         &initrsa,          0, "RSA initialisation", NULL },
       { "debug",'d',       POPT_ARG_INT,
         &debug_level,      0, "Debug level", "LEVEL" },
       { "max_client", 'm', POPT_ARG_INT,
         &max_client,       0, "Maximum of connexions allowed", "MAX" },
       { "home",       'H', POPT_ARG_STRING,
         &home,             0, "Home directory", "HOME" },
       { "conffile",   'c', POPT_ARG_STRING,
         &file,             0, "Configuration file", "FILE" },
       { "help",       'h', POPT_ARG_NONE,
         &help,             0, "Help", NULL },
       POPT_TABLEEND
     };
    poptContext context;
    int rc;

    home = NULL;
    file = NULL;
    port           = -1;
    max_client     = -1;
    debug_level    = -1;
    initrsa        = 0;
    initdb         = 0;    
    fg             = 0;
    help           = 0;

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
    if (port!=-1)        Config.port        = port;                    /* Priorite à la ligne de commande */
    if (debug_level!=-1) Config.debug_level = debug_level;
    if (max_client!=-1)  Config.max_client  = max_client;
    if (home)            g_snprintf( Config.home, sizeof(Config.home), "%s", home );

    if (chdir(Config.home))                                         /* Positionnement à la racine du home */
     { printf( "Chdir %s failed\n", Config.home ); exit(EXIT_ERREUR); }

    if (initdb)                                                   /* Doit-on initialiser les databases ?? */
     { gchar *chaine;
       Config.log = Info_init( "Watchdogd", Config.debug_level );                  /* Init msgs d'erreurs */
       Print_config();
       chaine = Init_db_watchdog();
       if (chaine) { printf( " Initialisation of Databases: \n %s\n", chaine );
                     g_free(chaine);
                   }
              else { printf( " Initialisation of Databases OK\n" ); }
       exit(EXIT_OK);
     }

    nbr_bytes = Config.taille_clef_rsa>>3;
    chaine = g_malloc0( nbr_bytes );
    if (!chaine)
     { printf( " Not enough memory\n" );
       exit(EXIT_OK);
     }

    if (initrsa)
     { RSA *rsa;

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
/* Exporter : Exporte les données de base Watchdog pour préparer le RELOAD                                */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Exporter ( void )
  { int fd;
    unlink ( FICHIER_EXPORT );
    fd = open( FICHIER_EXPORT, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd>0) { write (fd, Partage, sizeof(struct PARTAGE) );
                Info_c( Config.log, DEBUG_FORK, "Donnees exportées", FICHIER_EXPORT );
              }
    else      { Info_c( Config.log, DEBUG_FORK, "Could not export", FICHIER_EXPORT ); }
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
                Info_c( Config.log, DEBUG_FORK, "Donnees importées", FICHIER_EXPORT );
                close (fd);
                return(1);
              }
    return(0);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du serveur watchdog                                                          */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 int main ( int argc, char *argv[], char *envp[] )
  { struct sigaction sig;
    gchar strpid[12];
    struct DB *db;
    gint fd_lock;
    gboolean fg;

    fg = Lire_ligne_commande( argc, argv );                   /* Lecture du fichier conf et des arguments */

    fd_lock = open( VERROU_SERVEUR, O_RDWR | O_CREAT, 0640 );
    if (fd_lock<0)
     { printf( "Lock file creation failed: %s/%s\n", Config.home, VERROU_SERVEUR );
       exit(EXIT_ERREUR);
     }
    if (lockf( fd_lock, F_TLOCK, 0 )<0)                            /* Creation d'un verrou sur le fichier */
     { printf( "Cannot lock %s/%s. Probably another daemon is running\n", Config.home, VERROU_SERVEUR );
       close(fd_lock);
       exit(EXIT_ERREUR);
     }

    if (fg == FALSE)                                                   /* On tourne en tant que daemon ?? */
     { gint pid;
       pid = fork();
       if (pid<0) { printf("Fork 1 failed\n"); exit(EXIT_ERREUR); }                       /* On daemonize */
       if (pid>0) exit(EXIT_OK);                                                        /* On kill le père */
      
       setsid();                                                             /* Indépendance du processus */
      
       pid = fork();
       if (pid<0) { printf("Fork 2 failed\n"); exit(EXIT_ERREUR); }     /* Evite des pb (linuxmag 44 p78) */
       if (pid>0) exit(EXIT_OK);                                                        /* On kill le père */

       setsid();                                                             /* Indépendance du processus */
    }
    
    g_snprintf( strpid, sizeof(strpid), "%d\n", getpid() );            /* Enregistrement du pid au cas ou */
    write( fd_lock, strpid, strlen(strpid) );

#ifdef bouh
    if (fg == FALSE)
     { int i;
       for (i=getdtablesize(); i>=0; i--)
        { if (i!=fd_lock) close(i); }                                       /* Fermeture des descripteurs */
       umask(022);                                                       /* Masque de creation de fichier */
     }
#endif

    Config.log = Info_init( "Watchdogd", Config.debug_level );                     /* Init msgs d'erreurs */

    Info( Config.log, DEBUG_INFO, "Start" );
    Print_config();

    Socket_ecoute = Activer_ecoute();                             /* Initialisation de l'écoute via TCPIP */
    if ( Socket_ecoute<0 )            
     { Info( Config.log, DEBUG_INFO, "Network down, foreign connexions disabled" );
       return(EXIT_OK);
     }

    Partage = Shm_init();                                        /* Initialisation de la mémoire partagée */
    if (!Partage)
     { Info( Config.log, DEBUG_MEM, "Shared memory failed to allocate" ); }
    else
     { gint import;
       pthread_mutexattr_t attr;                                   /* Initialisation des mutex de synchro */
       gint i;
       memset( Partage, 0, sizeof(struct PARTAGE) );                             /* RAZ des bits internes */
       import = Importer();                         /* Tente d'importer les données juste après un reload */

       memset( &Partage->new_histo, 0, sizeof(Partage->new_histo) );
       memset( &Partage->del_histo, 0, sizeof(Partage->del_histo) );
       Partage->Arret            = 0;                     /* On n'arrete pas tout de suite le serveur ;-) */
       Partage->jeton            = -1;                           /* Initialisation de la mémoire partagée */
       Partage->top              = 0;
       Partage->top_cdg_plugin_dls = 0;
       
       Partage->Sous_serveur = &Partage->ss_serveur;                 /* Initialisation du pointeur global */
       for (i=0; i<Config.max_serveur; i++)
        { Partage->Sous_serveur[i].pid = -1;
          Partage->Sous_serveur[i].nb_client = -1;
          Partage->Sous_serveur[i].type_info = TYPE_INFO_VIDE;                    /* Pas d'info à traiter */
        }

       pthread_mutexattr_init( &attr );
       pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_SHARED );
       pthread_mutex_init( &Partage->com_rs485.synchro, &attr );
       pthread_mutex_init( &Partage->com_msrv_sms.synchro, &attr );
       pthread_mutex_init( &Partage->com_dls_msrv.synchro, &attr );
       pthread_mutex_init( &Partage->com_ssrv_dls.synchro, &attr );
       pthread_mutex_init( &Partage->com_arch.synchro, &attr );
       pthread_mutex_init( &Partage->com_audio.synchro, &attr );
       pthread_mutex_init( &Partage->com_admin.synchro, &attr );
             
       sig.sa_handler = Traitement_signaux;                     /* Gestionnaire de traitement des signaux */
       sig.sa_flags = SA_RESTART;     /* Voir Linux mag de novembre 2002 pour le flag anti cut read/write */
       sigaction( SIGPIPE, &sig, NULL );
       sigaction( SIGHUP,  &sig, NULL );                                         /* Reinitialisation soft */
       sigaction( SIGINT,  &sig, NULL );                                         /* Reinitialisation soft */
       sigaction( SIGALRM,  &sig, NULL );                                        /* Reinitialisation soft */
       sigaction( SIGUSR1, &sig, NULL );                               /* Reinitialisation DLS uniquement */
       sigaction( SIGUSR2, &sig, NULL );                               /* Reinitialisation DLS uniquement */
       sigaction( SIGIO, &sig, NULL );                                 /* Reinitialisation DLS uniquement */
       sigaction( SIGTERM, &sig, NULL );
encore:   

       Info( Config.log, DEBUG_INFO, "MSRV: Chargement des EANA" );
       Charger_eana();
       Info( Config.log, DEBUG_INFO, "MSRV: Chargement des EANA fait" );

       Info( Config.log, DEBUG_INFO, "MSRV: Chargement des SCENARIO" );
       Charger_scenario();
       Info( Config.log, DEBUG_INFO, "MSRV: Chargement des SCENARIO fait" );

       if (!import)
        { Info( Config.log, DEBUG_INFO, "MSRV: Clear Histo" );
          Clear_histoDB ();                            /* Clear de la table histo au boot */
          Info( Config.log, DEBUG_INFO, "MSRV: Clear Histo fait" );
        } else Info( Config.log, DEBUG_INFO, "MSRV: Import => pas de clear histo" );

       Info( Config.log, DEBUG_INFO, "MSRV: Chargement des compteurs horaires" );
       Charger_cpth();
       Info( Config.log, DEBUG_INFO, "MSRV: Chargement des compteurs horaires fait" );

       Ssl_ctx = Init_ssl();                                                        /* Initialisation SSL */
       if (!Ssl_ctx)
        { Info( Config.log, DEBUG_CRYPTO, "Init ssl failed" ); }
       else
#ifdef bouh
       if (!Demarrer_arch())                                               /* Demarrage gestion Archivage */
        { Info( Config.log, DEBUG_FORK, "MSRV: Pb ARCH -> Arret" ); }
       else
       if (!Demarrer_rs485())                                           /* Demarrage gestion module RS485 */
        { Info( Config.log, DEBUG_FORK, "MSRV: Pb RS485 -> Arret" ); }
       else
       if (!Demarrer_modbus())                                         /* Demarrage gestion module MODBUS */
        { Info( Config.log, DEBUG_FORK, "MSRV: Pb MODBUS -> Arret" ); }
       else
       if (!Demarrer_sms())                                                           /* Démarrage S.M.S. */
        { Info( Config.log, DEBUG_FORK, "MSRV: Pb SMS -> Arret" ); }
       else
       if (!Demarrer_audio())                                                      /* Démarrage A.U.D.I.O */
        { Info( Config.log, DEBUG_FORK, "MSRV: Pb AUDIO -> Arret" ); }
       else
#endif
       if (!Demarrer_admin())                                                      /* Démarrage A.U.D.I.O */
        { Info( Config.log, DEBUG_FORK, "MSRV: Pb Admin -> Arret" ); }
       else
       if (!Demarrer_dls())                                                           /* Démarrage D.L.S. */
        { Info( Config.log, DEBUG_FORK, "MSRV: Pb DLS -> Arret" ); }
       else
        { pthread_t TID;

          /*sigaction( SIGCHLD, &sig, NULL );*/
          pthread_create( &TID, NULL, (void *)Boucle_pere, NULL );
          pthread_join( TID, NULL );
          Stopper_fils();                                              /* Arret de tous les fils watchdog */
          SSL_CTX_free( Ssl_ctx );                                                  /* Libération mémoire */
        }
      }

    if (Partage->Arret == RELOAD)
     { Lire_config( NULL );                                 /* Lecture sur le fichier /etc/watchdogd.conf */
       Partage->Arret = TOURNE;
       goto encore;
     }

    pthread_mutex_destroy( &Partage->com_rs485.synchro );
    pthread_mutex_destroy( &Partage->com_msrv_sms.synchro );
    pthread_mutex_destroy( &Partage->com_dls_msrv.synchro );
    pthread_mutex_destroy( &Partage->com_ssrv_dls.synchro );
    pthread_mutex_destroy( &Partage->com_arch.synchro );
    pthread_mutex_destroy( &Partage->com_audio.synchro );
    pthread_mutex_destroy( &Partage->com_admin.synchro );
    close(fd_lock);

    if (Socket_ecoute>0) close(Socket_ecoute);
    if (Config.rsa) RSA_free( Config.rsa );

    if (Partage->Arret != CLEARREBOOT) Exporter();           /* Tente d'exporter les données avant reload */
    if (Partage->Arret == REBOOT)
     { gint pid;
       Info( Config.log, DEBUG_INFO, "Rebooting ..." );
       pid = fork();
       if (pid<0) { Info( Config.log, DEBUG_INFO, "Fork Failed on reboot" );
                    printf("Fork 1 failed\n"); exit(EXIT_ERREUR); }                       /* On daemonize */
       if (pid>0)
        { Shm_stop( Partage );                                             /* Libération mémoire partagée */
          exit(EXIT_OK);                                                               /* On kill le père */
        }
       Info_c( Config.log, DEBUG_INFO, "Rebooting in progress", argv[0] );
       sleep(5);
       execvp ( argv[0], argv );
       Info_c( Config.log, DEBUG_INFO, "Rebooting ERROR !", strerror(errno) );
       exit(EXIT_ERREUR);
     }

    Shm_stop( Partage );                                                   /* Libération mémoire partagée */
    Info( Config.log, DEBUG_INFO, "Stopped" );
    return(EXIT_OK);
  }
/*--------------------------------------------------------------------------------------------------------*/
