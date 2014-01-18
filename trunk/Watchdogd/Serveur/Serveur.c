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
 #include "Sous_serveur.h"

/**********************************************************************************************************/
/* Ssrv_Lire_config : Lit la config Watchdog et rempli la structure mémoire                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Ssrv_Lire_config ( void )
  { GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Ssrv_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_ssrv.lib->Thread_debug = g_key_file_get_boolean ( gkf, "SERVER", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */

/********************************************* Partie SERVER **********************************************/
    Cfg_ssrv.ssl_crypt          = g_key_file_get_boolean ( gkf, "SERVER", "ssl_crypt", NULL );

    Cfg_ssrv.port               = g_key_file_get_integer ( gkf, "SERVER", "port", NULL );
    if (!Cfg_ssrv.port) Cfg_ssrv.port = DEFAUT_PORT;

    Cfg_ssrv.max_client         = g_key_file_get_integer ( gkf, "SERVER", "max_client", NULL );
    if (!Cfg_ssrv.max_client) Cfg_ssrv.max_client = DEFAUT_MAX_CLIENT;

    Cfg_ssrv.min_serveur        = g_key_file_get_integer ( gkf, "SERVER", "min_serveur", NULL );
    if (!Cfg_ssrv.min_serveur) Cfg_ssrv.min_serveur = DEFAUT_MIN_SERVEUR;

    Cfg_ssrv.max_serveur        = g_key_file_get_integer ( gkf, "SERVER", "max_serveur", NULL );
    if (!Cfg_ssrv.max_serveur) Cfg_ssrv.max_serveur = DEFAUT_MAX_SERVEUR;

    Cfg_ssrv.max_inactivite     = g_key_file_get_integer ( gkf, "SERVER", "max_inactivite", NULL );
    if (!Cfg_ssrv.max_inactivite) Cfg_ssrv.max_inactivite = DEFAUT_MAX_INACTIVITE;

    Cfg_ssrv.max_login_failed   = g_key_file_get_integer ( gkf, "SERVER", "max_login_failed", NULL );
    if (!Cfg_ssrv.max_login_failed) Cfg_ssrv.max_login_failed = DEFAUT_MAX_LOGIN_FAILED;

    Cfg_ssrv.timeout_connexion  = g_key_file_get_integer ( gkf, "SERVER", "timeout_connexion", NULL );
    if (!Cfg_ssrv.timeout_connexion) Cfg_ssrv.timeout_connexion = DEFAUT_TIMEOUT_CONNEXION;

    Cfg_ssrv.taille_clef_dh     = g_key_file_get_integer ( gkf, "SERVER", "taille_clef_dh", NULL );
    if (!Cfg_ssrv.taille_clef_dh) Cfg_ssrv.taille_clef_dh = DEFAUT_TAILLE_CLEF_DH;

    Cfg_ssrv.taille_clef_rsa    = g_key_file_get_integer ( gkf, "SERVER", "taille_clef_rsa", NULL );
    if (!Cfg_ssrv.taille_clef_rsa) Cfg_ssrv.taille_clef_rsa = DEFAUT_TAILLE_CLEF_RSA;

    Cfg_ssrv.taille_bloc_reseau = g_key_file_get_integer ( gkf, "SERVER", "taille_bloc_reseau", NULL );
    if (!Cfg_ssrv.taille_bloc_reseau) Cfg_ssrv.taille_bloc_reseau = DEFAUT_TAILLE_BLOC_RESEAU;

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Ssrv_Liberer_config : Libere la mémoire allouer précédemment pour lire la config imsg                 */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Ssrv_Liberer_config ( void )
  {
  }
