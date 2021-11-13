/**********************************************************************************************************/
/* Watchdogd/WatchdogdAdmin.c        Administration de Watchdog                                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 24 juil. 2011 21:18:07 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * WatchdogdAdmin.c
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
 #include <sys/socket.h>
 #include <sys/un.h>                                               /* Description de la structure AF UNIX */
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <popt.h>
 #include <string.h>
 #include <stdio.h>
 #include <readline/readline.h>
 #include <readline/history.h>
 #include <errno.h>

 #include "Reseaux.h"
 #include "config.h"

 #define PROMPT   "#Watchdogd*CLI> "
 static struct CONNEXION *Connexion= NULL;                                        /* connexion au serveur */
 static struct LOG *Log = NULL;
 static gint Log_level = 0;
 static gchar Socket_file[128];
 static gboolean Arret = FALSE;
/**********************************************************************************************************/
/* Deconnecter_admin: Ferme la socket admin                                                               */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Deconnecter_admin ( void )
  { Fermer_connexion( Connexion );
  }
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
 static gboolean Connecter_au_serveur ( void )
  { struct sockaddr_un src;                                            /* Données locales: pas le serveur */
    int connexion;

    src.sun_family = AF_UNIX;
    g_snprintf( src.sun_path, sizeof(src.sun_path), Socket_file );

    if ( (connexion = socket( AF_UNIX, SOCK_STREAM, 0)) == -1)                          /* Protocol = TCP */
     { printf("Creation socket impossible\n");
       return(FALSE);
     }

    if (connect (connexion, (struct sockaddr *)&src, sizeof(src)) == -1)
     { printf("Connexion to '%s' failed\n", Socket_file);
       close(connexion);
       return(FALSE);
     }

    Connexion = Nouvelle_connexion( NULL, connexion, -1 );                    /* Creation de la structure */
    if (!Connexion)
     { printf("Not enough memory to open connexion\n");
       return(FALSE);       
     }

    return(TRUE);
  }
/**********************************************************************************************************/
/* Lire_ligne_commande: Parse la ligne de commande pour d'eventuels parametres                            */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 static void Lire_ligne_commande( int argc, char *argv[] )
  { gint help, debug;
    gchar *file;
    struct poptOption Options[]= 
     { { "socket",   's', POPT_ARG_STRING,
         &file,        0, "Admin Socket", "FILE" },
       { "help",       'h', POPT_ARG_NONE,
         &help,             0, "Help", NULL },
       { "debug",      'd', POPT_ARG_NONE,
         &debug,            0, "Debug", NULL },
       POPT_TABLEEND
     };
    poptContext context;
    int rc;

    file = NULL;
    help = debug = 0;

    context = poptGetContext( NULL, argc, (const char **)argv, Options, POPT_CONTEXT_ARG_OPTS );
    while ( (rc = poptGetNextOpt( context )) != -1)                      /* Parse de la ligne de commande */
     { switch (rc)
        { case POPT_ERROR_BADOPT: printf( "Option %s unknown\n", poptBadOption(context, 0) );
                                  help=1; break;
          default: printf("Erreur de parsing ligne de commande\n");
        }
     }

    if (file)  g_snprintf( Socket_file, sizeof(Socket_file),  "%s", file );

    if (help)                                                             /* Affichage de l'aide en ligne */
     { poptPrintHelp(context, stdout, 0);
       poptFreeContext(context);
       _exit(0);
     }
    if (!debug) Log_level = LOG_INFO;
           else Log_level = LOG_DEBUG;
    poptFreeContext( context );                                                     /* Liberation memoire */
 }
/**********************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme                                         */
/* Entrée: numero du signal à gerer                                                                       */
/**********************************************************************************************************/
 static void Traitement_signaux( int num )                                        /* Accrochage du signal */
  { switch (num)
     { case SIGQUIT:
       case SIGINT:  
       case SIGTERM: Arret = TRUE; break;
       case SIGCHLD: printf( "Recu SIGCHLD" ); break;
       case SIGALRM: printf( "Recu SIGALRM" ); break;
       case SIGUSR1: printf( "Recu SIGUSR1" ); break;
       case SIGPIPE: printf( "Recu SIGPIPE" ); break;
       case SIGBUS:  printf( "Recu SIGBUS" );  break;
       case SIGIO:   
                     break;
       default: printf ("Recu signal %d ", num ); break;
     }
  }
/**********************************************************************************************************/
/* CB_envoyer_commande_admin: appelé par la librairie readline lorsque une ligne est prete                */
/* Entrée: la ligne a envoyer au serveur                                                                  */
/**********************************************************************************************************/
 static void CB_envoyer_commande_admin ( char *ligne )
  { struct CMD_TYPE_ADMIN admin;
    gchar commande_old[128];

    if (strlen(ligne) == 0) return;

    if ( ! strcmp( "quit", ligne ) ) Arret = TRUE;
    else
     { if ( strcmp ( ligne, commande_old ) )
        { g_snprintf( commande_old, sizeof(commande_old), "%s", ligne );
          add_history(ligne);                                        /* Ajoute la commande à l'historique */
        }

       g_snprintf( admin.buffer, sizeof(admin.buffer), "%s", ligne );                 /* Envoi au serveur */
       Envoyer_reseau( Connexion, TAG_ADMIN, SSTAG_CLIENT_REQUEST,
                       (gchar *)&admin, sizeof(struct CMD_TYPE_ADMIN) );
     }
  }
