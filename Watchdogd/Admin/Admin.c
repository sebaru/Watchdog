/**********************************************************************************************************/
/* Watchdogd/Admin/Admin.c        Gestion des connexions Admin au serveur watchdog                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 #include <sys/prctl.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <errno.h>

 #include "watchdogd.h"

 static GSList *Clients = NULL;                                     /* Leste des clients d'admin connectés */
 static gint Fd_ecoute = 0;                                          /* File descriptor de l'ecoute admin */

/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gint Activer_ecoute_admin ( void )
  { struct sockaddr_un local;
    gint opt, ecoute;

    if ( (ecoute = socket ( AF_UNIX, SOCK_STREAM, 0 )) == -1)                           /* Protocol = TCP */
     { Info_new( Config.log, FALSE, LOG_CRIT,
                 "Activer_ecoute_admin: Socket failure...:%s", strerror(errno) );
       return(-1);
     }

    opt = 1;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE,
                     (char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, FALSE, LOG_CRIT,
                 "Activer_ecoute_admin: Set Socket Option failed...:%s", strerror(errno) );
       return(-1);
     }

    opt = 16834;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_SNDBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, FALSE, LOG_CRIT,
                 "Activer_ecoute_admin: Set SEND BUF failed...:%s", strerror(errno) );
       return(-1);
     }
    if ( setsockopt( ecoute, SOL_SOCKET, SO_RCVBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, FALSE, LOG_CRIT,
                 "Activer_ecoute_admin: Set RCV BUF failed...:%s", strerror(errno) );
       return(-1);
     }

    memset( &local, 0, sizeof(local) );
    unlink(NOM_SOCKET);
    local.sun_family = AF_UNIX;
    g_snprintf( local.sun_path, sizeof(local.sun_path), NOM_SOCKET );
    if (bind( ecoute, (struct sockaddr *)&local, sizeof(local)) == -1)
     { Info_new( Config.log, FALSE, LOG_CRIT,
                 "Activer_ecoute_admin: Bind Failure...:%s", strerror(errno) );
       close(ecoute);
       return(-1);
     }

    if (listen(ecoute, 1) == -1)                                       /* On demande d'écouter aux portes */
     { Info_new( Config.log, FALSE, LOG_CRIT,
                 "Activer_ecoute_admin: Listen failure...:%s", strerror(errno) );
       close(ecoute);
       return(-1);
     }
    fcntl( ecoute, F_SETFL, O_NONBLOCK );                                            /* Mode non bloquant */
    return( ecoute );
  }
/**********************************************************************************************************/
/* Desactiver_ecoute_admin: Ferme la socker fifo d'administration                                         */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Desactiver_ecoute_admin ( void )
  { close (Fd_ecoute);
    Fd_ecoute = 0;
    Info_new( Config.log, FALSE, LOG_INFO, "Desactiver_ecoute_admin: socket disabled" );
  }
/**********************************************************************************************************/
/* Deconnecter_admin: Ferme la socket admin en parametre                                                  */
/* Entrée: le client                                                                                      */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Deconnecter_admin ( struct CLIENT_ADMIN *client )
  { Write_admin( client->connexion, "\n - Disconnected - \n" );
    close ( client->connexion );
    Info_new( Config.log, FALSE, LOG_INFO,
              "Deconnecter_admin : connection closed with client %d", client->connexion );
    Clients = g_slist_remove ( Clients, client );
    g_free(client);
  }
