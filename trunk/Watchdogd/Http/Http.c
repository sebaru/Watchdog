/**********************************************************************************************************/
/* Watchdogd/Http/Http.c        Gestion des connexions HTTPMobile de watchdog */
/* Projet WatchDog version 2.0       Gestion d'habitat                   mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Http.c
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <gnutls/gnutls.h>
 #include <gnutls/x509.h>
 #include <openssl/rand.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"
 #include "Http.h"

/******************************************************************************************************************************/
/* Http_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                   */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Http_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_http.lib->Thread_debug = FALSE;                                                        /* Settings default parameters */
    Cfg_http.tcp_port          = HTTP_DEFAUT_TCP_PORT;
    Cfg_http.ssl_enable        = FALSE; 
    Cfg_http.authenticate      = TRUE; 
    Cfg_http.nbr_max_connexion = HTTP_DEFAUT_MAX_CONNEXION;
    g_snprintf( Cfg_http.ssl_cert_filepath,        sizeof(Cfg_http.ssl_cert_filepath), "%s", HTTP_DEFAUT_FILE_CERT );
    g_snprintf( Cfg_http.ssl_private_key_filepath, sizeof(Cfg_http.ssl_private_key_filepath), "%s", HTTP_DEFAUT_FILE_KEY );
    g_snprintf( Cfg_http.ssl_ca_filepath,          sizeof(Cfg_http.ssl_ca_filepath), "%s", HTTP_DEFAUT_FILE_CA  );
    g_snprintf( Cfg_http.ssl_cipher_list,          sizeof(Cfg_http.ssl_cipher_list), "%s", HTTP_DEFAUT_SSL_CIPHER );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                "Http_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     {      if ( ! g_ascii_strcasecmp ( nom, "ssl_file_cert" ) )
        { g_snprintf( Cfg_http.ssl_cert_filepath, sizeof(Cfg_http.ssl_cert_filepath), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_file_ca" ) )
        { g_snprintf( Cfg_http.ssl_ca_filepath, sizeof(Cfg_http.ssl_ca_filepath), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_file_key" ) )
        { g_snprintf( Cfg_http.ssl_private_key_filepath, sizeof(Cfg_http.ssl_private_key_filepath), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_cipher" ) )
        { g_snprintf( Cfg_http.ssl_cipher_list, sizeof(Cfg_http.ssl_cipher_list), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "max_connexion" ) )
        { Cfg_http.nbr_max_connexion = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_http.ssl_enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "tcp_port" ) )
        { Cfg_http.tcp_port = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "authenticate" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "false" ) ) Cfg_http.authenticate = FALSE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_http.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                   "Http_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* CB_ws_status : Gere le protocole WS status (appellée par libwebsockets)                                                    */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 static gint CB_ws_status ( struct lws *wsi, enum lws_callback_reasons tag, void *user, void *data, size_t taille )
  { return(1);
  }
/******************************************************************************************************************************/
/* CB_http : Gere les connexion HTTP pures (appellée par libwebsockets)                                                       */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 static gint CB_http ( struct lws *wsi, enum lws_callback_reasons tag, void *user, void *data, size_t taille )
  { gchar remote_name[80], remote_ip[80];

 /*   Http_Log_request(connection, url, method, version, upload_data_size, con_cls);*/
    switch (tag)
     { case LWS_CALLBACK_ESTABLISHED:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "CB_http: connexion established" );
		          break;
       case LWS_CALLBACK_CLOSED_HTTP:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "CB_http: connexion closed" );
		          break;
       case LWS_CALLBACK_PROTOCOL_INIT:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "CB_http: Protocol initialized !" );
            break;
       case LWS_CALLBACK_PROTOCOL_DESTROY:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "CB_http: Destroy protocol" );
		          break;
       case LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "CB_http: Need SSL Private key" );
		          break;
       case LWS_CALLBACK_RECEIVE:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "CB_http: data received" );
		          break;
       case LWS_CALLBACK_HTTP:
             { gchar *url = (gchar *)data;
               gint retour;
               lws_get_peer_addresses ( wsi, lws_get_socket_fd(wsi),
                                        (char *)&remote_name, sizeof(remote_name),
                                        (char *)&remote_ip, sizeof(remote_ip) );
               Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "CB_http: HTTP request from %s(%s): %s",
                         remote_name, remote_ip, url );
               if ( ! strcasecmp ( url, "/favicon.ico" ) )
                { retour = lws_serve_http_file ( wsi, "WEB/favicon.gif", "image/gif", NULL, 0);
                  if (retour != 0) return(1);                             /* Si erreur (<0) ou si ok (>0), on ferme la socket */
                  return(0);                    /* si besoin de plus de temps, on laisse la ws http ouverte pour libwebsocket */
                }
               else if ( ! strncasecmp ( url, "/ui/", 4 ) )
                { return( Http_Traiter_request_getui ( wsi, remote_name, remote_ip, url+4 ) ); }
               else if ( ! strcasecmp ( url, "/status" ) )
                { Http_Traiter_request_getstatus ( wsi ); }
               else if ( ! strncasecmp ( url, "/gif/", 5 ) )
                { return( Http_Traiter_request_getgif ( wsi, remote_name, remote_ip, url+5 ) ); }
               else if ( ! strncasecmp ( url, "/audio/", 7 ) )
                { return( Http_Traiter_request_getaudio ( wsi, remote_name, remote_ip, url+7 ) ); }
               return(1);                                                                    /* Par défaut, on clos la socket */
             }
		          break;
       case LWS_CALLBACK_HTTP_FILE_COMPLETION:
             { lws_get_peer_addresses ( wsi, lws_get_socket_fd(wsi),
                                        (char *)&remote_name, sizeof(remote_name),
                                        (char *)&remote_ip, sizeof(remote_ip) );
               Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                         "CB_http: file sent for %s(%s)", remote_name, remote_ip );
             }
            break;
       case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "CB_http: need to verifi Client SSL Certs" );
		          break;
	      default: return(0);                                                    /* Par défaut, on laisse la connexion continuer */
     }
   return(0);                                                                                           /* Continue connexion */
  }
