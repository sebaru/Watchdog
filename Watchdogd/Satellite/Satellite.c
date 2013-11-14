/**********************************************************************************************************/
/* Watchdogd/satellite.c        Gestion des SATELLITE de Watchdog v2.0                                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                    lun. 18 févr. 2013 18:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Satellite.c
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
 #include <sys/socket.h>
 #include <sys/stat.h>
 #include <netinet/in.h>
 #include <fcntl.h>
 #include <netdb.h>
 #include <gnutls/gnutls.h>
 #include <gnutls/x509.h>
 #include <curl/curl.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Satellite.h"

/**********************************************************************************************************/
/* Satellite_Lire_config : Lit la config Watchdog et rempli la structure mémoire                          */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 gboolean Satellite_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_satellite.lib->Thread_debug = FALSE;                               /* Settings default parameters */
    Cfg_satellite.enable            = FALSE; 
    Cfg_satellite.bit_state         = 0; 
    g_snprintf( Cfg_satellite.https_file_cert, sizeof(Cfg_satellite.https_file_cert),
               "%s", SATELLITE_DEFAUT_FILE_SERVER );
    g_snprintf( Cfg_satellite.https_file_key,  sizeof(Cfg_satellite.https_file_key),
               "%s", SATELLITE_DEFAUT_FILE_KEY );
    g_snprintf( Cfg_satellite.https_file_ca,   sizeof(Cfg_satellite.https_file_ca),
               "%s", SATELLITE_DEFAUT_FILE_CA );
    g_snprintf( Cfg_satellite.send_to_url,     sizeof(Cfg_satellite.send_to_url), "Unknown" );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                     /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                "Satellite_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )       /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,                   /* Print Config */
                "Satellite_Lire_config: '%s' = %d", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "https_file_cert" ) )
        { g_snprintf( Cfg_satellite.https_file_cert, sizeof(Cfg_satellite.https_file_cert), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "https_file_key" ) )
        { g_snprintf( Cfg_satellite.https_file_key,  sizeof(Cfg_satellite.https_file_key),  "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "https_file_ca" ) )
        { g_snprintf( Cfg_satellite.https_file_ca,   sizeof(Cfg_satellite.https_file_ca),   "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "send_to_url" ) )
        { g_snprintf( Cfg_satellite.send_to_url,     sizeof(Cfg_satellite.send_to_url),     "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_satellite.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_satellite.lib->Thread_debug = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "bit_state" ) )
        { Cfg_satellite.bit_state = atoi(valeur);  }
       else
        { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
                   "Satellite_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Satellite_Gerer_message: Fonction d'abonné appellé lorsqu'une EANA est modifiée.                       */
/* Entrée: le numéro de EANA                                                                              */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Satellite_Gerer_entreeANA ( gint num_ea )
  { gint taille;

    pthread_mutex_lock( &Cfg_satellite.lib->synchro );                   /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_satellite.Liste_entreeANA );
    pthread_mutex_unlock( &Cfg_satellite.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                "Satellite_Gerer_entreeANA: DROP EANA %d (length = %d > 150)", num_ea, taille);
       return;
     }
    else if (Cfg_satellite.lib->Thread_run == FALSE)
     { Info_new( Config.log, Config.log_arch, LOG_INFO,
                "Satellite_Gerer_entreeANA: Thread is down. Dropping num_ea = %d", num_ea );
       return;
     }

    pthread_mutex_lock ( &Cfg_satellite.lib->synchro );                               /* Ajout a la liste */
    Cfg_satellite.Liste_entreeANA = g_slist_append ( Cfg_satellite.Liste_entreeANA, GINT_TO_POINTER(num_ea) );
    pthread_mutex_unlock ( &Cfg_satellite.lib->synchro );
  }
/**********************************************************************************************************/
/* Satellite_Gerer_message: Fonction d'abonné appellé lorsqu'une EANA est modifiée.                       */
/* Entrée: le numéro de EANA                                                                              */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Satellite_Gerer_entree ( gint num )
  { gint taille;

    pthread_mutex_lock( &Cfg_satellite.lib->synchro );                   /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_satellite.Liste_entreeTOR );
    pthread_mutex_unlock( &Cfg_satellite.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                "Satellite_Gerer_entree: DROP E%d (length = %d > 150)", num, taille);
       return;
     }
    else if (Cfg_satellite.lib->Thread_run == FALSE)
     { Info_new( Config.log, Config.log_arch, LOG_INFO,
                "Satellite_Gerer_entree: Thread is down. Dropping num_e = %d", num );
       return;
     }

    pthread_mutex_lock ( &Cfg_satellite.lib->synchro );                               /* Ajout a la liste */
    Cfg_satellite.Liste_entreeTOR = g_slist_append ( Cfg_satellite.Liste_entreeTOR, GINT_TO_POINTER(num) );
    pthread_mutex_unlock ( &Cfg_satellite.lib->synchro );
  }
/**********************************************************************************************************/
/* Satellite_Receive_response : Recupere la reponse du serveur (master)                                   */
/* Entrée : Les informations à sauvegarder                                                                */
/**********************************************************************************************************/
 static size_t Satellite_Receive_response( char *ptr, size_t size, size_t nmemb, void *userdata )
  { gchar *new_buffer;
    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_DEBUG,
              "Satellite_Receive_response: Récupération de %d*%d octets depuis le master", size, nmemb );
    new_buffer = g_try_realloc ( Cfg_satellite.received_buffer,
                                 Cfg_satellite.received_size +  size*nmemb );
    if (!new_buffer)                                                 /* Si erreur, on arrete le transfert */
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                "Satellite_Receive_response: Memory Error realloc (%s).", strerror(errno) );
       g_free(Cfg_satellite.received_buffer);
       Cfg_satellite.received_buffer = NULL;
       return(-1);
     } else Cfg_satellite.received_buffer = new_buffer;
    memcpy( Cfg_satellite.received_buffer + Cfg_satellite.received_size, ptr, size*nmemb );
    return(size*nmemb);
  }
