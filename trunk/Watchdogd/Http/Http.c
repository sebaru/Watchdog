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

    Cfg_http.lib->Thread_debug = FALSE;
    Creer_configDB ( NOM_THREAD, "debug", "false" );

    g_snprintf( Cfg_http.ssl_cert_filepath, sizeof(Cfg_http.ssl_cert_filepath), "%s", HTTP_DEFAUT_FILE_CERT );
    Creer_configDB ( NOM_THREAD, "ssl_file_cert", Cfg_http.ssl_cert_filepath );

    g_snprintf( Cfg_http.ssl_private_key_filepath, sizeof(Cfg_http.ssl_private_key_filepath), "%s", HTTP_DEFAUT_FILE_KEY );
    Creer_configDB ( NOM_THREAD, "ssl_file_key", Cfg_http.ssl_private_key_filepath );

    Cfg_http.ssl_enable = TRUE;
    Creer_configDB ( NOM_THREAD, "ssl", "true" );

    Cfg_http.tcp_port = 5560;
    Creer_configDB_int ( NOM_THREAD, "tcp_port", Cfg_http.tcp_port );

    Cfg_http.wtd_session_expiry = 7200;
    Creer_configDB_int ( NOM_THREAD, "wtd_session_expiry", Cfg_http.wtd_session_expiry );

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
        { if ( g_ascii_strcasecmp( valeur, "true" ) ) Cfg_http.ssl_enable = FALSE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "tcp_port" ) )
        { Cfg_http.tcp_port = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "wtd_session_expiry" ) )
        { Cfg_http.wtd_session_expiry = atoi(valeur); }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_http.lib->Thread_debug = TRUE;  }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Http_traiter_log: Répond aux requetes sur l'URI log                                                                        */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_log ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data)
  { GBytes *request_brute;
    gsize taille;

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( ! Json_has_member ( request, "log_level" ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *log_level = Json_get_string ( request, "log_level" );
         if ( ! g_ascii_strcasecmp ( log_level, "LOG_DEBUG"   ) ) { Info_change_log_level ( Config.log, LOG_DEBUG   ); }
    else if ( ! g_ascii_strcasecmp ( log_level, "LOG_NOTICE"  ) ) { Info_change_log_level ( Config.log, LOG_NOTICE  ); }
    else if ( ! g_ascii_strcasecmp ( log_level, "LOG_INFO"    ) ) { Info_change_log_level ( Config.log, LOG_INFO    ); }
    else if ( ! g_ascii_strcasecmp ( log_level, "LOG_WARNING" ) ) { Info_change_log_level ( Config.log, LOG_WARNING ); }
    else if ( ! g_ascii_strcasecmp ( log_level, "LOG_ERROR"   ) ) { Info_change_log_level ( Config.log, LOG_ERR     ); }
	   else soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais niveau de log");
    json_node_unref(request);
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_redirect_to_slave: Proxifie une requete vers un slave                                                                 */
/* Entrée : le message source, le nom de l'instance cible                                                                     */
/* Sortie : le contenu de la reponse du slave                                                                                 */
/******************************************************************************************************************************/
 void Http_redirect_to_slave ( SoupMessage *msg, gchar *target )
  { SoupSession *connexion;
    connexion = soup_session_new();
    g_object_set ( G_OBJECT(connexion), "ssl-strict", FALSE, NULL );
    SoupURI *URI = soup_uri_copy (soup_message_get_uri (msg));
    soup_uri_set_host ( URI, target );
    SoupMessage *new_msg = soup_message_new_from_uri ( msg->method, URI );
    soup_uri_free(URI);
    soup_message_set_request ( new_msg, "application/json; charset=UTF-8",
                               SOUP_MEMORY_COPY, msg->request_body->data, msg->request_body->length );
    soup_session_send_message ( connexion, new_msg );
    soup_message_set_status  ( msg, new_msg->status_code );
    soup_message_set_response ( msg, "application/json; charset=UTF-8",
                                SOUP_MEMORY_COPY, new_msg->response_body->data, new_msg->response_body->length );
    g_object_unref ( new_msg );
    g_object_unref( connexion );
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

    if ( Config.instance_is_master == FALSE )
     { static struct HTTP_CLIENT_SESSION Slave_session = { "system_user", "none", "no_sid", 9, 0 };
       return(&Slave_session);
     }

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
/* Http_print_request: affiche les données relatives à une requete                                                            */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 struct HTTP_CLIENT_SESSION *Http_print_request ( SoupServer *server, SoupMessage *msg, const char *path, SoupClientContext *client )
  { struct HTTP_CLIENT_SESSION *session = Http_rechercher_session_by_msg ( msg );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: sid '%s' (%s@%s, Level %d) : '%s'", __func__,
              (session ? session->wtd_session : "none"),
              (session ? session->username : "none"), soup_client_context_get_host(client),
              (session ? session->access_level : -1), path
               );
    return(session);
  }
/******************************************************************************************************************************/
/* Http_print_request: affiche les données relatives à une requete                                                            */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 gboolean Http_check_session ( SoupMessage *msg, struct HTTP_CLIENT_SESSION * session, gint min_access_level )
  { if (!session)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Not Connected");
       return(FALSE);
     }

    if (session->access_level<min_access_level)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Not high enough");
       return(FALSE);
     }
    time(&session->last_request);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Http_traiter_connect: Répond aux requetes sur l'URI connect                                                                */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_ping ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
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
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_add_bool   ( builder, "installed", Config.installed );

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_traiter_disconnect: Répond aux requetes sur l'URI disconnect                                                          */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_disconnect ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }
    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (session)
     { Cfg_http.liste_http_clients = g_slist_remove ( Cfg_http.liste_http_clients, session );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: sid '%s' ('%s', level %d) disconnected", __func__,
                 session->wtd_session, session->username, session->access_level );
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
    GBytes *request_brute;
    JsonBuilder *builder;
    gsize taille_buf;
    gsize taille;
    gchar *buf;

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    if (!Config.installed)
     {	Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Redirecting to /tech/install", __func__ );
       soup_message_set_redirect (msg, SOUP_STATUS_TEMPORARY_REDIRECT, "/tech/install" );
		     return;
     }

    Http_print_request ( server, msg, path, client );

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( ! (request && Json_has_member ( request, "username" ) && Json_has_member ( request, "password" ) ) )
     { if (request) json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    name = Normaliser_chaine ( Json_get_string ( request, "username" ) );                    /* Formatage correct des chaines */
    if (!name)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT username,comment,access_level,enable,hash FROM users WHERE username='%s' LIMIT 1", name );
    g_free(name);

    struct DB *db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: DB connexion failed for user '%s'", __func__, Json_get_string ( request, "username" ) );
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "DB Error");
       return;
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: DB request failed for user '%s'",__func__, Json_get_string ( request, "username" ) );
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "DB Error");
       return;
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                "%s: User '%s' not found in DB", __func__, Json_get_string ( request, "username" ) );
       json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Acces interdit !");
       return;
     }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: User '%s' (%s) found in database.",
              __func__, db->row[0], db->row[1] );