/******************************************************************************************************************************/
/* Run_thread: Thread principal                                                                                               */
/* Entrée: une structure LIBRAIRIE                                                                                            */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct lws_protocols WS_PROTOS[] =
     { { "http-only", CB_http, 0, 0 }, 	                                        /* first protocol must always be HTTP handler */
       { "ws-status", CB_ws_status, 0, 0 },
       { NULL, NULL, 0, 0 } /* terminator */
     };
    struct stat sbuf;

    prctl(PR_SET_NAME, "W-HTTP", 0, 0, 0 );
    memset( &Cfg_http, 0, sizeof(Cfg_http) );                                       /* Mise a zero de la structure de travail */
    Cfg_http.lib = lib;                                            /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_http.lib->TID = pthread_self();                                                     /* Sauvegarde du TID pour le pere */
    Http_Lire_config ();                                                    /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );

    g_snprintf( Cfg_http.lib->admin_prompt, sizeof(Cfg_http.lib->admin_prompt), NOM_THREAD );
    g_snprintf( Cfg_http.lib->admin_help,   sizeof(Cfg_http.lib->admin_help),   "Manage Web Services with external Devices" );

    /*lws_set_log_level(debug_level, lwsl_emit_syslog);*/

    Cfg_http.ws_info.iface = NULL;                                                      /* Configuration du serveur Websocket */
	   Cfg_http.ws_info.protocols = WS_PROTOS;
    Cfg_http.ws_info.port = Cfg_http.tcp_port;
	   Cfg_http.ws_info.gid = -1;
	   Cfg_http.ws_info.uid = -1;
	   Cfg_http.ws_info.max_http_header_pool = Cfg_http.nbr_max_connexion;
	   Cfg_http.ws_info.options |= LWS_SERVER_OPTION_VALIDATE_UTF8;
	   Cfg_http.ws_info.extensions = NULL;
	   Cfg_http.ws_info.timeout_secs = 30;

    if (Cfg_http.ssl_enable)                                                                           /* Configuration SSL ? */
     { if ( stat ( Cfg_http.ssl_cert_filepath, &sbuf ) == -1)                                     /* Test présence du fichier */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "Run_thread: unable to load '%s' (error '%s'). Setting ssl=FALSE",
                    Cfg_http.ssl_cert_filepath, strerror(errno) );
          Cfg_http.ssl_enable=FALSE;
        }
       else if ( stat ( Cfg_http.ssl_private_key_filepath, &sbuf ) == -1)                         /* Test présence du fichier */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "Run_thread: unable to load '%s' (error '%s'). Setting ssl=FALSE",
                    Cfg_http.ssl_private_key_filepath, strerror(errno) );
          Cfg_http.ssl_enable=FALSE;
        }
       else if ( stat ( Cfg_http.ssl_ca_filepath, &sbuf ) == -1)                                  /* Test présence du fichier */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "Run_thread: unable to load '%s' (error '%s'). Setting ssl=FALSE",
                    Cfg_http.ssl_ca_filepath, strerror(errno) );
          Cfg_http.ssl_enable=FALSE;
        }
       else
        { Cfg_http.ws_info.ssl_cipher_list          = Cfg_http.ws_info.ssl_cipher_list;
   	      Cfg_http.ws_info.ssl_cert_filepath        = Cfg_http.ssl_cert_filepath;
	         Cfg_http.ws_info.ssl_ca_filepath          = Cfg_http.ssl_ca_filepath;
   	      Cfg_http.ws_info.ssl_private_key_filepath = Cfg_http.ssl_private_key_filepath;
          Cfg_http.ws_info.options |= LWS_SERVER_OPTION_PEER_CERT_NOT_REQUIRED;
          /*Cfg_http.ws_info.options |= LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS;*/
          /*Cfg_http.ws_info.options |= LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;*/
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                   "Run_thread: Stat '%s' OK", Cfg_http.ws_info.ssl_cert_filepath );
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                   "Run_thread: Stat '%s' OK", Cfg_http.ws_info.ssl_private_key_filepath );
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                   "Run_thread: Stat '%s' OK", Cfg_http.ws_info.ssl_ca_filepath );
        }
     }

	   Cfg_http.ws_context = lws_create_context(&Cfg_http.ws_info);
 
    if (!Cfg_http.ws_context)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Run_thread: WebSocket Create Context creation error (%s). Shutting Down %p",
                 strerror(errno), pthread_self() );
       goto end;
     }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
             "Run_thread: WebSocket Create OK. Listening on port %d with ssl=%d", Cfg_http.tcp_port, Cfg_http.ssl_enable );

