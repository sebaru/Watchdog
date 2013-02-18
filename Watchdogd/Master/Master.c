/**********************************************************************************************************/
/* Watchdogd/master.c        Gestion des MASTER de Watchdog v2.0                                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                    lun. 18 févr. 2013 18:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Master.c
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
 
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/socket.h>
 #include <netinet/tcp.h>
 #include <netinet/in.h>                                          /* Pour les structures d'entrées SOCKET */
 #include <sys/wait.h>
 #include <netinet/in.h>                                          /* Pour les structures d'entrées SOCKET */
 #include <fcntl.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Master.h"

/**********************************************************************************************************/
/* Master_Lire_config : Lit la config Watchdog et rempli la structure mémoire                             */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Master_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Master_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_master.lib->Thread_debug = g_key_file_get_boolean ( gkf, "MASTER", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */

    Cfg_master.enable = g_key_file_get_boolean ( gkf, "MASTER", "enable", NULL ); 
    Cfg_master.port   = g_key_file_get_integer ( gkf, "MASTER", "port", NULL );
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Master_Liberer_config : Libere la mémoire allouer précédemment pour lire la config master              */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Master_Liberer_config ( void )
  {
  }
/**********************************************************************************************************/
/* Master_Gerer_message: Fonction d'abonné appellé lorsqu'un message est disponible.                      */
/* Entrée: une structure CMD_TYPE_HISTO                                                                   */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Master_Gerer_message ( struct CMD_TYPE_MESSAGE *msg )
  { gint taille;

    pthread_mutex_lock( &Cfg_master.lib->synchro );                      /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_master.Liste_message );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Master_Gerer_message: DROP message %d (length = %d > 150)", msg->num, taille);
       g_free(msg);
       return;
     }

    pthread_mutex_lock ( &Cfg_master.lib->synchro );
    Cfg_master.Liste_message = g_slist_append ( Cfg_master.Liste_message, msg );      /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_master.lib->synchro );
  }
/**********************************************************************************************************/
/* Master_Gerer_sortie: Ajoute une demande d'envoi RF dans la liste des envois RFXCOM                     */
/* Entrées: le numéro de la sortie                                                                        */
/**********************************************************************************************************/
 void Master_Gerer_sortie( gint num_a )                                    /* Num_a est l'id de la sortie */
  { gint taille;

    pthread_mutex_lock( &Cfg_master.lib->synchro );              /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_master.Liste_sortie );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Master_Gerer_sortie: DROP sortie %d (length = %d > 150)", num_a, taille );
       return;
     }

    pthread_mutex_lock( &Cfg_master.lib->synchro );       /* Ajout dans la liste de tell a traiter */
    Cfg_master.Liste_sortie = g_slist_prepend( Cfg_master.Liste_sortie, GINT_TO_POINTER(num_a) );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gint Activer_ecoute_slave ( void )
  { struct sockaddr_in local;
    gint opt, ecoute;

    if ( (ecoute = socket ( AF_INET, SOCK_STREAM, 0 )) == -1)                           /* Protocol = TCP */
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR, "Socket failure (%s)", strerror(errno) ); return(-1); }

    opt = 1;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE,
                     (char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                "Set option failed (%s)", strerror(errno) );
       return(-1);
     }

    opt = 16834;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_SNDBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                "SO_SNDBUF failed (%s)", strerror(errno) );
       return(-1);
     }
    if ( setsockopt( ecoute, SOL_SOCKET, SO_RCVBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                "SO_RCVBUF failed (%s)", strerror(errno) );
       return(-1);
     }

    opt = 1;
    if ( setsockopt( ecoute, SOL_TCP, TCP_NODELAY,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                "TCP_NODELAY failed (%s)", strerror(errno) );
       return(-1);
     }

    memset( &local, 0, sizeof(local) );
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(Cfg_master.port);                  /* Attention: en mode network, pas host !!! */
    if (bind( ecoute, (struct sockaddr *)&local, sizeof(local)) == -1)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                "Bind failure (%s)", strerror(errno) );
       close(ecoute);
       return(-1);
     }

    if (listen(ecoute, 1) == -1)                                       /* On demande d'écouter aux portes */
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                "Listen failure (%s)", strerror(errno));
       close(ecoute);
       return(-1);
     }
    fcntl( ecoute, F_SETFL, O_NONBLOCK );        /* Mode non bloquant, ça aide pour une telle application */
    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
              "Ecoute du port %d with socket %d", Config.port, ecoute );
    return( ecoute );                                                            /* Tout s'est bien passé */
  }
/**********************************************************************************************************/
/* Desactiver_ecoute_master: Ferme la socker fifo d'masteristration                                         */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Desactiver_ecoute_slave ( void )
  { close (Cfg_master.Fd_ecoute);
    Cfg_master.Fd_ecoute = 0;
    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Desactiver_ecoute_slave: socket disabled" );
  }
/**********************************************************************************************************/
/* Deconnecter_un_slave: Ferme la socket master en parametre                                                  */
/* Entrée: le client                                                                                      */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Deconnecter_un_slave ( struct CLIENT *client )
  { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_OFF, NULL, 0 );
    Fermer_connexion( client->connexion );
    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
              "Deconnecter_un_slave : connection closed with client %d", client->connexion->socket );
    Cfg_master.Slaves = g_slist_remove ( Cfg_master.Slaves, client );
    g_free(client);
  }
