/******************************************************************************************************************************/
/* Watchdogd/Http/Http.c        Gestion des connexions HTTP WebService de watchdog                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Http.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
/* Http_add_cookie: Ajoute un cookie a la reponse                                                                             */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_add_cookie ( SoupServerMessage *msg, gchar *name, gchar *value, gint life )
  { gchar chaine[256];
    g_snprintf( chaine, sizeof(chaine), "%s=%s", name, value );
    SoupMessageHeaders *response_headers = soup_server_message_get_response_headers (msg);
    soup_message_headers_replace (response_headers, "Set-Cookie", chaine );
  }
/******************************************************************************************************************************/
/* Http_traiter_connect: Répond aux requetes sur l'URI connect                                                                */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_connect ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data )
  { gchar requete[256], *name;

    if (soup_server_message_get_method(msg) != SOUP_METHOD_POST)
     { soup_server_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED, NULL);
       return;
     }

    Http_print_request ( server, msg, path );
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "username" ) && Json_has_member ( request, "password" ) &&
            Json_has_member ( request, "useragent" ) && Json_has_member ( request, "appareil" )
           )
       )
     { Json_node_unref(request);
       soup_server_message_set_status (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    name = Normaliser_chaine ( Json_get_string ( request, "username" ) );                    /* Formatage correct des chaines */
    if (!name)
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "Normalisation impossible" );
       Json_node_unref(request);
       soup_server_message_set_status (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT username,comment,access_level,enable,salt,hash FROM users WHERE username='%s' LIMIT 1", name );
    g_free(name);

    struct DB *db = Init_DB_SQL();
    if (!db)
     { Info_new( __func__, Config.log_msrv, LOG_ERR,
                "DB connexion failed for user '%s'", Json_get_string ( request, "username" ) );
       soup_server_message_set_status (msg, SOUP_STATUS_BAD_REQUEST, "DB Error");
       return;
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Info_new( __func__, Config.log_msrv, LOG_ERR,
                "%s: DB request failed for user '%s'",__func__, Json_get_string ( request, "username" ) );
       soup_server_message_set_status (msg, SOUP_STATUS_BAD_REQUEST, "DB Error");
       return;
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( __func__, Config.log_msrv, LOG_WARNING,
                "User '%s' not found in DB", Json_get_string ( request, "username" ) );
       Json_node_unref(request);
       soup_server_message_set_status (msg, SOUP_STATUS_FORBIDDEN, "Acces interdit !");
       return;
     }

    Info_new( __func__, Config.log_msrv, LOG_INFO, "%s: User '%s' (%s) found in database.",
              __func__, db->row[0], db->row[1] );

/*********************************************************** Compte du client *************************************************/
    if (atoi(db->row[3]) != 1)                                                 /* Est-ce que son compte est toujours actif ?? */
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "%s: User '%s' not enabled",
                 __func__, db->row[0] );
       Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Json_node_unref(request);
       soup_server_message_set_status (msg, SOUP_STATUS_FORBIDDEN, "Acces interdit !");
       return;
     }
