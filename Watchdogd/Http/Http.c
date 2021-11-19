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
    Creer_configDB ( Cfg_http.lib->name, "debug", "false" );

    g_snprintf( Cfg_http.ssl_cert_filepath, sizeof(Cfg_http.ssl_cert_filepath), "%s", HTTP_DEFAUT_FILE_CERT );
    Creer_configDB ( Cfg_http.lib->name, "ssl_file_cert", Cfg_http.ssl_cert_filepath );

    g_snprintf( Cfg_http.ssl_private_key_filepath, sizeof(Cfg_http.ssl_private_key_filepath), "%s", HTTP_DEFAUT_FILE_KEY );
    Creer_configDB ( Cfg_http.lib->name, "ssl_file_key", Cfg_http.ssl_private_key_filepath );

    Cfg_http.ssl_enable = TRUE;
    Creer_configDB ( Cfg_http.lib->name, "ssl", "true" );

    Cfg_http.tcp_port = 5560;
    Creer_configDB_int ( Cfg_http.lib->name, "tcp_port", Cfg_http.tcp_port );

    Cfg_http.wtd_session_expiry = 6000; /* En 1/10 secondes */
    Creer_configDB_int ( Cfg_http.lib->name, "wtd_session_expiry", Cfg_http.wtd_session_expiry );

    if ( ! Recuperer_configDB( &db, Cfg_http.lib->name ) )                                          /* Connexion a la base de données */
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
/* Http_destroy_session: Libère une session en paramètre                                                                      */
/* Entrées: la session                                                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_destroy_session ( struct HTTP_CLIENT_SESSION *session )
  { while ( session->liste_ws_clients ) Http_ws_destroy_session ( (struct WS_CLIENT_SESSION *)session->liste_ws_clients->data );
    g_slist_free ( session->Liste_bit_cadrans );
    g_free(session);
  }
/******************************************************************************************************************************/
/* Http_Save_and_close_sessions: Sauvegarde les sessions en base de données                                                   */
/* Entrées: néant                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_Save_and_close_sessions ( void )
  { while ( Cfg_http.liste_http_clients )
     { struct HTTP_CLIENT_SESSION *session = Cfg_http.liste_http_clients->data;
       SQL_Write_new ( "INSERT INTO users_sessions SET id='%d', username='%s', appareil='%s', useragent='%s', "
                       "wtd_session='%s', host='%s', last_request='%d' "
                       "ON DUPLICATE KEY UPDATE last_request=VALUES(last_request)",
                       session->id, session->username, session->appareil, session->useragent,
                       session->wtd_session, session->host, session->last_request );
       Cfg_http.liste_http_clients = g_slist_remove ( Cfg_http.liste_http_clients, session );
       Http_destroy_session(session);
     }
    Cfg_http.liste_http_clients = NULL;
  }
/******************************************************************************************************************************/
/* Http_Load_sessions: Charge les sessions en base de données                                                                 */
/* Entrées: néant                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_Load_one_session ( JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct HTTP_CLIENT_SESSION *session = g_try_malloc0( sizeof ( struct HTTP_CLIENT_SESSION ) );
    if (!session) return;
    g_snprintf( session->username,    sizeof(session->username),    "%s", Json_get_string ( element, "username" ) );
    g_snprintf( session->appareil,    sizeof(session->appareil),    "%s", Json_get_string ( element, "appareil" ) );
    g_snprintf( session->useragent,   sizeof(session->useragent),   "%s", Json_get_string ( element, "useragent" ) );
    g_snprintf( session->wtd_session, sizeof(session->wtd_session), "%s", Json_get_string ( element, "wtd_session" ) );
    g_snprintf( session->host,        sizeof(session->host),        "%s", Json_get_string ( element, "host" ) );
    session->id           = Json_get_int ( element, "id" );
    session->access_level = Json_get_int ( element, "access_level" );
    session->last_request = Json_get_int ( element, "last_request" );
    Cfg_http.liste_http_clients = g_slist_prepend ( Cfg_http.liste_http_clients, session );
    if (session->id >= Cfg_http.num_session) Cfg_http.num_session = session->id+1;            /* Calcul du MAX du num session */
  }
