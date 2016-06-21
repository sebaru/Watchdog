/******************************************************************************************************************************/
/* Watchdogd/Serveur/serveur.c                Comportement d'un sous-serveur Watchdog                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           jeu 02 fév 2006 13:01:57 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Serveur.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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
 
 #include <glib.h>
 #include <openssl/err.h>
 #include <openssl/rsa.h>
 #include <openssl/x509.h>
 #include <signal.h>
 #include <unistd.h>
 #include <time.h>
 #include <openssl/ssl.h>
 #include <openssl/rand.h>
 #include <sys/socket.h>
 #include <sys/prctl.h>
 #include <netinet/tcp.h>
 #include <netinet/in.h>                                                              /* Pour les structures d'entrées SOCKET */
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <fcntl.h>
 #include <string.h>
 #include <pthread.h>
 #include <locale.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"

/******************************************************************************************************************************/
/* Ssrv_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                   */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Ssrv_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_ssrv.lib->Thread_debug = FALSE;                                                        /* Settings default parameters */
    /*Cfg_ssrv.enable            = FALSE; */

    Cfg_ssrv.ssl_needed         = SSRV_DEFAUT_SSL_NEEDED;
    Cfg_ssrv.ssl_peer_cert      = SSRV_DEFAUT_SSL_PEER_CERT;
    Cfg_ssrv.port               = SSRV_DEFAUT_PORT;
    Cfg_ssrv.taille_bloc_reseau = SSRV_DEFAUT_NETWORK_BUFFER;
    g_snprintf( Cfg_ssrv.ssl_file_cert, sizeof(Cfg_ssrv.ssl_file_cert), "%s", SSRV_DEFAUT_FILE_CERT );
    g_snprintf( Cfg_ssrv.ssl_file_key,  sizeof(Cfg_ssrv.ssl_file_key),  "%s", SSRV_DEFAUT_FILE_KEY );
    g_snprintf( Cfg_ssrv.ssl_file_ca,   sizeof(Cfg_ssrv.ssl_file_ca),   "%s", SSRV_DEFAUT_FILE_CA  );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Ssrv_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,                                            /* Print Config */
                "Ssrv_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "ssl_file_cert" ) )
        { g_snprintf( Cfg_ssrv.ssl_file_cert, sizeof(Cfg_ssrv.ssl_file_cert), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_file_ca" ) )
        { g_snprintf( Cfg_ssrv.ssl_file_ca, sizeof(Cfg_ssrv.ssl_file_ca), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_file_key" ) )
        { g_snprintf( Cfg_ssrv.ssl_file_key, sizeof(Cfg_ssrv.ssl_file_key), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_ssrv.lib->Thread_debug = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_needed" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_ssrv.ssl_needed = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_peer_cert" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_ssrv.ssl_peer_cert = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "port" ) )
        { Cfg_ssrv.port = atoi(valeur); }
       else if ( ! g_ascii_strcasecmp ( nom, "network_buffer" ) )
        { Cfg_ssrv.taille_bloc_reseau = atoi(valeur);  }
       else
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
                   "Ssrv_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Ref_client et Unref_client servent a referencer ou non une structure CLIENT en mémoire                                     */
/* Entrée: un client                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Unref_client ( struct CLIENT *client )
  { pthread_mutex_lock( &client->mutex_struct_used );
    if (client->struct_used) client->struct_used--;
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Unref_client: struct_used = %d for %s ",
              client->struct_used, client->machine );
    pthread_mutex_unlock( &client->mutex_struct_used );

    if (client->struct_used == 0)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Unref_client: struct_used = 0. closing %s. ",
                 client->machine );

       pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
       Cfg_ssrv.Clients = g_slist_remove( Cfg_ssrv.Clients, client );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Unref_client: %03d clients managed left", g_slist_length( Cfg_ssrv.Clients ) );
       pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
    
       Fermer_connexion( client->connexion );
       pthread_mutex_destroy( &client->mutex_struct_used );
       if (client->util)            { g_free( client->util ); }
       if (client->syn_to_send)     { g_free( client->syn_to_send ); }
       if (client->Liste_bit_syns)  { g_slist_free(client->Liste_bit_syns); }
       if (client->Liste_bit_capteurs)
                                    { g_slist_foreach( client->Liste_bit_capteurs, (GFunc) g_free, NULL );
                                      g_slist_free(client->Liste_bit_capteurs);
                                    }
       if (client->courbes)         { g_list_foreach( client->courbes, (GFunc)g_free, NULL );
                                      g_list_free(client->courbes);
                                    }
       if (client->Liste_histo)     { g_slist_foreach( client->Liste_histo, (GFunc) g_free, NULL );
                                      g_slist_free ( client->Liste_histo );
                                    }
       g_free(client);
     }
  }
/******************************************************************************************************************************/
/* Ref_client et Unref_client servent a referencer ou non une structure CLIENT en mémoire                                     */
/* Entrée: un client                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Ref_client ( struct CLIENT *client, gchar *reason )
  { pthread_mutex_lock( &client->mutex_struct_used );
    client->struct_used++;
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Ref_client: struct_used = %d for %s(SSRV%06d) - %s. ",
              client->struct_used, client->machine, client->ssrv_id, reason );
    pthread_mutex_unlock( &client->mutex_struct_used );
  }
/******************************************************************************************************************************/
/* Mode_vers_string: Conversion d'un mode vers une chaine de caracteres                                                       */
/* Entrée: un mode                                                                                                            */
/* Sortie: un gchar *                                                                                                         */
/******************************************************************************************************************************/
 gchar *Mode_vers_string ( gint mode )
  { switch (mode)
     { case ATTENTE_CONNEXION_SSL       : return("ATTENTE_CONNEXION_SSL");
       case ENVOI_INTERNAL              : return("ENVOI_INTERNAL");
       case WAIT_FOR_IDENT              : return("WAIT_FOR_IDENT");
       case WAIT_FOR_NEWPWD             : return("WAIT_FOR_NEWPWD");
       case ENVOI_SYNCHRO               : return("ENVOI_SYNCHRO");

       case ENVOI_GROUPE_FOR_UTIL       : return("ENVOI_GROUPE_FOR_UTIL");
       case ENVOI_GROUPE_FOR_SYNOPTIQUE : return("ENVOI_GROUPE_FOR_SYNOPTIQUE");
       case ENVOI_CLASSE_FOR_ATELIER    : return("ENVOI_CLASSE_FOR_ATELIER");
       case ENVOI_ICONE_FOR_ATELIER     : return("ENVOI_ICONE_FOR_ATELIER");
       case ENVOI_SYNOPTIQUE_FOR_ATELIER: return("ENVOI_SYNOPTIQUE_FOR_ATELIER");
       case ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE
                                        : return("ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE");
       case ENVOI_PALETTE_FOR_ATELIER_PALETTE
                                        : return("ENVOI_PALETTE_FOR_ATELIER_PALETTE");

       case VALIDE_NON_ROOT             : return("VALIDE_NON_ROOT");
       case VALIDE                      : return("VALIDE");
       case DECONNECTE                  : return("DECONNECTE");
     }
    return("Inconnu");
  }
/******************************************************************************************************************************/
/* Client_mode: Mise d'un client dans un mode precis                                                                          */
/* Entrée: un client et un mode                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Client_mode ( struct CLIENT *client, gint mode )
  { if (client->mode == DECONNECTE)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "Client_mode: positionnement impossible pour %s car mode = DECONNECTE", client->util->nom);
       return;
     }

    if (client->mode == VALIDE_NON_ROOT && mode == VALIDE)                                        /* Nous prevenons le client */
     { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_CLI_VALIDE, NULL, 0 ); }
    client->mode = mode;
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "Client_mode: client %s (SSRV%06d) en mode %s",
                 client->machine, client->ssrv_id, Mode_vers_string(mode) );
  }
/******************************************************************************************************************************/
/* Deconnecter: Deconnection d'un client                                                                                      */
/* Entrée: un client                                                                                                          */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Deconnecter ( struct CLIENT *client )
  { client->mode = VALIDE;                                                /* Envoi un dernier paquet "OFF" avant deconnexion" */
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
              "Deconnecter: deconnexion client %s", client->machine );
    Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_OFF, NULL, 0 );
    client->mode = DECONNECTE;
                                                                        /* Le client n'est plus connecté, on en informe D.L.S */
    if (client->util && client->util->ssrv_bit_presence) SB(client->util->ssrv_bit_presence, 0);
    Unref_client( client ); 
  }