/**********************************************************************************************************/
/* Accueillir_nouveaux_clients: Cette fonction permet de loguer d'éventuels nouveaux clients distants     */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE si un nouveau client est arrivé                                                           */
/**********************************************************************************************************/
 static gboolean Accueillir_un_slave( void )
  { struct CLIENT *client;
    struct sockaddr_in distant;
    guint taille_distant, id;
 
    taille_distant = sizeof(distant);
    if ( (id=accept( Cfg_master.Fd_ecoute, (struct sockaddr *)&distant, &taille_distant )) != -1)         /* demande ?? */
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                 "Accueillir_un_slave: Connexion wanted. ID=%d", id );

       client = g_try_malloc0( sizeof(struct CLIENT) );  /* On alloue donc une nouvelle structure cliente */
       if (!client) { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                                "Accueillir_un_slave: Not enought memory to connect slave %d", id );
                      close(id);
                      return(FALSE);                                    /* On traite bien sûr les erreurs */
                    }

       client->connexion = Nouvelle_connexion( Config.log, id, Config.taille_bloc_reseau );

       if (!client->connexion)
        { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                   "Accueillir_un_slave: Not enought memory for %d", id );
          close(id);
          g_free( client );
          return(FALSE);
        }

       Cfg_master.Slaves = g_slist_prepend( Cfg_master.Slaves, client );
       Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                 "Accueillir_un_slave: Connexion granted to ID=%d", id );
       Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_PAQUETSIZE,         /* Envoi des infos internes */
                     NULL, client->connexion->taille_bloc );
       Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_END,                /* Tag de fin */
                     NULL, 0 );

       return(TRUE);
     }
    return(FALSE);
  }
#ifdef bouh
/**********************************************************************************************************/
/* Admin_write : Concatene la chaine en parametre dans le buffer de reponse                               */
/* Entrée : le buffer et la chaine                                                                        */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Master_write ( struct CLIENT *client, gchar *response )
  { Envoi_client (client, TAG_MASTER, SSTAG_SERVEUR_RESPONSE_BUFFER, response, strlen(response)+1 );
  }
#endif
/**********************************************************************************************************/
/* Ecouter_slave: Ecoute ce que dis le client                                                             */
/* Entrée: le client                                                                                      */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Ecouter_slave ( struct CLIENT *client )
  { gint recu;

    recu = Recevoir_reseau( client->connexion );
    if (recu==RECU_OK)
     { if ( Reseau_tag(client->connexion) == TAG_MASTER_SLAVE )
        { switch ( Reseau_ss_tag(client->connexion) )
           { default : Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_DEBUG,
                                "Ecouter_slave: SSTAG unknown" );
           }
        } else
        { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_DEBUG, "Ecouter_slave: Wrong TAG" ); }
     }
    else if (recu>=RECU_ERREUR)                                             /* Erreur reseau->deconnexion */
     { switch( recu )
        { case RECU_ERREUR_CONNRESET: Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_DEBUG,
                                               "Ecouter_slave: Reset connexion" );
                                      break;
        }
       Deconnecter_un_slave ( client );
     }             
  }

/**********************************************************************************************************/
/* Run_thread: Thread principal                                                                           */
/* Entrée: une structure LIBRAIRIE                                                                        */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct CMD_TYPE_MESSAGE *msg;

    prctl(PR_SET_NAME, "W-MASTER", 0, 0, 0 );
    memset( &Cfg_master, 0, sizeof(Cfg_master) );               /* Mise a zero de la structure de travail */
    Cfg_master.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    Master_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );

    g_snprintf( Cfg_master.lib->admin_prompt, sizeof(Cfg_master.lib->admin_prompt), "master" );
    g_snprintf( Cfg_master.lib->admin_help,   sizeof(Cfg_master.lib->admin_help),   "Manage communications with Slaves Watchdog" );

    if (!Cfg_master.enable)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread not enable in config. Shuting Down %d", pthread_self() );
       goto end;
     }

    Cfg_master.Fd_ecoute = Activer_ecoute_slave ();
    if ( Cfg_master.Fd_ecoute < 0 )
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_CRIT,
              "Run_thread: Unable to open Socket -> Stop" );
       goto end;
     }
    Cfg_master.Slaves = NULL;                                             /* Initialisation des variables du thread */
    Cfg_master.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */

    Abonner_distribution_message ( Master_Gerer_message );      /* Abonnement à la diffusion des messages */
    Abonner_distribution_sortie  ( Master_Gerer_sortie );        /* Abonnement à la diffusion des sorties */

    while(Cfg_master.lib->Thread_run == TRUE)                            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_master.lib->Thread_sigusr1)                                /* A-t'on recu un signal USR1 ? */
        { int nbr_msg, nbr_sortie;

          Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
          pthread_mutex_lock( &Cfg_master.lib->synchro );     /* On recupere le nombre de msgs en attente */
          nbr_msg    = g_slist_length(Cfg_master.Liste_message);
          nbr_sortie = g_slist_length(Cfg_master.Liste_sortie);
          pthread_mutex_unlock( &Cfg_master.lib->synchro );
          Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                    "Run_thread: In Queue : %d MSGS, %d A", nbr_msg, nbr_sortie );
          Cfg_master.lib->Thread_sigusr1 = FALSE;
        }

     }

    Desactiver_ecoute_slave ();                                          /* Arret de l'ecoute du port TCP */
    Desabonner_distribution_sortie  ( Master_Gerer_sortie ); /* Desabonnement de la diffusion des sorties */
    Desabonner_distribution_message ( Master_Gerer_message );/* Desabonnement de la diffusion des messages */

end:
    Master_Liberer_config();                                  /* Liberation de la configuration du thread */
    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_master.lib->TID = 0;                              /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
