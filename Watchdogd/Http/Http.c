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
 #include <string.h>
 #include <unistd.h>
 #include <microhttpd.h>
 #include <libxml/xmlwriter.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <netdb.h>
 #include <gnutls/gnutls.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Http.h"

/**********************************************************************************************************/
/* Http_Lire_config : Lit la config Watchdog et rempli la structure mémoire                             */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Http_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Http_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_http.lib->Thread_debug = g_key_file_get_boolean ( gkf, "HTTP", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */
    Cfg_http.nbr_max_connexion = g_key_file_get_integer ( gkf, "HTTP", "max_connexion", NULL );
    if ( !Cfg_http.nbr_max_connexion) Cfg_http.nbr_max_connexion = DEFAUT_MAX_CONNEXION;
    Cfg_http.http_enable       = g_key_file_get_boolean ( gkf, "HTTP", "http_enable", NULL ); 
    Cfg_http.http_port         = g_key_file_get_integer ( gkf, "HTTP", "http_port", NULL );
    Cfg_http.https_enable      = g_key_file_get_boolean ( gkf, "HTTP", "https_enable", NULL ); 
    Cfg_http.https_port        = g_key_file_get_integer ( gkf, "HTTP", "https_port", NULL );

    chaine                     = g_key_file_get_string  ( gkf, "HTTP", "https_file_cert", NULL );
    if (chaine)
     { g_snprintf( Cfg_http.https_file_cert, sizeof(Cfg_http.https_file_cert), "%s", chaine ); g_free(chaine); }
    else
     { g_snprintf( Cfg_http.https_file_cert, sizeof(Cfg_http.https_file_cert), "%s", FICHIER_CERTIF_SERVEUR  ); }

    chaine                     = g_key_file_get_string  ( gkf, "HTTP", "https_file_key", NULL );
    if (chaine)
     { g_snprintf( Cfg_http.https_file_key, sizeof(Cfg_http.https_file_key), "%s", chaine ); g_free(chaine); }
    else
     { g_snprintf( Cfg_http.https_file_key, sizeof(Cfg_http.https_file_key), "%s", FICHIER_CERTIF_CLEF_SERVEUR  ); }

    chaine                     = g_key_file_get_string  ( gkf, "HTTP", "https_file_ca", NULL );
    if (chaine)
     { g_snprintf( Cfg_http.https_file_ca, sizeof(Cfg_http.https_file_ca), "%s", chaine ); g_free(chaine); }
    else
     { g_snprintf( Cfg_http.https_file_ca, sizeof(Cfg_http.https_file_ca), "%s", FICHIER_CERTIF_CA  ); }

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Http_Liberer_config : Libere la mémoire allouer précédemment pour lire la config http                  */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Http_Liberer_config ( void )
  {
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
                "Charger_certificat : Loading certif %s into memory failed (%s)",
                 Cfg_http.https_file_cert, strerror(errno) );
       return(FALSE);
     }
    read ( fd, Cfg_http.ssl_cert, sbuf.st_size );
    close(fd);

    fd = open ( Cfg_http.https_file_key, O_RDONLY);                              /* Chargement de la clef */
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading file key %s failed (%s)",
                 Cfg_http.https_file_cert, strerror(errno) );
       if (fd!=-1) close(fd);
       return(FALSE);
     }
    Cfg_http.ssl_key = (gchar *)g_try_malloc0( sbuf.st_size );
    if (!Cfg_http.ssl_key)
     { close(fd);
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Charger_certificat : Loading key %s into memory failed (%s)",
                 Cfg_http.https_file_key, strerror(errno) );
       return(FALSE);
     }
    read ( fd, Cfg_http.ssl_key, sbuf.st_size );
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
                "Charger_certificat : Loading CA %s into memory failed (%s)",
                 Cfg_http.https_file_ca, strerror(errno) );
       return(FALSE);
     }
    read ( fd, Cfg_http.ssl_ca, sbuf.st_size );
    close(fd);
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
             "Charger_certificat : Loading file CERT %s, KEY %s and CA %s successfull",
              Cfg_http.https_file_cert, Cfg_http.https_file_key, Cfg_http.https_file_ca );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Http_Liberer_config : Libere la mémoire allouer précédemment pour lire la config http                  */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Liberer_certificat ( void )
  { if ( Cfg_http.ssl_cert) g_free( Cfg_http.ssl_cert );
    if ( Cfg_http.ssl_key ) g_free( Cfg_http.ssl_key );
    if ( Cfg_http.ssl_ca )  g_free( Cfg_http.ssl_ca );
  }