/**********************************************************************************************************/
/* Envoyer_les_infos_au_master : se connecte au master pour lui envoyer les infos                         */
/* Entrée : rien, sortie : rien                                                                           */
/**********************************************************************************************************/
 static void Envoyer_les_infos_au_master ( void )
  { gchar erreur[CURL_ERROR_SIZE+1];
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    guint retour;
    CURLcode res;
    CURL *curl;

    buf = xmlBufferCreate();                                                    /* Creation du buffer xml */
    if (buf == NULL)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                 "Satellite_Envoyer_les_infos_aux_slaves : XML Buffer creation failed" );
       return;
     }

    writer = xmlNewTextWriterMemory(buf, 0);                                     /* Creation du write XML */
    if (writer == NULL)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                 "Satellite_Envoyer_les_infos_aux_slaves : XML Writer creation failed" );
       xmlBufferFree(buf);
       return;
     }

    retour = xmlTextWriterStartDocument(writer, NULL, "UTF-8", "yes" );           /* Creation du document */
    if (retour < 0)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                 "Satellite_Envoyer_les_infos_aux_slaves : XML Start document failed" );
       xmlBufferFree(buf);
       return;
     }

    retour = xmlTextWriterStartElement(writer, (const unsigned char *) "SatelliteInfos");
    if (retour < 0)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                 "Satellite_Envoyer_les_infos_aux_slaves : XML Failed to Start element SatelliteInfos" );
       xmlBufferFree(buf);
       return;
     }

/*------------------------------------------- Sending identification -------------------------------------*/
    xmlTextWriterStartElement(writer, (const unsigned char *)"Ident");                     /* Start EAxxx */
    xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"instance_id", "%s", Config.instance_id );
    xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"bit_state",   "%d", Cfg_satellite.bit_state );
    xmlTextWriterEndElement(writer);                                                      /* End EAxxx */