/**********************************************************************************************************/
/* Ref_client et Unref_client servent a referencer ou non une structure CLIENT en mémoire                 */
/* Entrée: un client                                                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Unref_client ( struct CLIENT *client )
  { pthread_mutex_lock( &client->mutex_struct_used );
    if (client->struct_used) client->struct_used--;
    pthread_mutex_unlock( &client->mutex_struct_used );

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Unref_client: struct_used = %d for %s(SSRV%06d) ",
              client->struct_used, client->machine, client->ssrv_id );

    if (client->struct_used == 0)
     { pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
       Cfg_ssrv.Clients = g_slist_remove( Cfg_ssrv.Clients, client );
       pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
    
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Unref_client: struct_used = 0. closing  %s(SSRV%06d)... ",
                 client->machine, client->ssrv_id );
       Fermer_connexion( client->connexion );
       pthread_mutex_destroy( &client->mutex_struct_used );
       if (client->util)            { g_free( client->util ); }
       if (client->Liste_bit_syns)  { g_slist_free(client->Liste_bit_syns); }
       if (client->bit_init_syn)    { g_list_free(client->bit_init_syn); }
       if (client->bit_capteurs)    { g_list_foreach( client->bit_capteurs, (GFunc) g_free, NULL );
                                      g_list_free(client->bit_capteurs);
                                    }
       if (client->bit_init_capteur)
                                    { g_list_foreach( client->bit_init_capteur, (GFunc) g_free, NULL );
                                      g_list_free(client->bit_init_capteur);
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
/**********************************************************************************************************/
/* Ref_client et Unref_client servent a referencer ou non une structure CLIENT en mémoire                 */
/* Entrée: un client                                                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Ref_client ( struct CLIENT *client )
  { pthread_mutex_lock( &client->mutex_struct_used );
    client->struct_used++;
    pthread_mutex_unlock( &client->mutex_struct_used );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Ref_client: struct_used = %d for %s(SSRV%06d)... ",
              client->struct_used, client->machine, client->ssrv_id );
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
       case WAIT_FOR_IDENT              : return("WAIT_FOR_IDENT");
       case WAIT_FOR_HASH               : return("WAIT_FOR_HASH");
       case WAIT_FOR_NEWPWD             : return("WAIT_FOR_NEWPWD");
       case ENVOI_HISTO                 : return("ENVOI_HISTO");

       case ENVOI_GROUPE_FOR_UTIL       : return("ENVOI_GROUPE_FOR_UTIL");
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
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "Client_mode: positionnement impossible pour %s car mode = DECONNECTE", client->util->nom);
       return;
     }

    if (client->mode == VALIDE_NON_ROOT && mode == VALIDE)                    /* Nous prevenons le client */
     { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_CLI_VALIDE, NULL, 0 ); }
    client->mode = mode;
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "Client_mode: client %s (SSRV%06d) en mode %s",
                 client->machine, client->ssrv_id, Mode_vers_string(mode) );
  }
/**********************************************************************************************************/
/* Deconnecter: Deconnection d'un client                                                                  */
/* Entrée: un client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Deconnecter ( struct CLIENT *client )
  { client->mode = VALIDE;                            /* Envoi un dernier paquet "OFF" avant deconnexion" */
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
              "Deconnecter: deconnexion client %s", client->machine );
    Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_OFF, NULL, 0 );
    client->mode = DECONNECTE;
    Unref_client( client ); 
  }
/**********************************************************************************************************/
/* Ssrv_Gerer_message: Ajoute une demande d'envoi des messages aux thread                                 */
/* Entrées: le numéro de la sortie                                                                        */
/**********************************************************************************************************/
 static void Ssrv_Gerer_histo( struct CMD_TYPE_HISTO *histo )
  { gint taille;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_ssrv.Liste_histo );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Ssrv_Gerer_histo: DROP (taille>150) msg=%d", histo->msg.num );
       g_free(histo);
       return;
     }

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Ssrv_Gerer_histo: Message a traiter : msg=%d", histo->msg.num );
       
    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                /* Ajout dans la liste de tell a traiter */
    Cfg_ssrv.Liste_histo = g_slist_append( Cfg_ssrv.Liste_histo, histo ); /* Append pour l'ordre d'arrive ! */
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
  }
/**********************************************************************************************************/
/* Envoyer_histo_aux_thread: duplique l'histo recu et en envoi la copie a chacun des thread               */
/* Entrée : néant                                                                                         */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
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
                   "Envoyer_histo_aux_threads: Envoi du MSG%d=%d au client %s",
                   dup_histo->msg.num, dup_histo->alive, client->machine );
          client->Liste_histo = g_slist_append ( client->Liste_histo, dup_histo );
        }
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
    g_free(histo);                                      /* On a plus besoin de la structure, on la libere */
  }