/**********************************************************************************************************/
/* Http_Gerer_message: Fonction d'abonné appellé lorsqu'un message est disponible.                        */
/* Entrée: une structure CMD_TYPE_HISTO                                                                   */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Http_Gerer_message ( struct CMD_TYPE_MESSAGE *msg )
  { gint taille;
#ifdef bouh
    pthread_mutex_lock( &Cfg_http.lib->synchro );                      /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_http.Liste_message );
    pthread_mutex_unlock( &Cfg_http.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                "Http_Gerer_message: DROP message %d (length = %d > 150)", msg->num, taille);
       g_free(msg);
       return;
     }

    pthread_mutex_lock ( &Cfg_http.lib->synchro );
    Cfg_http.Liste_message = g_slist_append ( Cfg_http.Liste_message, msg );      /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_http.lib->synchro );

#endif
  }
/**********************************************************************************************************/
/* Http_Gerer_sortie: Ajoute une demande d'envoi RF dans la liste des envois RFXCOM                     */
/* Entrées: le numéro de la sortie                                                                        */
/**********************************************************************************************************/
 void Http_Gerer_sortie( gint num_a )                                    /* Num_a est l'id de la sortie */
  { gint taille;
#ifdef bouh
    pthread_mutex_lock( &Cfg_http.lib->synchro );              /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_http.Liste_sortie );
    pthread_mutex_unlock( &Cfg_http.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                "Http_Gerer_sortie: DROP sortie %d (length = %d > 150)", num_a, taille );
       return;
     }

    pthread_mutex_lock( &Cfg_http.lib->synchro );       /* Ajout dans la liste de tell a traiter */
    Cfg_http.Liste_sortie = g_slist_prepend( Cfg_http.Liste_sortie, GINT_TO_POINTER(num_a) );
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
#endif
  }

/**********************************************************************************************************/
/* Traiter_dynxml: Traite une requete sur l'URI dynxml                                                    */
/* Entrées: la connexion MHD                                                                              */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void Traiter_dynxml ( struct MHD_Connection *connection )
  { static gint i = 1;
    struct MHD_Response *response;
    int rc;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;

    /* Create a new XML buffer, to which the XML document will be
     * written */
    buf = xmlBufferCreate();
    if (buf == NULL) {
        printf("testXmlwriterMemory: Error creating the xml buffer\n");
        return;
    }

    /* Create a new XmlWriter for memory, with no compression.
     * Remark: there is no compression for this kind of xmlTextWriter */
    writer = xmlNewTextWriterMemory(buf, 0);
    if (writer == NULL) {
        printf("testXmlwriterMemory: Error creating the xml writer\n");
        return;
    }

    /* Start the document with the xml default for the version,
     * encoding ISO 8859-1 and the default for the standalone
     * declaration. */
    rc = xmlTextWriterStartDocument(writer, NULL, "UTF-8", "yes" );
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterStartDocument\n");
        return;
    }

    /* Start an element named "EXAMPLE". Since thist is the first
     * element, this will be the root element of the document. */
    rc = xmlTextWriterStartElement(writer, "testbalise");
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
        return;
    }

    rc = xmlTextWriterWriteComment(writer, "commentaire !!");
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterWriteComment\n");
        return;
    }

    /* Start an element named "ORDER" as child of EXAMPLE. */
    rc = xmlTextWriterStartElement(writer, "element");
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
        return;
    }

    /* Add an attribute with name "version" and value "1.0" to ORDER. */
    rc = xmlTextWriterWriteAttribute(writer, "version",
                                     "1.0");
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
        return;
    }

    /* Add an attribute with name "xml:lang" and value "de" to ORDER. */
    rc = xmlTextWriterWriteAttribute(writer, "xml:lang",
                                     "de");
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
        return;
    }

    rc = xmlTextWriterWriteFormatComment(writer,
		     "This is another comment with special chars: %d",
                                         i++ );
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterWriteFormatComment\n");
        return;
    }

    /* Start an element named "HEADER" as child of ORDER. */
    rc = xmlTextWriterStartElement(writer, "sselement");
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
        return;
    }

    /* Write an element named "X_ORDER_ID" as child of HEADER. */
    rc = xmlTextWriterWriteFormatElement(writer, "parametre1",
                                         "%010d", 22101980 );
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterWriteFormatElement\n");
        return;
    }

    /* Write an element named "CUSTOMER_ID" as child of HEADER. */
    rc = xmlTextWriterWriteFormatElement(writer, "viral_alert",
                                         "%d fois", i++);
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterWriteFormatElement\n");
        return;
    }

    /* Close the element named HEADER. */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
        return;
    }

    /* Start an element named "ENTRIES" as child of ORDER. */
    rc = xmlTextWriterStartElement(writer, "testcoding");
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
        return;
    }

    /* Write an element named "ARTICLE" as child of ENTRY. */
    rc = xmlTextWriterWriteElement(writer, "ARTICLE",
                                   "<Test>");
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterWriteElement\n");
        return;
    }

    /* Write an element named "ENTRY_NO" as child of ENTRY. */
    rc = xmlTextWriterWriteFormatElement(writer, "ENTRY_NO", "%d",
                                         10);
    if (rc < 0) {
        printf
            ("testXmlwriterMemory: Error at xmlTextWriterWriteFormatElement\n");
        return;
    }

    /* Close the element named ENTRIES. */
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
        return;
    }


    /* Here we could close the elements ORDER and EXAMPLE using the
     * function xmlTextWriterEndElement, but since we do not want to
     * write any other elements, we simply call xmlTextWriterEndDocument,
     * which will do all the work. */
    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0) {
        printf("testXmlwriterMemory: Error at xmlTextWriterEndDocument\n");
        return;
    }

    xmlFreeTextWriter(writer);

      response = MHD_create_response_from_buffer (buf->use, buf->content, MHD_RESPMEM_MUST_COPY);
      MHD_add_response_header (response, "Content-Type", "application/xml");
      MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);

    xmlBufferFree(buf);
}

