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
 #include <microhttpd.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <gnutls/gnutls.h>
 #include <gnutls/x509.h>
 #include <openssl/rand.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Http.h"

/**********************************************************************************************************/
/* Http_Lire_config : Lit la config Watchdog et rempli la structure mémoire                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 gboolean Http_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_http.lib->Thread_debug = FALSE;                                     /* Settings default parameters */
    Cfg_http.http_enable       = FALSE; 
    Cfg_http.http_port         = HTTP_DEFAUT_PORT_HTTP;
    Cfg_http.https_enable      = FALSE; 
    Cfg_http.https_port        = HTTP_DEFAUT_PORT_HTTPS;
    Cfg_http.authenticate      = TRUE; 
    Cfg_http.nbr_max_connexion = HTTP_DEFAUT_MAX_CONNEXION;
    g_snprintf( Cfg_http.https_cipher,    sizeof(Cfg_http.https_cipher),    "%s", HTTP_DEFAUT_HTTPS_CIPHER );
    g_snprintf( Cfg_http.https_file_cert, sizeof(Cfg_http.https_file_cert), "%s", HTTP_DEFAUT_FILE_CERT );
    g_snprintf( Cfg_http.https_file_key,  sizeof(Cfg_http.https_file_key),  "%s", HTTP_DEFAUT_FILE_KEY );
    g_snprintf( Cfg_http.https_file_ca,   sizeof(Cfg_http.https_file_ca),   "%s", HTTP_DEFAUT_FILE_CA  );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                     /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                "Http_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )       /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,                         /* Print Config */
                "Http_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "https_file_cert" ) )
        { g_snprintf( Cfg_http.https_file_cert, sizeof(Cfg_http.https_file_cert), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "https_file_ca" ) )
        { g_snprintf( Cfg_http.https_file_ca, sizeof(Cfg_http.https_file_ca), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "https_file_key" ) )
        { g_snprintf( Cfg_http.https_file_key, sizeof(Cfg_http.https_file_key), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "https_cipher" ) )
        { g_snprintf( Cfg_http.https_cipher, sizeof(Cfg_http.https_cipher), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "max_connexion" ) )
        { Cfg_http.nbr_max_connexion = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "http_enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_http.http_enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "http_port" ) )
        { Cfg_http.http_port = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "https_enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_http.https_enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "https_port" ) )
        { Cfg_http.https_port = atoi(valeur);  }
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
/**********************************************************************************************************/
/* Charger_certificat : Charge les certificats en mémoire                                                 */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static gboolean Charger_certificat ( void )
  { struct stat sbuf;
    gint fd;

    fd = open ( Cfg_http.https_file_cert, O_RDONLY);                          /* Chargement du certificat */
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading file certif %s failed (%s)",
                 Cfg_http.https_file_cert, strerror(errno) );
       if (fd!=-1) close(fd);
       return(FALSE);
     }
    Cfg_http.ssl_cert = (gchar *)g_try_malloc0( sbuf.st_size );
    if (!Cfg_http.ssl_cert)
     { close(fd);
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading certif %s : memory error (%s)",
                 Cfg_http.https_file_cert, strerror(errno) );
       return(FALSE);
     }
    if (read ( fd, Cfg_http.ssl_cert, sbuf.st_size ) < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading certif %s : read error (%s)",
                 Cfg_http.https_file_cert, strerror(errno) );
       return(FALSE);
     }
    close(fd);

    fd = open ( Cfg_http.https_file_key, O_RDONLY);                              /* Chargement de la clef */
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading file key %s failed (%s)",
                 Cfg_http.https_file_key, strerror(errno) );
       if (fd!=-1) close(fd);
       return(FALSE);
     }
    Cfg_http.ssl_key = (gchar *)g_try_malloc0( sbuf.st_size );
    if (!Cfg_http.ssl_key)
     { close(fd);
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading key %s : memory error (%s)",
                 Cfg_http.https_file_key, strerror(errno) );
       return(FALSE);
     }
    if (read ( fd, Cfg_http.ssl_key, sbuf.st_size ) < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading certif %s : read error (%s)",
                 Cfg_http.https_file_key, strerror(errno) );
       return(FALSE);
     }
    close(fd);

    fd = open ( Cfg_http.https_file_ca, O_RDONLY);           /* Chargement du CA pour truster les clients */
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading file CA %s failed (%s)",
                 Cfg_http.https_file_ca, strerror(errno) );
       if (fd!=-1) close(fd);
       return(FALSE);
     }
    Cfg_http.ssl_ca = (gchar *)g_try_malloc0( sbuf.st_size );
    if (!Cfg_http.ssl_ca)
     { close(fd);
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading CA %s : memory error (%s)",
                 Cfg_http.https_file_ca, strerror(errno) );
       return(FALSE);
     }
    if (read ( fd, Cfg_http.ssl_ca, sbuf.st_size ) < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading certif %s : read error (%s)",
                 Cfg_http.https_file_ca, strerror(errno) );
       return(FALSE);
     }
    close(fd);
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
             "Charger_certificat : Loading file CERT %s, KEY %s and CA %s successfull",
              Cfg_http.https_file_cert, Cfg_http.https_file_key, Cfg_http.https_file_ca );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Http_Add_response_header : Ajoute les header HTTP de connexion pour titanium                  */