/******************************************************************************************************************************/
/* Ssrv_Gerer_events: Receptionne un evenement fourni par MSRV                                                                */
/* Entrées: l'evenemen a recupérer                                                                                            */
/******************************************************************************************************************************/
 static void Ssrv_Gerer_events( struct CMD_TYPE_MSRV_EVENT *event )
  { gint taille;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                                    /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_ssrv.Liste_events );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );

    if (taille > MAX_ENREG_QUEUE)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Ssrv_Gerer_events: DROP (taille>MAX_ENREG_QUEUE(%d))", MAX_ENREG_QUEUE );
       g_free(event);
       return;
     }

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                                    /* Ajout dans la liste de tell a traiter */
    Cfg_ssrv.Liste_events = g_slist_prepend( Cfg_ssrv.Liste_events, event );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
  }
/******************************************************************************************************************************/
/* Envoyer_events_aux_thread: duplique les evenements recus et en envoi la copie a chacun des threads                         */
/* Entrée : néant                                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Envoyer_events_aux_threads ( void )
  { struct CMD_TYPE_MSRV_EVENT *event;
    GSList *liste;
    
    if ( Cfg_ssrv.Liste_events == NULL ) return;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    event = (struct CMD_TYPE_MSRV_EVENT *) Cfg_ssrv.Liste_events->data;
    Cfg_ssrv.Liste_events = g_slist_remove ( Cfg_ssrv.Liste_events, event );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
       
    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    liste = Cfg_ssrv.Clients;
    while (liste && Cfg_ssrv.lib->Thread_run)
     { struct CLIENT *client;
       struct CMD_TYPE_MSRV_EVENT *dup_event;
       client = (struct CLIENT *)liste->data;

       if (client->mode == VALIDE)
        { dup_event = (struct CMD_TYPE_MSRV_EVENT *)g_try_malloc0( sizeof ( struct CMD_TYPE_MSRV_EVENT ) );
          if (!dup_event)
           { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                      "Envoyer_event_aux_threads: Memory error" );
             break;
           }
          else memcpy ( dup_event, event, sizeof(struct CMD_TYPE_MSRV_EVENT) );

          Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "Envoyer_event_aux_threads: Envoi au thread %06d (client %s) -> Event %s %s %s",
                   client->ssrv_id, client->machine, event->instance, event->thread, event->objet );
          client->Liste_events = g_slist_prepend ( client->Liste_events, dup_event );
        }
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
    g_free(event);                                                          /* On a plus besoin de la structure, on la libere */
  }
