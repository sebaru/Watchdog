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
 #include <crypt.h>

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

    Cfg_http.lib->Thread_debug  = FALSE;                                                        /* Settings default parameters */
    Cfg_http.tcp_port           = HTTP_DEFAUT_TCP_PORT;
    Cfg_http.ssl_enable         = FALSE;
    Cfg_http.wtd_session_expiry = 3600*2;
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
       else if ( ! g_ascii_strcasecmp ( nom, "wtd_session_expiry" ) )
        { Cfg_http.wtd_session_expiry = atoi(valeur); }
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
/* Http_traiter_log: Répond aux requetes sur l'URI log                                                                        */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_log_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data)
  { gchar *buf, chaine[256];
    JsonBuilder *builder;
    gsize taille_buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!session)
     { soup_message_set_status (msg, SOUP_STATUS_FORBIDDEN);
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
    g_snprintf( chaine, sizeof(chaine), "SELECT * FROM audit_log WHERE access_level<=%d LIMIT 2000", session->access_level );
    SQL_Select_to_JSON ( builder, "logs", chaine );

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
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
/* Check_utilisateur_password: Vérifie le mot de passe fourni                                                                 */
/* Entrées: une structure util, un code confidentiel                                                                          */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Http_check_utilisateur_password( gchar *hash, gchar *pwd )
  { struct crypt_data *Data;
    gboolean retour=FALSE;
    gchar *result;

    Data = (struct crypt_data *)g_try_malloc0(sizeof(struct crypt_data));
    if( !Data)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: Memory Error", __func__ );
       return(FALSE);
     }
    result = crypt_r( pwd, hash, Data);
    retour = memcmp( result, hash, strlen( hash ) );                                                  /* Comparaison des hash */
    g_free(Data);
    if (retour==0) return(TRUE);
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Http_rechercher_session: Recherche une session dans la liste des session                                                   */
/* Entrée: l'uuid associé a la session                                                                                        */
/* Sortie: la session, ou NULL si pas trouvé                                                                                  */
/******************************************************************************************************************************/
 struct HTTP_CLIENT_SESSION *Http_rechercher_session_by_msg ( SoupMessage *msg )
  { struct HTTP_CLIENT_SESSION *result = NULL;
    GSList *cookies, *liste;
    cookies = soup_cookies_from_request(msg);
    liste = cookies;
    while ( liste )
     { SoupCookie *cookie = liste->data;
       const char *name = soup_cookie_get_name (cookie);
       if (!strcmp(name,"wtd_session"))
        { gchar *wtd_session = soup_cookie_get_value(cookie);
          GSList *clients = Cfg_http.liste_http_clients;
          while(clients)
           { struct HTTP_CLIENT_SESSION *session = clients->data;
             if (!strcmp(session->wtd_session, wtd_session))
              { result = session;
                break;
              }
             clients = g_slist_next ( clients );
           }
        }
       if (result) break;
       liste = g_slist_next(liste);
     }
    soup_cookies_free(cookies);
    return(result);
  }
/******************************************************************************************************************************/
/* Http_test_session_CB: Test si une requete doit etre authentifiée, ou si elle l'est déjà au travers de cookie de session    */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: TRUE si la requete DOIT etre authentifiée                                                                          */
/******************************************************************************************************************************/
 static gboolean Http_test_session_CB (SoupAuthDomain *domain, SoupMessage *msg, gpointer user_data)
  { if (Http_rechercher_session_by_msg ( msg )) return(FALSE);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Http_traiter_connect: Répond aux requetes sur l'URI connect                                                                */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Http_authenticate_CB ( SoupAuthDomain *domain, SoupMessage *msg, const char *username, const char *password,
                                        gpointer user_data )
  { gchar requete[256], *name;

    name = Normaliser_chaine ( username );                                                   /* Formatage correct des chaines */
    if (!name)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT username,comment,enable,hash "
                "FROM users WHERE username='%s' LIMIT 1", name );
    g_free(name);

    struct DB *db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: DB connexion failed for user '%s'", __func__, username );
       return(FALSE);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: DB request failed for user '%s'", __func__, username );
       return(FALSE);
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: User '%s' not found in DB", __func__, username );
       return(FALSE);
     }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: User '%s' (%s) found in database.",
              __func__, db->row[0], db->row[1] );

/*********************************************************** Compte du client *************************************************/
    if (atoi(db->row[2]) != 1)                                                 /* Est-ce que son compte est toujours actif ?? */
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: User '%s' not enabled", __func__, username );
       return(FALSE);
     }