/*********************************************** Authentification du client par login mot de passe ****************************/
    if ( Http_check_utilisateur_password( db->row[4], db->row[5], Json_get_string ( request, "password" ) ) == FALSE )/* Comparaison MDP */
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "%s: Password error for '%s' (%s)",
                 __func__, db->row[0], db->row[1] );
       Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Json_node_unref(request);
       soup_server_message_set_status (msg, SOUP_STATUS_FORBIDDEN, "Acces interdit !");
       return;
     }

    struct HTTP_CLIENT_SESSION *session = g_try_malloc0( sizeof(struct HTTP_CLIENT_SESSION) );
    if (!session)
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( __func__, Config.log_msrv, LOG_ERR, "Session creation Error" );
       Json_node_unref(request);
       soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    pthread_mutex_lock( &Partage->com_http.synchro );                                  /* On prend un numéro de session tout neuf */
    session->id = Partage->com_http.num_session++;
    pthread_mutex_unlock( &Partage->com_http.synchro );

    g_snprintf( session->username, sizeof(session->username), "%s", db->row[0] );
    gchar *useragent = Normaliser_chaine ( Json_get_string ( request, "useragent" ) );
    g_snprintf( session->useragent, sizeof(session->useragent), "%s", useragent );
    g_free(useragent);
    gchar *appareil = Normaliser_chaine ( Json_get_string ( request, "appareil" ) );
    g_snprintf( session->appareil, sizeof(session->appareil), "%s", appareil );
    g_free(appareil);
    g_snprintf( session->host, sizeof(session->host), "inconnu" );
    session->access_level = atoi(db->row[2]);
    Liberer_resultat_SQL (db);
    Libere_DB_SQL( &db );

    time(&session->last_request);
    UUID_New ( session->wtd_session );
    if (strlen(session->wtd_session) != 36)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "SID Parse Error (%d)", strlen(session->wtd_session) );
       g_free(session);
       soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "UUID Error");
       return;
     }
    Partage->com_http.liste_http_clients = g_slist_append ( Partage->com_http.liste_http_clients, session );

    Http_add_cookie ( msg, "wtd_session", session->wtd_session, 180*SOUP_COOKIE_MAX_AGE_ONE_DAY );
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "User '%s:%s' connected",
              session->username, session->host );

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "JSon RootNode creation failed" );
       soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_node_add_bool   ( RootNode, "connected", TRUE );
    Json_node_add_string ( RootNode, "version",  WTD_VERSION );
    Json_node_add_string ( RootNode, "branche",  WTD_BRANCHE );
    Json_node_add_string ( RootNode, "username", session->username );
    Json_node_add_string ( RootNode, "appareil", session->appareil );
    Json_node_add_string ( RootNode, "instance", g_get_host_name() );
    Json_node_add_bool   ( RootNode, "instance_is_master", Config.instance_is_master );
    Json_node_add_bool   ( RootNode, "ssl", soup_server_is_https (server) );
    Json_node_add_int    ( RootNode, "access_level", session->access_level );
    Json_node_add_string ( RootNode, "wtd_session", session->wtd_session );
    Json_node_add_string ( RootNode, "message", "Welcome back Home !" );
/*************************************************** Envoi au client **********************************************************/
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "application/json; charset=UTF-8", RootNode );
  }
/******************************************************************************************************************************/
/* Http_traiter_disconnect: Répond aux requetes sur l'URI disconnect                                                          */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_disconnect ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data )
  { if (soup_server_message_get_method(msg) != SOUP_METHOD_PUT)
     {	soup_server_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED, NULL);
		     return;
     }
    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path );
    if (session)
     { Partage->com_http.liste_http_clients = g_slist_remove ( Partage->com_http.liste_http_clients, session );
       Info_new( __func__, Config.log_msrv, LOG_NOTICE, "sid '%s' ('%s', level %d) disconnected",
                 session->wtd_session, session->username, session->access_level );
       g_free(session);
       Info_new( __func__, Config.log_msrv, LOG_DEBUG,
                 "'%d' session left", g_slist_length(Partage->com_http.liste_http_clients) );
     }
    soup_server_message_set_status (msg, SOUP_STATUS_OK, NULL);
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
/* Http_rechercher_session: Recherche une session dans la liste des session                                                   */
/* Entrée: le message libsoup                                                                                                 */
/* Sortie: la session, ou NULL si pas trouvé                                                                                  */
/******************************************************************************************************************************/
 struct HTTP_CLIENT_SESSION *Http_rechercher_session_by_msg ( SoupServerMessage *msg )
  { struct HTTP_CLIENT_SESSION *result = NULL;

     if (!Partage->com_http.liste_http_clients) return(NULL);
     struct HTTP_CLIENT_SESSION *session = Partage->com_http.liste_http_clients->data;
     return(session);


    if ( Config.instance_is_master == FALSE )
     { static struct HTTP_CLIENT_SESSION Slave_session = { -1, "system_user", "internal device", "Watchdog Server", "none", "no_sid", 9, 0 };
       return(&Slave_session);
     }

    const char *cookies_char = soup_message_headers_get_list ( soup_server_message_get_request_headers ( msg ), "Cookie" );
    SoupCookie *cookie = soup_cookie_parse ( cookies_char, NULL );
    const char *name = soup_cookie_get_name (cookie);
    if (!strcmp(name,"wtd_session"))
     { gchar *wtd_session = soup_cookie_get_value(cookie);
       GSList *clients = Partage->com_http.liste_http_clients;
       while(clients)
        { struct HTTP_CLIENT_SESSION *session = clients->data;
          if (!strcmp(session->wtd_session, wtd_session))
           { result = session;
             break;
           }
           clients = g_slist_next ( clients );
        }
     }
    return(result);
  }
