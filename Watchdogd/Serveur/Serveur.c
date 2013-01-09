/**********************************************************************************************************/
/* Watchdogd/Serveur/serveur.c                Comportement d'un sous-serveur Watchdog                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                       jeu 02 fév 2006 13:01:57 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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
 #include <netinet/in.h>                                          /* Pour les structures d'entrées SOCKET */
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <fcntl.h>
 #include <string.h>
 #include <pthread.h>
 #include <locale.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"

 extern gint Socket_ecoute;                                  /* Socket de connexion (d'écoute) du serveur */

/**********************************************************************************************************/
/* Ref_client et Unref_client servent a referencer ou non une structure CLIENT en mémoire                 */
/* Entrée: un client                                                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Unref_client ( struct CLIENT *client )
  { pthread_mutex_lock( &client->mutex_struct_used );
    if (client->struct_used) client->struct_used--;
    pthread_mutex_unlock( &client->mutex_struct_used );
  }
/**********************************************************************************************************/
/* Ref_client et Unref_client servent a referencer ou non une structure CLIENT en mémoire                 */
/* Entrée: un client                                                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Ref_client ( struct CLIENT *client )
  { pthread_mutex_lock( &client->mutex_struct_used );
    client->struct_used++;
    pthread_mutex_unlock( &client->mutex_struct_used );
  }
/**********************************************************************************************************/
/* Mode_vers_string: Conversion d'un mode vers une chaine de caracteres                                   */
/* Entrée: un mode                                                                                        */
/* Sortie: un gchar *                                                                                     */
/**********************************************************************************************************/
 static gchar *Mode_vers_string ( gint mode )
  { switch (mode)
     { case ATTENTE_CONNEXION_SSL       : return("ATTENTE_CONNEXION_SSL");
       case ENVOI_INTERNAL              : return("ENVOI_INTERNAL");
       case ATTENTE_IDENT               : return("ATTENTE_IDENT");
       case ENVOI_AUTORISATION          : return("ENVOI_AUTORISATION");
       case ATTENTE_NEW_PASSWORD        : return("ATTENTE_NEW_PASSWORD");
       case ENVOI_DONNEES               : return("ENVOI_DONNEES");
       case ENVOI_HISTO                 : return("ENVOI_HISTO");

       case ENVOI_GROUPE_FOR_UTIL       : return("ENVOI_GROUPE_FOR_UTIL");
       case ENVOI_SOURCE_DLS            : return("ENVOI_SOURCE_DLS");
       case ENVOI_MNEMONIQUE_FOR_COURBE : return("ENVOI_MNEMONIQUE_FOR_COURBE");
       case ENVOI_MNEMONIQUE_FOR_HISTO_COURBE : return("ENVOI_MNEMONIQUE_FOR_HISTO_COURBE");
       case ENVOI_MOTIF_ATELIER         : return("ENVOI_MOTIF_ATELIER");
       case ENVOI_COMMENT_ATELIER       : return("ENVOI_COMMENT_ATELIER");
       case ENVOI_PASSERELLE_ATELIER    : return("ENVOI_PASSERELLE_ATELIER");
       case ENVOI_CAPTEUR_ATELIER       : return("ENVOI_CAPTEUR_ATELIER");
       case ENVOI_COMMENT_SUPERVISION   : return("ENVOI_COMMENT_SUPERVISION");
       case ENVOI_MOTIF_SUPERVISION     : return("ENVOI_MOTIF_SUPERVISION");
       case ENVOI_PASSERELLE_SUPERVISION: return("ENVOI_PASSERELLE_SUPERVISION");
       case ENVOI_PALETTE_SUPERVISION   : return("ENVOI_PALETTE_SUPERVISION");
       case ENVOI_CAPTEUR_SUPERVISION   : return("ENVOI_CAPTEUR_SUPERVISION");
       case ENVOI_IXXX_SUPERVISION      : return("ENVOI_IXXX_SUPERVISION");
       case ENVOI_GROUPE_FOR_SYNOPTIQUE : return("ENVOI_GROUPE_FOR_SYNOPTIQUE");
       case ENVOI_CAMERA_SUP_SUPERVISION: return("ENVOI_CAMERA_SUP_SUPERVISION");
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
/**********************************************************************************************************/
/* Client_mode: Mise d'un client dans un mode precis                                                      */
/* Entrée: un client et un mode                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Client_mode ( struct CLIENT *client, gint mode )
  { if (client->mode == DECONNECTE)
     { Info_new( Config.log, Config.log_all, LOG_INFO,
                "Client_mode: positionnement impossible pour %s car mode = DECONNECTE", client->util->nom);
       return;
     }

    if (client->mode == VALIDE_NON_ROOT && mode == VALIDE)                    /* Nous prevenons le client */
     { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_CLI_VALIDE, NULL, 0 ); }
    client->mode = mode;
    Info_new( Config.log, Config.log_all, LOG_INFO,
                "Client_mode: client %s en mode %s", client->machine, Mode_vers_string(mode) );
  }
