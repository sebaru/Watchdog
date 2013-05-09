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
 static void Satellite_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Satellite_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_satellite.lib->Thread_debug = g_key_file_get_boolean ( gkf, "SATELLITE", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */
    Cfg_satellite.enable        = g_key_file_get_boolean ( gkf, "SATELLITE", "enable", NULL ); 

    chaine = g_key_file_get_string  ( gkf, "SATELLITE", "send_to_url", NULL );
    if (!chaine)
     { Info_new( Config.log, TRUE, LOG_ERR,
                 "Satellite_Lire_config : No 'Send_to' URL in config !" );
       g_snprintf( Cfg_satellite.send_to_url, sizeof(Cfg_satellite.send_to_url), "Unknown" );
     }
    else
     { g_snprintf( Cfg_satellite.send_to_url, sizeof(Cfg_satellite.send_to_url), "%s", chaine ); g_free(chaine); }

    chaine                     = g_key_file_get_string  ( gkf, "SATELLITE", "https_file_cert", NULL );
    if (chaine)
     { g_snprintf( Cfg_satellite.https_file_cert, sizeof(Cfg_satellite.https_file_cert), "%s", chaine ); g_free(chaine); }
    else
     { g_snprintf( Cfg_satellite.https_file_cert, sizeof(Cfg_satellite.https_file_cert), "%s", SATELLITE_DEFAUT_FILE_SERVER  ); }

    chaine                     = g_key_file_get_string  ( gkf, "SATELLITE", "https_file_key", NULL );
    if (chaine)
     { g_snprintf( Cfg_satellite.https_file_key, sizeof(Cfg_satellite.https_file_key), "%s", chaine ); g_free(chaine); }
    else
     { g_snprintf( Cfg_satellite.https_file_key, sizeof(Cfg_satellite.https_file_key), "%s", SATELLITE_DEFAUT_FILE_KEY ); }

    chaine                     = g_key_file_get_string  ( gkf, "SATELLITE", "https_file_ca", NULL );
    if (chaine)
     { g_snprintf( Cfg_satellite.https_file_ca, sizeof(Cfg_satellite.https_file_ca), "%s", chaine ); g_free(chaine); }
    else
     { g_snprintf( Cfg_satellite.https_file_ca, sizeof(Cfg_satellite.https_file_ca), "%s", SATELLITE_DEFAUT_FILE_CA ); }

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Satellite_Liberer_config : Libere la mémoire allouer précédemment pour lire la config satellite        */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Satellite_Liberer_config ( void )
  {
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

    pthread_mutex_lock ( &Cfg_satellite.lib->synchro );                               /* Ajout a la liste */
    Cfg_satellite.Liste_entreeANA = g_slist_append ( Cfg_satellite.Liste_entreeANA, GINT_TO_POINTER(num_ea) );
    pthread_mutex_unlock ( &Cfg_satellite.lib->synchro );
  }
/**********************************************************************************************************/
/* Satellite_Gerer_sortie: Ajoute une demande d'envoi RF dans la liste des envois RFXCOM                     */
/* Entrées: le numéro de la sortie                                                                        */
/**********************************************************************************************************/
 void Satellite_Gerer_sortie( gint num_a )                                    /* Num_a est l'id de la sortie */
  { gint taille;
#ifdef bouh
    pthread_mutex_lock( &Cfg_satellite.lib->synchro );              /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_satellite.Liste_sortie );
    pthread_mutex_unlock( &Cfg_satellite.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                "Satellite_Gerer_sortie: DROP sortie %d (length = %d > 150)", num_a, taille );
       return;
     }

    pthread_mutex_lock( &Cfg_satellite.lib->synchro );       /* Ajout dans la liste de tell a traiter */
    Cfg_satellite.Liste_sortie = g_slist_prepend( Cfg_satellite.Liste_sortie, GINT_TO_POINTER(num_a) );
    pthread_mutex_unlock( &Cfg_satellite.lib->synchro );
#endif
  }
/**********************************************************************************************************/
/* Satellite_Receive_response : Recupere la reponse du serveur (master)                                   */
/* Entrée : Les informations à sauvegarder                                                                */
/**********************************************************************************************************/
 static size_t Satellite_Receive_response( char *ptr, size_t size, size_t nmemb, void *userdata )
  { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_DEBUG,
              "Satellite_Receive_response: Récupération de %d*%d octets depuis le master", size, nmemb );
    Cfg_satellite.received_buffer = g_try_realloc ( Cfg_satellite.received_buffer,
                                                    Cfg_satellite.received_size +  size*nmemb );
    if (!Cfg_satellite.received_buffer)                              /* Si erreur, on arrete le transfert */
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_DEBUG,
                "Satellite_Receive_response: Memory Error realloc (%s).", strerror(errno) );
       return(-1);
     }
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

/*    xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"id",      "%d", syndb->id );*/

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
       xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"val_avant_ech", "%f", Partage->ea[num_ea].val_avant_ech );
       xmlTextWriterEndElement(writer);                                                      /* End EAxxx */
     }
    xmlTextWriterWriteComment(writer, (const unsigned char *)"End dumping EAxxx !!");


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
        { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
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
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );

    g_snprintf( Cfg_satellite.lib->admin_prompt, sizeof(Cfg_satellite.lib->admin_prompt), "satellite" );
    g_snprintf( Cfg_satellite.lib->admin_help,   sizeof(Cfg_satellite.lib->admin_help),   "Manage communications with Master Watchdog" );

    if (!Cfg_satellite.enable)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread bot enable in config. Shutting Down %d",
                 pthread_self() );
       goto end;
     }

/*  Abonner_distribution_sortie    ( Satellite_Gerer_sortie );   /* Abonnement à la diffusion des entrees */
/*  Abonner_distribution_sortieANA ( Satellite_Gerer_sortieANA );/* Abonnement à la diffusion des entrees */
/*  Abonner_distribution_entree    ( Satellite_Gerer_entree );   /* Abonnement à la diffusion des entrees */
    Abonner_distribution_entreeANA ( Satellite_Gerer_entreeANA );/* Abonnement à la diffusion des entrees */

    Cfg_satellite.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */
    while(Cfg_satellite.lib->Thread_run == TRUE)                            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_satellite.lib->Thread_sigusr1)                                /* A-t'on recu un signal USR1 ? */
        { int nbr_msg, nbr_sortie;

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

       Envoyer_les_infos_au_master();
     }

   Desabonner_distribution_entreeANA ( Satellite_Gerer_entreeANA );/* Abonnement à la diffusion des entrees */
/* Desabonner_distribution_sortie ( Satellite_Gerer_sortie );    /* Abonnement à la diffusion des sorties */

end:
    Satellite_Liberer_config();                                  /* Liberation de la configuration du thread */
    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_satellite.lib->TID = 0;                              /* On indique au satellite que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
