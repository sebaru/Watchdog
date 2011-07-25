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

 #include "config.h"

 static gint Socket, wait_reponse;                                             /* Socket d'administration */
 static gchar Socket_file[128];

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
     { printf("Connexion impossible\n");
       close(connexion);
       return(FALSE);
     }
  fcntl( connexion, F_SETFL, O_ASYNC | O_NONBLOCK );/* Mode non bloquant, ça aide pour une telle application */
  fcntl( connexion, F_SETOWN, getpid() );     /* Mode non bloquant, ça aide pour une telle application */

    Socket = connexion;
    return(TRUE);
  }
/**********************************************************************************************************/
/* Lire_ligne_commande: Parse la ligne de commande pour d'eventuels parametres                            */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 static void Lire_ligne_commande( int argc, char *argv[] )
  { gint help;
    gchar *file;
    struct poptOption Options[]= 
     { { "socket",   's', POPT_ARG_STRING,
         &file,        0, "Admin Socket", "FILE" },
       { "help",       'h', POPT_ARG_NONE,
         &help,             0, "Help", NULL },
       POPT_TABLEEND
     };
    poptContext context;
    int rc;

    file = NULL;
    help           = 0;

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
    poptFreeContext( context );                                                     /* Liberation memoire */
 }
/**********************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme                                         */
/* Entrée: numero du signal à gerer                                                                       */
/**********************************************************************************************************/
 static void Traitement_signaux( int num )
  { static gint nbr_slash_n = 0;
    gchar reponse[2];
    gint taille;
    

    switch (num)
     { case SIGQUIT:
       case SIGINT:  printf( "Recu SIGINT" );  break;
       case SIGTERM: printf( "Recu SIGTERM" ); break;
       case SIGCHLD: printf( "Recu SIGCHLD" ); break;
       case SIGPIPE: printf( "Recu SIGPIPE" ); break;
       case SIGBUS:  printf( "Recu SIGBUS" );  break;
       case SIGIO:   while ( (taille = read( Socket, reponse, 1 )) > 0 )
                      { reponse[taille] = 0;
                        printf("%s", reponse );
                        if (reponse[0] == '\n')
                         { if (nbr_slash_n == 1) { wait_reponse = FALSE; nbr_slash_n = 0; }
                           else nbr_slash_n++;
                         }
                      }
                     fflush(stdout);
                     break;
       default: printf ("Recu signal %d ", num ); break;
     }
  }
/**********************************************************************************************************/
/* Main: Fonction principale de l'outil d'admin Watchdog                                                  */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 int main ( int argc, char *argv[] )
  { struct sigaction sig;
    gchar *commande;

    g_snprintf( Socket_file, sizeof(Socket_file), "%s/socket.wdg", g_get_home_dir() );      /* Par défaut */
    Lire_ligne_commande( argc, argv );                        /* Lecture du fichier conf et des arguments */

    sig.sa_handler = Traitement_signaux;                        /* Gestionnaire de traitement des signaux */
    sig.sa_flags = SA_RESTART;        /* Voir Linux mag de novembre 2002 pour le flag anti cut read/write */
    sigaction( SIGPIPE, &sig, NULL );
    sigaction( SIGIO, &sig, NULL );                                 /* Accrochage du signal a son handler */

    read_history ( NULL );                           /* Lecture de l'historique des commandes précédentes */

    printf("  --  WatchdogdAdmin  v%s \n", VERSION );
    if ( Connecter_au_serveur () == FALSE ) _exit(-1); 

    write ( Socket, "ident", 6 );             /* Demande l'envoi de la chaine d'identification du serveur */
    wait_reponse = TRUE;                               /* Précisons que l'on attend la réponse du serveur */

    for ( ; ; )
     { while (wait_reponse != FALSE);
       commande = readline ("#Watchdogd*CLI> ");
       if (!commande)
        { write ( Socket, "nocde", strlen("nocde")+1 );
          fsync(Socket);                                                             /* Flush la sortie ! */
          continue;
        }

       if ( strlen(commande) )
        { add_history(commande);

          if ( ! strcmp ( commande, "quit" ) ) break;                                    /* On s'arrete ? */
          else
           { write ( Socket, commande, strlen(commande) );
             fsync(Socket);                                                          /* Flush la sortie ! */
             wait_reponse = TRUE;                      /* Précisons que l'on attend la réponse du serveur */
           }
        }
       g_free (commande);
     }

    close( Socket );
    write_history ( NULL );                         /* Ecriture de l'historique des commandes précédentes */
    return(0);
  }
/*--------------------------------------------------------------------------------------------------------*/