/**********************************************************************************************************/
/* Ssrv_Gerer_motif: Ajoute une demande d'envoi des motif Ixxx aux thread                                 */
/* Entrées: le numéro de la sortie                                                                        */
/**********************************************************************************************************/
 static void Ssrv_Gerer_motif( gint num )
  { gint taille;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_ssrv.Liste_motif );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Ssrv_Gerer_motif: DROP (taille>150) mum=%d", num );
       return;
     }

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Ssrv_Gerer_motif: Motif a traiter : num=%d", num );
       

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                /* Ajout dans la liste de tell a traiter */
    Cfg_ssrv.Liste_motif = g_slist_append( Cfg_ssrv.Liste_motif, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
  }
/**********************************************************************************************************/
/* Envoyer_motif_aux_thread: duplique le motif recu et en envoi la copie a chacun des thread              */
/* Entrée : néant                                                                                         */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void Envoyer_motif_aux_threads ( void )
  { struct CMD_ETAT_BIT_CTRL motif;
    GSList *liste;
    gint num;
    
    if ( Cfg_ssrv.Liste_motif == NULL ) return;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    num = GPOINTER_TO_INT (Cfg_ssrv.Liste_motif->data);
    Cfg_ssrv.Liste_motif = g_slist_remove ( Cfg_ssrv.Liste_motif, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );

/***************************** Création de la structure passée aux clients ********************************/
    motif.num    = num;
    motif.etat   = Partage->i[num].etat;
    motif.rouge  = Partage->i[num].rouge;
    motif.vert   = Partage->i[num].vert;
    motif.bleu   = Partage->i[num].bleu;
    motif.cligno = Partage->i[num].cligno;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );                              /* Envoi a tous les thread */
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
                "Envoyer_motif_aux_threads: Envoi du I%03d = %d, r%03d, v%03d, b%03d, c%d au thread %06d",
                dup_motif->num, dup_motif->etat,
                dup_motif->rouge, dup_motif->vert, dup_motif->bleu,
                dup_motif->cligno, client->ssrv_id );
       client->Liste_new_motif = g_slist_append ( client->Liste_new_motif, dup_motif );
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
  }
