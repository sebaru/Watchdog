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

 static void *Socket;

/******************************************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes d'admin au serveur watchdog                                             */
/* Entrée: Néant                                                                                                              */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Activer_ecoute_admin ( void )
  { Socket = New_zmq ( ZMQ_REP, "listen-local-admin" );
    return (Bind_zmq ( Socket, "ipc", NOM_SOCKET, 0 ));
  }
/******************************************************************************************************************************/
/* Desactiver_ecoute_admin: Ferme la socker fifo d'administration                                                             */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Desactiver_ecoute_admin ( void )
  { Close_zmq ( Socket );
    unlink(NOM_SOCKET);                                                                   /* Suppression du fichier de socket */
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: socket disabled", __func__ );
  }
/******************************************************************************************************************************/
/* Admin_write : Concatene deux chaines de caracteres proprementvvvvvvv                                                       */
/* Entrée : le buffer et la chaine                                                                                            */
/* Sortie: La nouvelle chaine                                                                                                 */
/******************************************************************************************************************************/
 gchar *Admin_write ( gchar *response, gchar *new_ligne )
  { gchar *new;
    if (response == NULL) { response = g_strdup(""); }
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

    g_snprintf( chaine, sizeof(chaine), "At %010.1f, processing '%s' on instance '%s'",
                (gdouble)Partage->top/10.0, ligne, g_get_host_name() );
    response = Admin_write ( g_strdup(chaine), "\n" );

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

            if ( ! strcmp ( commande, "process"   ) ) { response = Admin_process  ( response, ligne + 8 ); }
       else if ( ! strcmp ( commande, "dls"       ) ) { response = Admin_dls      ( response, ligne + 4 ); }
       else if ( ! strcmp ( commande, "set"       ) ) { response = Admin_set      ( response, ligne + 4);  }
       else if ( ! strcmp ( commande, "get"       ) ) { response = Admin_get      ( response, ligne + 4);  }
       else if ( ! strcmp ( commande, "user"      ) ) { response = Admin_user     ( response, ligne + 5);  }
       else if ( ! strcmp ( commande, "dbcfg"     ) ) { response = Admin_dbcfg    ( response, ligne + 6);  }
       else if ( ! strcmp ( commande, "arch"      ) ) { response = Admin_arch     ( response, ligne + 5);  }
       else { gboolean found = FALSE;
              liste = Partage->com_msrv.Librairies;                                      /* Parcours de toutes les librairies */
              while(liste)
               { lib = (struct LIBRAIRIE *)liste->data;
                 if ( ! strcmp( commande, lib->admin_prompt ) )
                  { if (lib->Thread_run == FALSE)
                     { response = Admin_write ( response, " | -- WARNING --" );
                       response = Admin_write ( response, " | -- Thread is not started, Running config is not loaded --");
                       response = Admin_write ( response, " | -- WARNING --" );
                     }    
                    if (lib->Admin_command)                        /* Ancienne mode, via appel de fonction intégrée au thread */
                     { response =  lib->Admin_command ( response, ligne + strlen(lib->admin_prompt)+1 ); }     /* Appel local */
                    else if (lib->Thread_run == TRUE)                         /* Nouvelle méthode, en utilisant les files ZMQ */
                     { gchar endpoint[128], buffer[2048];
                       gint byte;
                       struct ZMQUEUE *zmq_admin;
                       zmq_admin = New_zmq ( ZMQ_REQ, "send-to-admin" );
                       g_snprintf(endpoint, sizeof(endpoint), "%s-admin", lib->admin_prompt );
                       Connect_zmq (zmq_admin, "inproc", endpoint, 0 );
                       Send_zmq ( zmq_admin, ligne + strlen(lib->admin_prompt)+1, strlen(ligne) - strlen(lib->admin_prompt) );
                       byte = Recv_zmq_block ( zmq_admin, &buffer, sizeof(buffer) );
                       buffer[byte-1]=0;                                              /* caractere NULL de fin si depassement */
                       response = Admin_write ( response, buffer );                                    /* Appel via zmq local */
                       Close_zmq ( zmq_admin );
                     }
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
/* Run_admin: Ecoute les commandes d'admin locale et les traite                                                               */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Run_admin ( void )
  { prctl(PR_SET_NAME, "W-Admin", 0, 0, 0 );

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
              "%s: Demarrage . . . TID = %p", __func__, pthread_self() );

    if ( Activer_ecoute_admin() == FALSE )
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Unable to open Socket -> Stop", __func__ );
       Partage->com_admin.TID = 0;                                            /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     } else Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                      "%s: Socket is enabled, waiting for clients", __func__ );

    Partage->com_admin.Thread_run = TRUE;                                                               /* Le thread tourne ! */
    while(Partage->com_admin.Thread_run == TRUE)                                             /* On tourne tant que necessaire */
     { gchar buffer[2048];
       if (Partage->com_admin.Thread_sigusr1)                                                         /* On a recu sigusr1 ?? */
        { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: recu SIGUSR1", __func__ );
          Partage->com_admin.Thread_sigusr1 = FALSE;
        }

       if ( Recv_zmq ( Socket, &buffer, sizeof(buffer) ) > 0 )
        { gchar *response;
          response = Processer_commande_admin ( "localuser", "localhost", buffer );
          Send_zmq ( Socket, response, strlen(response)+1 );
          Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Response = %s", __func__, response );
          g_free(response);
        }

       sched_yield();
       usleep(10000);
     }

    Desactiver_ecoute_admin ();
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    Partage->com_admin.TID = 0;                                               /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
