/**********************************************************************************************************/
/* Watchdogd/HttpMobile/HttpMobile.c        Gestion des connexions HTTPMobile de watchdog */
/* Projet WatchDog version 2.0       Gestion d'habitat                   mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * HttpMobile.c
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
 #include <libsoup/soup.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "HttpMobile.h"

/**********************************************************************************************************/
/* HttpMobile_Lire_config : Lit la config Watchdog et rempli la structure mémoire                             */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void HttpMobile_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "HttpMobile_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_httpmobile.lib->Thread_debug = g_key_file_get_boolean ( gkf, "HTTP", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */

    Cfg_httpmobile.enable        = g_key_file_get_boolean ( gkf, "HTTP", "enable", NULL ); 
    Cfg_httpmobile.httpmobile_enable = g_key_file_get_boolean ( gkf, "HTTP", "http_enable", NULL ); 
    Cfg_httpmobile.port          = g_key_file_get_integer ( gkf, "HTTP", "port", NULL );
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* HttpMobile_Liberer_config : Libere la mémoire allouer précédemment pour lire la config httpmobile      */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void HttpMobile_Liberer_config ( void )
  {
  }
/**********************************************************************************************************/
/* HttpMobile_Gerer_message: Fonction d'abonné appellé lorsqu'un message est disponible.                  */
/* Entrée: une structure CMD_TYPE_HISTO                                                                   */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void HttpMobile_Gerer_message ( struct CMD_TYPE_MESSAGE *msg )
  { gint taille;
#ifdef bouh
    pthread_mutex_lock( &Cfg_httpmobile.lib->synchro );                      /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_httpmobile.Liste_message );
    pthread_mutex_unlock( &Cfg_httpmobile.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_WARNING,
                "HttpMobile_Gerer_message: DROP message %d (length = %d > 150)", msg->num, taille);
       g_free(msg);
       return;
     }

    pthread_mutex_lock ( &Cfg_httpmobile.lib->synchro );
    Cfg_httpmobile.Liste_message = g_slist_append ( Cfg_httpmobile.Liste_message, msg );      /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_httpmobile.lib->synchro );

#endif
  }
/**********************************************************************************************************/
/* HttpMobile_Gerer_sortie: Ajoute une demande d'envoi RF dans la liste des envois RFXCOM                     */
/* Entrées: le numéro de la sortie                                                                        */
/**********************************************************************************************************/
 void HttpMobile_Gerer_sortie( gint num_a )                                    /* Num_a est l'id de la sortie */
  { gint taille;
#ifdef bouh
    pthread_mutex_lock( &Cfg_httpmobile.lib->synchro );              /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_httpmobile.Liste_sortie );
    pthread_mutex_unlock( &Cfg_httpmobile.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_WARNING,
                "HttpMobile_Gerer_sortie: DROP sortie %d (length = %d > 150)", num_a, taille );
       return;
     }

    pthread_mutex_lock( &Cfg_httpmobile.lib->synchro );       /* Ajout dans la liste de tell a traiter */
    Cfg_httpmobile.Liste_sortie = g_slist_prepend( Cfg_httpmobile.Liste_sortie, GINT_TO_POINTER(num_a) );
    pthread_mutex_unlock( &Cfg_httpmobile.lib->synchro );
#endif
  }
/**********************************************************************************************************/
/* HttpMobile _log_query : Log la query dans les logs systemes                                            */
/* Entrées : le contexte, le msg                                                                          */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void Print_query ( SoupClientContext *client, SoupMessage *msg )
  { gchar *uri_text;
    SoupURI *uri;
    uri = soup_message_get_uri ( msg );
    uri_text = soup_uri_to_string ( uri, FALSE );
    Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_DEBUG,
              "HttpMobile_/_CB: get MSG from %s (HTTP version 1.%d) for URI %s",
              soup_client_context_get_host (client),
              soup_message_get_http_version ( msg ),
              uri_text
            );
    soup_uri_free( uri );
  }
/**********************************************************************************************************/
/* HttpMobile Callback : Renvoi une reponse suite a une demande d'un slave (appellée par libsoup)         */
/* Entrées : le contexte, le message, l'URL                                                               */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void HttpMobile_slash_CB (SoupServer        *server,  SoupMessage       *msg, 
                                  const char        *path,  GHashTable          *query,
                                  SoupClientContext *client, gpointer           user_data)
  { gchar *response = "This is the end !!\n";

    Print_query ( client, msg );

    soup_message_set_response ( msg, "text/plain", SOUP_MEMORY_COPY, response, strlen(response) );
    soup_message_set_status   ( msg, SOUP_STATUS_OK );

  }  