/**********************************************************************************************************/
/* Accueillir_nouveaux_clients: Cette fonction permet de loguer d'éventuels nouveaux clients distants     */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE si un nouveau client est arrivé                                                           */
/**********************************************************************************************************/
 static gboolean Accueillir_un_admin( gint ecoute )
  { struct CLIENT_ADMIN *client;
    struct sockaddr_un distant;
    guint taille_distant, id;
 
    taille_distant = sizeof(distant);
    if ( (id=accept( ecoute, (struct sockaddr *)&distant, &taille_distant )) != -1)         /* demande ?? */
     { Info_new( Config.log, FALSE, LOG_INFO,
                 "Accueillir_un_admin: Connexion wanted. ID=%d", id );

       client = g_malloc0( sizeof(struct CLIENT_ADMIN) );/* On alloue donc une nouvelle structure cliente */
       if (!client) { Info_new( Config.log, FALSE, LOG_ERR,
                                "Accueillir_un_admin: Not enought memory to connect client %d", id );
                      close(id);
                      return(FALSE);                                    /* On traite bien sûr les erreurs */
                    }

       client->connexion = id;
       client->last_use = Partage->top;
       fcntl( client->connexion, F_SETFL, O_NONBLOCK );                              /* Mode non bloquant */

       Clients = g_slist_prepend( Clients, client );
       Info_new( Config.log, FALSE, LOG_INFO,
                 "Accueillir_un_admin: Connexion granted to ID=%d", id );
       return(TRUE);
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Write_admin: Envoi la réponse au client                                                                */
/* Entrée: la chaine de caractère                                                                         */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Write_admin ( gint fd, gchar *chaine )
  { write ( fd, chaine, strlen(chaine) ); }
/**********************************************************************************************************/
/* Ecouter_admin: Ecoute ce que dis le client                                                             */
/* Entrée: le client                                                                                      */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Ecouter_admin ( struct CLIENT_ADMIN *client )
  { gchar ligne[128], commande[128], chaine[128];
    struct LIBRAIRIE *lib;
    GSList *liste;
    gint taille;

    memset( ligne, 0, sizeof(ligne) );
    taille = read( client->connexion, ligne, sizeof(ligne) );

    if (taille > 0)
     { ligne[taille] = 0;

       client->last_use = Partage->top;
       Info_new( Config.log, FALSE, LOG_NOTICE,
                 "Ecuter_admin : Command received = %s\n", ligne );
       sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */

            if ( ! strcmp ( commande, "modbus"    ) ) { Admin_modbus   ( client, ligne + 7 ); }
       else if ( ! strcmp ( commande, "process"   ) ) { Admin_process  ( client, ligne + 8 ); }
       else if ( ! strcmp ( commande, "dls"       ) ) { Admin_dls      ( client, ligne + 4 ); }
       else if ( ! strcmp ( commande, "onduleur"  ) ) { Admin_onduleur ( client, ligne + 9 ); }
       else if ( ! strcmp ( commande, "tellstick" ) ) { Admin_tellstick( client, ligne + 10); }
       else if ( ! strcmp ( commande, "set"       ) ) { Admin_set      ( client, ligne + 4);  }
       else if ( ! strcmp ( commande, "get"       ) ) { Admin_get      ( client, ligne + 4);  }
       else if ( ! strcmp ( commande, "sms"       ) ) { Admin_sms      ( client, ligne + 4);  }
       else                                           { Admin_running  ( client, ligne ); }

       liste = Partage->com_msrv.Librairies;                           /* Parcours de toutes les librairies */
       while(liste)
        { lib = (struct LIBRAIRIE *)liste->data;
          if ( ! strcmp( commande, lib->admin_prompt ) )
           { lib->Admin_command ( client, ligne + strlen(lib->admin_prompt)+1 ); }
          liste = liste->next;
        }

       g_snprintf( chaine, sizeof(chaine), "\n" );
       Write_admin ( client->connexion, chaine );
     }
  }
/**********************************************************************************************************/
/* Run_admin: Ecoute les commandes d'admin locale et les traite                                           */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Run_admin ( void )
  { prctl(PR_SET_NAME, "W-Admin", 0, 0, 0 );

    Info_new( Config.log, FALSE, LOG_NOTICE,
              "Run_admin: Demarrage . . . TID = %d", pthread_self() );

    Fd_ecoute = Activer_ecoute_admin ();
    if ( Fd_ecoute < 0 )
     { Info_new( Config.log, FALSE, LOG_CRIT,
              "Run_admin: Unable to open Socket -> Stop" );
       Partage->com_admin.TID = 0;                        /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     } else Info_new( Config.log, FALSE, LOG_NOTICE,
                      "Run_admin: Socket is enabled, waiting for clients" );

    Clients = NULL;                                             /* Initialisation des variables du thread */
    Partage->com_admin.Thread_run = TRUE;                                           /* Le thread tourne ! */
    while(Partage->com_admin.Thread_run == TRUE)                         /* On tourne tant que necessaire */
     {

       if (Partage->com_admin.Thread_sigusr1)                                     /* On a recu sigusr1 ?? */
        { Info_new( Config.log, FALSE, LOG_NOTICE, "Run_admin: recu SIGUSR1" );
          Partage->com_admin.Thread_sigusr1 = FALSE;
        }

       Accueillir_un_admin( Fd_ecoute );                  /* Accueille les nouveaux admin */

       if ( Clients )                                          /* Ecoutons nos clients */
        { struct CLIENT_ADMIN *client;
          GSList *liste;

          liste = Clients;
          while (liste)
           { client = (struct CLIENT_ADMIN *)liste->data;

             if ( Partage->top > client->last_use + 3000 )    /* Deconnexion = 300 secondes si inactivité */
              { Write_admin( client->connexion, "timeout\n" );
                Deconnecter_admin ( client ); 
                liste = Clients;
                continue;
              }

             Ecouter_admin( client );
             liste = liste->next;
           }
        }
       sched_yield();
       usleep(10000);
     }

    while(Clients)                                                    /* Parcours de la liste des clients */
     { struct CLIENT_ADMIN *client;                                   /* Deconnection de tous les clients */
       client = (struct CLIENT_ADMIN *)Clients->data;
       Deconnecter_admin ( client );
     }

    Desactiver_ecoute_admin ();
    Info_new( Config.log, FALSE, LOG_NOTICE,
              "Run_admin: Down . . . TID = %d", pthread_self() );
    Info_n( Config.log, DEBUG_ADMIN, "Admin: Run_admin: Down", pthread_self() );
    Partage->com_admin.TID = 0;                           /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
