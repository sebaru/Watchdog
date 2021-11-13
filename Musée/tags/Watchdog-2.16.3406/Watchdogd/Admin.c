/******************************************************************************************************************************/
/* Watchdogd/Admin/Admin.c        Gestion des connexions Admin au serveur watchdog                                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
 
 #include <sys/socket.h>
 #include <sys/un.h>                                                                   /* Description de la structure AF UNIX */
 #include <sys/types.h>
 #include <sys/prctl.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "watchdogd.h"

 static GSList *Clients = NULL;                                                        /* Liste des clients d'admin connectés */
 static gint Fd_ecoute = 0;                                                              /* File descriptor de l'ecoute admin */

/******************************************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes d'admin au serveur watchdog                                             */
/* Entrée: Néant                                                                                                              */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gint Activer_ecoute_admin ( void )
  { struct sockaddr_un local;
    gint opt, ecoute;

    if ( (ecoute = socket ( AF_UNIX, SOCK_STREAM, 0 )) == -1)                                               /* Protocol = TCP */
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                 "Activer_ecoute_admin: Socket failure...:%s", strerror(errno) );
       return(-1);
     }

    opt = 1;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE,
                     (char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                 "Activer_ecoute_admin: Set Socket Option failed...:%s", strerror(errno) );
       return(-1);
     }

    opt = 16834;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_SNDBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                 "Activer_ecoute_admin: Set SEND BUF failed...:%s", strerror(errno) );
       return(-1);
     }
    if ( setsockopt( ecoute, SOL_SOCKET, SO_RCVBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                 "Activer_ecoute_admin: Set RCV BUF failed...:%s", strerror(errno) );
       return(-1);
     }

    memset( &local, 0, sizeof(local) );
    unlink(NOM_SOCKET);
    local.sun_family = AF_UNIX;
    g_snprintf( local.sun_path, sizeof(local.sun_path), NOM_SOCKET );
    if (bind( ecoute, (struct sockaddr *)&local, sizeof(local)) == -1)
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                 "Activer_ecoute_admin: Bind Failure for '%s' : %s", NOM_SOCKET, strerror(errno) );
       close(ecoute);
       return(-1);
     }

    if (listen(ecoute, 1) == -1)                                                           /* On demande d'écouter aux portes */
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                 "Activer_ecoute_admin: Listen failure for '%s' : %s", NOM_SOCKET, strerror(errno) );
       close(ecoute);
       return(-1);
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO,
              "Activer_ecoute_admin: Listen success for %s", NOM_SOCKET );
    fcntl( ecoute, F_SETFL, O_NONBLOCK );                                                                /* Mode non bloquant */
    return( ecoute );
  }
/******************************************************************************************************************************/
/* Desactiver_ecoute_admin: Ferme la socker fifo d'administration                                                             */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Desactiver_ecoute_admin ( void )
  { close (Fd_ecoute);
    Fd_ecoute = 0;
    unlink(NOM_SOCKET);                                               /* Suppression du fichier de socket */
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Desactiver_ecoute_admin: socket disabled" );
  }
/******************************************************************************************************************************/
/* Deconnecter_admin: Ferme la socket admin en parametre                                                                      */
/* Entrée: le CLIENT                                                                                                          */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Deconnecter_admin ( struct CONNEXION *connexion )
  { Envoyer_reseau( connexion, TAG_CONNEXION, SSTAG_SERVEUR_OFF, NULL, 0 );
    Clients = g_slist_remove ( Clients, connexion );
    Info_new( Config.log, Config.log_msrv, LOG_INFO,
              "Deconnecter_admin : connection closed with CLIENT %d", connexion->socket );
    Fermer_connexion( connexion );
  }
