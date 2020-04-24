/******************************************************************************************************************************/
/* Watchdogd/Http/Http.c        Gestion des connexions HTTP WebService de watchdog                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Http.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"
 #include "Http.h"
 struct HTTP_CONFIG Cfg_http;
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
    g_snprintf( Cfg_http.ssl_cert_filepath,        sizeof(Cfg_http.ssl_cert_filepath), "%s", HTTP_DEFAUT_FILE_CERT );
    g_snprintf( Cfg_http.ssl_private_key_filepath, sizeof(Cfg_http.ssl_private_key_filepath), "%s", HTTP_DEFAUT_FILE_KEY );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     {      if ( ! g_ascii_strcasecmp ( nom, "ssl_file_cert" ) )
        { g_snprintf( Cfg_http.ssl_cert_filepath, sizeof(Cfg_http.ssl_cert_filepath), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_file_key" ) )
        { g_snprintf( Cfg_http.ssl_private_key_filepath, sizeof(Cfg_http.ssl_private_key_filepath), "%s", valeur ); }
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
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Http_send_histo : envoie un histo au client                                                                                */
/* Entrée : la connexion client WebSocket et l'histo a envoyer                                                                */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
#ifdef bouh
 static void WS_send_histo ( struct lws *wsi, struct CMD_TYPE_HISTO *histo )
  { struct WS_PER_SESSION_DATA *pss;
    gchar *buf;
    JsonBuilder *builder;
    gsize taille_buf;

    pss = lws_wsi_user ( wsi );
    builder = Json_create ();
    if (!builder) return;

    json_builder_begin_object (builder);                                                                  /* Contenu du Histo */
    Json_add_bool ( builder, "alive", histo->alive );
    Json_add_string ( builder, "date_create",   histo->date_create );
    Json_add_string ( builder, "nom_ack",       histo->nom_ack );
    Json_add_string ( builder, "libelle",       histo->msg.libelle );
    Json_add_string ( builder, "syn_groupe",    histo->msg.syn_parent_page );
    Json_add_string ( builder, "syn_page",      histo->msg.syn_page );
    Json_add_string ( builder, "syn_libelle",   histo->msg.syn_libelle );
    Json_add_string ( builder, "dls_shortname", histo->msg.dls_shortname );
    json_builder_end_object (builder);                                                                           /* End Histo */

    buf = Json_get_buf ( builder, &taille_buf );
#ifdef bouh
    buf_to_send = g_try_malloc0( taille_buf + LWS_PRE );
    if (buf_to_send)
     { memcpy( buf_to_send + LWS_PRE, buf, taille_buf );
       lws_write(wsi, &buf_to_send[LWS_PRE], taille_buf, LWS_WRITE_TEXT );
       g_free(buf_to_send);                                           /* Libération du buffer dont nous n'avons plus besoin ! */
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
              "%s: send %d byte to '%s' ('%s')", __func__, taille_buf, pss->sid, pss->util );
#endif
    g_free(buf);
  }
#endif
/******************************************************************************************************************************/
/* Rechercher_utilisateurDB_by_sid: Recuperation de tous les champs de l'utilisateur dont le sid est en parametre             */
/* Entrées: le Session ID                                                                                                     */
/* Sortie: une structure utilisateur, ou null si erreur                                                                       */
/******************************************************************************************************************************/
 struct DB *Rechercher_utilisateurDB_by_sid( gchar *sid_brut )
  { gchar requete[512], *sid;
    struct DB *db;

    sid = Normaliser_chaine ( sid_brut );                                                    /* Formatage correct des chaines */
    if (!sid)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT username,id,access_level,enable "
                "FROM users WHERE session_id='%s' LIMIT 1", sid );
    g_free(sid);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed for sid '%s'", __func__, sid_brut );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }
    return(db);
  }
#ifdef bouh