/* Entrée: la repsonse MHD                                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Http_Add_response_header ( struct MHD_Response *response )
  { MHD_add_response_header ( response, "Access-Control-Allow-Origin", "*" );
    MHD_add_response_header ( response, "Connection", "close" );
  }
/**********************************************************************************************************/
/* Liberer_certificat: Libere la mémoire allouée précédemment pour stocker les certificats                */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Liberer_certificat ( void )
  { if ( Cfg_http.ssl_cert) g_free( Cfg_http.ssl_cert );
    if ( Cfg_http.ssl_key ) g_free( Cfg_http.ssl_key );
    if ( Cfg_http.ssl_ca )  g_free( Cfg_http.ssl_ca );
  }
/**********************************************************************************************************/
/* Satellite_update_infos : top les infos de la liste des connexions Satellite et maj les bits internes   */
/* Entrées: les informations du satellite                                                                 */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static struct HTTP_SESSION *Http_get_session ( const gchar *sid )
  { struct HTTP_SESSION *session = NULL;
    GSList *liste;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "Http_get_session : Searching for sid %s", sid );
    pthread_mutex_lock( &Cfg_http.lib->synchro );                        /* Ajout dans la liste a traiter */
    liste = Cfg_http.Liste_sessions;
    while ( liste )
     { session = (struct HTTP_SESSION *)liste->data;
       if ( ! g_strcmp0 ( session->sid, sid ) ) break;
       liste = liste->next;
     }
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
    if (liste) return(session);
          else return(NULL);
  }
/**********************************************************************************************************/
/* Http Liberer_session : Libere la mémoire réservée par la structure session de reception de la requete  */
/* Entrées : la session à libérer                                                                         */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void Http_Liberer_session ( struct HTTP_SESSION *session )
  { if (session->buffer) g_free(session->buffer);
    if (session->util)   g_free(session->util);
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
             "Http_Liberer_session : session close for SID %s", session->sid );
    g_free(session);                                                 /* Libération mémoire le cas échéant */
  }
/**********************************************************************************************************/
/* Http_Check_sessions : Fait tomber les sessions sur timeout                                             */
/* Entrées: néant                                                                                         */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void Http_Check_sessions ( void )
  { struct HTTP_SESSION *session = NULL;
    GSList *liste;
    pthread_mutex_lock( &Cfg_http.lib->synchro );                        /* Ajout dans la liste a traiter */
    liste = Cfg_http.Liste_sessions;
    while ( liste )
     { session = (struct HTTP_SESSION *)liste->data;
       if ( (session->last_top && Partage->top - session->last_top >= 6000) ||
            session->type == SESSION_TO_BE_CLEANED )
        { Cfg_http.Liste_sessions = g_slist_remove( Cfg_http.Liste_sessions, session );
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                   "Http_Check_sessions : closing SID %s", session->sid );
          Http_Liberer_session(session);
          liste = Cfg_http.Liste_sessions;
        }
       else liste = liste->next;
     }
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
  }
