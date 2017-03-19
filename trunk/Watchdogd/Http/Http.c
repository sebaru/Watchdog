/******************************************************************************************************************************/
/* Watchdogd/Http/Http.c        Gestion des connexions HTTP WebService de watchdog                                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
    Cfg_http.max_upload_bytes  = HTTP_DEFAUT_MAX_UPLOAD_BYTES;
    Cfg_http.lws_debug_level   = HTTP_DEFAUT_LWS_DEBUG_LEVEL;
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
       else if ( ! g_ascii_strcasecmp ( nom, "max_upload_bytes" ) )
        { Cfg_http.max_upload_bytes = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "lws_debug_level" ) )
        { Cfg_http.lws_debug_level = atoi(valeur);  }
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
/* Http_json_get_int : Récupère un entier dans l'object JSON passé en paramètre, dont le nom est name                         */
/* Entrées : l'object JSON et le name                                                                                         */
/* Sortie : un entier, ou -1 si erreur                                                                                        */
/******************************************************************************************************************************/
 gint Http_json_get_int ( JsonObject *object, gchar *name )
  { JsonNode *node;
    GValue valeur = G_VALUE_INIT;

    if (!object) return(-1);
    node = json_object_get_member(object, name );
    if(!node) return(-1);
    if(json_node_get_node_type (node) != JSON_NODE_VALUE) return(-1);
    json_node_get_value (node, &valeur);
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: Parsing value type %d ('%s') for attribut '%s'", __func__, G_VALUE_TYPE(&valeur), G_VALUE_TYPE_NAME(&valeur), name );
    switch( G_VALUE_TYPE(&valeur) )             
     { case G_TYPE_BOOLEAN:
            return( json_node_get_boolean(node) );
       case G_TYPE_INT64:
       case G_TYPE_INT:
            return( json_node_get_int(node) );
       case G_TYPE_STRING:
            return (atoi ( json_node_get_string(node) ));
       default:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                     "%s: Valeur type unknown (%d) for name '%s'", __func__, json_node_get_value_type (node), name );
     }
    return(-1);
  }
/******************************************************************************************************************************/
/* Http_Send_error_code: Utiliser pour renvoyer un code d'erreur                                                              */
/* Entrée: La structure wsi de reference                                                                                      */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_Send_error_code ( struct lws *wsi, gint code )
  { unsigned char header[256], *header_cur, *header_end;
   	struct HTTP_PER_SESSION_DATA *pss;
    gint retour;

    pss = lws_wsi_user ( wsi );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
             "%s: (sid %.12s) Sending Error code '%d' for '%s'", __func__, Http_get_session_id(pss->session), code,
             (pss->session ? (pss->session->util ? pss->session->util->nom : "--no user--") : "--no session--")
            );

    header_cur = header;
    header_end = header + sizeof(header);

    retour = lws_add_http_header_status( wsi, code, &header_cur, header_end );
    retour = lws_finalize_http_header ( wsi, &header_cur, header_end );
   *header_cur='\0';                                                                                /* Caractere null d'arret */
    lws_write( wsi, header, header_cur - header, LWS_WRITE_HTTP_HEADERS );
  }
/******************************************************************************************************************************/
/* CB_ws_login : Gere le protocole WS status (appellée par libwebsockets)                                                    */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 static gint CB_ws_login ( struct lws *wsi, enum lws_callback_reasons tag, void *user, void *data, size_t taille )
  { return(1);
  }
/******************************************************************************************************************************/
/* Http_CB_file_upload : Récupère les data de la requete POST en cours                                                        */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 static gint Http_CB_file_upload( struct lws *wsi, char *buffer, int taille )
  {	struct HTTP_PER_SESSION_DATA *pss;
    pss = lws_wsi_user ( wsi );
    
    if (pss->post_data_length >= Cfg_http.max_upload_bytes)                  /* Si taille de fichier trop importante, on vire */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_CB_file_upload: (sid %.12s) file too long (%d/%d), aborting",
                 Http_get_session_id(pss->session), pss->post_data_length, Cfg_http.max_upload_bytes );
       return(1);
     }

    pss->post_data = g_try_realloc ( pss->post_data, pss->post_data_length + taille );
    memcpy ( pss->post_data + pss->post_data_length, buffer, taille );
    pss->post_data_length += taille;

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "Http_CB_file_upload: (sid %.12s) received %d bytes (total length=%d, max %d)",
              Http_get_session_id(pss->session), taille, pss->post_data_length, Cfg_http.max_upload_bytes );
    return 0;
  }