/**********************************************************************************************************/
/* Deconnecter: Deconnection d'un client                                                                  */
/* Entrée: un client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Deconnecter ( struct CLIENT *client )
  {
    client->mode = VALIDE;                            /* Envoi un dernier paquet "OFF" avant deconnexion" */
    Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_OFF, NULL, 0 );
    client->mode = DECONNECTE;

    pthread_mutex_lock( &Partage->Sous_serveur[client->Id_serveur].synchro );
    Partage->Sous_serveur[client->Id_serveur].Clients = g_list_remove( Partage->Sous_serveur[client->Id_serveur].Clients, client );
    pthread_mutex_unlock( &Partage->Sous_serveur[client->Id_serveur].synchro );
    
    Fermer_connexion( client->connexion );
    pthread_mutex_destroy( &client->mutex_write );
    pthread_mutex_destroy( &client->mutex_struct_used );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Deconnecter: Connexion %d stopped", client->connexion->socket );
    if (client->util)         { g_free( client->util ); }
    if (client->bit_syns)     { g_list_free(client->bit_syns); }
    if (client->bit_init_syn) { g_list_free(client->bit_init_syn); }
    if (client->bit_capteurs) { g_list_foreach( client->bit_capteurs, (GFunc) g_free, NULL );
                                g_list_free(client->bit_capteurs);
                              }
    if (client->bit_init_capteur)
                              { g_list_foreach( client->bit_init_capteur, (GFunc) g_free, NULL );
                                g_list_free(client->bit_init_capteur);
                              }
    if (client->courbes)      { g_list_foreach( client->courbes, (GFunc)g_free, NULL );
                                g_list_free(client->courbes);
                              }
    if (client->Db_watchdog)  { Libere_DB_SQL( Config.log, &client->Db_watchdog ); }    /* Deconnexion DB */
    g_free( client );
  }