/******************************************************************************************************************************/
/* Ssrv_Gerer_message: Ajoute une demande d'envoi des messages aux thread                                                     */
/* Entrées: le numéro de la sortie                                                                                            */
/******************************************************************************************************************************/
 static void Ssrv_Gerer_histo( struct CMD_TYPE_HISTO *histo )
  { gint taille;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                                    /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_ssrv.Liste_histo );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );

    if (taille > MAX_ENREG_QUEUE)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Ssrv_Gerer_histo: DROP (taille>MAX_ENREG_QUEUE(%d)) msg=%d", MAX_ENREG_QUEUE, histo->msg.num );
       g_free(histo);
       return;
     }

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Ssrv_Gerer_histo: Message a traiter : msg=%d", histo->msg.num );
       
    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                                    /* Ajout dans la liste de tell a traiter */
    Cfg_ssrv.Liste_histo = g_slist_append( Cfg_ssrv.Liste_histo, histo );                   /* Append pour l'ordre d'arrive ! */
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
  }
/******************************************************************************************************************************/
/* Envoyer_histo_aux_thread: duplique l'histo recu et en envoi la copie a chacun des threads                                  */
/* Entrée : néant                                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Envoyer_histo_aux_threads ( void )
  { struct CMD_TYPE_HISTO *histo;
    GSList *liste;
    
    if ( Cfg_ssrv.Liste_histo == NULL ) return;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    histo = (struct CMD_TYPE_HISTO *) Cfg_ssrv.Liste_histo->data;
    Cfg_ssrv.Liste_histo = g_slist_remove ( Cfg_ssrv.Liste_histo, histo );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
       
    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    liste = Cfg_ssrv.Clients;
    while (liste && Cfg_ssrv.lib->Thread_run)
     { struct CLIENT *client;
       struct CMD_TYPE_HISTO *dup_histo;
       client = (struct CLIENT *)liste->data;

       if (client->mode == VALIDE)
        { dup_histo = (struct CMD_TYPE_HISTO *)g_try_malloc0( sizeof ( struct CMD_TYPE_HISTO ) );
          if (!dup_histo)
           { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                      "Envoyer_histo_aux_threads: Memory error" );
             break;
           }
          else memcpy ( dup_histo, histo, sizeof(struct CMD_TYPE_HISTO) );

          Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "Envoyer_histo_aux_threads: Envoi du MSG%d=%d au thread %06d (client %s)",
                   dup_histo->msg.num, dup_histo->alive, client->ssrv_id, client->machine );
          client->Liste_histo = g_slist_append ( client->Liste_histo, dup_histo );
        }
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
    g_free(histo);                                                          /* On a plus besoin de la structure, on la libere */
  }