/******************************************************************************************************************************/
/* CB_ws_histos : Gere le protocole WS histos (appellée par libwebsockets)                                                    */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 static gint CB_ws_live_motifs ( struct lws *wsi, enum lws_callback_reasons tag, void *user, void *data, size_t taille )
  { struct WS_PER_SESSION_DATA *pss;
    gchar buffer[256];
    struct DB*db;
    pss = lws_wsi_user ( wsi );
/*    gchar *util;*/
    switch (tag)
     { case LWS_CALLBACK_ESTABLISHED:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: Checking cookie", __func__ );
            if ( lws_hdr_total_length( wsi, WSI_TOKEN_HTTP_COOKIE ) <= 0)
             { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: No Cookie found", __func__ );
               return(1);
             }
            lws_callback_on_writable(wsi);
            if ( lws_hdr_copy( wsi, buffer, sizeof(buffer), WSI_TOKEN_HTTP_COOKIE ) != -1 )/* Récupération de la valeur du token */
             { gchar *cookies, *cookie, *savecookies;
               gchar *cookie_name, *cookie_value, *savecookie;
               cookies = buffer;
               while ( (cookie=strtok_r( cookies, ";", &savecookies)) != NULL )                          /* Découpage par ';' */
                { cookies=NULL;
                  cookie_name=strtok_r( cookie, "=", &savecookie);                                       /* Découpage par "=" */
                  if (cookie_name)
                   { cookie_value = strtok_r ( NULL, "=", &savecookie );
                     if (!strcasecmp(cookie_name, "ci_session"))
                      { g_snprintf( pss->sid, sizeof(pss->sid), "%s", cookie_value ); }
                   }
                  Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                           "%s: Cookie found for: %s=%s", __func__,
                           (cookie_name ? cookie_name : "none"), (cookie_value ? cookie_value : "none") );
                }
             }
            db = Rechercher_utilisateurDB_by_sid ( pss->sid );
            if (!db)
             { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: No user found for session %s.", __func__, pss->sid );
               return(1);
             }
            g_snprintf( pss->username, sizeof(pss->username), "%s", db->row[0] );
            pss->user_id     = atoi(db->row[1]);
            pss->user_level  = atoi(db->row[2]);
            pss->user_enable = atoi(db->row[3]);
            Libere_DB_SQL ( &db );
            if (!pss->user_enable)
             { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                         "%s: %s: user found but disabled for session %s.", __func__, pss->username, pss->sid );
               return(1);
             }

            pss->zmq = Connect_zmq ( ZMQ_SUB, "listen-to-motifs", "inproc", ZMQUEUE_LIVE_MOTIFS, 0 );
            /*pss->zmq_local_bus = Connect_zmq ( ZMQ_SUB, "listen-to-bus",   "inproc", ZMQUEUE_LOCAL_BUS, 0 );*/
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: %s: WS callback established", __func__, pss->username );
            break;
       case LWS_CALLBACK_RECEIVE:
             { gchar *buffer;
               buffer = g_try_malloc0(taille+1);
               if (!buffer)
                { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: %s: Malloc Error %s.", __func__, pss->username );
                  return(1);
                }
               memcpy ( buffer, data, taille );
               Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "%s: WS callback receive %d bytes: %s", __func__, taille, buffer );
               g_free(buffer);
               lws_callback_on_writable(wsi);
             }
            break;
       case LWS_CALLBACK_CLOSED:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: %s: WS callback closed", __func__, pss->username );
            if (pss->zmq) Close_zmq(pss->zmq);
            /*if (pss->zmq_local_bus) Close_zmq(pss->zmq_local_bus);*/
            break;
       case LWS_CALLBACK_SERVER_WRITEABLE:
             { struct DLS_VISUEL visu;
               gchar json_buffer[2048];
               gint taille_buf;
               if ( pss->zmq && Recv_zmq ( pss->zmq, &visu, sizeof(struct DLS_VISUEL) ) == sizeof(struct DLS_VISUEL) )
                { JsonBuilder *builder;
                  gsize taille_buf;
                  gchar *buf, *result;
                  Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                            "%s: %s: Visuel %s:%s received", __func__, pss->username, visu.tech_id, visu.acronyme );
                  builder = Json_create ();
                  if (builder == NULL)
                   { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: JSon builder creation failed", __func__ );
                     return(1);
                   }
                  json_builder_begin_object (builder);                                         /* Création du noeud principal */
                  Json_add_string ( builder, "tech_id",  visu.tech_id );
                  Json_add_string ( builder, "acronyme", visu.acronyme );
                  Json_add_int    ( builder, "mode",     visu.mode );
                  Json_add_string ( builder, "color",    visu.color );
                  Json_add_bool   ( builder, "cligno",   visu.cligno );
                  json_builder_end_object (builder);                                                          /* End Document */
                  buf = Json_get_buf ( builder, &taille_buf );
                  result = (gchar *)g_malloc(LWS_SEND_BUFFER_PRE_PADDING + taille_buf);
                  if (result == NULL)
                   { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                               "%s: %s: JSon Result creation failed", __func__, pss->username );
                     g_free(buf);
                     return(1);
                   }
                  memcpy ( result + LWS_SEND_BUFFER_PRE_PADDING , buf, taille_buf );
                  g_free(buf);
                  lws_write	(	wsi,	result+LWS_SEND_BUFFER_PRE_PADDING, taille_buf, LWS_WRITE_TEXT );
                  g_free(result);
                }
               /*else if ( pss->zmq_local_bus && (taille_buf = Recv_zmq ( pss->zmq_local_bus, &json_buffer, sizeof(json_buffer) )) > 0 )
                { gchar *result = (gchar *)g_malloc(LWS_SEND_BUFFER_PRE_PADDING + taille_buf);
                  if (result == NULL)
                   { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon Result creation failed", __func__ );
                     return(1);
                   }
                  memcpy ( result + LWS_SEND_BUFFER_PRE_PADDING , json_buffer, taille_buf );
                  Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                            "%s: %s: bus %s received", __func__, pss->username, json_buffer );
                  lws_write	(	wsi,	result+LWS_SEND_BUFFER_PRE_PADDING, taille_buf, LWS_WRITE_TEXT );
                  g_free(result);
                }*/
             }
            lws_callback_on_writable(wsi);
            break;
       default: return(0);
     }
    return(0);
  }