/*********************************************** Authentification du client par login mot de passe ****************************/
    if ( Http_check_utilisateur_password( db->row[3], password ) == FALSE )                          /* Comparaison des codes */
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: Password error for '%s'(id=%d)", __func__, username );
       return(FALSE);
     }
    Liberer_resultat_SQL (db);
    Libere_DB_SQL( &db );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Http_print_request: affiche les données relatives à une requete                                                            */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 struct HTTP_CLIENT_SESSION *Http_print_request ( SoupServer *server, SoupMessage *msg, const char *path, SoupClientContext *client )
  { struct HTTP_CLIENT_SESSION *session = Http_rechercher_session_by_msg ( msg );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: sid '%s' (%s@%s, Level %d) : '%s'", __func__,
              (session ? session->wtd_session : "none"),
              (session ? session->username : soup_client_context_get_auth_user (client)), soup_client_context_get_host(client),
              (session ? session->access_level : -1), path
               );
    return(session);
  }
/******************************************************************************************************************************/
/* Http_traiter_disconnect: Répond aux requetes sur l'URI disconnect                                                          */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_disconnect ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }
    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (session)
     { Cfg_http.liste_http_clients = g_slist_remove ( Cfg_http.liste_http_clients, session );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: Session '%s' disconnected", __func__, session->wtd_session );
       g_free(session);
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: '%d' session left", __func__, g_slist_length(Cfg_http.liste_http_clients) );
     }
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_traiter_connect: Répond aux requetes sur l'URI connect                                                                */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_connect ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { gchar requete[256], *name;
    JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    Http_print_request ( server, msg, path, client );

    name = Normaliser_chaine ( soup_client_context_get_auth_user(client) );                  /* Formatage correct des chaines */
    if (!name)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT username,comment,access_level FROM users WHERE username='%s' LIMIT 1", name );
    g_free(name);

    struct DB *db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: DB connexion failed for user '%s'", __func__, soup_client_context_get_auth_user(client) );
       return;
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: DB request failed for user '%s'",__func__, soup_client_context_get_auth_user(client) );
       return;
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                "%s: User '%s' not found in DB", __func__, soup_client_context_get_auth_user(client) );
       return;
     }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: User '%s' (%s) found in database.",
              __func__, db->row[0], db->row[1] );

    struct HTTP_CLIENT_SESSION *session = g_try_malloc0( sizeof(struct HTTP_CLIENT_SESSION) );
    if (!session)
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Session creation Error", __func__ );
       return;
     }

    g_snprintf( session->username, sizeof(session->username), "%s", soup_client_context_get_auth_user(client) );
    session->access_level = atoi(db->row[2]);
    Liberer_resultat_SQL (db);
    Libere_DB_SQL( &db );

    session->last_request = Partage->top;
    uuid_t uuid_hex;
    uuid_generate(uuid_hex);
    uuid_unparse_lower(uuid_hex, session->wtd_session);
    if (strlen(session->wtd_session) != 36)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: SID Parse Error (%d)", __func__, strlen(session->wtd_session) );
       g_free(session);
       return;
     }
    Cfg_http.liste_http_clients = g_slist_append ( Cfg_http.liste_http_clients, session );

    SoupCookie *wtd_session = soup_cookie_new ( "wtd_session", session->wtd_session, NULL, "/", 86400 );
    soup_cookie_set_http_only ( wtd_session, TRUE );
    if (Cfg_http.ssl_enable) soup_cookie_set_secure ( wtd_session, TRUE );
    GSList *liste = g_slist_append ( NULL, wtd_session );
    soup_cookies_to_response ( liste, msg );
    g_slist_free(liste);

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_add_bool   ( builder, "connected", TRUE );
    Json_add_string ( builder, "version",  VERSION );
    Json_add_string ( builder, "username", soup_client_context_get_auth_user(client) );
    Json_add_string ( builder, "instance", g_get_host_name() );
    Json_add_bool   ( builder, "instance_is_master", Config.instance_is_master );
    Json_add_bool   ( builder, "ssl", soup_server_is_https (server) );
    Json_add_int    ( builder, "access_level", session->access_level );
    Json_add_string ( builder, "wtd_session", session->wtd_session );
    Json_add_int    ( builder, "wtd_session_expiry", Cfg_http.wtd_session_expiry );
    Json_add_string ( builder, "message", "Welcome back Home !" );
    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Run_thread: Thread principal                                                                                               */