/******************************************************************************************************************************/
/* Ssrv_Gerer_motif: Ajoute une demande d'envoi des motif Ixxx aux thread                                                     */
/* Entrées: le numéro de la sortie                                                                                            */
/******************************************************************************************************************************/
 static void Ssrv_Gerer_motif( gint num )
  { gint taille;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                                    /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_ssrv.Liste_motif );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );

    if (taille > MAX_ENREG_QUEUE)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Ssrv_Gerer_motif: DROP (taille>MAX_ENREG_QUEUE(%d)) num=%d", MAX_ENREG_QUEUE, num );
       return;
     }

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Ssrv_Gerer_motif: Motif a traiter : num=%d", num );

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                                    /* Ajout dans la liste de tell a traiter */
    Cfg_ssrv.Liste_motif = g_slist_append( Cfg_ssrv.Liste_motif, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
  }
/******************************************************************************************************************************/
/* Envoyer_motif_aux_thread: duplique le motif recu et en envoi la copie a chacun des thread                                  */
/* Entrée : néant                                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Envoyer_motif_aux_threads ( void )
  { struct CMD_ETAT_BIT_CTRL motif;
    GSList *liste;
    gint num;
    
    if ( Cfg_ssrv.Liste_motif == NULL ) return;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    num = GPOINTER_TO_INT (Cfg_ssrv.Liste_motif->data);
    Cfg_ssrv.Liste_motif = g_slist_remove ( Cfg_ssrv.Liste_motif, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );

/**************************************** Création de la structure passée aux clients *****************************************/
    motif.num    = num;
    motif.etat   = Partage->i[num].etat;
    motif.rouge  = Partage->i[num].rouge;
    motif.vert   = Partage->i[num].vert;
    motif.bleu   = Partage->i[num].bleu;
    motif.cligno = Partage->i[num].cligno;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                                                  /* Envoi a tous les thread */
    liste = Cfg_ssrv.Clients;
    while (liste && Cfg_ssrv.lib->Thread_run)
     { struct CLIENT *client;
       struct CMD_ETAT_BIT_CTRL *dup_motif;
       client = (struct CLIENT *)liste->data;

       dup_motif = (struct CMD_ETAT_BIT_CTRL *)g_try_malloc0( sizeof ( struct CMD_ETAT_BIT_CTRL ) );
       if (!dup_motif)
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                   "Envoyer_motif_aux_threads: Memory error" );
          break;
        }
       else memcpy ( dup_motif, &motif, sizeof(struct CMD_ETAT_BIT_CTRL));

       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_motif_aux_threads: Envoi du I%03d = %d, r%03d, v%03d, b%03d, c%d au thread %06d (client %s)",
                dup_motif->num, dup_motif->etat,
                dup_motif->rouge, dup_motif->vert, dup_motif->bleu,
                dup_motif->cligno, client->ssrv_id, client->machine );
       client->Liste_new_motif = g_slist_append ( client->Liste_new_motif, dup_motif );
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
  }