/*------------------------------------------- Dumping EAxxx ----------------------------------------------*/
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Start dumping EAxxx !!");

    while (Cfg_satellite.Liste_entreeANA)
     { gint num_ea;
       pthread_mutex_lock( &Cfg_satellite.lib->synchro );               /* Récupération de l'EA a traiter */
       num_ea = GPOINTER_TO_INT(Cfg_satellite.Liste_entreeANA->data);
       Cfg_satellite.Liste_entreeANA = g_slist_remove( Cfg_satellite.Liste_entreeANA, GINT_TO_POINTER(num_ea) );
       pthread_mutex_unlock( &Cfg_satellite.lib->synchro );

       xmlTextWriterStartElement(writer, (const unsigned char *)"EntreeANA");              /* Start EAxxx */
       xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"num",   "%d", num_ea );
       xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"in_range", "%d", Partage->ea[num_ea].inrange );
       xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"val_avant_ech", "%f", Partage->ea[num_ea].val_avant_ech );
       xmlTextWriterEndElement(writer);                                                      /* End EAxxx */
     }
    xmlTextWriterWriteComment(writer, (const unsigned char *)"End dumping EAxxx !!");
/*------------------------------------------- Dumping EAxxx ----------------------------------------------*/
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Start dumping Exxx !!");

    while (Cfg_satellite.Liste_entreeTOR)
     { gint num;
       pthread_mutex_lock( &Cfg_satellite.lib->synchro );                /* Récupération de l'E a traiter */
       num = GPOINTER_TO_INT(Cfg_satellite.Liste_entreeTOR->data);
       Cfg_satellite.Liste_entreeTOR = g_slist_remove( Cfg_satellite.Liste_entreeTOR, GINT_TO_POINTER(num) );
       pthread_mutex_unlock( &Cfg_satellite.lib->synchro );

       xmlTextWriterStartElement(writer, (const unsigned char *)"EntreeTOR");               /* Start Exxx */
       xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"num",  "%d", num );
       xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"etat", "%d", E(num) );
       xmlTextWriterEndElement(writer);                                                       /* End Exxx */
     }
    xmlTextWriterWriteComment(writer, (const unsigned char *)"End dumping Exxx !!");


    retour = xmlTextWriterEndElement(writer);                                       /* End SatelliteInfos */
    if (retour < 0)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                 "Satellite_Envoyer_les_infos_aux_slaves : Failed to end element SatelliteInfos" );
       xmlBufferFree(buf);
       return;
     }

    retour = xmlTextWriterEndDocument(writer);
    if (retour < 0)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                 "Satellite_Envoyer_les_infos_aux_slaves : Failed to end Document" );
       xmlBufferFree(buf);
       return;
     }

    xmlFreeTextWriter(writer);                                                /* Libération du writer XML */

/*------------------------------------------- Le buffer XML est pret -------------------------------------*/
    Cfg_satellite.received_buffer = NULL;                           /* Init du tampon de reception à NULL */
    Cfg_satellite.received_size = 0;                                /* Init du tampon de reception à NULL */
    curl = curl_easy_init();                                            /* Preparation de la requete CURL */
    if (curl)
     { struct curl_slist *slist = NULL;
       gchar url[128], chaine[128];
       g_snprintf( url, sizeof(url), "%s/set_internal", Cfg_satellite.send_to_url );
       curl_easy_setopt(curl, CURLOPT_URL, url );
       curl_easy_setopt(curl, CURLOPT_POST, 1 );
       curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)buf->content);
       curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, buf->use);
       slist = curl_slist_append(slist, "Content-Type: application/xml");
       curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
       curl_easy_setopt(curl, CURLOPT_HEADER, 1);
       curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, erreur );
       curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Satellite_Receive_response );
       curl_easy_setopt(curl, CURLOPT_VERBOSE, Cfg_satellite.lib->Thread_debug );
       curl_easy_setopt(curl, CURLOPT_USERAGENT, "Watchdog Satellite - libcurl");
       curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0 );
       curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );                                   /* Warning ! */
       curl_easy_setopt(curl, CURLOPT_CAINFO, Cfg_satellite.https_file_ca );
       curl_easy_setopt(curl, CURLOPT_SSLKEY, Cfg_satellite.https_file_key );
       g_snprintf( chaine, sizeof(chaine), "./%s", Cfg_satellite.https_file_cert );
       curl_easy_setopt(curl, CURLOPT_SSLCERT, chaine );

       res = curl_easy_perform(curl);
       if (!res)
        { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_DEBUG,
                   "Envoyer_les_infos_au_master: Sending Update Request to %s OK",
                    Cfg_satellite.send_to_url );
        }
       else
        { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                   "Envoyer_les_infos_au_master: Sending Update Request to %s failed (%s), CA %s, Cert %s, Key %s",
                    Cfg_satellite.send_to_url, erreur,
                    Cfg_satellite.https_file_ca, Cfg_satellite.https_file_cert,Cfg_satellite.https_file_key
                  );
        }
       curl_easy_cleanup(curl);
       curl_slist_free_all(slist);
       if (Cfg_satellite.received_buffer)
        { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_DEBUG,
                   "Envoyer_les_infos_au_master: Master Response : %s",
                    Cfg_satellite.received_buffer);
          g_free(Cfg_satellite.received_buffer);
        }
     }
    else
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                "Envoyer_les_infos_au_master: Sending Update Request to %s failed with cURL init",
                 Cfg_satellite.send_to_url );
     }
    xmlBufferFree(buf);                                                       /* Libération du buffer XML */
    Cfg_satellite.last_sent = Partage->top;                         /* Sauvegarde du top du dernier envoi */
  }