/* Entrée: une structure LIBRAIRIE                                                                                            */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { void *zmq_motifs, *zmq_msgs;
    SoupAuthDomain *domain;
    gint last_pulse = 0;

reload:
    memset( &Cfg_http, 0, sizeof(Cfg_http) );                                       /* Mise a zero de la structure de travail */
    Cfg_http.lib = lib;                                            /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "W-HTTP", lib, NOM_THREAD, "Manage Web Services with external Devices" );
    Http_Lire_config ();                                                    /* Lecture de la configuration logiciel du thread */

    SoupServer *socket = soup_server_new("server-header", "Watchdogd HTTP Server", NULL);
    if (!socket)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: SoupServer new Failed !", __func__ );
       goto end;
     }
    soup_server_add_handler ( socket, "/connect",        Http_traiter_connect, NULL, NULL );
    soup_server_add_handler ( socket, "/disconnect",     Http_traiter_disconnect, NULL, NULL );
    soup_server_add_handler ( socket, "/dls/del/",       Http_traiter_dls_del, NULL, NULL );
    soup_server_add_handler ( socket, "/dls",            Http_traiter_dls, NULL, NULL );
    soup_server_add_handler ( socket, "/mnemos/list/",   Http_traiter_mnemos_list, NULL, NULL );
    soup_server_add_handler ( socket, "/syn/list",       Http_traiter_syn_list, NULL, NULL );
    soup_server_add_handler ( socket, "/syn/show/",      Http_traiter_syn_show, NULL, NULL );
    soup_server_add_handler ( socket, "/syn/del/",       Http_traiter_syn_del, NULL, NULL );
    soup_server_add_handler ( socket, "/syn/get/",       Http_traiter_syn_get, NULL, NULL );
    soup_server_add_handler ( socket, "/syn/set",        Http_traiter_syn_set, NULL, NULL );
    soup_server_add_handler ( socket, "/syn/clic/",      Http_traiter_syn_clic, NULL, NULL );
    soup_server_add_handler ( socket, "/syn/update_motifs", Http_traiter_syn_update_motifs, NULL, NULL );
    soup_server_add_handler ( socket, "/archive/get/",   Http_traiter_archive_get, NULL, NULL );
    soup_server_add_handler ( socket, "/process",        Http_traiter_process, NULL, NULL );
    soup_server_add_handler ( socket, "/instance/list",  Http_traiter_instance_list, NULL, NULL );
    soup_server_add_handler ( socket, "/status",         Http_traiter_status, NULL, NULL );
    soup_server_add_handler ( socket, "/log/get",        Http_traiter_log_get, NULL, NULL );
    soup_server_add_handler ( socket, "/log",            Http_traiter_log, NULL, NULL );
    soup_server_add_handler ( socket, "/bus",            Http_traiter_bus, NULL, NULL );
    soup_server_add_handler ( socket, "/memory",         Http_traiter_memory, NULL, NULL );
    soup_server_add_handler ( socket, "/users/list",     Http_traiter_users_list, NULL, NULL );
    soup_server_add_handler ( socket, "/users/kill",     Http_traiter_users_kill, NULL, NULL );
    soup_server_add_handler ( socket, "/users/sessions", Http_traiter_users_sessions, NULL, NULL );
    soup_server_add_handler ( socket, "/histo/alive",    Http_traiter_histo_alive, NULL, NULL );
    soup_server_add_handler ( socket, "/histo/ack",      Http_traiter_histo_ack, NULL, NULL );
    gchar *protocols[] = { "live-motifs", "live-msgs" };
    soup_server_add_websocket_handler ( socket, "/live-motifs", NULL, protocols, Http_traiter_open_websocket_motifs_CB, NULL, NULL );
    soup_server_add_websocket_handler ( socket, "/live-msgs",   NULL, protocols, Http_traiter_open_websocket_msgs_CB, NULL, NULL );

    domain = soup_auth_domain_basic_new ( SOUP_AUTH_DOMAIN_REALM, "WatchdogServer",
                                          SOUP_AUTH_DOMAIN_BASIC_AUTH_CALLBACK, Http_authenticate_CB,
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/archive",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/connect",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/disconnect",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/dls",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/syn",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/process",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/instance",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/log",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/histo",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/mnemos",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/bus",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/users",
                                          SOUP_AUTH_DOMAIN_ADD_PATH, "/memory",
                                          NULL );
    soup_auth_domain_set_filter ( domain, Http_test_session_CB, NULL, NULL );
    soup_server_add_auth_domain(socket, domain);
    g_object_unref (domain);


    if (Cfg_http.ssl_enable)                                                                           /* Configuration SSL ? */
     { struct stat sbuf;
       if ( stat ( Cfg_http.ssl_cert_filepath, &sbuf ) == -1)                                     /* Test présence du fichier */
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
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
              "%s: HTTP SoupServer Listen OK on port %d, SSL=%d !", __func__,
              Cfg_http.tcp_port, Cfg_http.ssl_enable );

    GMainLoop *loop = g_main_loop_new (NULL, TRUE);
    GMainContext *loop_context = g_main_loop_get_context ( loop );

    zmq_msgs   = Connect_zmq ( ZMQ_SUB, "listen-to-msgs",   "inproc", ZMQUEUE_LIVE_MSGS, 0 );
    zmq_motifs = Connect_zmq ( ZMQ_SUB, "listen-to-motifs", "inproc", ZMQUEUE_LIVE_MOTIFS, 0 );
    Cfg_http.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );
    Cfg_http.lib->Thread_run = TRUE;                                                                    /* Le thread tourne ! */
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { struct CMD_TYPE_HISTO histo;
       struct DLS_VISUEL visu;
       usleep(1000);
       sched_yield();

       if (Cfg_http.lib->Thread_reload)                                                      /* A-t'on recu un signal USR1 ? */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Thread Reload !", __func__ );
          break;
        }

       Http_Envoyer_les_cadrans ();

       if ( Recv_zmq ( zmq_motifs, &visu, sizeof(struct DLS_VISUEL) ) == sizeof(struct DLS_VISUEL) && Cfg_http.liste_ws_motifs_clients )
        { JsonBuilder *builder;
          gsize taille_buf;
          gchar *buf;
          GSList *liste;

          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: Visuel %s:%s received",
                    __func__, visu.tech_id, visu.acronyme );
          builder = Json_create ();
          if (builder == NULL)
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: JSon builder creation failed", __func__ );
             continue;
           }
          Json_add_string ( builder, "msg_type", "update_motif" );
          Dls_VISUEL_to_json ( builder, &visu );
          buf = Json_get_buf ( builder, &taille_buf );
          liste = Cfg_http.liste_ws_motifs_clients;
          while (liste)
           { struct WS_CLIENT_SESSION *client = liste->data;
             soup_websocket_connection_send_text ( client->connexion, buf );
             liste = g_slist_next(liste);
           }
          g_free(buf);
        }

       if ( Recv_zmq ( zmq_msgs, &histo, sizeof(histo) ) == sizeof(struct CMD_TYPE_HISTO) && Cfg_http.liste_ws_msgs_clients )
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: MSG %s:%s=%d received",
                    __func__, histo.msg.tech_id, histo.msg.acronyme, histo.alive );
          Http_msgs_send_histo_to_all(&histo);
        }

       if ( Partage->top > last_pulse + 50 )
        { Http_msgs_send_pulse_to_all();
          last_pulse = Partage->top;
          GSList *liste = Cfg_http.liste_http_clients;
          while(liste)
           { struct HTTP_CLIENT_SESSION *session = liste->data;
             liste = g_slist_next ( liste );
             if (session->last_request + Cfg_http.wtd_session_expiry*10 < Partage->top )
              { Cfg_http.liste_http_clients = g_slist_remove ( Cfg_http.liste_http_clients, session );
                Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: Session '%s' out of time", __func__, session->wtd_session );
                g_free(session);
              }
           }
        }


       g_main_context_iteration ( loop_context, FALSE );
     }

    soup_server_disconnect (socket);                                                            /* Arret du serveur WebSocket */
    /*Close_zmq ( Cfg_http.zmq_from_bus );*/
    Close_zmq ( Cfg_http.zmq_to_master );
    Close_zmq ( zmq_motifs );
    Close_zmq ( zmq_msgs );
    g_main_loop_unref(loop);

    g_slist_foreach ( Cfg_http.liste_http_clients, (GFunc) g_free, NULL );
    g_slist_free ( Cfg_http.liste_http_clients );
    Cfg_http.liste_http_clients = NULL;

    g_slist_foreach ( Cfg_http.liste_ws_motifs_clients, (GFunc) g_free, NULL );
    g_slist_free ( Cfg_http.liste_ws_motifs_clients );
    Cfg_http.liste_ws_motifs_clients = NULL;

    g_slist_foreach ( Cfg_http.liste_ws_msgs_clients, (GFunc) g_free, NULL );
    g_slist_free ( Cfg_http.liste_ws_msgs_clients );
    Cfg_http.liste_ws_msgs_clients = NULL;

end:
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    if (lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
