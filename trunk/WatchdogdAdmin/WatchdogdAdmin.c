/******************************************************************************************************************************/
/* Watchdogd/WatchdogdAdmin.c        Administration de Watchdog                                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       dim. 24 juil. 2011 21:18:07 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * WatchdogdAdmin.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien LEFEVRE
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
 static void *to_master, *from_master;
 static struct LOG *Log = NULL;
 static gint Log_level = 0;
 static gchar Socket_file[128];
 static gboolean Connect_like_slave;
 static gboolean Arret = FALSE;

 struct ZMQ_TARGET
  { gint8 tag;
    gchar instance[12];
    gchar thread[12];
  } target;

/******************************************************************************************************************************/
/* Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                        */
/* Entrée: le nom de fichier à lire                                                                                           */
/* Sortie: La structure mémoire est à jour                                                                                    */
/******************************************************************************************************************************/
 void Lire_config ( char *fichier_config )
  { gchar *chaine, *fichier;
    GError *error = NULL;
	   gint num;
    GKeyFile *gkf;

    if (!fichier_config) fichier = "/etc/watchdogd.conf";                              /* Lecture du fichier de configuration */
                    else fichier = fichier_config;
    printf("Using config file %s\n", fichier );
    gkf = g_key_file_new();

    if (g_key_file_load_from_file(gkf, fichier, G_KEY_FILE_NONE, &error))
     {
/******************************************************* Partie GLOBAL ********************************************************/
       chaine = g_key_file_get_string ( gkf, "GLOBAL", "home", NULL );
       if (chaine)
        { g_snprintf( Socket_file, sizeof(Socket_file), "%s/socket.wdg", chaine ); g_free(chaine); }
     }
    else 
     { printf("Unable to parse config file %s, error %s\n", fichier, error->message );
       g_error_free( error );
     }
    g_key_file_free(gkf);
  }
/******************************************************************************************************************************/
/* CB_envoyer_commande_admin: appelé par la librairie readline lorsque une ligne est prete                                    */
/* Entrée: la ligne a envoyer au serveur                                                                                      */
/******************************************************************************************************************************/
 static void CB_send_to_master ( char *ligne )
  { gchar commande_old[128];
    struct ZMQ_TARGET target;
    gchar buffer[256];
    gint recu;

    if (strlen(ligne) == 0) return;

    if ( ! strcmp( "quit", ligne ) ) Arret = TRUE;
    else if ( ! strcmp( "exit", ligne ) ) Arret = TRUE;
    else
     { if ( strcmp ( ligne, commande_old ) )
        { g_snprintf( commande_old, sizeof(commande_old), "%s", ligne );
          add_history(ligne);                                                            /* Ajoute la commande à l'historique */
        }
       memcpy(buffer, &target, sizeof(target));
       g_snprintf( buffer+sizeof(target), sizeof(buffer) - sizeof(target),"%s", ligne );
       zmq_send( to_master, buffer, sizeof(target)+strlen(ligne)+1, 0 );
     }
  }
/******************************************************************************************************************************/
/* New_main: Se connecte au serveur via le ZMQ 'classique' entre un slave et son master                                       */
/* Entrée: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void New_main ( void )
  { gchar endpoint[80], buffer[256];
    struct timeval tv;
    fd_set fd;
    gint retour;
    gint recu;

    zmq_ctx = zmq_ctx_new ();                                                      /* Initialisation du context d'echange ZMQ */
    if (!zmq_ctx)
     { printf( "%s: Init ZMQ Context Failed (%s)", __func__, zmq_strerror(errno) ); return; }

    printf("  --  WatchdogdAdmin  v%s ('quit' to end session) - Slave Mode. Using host '%s'.\n", VERSION, Socket_file );

    if ( (to_master = zmq_socket ( zmq_ctx, ZMQ_PUB )) == NULL)
     { printf( "%s: New ZMQ Socket to_master Failed (%s)", __func__, zmq_strerror(errno) ); return; }

    g_snprintf( endpoint, sizeof(endpoint), "tcp://%s:%d", Socket_file, 5556 );
    if ( zmq_connect (to_master, endpoint) == -1 ) 
     { printf( "%s: ZMQ Connect to '%s' Failed (%s)", __func__, endpoint, zmq_strerror(errno) ); return; }

    if ( (from_master = zmq_socket ( zmq_ctx, ZMQ_SUB )) == NULL)
     { printf( "%s: New ZMQ Socket from_master Failed (%s)", __func__, zmq_strerror(errno) ); return; }

    g_snprintf( endpoint, sizeof(endpoint), "tcp://%s:%d", Socket_file, 5555 );
    if ( zmq_connect (from_master, endpoint) == -1 ) 
     { printf( "%s: ZMQ Connect to '%s' Failed (%s)", __func__, endpoint, zmq_strerror(errno) ); return; }

    if ( zmq_setsockopt ( from_master, ZMQ_SUBSCRIBE, NULL, 0 ) == -1 )                          /* Subscribe to all messages */
     { printf( "%s: ZMQ subscript to all for '%s' failed (%s)", __func__, endpoint, zmq_strerror(errno) );
       return;
     }

    target.tag = 4;
    g_snprintf(target.instance, sizeof(target.instance), "%s", Socket_file );
    g_snprintf(target.thread,   sizeof(target.thread),   "process" );

    rl_callback_handler_install ( PROMPT, &CB_send_to_master );

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
       recu = zmq_recv ( from_master, &buffer, sizeof(buffer),  ZMQ_DONTWAIT );            /* Ecoute de ce que dit le serveur */
       if (recu>0)
        { printf("%s", buffer );
          fflush(stdout);
        }
     }

    rl_callback_handler_remove();
    printf("\n");
  }
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
    gint help, new;
    struct poptOption Options[]= 
     { { "socket",     's', POPT_ARG_STRING,
         &file,        0, "Admin Socket or Master Hostname", "FILE" },
       { "help",       'h', POPT_ARG_NONE,
         &help,             0, "Help", NULL },
       { "new",        'n', POPT_ARG_NONE,
         &new,              0, "Connect like all slaves", NULL },
       POPT_TABLEEND
     };
    poptContext context;
    int rc;

    file = NULL;
    new = help = 0;

    context = poptGetContext( NULL, argc, (const char **)argv, Options, POPT_CONTEXT_ARG_OPTS );
    while ( (rc = poptGetNextOpt( context )) != -1)                      /* Parse de la ligne de commande */
     { switch (rc)
        { case POPT_ERROR_BADOPT: printf( "Option %s unknown\n", poptBadOption(context, 0) );
                                  help=1; break;
          default: printf("Erreur de parsing ligne de commande\n");
        }
     }

    if (file)  g_snprintf( Socket_file, sizeof(Socket_file),  "%s", file );
    if (new) Connect_like_slave = TRUE;
        else Connect_like_slave = FALSE;

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

    Lire_config(NULL);                                                                                          /* Par défaut */
    Lire_ligne_commande( argc, argv );                                            /* Lecture du fichier conf et des arguments */

    read_history ( NULL );                                               /* Lecture de l'historique des commandes précédentes */

    if (Connect_like_slave)
     { New_main();
       goto end;
     }

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

end:
    write_history ( NULL );                                             /* Ecriture de l'historique des commandes précédentes */
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