/**********************************************************************************************************/
/* Accueillir_nouveaux_clients: Cette fonction permet de loguer d'éventuels nouveaux clients distants     */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE si un nouveau client est arrivé                                                           */
/**********************************************************************************************************/
 static gboolean Accueillir_un_client( guint ss_id )
  { struct sockaddr_in distant;
    guint taille_distant, id;
    struct CLIENT *client;
    struct hostent *host;

    taille_distant = sizeof(distant);
    if ( (id=accept( Socket_ecoute, (struct sockaddr *)&distant, &taille_distant )) != -1)  /* demande ?? */
     { Info_new( Config.log, Config.log_all, LOG_INFO,
                "Accueillir_nouveaux_client: Connexion wanted. ID=%d", id );

       client = g_try_malloc0( sizeof(struct CLIENT) );  /* On alloue donc une nouvelle structure cliente */
       if (!client) { Info_new( Config.log, Config.log_all, LOG_ERR,
                               "Accueillir_nouveaux_client: Not enought memory to connect %d", id );
                      close(id);
                      return(FALSE);                                    /* On traite bien sûr les erreurs */
                    }

       client->connexion = Nouvelle_connexion( Config.log, id, Config.taille_bloc_reseau );
       if (!client->connexion)
        { Info_new( Config.log, Config.log_all, LOG_ERR,
                   "Accueillir_nouveaux_client: Not enought memory for %d", id );
          close(id);
          g_free( client );
          return(FALSE);
        }

       host = gethostbyaddr( (char*)&distant.sin_addr, sizeof(distant.sin_addr), AF_INET );/*InfosClients */
       if (host) { g_snprintf( client->machine, TAILLE_MACHINE, "%s", host->h_name ); }   /* Nom en clair */
            else { g_snprintf( client->machine, TAILLE_MACHINE, "%s",               /* Ou bien Adresse IP */
                               (gchar *)inet_ntoa(distant.sin_addr) );
                 }
       time( &client->date_connexion );                /* Enregistrement de la date de debut de connexion */
       client->pulse = Partage->top;
       client->Id_serveur = ss_id;
       client->courbe.num = -1;                           /* Init: pas de courbe a envoyer pour le moment */
       pthread_mutex_init( &client->mutex_write, NULL );
       pthread_mutex_init( &client->mutex_struct_used, NULL );
       client->struct_used = 0;                            /* Par défaut, personne n'utilise la structure */

       client->Db_watchdog = Init_DB_SQL( Config.log );
       if (!client->Db_watchdog)
        { Info_new( Config.log, Config.log_all, LOG_ERR,
                  "Accueillir_nouveaux_client: Unable to open database" );
          Deconnecter( client );
        }
       else
        { pthread_mutex_lock( &Partage->Sous_serveur[ss_id].synchro );
          Partage->Sous_serveur[ss_id].Clients = g_list_append( Partage->Sous_serveur[ss_id].Clients, client );
          pthread_mutex_unlock( &Partage->Sous_serveur[ss_id].synchro );
          Info_new( Config.log, Config.log_all, LOG_INFO,
                   "Accueillir_un_client: Connexion accepted (id=%d) from %s", id, client->machine );
          Client_mode( client, ENVOI_INTERNAL );
          return(TRUE);
        }
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Run_serveur boucle principale d'un serveur Watchdog                                                    */
/* Entree: l'id du serveur et le pid du pere                                                              */
/* Sortie: un code d'erreur EXIT_xxx                                                                      */
/**********************************************************************************************************/
 void Run_serveur ( gint id )
  { time_t version_d_serveur;
    struct sigaction sig;
    pthread_t tid;
    gchar nom[16];

    g_snprintf(nom, sizeof(nom), "W-SRV%03d", id );
    prctl(PR_SET_NAME, nom, 0, 0, 0 );

    setlocale( LC_ALL, "C" );                        /* Pour le formattage correct des , . dans les float */
    sig.sa_handler = SIG_IGN;
    sig.sa_flags = SA_RESTART;        /* Voir Linux mag de novembre 2002 pour le flag anti cut read/write */
    sigfillset (&sig.sa_mask);                                /* Par défaut tous les signaux sont bloqués */
    sigaction( SIGINT, &sig, NULL );                                               /* On ignore le SIGINT */

                                                  /* Initialisation de la zone interne et comm du serveur */
    Partage->Sous_serveur[id].inactivite = Partage->top;                     /* On prend l'heure actuelle */
    Partage->Sous_serveur[id].Thread_run = TRUE;                                    /* Le thread tourne ! */
    Partage->Sous_serveur[id].Clients = NULL;                     /* Au départ, nous n'avons aucun client */

    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_serveur: Enable", id );
         
    while( Partage->Sous_serveur[id].Thread_run == TRUE )                /* On tourne tant que necessaire */
     { if (Partage->jeton == id)                                                /* Avons nous le jeton ?? */
        { if (Accueillir_un_client( id ) == TRUE)                         /* Un client vient d'arriver ?? */
           { Partage->jeton = -1;                                /* On signale que l'on accepte le client */
             Info_new( Config.log, Config.log_all, LOG_INFO, "Run_serveur: jeton rendu (%d)", id );
           }
        }

       if (Partage->Sous_serveur[id].Thread_sigusr1)                          /* Gestion des signaux USR1 */
        { pthread_mutex_lock( &Partage->Sous_serveur[id].synchro );
          Info_new( Config.log, Config.log_all, LOG_INFO, "Run_serveur: id %d, pid %d, nbr_client %d",
                      id, (guint)Partage->Sous_serveur[id].pid, g_list_length(Partage->Sous_serveur[id].Clients) );
          pthread_mutex_unlock( &Partage->Sous_serveur[id].synchro );
          Partage->Sous_serveur[id].Thread_sigusr1 = FALSE;
        }

       if (Partage->Sous_serveur[id].Thread_reload)                         /* Gestion des signaux RELOAD */
        { Partage->Sous_serveur[id].Thread_reload = FALSE;
          Info_new( Config.log, Config.log_all, LOG_INFO, "Run_serveur: RELOAD (%d)", id );
        }

       if (Partage->Sous_serveur[id].Clients)                                    /* Si il y a des clients */
        { gint new_mode;
          GList *liste;

          Partage->Sous_serveur[id].inactivite = Partage->top;
          liste = Partage->Sous_serveur[id].Clients;
          while(liste)                                                /* Parcours de la liste des clients */
           { struct CLIENT *client;
             client = (struct CLIENT *)liste->data;

             if (client->mode == DECONNECTE && client->struct_used == 0)      /* Deconnection des clients */
              { Deconnecter( client );
                break;
              }

             switch (client->mode)
              { case ENVOI_INTERNAL:
                     Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_PAQUETSIZE,
                                   NULL, client->connexion->taille_bloc );
                     if (Config.ssl_crypt)
                      { Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_SSLNEEDED, NULL, 0 ); 
                        Client_mode ( client, ATTENTE_CONNEXION_SSL );
                      }
                     else
                      { Client_mode ( client, ATTENTE_IDENT ); }
                     Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_END,                /* Tag de fin */
                                   NULL, 0 );
                     break;                 
                case ATTENTE_CONNEXION_SSL:
                     Connecter_ssl ( client );                        /* Tentative de connexion securisée */
                     break;
                case ENVOI_AUTORISATION :
                     new_mode = Tester_autorisation( id, client );
                     if (new_mode == ENVOI_DONNEES)/* Optimisation si pas necessaire */
                      { version_d_serveur = Lire_version_donnees( Config.log );
                        if ( version_d_serveur > client->ident.version_d )
                         {  gint taille;
                            taille = 0;
                            taille += Ajouter_repertoire_liste( client, "Gif", client->ident.version_d );
                            client->transfert.taille = taille;
                         }
                      }
                     Client_mode ( client, new_mode );
                     break;
                case ENVOI_DONNEES      :
                     if(Envoyer_gif( client )) Client_mode (client, ENVOI_HISTO);
                     break;
                case ENVOI_HISTO        :
                     Client_mode( client, VALIDE_NON_ROOT );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_histo_thread, client );
                     pthread_detach( tid );
                     break;

                case VALIDE_NON_ROOT    : /*Client_mode(client, VALIDE); */           /* Etat transitoire */
                     break;
                case ENVOI_GROUPE_FOR_UTIL:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_groupes_pour_util_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_GROUPE_FOR_SYNOPTIQUE:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_groupes_pour_synoptique_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_GROUPE_FOR_PROPRIETE_SYNOPTIQUE:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_groupes_pour_propriete_synoptique_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_SOURCE_DLS   :
                     if(Envoyer_source_dls( client )) Client_mode(client, VALIDE);
                     break;
                case ENVOI_MNEMONIQUE_FOR_COURBE:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_mnemoniques_for_courbe_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_MNEMONIQUE_FOR_HISTO_COURBE:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_mnemoniques_for_histo_courbe_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_MOTIF_ATELIER:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_motif_atelier_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_COMMENT_ATELIER:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_comment_atelier_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_PASSERELLE_ATELIER:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_passerelle_atelier_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_CAPTEUR_ATELIER:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_capteur_atelier_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_CAMERA_SUP_ATELIER:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_camera_sup_atelier_thread, client );
                     pthread_detach( tid );
                     break;

                case ENVOI_MOTIF_SUPERVISION:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_motif_supervision_thread, client );
                     pthread_detach( tid );
                     break;   
                case ENVOI_COMMENT_SUPERVISION:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_comment_supervision_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_PASSERELLE_SUPERVISION:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_passerelle_supervision_thread, client );
                     pthread_detach( tid );
                     break;   
                case ENVOI_PALETTE_SUPERVISION:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_palette_supervision_thread, client );
                     pthread_detach( tid );
                     break;   
                case ENVOI_CAPTEUR_SUPERVISION:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_capteur_supervision_thread, client );
                     pthread_detach( tid );
                     break;   
                case ENVOI_CAMERA_SUP_SUPERVISION:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_camera_sup_supervision_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_IXXX_SUPERVISION :
                     Client_mode( client, VALIDE );
                     Ref_client( client );  /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_bit_init_supervision_thread, client );
                     pthread_detach( tid );
                     break;   
                case ENVOI_ICONE_FOR_ATELIER:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_icones_pour_atelier_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_CLASSE_FOR_ATELIER:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_classes_pour_atelier_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_SYNOPTIQUE_FOR_ATELIER:
                     Client_mode( client, VALIDE );
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_synoptiques_pour_atelier_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE:
                     Client_mode( client, VALIDE );                             /* Si pas de comments ... */
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_synoptiques_pour_atelier_palette_thread, client );
                     pthread_detach( tid );
                     break;
                case ENVOI_PALETTE_FOR_ATELIER_PALETTE:
                     Client_mode( client, VALIDE );                             /* Si pas de comments ... */
                     Ref_client( client );                       /* Indique que la structure est utilisée */
                     pthread_create( &tid, NULL, (void *)Envoyer_palette_atelier_thread, client );
                     pthread_detach( tid );
                     break;
              }