/*********************************************************** Compte du client *************************************************/
    if (atoi(db->row[3]) != 1)                                                 /* Est-ce que son compte est toujours actif ?? */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: User '%s' not enabled",
                 __func__, db->row[0] );
       Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Acces interdit !");
       return;
     }
/*********************************************** Authentification du client par login mot de passe ****************************/
    if ( Http_check_utilisateur_password( db->row[4], Json_get_string ( request, "password" ) ) == FALSE ) /* Comparaison MDP */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: Password error for '%s' (%s)",
                 __func__, db->row[0], db->row[1] );
       Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Acces interdit !");
       return;
     }

    struct HTTP_CLIENT_SESSION *session = g_try_malloc0( sizeof(struct HTTP_CLIENT_SESSION) );
    if (!session)
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Session creation Error", __func__ );
       json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    g_snprintf( session->username, sizeof(session->username), "%s", db->row[0] );
    gchar *temp = g_inet_address_to_string ( g_inet_socket_address_get_address ( G_INET_SOCKET_ADDRESS(soup_client_context_get_remote_address (client) )) );
    g_snprintf( session->host, sizeof(session->host), "%s", temp );
    g_free(temp);
    session->access_level = atoi(db->row[2]);
    Liberer_resultat_SQL (db);
    Libere_DB_SQL( &db );

    time(&session->last_request);
    uuid_t uuid_hex;
    uuid_generate(uuid_hex);
    uuid_unparse_lower(uuid_hex, session->wtd_session);
    if (strlen(session->wtd_session) != 36)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: SID Parse Error (%d)", __func__, strlen(session->wtd_session) );
       g_free(session);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "UUID Error");
       return;
     }
    Cfg_http.liste_http_clients = g_slist_append ( Cfg_http.liste_http_clients, session );

    SoupCookie *wtd_session = soup_cookie_new ( "wtd_session", session->wtd_session, NULL, "/", Cfg_http.wtd_session_expiry );
    soup_cookie_set_http_only ( wtd_session, TRUE );
    if (Cfg_http.ssl_enable) soup_cookie_set_secure ( wtd_session, TRUE );
    GSList *liste = g_slist_append ( NULL, wtd_session );
    soup_cookies_to_response ( liste, msg );
    g_slist_free(liste);
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: User '%s:%s' connected", __func__,
              session->username, session->host );

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
    Json_add_string ( builder, "version",  WTD_VERSION );
    Json_add_string ( builder, "username", session->username );
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
/* Http_Traiter_users_kill: Kill une session utilisateur                                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 void Http_traiter_search ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille_buf;
    gsize taille;

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    if (!request_brute)
     { soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }

    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( !(Json_has_member ( request, "search_for" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar chaine[256];
    gchar *search = Normaliser_chaine ( Json_get_string(request, "search_for" ) );
    json_node_unref(request);

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
    if (search)

     { g_snprintf ( chaine, sizeof (chaine), "SELECT * FROM dictionnaire WHERE tech_id LIKE '%s' OR acronyme LIKE '%s' OR libelle LIKE '%s'",
                    search, search, search );
       g_free(search);
       if (SQL_Select_to_JSON ( builder, "results", chaine ))
            { soup_message_set_status (msg, SOUP_STATUS_OK); }
       else { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error"); }
     }
	   else soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );

    gchar *buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Run_thread: Thread principal                                                                                               */