/******************************************************************************************************************************/
/* CB_ws_histos : Gere le protocole WS histos (appellée par libwebsockets)                                                    */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 static gint CB_ws_histos ( struct lws *wsi, enum lws_callback_reasons tag, void *user, void *data, size_t taille )
  {
    struct WS_PER_SESSION_DATA *pss;
  /*  gchar *util;*/
    pss = lws_wsi_user ( wsi );
    switch (tag)
     { case LWS_CALLBACK_ESTABLISHED: lws_callback_on_writable(wsi);
/*            if (Get_phpsessionid_cookie(wsi)==FALSE)                                              /* Recupere le PHPSessionID */
/*             { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: No PHPSESSID. Killing.", __func__ );
               return(1);
             }
/*            util = Rechercher_util_by_phpsessionid ( pss->sid );*/
/*            if (!util)
             { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: No user found for session %s.", __func__, pss->sid );
               return(1);
             }
            g_snprintf( pss->util, sizeof(pss->util), "%s", util );
            g_free(util);

            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: WS callback established for %s", __func__, pss->util );*/
            pss->zmq = Connect_zmq ( ZMQ_SUB, "listen-to-msgs",   "inproc", ZMQUEUE_LIVE_MSGS, 0 );
            break;
       case LWS_CALLBACK_CLOSED:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: WS callback closed", __func__ );
            if (pss->zmq) Close_zmq(pss->zmq);
            break;
       case LWS_CALLBACK_SERVER_WRITEABLE:
             { struct CMD_TYPE_HISTO histo_buf;
               if ( pss->zmq && Recv_zmq ( pss->zmq, &histo_buf, sizeof(struct CMD_TYPE_HISTO) ) == sizeof(struct CMD_TYPE_HISTO) )
                { WS_send_histo ( wsi, &histo_buf );
                }
             }
            lws_callback_on_writable(wsi);
            break;
       default: return(0);
     }
    return(0);
  }
#endif
/******************************************************************************************************************************/
/* Http_traiter_log: Répond aux requetes sur l'URI log                                                                        */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_log ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data)
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    Http_print_request ( server, msg, path, client );
	   soup_message_set_status (msg, SOUP_STATUS_OK);
         if ( ! strcasecmp ( path, "/log/debug"   ) ) { Info_change_log_level ( Config.log, LOG_DEBUG   ); }
    else if ( ! strcasecmp ( path, "/log/notice"  ) ) { Info_change_log_level ( Config.log, LOG_NOTICE  ); }
    else if ( ! strcasecmp ( path, "/log/info"    ) ) { Info_change_log_level ( Config.log, LOG_INFO    ); }
    else if ( ! strcasecmp ( path, "/log/warning" ) ) { Info_change_log_level ( Config.log, LOG_WARNING ); }
    else if ( ! strcasecmp ( path, "/log/error"   ) ) { Info_change_log_level ( Config.log, LOG_ERR     ); }
	   else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
    /*soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_STATIC, "LogLevel set", 18 );*/
  }
/******************************************************************************************************************************/
/* Http_traiter_connect: Répond aux requetes sur l'URI connect                                                                */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_connect ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data)
  {
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_STATIC, "ceci est un test !!", 18 );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Gatcha !" );
  }
/******************************************************************************************************************************/
/* Http_print_request: affiche les données relatives à une requete                                                            */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_print_request ( SoupServer *server, SoupMessage *msg, const char *path, SoupClientContext *client )
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: %s from %s@%s", __func__,
              path, soup_client_context_get_auth_user (client), soup_client_context_get_host(client) );
  }
/******************************************************************************************************************************/
/* Run_thread: Thread principal                                                                                               */
/* Entrée: une structure LIBRAIRIE                                                                                            */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct stat sbuf;

    prctl(PR_SET_NAME, "W-HTTP", 0, 0, 0 );