/******************************************************************************************************************************/
/* Accueillir_nouveaux_clients: Cette fonction permet de loguer d'éventuels nouveaux clients distants                         */
/* Entrée: rien                                                                                                               */
/* Sortie: TRUE si un nouveau client est arrivé                                                                               */
/******************************************************************************************************************************/
 static struct CLIENT *Accueillir_un_client( void )
  { struct sockaddr_in distant;
    guint taille_distant, id;
    struct CLIENT *client;
    struct hostent *host;

    taille_distant = sizeof(distant);
    if ( (id=accept( Cfg_ssrv.Socket_ecoute, (struct sockaddr *)&distant, &taille_distant )) != -1)             /* demande ?? */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
                "Accueillir_nouveaux_client: Connexion wanted. ID=%d", id );

       client = g_try_malloc0( sizeof(struct CLIENT) );                      /* On alloue donc une nouvelle structure cliente */
       if (!client) { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                               "Accueillir_nouveaux_client: Not enought memory to connect %d", id );
                      close(id);
                      return(NULL);                                                         /* On traite bien sûr les erreurs */
                    }

       client->connexion = Nouvelle_connexion( Config.log, id, Cfg_ssrv.taille_bloc_reseau );
       if (!client->connexion)
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                   "Accueillir_nouveaux_client: Not enought memory for %d", id );
          close(id);
          g_free( client );
          return(NULL);
        }

       host = gethostbyaddr( (char*)&distant.sin_addr, sizeof(distant.sin_addr), AF_INET );                    /*InfosClients */
       if (host) { g_snprintf( client->machine, TAILLE_MACHINE, "%s", host->h_name ); }                       /* Nom en clair */
            else { g_snprintf( client->machine, TAILLE_MACHINE, "%s",                                   /* Ou bien Adresse IP */
                               (gchar *)inet_ntoa(distant.sin_addr) );
                 }
       time( &client->date_connexion );                                    /* Enregistrement de la date de debut de connexion */
       client->pulse = Partage->top;
       client->courbe.num = -1;                                               /* Init: pas de courbe a envoyer pour le moment */
       pthread_mutex_init( &client->mutex_struct_used, NULL );
       client->struct_used = 1;                        /* Par défaut, la structure est utilisée par le thread de surveillance */

       pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
       Cfg_ssrv.Clients = g_slist_prepend( Cfg_ssrv.Clients, client );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "Accueillir_un_client: Connexion accepted (id=%d) from %s - (%03d clients managed)",
                 id, client->machine, g_slist_length(Cfg_ssrv.Clients) );
       pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
       Client_mode( client, ENVOI_INTERNAL );
       return(client);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Run_serveur boucle principale d'un sous-serveur Watchdog                                                                   */