/******************************************************************************************************************************/
/* Http_print_request: affiche les données relatives à une requete                                                            */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 struct HTTP_CLIENT_SESSION *Http_print_request ( SoupServer *server, SoupServerMessage *msg, const char *path )
  { struct HTTP_CLIENT_SESSION *session = Http_rechercher_session_by_msg ( msg );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "sid '%s' (%s@%s, Level %d) : '%s'",
              (session ? session->wtd_session : "none"),
              (session ? session->username : "none"), soup_server_message_get_remote_host(msg),
              (session ? session->access_level : -1), path
            );
    return(session);
  }
/******************************************************************************************************************************/
/* Http_print_request: affiche les données relatives à une requete                                                            */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 gboolean Http_check_session ( SoupServerMessage *msg, struct HTTP_CLIENT_SESSION *session, gint min_access_level )
  { if (!session)
     { soup_server_message_set_status (msg, SOUP_STATUS_FORBIDDEN, "Not Connected");
       return(FALSE);
     }

    time(&session->last_request);
    if (session->access_level>=min_access_level) return(TRUE);
    soup_server_message_set_status (msg, SOUP_STATUS_FORBIDDEN, "Session Level forbidden");
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Http_traiter_ping: Répond aux requetes sur l'URI ping, et renouvelle le cookie de session                                  */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_traiter_ping ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data )
  { if (soup_server_message_get_method(msg) != SOUP_METHOD_GET)
     {	soup_server_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED, NULL);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path );
    if (!Http_check_session( msg, session, 0 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "JSon RootNode creation failed" );
       soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_node_add_string ( RootNode, "response", "pong" );
    Json_node_add_bool   ( RootNode, "Thread_run", Partage->com_msrv.Thread_run );
    Http_Send_json_response (msg, SOUP_STATUS_OK, NULL, RootNode );
  }
#warning a migrer coté API
#ifdef bouh
/******************************************************************************************************************************/
/* Http_Traiter_users_kill: Kill une session utilisateur                                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 void Http_traiter_search ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data )
  { if (soup_server_message_get_method(msg) != SOUP_METHOD_GET)
     {	soup_server_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 1 )) return;

    gchar *search;
    gpointer search_string = g_hash_table_lookup ( query, "search[value]" );
    if (!search_string) { search = g_strdup (""); }
                   else { search = Normaliser_chaine ( search_string ); }
    if (!search)
     { soup_server_message_set_status (msg, SOUP_STATUS_BAD_REQUEST, "Memory Error");
       return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "JSon RootNode creation failed" );
       soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
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
     { soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       Json_node_unref ( RootNode );
       g_free(search);
       return;
     }
    if (SQL_Select_to_json_node ( RootNode, NULL,
                                  "SELECT COUNT(*) AS recordsFiltered FROM dictionnaire "
                                  "WHERE tech_id LIKE '%%%s%%' OR acronyme LIKE '%%%s%%' OR libelle LIKE '%%%s%%' "
                                  "LIMIT %d OFFSET %d",
                                  search, search, search, length, start )==FALSE)
     { soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       Json_node_unref ( RootNode );
       g_free(search);
       return;
     }
    if (SQL_Select_to_json_node ( RootNode, "data",
                                  "SELECT * FROM dictionnaire "
                                  "WHERE tech_id LIKE '%%%s%%' OR acronyme LIKE '%%%s%%' OR libelle LIKE '%%%s%%' "
                                  "LIMIT %d OFFSET %d",
                                  search, search, search, length, start )==FALSE)
     { soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       Json_node_unref ( RootNode );
       g_free(search);
       return;
     }
    g_free(search);

    gchar *buf = Json_node_to_string ( RootNode );
    Json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_server_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
#endif
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Msg_to_Json ( SoupServerMessage *msg )
  { gsize taille;

    SoupMessageBody *body = soup_server_message_get_request_body ( msg );
    GBytes *buffer        = soup_message_body_flatten ( body );                                    /* Add \0 to end of buffer */
    JsonNode *request     = Json_get_from_string ( g_bytes_get_data ( buffer, &taille ) );
    g_bytes_unref(buffer);
    return(request);
  }