/**********************************************************************************************************/
/* Http_Send_file: Enoie un fichier au client                                                             */
/* Entrées: La session, et le nom de fichier                                                              */
/* Sortie : void                                                                                          */
/**********************************************************************************************************/
 static gint Http_Send_file ( struct HTTP_SESSION *session, struct MHD_Connection *connection,
                              gchar *file, gchar *content_type )
  { struct MHD_Response *response;
    struct stat sbuf;
    gint fd;
    fd = open (file, O_RDONLY);
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Send_file : Error sending file %s to %s: %s",
                file, (session ? session->sid : "Unathenticated"), strerror(errno) );
       if (fd!=-1) close(fd);
       response = MHD_create_response_from_buffer ( strlen (RESPONSE_INTERNAL_ERROR)+1,
                                                    RESPONSE_INTERNAL_ERROR, MHD_RESPMEM_PERSISTENT );
       if (response == NULL) return(MHD_NO);
       Http_Add_response_header ( response );
       MHD_queue_response ( connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response );
       MHD_destroy_response (response);
       return(MHD_YES);
     }
    response = MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
    MHD_add_response_header(response, "Content-Type", content_type);
    Http_Add_response_header(response);
    MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "Http_Send_file : %s sent to session '%s'",
              file, (session ? session->sid : "Unathenticated") );
    return MHD_YES;
  }
/**********************************************************************************************************/
/* Get_client_cert : Recupere le certificat client depuis la session TLS                                  */
/* Entrées : la session TLS                                                                               */
/* Sortie  : TRUE si OK, FALSE si erreur                                                                  */
/**********************************************************************************************************/
 static struct HTTP_SESSION *Http_new_session( struct MHD_Connection *connection, const char *url, 
                                               const char *method, const char *version, size_t *upload_data_size,
                                               void **con_cls )
  { const union MHD_ConnectionInfo *info;
    struct HTTP_SESSION *session;
    struct sockaddr *client_addr;
    gint retour, size, cpt;
    void *tls_session;
    gchar sid[32];

    client_addr = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    if (client_addr->sa_family == AF_INET)  size = sizeof(struct sockaddr_in);
    else
    if (client_addr->sa_family == AF_INET6) size = sizeof(struct sockaddr_in6);
    else
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "New_session :  MHD_CONNECTION_INFO_CLIENT_ADDRESS failed, wrong family" );
       return(NULL);
     }

    session = (struct HTTP_SESSION *) g_try_malloc0 ( sizeof( struct HTTP_SESSION ) );
    if (!session)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "New_session: Memory Alloc ERROR session" );
       return(NULL);
     }
    session->type = SESSION_INIT;
    session->last_top = Partage->top;

    retour = getnameinfo( client_addr, size,
                          session->client_host,    sizeof(session->client_host),
                          session->client_service, sizeof(session->client_service),
                          NI_NUMERICHOST | NI_NUMERICSERV );
    if (retour) 
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "New_session : GetName failed : %s", gai_strerror(retour) );
       g_free(session);
       return(NULL);
     }

    info = MHD_get_connection_info ( connection, MHD_CONNECTION_INFO_CIPHER_ALGO );
    if ( info ) { session->ssl_algo = info->cipher_algorithm; }

    info = MHD_get_connection_info ( connection, MHD_CONNECTION_INFO_PROTOCOL );
    if ( info ) { session->ssl_proto = info->protocol; }

    info = MHD_get_connection_info ( connection, MHD_CONNECTION_INFO_GNUTLS_SESSION );
    if ( info ) { tls_session = info->tls_session; }
           else { tls_session = NULL; }

    g_snprintf( session->user_agent, sizeof(session->user_agent), "%s",
                MHD_lookup_connection_value (connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_USER_AGENT) );

    g_snprintf( session->origine,    sizeof(session->origine),    "%s",
                MHD_lookup_connection_value (connection, MHD_HEADER_KIND, "Origin") );

    RAND_pseudo_bytes( (guchar *)sid, sizeof(sid) );                     /* Récupération d'un nouveau SID */
    for (cpt=0; cpt<sizeof(sid); cpt++)                                    /* Mise en forme au format HEX */
     { g_snprintf( &session->sid[2*cpt], 3, "%02X", (guchar)sid[cpt] ); }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "New_session : Creation session '%s'", session->sid );

    if (tls_session)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "New_session : New ANON HTTPS %s %s %s request (Payload size %d) from Host=%s/Service=%s"
                 " (Cipher=%s/Proto=%s). User-Agent=%s. Origin=%s",
                  method, url, version, (upload_data_size ? *upload_data_size : 0),
                  session->client_host, session->client_service,
                  gnutls_cipher_get_name (session->ssl_algo), gnutls_protocol_get_name (session->ssl_proto),
                  session->user_agent, session->origine
                );
     }
    else Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                  "New_session : New ANON HTTP  %s %s %s request (Payload size %d) from Host=%s/Service=%s. User-Agent=%s. Origin=%s",
                   method, url, version, *upload_data_size,
                   session->client_host, session->client_service, session->user_agent, session->origine
                );
    *con_cls = session;
    pthread_mutex_lock( &Cfg_http.lib->synchro );                        /* Ajout dans la liste a traiter */
    Cfg_http.Liste_sessions = g_slist_prepend( Cfg_http.Liste_sessions, session );
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
    return(session);
  }