reload:
    memset( &Cfg_http, 0, sizeof(Cfg_http) );                                       /* Mise a zero de la structure de travail */
    Cfg_http.lib = lib;                                            /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_http.lib->TID = pthread_self();                                                     /* Sauvegarde du TID pour le pere */
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );
    Http_Lire_config ();                                                    /* Lecture de la configuration logiciel du thread */

    g_snprintf( Cfg_http.lib->admin_prompt, sizeof(Cfg_http.lib->admin_prompt), NOM_THREAD );
    g_snprintf( Cfg_http.lib->admin_help,   sizeof(Cfg_http.lib->admin_help),   "Manage Web Services with external Devices" );

    /*Cfg_http.zmq_from_bus = New_zmq ( ZMQ_SUB, "listen-to-bus" );
    Connect_zmq ( Cfg_http.zmq_from_bus, "inproc", ZMQUEUE_LOCAL_BUS, 0 );*/

    SoupServer *socket = soup_server_new("server-header", "Watchdogd HTTP Server", NULL);
    if (!socket)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: SoupServer new Failed !", __func__ );
       goto end;
     }
    soup_server_add_handler (socket, "/connect", Http_traiter_connect, NULL, NULL );
    soup_server_add_handler (socket, "/dls",     Http_traiter_dls, NULL, NULL );
    soup_server_add_handler (socket, "/process", Http_traiter_process, NULL, NULL );
    soup_server_add_handler (socket, "/status",  Http_traiter_status, NULL, NULL );
    soup_server_add_handler (socket, "/log",     Http_traiter_log, NULL, NULL );
    soup_server_add_handler (socket, "/bus",     Http_traiter_bus, NULL, NULL );
    soup_server_add_handler (socket, "/memory",  Http_traiter_memory, NULL, NULL );
/*
SoupAuthDomain *domain;

domain = soup_auth_domain_basic_new (
	SOUP_AUTH_DOMAIN_REALM, "My Realm",
	SOUP_AUTH_DOMAIN_BASIC_AUTH_CALLBACK, auth_callback,
	SOUP_AUTH_DOMAIN_BASIC_AUTH_DATA, auth_data,
	SOUP_AUTH_DOMAIN_ADD_PATH, "/foo",
	SOUP_AUTH_DOMAIN_ADD_PATH, "/bar/private",
	NULL);
soup_server_add_auth_domain (server, domain);
g_object_unref (domain);
*/
    if (Cfg_http.ssl_enable)                                                                           /* Configuration SSL ? */
     { if ( stat ( Cfg_http.ssl_cert_filepath, &sbuf ) == -1)                                     /* Test présence du fichier */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "%s: unable to load '%s' (error '%s'). Setting ssl=FALSE", __func__,
                    Cfg_http.ssl_cert_filepath, strerror(errno) );
          Cfg_http.ssl_enable=FALSE;
        }
       else if ( stat ( Cfg_http.ssl_private_key_filepath, &sbuf ) == -1)                         /* Test présence du fichier */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "%s: unable to load '%s' (error '%s'). Setting ssl=FALSE", __func__,
                    Cfg_http.ssl_private_key_filepath, strerror(errno) );
          Cfg_http.ssl_enable=FALSE;
        }
       else
        { if (!soup_server_set_ssl_cert_file ( socket, Cfg_http.ssl_cert_filepath, Cfg_http.ssl_private_key_filepath, NULL ))
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: Failed to load SSL Certificate", __func__ );
             Cfg_http.ssl_enable=FALSE;
           }
        }
     }

    if (!soup_server_listen_all (socket, Cfg_http.tcp_port, 0, NULL))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: SoupServer Listen Failed !", __func__ );
       goto end;
     }
    else
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                 "%s: HTTP SoupServer Listen on port %d OK !", __func__, Cfg_http.tcp_port );
     }
    GMainLoop *loop = g_main_loop_new (NULL, TRUE);
    GMainContext *loop_context = g_main_loop_get_context ( loop );

    Cfg_http.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );
    Cfg_http.lib->Thread_run = TRUE;                                                                    /* Le thread tourne ! */
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { usleep(1000);
       sched_yield();

       if (Cfg_http.lib->Thread_reload)                                                      /* A-t'on recu un signal USR1 ? */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Thread Reload !", __func__ );
          break;
        }

       g_main_context_iteration ( loop_context, FALSE );
     }

    soup_server_disconnect (socket);                                                            /* Arret du serveur WebSocket */
    /*Close_zmq ( Cfg_http.zmq_from_bus );*/
    Close_zmq ( Cfg_http.zmq_to_master );
end:
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    if (lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Cfg_http.lib->Thread_run = FALSE;                                                           /* Le thread ne tourne plus ! */
    Cfg_http.lib->TID = 0;                                                    /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