/**********************************************************************************************************/
/* Run_thread: Thread principal                                                                           */
/* Entrée: une structure LIBRAIRIE                                                                        */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { prctl(PR_SET_NAME, "W-SATELLITE", 0, 0, 0 );
    memset( &Cfg_satellite, 0, sizeof(Cfg_satellite) );               /* Mise a zero de la structure de travail */
    Cfg_satellite.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    Satellite_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );

    g_snprintf( Cfg_satellite.lib->admin_prompt, sizeof(Cfg_satellite.lib->admin_prompt), NOM_THREAD );
    g_snprintf( Cfg_satellite.lib->admin_help,   sizeof(Cfg_satellite.lib->admin_help),   "Manage communications with Master Watchdog" );

    if (!Cfg_satellite.enable)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread is not enabled in config. Shutting Down %p",
                 pthread_self() );
       goto end;
     }

    Abonner_distribution_entree    ( Satellite_Gerer_entree );   /* Abonnement à la diffusion des entrees */
    Abonner_distribution_entreeANA ( Satellite_Gerer_entreeANA );/* Abonnement à la diffusion des entrees */

    Cfg_satellite.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */
    while(Cfg_satellite.lib->Thread_run == TRUE)                            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_satellite.lib->Thread_sigusr1)                                /* A-t'on recu un signal USR1 ? */
        { 

          Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
#ifdef bouh
          pthread_mutex_lock( &Cfg_satellite.lib->synchro );     /* On recupere le nombre de msgs en attente */
          nbr_msg    = g_slist_length(Cfg_satellite.Liste_entreeANA);
          nbr_sortie = g_slist_length(Cfg_satellite.Liste_sortie);
          pthread_mutex_unlock( &Cfg_satellite.lib->synchro );
          Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,
                    "Run_thread: In Queue : %d MSGS, %d A", nbr_msg, nbr_sortie );
#endif
          Cfg_satellite.lib->Thread_sigusr1 = FALSE;
        }

       if ( Cfg_satellite.Liste_entreeANA ||                            /* Si changement, envoi au master */
           (Cfg_satellite.last_sent + 2400 <= Partage->top) )    /* Ou au pire toutes les 4 minutes 'top' */
        { Envoyer_les_infos_au_master(); } 
     }

   Desabonner_distribution_entreeANA ( Satellite_Gerer_entreeANA );/* Abonnement à la diffusion des entrees */
   Desabonner_distribution_entree    ( Satellite_Gerer_entree    );/* Abonnement à la diffusion des entrees */

end:
    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_satellite.lib->TID = 0;                              /* On indique au satellite que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