/****************************************** Envoi des chaines capteurs ************************************/
             if (client->mode == VALIDE && client->bit_capteurs && client->date_next_send_capteur < Partage->top)
              { struct CAPTEUR *capteur;
                GList *liste_capteur;
                client->date_next_send_capteur = Partage->top + TEMPS_UPDATE_CAPTEUR;
                liste_capteur = client->bit_capteurs;
                while (liste_capteur)                                 /* Pour tous les capteurs du client */
                 { capteur = (struct CAPTEUR *)liste_capteur->data;

                   if (Tester_update_capteur(capteur))             /* Doit-on updater le capteur client ? */
                    { struct CMD_ETAT_BIT_CAPTEUR *etat;
                      etat = Formater_capteur(capteur);                /* Formatage de la chaine associée */
                      if (etat)                                                           /* envoi client */
                       { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_CAPTEUR,
                                       (gchar *)etat, sizeof(struct CMD_ETAT_BIT_CAPTEUR) );
                         g_free(etat);                                            /* On libere la mémoire */
                       }
                      else Info_new( Config.log, Config.log_all, LOG_ERR, "Not enought memory envoi capteur" );
                    }
                   liste_capteur = liste_capteur->next;                    /* On passe au capteur suivant */
                 }
              }
/****************************************** Envoi des courbes analogiques *********************************/
             if (client->mode == VALIDE && client->courbes)
              { static gint update;
                struct CMD_TYPE_COURBE *courbe;
                GList *liste_courbe;

                if (update < Partage->top)
                 { struct CMD_APPEND_COURBE envoi_courbe;
                   update = Partage->top + COURBE_TEMPS_TOP*10;/* Refresh toutes les 5 secondes au client */
                   envoi_courbe.date = time(NULL);

                   liste_courbe = client->courbes;
                   while (liste_courbe)
                    { courbe = (struct CMD_TYPE_COURBE *)liste_courbe->data;

                      envoi_courbe.slot_id = courbe->slot_id;
                      envoi_courbe.type    = courbe->type;
                              
                      switch (courbe->type)
                       { case MNEMO_SORTIE:
                              envoi_courbe.val_avant_ech = 1.0*A(courbe->num);
                              Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                            (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                              break;
                         case MNEMO_ENTREE:
                              envoi_courbe.val_avant_ech = 1.0*E(courbe->num);
                              Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                            (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                              break;
                         case MNEMO_ENTREE_ANA:
                              envoi_courbe.val_avant_ech = Partage->ea[courbe->num].val_avant_ech;
                              Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                            (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                              break;
                         default: printf("type courbe inconnu\n");
                       }
                      liste_courbe = liste_courbe->next;
                    }
                 }
              }


             if (client->mode >= ATTENTE_IDENT) Ecouter_client( id, client );

             if (Partage->top > client->pulse && client->mode == VALIDE)          /* Gestion du KEEPALIVE */
              { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_PULSE, NULL, 0 );
                SQL_ping ( Config.log, client->Db_watchdog );/* A remplacer plus tard par des connexions furtives */
                client->pulse = Partage->top + TEMPS_PULSE;
              }

             liste = liste->next;
           }
        }
       else
       if ( Partage->Sous_serveur[id].inactivite + 10*Config.max_inactivite < Partage->top )/* Inactivite ? */
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Inactivity time reached" );
          Partage->Sous_serveur[id].Thread_run = FALSE;                       /* Arret "Local" du process */
        }