/******************************************************************************************************************************/
/* HTTP_Handle_request: Repond aux requests reçues                                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void HTTP_Handle_request_CB ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data )
  { SoupMessageHeaders *headers = soup_server_message_get_response_headers ( msg );
    soup_message_headers_append ( headers, "Cache-Control", "no-store, must-revalidate" );
    soup_message_headers_append ( headers, "Access-Control-Allow-Origin", "*" );
    soup_message_headers_append ( headers, "Access-Control-Allow-Methods", "*" );
    soup_message_headers_append ( headers, "Access-Control-Allow-Headers", "content-type, authorization, X-ABLS-DOMAIN" );
/*---------------------------------------------------- OPTIONS ---------------------------------------------------------------*/
    if (soup_server_message_get_method(msg) == SOUP_METHOD_OPTIONS)
     { soup_message_headers_append ( headers, "Access-Control-Max-Age", "86400" );
       Http_Send_json_response (msg, SOUP_STATUS_OK, NULL, NULL );
       return;
     }
/*------------------------------------------------------ GET -----------------------------------------------------------------*/

    if (soup_server_message_get_method(msg) == SOUP_METHOD_GET)
     {      if (!strcasecmp ( path, "/" ))           Http_traiter_status     ( server, msg, path, query, user_data );
       else if (!strcasecmp ( path, "/status" ))     Http_traiter_status     ( server, msg, path, query, user_data );
       else if (!strcasecmp ( path, "/dls/status" )) Http_traiter_dls_status ( server, msg, path, query, user_data );
       else if (!strcasecmp ( path, "/dls/run" ))    Http_traiter_dls_run    ( server, msg, path, query, user_data );
       else if (!strcasecmp ( path, "/get_do" ))     Http_traiter_get_do     ( server, msg, path, query );
       else { Http_Send_json_response (msg, SOUP_STATUS_NOT_IMPLEMENTED, NULL, NULL ); return; }
     }
    else if (soup_server_message_get_method(msg) == SOUP_METHOD_POST)
     { JsonNode *request = Http_Msg_to_Json ( msg );
       if (!request)
        { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Parsing Request Failed", NULL );
          return;
        }
            if (!strcasecmp ( path, "/dls/run/set" ))      Http_traiter_dls_run_set       ( server, msg, path, request );
       else if (!strcasecmp ( path, "/dls/run/acquitter")) Http_traiter_dls_run_acquitter ( server, msg, path, request );
       else if (!strcasecmp ( path, "/set_di"))            Http_traiter_set_di_post       ( server, msg, path, request );
       else if (!strcasecmp ( path, "/set_ai"))            Http_traiter_set_ai_post       ( server, msg, path, request );
       else if (!strcasecmp ( path, "/set_cde"))           Http_traiter_set_cde_post      ( server, msg, path, request );
       else if (!strcasecmp ( path, "/set_watchdog"))      Http_traiter_set_watchdog_post ( server, msg, path, request );
       else Http_Send_json_response (msg, SOUP_STATUS_NOT_FOUND, "Not found", NULL );
       Json_node_unref ( request );
     }
    else { Http_Send_json_response (msg, SOUP_STATUS_NOT_IMPLEMENTED, "Method not implemented", NULL ); return; }
  }