/******************************************************************************************************************************/
/* Accueillir_nouveaux_clients: Cette fonction permet de loguer d'éventuels nouveaux clients distants                         */
/* Entrée: rien                                                                                                               */
/* Sortie: TRUE si un nouveau CLIENT est arrivé                                                                               */
/******************************************************************************************************************************/
 static gboolean Accueillir_un_admin( gint ecoute )
  { struct CONNEXION *connexion;
    struct sockaddr_un distant;
    guint taille_distant, id;
 
    taille_distant = sizeof(distant);
    if ( (id=accept( ecoute, (struct sockaddr *)&distant, &taille_distant )) != -1)                             /* demande ?? */
     { Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "Accueillir_un_admin: Connexion wanted. ID=%d", id );

       connexion = Nouvelle_connexion( Config.log, id, 16384 );

       if (!connexion)
        { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                   "Accueillir_un_admin: Not enought memory for %d", id );
          close(id);
          return(FALSE);
        }

       Clients = g_slist_prepend( Clients, connexion );
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                "Accueillir_un_admin: Connexion granted to ID=%d. Sending TAG_INTERNAL", id );
       Envoyer_reseau( connexion, TAG_INTERNAL, SSTAG_INTERNAL_PAQUETSIZE,/* Envoi des infos internes */
                       NULL, connexion->taille_bloc );
       Envoyer_reseau( connexion, TAG_INTERNAL, SSTAG_INTERNAL_END,                 /* Tag de fin */
                       NULL, 0 );
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                "Accueillir_un_admin: TAG_INTERNAL sent to %d", id );
       return(TRUE);
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Admin_write : Concatene deux chaines de caracteres proprementvvvvvvv                                                       */
/* Entrée : le buffer et la chaine                                                                                            */
/* Sortie: La nouvelle chaine                                                                                                 */
/******************************************************************************************************************************/
 gchar *Admin_write ( gchar *response, gchar *new_ligne )
  { gchar *new;
    new = g_strconcat( response, new_ligne, "\n", NULL );
    g_free(response);
    return(new);
  }
/******************************************************************************************************************************/
/* Ecouter_admin: Ecoute ce que dis le CLIENT                                                                                 */
/* Entrée: la connexion, le user host d'origine et commande a parser                                                          */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Processer_commande_admin ( gchar *user, gchar *host, gchar *ligne )
  { gchar commande[128], chaine[256];
    struct LIBRAIRIE *lib;
    gchar *response=NULL;
    GSList *liste;

    if (!(user && host)) return(NULL);

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
             "%s: Commande Received from %s@%s : %s", __func__, user, host, ligne );

    g_snprintf( chaine, sizeof(chaine), "At %010.1f, processing '%s'", (gdouble)Partage->top/10.0, ligne );
    response = Admin_write ( g_strdup(chaine), "" );

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

            if ( ! strcmp ( commande, "process"   ) ) { response = Admin_process  ( response, ligne + 8 ); }
       else if ( ! strcmp ( commande, "dls"       ) ) { response = Admin_dls      ( response, ligne + 4 ); }
       else if ( ! strcmp ( commande, "set"       ) ) { response = Admin_set      ( response, ligne + 4);  }
       else if ( ! strcmp ( commande, "get"       ) ) { response = Admin_get      ( response, ligne + 4);  }
       else if ( ! strcmp ( commande, "user"      ) ) { response = Admin_user     ( response, ligne + 5);  }
       else if ( ! strcmp ( commande, "arch"      ) ) { response = Admin_arch     ( response, ligne + 5);  }
       else { gboolean found = FALSE;
              liste = Partage->com_msrv.Librairies;                                      /* Parcours de toutes les librairies */
              while(liste)
               { lib = (struct LIBRAIRIE *)liste->data;
                 if ( ! strcmp( commande, lib->admin_prompt ) )
                  { if (lib->Thread_run == FALSE)
                     { response = Admin_write ( response, " -- WARNING --" );
                       response = Admin_write ( response, " -- Thread is not started, Running config is not loaded --");
                       response = Admin_write ( response, " -- WARNING --" );
                     }    
                    response =  lib->Admin_command ( response, ligne + strlen(lib->admin_prompt)+1 );          /* Appel local */
                    found = TRUE;
                  }
                 liste = liste->next;
               }
              if (found == FALSE)                                                /* Si pas trouvé, rollback sur Admin_running */
               { response = Admin_running ( response, ligne ); }
            }
    response = Admin_write ( response, " -\n" );
    return(response);                                                                                    /* Fin de la reponse */
  }