/**********************************************************************************************************/
/* Get_client_cert : Recupere le certificat client depuis la session TLS                                  */
/* Entrées : la session TLS                                                                               */
/* Sortie  : TRUE si OK, FALSE si erreur                                                                  */
/**********************************************************************************************************/
 static void Http_Log_request ( struct MHD_Connection *connection, const char *url, 
                                const char *method, const char *version, size_t *upload_data_size,
                                void **con_cls )
  { const union MHD_ConnectionInfo *info;
    struct HTTP_SESSION session;
    struct sockaddr *client_addr;
    void *tls_session;
    gint retour, size;

    client_addr = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    if (client_addr->sa_family == AF_INET)  size = sizeof(struct sockaddr_in);
    else
    if (client_addr->sa_family == AF_INET6) size = sizeof(struct sockaddr_in6);
    else
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Log_request :  MHD_CONNECTION_INFO_CLIENT_ADDRESS failed, wrong family" );
       return;
     }

    retour = getnameinfo( client_addr, size,
                          session.client_host,    sizeof(session.client_host),
                          session.client_service, sizeof(session.client_service),
                          NI_NUMERICHOST | NI_NUMERICSERV );
    if (retour) 
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Log_request : GetName failed : %s", gai_strerror(retour) );
       return;
     }

    info = MHD_get_connection_info ( connection, MHD_CONNECTION_INFO_GNUTLS_SESSION );
    if ( info ) { tls_session = info->tls_session; }
           else { tls_session = NULL; }

    g_snprintf( session.user_agent, sizeof(session.user_agent), "%s",
                MHD_lookup_connection_value (connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_USER_AGENT) );

    g_snprintf( session.origine,    sizeof(session.origine),    "%s",
                MHD_lookup_connection_value (connection, MHD_HEADER_KIND, "Origin") );

    if (tls_session)
     { info = MHD_get_connection_info ( connection, MHD_CONNECTION_INFO_CIPHER_ALGO );
       if ( info ) { session.ssl_algo = info->cipher_algorithm; }

       info = MHD_get_connection_info ( connection, MHD_CONNECTION_INFO_PROTOCOL );
       if ( info ) { session.ssl_proto = info->protocol; }

       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "Http_Log_request : New HTTPS %s %s %s request (Payload size %d) from Host=%s/Service=%s"
                 " (Cipher=%s/Proto=%s). User-Agent=%s. Origin=%s",
                  method, url, version, (upload_data_size ? *upload_data_size : 0),
                  session.client_host, session.client_service,
                  gnutls_cipher_get_name (session.ssl_algo), gnutls_protocol_get_name (session.ssl_proto),
                  session.user_agent, session.origine
                );
     }
    else Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                  "Http_Log_request : New HTTP  %s %s %s request (Payload size %d) from Host=%s/Service=%s. User-Agent=%s. Origin=%s",
                   method, url, version, *upload_data_size,
                   session.client_host, session.client_service, session.user_agent, session.origine
                 );
  }