/****************************************** Ecoute des messages histo  ************************************/
       if (Partage->Sous_serveur[id].new_histo)
        { struct CMD_TYPE_HISTO *histo;

          pthread_mutex_lock( &Partage->Sous_serveur[id].synchro );
          histo = (struct CMD_TYPE_HISTO *) Partage->Sous_serveur[id].new_histo->data;
          Partage->Sous_serveur[id].new_histo = g_list_remove ( Partage->Sous_serveur[id].new_histo, histo );
          pthread_mutex_unlock( &Partage->Sous_serveur[id].synchro );
       
          Envoi_clients( id, TAG_HISTO, SSTAG_SERVEUR_SHOW_HISTO,
                         (gchar *)histo, sizeof(struct CMD_TYPE_HISTO) );
          g_free(histo);
        }

       if (Partage->Sous_serveur[id].del_histo)
        { struct CMD_TYPE_HISTO *histo;

          pthread_mutex_lock( &Partage->Sous_serveur[id].synchro );
          histo = (struct CMD_TYPE_HISTO *) Partage->Sous_serveur[id].del_histo->data;
          Partage->Sous_serveur[id].del_histo = g_list_remove ( Partage->Sous_serveur[id].del_histo, histo );
          pthread_mutex_unlock( &Partage->Sous_serveur[id].synchro );
       
          Envoi_clients( id, TAG_HISTO, SSTAG_SERVEUR_DEL_HISTO,
                         (gchar *)histo, sizeof(struct CMD_TYPE_HISTO) );
          g_free(histo);
        }

       if (Partage->Sous_serveur[id].new_motif)
        { GList *liste_clients;
          struct CLIENT *client;
          struct CMD_ETAT_BIT_CTRL *motif;

          pthread_mutex_lock( &Partage->Sous_serveur[id].synchro );
          motif = (struct CMD_ETAT_BIT_CTRL *) Partage->Sous_serveur[id].new_motif->data;
          Partage->Sous_serveur[id].new_motif = g_list_remove ( Partage->Sous_serveur[id].new_motif, motif );
          pthread_mutex_unlock( &Partage->Sous_serveur[id].synchro );

          liste_clients = Partage->Sous_serveur[id].Clients;
          while (liste_clients)
           { client = (struct CLIENT *)liste_clients->data;
             if ( g_list_find( client->bit_syns, GINT_TO_POINTER(motif->num) ) )
              { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF,
                              (gchar *)motif, sizeof(struct CMD_ETAT_BIT_CTRL) );
              }
             liste_clients = liste_clients->next;
           }
          g_free(motif);
        }

       usleep(1000); sched_yield();                                /* On ne sature pas le microprocesseur */
     }