/**********************************************************************************************************/
/* Http Callback : Renvoi une reponse suite a une demande d'un slave (appellée par libsoup)         */
/* Entrées : le contexte, le message, l'URL                                                               */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static gint Http_request (void *cls, struct MHD_Connection *connection, 
                           const char *url, 
                           const char *method, const char *version, 
                           const char *upload_data, 
                           size_t *upload_data_size, void **con_cls)
  { const char *Wrong_method   = "<html><body>Wrong method. Sorry... </body></html>";
    const char *Not_found      = "<html><body>URI not found on this server. Sorry... </body></html>";
    const char *Internal_error = "<html><body>An internal server error has occured!..</body></html>";
    gint ssl_algo, ssl_proto;
    struct sockaddr *client_addr;
    struct MHD_Response *response;
    void *client_cert, *tls_session;
    const union MHD_ConnectionInfo *info;
    gchar client_host[80], client_service[20];

    client_addr = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    getnameinfo( client_addr, sizeof(client_addr), client_host, sizeof(client_host),
                                                   client_service, sizeof(client_service), 0 );

    info = MHD_get_connection_info ( connection, MHD_CONNECTION_INFO_CIPHER_ALGO );
    if ( info ) { ssl_algo = info->cipher_algorithm; }
           else { ssl_algo = 0; }

    info = MHD_get_connection_info ( connection, MHD_CONNECTION_INFO_PROTOCOL );
    if ( info ) { ssl_proto = info->protocol; }
           else { ssl_proto = 0; }

    info = MHD_get_connection_info ( connection, MHD_CONNECTION_INFO_GNUTLS_SESSION );
    if ( info ) { tls_session = info->tls_session; }
           else { tls_session = NULL; }

    info = MHD_get_connection_info ( connection, MHD_CONNECTION_INFO_GNUTLS_CLIENT_CERT );
    if ( info ) { client_cert = info->client_cert; }
           else { client_cert = NULL; }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
              "New %s %s %s request from %s for %s/%s (%s/%s).",
               method, url, version,
               client_host, client_service,
               gnutls_cipher_get_name (ssl_algo), gnutls_protocol_get_name (ssl_proto)
            );

    if ( strcasecmp( method, MHD_HTTP_METHOD_GET ) )
     { response = MHD_create_response_from_buffer ( strlen (Wrong_method),
                                                   (void*) Wrong_method, MHD_RESPMEM_PERSISTENT);
       if (response)
        { MHD_queue_response ( connection, MHD_HTTP_METHOD_NOT_ALLOWED, response);  /* Method not allowed */
          MHD_destroy_response (response);
        }
       else return MHD_NO;
     }
    else if ( ! strcasecmp ( url, "/gifile" ) )
     { struct stat sbuf;
       gint fd;
       fd = open ("anna.jpg", O_RDONLY);
       if ( fd == -1 || fstat (fd, &sbuf) == -1)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                   "Http_request : Error /gifile %s", strerror(errno) );
          if (fd!=-1) close(fd);
          response = MHD_create_response_from_buffer ( strlen (Internal_error),
                                                       (void*) Internal_error, MHD_RESPMEM_PERSISTENT);
          if (response)
           { MHD_queue_response ( connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
             MHD_destroy_response (response);
             return(MHD_YES);
           }
          else return(MHD_NO);
       }
      response = MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
      MHD_add_response_header (response, "Content-Type", "image/jpg");
      MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
     }
    else if ( ! strcasecmp ( url, "/xml" ) )
     { struct stat sbuf;
       gint fd;
       fd = open ("test.xml", O_RDONLY);
       if ( fd == -1 || fstat (fd, &sbuf) == -1)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                   "Http_request : Error /xml %s", strerror(errno) );
          if (fd!=-1) close(fd);
          response = MHD_create_response_from_buffer ( strlen (Internal_error),
                                                       (void*) Internal_error, MHD_RESPMEM_PERSISTENT);
          if (response)
           { MHD_queue_response ( connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
             MHD_destroy_response (response);
             return(MHD_YES);
           }
          else return(MHD_NO);
       }
      response = MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
      MHD_add_response_header (response, "Content-Type", "application/xml");
      MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
     }
    else if ( ! strcasecmp ( url, "/dynxml" ) )
     { Traiter_dynxml ( connection );
     }
    else
     { response = MHD_create_response_from_buffer ( strlen (Not_found),
                                                   (void*) Not_found, MHD_RESPMEM_PERSISTENT);
       if (response) 
        { MHD_queue_response ( connection, MHD_HTTP_NOT_FOUND, response);
          MHD_destroy_response (response);
        }
       else return MHD_NO;
     }
    return MHD_YES;
  }

