/**********************************************************************************************************/
/* Watchdogd/Admin/Admin.c        Gestion des connexions Admin au serveur watchdog                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin.c
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
 #include <sys/socket.h>
 #include <sys/un.h>                                               /* Description de la structure AF UNIX */
 #include <sys/types.h>
 #include <sys/prctl.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <errno.h>

 #include "watchdogd.h"

 gchar *Mode_admin[NBR_MODE_ADMIN] =
  { "running", "modbus", "process" };
 
 extern struct CONFIG Config;
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gint Activer_ecoute_admin ( void )
  { struct sockaddr_un local;
    gint opt, ecoute;

    if ( (ecoute = socket ( AF_UNIX, SOCK_STREAM, 0 )) == -1)                           /* Protocol = TCP */
     { Info_c( Config.log, DEBUG_ADMIN, "Socket failure...", strerror(errno) ); return(-1); }

    opt = 1;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE,
                     (char*)&opt, sizeof(opt) ) == -1 )
     { Info_c( Config.log, DEBUG_ADMIN, "Set option failed", strerror(errno) ); return(-1); }

    opt = 16834;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_SNDBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_c( Config.log, DEBUG_ADMIN, "SO_SNDBUF failed", strerror(errno) ); return(-1); }
    if ( setsockopt( ecoute, SOL_SOCKET, SO_RCVBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_c( Config.log, DEBUG_ADMIN, "SO_RCVBUF failed", strerror(errno) ); return(-1); }

    memset( &local, 0, sizeof(local) );
    unlink(NOM_SOCKET);
    local.sun_family = AF_UNIX;
    g_snprintf( local.sun_path, sizeof(local.sun_path), NOM_SOCKET );
    if (bind( ecoute, (struct sockaddr *)&local, sizeof(local)) == -1)
     { Info_c( Config.log, DEBUG_ADMIN, "Bind failure...", strerror(errno) );
       close(ecoute);
       return(-1);
     }

    if (listen(ecoute, 1) == -1)                                       /* On demande d'écouter aux portes */
     { Info_c( Config.log, DEBUG_ADMIN, "Listen failure...", strerror(errno));
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
  { close (Partage->com_admin.ecoute);
    Partage->com_admin.ecoute = 0;
    Info( Config.log, DEBUG_ADMIN, "Desactivation socket" );
  }
/**********************************************************************************************************/
/* Deconnecter_admin: Ferme la socket admin en parametre                                                  */
/* Entrée: le client                                                                                      */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Deconnecter_admin ( struct CLIENT_ADMIN *client )
  { close ( client->connexion );
    g_free(client);
    Info_n( Config.log, DEBUG_ADMIN, "Connexion terminée ID", client->connexion );
    Partage->com_admin.Clients = g_list_remove ( Partage->com_admin.Clients, client );
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
     { Info_n( Config.log, DEBUG_ADMIN, "Accueillir_un_admin: Connexion wanted. ID", id );

       client = g_malloc0( sizeof(struct CLIENT_ADMIN) );/* On alloue donc une nouvelle structure cliente */
       if (!client) { Info_n ( Config.log, DEBUG_MEM,
                               "SSRV: Accueillir_un_admin: Not enought memory to connect", id );
                      close(id);
                      return(FALSE);                                    /* On traite bien sûr les erreurs */
                    }

       client->connexion = id;
       client->mode = MODE_ADMIN_RUNNING;
       client->last_use = Partage->top;
       fcntl( client->connexion, F_SETFL, O_NONBLOCK );                              /* Mode non bloquant */

       Partage->com_admin.Clients = g_list_append( Partage->com_admin.Clients, client );
       Info_n( Config.log, DEBUG_ADMIN, "Connexion acceptée ID", id);
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
    gint taille;

    taille = read( client->connexion, ligne, sizeof(ligne) );

    if (taille > 0)
     { ligne[taille] = 0;

       client->last_use = Partage->top;
       Info_c( Config.log, DEBUG_ADMIN, "Admin : received command", ligne );
       sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */

       if ( ! strcmp ( commande, "mode" ) )
        { gchar mode[128];
          int i;
          memset( mode, 0, sizeof(mode) );
          sscanf ( ligne, "%s %s", commande, mode );                 /* Découpage de la ligne de commande */
          i = 0;
          while ( i < NBR_MODE_ADMIN && strcmp ( mode, Mode_admin[i] ) ) i++;
          if ( i == NBR_MODE_ADMIN ) i = MODE_ADMIN_RUNNING;
          client->mode = i;
        } else
       if ( ! strcmp ( commande, "exit" ) )
        { client->mode = MODE_ADMIN_RUNNING; }

       switch ( client->mode )
        {
          case MODE_ADMIN_MODBUS : Admin_modbus  ( client, ligne ); break;
          case MODE_ADMIN_PROCESS: Admin_process ( client, ligne ); break;

          case MODE_ADMIN_RUNNING:
          default:                 break;
        }
       Admin_running( client, ligne );

       if (client->mode == MODE_ADMIN_RUNNING)
        { g_snprintf( chaine, sizeof(chaine), " #%s> ", Mode_admin[client->mode] ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " # - WARNING - %s> ", Mode_admin[client->mode] ); }
       Write_admin ( client->connexion, chaine );
     }
  }
/**********************************************************************************************************/
/* Gerer_fifo_admin: Ecoute les commandes d'admin locale et les traite                                    */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Run_admin ( void )
  { 

    prctl(PR_SET_NAME, "W-Admin", 0, 0, 0 );

    Info( Config.log, DEBUG_FORK, "Admin: demarrage" );

    Partage->com_admin.ecoute = Activer_ecoute_admin ();
    if ( Partage->com_admin.ecoute < 0 )
     { Info( Config.log, DEBUG_FORK, "ADMIN: Run_admin: Unable to open Socket -> Stop !" );
       pthread_exit(GINT_TO_POINTER(-1));
     } else Info( Config.log, DEBUG_FORK, "ADMIN: Run_admin: En ecoute !" );

    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { if (Partage->com_admin.sigusr1)                                            /* On a recu sigusr1 ?? */
        { Partage->com_admin.sigusr1 = FALSE;
          Info( Config.log, DEBUG_INFO, "ADMIN: Run_admin: SIGUSR1" );
        }

       Accueillir_un_admin( Partage->com_admin.ecoute );                  /* Accueille les nouveaux admin */

       if ( Partage->com_admin.Clients )                                          /* Ecoutons nos clients */
        { struct CLIENT_ADMIN *client;
          GList *liste;

          liste = Partage->com_admin.Clients;
          while (liste)
           { client = (struct CLIENT_ADMIN *)liste->data;

             if ( Partage->top > client->last_use + 60 )       /* Deconnexion = 60 secondes si inactivité */
              { Deconnecter_admin ( client ); 
                continue;
              }

             Ecouter_admin( client );
             liste=liste->next;
           }
        }
       sched_yield();
       usleep(10000);
     }

    while(Partage->com_admin.Clients)                                 /* Parcours de la liste des clients */
     { struct CLIENT_ADMIN *client;                                   /* Deconnection de tous les clients */
       client = (struct CLIENT_ADMIN *)Partage->com_admin.Clients->data;
       Deconnecter_admin ( client );
     }

    Desactiver_ecoute_admin ();
    Info_n( Config.log, DEBUG_FORK, "Admin: Run_admin: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