/******************************************************************************************************************************/
/* Http_Load_sessions: Charge les sessions en base de données                                                                 */
/* Entrées: néant                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_Load_sessions ( void )
  { JsonNode *RootNode = Json_node_create();
    SQL_Select_to_json_node ( RootNode, "sessions", "SELECT session.*, user.access_level "
                                                    "FROM users_sessions AS session "
                                                    "INNER JOIN users AS user ON session.username = user.username"
                            );
    Cfg_http.num_session = 0;
    if (Json_has_member ( RootNode, "sessions" ))
     { Json_node_foreach_array_element ( RootNode, "sessions", Http_Load_one_session, NULL ); }
    json_node_unref(RootNode);
  }
/******************************************************************************************************************************/
/* Check_utilisateur_password: Vérifie le mot de passe fourni                                                                 */
/* Entrées: une structure util, un code confidentiel                                                                          */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Http_check_utilisateur_password( gchar *dbsalt, gchar *dbhash, gchar *pwd )
  { guchar hash[EVP_MAX_MD_SIZE*2+1], hash_bin[EVP_MAX_MD_SIZE];
    memset ( hash, 0, sizeof(hash) );
    gint md_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                                                                   /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, dbsalt, strlen(dbsalt));
    EVP_DigestUpdate(mdctx, pwd, strlen(pwd) );
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);

    for (gint i=0; i<md_len; i++)
     { gchar chaine[3];
       g_snprintf(chaine, sizeof(chaine), "%02x", hash_bin[i] );
       g_strlcat( hash, chaine, sizeof(hash) );
     }

    return ( ! memcmp ( hash, dbhash, strlen(dbhash) ) );
  }
