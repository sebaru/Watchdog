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
 #include <microhttpd.h>
 #include <libxml/xmlwriter.h>

       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>



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


 static void Traiter_dynxml ( struct MHD_Connection *connection )
  { static gint i = 1;
    struct MHD_Response *response;
    int rc;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    xmlChar *tmp;

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
/* HttpMobile Callback : Renvoi une reponse suite a une demande d'un slave (appellée par libsoup)         */
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
    struct MHD_Response *response;

    Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_DEBUG,
              "New %s request for %s using version %s", method, url, version);
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
        { Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_DEBUG,
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
        { Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_DEBUG,
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


    Cfg_httpmobile.server = MHD_start_daemon ( MHD_USE_THREAD_PER_CONNECTION, Cfg_httpmobile.port, NULL, NULL, 
                                              &Http_request, NULL, MHD_OPTION_END);
    if (!Cfg_httpmobile.server)
     { Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: MHDServer creation error (%s). Shutting Down %d",
                 strerror(errno), pthread_self() );
       goto end;
     }
    else
     { Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_INFO,
                "Run_thread: MHDServer OK. Listening on port %d", Cfg_httpmobile.port );
     }

#ifdef bouh
    Abonner_distribution_message ( HttpMobile_Gerer_message );   /* Abonnement à la diffusion des messages */
    Abonner_distribution_sortie  ( HttpMobile_Gerer_sortie );     /* Abonnement à la diffusion des sorties */

#endif
    xmlInitParser();                                                      /* Initialisation du parser xml */
    Cfg_httpmobile.lib->Thread_run = TRUE;                                          /* Le thread tourne ! */
    while(Cfg_httpmobile.lib->Thread_run == TRUE)                        /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_httpmobile.lib->Thread_sigusr1)                            /* A-t'on recu un signal USR1 ? */
        { int nbr_msg, nbr_sortie;

          Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
#ifdef bouh
          pthread_mutex_lock( &Cfg_httpmobile.lib->synchro ); /* On recupere le nombre de msgs en attente */
          nbr_msg    = g_slist_length(Cfg_httpmobile.Liste_message);
          nbr_sortie = g_slist_length(Cfg_httpmobile.Liste_sortie);
          pthread_mutex_unlock( &Cfg_httpmobile.lib->synchro );
          Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_INFO,
                    "Run_thread: In Queue : %d MSGS, %d A", nbr_msg, nbr_sortie );
#endif
          Cfg_httpmobile.lib->Thread_sigusr1 = FALSE;
        }
     }

#ifdef bouh
    Desabonner_distribution_sortie  ( HttpMobile_Gerer_sortie ); /* Desabonnement de la diffusion des sorties */
    Desabonner_distribution_message ( HttpMobile_Gerer_message );/* Desabonnement de la diffusion des messages */
#endif

    MHD_stop_daemon (Cfg_httpmobile.server);
end:
    HttpMobile_Liberer_config();                                  /* Liberation de la configuration du thread */
    Info_new( Config.log, Cfg_httpmobile.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_httpmobile.lib->TID = 0;                              /* On indique au httpmobile que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