/**********************************************************************************************************/
/* Http request_CB : Renvoi une reponse suite a une demande d'un client (appellée par libmicrohttpd)      */
/* Entrées : le contexte, le message, l'URL                                                               */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static gint Http_request_CB ( void *cls, struct MHD_Connection *connection, 
                               const char *url, 
                               const char *method, const char *version, 
                               const char *upload_data, 
                               size_t *upload_data_size, void **con_cls )
  { const char *Wrong_method     = "<html><body>Wrong method. Sorry... </body></html>";
    const char *Options_response = "<html><body>This is the response to HTTP OPTIONS Method!..</body></html>";
    struct HTTP_SESSION *session = NULL;
    struct MHD_Response *response;

    Http_Log_request(connection, url, method, version, upload_data_size, con_cls);

/************************************* Ici, pas de session en cours, reponse directe **********************/
    if ( ! strcasecmp( method, MHD_HTTP_METHOD_GET ) && ! strcasecmp ( url, "/favicon.ico" ) )
     { return ( Http_Send_file (NULL, connection, "WEB/favicon.gif", "image/gif") ); }
    else if ( ! strcasecmp( method, MHD_HTTP_METHOD_GET ) && ! strcasecmp ( url, "/getgif.ws" ) )
     { return ( Http_Traiter_request_getgif ( connection ) ); }               /* Traitement de la requete */
    else if ( ! strcasecmp( method, MHD_HTTP_METHOD_GET ) && ! strcasecmp ( url, "/status.ws" ) )
     { return( Http_Traiter_request_getstatus ( connection ) ); }
    else if ( ! strcasecmp( method, MHD_HTTP_METHOD_OPTIONS ) )
     { response = MHD_create_response_from_buffer ( strlen (Options_response)+1,
                                                   (void*) Options_response, MHD_RESPMEM_PERSISTENT);
       if (response == NULL) return(MHD_NO);
       MHD_add_response_header ( response, "Access-Control-Allow-Origin", "*" );
       MHD_add_response_header ( response, "Access-Control-Allow-Methods", "GET, POST" );
       Http_Add_response_header ( response );
       MHD_queue_response (connection, MHD_HTTP_OK, response);
       MHD_destroy_response (response);
       return MHD_YES;
     }
    else if ( strcasecmp( method, MHD_HTTP_METHOD_GET ) && strcasecmp( method, MHD_HTTP_METHOD_POST ) )
     { response = MHD_create_response_from_buffer ( strlen (Wrong_method)+1,
                                                   (void*) Wrong_method, MHD_RESPMEM_PERSISTENT);
       if (response == NULL) return(MHD_NO);
       Http_Add_response_header ( response );
       MHD_queue_response ( connection, MHD_HTTP_METHOD_NOT_ALLOWED, response);     /* Method not allowed */
       MHD_destroy_response (response);
       return MHD_YES;
     }
/*-------------------------------- Récupération des login / Mot de passe ---------------------------------*/
    else if ( ! strcasecmp( method, MHD_HTTP_METHOD_POST ) && ! strcasecmp ( url, "/postlogin.ws" ) )
     { gchar chaine[128], username[128], password[128], **splited, **couple;
       struct HTTP_SESSION *session;
       if ((*upload_data_size == 0) & (*con_cls == NULL))
        { *con_cls = Http_new_session(connection, url, method, version, upload_data_size, con_cls);
          if (!*con_cls)
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                      "Http_request_CB : Error in Prepare_session. Disconnect" );
             return(MHD_NO);
           }
          return(MHD_YES);                                                    /* Attente du premier chunk */
        }
       session = *con_cls;
       if (*upload_data_size != 0)                                                /* Transfert en cours ? */
        { gchar *new_buffer;
          new_buffer = g_try_realloc( session->buffer, 1 + session->buffer_size + *upload_data_size );
          if (!new_buffer || session->buffer_size > 128)
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                      "Http_request_CB: Memory Alloc ERROR realloc buffer" );
             g_free(session->buffer);
             session->buffer = NULL;
             session->type = SESSION_TO_BE_CLEANED;
             return(MHD_NO);
           } else session->buffer = new_buffer;
          memcpy ( session->buffer + session->buffer_size, upload_data, *upload_data_size );   /* Recopie */
          session->buffer_size += *upload_data_size;
          *upload_data_size = 0;           /* Indique à MHD que l'on a traité l'ensemble des octets recus */
          return(MHD_YES);                                        /* On demande de continuer le transfert */
        }