/******************************************************************************************************************************/
/* Http_rechercher_session: Recherche une session dans la liste des session                                                   */
/* Entrée: le message libsoup                                                                                                 */
/* Sortie: la session, ou NULL si pas trouvé                                                                                  */
/******************************************************************************************************************************/
 struct HTTP_CLIENT_SESSION *Http_rechercher_session_by_msg ( SoupMessage *msg )
  { struct HTTP_CLIENT_SESSION *result = NULL;
    GSList *cookies, *liste;

    if ( Config.instance_is_master == FALSE )
     { static struct HTTP_CLIENT_SESSION Slave_session = { -1, "system_user", "internal device", "Watchdog Server", "none", "no_sid", 9, 0 };
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
/* Http_add_cookie: Ajoute un cookie a la reponse                                                                             */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_add_cookie ( SoupMessage *msg, gchar *name, gchar *value, gint life )
  { SoupCookie *cookie = soup_cookie_new ( name, value, "", "/", life );
    soup_cookie_set_http_only ( cookie, TRUE );
    if (Cfg_http.ssl_enable) soup_cookie_set_secure ( cookie, TRUE );
    GSList *liste = g_slist_append ( NULL, cookie );
    soup_cookies_to_response ( liste, msg );
    g_slist_free(liste);
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
 gboolean Http_check_session ( SoupMessage *msg, struct HTTP_CLIENT_SESSION *session, gint min_access_level )
  { if (!session)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Not Connected");
       return(FALSE);
     }

    time(&session->last_request);
    if (session->access_level>=min_access_level) return(TRUE);
    soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Session Level forbidden");
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Http_traiter_ping: Répond aux requetes sur l'URI ping, et renouvelle le cookie de session                                  */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_ping ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_node_add_string ( RootNode, "response", "pong" );

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
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
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "username" ) && Json_has_member ( request, "password" ) &&
            Json_has_member ( request, "useragent" ) && Json_has_member ( request, "appareil" )
           )
       )
     { json_node_unref(request);
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
                "SELECT username,comment,access_level,enable,salt,hash FROM users WHERE username='%s' LIMIT 1", name );
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
    if ( Http_check_utilisateur_password( db->row[4], db->row[5], Json_get_string ( request, "password" ) ) == FALSE )/* Comparaison MDP */
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

    pthread_mutex_lock( &Cfg_http.lib->synchro );                                  /* On prend un numéro de session tout neuf */
    session->id = Cfg_http.num_session++;
    pthread_mutex_unlock( &Cfg_http.lib->synchro );

    g_snprintf( session->username, sizeof(session->username), "%s", db->row[0] );
    gchar *useragent = Normaliser_chaine ( Json_get_string ( request, "useragent" ) );
    g_snprintf( session->useragent, sizeof(session->useragent), "%s", useragent );
    g_free(useragent);
    gchar *appareil = Normaliser_chaine ( Json_get_string ( request, "appareil" ) );
    g_snprintf( session->appareil, sizeof(session->appareil), "%s", appareil );
    g_free(appareil);
    gchar *temp = g_inet_address_to_string ( g_inet_socket_address_get_address ( G_INET_SOCKET_ADDRESS(soup_client_context_get_remote_address (client) )) );
    g_snprintf( session->host, sizeof(session->host), "%s", temp );
    g_free(temp);
    session->access_level = atoi(db->row[2]);
    Liberer_resultat_SQL (db);
    Libere_DB_SQL( &db );

    time(&session->last_request);
    New_uuid ( session->wtd_session );
    if (strlen(session->wtd_session) != 36)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: SID Parse Error (%d)", __func__, strlen(session->wtd_session) );
       g_free(session);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "UUID Error");
       return;
     }
    Cfg_http.liste_http_clients = g_slist_append ( Cfg_http.liste_http_clients, session );

    Http_add_cookie ( msg, "wtd_session", session->wtd_session, 180*SOUP_COOKIE_MAX_AGE_ONE_DAY );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: User '%s:%s' connected", __func__,
              session->username, session->host );

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_node_add_bool   ( RootNode, "connected", TRUE );
    Json_node_add_string ( RootNode, "version",  WTD_VERSION );
    Json_node_add_string ( RootNode, "username", session->username );
    Json_node_add_string ( RootNode, "appareil", session->appareil );
    Json_node_add_string ( RootNode, "instance", g_get_host_name() );
    Json_node_add_bool   ( RootNode, "instance_is_master", Config.instance_is_master );
    Json_node_add_bool   ( RootNode, "ssl", soup_server_is_https (server) );
    Json_node_add_int    ( RootNode, "access_level", session->access_level );
    Json_node_add_string ( RootNode, "wtd_session", session->wtd_session );
    Json_node_add_string ( RootNode, "message", "Welcome back Home !" );
    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_users_kill: Kill une session utilisateur                                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 void Http_traiter_search ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;

    gchar *search;
    gpointer search_string = g_hash_table_lookup ( query, "search[value]" );
    if (!search_string) { search = g_strdup (""); }
                   else { search = Normaliser_chaine ( search_string ); }
    if (!search)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Memory Error");
       return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    gchar *draw_string = g_hash_table_lookup ( query, "draw" );
    if (draw_string) Json_node_add_int ( RootNode, "draw", atoi(draw_string) );
                else Json_node_add_int ( RootNode, "draw", 1 );

    gint start;
    gchar *start_string = g_hash_table_lookup ( query, "start" );
    if (start_string) start = atoi(start_string);
                 else start = 200;

    gint length;
    gchar *length_string = g_hash_table_lookup ( query, "length" );
    if (length_string) length = atoi(length_string);
                  else length = 200;


    if (SQL_Select_to_json_node ( RootNode, NULL, "SELECT COUNT(*) AS recordsTotal FROM dictionnaire LIMIT %d", length )==FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref ( RootNode );
       g_free(search);
       return;
     }
    if (SQL_Select_to_json_node ( RootNode, NULL,
                                  "SELECT COUNT(*) AS recordsFiltered FROM dictionnaire "
                                  "WHERE tech_id LIKE '%%%s%%' OR acronyme LIKE '%%%s%%' OR libelle LIKE '%%%s%%' "
                                  "LIMIT %d OFFSET %d",
                                  search, search, search, length, start )==FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref ( RootNode );
       g_free(search);
       return;
     }
    if (SQL_Select_to_json_node ( RootNode, "data",
                                  "SELECT * FROM dictionnaire "
                                  "WHERE tech_id LIKE '%%%s%%' OR acronyme LIKE '%%%s%%' OR libelle LIKE '%%%s%%' "
                                  "LIMIT %d OFFSET %d",
                                  search, search, search, length, start )==FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref ( RootNode );
       g_free(search);
       return;
     }
    g_free(search);

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Run_thread: Thread principal                                                                                               */
/* Entrée: une structure LIBRAIRIE                                                                                            */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { gint last_pulse = 0;
    GError *error;

reload:
    memset( &Cfg_http, 0, sizeof(Cfg_http) );                                       /* Mise a zero de la structure de travail */
    Cfg_http.lib = lib;                                            /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "http", "IHM/API", lib, WTD_VERSION, "Manage Web Services with external Devices" );
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
    soup_server_add_handler ( socket, "/api/dls/run/set" ,   Http_traiter_dls_run_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/run" ,       Http_traiter_dls_run, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/debug" ,     Http_traiter_dls_debug, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/undebug",    Http_traiter_dls_undebug, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/start" ,     Http_traiter_dls_start, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/stop" ,      Http_traiter_dls_stop, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/acquitter",  Http_traiter_dls_acquitter, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/compil" ,    Http_traiter_dls_compil, NULL, NULL );
    soup_server_add_handler ( socket, "/api/dls/set" ,       Http_traiter_dls_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/mnemos/validate",Http_traiter_mnemos_validate, NULL, NULL );
    soup_server_add_handler ( socket, "/api/mnemos/list",    Http_traiter_mnemos_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/mnemos/set",     Http_traiter_mnemos_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/map/list",       Http_traiter_map_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/map/del",        Http_traiter_map_del, NULL, NULL );
    soup_server_add_handler ( socket, "/api/map/set",        Http_traiter_map_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/list",       Http_traiter_syn_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/del",        Http_traiter_syn_del, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/get",        Http_traiter_syn_get, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/set",        Http_traiter_syn_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/clic",       Http_traiter_syn_clic, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/show",       Http_traiter_syn_show, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/ack",        Http_traiter_syn_ack, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/save",       Http_traiter_syn_save, NULL, NULL );
    soup_server_add_handler ( socket, "/api/horloge/get",    Http_traiter_horloge_get, NULL, NULL );
    soup_server_add_handler ( socket, "/api/horloge/ticks/set", Http_traiter_horloge_ticks_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/horloge/ticks/del", Http_traiter_horloge_ticks_del, NULL, NULL );
    soup_server_add_handler ( socket, "/api/horloge/ticks/list",Http_traiter_horloge_ticks_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/tableau/list",   Http_traiter_tableau_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/tableau/del",    Http_traiter_tableau_del, NULL, NULL );
    soup_server_add_handler ( socket, "/api/tableau/set",    Http_traiter_tableau_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/tableau/map/list", Http_traiter_tableau_map_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/tableau/map/del",  Http_traiter_tableau_map_del, NULL, NULL );
    soup_server_add_handler ( socket, "/api/tableau/map/set",  Http_traiter_tableau_map_set, NULL, NULL );
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
    soup_server_add_handler ( socket, "/api/instance/loglevel",
                                                             Http_traiter_instance_loglevel, NULL, NULL );
    soup_server_add_handler ( socket, "/api/instance/reload_icons",
                                                             Http_traiter_instance_reload_icons, NULL, NULL );
    soup_server_add_handler ( socket, "/api/status",         Http_traiter_status, NULL, NULL );
    soup_server_add_handler ( socket, "/api/log/get",        Http_traiter_log_get, NULL, NULL );
    soup_server_add_handler ( socket, "/api/install",        Http_traiter_install, NULL, NULL );
    soup_server_add_handler ( socket, "/api/bus",            Http_traiter_bus, NULL, NULL );
    soup_server_add_handler ( socket, "/api/ping",           Http_traiter_ping, NULL, NULL );
    soup_server_add_handler ( socket, "/api/search",         Http_traiter_search, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/list",     Http_traiter_users_list, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/set",      Http_traiter_users_set, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/get",      Http_traiter_users_get, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/add",      Http_traiter_users_add, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/del",      Http_traiter_users_del, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/kill",     Http_traiter_users_kill, NULL, NULL );
    soup_server_add_handler ( socket, "/api/users/sessions", Http_traiter_users_sessions, NULL, NULL );
    soup_server_add_handler ( socket, "/api/histo/alive",    Http_traiter_histo_alive, NULL, NULL );
    soup_server_add_handler ( socket, "/api/histo/ack",      Http_traiter_histo_ack, NULL, NULL );
    soup_server_add_handler ( socket, "/api/upload",         Http_traiter_upload, NULL, NULL );
    soup_server_add_handler ( socket, "/",                   Http_traiter_file, NULL, NULL );
    if (Config.instance_is_master==TRUE)
     { static gchar *protocols[] = { "live-motifs" };
       soup_server_add_websocket_handler ( socket, "/api/live-motifs", NULL, protocols, Http_traiter_open_websocket_motifs_CB, NULL, NULL );
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

    Http_Load_sessions ();
    Cfg_http.lib->Thread_run = TRUE;                                                                    /* Le thread tourne ! */
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { usleep(1000);
       sched_yield();

       if (Cfg_http.lib->Thread_reload)                                                      /* A-t'on recu un signal USR1 ? */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Thread Reload !", __func__ );
          break;
        }

       Http_Envoyer_les_cadrans ();

       JsonNode *request;
       while ( (request = Thread_Listen_to_master ( lib ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
               if (!strcasecmp( zmq_tag, "DLS_HISTO" ))    { Http_ws_send_to_all( request ); }
          else if (!strcasecmp( zmq_tag, "DLS_VISUEL" ))   { Http_ws_send_to_all( request ); }
          else if (!strcasecmp( zmq_tag, "SET_SYN_VARS" )) { Http_ws_send_to_all( request ); }
          json_node_unref ( request );
        }

       if ( Partage->top > last_pulse + 50 )
        { last_pulse = Partage->top;
          JsonNode *pulse = Json_node_create();
          if (pulse)
           { Json_node_add_string( pulse, "zmq_tag", "PULSE" );
             Http_ws_send_to_all ( pulse );
             json_node_unref(pulse);
           }
          pthread_mutex_lock( &Cfg_http.lib->synchro );
          GSList *liste = Cfg_http.liste_http_clients;
          while(liste)
           { struct HTTP_CLIENT_SESSION *client = liste->data;
             liste = g_slist_next ( liste );
             if (client->last_request + Cfg_http.wtd_session_expiry < Partage->top )
              { Cfg_http.liste_http_clients = g_slist_remove ( Cfg_http.liste_http_clients, client );
                Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: Session '%s' out of time", __func__, client->wtd_session );
                Http_destroy_session ( client );
              }
           }
          pthread_mutex_unlock( &Cfg_http.lib->synchro );
        }

       g_main_context_iteration ( loop_context, FALSE );
     }

    soup_server_disconnect (socket);                                                            /* Arret du serveur WebSocket */
    g_main_loop_unref(loop);

    Http_Save_and_close_sessions();

end:
    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