/********************************************* Arret du serveur *******************************************/
    if (Partage->jeton == id)
     { Partage->jeton = -1;                                            /* On rend le jeton le cas échéant */
       Info_new( Config.log, Config.log_all, LOG_INFO, "Run_serveur: jeton rendu (%d)", id );
     }

    while(Partage->Sous_serveur[id].Clients)                          /* Parcours de la liste des clients */
     { struct CLIENT *client;                                         /* Deconnection de tous les clients */
       client = (struct CLIENT *)Partage->Sous_serveur[id].Clients->data;
       Info_new( Config.log, Config.log_all, LOG_INFO, "Run_serveur: deconnexion client from %s", client->machine );
       Deconnecter(client);
     }

    Partage->Sous_serveur[id].Clients   = NULL;
    Partage->Sous_serveur[id].pid       = 0;
    if (Partage->Sous_serveur[id].new_histo)
     { g_list_foreach( Partage->Sous_serveur[id].new_histo, (GFunc) g_free, NULL );
       g_list_free ( Partage->Sous_serveur[id].new_histo );
       Partage->Sous_serveur[id].new_histo = NULL;
     }
    if (Partage->Sous_serveur[id].del_histo)
     { g_list_foreach( Partage->Sous_serveur[id].del_histo, (GFunc) g_free, NULL );
       g_list_free ( Partage->Sous_serveur[id].del_histo );
       Partage->Sous_serveur[id].del_histo = NULL;
     }
    if (Partage->Sous_serveur[id].new_motif)
     { g_list_foreach( Partage->Sous_serveur[id].new_motif, (GFunc) g_free, NULL );
       g_list_free ( Partage->Sous_serveur[id].new_motif );
       Partage->Sous_serveur[id].new_motif = NULL;
     }
    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_serveur: Down (id=%d)", id );
    pthread_exit( NULL );
  }
/*--------------------------------------------------------------------------------------------------------*/