/**********************************************************************************************************/
/* Run_thread: Thread principal                                                                           */
/* Entrée: une structure LIBRAIRIE                                                                        */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { prctl(PR_SET_NAME, "W-HTTP", 0, 0, 0 );
    memset( &Cfg_http, 0, sizeof(Cfg_http) );               /* Mise a zero de la structure de travail */
    Cfg_http.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    Http_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );

    g_snprintf( Cfg_http.lib->admin_prompt, sizeof(Cfg_http.lib->admin_prompt), "http" );
    g_snprintf( Cfg_http.lib->admin_help,   sizeof(Cfg_http.lib->admin_help),   "Manage communications with Http Devices" );

    if (!Cfg_http.http_enable && !Cfg_http.https_enable)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread Http/Https not enable in config. Shutting Down %d", pthread_self() );
       goto end;
     }

    if (Cfg_http.http_enable)
     { Cfg_http.http_server = MHD_start_daemon ( MHD_USE_SELECT_INTERNALLY,
                                                 Cfg_http.http_port, NULL, NULL, 
                                                &Http_request, NULL,
                                                 MHD_OPTION_CONNECTION_LIMIT, Cfg_http.nbr_max_connexion,
                                                 MHD_OPTION_END);
       if (!Cfg_http.http_server)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                   "Run_thread: MHDServer HTTP creation error (%s). Shutting Down %d",
                    strerror(errno), pthread_self() );
        }
       else
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                   "Run_thread: MHDServer HTTP OK. Listening on port %d", Cfg_http.http_port );
        }
     }

    if (Cfg_http.https_enable && Charger_certificat() )
     { Cfg_http.https_server = MHD_start_daemon ( MHD_USE_SELECT_INTERNALLY | MHD_USE_SSL,
                                                  Cfg_http.https_port, NULL, NULL, 
                                                 &Http_request, NULL,
                                                  MHD_OPTION_CONNECTION_LIMIT, Cfg_http.nbr_max_connexion,
                                                  MHD_OPTION_HTTPS_MEM_CERT, Cfg_http.ssl_cert,
                                                  MHD_OPTION_HTTPS_MEM_KEY,  Cfg_http.ssl_key,
                                                  MHD_OPTION_HTTPS_MEM_TRUST, Cfg_http.ssl_ca,/* Require Client SSL */
                                                  MHD_OPTION_END);
       if (!Cfg_http.https_server)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                   "Run_thread: MHDServer HTTPS creation error (%s). Shutting Down %d",
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
     { usleep(10000);
       sched_yield();

       if (Cfg_http.lib->Thread_sigusr1)                                  /* A-t'on recu un signal USR1 ? */
        { int nbr_msg, nbr_sortie;

          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
#ifdef bouh
          pthread_mutex_lock( &Cfg_http.lib->synchro );       /* On recupere le nombre de msgs en attente */
          nbr_msg    = g_slist_length(Cfg_http.Liste_message);
          nbr_sortie = g_slist_length(Cfg_http.Liste_sortie);
          pthread_mutex_unlock( &Cfg_http.lib->synchro );
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                    "Run_thread: In Queue : %d MSGS, %d A", nbr_msg, nbr_sortie );
#endif
          Cfg_http.lib->Thread_sigusr1 = FALSE;
        }
     }

#ifdef bouh
    Desabonner_distribution_sortie  ( Http_Gerer_sortie ); /* Desabonnement de la diffusion des sorties */
    Desabonner_distribution_message ( Http_Gerer_message );/* Desabonnement de la diffusion des messages */
#endif
    if (Cfg_http.http_server)  MHD_stop_daemon (Cfg_http.http_server);
    if (Cfg_http.https_server)
     { MHD_stop_daemon (Cfg_http.https_server);
       Liberer_certificat();
     }
end:
    Http_Liberer_config();                                  /* Liberation de la configuration du thread */
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_http.lib->TID = 0;                                  /* On indique au http que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