/******************************************************************************************************************************/
/* Run_HTTP: Thread principal                                                                                                 */
/* Entrée: une structure PROCESS                                                                                              */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_HTTP ( void )
  { GError *error = NULL;
    static gint last_pulse = 0;

    prctl(PR_SET_NAME, "W-HTTP", 0, 0, 0 );
    Partage->com_http.Thread_run = TRUE;                                                                /* Le thread tourne ! */
    Info_new( __func__, Partage->com_http.Thread_debug, LOG_NOTICE, "Demarrage . . . TID = %p", pthread_self() );

    struct stat sbuf;
    if ( stat ( HTTP_DEFAUT_FILE_CERT, &sbuf ) == -1 ||                                           /* Test présence du fichier */
         stat ( HTTP_DEFAUT_FILE_KEY, &sbuf ) == -1 )                                             /* Test présence du fichier */
     { gchar chaine[256];
       Info_new( __func__, Config.log_msrv, LOG_ERR,
                "unable to load '%s' and '%s' (error '%s'). Generating new ones.",
                 HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY, strerror(errno) );
       g_snprintf( chaine, sizeof(chaine),
                   "openssl req -subj '/C=FR/ST=FRANCE/O=ABLS-HABITAT/OU=PRODUCTION/CN=Watchdog Server on %s' -new -newkey rsa:2048 -sha256 -days 3650 -nodes -x509 -out '%s' -keyout '%s'",
                   g_get_host_name(), HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY );
       system( chaine );
     }

		  GTlsCertificate *cert = g_tls_certificate_new_from_files (HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY, &error);
	  	if (error)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Failed to load SSL Certificate '%s' and '%s'. Error '%s'",
                 HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY, error->message  );
       g_error_free(error);
       return;
     }

    SoupServer *socket = Partage->com_http.socket = soup_server_new( "server-header", "Watchdogd Ex-API Server", "tls-certificate", cert, NULL);
		  g_object_unref (cert);
    if (!socket)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "SoupServer new Failed !" );
       return;
     }
    Info_new( __func__, Config.log_msrv, LOG_INFO, "SSL Loaded with '%s' and '%s'", HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY );

    soup_server_add_handler ( socket, "/api/dls/status" ,    Http_traiter_dls_status, NULL, NULL );
    soup_server_add_handler ( socket, "/api/connect",        Http_traiter_connect, NULL, NULL );
    soup_server_add_handler ( socket, "/api/disconnect",     Http_traiter_disconnect, NULL, NULL );

    soup_server_add_handler ( socket, "/api/syn/show",       Http_traiter_syn_show, NULL, NULL );
    soup_server_add_handler ( socket, "/api/syn/clic",       Http_traiter_syn_clic, NULL, NULL );

    soup_server_add_handler ( socket, "/api/status",         Http_traiter_status, NULL, NULL );
    soup_server_add_handler ( socket, "/api/ping",           Http_traiter_ping, NULL, NULL );
    if (Config.instance_is_master==TRUE)
     { static gchar *protocols[] = { "live-motifs", NULL };
       soup_server_add_websocket_handler ( socket, "/api/live-motifs", NULL, protocols, Http_traiter_open_websocket_motifs_CB, NULL, NULL );
     }

    if (!soup_server_listen_all (socket, HTTP_DEFAUT_TCP_PORT, SOUP_SERVER_LISTEN_HTTPS, &error))
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "SoupServer Listen Failed '%s' !", error->message );
       g_error_free(error);
       soup_server_disconnect(socket);
       Partage->com_http.socket = NULL;
       return;
     }
    Info_new( __func__, Config.log_msrv, LOG_INFO, "HTTP SoupServer SSL Listen OK on port %d !", HTTP_DEFAUT_TCP_PORT );

    Partage->com_http.loop = g_main_loop_new (NULL, TRUE);