/* Entree: l'id du serveur et le pid du pere                                                                                  */
/* Sortie: un code d'erreur EXIT_xxx                                                                                          */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { gchar nom[32];
    
    g_snprintf(nom, sizeof(nom), "W-SSRV-LISTEN" );
    prctl(PR_SET_NAME, nom, 0, 0, 0 );

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */

                                                                      /* Initialisation de la zone interne et comm du serveur */
    memset( &Cfg_ssrv, 0, sizeof(Cfg_ssrv) );                                       /* Mise a zero de la structure de travail */
    Cfg_ssrv.lib = lib;                                            /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_ssrv.lib->TID = pthread_self();                                                     /* Sauvegarde du TID pour le pere */
    Ssrv_Lire_config ();                                                    /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );
    Cfg_ssrv.lib->Thread_run = TRUE;                                                                    /* Le thread tourne ! */

    g_snprintf( Cfg_ssrv.lib->admin_prompt, sizeof(Cfg_ssrv.lib->admin_prompt), NOM_THREAD );
    g_snprintf( Cfg_ssrv.lib->admin_help,   sizeof(Cfg_ssrv.lib->admin_help),   "Manage SSRV Modules" );

    if (Cfg_ssrv.ssl_needed)
     { Cfg_ssrv.Ssl_ctx = Init_ssl();                                                                   /* Initialisation SSL */
       if (!Cfg_ssrv.Ssl_ctx)
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                   "Run_thread: Init ssl failed but needed. Stopping..." );
          goto end;
        }
     }
    else { Cfg_ssrv.Ssl_ctx = NULL;
           Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
                    "Run_thread: ssl_needed = FALSE. Warning, Network is not ciphered !" );
         }

    Cfg_ssrv.Socket_ecoute = Activer_ecoute();                                        /* Initialisation de l'écoute via TCPIP */
    if ( Cfg_ssrv.Socket_ecoute<0 )            
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_CRIT, "Network down, foreign connexions disabled" );
       goto end;
     }

    Abonner_distribution_motif ( Ssrv_Gerer_motif );                                    /* Abonnement a la liste de diffusion */
    Abonner_distribution_histo ( Ssrv_Gerer_histo );                                    /* Abonnement a la liste de diffusion */
    Abonner_distribution_events ( Ssrv_Gerer_events, NOM_THREAD );                      /* Abonnement a la liste de diffusion */

    while(lib->Thread_run == TRUE)                                                           /* On tourne tant que necessaire */
     { struct CLIENT *client;
       pthread_t tid;
       usleep(100000);
       sched_yield();

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
                    "Run_thread: Run_ssrv: SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       client = Accueillir_un_client();
       if (client)                                                                            /* Un client vient d'arriver ?? */
        { pthread_create( &tid, NULL, (void *)Run_handle_client, client );
          pthread_detach( tid );
        }

       Envoyer_histo_aux_threads();                                                /* Envoi les histos aux thread s'il y en a */
       Envoyer_motif_aux_threads();                                                /* Envoi les motifs aux thread s'il y en a */
       Envoyer_events_aux_threads();                                               /* Envoi les events aux thread s'il y en a */
      }                                                                                        /* Fin du while partage->arret */

    Desabonner_distribution_events ( Ssrv_Gerer_events );                        /* Desabonnement de la diffusion des sorties */
    if (Cfg_ssrv.Liste_events)                                                               /* Si la liste est encore pleine */
     { g_slist_foreach( Cfg_ssrv.Liste_events, (GFunc) g_free, NULL );
       g_slist_free ( Cfg_ssrv.Liste_events );
     }

    Desabonner_distribution_histo ( Ssrv_Gerer_histo );                                 /* Abonnement a la liste de diffusion */
    if (Cfg_ssrv.Liste_histo)                                                                /* Si la liste est encore pleine */
     { g_slist_foreach( Cfg_ssrv.Liste_histo, (GFunc) g_free, NULL );
       g_slist_free ( Cfg_ssrv.Liste_histo );
     }
    Desabonner_distribution_motif ( Ssrv_Gerer_motif );                                 /* Abonnement a la liste de diffusion */

end:
    while (Cfg_ssrv.Clients)                                                      /* Tant que des clients sont encore managés */
     { gint nbr;
       sleep(1);
       pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
       nbr = g_slist_length( Cfg_ssrv.Clients );
       pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "Run_thread: Wating for %03d clients to shutdown", nbr );
       sched_yield();
     }
    Cfg_ssrv.lib->Thread_run = FALSE;                                                           /* Le thread ne tourne plus ! */
    Liberer_SSL ();                                                                                     /* Libération mémoire */
    if (Cfg_ssrv.Socket_ecoute>0) close(Cfg_ssrv.Socket_ecoute);
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_ssrv.lib->TID = 0;                                                    /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
