/**********************************************************************************************************/
/* Watchdogd/WatchdogdAdmin.c        Administration de Watchdog                                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 07 sep 2008 12:16:53 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * WatchdogdAdmin.c
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <unistd.h>
 #include <popt.h>
 #include <string.h>
 #include <stdio.h>

 #define PROMPT      "#> "

 static gint Socket_read;                                                      /* Socket d'administration */
 static gint Socket_write;                                                     /* Socket d'administration */
 static gchar Fifo_file_read[128], Fifo_file_write[128];

/**********************************************************************************************************/
/* Lire_ligne_commande: Parse la ligne de commande pour d'eventuels parametres                            */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 static void Lire_ligne_commande( int argc, char *argv[] )
  { gint help;
    gchar *file, *file2;
    struct poptOption Options[]= 
     { { "readfile",   'r', POPT_ARG_STRING,
         &file,             0, "Admin FIFO READ", "FILE" },
       { "writefile",  'w', POPT_ARG_STRING,
         &file2,            0, "Admin FIFO WRITE", "FILE" },
       { "help",       'h', POPT_ARG_NONE,
         &help,             0, "Help", NULL },
       POPT_TABLEEND
     };
    poptContext context;
    int rc;

    file = file2 = NULL;
    help           = 0;

    context = poptGetContext( NULL, argc, (const char **)argv, Options, POPT_CONTEXT_ARG_OPTS );
    while ( (rc = poptGetNextOpt( context )) != -1)                      /* Parse de la ligne de commande */
     { switch (rc)
        { case POPT_ERROR_BADOPT: printf( "Option %s unknown\n", poptBadOption(context, 0) );
                                  help=1; break;
          default: printf("Erreur de parsing ligne de commande\n");
        }
     }

    if (file)  g_snprintf( Fifo_file_read,  sizeof(Fifo_file_read),  "%s", file );
    if (file2) g_snprintf( Fifo_file_write, sizeof(Fifo_file_write), "%s", file2 );

    if (help)                                                             /* Affichage de l'aide en ligne */
     { poptPrintHelp(context, stdout, 0);
       poptFreeContext(context);
       _exit(0);
     }
    poptFreeContext( context );                                                     /* Liberation memoire */
 }
/**********************************************************************************************************/
/* Main: Fonction principale du serveur watchdog                                                          */
/* Entrée: argc, argv                                                                                     */
/* Sortie: -1 si erreur, 0 si ok                                                                          */
/**********************************************************************************************************/
 int main ( int argc, char *argv[] )
  { gchar reponse[128], commande[128], commande_hold[128];
    gint retval, taille;
    struct timeval tv;
    fd_set fdselect;

    Lire_ligne_commande( argc, argv );                        /* Lecture du fichier conf et des arguments */

    Socket_read = open( Fifo_file_read, O_RDWR );
    if ( Socket_read < 0 )
     { perror("Failed : ");
       printf("WatchdogdAdmin: impossible d'ouvrir le FIFO READ\n");
       return(-1);
     }
    fcntl( Socket_read, F_SETFL, O_NONBLOCK );   /* Mode non bloquant, ça aide pour une telle application */

    Socket_write = open( Fifo_file_write, O_WRONLY );
    if ( Socket_write < 0 )
     { perror("Failed : ");
       printf("WatchdogdAdmin: impossible d'ouvrir le FIFO READ\n");
       close( Socket_read );
       return(-1);
     }

    printf(" WatchdogdAdmin  v1.0 \n");
    write ( Socket_write, "ident", 6 );
    g_snprintf( commande_hold, sizeof(commande_hold), "nocde");

    for ( ; ; )
     { FD_ZERO(&fdselect);
       FD_SET(0, &fdselect);
       FD_SET(Socket_read, &fdselect );
       tv.tv_sec = 0;
       tv.tv_usec= 100;
       retval = select(Socket_read+1, &fdselect, NULL, NULL, &tv );            /* Attente d'un caractere */

       if (retval>=0)
	{ if (FD_ISSET(0, &fdselect))                                             /* Est-ce au clavier ?? */
           { taille = read( 0, commande, sizeof(commande) );
             taille--;                                                       /* -1 car caractère "entrée" */
             commande[taille] = 0;                                           /* Caractère de fin de ligne */

             if ( ! strcmp ( commande, "quit" ) ) break;
             else { if (taille) memcpy( commande_hold, commande, sizeof(commande_hold) );
                           else memcpy( commande, commande_hold, sizeof(commande) );
                    write ( Socket_write, commande, strlen(commande) );
                  }
	   }
	  else if (FD_ISSET(Socket_read, &fdselect))                 /* Est-ce sur la ligne d'admin local */
           { taille = read( Socket_read, reponse, sizeof(reponse) );
             reponse[taille] = 0;
             printf("%s", reponse ); fflush(stdout);
           }
	}
     }

    close( Socket_read );
    close (Socket_write);
    return(0);
  }
/*--------------------------------------------------------------------------------------------------------*/