/******************************************************************************************************************************/
/* CB_http : Gere les connexion HTTP pures (appellée par libwebsockets)                                                       */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 static gint CB_http ( struct lws *wsi, enum lws_callback_reasons tag, void *user, void *data, size_t taille )
  { gchar remote_name[80], remote_ip[80];
    struct HTTP_PER_SESSION_DATA *pss = (struct HTTP_PER_SESSION_DATA *)user;

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
       case LWS_CALLBACK_HTTP_BODY:
             { if ( ! strcasecmp ( pss->url, "/ws/login" ) )                               /* si OK, on poursuit la connexion */
                { return( Http_Traiter_request_body_login ( wsi, data, taille ) ); }
               else if ( ! strcasecmp ( pss->url, "/ws/postsvg" ) )
                { return( Http_CB_file_upload( wsi, data, taille ) ); }
               else if ( ! strcasecmp ( pss->url, "/ws/setmessage" ) )
                { return( Http_CB_file_upload( wsi, data, taille ) ); }
             }
            break;
       case LWS_CALLBACK_HTTP_BODY_COMPLETION:
             { lws_get_peer_addresses ( wsi, lws_get_socket_fd(wsi),
                                           (char *)&remote_name, sizeof(remote_name),
                                           (char *)&remote_ip, sizeof(remote_ip) );
               if ( ! strcasecmp ( pss->url, "/ws/login" ) )                               /* si OK, on poursuit la connexion */
                { return( Http_Traiter_request_body_completion_login ( wsi, remote_name, remote_ip ) ); }
               else if ( ! strcasecmp ( pss->url, "/ws/postsvg" ) )
                { return( Http_Traiter_request_body_completion_postsvg ( wsi ) ); }
               else if ( ! strcasecmp ( pss->url, "/ws/setmessage" ) )
                { return( Http_Traiter_request_body_completion_setmessage ( wsi ) ); }
               else if ( ! strcasecmp ( pss->url, "/ws/delmessage" ) )
                { return( Http_Traiter_request_body_completion_delmessage ( wsi ) ); }
              }
            break;
       case LWS_CALLBACK_HTTP:
             { struct HTTP_SESSION *session;
               gchar *url = (gchar *)data;
               gint retour;
               lws_get_peer_addresses ( wsi, lws_get_socket_fd(wsi),
                                        (char *)&remote_name, sizeof(remote_name),
                                        (char *)&remote_ip, sizeof(remote_ip) );
               session = Http_get_session ( wsi, remote_name, remote_ip );
               Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: Request from %s/%s (sid %.12s): %s",
                         __func__, remote_name, remote_ip, Http_get_session_id(session), url );
               if (session) session->last_top = Partage->top;                                             /* Tagging temporel */

               if ( ! strcasecmp ( url, "/favicon.ico" ) )
                { retour = lws_serve_http_file ( wsi, "WEB/favicon.gif", "image/gif", NULL, 0);
                  if (retour != 0) return(1);                             /* Si erreur (<0) ou si ok (>0), on ferme la socket */
                  return(0);                    /* si besoin de plus de temps, on laisse la ws http ouverte pour libwebsocket */
                }

               else if ( ! strcasecmp ( url, "/ws/login" ) )                               /* si OK, on poursuit la connexion */
                { return( Http_Traiter_request_login ( session, wsi, remote_name, remote_ip ) ); }
               else if ( ! strcasecmp ( url, "/ws/logoff" ) )
                { if (session) Http_Close_session ( wsi, session ); }
               else if ( ! strcasecmp ( url, "/ws/status" ) )
                { Http_Traiter_request_getstatus ( wsi ); }
               else if ( ! strcasecmp ( url, "/ws/messages" ) )
                { return( Http_Traiter_request_getmessage ( wsi, session ) ); }
               else if ( ! strcasecmp ( url, "/ws/setmessage" ) )
                { return( Http_Traiter_request_setmessage ( wsi, session, remote_name, remote_ip ) ); }
               else if ( ! strcasecmp ( url, "/ws/delmessage" ) )
                { return( Http_Traiter_request_delmessage ( wsi, session ) ); }
               else if ( ! strcasecmp ( url, "/ws/getpluginsDLS" ) )
                { return( Http_Traiter_request_getpluginsDLS ( wsi, session ) ); }
               else if ( ! strncasecmp ( url, "/ws/getmnemo", 13 ) )
                { return( Http_Traiter_request_getmnemo ( wsi, session, url+13 ) ); }
               else if ( ! strncasecmp ( url, "/ws/getsyn", 11 ) )
                { return( Http_Traiter_request_getsyn ( wsi, session ) ); }
               else if ( ! strcasecmp ( url, "/ws/getsvg" ) )
                { return( Http_Traiter_request_getsvg ( wsi, session ) ); }
               else if ( ! strncasecmp ( url, "/ws/gif/", 8 ) )
                { return( Http_Traiter_request_getgif ( wsi, remote_name, remote_ip, url+8 ) ); }
               else if ( ! strncasecmp ( url, "/ws/audio/", 10 ) )
                { return( Http_Traiter_request_getaudio ( wsi, remote_name, remote_ip, url+10 ) ); }
               else if ( ! strcasecmp ( url, "/ws/postsvg" ) )
                { return( Http_Traiter_request_postsvg ( wsi, session, remote_name, remote_ip ) ); }
               else                                                                                             /* Par défaut */
                { return( Http_Traiter_request_getui ( wsi, remote_name, remote_ip, url+1 ) ); }
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
                      "CB_http: need to verify Client SSL Certs" );
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
     { { "http-only", CB_http, sizeof(struct HTTP_PER_SESSION_DATA), 0 },       /* first protocol must always be HTTP handler */
       { "ws-login", CB_ws_login, 0, 0 },
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

    lws_set_log_level(Cfg_http.lws_debug_level, lwsl_emit_syslog);

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
     { static gint last_top = 0;
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

       if ( last_top + 600 <= Partage->top )                                                            /* Toutes les minutes */
        { Http_Check_sessions ();
          last_top = Partage->top;
        }
     }

    while ( Cfg_http.Liste_sessions ) Http_Liberer_session ( Cfg_http.Liste_sessions->data );     /* Libérations des sessions */
    
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