/**********************************************************************************************************/
/* Accueillir_nouveaux_clients: Cette fonction permet de loguer d'éventuels nouveaux clients distants     */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE si un nouveau client est arrivé                                                           */
/**********************************************************************************************************/
 static struct CLIENT *Accueillir_un_client( void )
  { struct sockaddr_in distant;
    guint taille_distant, id;
    struct CLIENT *client;
    struct hostent *host;

    taille_distant = sizeof(distant);
    if ( (id=accept( Cfg_ssrv.Socket_ecoute, (struct sockaddr *)&distant, &taille_distant )) != -1)  /* demande ?? */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
                "Accueillir_nouveaux_client: Connexion wanted. ID=%d", id );

       client = g_try_malloc0( sizeof(struct CLIENT) );  /* On alloue donc une nouvelle structure cliente */
       if (!client) { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                               "Accueillir_nouveaux_client: Not enought memory to connect %d", id );
                      close(id);
                      return(NULL);                                     /* On traite bien sûr les erreurs */
                    }

       client->connexion = Nouvelle_connexion( Config.log, id, Cfg_ssrv.taille_bloc_reseau );
       if (!client->connexion)
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                   "Accueillir_nouveaux_client: Not enought memory for %d", id );
          close(id);
          g_free( client );
          return(NULL);
        }

       host = gethostbyaddr( (char*)&distant.sin_addr, sizeof(distant.sin_addr), AF_INET );/*InfosClients */
       if (host) { g_snprintf( client->machine, TAILLE_MACHINE, "%s", host->h_name ); }   /* Nom en clair */
            else { g_snprintf( client->machine, TAILLE_MACHINE, "%s",               /* Ou bien Adresse IP */
                               (gchar *)inet_ntoa(distant.sin_addr) );
                 }
       time( &client->date_connexion );                /* Enregistrement de la date de debut de connexion */
       client->pulse = Partage->top;
       client->courbe.num = -1;                           /* Init: pas de courbe a envoyer pour le moment */
       pthread_mutex_init( &client->mutex_struct_used, NULL );
       client->struct_used = 1;/* Par défaut, personne la structure est utilisée par le thread de surveilance */

       pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
       Cfg_ssrv.Clients = g_slist_prepend( Cfg_ssrv.Clients, client );
       pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "Accueillir_un_client: Connexion accepted (id=%d) from %s", id, client->machine );
       Client_mode( client, ENVOI_INTERNAL );
       return(client);
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Run_serveur boucle principale d'un sous-serveur Watchdog                                               */
/* Entree: l'id du serveur et le pid du pere                                                              */
/* Sortie: un code d'erreur EXIT_xxx                                                                      */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { gchar nom[32];
    
    g_snprintf(nom, sizeof(nom), "W-SSRV-LISTEN" );
    prctl(PR_SET_NAME, nom, 0, 0, 0 );

    setlocale( LC_ALL, "C" );                        /* Pour le formattage correct des , . dans les float */

                                                  /* Initialisation de la zone interne et comm du serveur */
    memset( &Cfg_ssrv, 0, sizeof(Cfg_ssrv) );                   /* Mise a zero de la structure de travail */
    Cfg_ssrv.lib = lib;                        /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_ssrv.lib->TID = pthread_self();                                 /* Sauvegarde du TID pour le pere */
    Ssrv_Lire_config ();                                /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );
    Cfg_ssrv.lib->Thread_run = TRUE;                                                /* Le thread tourne ! */

    g_snprintf( Cfg_ssrv.lib->admin_prompt, sizeof(Cfg_ssrv.lib->admin_prompt), "ssrv" );
    g_snprintf( Cfg_ssrv.lib->admin_help,   sizeof(Cfg_ssrv.lib->admin_help),   "Manage SSRV Modules" );

    Init_RSA();                                             /* Initialisation des clefs RSA et chargement */
    if (Cfg_ssrv.ssl_crypt) { Cfg_ssrv.Ssl_ctx = Init_ssl();                        /* Initialisation SSL */
                              if (!Cfg_ssrv.Ssl_ctx)
                               { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                                           "Init ssl failed but needed. Stopping..." );
                                 goto end;
                               }
                            }
                       else { Cfg_ssrv.Ssl_ctx = NULL; }

    Cfg_ssrv.Socket_ecoute = Activer_ecoute();                    /* Initialisation de l'écoute via TCPIP */
    if ( Cfg_ssrv.Socket_ecoute<0 )            
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_CRIT, "Network down, foreign connexions disabled" );
       goto end;
     }
    Abonner_distribution_motif ( Ssrv_Gerer_motif );                /* Abonnement a la liste de diffusion */
    Abonner_distribution_histo ( Ssrv_Gerer_histo );                /* Abonnement a la liste de diffusion */

    while(lib->Thread_run == TRUE)                                       /* On tourne tant que necessaire */
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
       if (client)                                                        /* Un client vient d'arriver ?? */
        { pthread_create( &tid, NULL, (void *)Run_handle_client, client );
          pthread_detach( tid );
        }

       Envoyer_histo_aux_threads();                            /* Envoi les histos aux thread s'il y en a */
       Envoyer_motif_aux_threads();                            /* Envoi les motifs aux thread s'il y en a */
      }                                                                    /* Fin du while partage->arret */

    Desabonner_distribution_histo ( Ssrv_Gerer_histo );             /* Abonnement a la liste de diffusion */
    Desabonner_distribution_motif ( Ssrv_Gerer_motif );             /* Abonnement a la liste de diffusion */
    if (Cfg_ssrv.Liste_histo)                                            /* Si la liste est encore pleine */
     { g_slist_foreach( Cfg_ssrv.Liste_histo, (GFunc) g_free, NULL );
       g_slist_free ( Cfg_ssrv.Liste_histo );
     }
end:
    Cfg_ssrv.lib->Thread_run = FALSE;                                       /* Le thread ne tourne plus ! */
    Ssrv_Liberer_config ();                             /* Lecture de la configuration logiciel du thread */
    Liberer_SSL ();                                                                 /* Libération mémoire */
    if (Cfg_ssrv.Socket_ecoute>0) close(Cfg_ssrv.Socket_ecoute);
    if (Cfg_ssrv.rsa) RSA_free( Cfg_ssrv.rsa );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_ssrv.lib->TID = 0;                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
 }
/*--------------------------------------------------------------------------------------------------------*/