#ifdef bouh
    Abonner_distribution_message ( Http_Gerer_message );                            /* Abonnement à la diffusion des messages */
    Abonner_distribution_sortie  ( Http_Gerer_sortie );                              /* Abonnement à la diffusion des sorties */

#endif
    Cfg_http.lib->Thread_run = TRUE;                                                                    /* Le thread tourne ! */
    while(Cfg_http.lib->Thread_run == TRUE)                                                  /* On tourne tant que necessaire */
     { /*static gint last_top = 0;*/
       usleep(10000);
       sched_yield();

       if (Cfg_http.lib->Thread_sigusr1)                                  /* A-t'on recu un signal USR1 ? */
        { pthread_mutex_lock( &Cfg_http.lib->synchro );                  /* Ajout dans la liste a traiter */
          pthread_mutex_unlock( &Cfg_http.lib->synchro );
          /*Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                   "Run_thread: SIGUSR1. %03d sessions", nbr );*/
          Cfg_http.lib->Thread_sigusr1 = FALSE;
        }

   	   lws_service( Cfg_http.ws_context, 1000);                                 /* On lance l'écoute des connexions websocket */

#ifdef bouh
        if ( last_top + 300 <= Partage->top )                                    /* Toutes les 30 secondes */
        { Http_Check_sessions ();
          last_top = Partage->top;
        }
#endif
     }

    lws_context_destroy(Cfg_http.ws_context);                                                   /* Arret du serveur WebSocket */
    Cfg_http.ws_context = NULL;

end:
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
             "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_http.lib->Thread_run = FALSE;                                                           /* Le thread ne tourne plus ! */
    Cfg_http.lib->TID = 0;                                                    /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