/********************************************* New API ************************************************************************/
		  cert = g_tls_certificate_new_from_files (HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY, &error);
	  	if (error)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Failed to load SSL Certificate '%s' and '%s'. Error '%s'",
                 HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY, error->message  );
       g_error_free(error);
       return;
     }

    socket = Partage->com_http.local_socket = soup_server_new( "server-header", "Watchdogd API Server", "tls-certificate", cert, NULL);
		  g_object_unref (cert);
    if (!socket)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "SoupServer new Failed !" );
       return;
     }
    Info_new( __func__, Config.log_msrv, LOG_INFO, "SSL Loaded with '%s' and '%s'", HTTP_DEFAUT_FILE_CERT, HTTP_DEFAUT_FILE_KEY );

    soup_server_add_handler ( socket, "/" , HTTP_Handle_request_CB, NULL, NULL );

    if (Config.instance_is_master)
     { static gchar *protocols[] = { "live-bus", NULL };
       soup_server_add_websocket_handler ( socket, "/ws_bus" , NULL, protocols, Http_traiter_open_websocket_for_slaves_CB, NULL, NULL );
     }

    if (!soup_server_listen_all (socket, 5559, SOUP_SERVER_LISTEN_HTTPS, &error))
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "SoupServer Listen Failed '%s' !", error->message );
       g_error_free(error);
       soup_server_disconnect(socket);
       Partage->com_http.local_socket = NULL;
       return;
     }
    Info_new( __func__, Config.log_msrv, LOG_INFO, "HTTP SoupServer SSL Listen OK on port %d !", 5559 );

    while(Partage->com_http.Thread_run == TRUE)
     { sched_yield();
       usleep(1000);
       Http_Envoyer_les_cadrans ();

       if ( Partage->top > last_pulse + 50 )
        { last_pulse = Partage->top;
          pthread_mutex_lock( &Partage->com_http.synchro );
          GSList *liste = Partage->com_http.liste_http_clients;
          while(liste)
           { struct HTTP_CLIENT_SESSION *client = liste->data;
             liste = g_slist_next ( liste );
             if (client->last_request + 864000 < Partage->top )
              { Partage->com_http.liste_http_clients = g_slist_remove ( Partage->com_http.liste_http_clients, client );
                Info_new( __func__, Config.log_msrv, LOG_INFO, "Session '%s' out of time", client->wtd_session );
                Http_destroy_session ( client );
              }
           }
          pthread_mutex_unlock( &Partage->com_http.synchro );
        }

       if (Partage->com_http.loop) g_main_context_iteration ( g_main_loop_get_context ( Partage->com_http.loop ), FALSE );
     }

    if (Partage->com_http.loop) g_main_loop_unref( Partage->com_http.loop );
    if (Partage->com_http.socket)
     { soup_server_disconnect ( Partage->com_http.socket );                                     /* Arret du serveur WebSocket */
       g_object_unref(Partage->com_http.socket);
     }
    if (Partage->com_http.local_socket)
     { soup_server_disconnect ( Partage->com_http.local_socket );
       g_object_unref(Partage->com_http.local_socket);
     }

    Info_new( __func__, Partage->com_http.Thread_debug, LOG_NOTICE, "HTTP Down (%p)", pthread_self() );
    Partage->com_http.TID = 0;                                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
