/******************************************************************************************************************************/
/* Watchdogd/WatchdogdAdmin.c        Administration de Watchdog                                                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       dim. 24 juil. 2011 21:18:07 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
#ifdef bouh
 #include <sys/socket.h>
 #include <sys/un.h>                                               /* Description de la structure AF UNIX */
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
#endif
 #include <stdlib.h>
 #include <popt.h>
 #include <string.h>
 #include <stdio.h>
 #include <readline/readline.h>
 #include <readline/history.h>
 #include <errno.h>
 #include <zmq.h>

 #include "config.h"

 #define PROMPT   "#Watchdogd*CLI> "
 static void *socket;                                                                                 /* connexion au serveur */
 static void *zmq_ctx;
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
  { zmq_close( socket );
    zmq_ctx_term( zmq_ctx );
    zmq_ctx_destroy( zmq_ctx );
  }
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
 static gboolean Connecter_au_serveur ( gchar *file )
  { gchar endpoint[128];

    socket = zmq_socket ( zmq_ctx, ZMQ_REQ );
    g_snprintf( endpoint, sizeof(endpoint), "ipc://%s", file );
    if ( zmq_connect ( socket, endpoint ) == -1 )
     { printf("Creation socket impossible\n");
       return(FALSE);
     }
    printf("Connexion to '%s' OK\n", endpoint);

    return(TRUE);
  }
/**********************************************************************************************************/
/* Lire_ligne_commande: Parse la ligne de commande pour d'eventuels parametres                            */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 static void Lire_ligne_commande( int argc, char *argv[] )
  { gchar *file;
    gint help;
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
    help = 0;

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
       exit(0);
     }
    poptFreeContext( context );                                                     /* Liberation memoire */
 }
/******************************************************************************************************************************/
/* CB_envoyer_commande_admin: appelé par la librairie readline lorsque une ligne est prete                                    */
/* Entrée: la ligne a envoyer au serveur                                                                                      */
/******************************************************************************************************************************/
 static void CB_envoyer_commande_admin ( char *ligne )
  { gchar commande_old[128];
    gchar buffer[10240];
    gint recu;

    if (strlen(ligne) == 0) return;

    if ( ! strcmp( "quit", ligne ) ) Arret = TRUE;
    else if ( ! strcmp( "exit", ligne ) ) Arret = TRUE;
    else
     { if ( strcmp ( ligne, commande_old ) )
        { g_snprintf( commande_old, sizeof(commande_old), "%s", ligne );
          add_history(ligne);                                                            /* Ajoute la commande à l'historique */
        }

       zmq_send( socket, ligne, strlen(ligne)+1, 0 );
       recu = zmq_recv ( socket, &buffer, sizeof(buffer), 0 );                             /* Ecoute de ce que dit le serveur */
       if (recu>0)
        { printf("%s", buffer );
          fflush(stdout);
        }
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
    gint retour;

    g_snprintf( Socket_file, sizeof(Socket_file), "%s/socket.wdg", g_get_home_dir() );                          /* Par défaut */
    Lire_ligne_commande( argc, argv );                                            /* Lecture du fichier conf et des arguments */

    read_history ( NULL );                                               /* Lecture de l'historique des commandes précédentes */

    zmq_ctx = zmq_ctx_new ();                                                      /* Initialisation du context d'echange ZMQ */
    if (!zmq_ctx)
     { printf( "%s: Init ZMQ Context Failed (%s)", __func__, zmq_strerror(errno) ); exit(-1); }

    printf("  --  WatchdogdAdmin  v%s ('quit' to end session).\n", VERSION );
    if ( Connecter_au_serveur ( Socket_file ) == FALSE )
     { printf("exiting...\n"); exit(-1); }

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
     }
    Deconnecter_admin ();
    rl_callback_handler_remove();
    printf("\n");
    write_history ( NULL );                                             /* Ecriture de l'historique des commandes précédentes */
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