/*-------------------------------- Fin de transfert. On découpe le buffer d'entré ------------------------*/
       session->buffer[session->buffer_size] = 0;                                /* Caractère nul d'arret */
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "Http_request_CB: Buffer recu %s for session %s", session->buffer, session->sid );
       memset( username, 0, sizeof(username) );
       memset( password, 0, sizeof(password) );
       splited = g_strsplit( session->buffer, "&", 2 );
       g_free(session->buffer);                                         /* Libération du buffer d'origine */
       session->buffer = NULL;
       session->buffer_size = 0;
       if (splited[0])
        { gchar *new_string;
          couple = g_strsplit( splited[0], "=", 2 );
          if ( couple[0] && strcmp( couple[0], "remote_username" )==0 )
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "Http_request_CB: Login = %s for session %s", couple[1], session->sid );
             new_string = g_uri_unescape_string ( couple[1], NULL );
             g_snprintf( username , sizeof(username), "%s", new_string );
             g_free(new_string);
           }
          g_strfreev( couple );
          couple = g_strsplit( splited[1], "=", 2 );
          if ( couple[0] && strcmp( couple[0], "remote_password" )==0 )
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "Http_request_CB: Password received for session %s", session->sid );
             new_string = g_uri_unescape_string ( couple[1], NULL );
             g_snprintf( password , sizeof(password), "%s", new_string );
             g_free(new_string);
           }
          g_strfreev( couple );
        }
       g_strfreev( splited );
/*------------------------------------- Checking credentials ---------------------------------------------*/
       session->util = Rechercher_utilisateurDB_by_name( username );
       if ( !session->util )
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                   "Http_request_CB: User %s unknown for session %s", username, session->sid );
          response = MHD_create_response_from_buffer ( strlen ("! Wrong Credentials !"),
                                                       (void*) "! Wrong Credentials !", MHD_RESPMEM_PERSISTENT);
          if (response == NULL) return(MHD_NO);
          Http_Add_response_header ( response );
          MHD_queue_response (connection, MHD_HTTP_FORBIDDEN, response);
          MHD_destroy_response (response);
          session->type = SESSION_TO_BE_CLEANED;                     /* On demande la purge de la session */
          return(MHD_YES);
        }

       if ( Check_utilisateur_password( session->util, password ) == FALSE )
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                   "Http_request_CB: Wrong password for user %s for session %s", username, session->sid );
          response = MHD_create_response_from_buffer ( strlen ("! Wrong Credentials !"),
                                                       (void*) "! Wrong Credentials !", MHD_RESPMEM_PERSISTENT);
          if (response == NULL) return(MHD_NO);
          Http_Add_response_header ( response );
          MHD_queue_response (connection, MHD_HTTP_FORBIDDEN, response);
          MHD_destroy_response (response);
          session->type = SESSION_TO_BE_CLEANED;                     /* On demande la purge de la session */
          return(MHD_YES);
        }

       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                "Http_request_CB: User %s authenticated for session %s", session->util->nom, session->sid );
          
       session->type = SESSION_AUTH;
       session->buffer = NULL;
       session->buffer_size = 0;
       response = MHD_create_response_from_buffer ( strlen (session->sid),
                                                   (void*) session->sid, MHD_RESPMEM_PERSISTENT);
       if (response == NULL) return(MHD_NO);
       g_snprintf ( chaine, sizeof(chaine), "sid=%s; ", session->sid );
       MHD_add_response_header (response, MHD_HTTP_HEADER_SET_COOKIE, chaine );
       Http_Add_response_header ( response );
       MHD_queue_response (connection, MHD_HTTP_OK, response);
       MHD_destroy_response (response);
       return (MHD_YES);
     }