/******************************************************************************************************************************/
/* Main: Fonction principale de l'outil d'admin Watchdog                                                                      */
/* Entrée: argc, argv                                                                                                         */
/* Sortie: -1 si erreur, 0 si ok                                                                                              */
/******************************************************************************************************************************/
 int main ( int argc, char *argv[] )
  { struct timeval tv;
    fd_set fd;
    gint retour, recu;
    struct sigaction sig;

    g_snprintf( Socket_file, sizeof(Socket_file), "socket.wdg" );                                               /* Par défaut */
    Lire_ligne_commande( argc, argv );                                            /* Lecture du fichier conf et des arguments */

    if (Log_level == LOG_DEBUG) Log = Info_init( argv[0], Log_level );
    sig.sa_handler = Traitement_signaux;                                            /* Gestionnaire de traitement des signaux */
    sig.sa_flags = SA_RESTART;                            /* Voir Linux mag de novembre 2002 pour le flag anti cut read/write */
    sigaction( SIGIO,   &sig, NULL );                                                   /* Accrochage du signal a son handler */
    sigaction( SIGALRM, &sig, NULL );                                                   /* Accrochage du signal a son handler */
    sigaction( SIGUSR1, &sig, NULL );                                                   /* Accrochage du signal a son handler */
    sigaction( SIGPIPE, &sig, NULL );
    sigaction( SIGINT,  &sig, NULL );
    sigaction( SIGTERM, &sig, NULL );
    sigfillset (&sig.sa_mask);                                                    /* Par défaut tous les signaux sont bloqués */
    sigdelset ( &sig.sa_mask, SIGIO   );
    sigdelset ( &sig.sa_mask, SIGINT  );
    sigdelset ( &sig.sa_mask, SIGALRM );
    sigdelset ( &sig.sa_mask, SIGUSR1 );
    sigdelset ( &sig.sa_mask, SIGTERM );
    sigdelset ( &sig.sa_mask, SIGPIPE );
    sigprocmask(SIG_SETMASK, &sig.sa_mask, NULL);

    read_history ( NULL );                                               /* Lecture de l'historique des commandes précédentes */

    printf("  --  WatchdogdAdmin  v%s ('quit' to end session).\n", VERSION );
    if ( Connecter_au_serveur () == FALSE )
     { g_snprintf( Socket_file, sizeof(Socket_file), "%s/socket.wdg", g_get_home_dir() );                       /* Par défaut */
       if ( Connecter_au_serveur () == FALSE )
        { printf("exiting...\n"); _exit(-1); }
     }
    rl_callback_handler_install ( PROMPT, &CB_envoyer_commande_admin );
    for (Arret=FALSE;Arret!=TRUE; )
     { FD_ZERO(&fd);
       FD_SET( 0, &fd );
       tv.tv_sec=0;
       tv.tv_usec=10000;
       retour = select( 1, &fd, NULL, NULL, &tv );
       if (retour==-1)
        { gint err;
          err = errno;
          printf("Erreur select %d=(%s), shutting down\n", err, strerror(errno) );
          Arret = TRUE;
        }

       if (retour==1 && FD_ISSET(0, &fd))
        { rl_callback_read_char(); }                                                     /* Lecture du character qui est pret */

ecoute_encore:
       recu = Recevoir_reseau( Connexion );                                                /* Ecoute de ce que dit le serveur */
       if (recu==RECU_OK)
        { if ( Reseau_tag(Connexion) == TAG_INTERNAL )
           { }
          else if ( Reseau_tag(Connexion) == TAG_CONNEXION && Reseau_ss_tag(Connexion) == SSTAG_SERVEUR_OFF )
           { printf( "\n You've been disconnected\n" );
             Arret=TRUE;
             break;
           }
          else if ( Reseau_tag(Connexion) == TAG_ADMIN )                                   /* Il s'agit donc d'un TAG_ADMIN ! */
           { struct CMD_TYPE_ADMIN *admin;
             admin = (struct CMD_TYPE_ADMIN *)Connexion->donnees;
             switch ( Reseau_ss_tag (Connexion) )
              { case SSTAG_SERVEUR_RESPONSE_START:  printf("\n" ); break;
                case SSTAG_SERVEUR_RESPONSE_BUFFER: printf("%s", admin->buffer );
                                                    break;
                case SSTAG_SERVEUR_RESPONSE_STOP:   printf("%s", PROMPT ); fflush(stdout); break;
                default: printf("Wrong SSTAG\n");
              }
           }
          goto ecoute_encore;
        }
       else if (recu>=RECU_ERREUR)                                                              /* Erreur reseau->deconnexion */
        { switch( recu )
           { case RECU_ERREUR_CONNRESET: printf ( "Ecouter_admin: Reset connexion\n" );
                                         break;
             default: printf ( "Ecouter_admin: Recu erreur\n" );
                      break;
           }
          Arret=TRUE;
       }
     }
    Deconnecter_admin ();
    rl_callback_handler_remove();
    printf("\n");
    write_history ( NULL );                                             /* Ecriture de l'historique des commandes précédentes */
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