/**********************************************************************************************************/
/* HttpMobile Callback : Renvoi une reponse suite a une demande d'un slave (appellée par libsoup)         */
/* Entrées : le contexte, le message, l'URL                                                               */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void HttpMobile_get_status_CB (SoupServer        *server,  SoupMessage       *msg, 
                                       const char        *path,  GHashTable        *query,
                                       SoupClientContext *client, gpointer           user_data)
  { gchar *response = "<xml> fait a la mimine !! </xml>";
    Print_query ( client, msg );

    soup_message_set_response ( msg, "text/xml", SOUP_MEMORY_COPY, response, strlen(response) );
    soup_message_set_status   ( msg, SOUP_STATUS_OK );
  }  
/**********************************************************************************************************/
/* Run_thread: Thread principal                                                                           */
/* Entrée: une structure LIBRAIRIE                                                                        */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { prctl(PR_SET_NAME, "W-HTTP", 0, 0, 0 );
    memset( &Cfg_httpmobile, 0, sizeof(Cfg_httpmobile) );               /* Mise a zero de la structure de travail */
    Cfg_httpmobile.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    HttpMobile_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );

    g_snprintf( Cfg_httpmobile.lib->admin_prompt, sizeof(Cfg_httpmobile.lib->admin_prompt), "http" );
    g_snprintf( Cfg_httpmobile.lib->admin_help,   sizeof(Cfg_httpmobile.lib->admin_help),   "Manage communications with Http Devices" );

    if (!Cfg_httpmobile.enable)
     { Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread not enable in config. Shutting Down %d", pthread_self() );
       goto end;
     }

    Cfg_httpmobile.context = g_main_context_new ();
    Cfg_httpmobile.server  = soup_server_new ( SOUP_SERVER_PORT, Cfg_httpmobile.port,
                                               SOUP_SERVER_ASYNC_CONTEXT, Cfg_httpmobile.context,
                                               NULL
                                             );
    if (!Cfg_httpmobile.server)
     { Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: SoupServer creation error. Shutting Down %d", pthread_self() );
       goto end;
     }

    soup_server_add_handler ( Cfg_httpmobile.server, "/",           HttpMobile_slash_CB, NULL, NULL );
    soup_server_add_handler ( Cfg_httpmobile.server, "/get_status", HttpMobile_get_status_CB, NULL, NULL );
    soup_server_run_async   ( Cfg_httpmobile.server );

#ifdef bouh
    Abonner_distribution_message ( HttpMobile_Gerer_message );   /* Abonnement à la diffusion des messages */
    Abonner_distribution_sortie  ( HttpMobile_Gerer_sortie );     /* Abonnement à la diffusion des sorties */

#endif

    Cfg_httpmobile.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */
    while(Cfg_httpmobile.lib->Thread_run == TRUE)                            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_httpmobile.lib->Thread_sigusr1)                                /* A-t'on recu un signal USR1 ? */
        { int nbr_msg, nbr_sortie;

          Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
#ifdef bouh
          pthread_mutex_lock( &Cfg_httpmobile.lib->synchro );     /* On recupere le nombre de msgs en attente */
          nbr_msg    = g_slist_length(Cfg_httpmobile.Liste_message);
          nbr_sortie = g_slist_length(Cfg_httpmobile.Liste_sortie);
          pthread_mutex_unlock( &Cfg_httpmobile.lib->synchro );
          Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_INFO,
                    "Run_thread: In Queue : %d MSGS, %d A", nbr_msg, nbr_sortie );
#endif
          Cfg_httpmobile.lib->Thread_sigusr1 = FALSE;
        }

      /* Envoyer_les_sorties_aux_slaves();*/

       g_main_context_iteration ( Cfg_httpmobile.context, FALSE );
     }

    Desabonner_distribution_sortie  ( HttpMobile_Gerer_sortie ); /* Desabonnement de la diffusion des sorties */
    Desabonner_distribution_message ( HttpMobile_Gerer_message );/* Desabonnement de la diffusion des messages */

    soup_server_disconnect ( Cfg_httpmobile.server );
    g_main_context_unref (Cfg_httpmobile.context );

end:
    HttpMobile_Liberer_config();                                  /* Liberation de la configuration du thread */
    Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_httpmobile.lib->TID = 0;                              /* On indique au httpmobile que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