/******************************************************************************************************************************/
/* Ecouter_admin: Ecoute ce que dis le CLIENT                                                                                 */
/* Entrée: le CLIENT                                                                                                          */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Ecouter_admin ( struct CONNEXION *connexion )
  { gint recu;

    recu = Recevoir_reseau( connexion );
    if (recu==RECU_OK)
     { if ( Reseau_tag(connexion) == TAG_ADMIN && Reseau_ss_tag (connexion) == SSTAG_CLIENT_REQUEST )
        { struct CMD_TYPE_ADMIN *admin;
          gchar *response;
          admin = (struct CMD_TYPE_ADMIN *)connexion->donnees;
          Envoyer_reseau ( connexion, TAG_ADMIN, SSTAG_SERVEUR_RESPONSE_START, NULL, 0 );              /* Debut de la reponse */
          response = Processer_commande_admin ( "localuser", "localhost", admin->buffer );
          Envoyer_reseau ( connexion, TAG_ADMIN, SSTAG_SERVEUR_RESPONSE_BUFFER, response, strlen(response)+1 );
          g_free(response);
          Envoyer_reseau ( connexion, TAG_ADMIN, SSTAG_SERVEUR_RESPONSE_STOP, NULL, 0 );                 /* Fin de la reponse */
        } else
        { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Ecouter_admin: Wrong TAG" ); }
     }
    else if (recu>=RECU_ERREUR)                                             /* Erreur reseau->deconnexion */
     { switch( recu )
        { case RECU_ERREUR_CONNRESET: Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                                               "Ecouter_admin: Reset connexion" );
                                      break;
        }
       Deconnecter_admin ( connexion );
     }             
  }
/******************************************************************************************************************************/
/* Run_admin: Ecoute les commandes d'admin locale et les traite                                                               */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Run_admin ( void )
  { prctl(PR_SET_NAME, "W-Admin", 0, 0, 0 );

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
              "Run_admin: Demarrage . . . TID = %p", pthread_self() );

    Fd_ecoute = Activer_ecoute_admin ();
    if ( Fd_ecoute < 0 )
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
              "Run_admin: Unable to open Socket -> Stop" );
       Partage->com_admin.TID = 0;                                            /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     } else Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                      "Run_admin: Socket is enabled, waiting for clients" );

    Clients = NULL;                                                                 /* Initialisation des variables du thread */
    Partage->com_admin.Thread_run = TRUE;                                                               /* Le thread tourne ! */
    while(Partage->com_admin.Thread_run == TRUE)                                             /* On tourne tant que necessaire */
     {

       if (Partage->com_admin.Thread_sigusr1)                                                         /* On a recu sigusr1 ?? */
        { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Run_admin: recu SIGUSR1" );
          Partage->com_admin.Thread_sigusr1 = FALSE;
        }

       Accueillir_un_admin( Fd_ecoute );                                                      /* Accueille les nouveaux admin */

       if ( Clients )                                                                                 /* Ecoutons nos clients */
        { struct CONNEXION *connexion;
          GSList *liste;

          liste = Clients;
          while (liste)
           { connexion = (struct CONNEXION *)liste->data;

             if ( time(NULL) > connexion->last_use + 300 )                        /* Deconnexion = 300 secondes si inactivité */
              { Info_new( Config.log, Config.log_msrv, LOG_INFO, "Run_admin: Deconnexion Admin sur inactivite" );
                Deconnecter_admin ( connexion ); 
                liste = Clients;
                continue;
              }
             Ecouter_admin( connexion );
             liste = liste->next;
           }
        }
       sched_yield();
       usleep(10000);
     }

    while(Clients)                                                                        /* Parcours de la liste des clients */
     { struct CONNEXION *connexion;                                                       /* Deconnection de tous les clients */
       connexion = ( struct CONNEXION *)Clients->data;
       Deconnecter_admin ( connexion ); 
     }

    Desactiver_ecoute_admin ();
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
              "Run_admin: Down . . . TID = %p", pthread_self() );
    Partage->com_admin.TID = 0;                                               /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