/************************************* A parti d'ici, la session est necessaire ***************************/
    session = Http_get_session ( MHD_lookup_connection_value (connection, MHD_COOKIE_KIND, "sid") );
    if (!session)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_request_CB : Get session failed for URL %s", url );
       return ( Http_Send_file (session, connection, "WEB/prelogin.html", "text/html") );
     }
                                                           /* On a une session, mais est-on authentifié ? */
    if (session->type != SESSION_AUTH)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_request_CB : Session_type != AUTH for %s", url );
       return(MHD_NO);
     }

    if (session->util == NULL)                                                    /* Si pas authentifié ! */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_request_CB : Non authenticated Request %s", url );
       return(MHD_NO);
     }

    session->last_top = Partage->top;

    if ( ! strcasecmp( method, MHD_HTTP_METHOD_GET ) && ! strcasecmp ( url, "/" ) )
     { return ( Http_Send_file (*con_cls, connection, "WEB/ui.html", "text/html") ); }
    else if ( ! strcasecmp( method, MHD_HTTP_METHOD_GET ) && ! strcasecmp ( url, "/getsyn.ws" ) )
     { if ( Http_Traiter_request_getsyn ( session, connection ) == FALSE)     /* Traitement de la requete */
        { response = MHD_create_response_from_buffer ( strlen (RESPONSE_INTERNAL_ERROR)+1,
                                                       RESPONSE_INTERNAL_ERROR, MHD_RESPMEM_PERSISTENT);
          if (response == NULL) return(MHD_NO);
          Http_Add_response_header ( response );
          MHD_queue_response ( connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
          MHD_destroy_response (response);
        }
       return MHD_YES;
     }
    else if ( ! strcasecmp( method, MHD_HTTP_METHOD_GET ) && ! strcasecmp ( url, "/setm.ws" ) )
     { if ( Http_Traiter_request_setm ( session, connection ) == FALSE )      /* Traitement de la requete */
        { response = MHD_create_response_from_buffer ( strlen (RESPONSE_INTERNAL_ERROR)+1,
                                                       RESPONSE_INTERNAL_ERROR, MHD_RESPMEM_PERSISTENT);
          if (response == NULL) return(MHD_NO);
          Http_Add_response_header ( response );
          MHD_queue_response ( connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
          MHD_destroy_response (response);
        }
       return MHD_YES;
     }
    return MHD_NO;
  }
/**********************************************************************************************************/
/* Http cleanup : Termine une connexion cliente (appellée par libmicrohttpd)                              */
/* Entrées : le contexte, le message, l'URL                                                               */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void Http_cleanup_CB ( void *cls, struct MHD_Connection *connection, void **con_cls,
                               enum MHD_RequestTerminationCode tcode )
  { struct HTTP_SESSION *session;
    session = *con_cls;
    if (!session) return;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
              "Http_cleanup_CB: fin de traitement de la requete session '%s'. Code retour %03d",
               session->sid, tcode );

  }
/**********************************************************************************************************/
/* Http_MHD_debug : fonction appellé pour debugger le Daemon MHD                                          */
/**********************************************************************************************************/
 static void Http_MHD_debug( void *arg, const char *fmt, va_list ap )
  { gchar chaine[256];
    g_snprintf(chaine, sizeof(chaine), "Http_MHD_debug : %s", fmt );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
              chaine, ap );
  }
