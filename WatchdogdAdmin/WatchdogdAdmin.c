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
 static struct CONNEXION *Connexion;                                              /* connexion au serveur */
 static gchar Socket_file[128];
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
     { printf("Connexion impossible\n");
       close(connexion);
       return(FALSE);
     }

    Connexion = Nouvelle_connexion( NULL, connexion, -1 );                    /* Creation de la structure */
    if (!Connexion)
     { printf("Not enough memory to open connexion\n");
       return(FALSE);       
     }

    fcntl( Connexion->socket, F_SETOWN, getpid() );      /* Active la reception du signal SIGIO sur ce FD */
    fcntl( Connexion->socket, F_SETFL, O_ASYNC );/* Mode non bloquant, ça aide pour une telle application */

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
 static void Traitement_signaux( int num )                                        /* Accrochage du signal */
  { gint recu;
    switch (num)
     { case SIGQUIT:
       case SIGINT:  write_history ( NULL );        /* Ecriture de l'historique des commandes précédentes */
                     break;
       case SIGTERM: printf( "Recu SIGTERM" ); break;
       case SIGCHLD: printf( "Recu SIGCHLD" ); break;
       case SIGALRM: printf( "Recu SIGALRM" ); break;
       case SIGUSR1: printf( "Recu SIGUSR1" ); break;
       case SIGPIPE: printf( "Recu SIGPIPE" ); break;
       case SIGBUS:  printf( "Recu SIGBUS" );  break;
       case SIGIO:   do { recu = Recevoir_reseau( Connexion );
                        }
                     while ( recu == RECU_EN_COURS );
                     if (recu==RECU_OK)
                      { if ( Reseau_tag(Connexion) == TAG_INTERNAL )
                         { break; }
                        else if ( Reseau_tag(Connexion) != TAG_ADMIN )
                         { printf( "Ecouter_admin: Wrong TAG\n" ); break; }
                        else                                           /* Il s'agit donc d'un TAG_ADMIN ! */
                         { struct CMD_TYPE_ADMIN *admin;
                           admin = (struct CMD_TYPE_ADMIN *)Connexion->donnees;
                           switch ( Reseau_ss_tag (Connexion) )
                            { case SSTAG_SERVEUR_RESPONSE_START:  printf("\n" ); break;
                              case SSTAG_SERVEUR_RESPONSE_BUFFER: printf("%s", admin->buffer ); break;
                              case SSTAG_SERVEUR_RESPONSE_STOP:   printf( PROMPT ); break;
                              default: printf("Wrong SSTAG\n");
                            }
                         }
                      }
                     else if (recu>=RECU_ERREUR)                                             /* Erreur reseau->deconnexion */
                      { switch( recu )
                         { case RECU_ERREUR_CONNRESET: printf ( "Ecouter_admin: Reset connexion\n" );
                                                       break;
                           default: printf ( "Ecouter_admin: Recu erreur\n" );
                                    break;
                         }
                        Deconnecter_admin ();
                      }
                     fflush(stdout);
                     break;
       default: printf ("Recu signal %d ", num ); break;
     }
  }

 static void CB_envoyer_commande_admin ( char *ligne )
  { struct CMD_TYPE_ADMIN admin;
    g_snprintf( admin.buffer, sizeof(admin.buffer), "%s", ligne );
    Envoyer_reseau( Connexion, TAG_ADMIN, SSTAG_CLIENT_REQUEST,
                    (gchar *)&admin, sizeof(struct CMD_TYPE_ADMIN) );
  }
/**********************************************************************************************************/
/* Main: Fonction principale de l'outil d'admin Watchdog                                                  */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 int main ( int argc, char *argv[] )
  { struct timeval tv;
    fd_set fd;
    gint taille, taille_old, retour;
    struct sigaction sig;
    gchar *commande, commande_old[128];
    gboolean Arret;

    g_snprintf( Socket_file, sizeof(Socket_file), "%s/socket.wdg", g_get_home_dir() );      /* Par défaut */
    Lire_ligne_commande( argc, argv );                        /* Lecture du fichier conf et des arguments */

    sig.sa_handler = Traitement_signaux;                        /* Gestionnaire de traitement des signaux */
    sig.sa_flags = SA_RESTART;        /* Voir Linux mag de novembre 2002 pour le flag anti cut read/write */
    sigaction( SIGIO,   &sig, NULL );                               /* Accrochage du signal a son handler */
    sigaction( SIGALRM, &sig, NULL );                               /* Accrochage du signal a son handler */
    sigaction( SIGUSR1, &sig, NULL );                               /* Accrochage du signal a son handler */
    sigaction( SIGPIPE, &sig, NULL );
    sigaction( SIGINT,  &sig, NULL );
    sigaction( SIGTERM, &sig, NULL );
    sigfillset (&sig.sa_mask);                             /* Par défaut tous les signaux sont bloqués */
    sigdelset ( &sig.sa_mask, SIGIO   );
    sigdelset ( &sig.sa_mask, SIGINT  );
    sigdelset ( &sig.sa_mask, SIGALRM );
    sigdelset ( &sig.sa_mask, SIGUSR1 );
    sigdelset ( &sig.sa_mask, SIGTERM );
    sigdelset ( &sig.sa_mask, SIGPIPE );
    sigprocmask(SIG_SETMASK, &sig.sa_mask, NULL);

    g_snprintf( commande_old, sizeof(commande_old), "nocde" );
    taille_old = 5;
    read_history ( NULL );                           /* Lecture de l'historique des commandes précédentes */

    printf("  --  WatchdogdAdmin  v%s ('quit' to end session)\n", VERSION );
    if ( Connecter_au_serveur () == FALSE ) _exit(-1); 
    rl_callback_handler_install ( PROMPT, &CB_envoyer_commande_admin );
    for (Arret=FALSE;Arret==FALSE; )
     { FD_ZERO(&fd);
       FD_SET( 0, &fd );
       tv.tv_sec=1;
       tv.tv_usec=0;
       retour = select( 1, NULL, &fd, NULL, &tv );
       if (retour==-1)
        { gint err;
          err = errno;
          printf("Erreur select %d=(%s), shuting down\n", err, strerror(errno) );
          Arret = TRUE;
        }
       if (retour==0)                            /* Traiter ensuite les signaux du genre relais brisé */
        { printf("Recu nu char\n");
          rl_callback_read_char();
        }

#ifdef bouh
       commande = readline ( PROMPT );
       taille = strlen(commande);
       if ( taille )
        { if (strncmp ( commande, commande_old, (taille < taille_old ? taille : taille_old)))
           { g_snprintf( commande_old, sizeof(commande_old), "%s", commande );
             taille_old = taille;                         /* On n'ajoute pas de doublon dans l'historique */
             add_history(commande);                                  /* Ajoute la commande à l'historique */
           }

          if ( ! strncmp ( commande, "quit", taille ) ) break;                           /* On s'arrete ? */
          else
           { struct CMD_TYPE_ADMIN admin;
             g_snprintf( admin.buffer, sizeof(admin.buffer), "%s", commande );
             Envoyer_reseau( Connexion, TAG_ADMIN, SSTAG_CLIENT_REQUEST,
                             (gchar *)&admin, sizeof(struct CMD_TYPE_ADMIN) );
           }
        }
       g_free (commande);
#endif
     }
    rl_callback_handler_remove();
    Fermer_connexion ( Connexion );
    write_history ( NULL );                         /* Ecriture de l'historique des commandes précédentes */
    return(0);
  }
/*--------------------------------------------------------------------------------------------------------*/