/* Entrée: une structure LIBRAIRIE                                                                                            */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { void *zmq_motifs, *zmq_from_bus;
    gint last_pulse = 0;
    GError *error;

reload:
    memset( &Cfg_http, 0, sizeof(Cfg_http) );                                       /* Mise a zero de la structure de travail */
    Cfg_http.lib = lib;                                            /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "W-HTTP", lib, WTD_VERSION, "Manage Web Services with external Devices" );
    Http_Lire_config ();                                                    /* Lecture de la configuration logiciel du thread */

    SoupServer *socket = soup_server_new("server-header", "Watchdogd HTTP Server", NULL);
    if (!socket)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: SoupServer new Failed !", __func__ );
       goto end;
     }

    if (Cfg_http.ssl_enable)                                                                           /* Configuration SSL ? */
     { struct stat sbuf;
       if ( stat ( Cfg_http.ssl_cert_filepath, &sbuf ) == -1 ||                                   /* Test présence du fichier */
            stat ( Cfg_http.ssl_private_key_filepath, &sbuf ) == -1 )                             /* Test présence du fichier */
        { gchar chaine[256];
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "%s: unable to load '%s' and '%s' (error '%s'). Generating new ones.", __func__,
                    Cfg_http.ssl_cert_filepath, Cfg_http.ssl_private_key_filepath, strerror(errno) );
          g_snprintf( chaine, sizeof(chaine),
                      "openssl req -subj '/C=FR/ST=FRANCE/O=ABLS-HABITAT/OU=PRODUCTION/CN=Watchdog Server on %s' -new -newkey rsa:2048 -sha256 -days 3650 -nodes -x509 -out '%s' -keyout '%s'",
                      g_get_host_name(), Cfg_http.ssl_cert_filepath, Cfg_http.ssl_private_key_filepath );
          system( chaine );
        }

       if (soup_server_set_ssl_cert_file ( socket, Cfg_http.ssl_cert_filepath, Cfg_http.ssl_private_key_filepath, &error ))
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: SSL Loaded with '%s' and '%s'", __func__,
                    Cfg_http.ssl_cert_filepath, Cfg_http.ssl_private_key_filepath );
        }
       else
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Failed to load SSL Certificate '%s' and '%s'. Error '%s'",
                     __func__, Cfg_http.ssl_cert_filepath, Cfg_http.ssl_private_key_filepath, error->message  );
          g_error_free(error);
        }
     }
    else
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: SSL is disabled", __func__ ); }

    soup_server_add_handler ( socket, "/api/connect",        Http_traiter_connect, NULL, NULL );
    soup_server_add_handler ( socket, "/api/disconnect",     Http_traiter_disconnect, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/list",       Http_traiter_dls_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/source",     Http_traiter_dls_source, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/del",        Http_traiter_dls_del, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/status" ,    Http_traiter_dls_status, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/run" ,       Http_traiter_dls_run, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/debug" ,     Http_traiter_dls_debug, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/undebug",    Http_traiter_dls_undebug, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/start" ,     Http_traiter_dls_start, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/stop" ,      Http_traiter_dls_stop, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/acquitter",  Http_traiter_dls_acquitter, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/compil" ,    Http_traiter_dls_compil, NULL, NULL );
    soup_server_add_handler ( socket, "/api/mnemos/validate",Http_traiter_mnemos_validate, NULL, NULL );
    soup_server_add_handler ( socket, "/api/mnemos/list",    Http_traiter_mnemos_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/mnemos/set",     Http_traiter_mnemos_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/map/list",       Http_traiter_map_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/map/del",        Http_traiter_map_del, NULL, NULL );
    soup_server_add_handler ( socket, "/api/map/set",        Http_traiter_map_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/list",       Http_traiter_syn_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/show",       Http_traiter_syn_show, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/del",        Http_traiter_syn_del, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/get",        Http_traiter_syn_get, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/set",        Http_traiter_syn_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/clic",       Http_traiter_syn_clic, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/update_motifs", Http_traiter_syn_update_motifs, NULL, NULL );
    soup_server_add_handler ( socket, "/api/archive/get",    Http_traiter_archive_get, NULL, NULL );
    soup_server_add_handler ( socket, "/api/process/reload", Http_traiter_process_reload, NULL, NULL );
    soup_server_add_handler ( socket, "/api/process/start",  Http_traiter_process_start, NULL, NULL );
    soup_server_add_handler ( socket, "/api/process/debug",  Http_traiter_process_debug, NULL, NULL );
    soup_server_add_handler ( socket, "/api/process/list",   Http_traiter_process_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/process",        Http_traiter_process, NULL, NULL );
    soup_server_add_handler ( socket, "/api/config/get",     Http_traiter_config_get, NULL, NULL );
    soup_server_add_handler ( socket, "/api/config/set",     Http_traiter_config_set, NULL, NULL );
/*    soup_server_add_handler ( socket, "/api/config/del",     Http_traiter_config_del, NULL, NULL );*/
    soup_server_add_handler ( socket, "/api/instance/list",  Http_traiter_instance_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/instance/reset", Http_traiter_instance_reset, NULL, NULL );
    soup_server_add_handler ( socket, "/api/status",         Http_traiter_status, NULL, NULL );
    soup_server_add_handler ( socket, "/api/log/get",        Http_traiter_log_get, NULL, NULL );
    soup_server_add_handler ( socket, "/api/log",            Http_traiter_log, NULL, NULL );
    soup_server_add_handler ( socket, "/api/install",        Http_traiter_install, NULL, NULL );
    soup_server_add_handler ( socket, "/api/bus",            Http_traiter_bus, NULL, NULL );
    soup_server_add_handler ( socket, "/api/ping",           Http_traiter_ping, NULL, NULL );
    soup_server_add_handler ( socket, "/api/search",         Http_traiter_search, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/list",     Http_traiter_users_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/set",      Http_traiter_users_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/kill",     Http_traiter_users_kill, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/sessions", Http_traiter_users_sessions, NULL, NULL );
    soup_server_add_handler ( socket, "/api/histo/alive",    Http_traiter_histo_alive, NULL, NULL );
    soup_server_add_handler ( socket, "/api/histo/ack",      Http_traiter_histo_ack, NULL, NULL );
    soup_server_add_handler ( socket, "/",                   Http_traiter_file, NULL, NULL );
    if (Config.instance_is_master==TRUE)
     { gchar *protocols[] = { "live-motifs", "live-msgs" };
       soup_server_add_websocket_handler ( socket, "/api/live-motifs", NULL, protocols, Http_traiter_open_websocket_motifs_CB, NULL, NULL );
       soup_server_add_websocket_handler ( socket, "/api/live-msgs",   NULL, protocols, Http_traiter_open_websocket_msgs_CB, NULL, NULL );
     }

    if (!soup_server_listen_all (socket, Cfg_http.tcp_port, (Cfg_http.ssl_enable ? SOUP_SERVER_LISTEN_HTTPS : 0), &error))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: SoupServer Listen Failed '%s' !", __func__, error->message );
       g_error_free(error);
       goto end;
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
              "%s: HTTP SoupServer Listen OK on port %d, SSL=%d !", __func__,
              Cfg_http.tcp_port, Cfg_http.ssl_enable );

    GMainLoop *loop = g_main_loop_new (NULL, TRUE);
    GMainContext *loop_context = g_main_loop_get_context ( loop );

    zmq_from_bus = Connect_zmq ( ZMQ_SUB, "listen-to-bus",    "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    zmq_motifs   = Connect_zmq ( ZMQ_SUB, "listen-to-motifs", "inproc", ZMQUEUE_LIVE_MOTIFS, 0 );
    Cfg_http.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );
    Cfg_http.lib->Thread_run = TRUE;                                                                    /* Le thread tourne ! */
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { struct DLS_VISUEL visu;
       gchar buffer[2048];
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

       JsonNode *request = Recv_zmq_with_json( zmq_from_bus, NULL, (gchar *)&buffer, sizeof(buffer) );
       if (request)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if (!strcasecmp( zmq_tag, "DLS_HISTO" ))
           { Http_msgs_send_histo_to_all( request ); }
          else json_node_unref ( request );
        }

       if ( Partage->top > last_pulse + 50 )
        { Http_msgs_send_pulse_to_all();
          last_pulse = Partage->top;
          GSList *liste = Cfg_http.liste_http_clients;
          while(liste)
           { struct HTTP_CLIENT_SESSION *client = liste->data;
             liste = g_slist_next ( liste );
             if (client->last_request + Cfg_http.wtd_session_expiry*10 < Partage->top )
              { Cfg_http.liste_http_clients = g_slist_remove ( Cfg_http.liste_http_clients, client );
                Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: Session '%s' out of time", __func__, client->wtd_session );
                g_free(client);
              }
           }
        }

       g_main_context_iteration ( loop_context, FALSE );
     }

    soup_server_disconnect (socket);                                                            /* Arret du serveur WebSocket */
    /*Close_zmq ( Cfg_http.zmq_from_bus );*/
    Close_zmq ( Cfg_http.zmq_to_master );
    Close_zmq ( zmq_motifs );
    Close_zmq ( zmq_from_bus );
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
    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