/**********************************************************************************************************/
/* Run_thread: Thread principal                                                                           */
/* Entrée: une structure LIBRAIRIE                                                                        */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { prctl(PR_SET_NAME, "W-HTTP", 0, 0, 0 );
    memset( &Cfg_http, 0, sizeof(Cfg_http) );                   /* Mise a zero de la structure de travail */
    Cfg_http.lib = lib;                        /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_http.lib->TID = pthread_self();                                 /* Sauvegarde du TID pour le pere */
    Http_Lire_config ();                                /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );

    g_snprintf( Cfg_http.lib->admin_prompt, sizeof(Cfg_http.lib->admin_prompt), "http" );
    g_snprintf( Cfg_http.lib->admin_help,   sizeof(Cfg_http.lib->admin_help),   "Manage communications with Http Devices" );

    if (!Cfg_http.http_enable && !Cfg_http.https_enable)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread Http/Https not enable in config. Shutting Down %p", pthread_self() );
       goto end;
     }

    if (Cfg_http.http_enable)
     { Cfg_http.http_server = MHD_start_daemon ( MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
                                                 Cfg_http.http_port, NULL, NULL, 
                                                &Http_request_CB, NULL,
                                                 MHD_OPTION_NOTIFY_COMPLETED, Http_cleanup_CB, NULL,
                                                 MHD_OPTION_CONNECTION_LIMIT, Cfg_http.nbr_max_connexion,
                                                /* MHD_OPTION_PER_IP_CONNECTION_LIMIT, 100, */
                                                 MHD_OPTION_CONNECTION_TIMEOUT, 60,
                                                 MHD_OPTION_EXTERNAL_LOGGER, Http_MHD_debug, NULL,
                                                 MHD_OPTION_END);
       if (!Cfg_http.http_server)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "Run_thread: MHDServer HTTP creation error (%s). Shutting Down %p",
                    strerror(errno), pthread_self() );
        }
       else
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                   "Run_thread: MHDServer HTTP OK. Listening on port %d", Cfg_http.http_port );
        }
     }

    if ( Cfg_http.https_enable && Charger_certificat() )
     { Cfg_http.https_server = MHD_start_daemon ( MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL | MHD_USE_DEBUG,
                                                  Cfg_http.https_port, NULL, NULL, 
                                                 &Http_request_CB, NULL,
                                                  MHD_OPTION_NOTIFY_COMPLETED, Http_cleanup_CB, NULL,
                                                  MHD_OPTION_CONNECTION_LIMIT, Cfg_http.nbr_max_connexion,
                                                 /* MHD_OPTION_PER_IP_CONNECTION_LIMIT, 100, */
                                                  MHD_OPTION_CONNECTION_TIMEOUT, 60,
                                                  MHD_OPTION_HTTPS_MEM_CERT, Cfg_http.ssl_cert,
                                                  MHD_OPTION_HTTPS_MEM_KEY,  Cfg_http.ssl_key,
                                                 /* MHD_OPTION_HTTPS_MEM_TRUST, Cfg_http.ssl_ca,/* Require Client SSL */
                                                  MHD_OPTION_HTTPS_PRIORITIES, Cfg_http.https_cipher,
                                                  MHD_OPTION_EXTERNAL_LOGGER, Http_MHD_debug, NULL,
                                                  MHD_OPTION_END);
       if (!Cfg_http.https_server)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "Run_thread: MHDServer HTTPS creation error (%s). Shutting Down %p",
                    strerror(errno), pthread_self() );
        }
       else
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                   "Run_thread: MHDServer HTTPS OK. Listening on port %d", Cfg_http.https_port );
        }
     }

    if (!Cfg_http.http_server && !Cfg_http.https_server) goto end;             /* si erreur de chargement */
#ifdef bouh
    Abonner_distribution_message ( Http_Gerer_message );   /* Abonnement à la diffusion des messages */
    Abonner_distribution_sortie  ( Http_Gerer_sortie );     /* Abonnement à la diffusion des sorties */

#endif
    Cfg_http.lib->Thread_run = TRUE;                                                /* Le thread tourne ! */
    while(Cfg_http.lib->Thread_run == TRUE)                              /* On tourne tant que necessaire */
     { static gint last_top = 0;
       usleep(10000);
       sched_yield();

       if (Cfg_http.lib->Thread_sigusr1)                                  /* A-t'on recu un signal USR1 ? */
        { int nbr;
          pthread_mutex_lock( &Cfg_http.lib->synchro );                  /* Ajout dans la liste a traiter */
          nbr = g_slist_length( Cfg_http.Liste_sessions );
          pthread_mutex_unlock( &Cfg_http.lib->synchro );
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                   "Run_thread: SIGUSR1. %03d sessions", nbr );
          Cfg_http.lib->Thread_sigusr1 = FALSE;
        }

       if ( last_top + 300 <= Partage->top )                                    /* Toutes les 30 secondes */
        { Http_Check_sessions ();
          last_top = Partage->top;
        }
     }

    if (Cfg_http.http_server)  MHD_stop_daemon (Cfg_http.http_server);          /* Arret des serveurs MHD */
    if (Cfg_http.https_server)
     { MHD_stop_daemon (Cfg_http.https_server);
       Liberer_certificat();
     }

    pthread_mutex_lock( &Cfg_http.lib->synchro );                        /* Ajout dans la liste a traiter */
    while (Cfg_http.Liste_sessions)                                   /* Libération des infos de sessions */
     { struct HTTP_SESSION *session;
       session = (struct HTTP_SESSION *)Cfg_http.Liste_sessions->data;
       Cfg_http.Liste_sessions = g_slist_remove( Cfg_http.Liste_sessions, session );
       Http_Liberer_session ( session );
     }
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
end:
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
             "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_http.lib->Thread_run = FALSE;                                       /* Le thread ne tourne plus ! */
    Cfg_http.lib->TID = 0;                                  /* On indique au http que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
