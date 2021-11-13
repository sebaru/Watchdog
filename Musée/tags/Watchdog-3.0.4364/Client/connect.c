/******************************************************************************************************************************/
/* Client/connect.c        Gestion du logon user sur module Client Watchdog                                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           sam 16 fév 2008 19:19:02 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * connect.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sébastien Lefevre
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

 #include <gnome.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <fcntl.h>
 #include <openssl/err.h>

 #include "Reseaux.h"
 #include "client.h"
 #include "Config_cli.h"

 static GtkWidget *F_ident;                                                      /* Fenetre d'identification de l'utilisateur */
/******************************************** Définitions des prototypes programme ********************************************/
 #include "config.h"
 #include "protocli.h"

 extern struct CLIENT Client;                                                        /* Identifiant de l'utilisateur en cours */
 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */
 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
/******************************************************************************************************************************/
/* Deconnecter: libere la mémoire et deconnecte le client                                                                     */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Deconnecter_sale ( void )
  { Fermer_connexion(Client.connexion);
    Client.connexion = NULL;
    if (Client.ssl_ctx) SSL_CTX_free(Client.ssl_ctx);                                           /* Libération du contexte SSL */
    Client.ssl_ctx = NULL;
    Client.mode = DISCONNECTED;
    Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO, "client en mode DISCONNECTED" );
    Effacer_pages();                                                                          /* Efface les pages du notebook */
  }
/******************************************************************************************************************************/
/* Deconnecter: libere la mémoire et deconnecte le client                                                                     */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Deconnecter ( void )
  { if (!Client.connexion) return;
    Envoyer_reseau( Client.connexion, TAG_CONNEXION, SSTAG_CLIENT_OFF, NULL, 0 );
    Deconnecter_sale();
    Log ( _("Disconnected") );
  }
/******************************************************************************************************************************/
/* Envoi_serveur: Envoi d'un paquet au serveur                                                                                */
/* Entrée: des infos sur le paquet à envoyer                                                                                  */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 gboolean Envoi_serveur ( gint tag, gint ss_tag, gchar *buffer, gint taille )
  { if ( Envoyer_reseau( Client.connexion, tag, ss_tag, buffer, taille ) )
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING, "Deconnexion sur erreur envoi au serveur" );
       Deconnecter_sale();
       Log ( _("Disconnected (server offline ?)") );
       return(FALSE);
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Envoyer_authentification: envoi de l'authentification cliente au serveur                                                   */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Envoyer_authentification ( void )
  { g_snprintf( Client.ident.version, sizeof(Client.ident.version), "%s", VERSION );
    if (!Client.cli_certif)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
                "Envoyer_identification: sending login(%s)/password(XX) and version number(%s)",
                 Client.ident.nom, Client.ident.version
               );
     } else
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
                "Envoyer_identification: sending login(%s) and version number(%s). Certificate already sent",
                 Client.ident.nom, Client.ident.version
               );
     }

    if ( !Envoi_serveur( TAG_CONNEXION, SSTAG_CLIENT_IDENT,
                         (gchar *)&Client.ident, sizeof(struct REZO_CLI_IDENT) ) )
     { Deconnecter();
       return;
     }

    Log ( _("Waiting for authorization") );
    Client.mode = ATTENTE_AUTORISATION;
    Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
             "Envoyer_identification: Client en mode ATTENTE_AUTORISATION" );
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 gboolean Connecter_au_serveur ( void )
  { struct addrinfo *result, *rp;
    struct addrinfo hints;
    gint s;
    gchar service[10];
    int connexion;

    Log( _("Trying to connect") );
    Raz_progress_pulse();

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    g_snprintf( service, sizeof(service), "%d", Config_cli.port_ihm );
    s = getaddrinfo( Client.host, service, &hints, &result);
    if (s != 0)
     { Log( _("DNS failed") );
       Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING,
                 _("Connecter_au_serveur: DNS failed %s(%s)"), Client.host, gai_strerror(s) );
       return(FALSE);
     }

   /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next)
     {
        connexion = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (connexion == -1)
         { Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING,
                    "Connecter_au_serveur: Socket creation failed" );
           continue;
         }

       if (connect(connexion, rp->ai_addr, rp->ai_addrlen) != -1)
        { Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
                   "Connecter_au_serveur: Connect OK to %s (%s) family=%d",
                    Client.host, service, rp->ai_family );
          break;                  /* Success */
        }
       else
        { Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING,
                    "Connecter_au_serveur: connexion refused by server %s (%s) family=%d",
                    Client.host, service, rp->ai_family );
        }
       close(connexion);
     }
    freeaddrinfo(result);
    if (rp == NULL)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING,
                    "Connecter_au_serveur: all family connexion failed for server %s (%s)",
                    Client.host, service );
       Log( "Connexion refused by server" );
       return(FALSE);                                                                                  /* Erreur de connexion */
     }

    Client.connexion = Nouvelle_connexion( Config_cli.log, connexion, -1 );                       /* Creation de la structure */
    if (!Client.connexion)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                 _("Connecter_au_serveur: cannot create new connexion") );
       Deconnecter();
       return(FALSE);
     }

    Client.mode = ATTENTE_INTERNAL;
    Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO, "client en mode ATTENTE_INTERNAL" );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Init_SSL: Initialisation de l'environnement SSL                                                                            */
/* Entrée: rien                                                                                                               */
/* Sortie: un contexte SSL                                                                                                    */
/******************************************************************************************************************************/
 static SSL_CTX *Init_ssl ( void )
  { gchar nom_fichier[80];
    SSL_CTX *ssl_ctx;
    gint retour;
    FILE *fd;

    SSL_load_error_strings();                                                                        /* Initialisation de SSL */
    SSL_library_init();                                                                 /* Init SSL et PRNG: number générator */

    ssl_ctx = SSL_CTX_new ( TLS_client_method() );                                              /* Création d'un contexte SSL */
    if (!ssl_ctx)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                 "Init_ssl : Error creation SSL_CTX_new %s", ERR_error_string( ERR_get_error(), NULL ) );
       return( NULL );
     }

    SSL_CTX_set_mode( ssl_ctx, SSL_MODE_AUTO_RETRY );                                                    /* Mode non bloquant */
    SSL_CTX_set_options( ssl_ctx, SSL_OP_SINGLE_DH_USE );                                        /* Options externe à OpenSSL */

    retour = SSL_CTX_load_verify_locations( ssl_ctx, Config_cli.ssl_file_ca, NULL );
    if (retour != 1)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                 "Init_ssl : load verify locations error (%s, file %s)",
                 ERR_error_string( ERR_get_error(), NULL ), Config_cli.ssl_file_ca );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     } else Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
                     "Init_ssl : load verify locations OK (file %s)", Config_cli.ssl_file_ca );

    SSL_CTX_set_verify( ssl_ctx, SSL_VERIFY_PEER, NULL );                             /* Type de verification des certificats */

    g_snprintf( nom_fichier, sizeof(nom_fichier), "%s.crt", Client.ident.nom );
    fd = fopen( nom_fichier, "r" );
    if (!fd)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                 "Init_ssl : failed to open file certif %s. Falling back to login/password authentication",
                 nom_fichier );
     }
    else                                                                                   /* Authentification par certificat */
     { Client.cli_certif = PEM_read_X509( fd, NULL, NULL, NULL );                                    /* Lecture du certificat */
       fclose(fd);
       if (!Client.cli_certif)
        { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                    "Init_ssl : Certif loading failed %s", nom_fichier );
          SSL_CTX_free(ssl_ctx);
          return(NULL);
        }

       retour = SSL_CTX_use_certificate( ssl_ctx, Client.cli_certif );
       if (retour != 1)
        { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                    "Init_ssl : Use certificate error (%s)", ERR_error_string( ERR_get_error(), NULL ) );
          SSL_CTX_free(ssl_ctx);
          return(NULL);
        }
       Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
                 "Init_ssl : Use of certificate %s", Nom_certif(Client.cli_certif) );
                                                                                                     /* Clef privée du client */
       g_snprintf( nom_fichier, sizeof(nom_fichier), "%s.pem", Client.ident.nom );
       retour = SSL_CTX_use_RSAPrivateKey_file( ssl_ctx, nom_fichier, SSL_FILETYPE_PEM );
       if (retour != 1)
        { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                    "Init_ssl : Error Use RSAPrivate key %s (%s)",
                    nom_fichier, ERR_error_string( ERR_get_error(), NULL ) );
          SSL_CTX_free(ssl_ctx);
          return(NULL);
        }

       retour = SSL_CTX_check_private_key( ssl_ctx );                                          /* Verification du certif/clef */
       if (retour != 1)
        { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                    "Init_ssl : check private key failed %s (%s)",
                    nom_fichier, ERR_error_string( ERR_get_error(), NULL ) );
          SSL_CTX_free(ssl_ctx);
          return(NULL);
        }
     }
    Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO, "SSL initialisation ok" );
    return(ssl_ctx);
  }
/******************************************************************************************************************************/
/* Connecter_ssl: Tentative de connexion sécurisée au serveur                                                                 */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 gboolean Connecter_ssl ( void )
  { gint retour;

    Client.ssl_ctx = Init_ssl();                                                                  /* Creation du contexte SSL */
    if (!Client.ssl_ctx)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                 "Connecter_ssl : Can't initialise SSL" );
       Log( "SSL init failed..." );
       return(FALSE);
     }

    Client.connexion->ssl = SSL_new( Client.ssl_ctx );                                           /* Instanciation du contexte */
    if (Client.connexion->ssl)                                                                    /* Si réussite d'allocation */
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO, "Connecter_ssl: SSL_new OK" );
       SSL_set_fd( Client.connexion->ssl, Client.connexion->socket );
       SSL_set_connect_state( Client.connexion->ssl );                                               /* Nous sommes un client */
     }
    else
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO, "Connecter_ssl: SSL_new failed" );
       Log( "SSL context Init failed" );
       Deconnecter();
       return(FALSE);
     }

encore:
    retour = SSL_connect( Client.connexion->ssl );
    if (retour<=0)
     { retour = SSL_get_error( Client.connexion->ssl, retour );
       if (retour == SSL_ERROR_WANT_READ || retour == SSL_ERROR_WANT_WRITE)
        { /*Info_new( Config_cli.log, Config_cli.log_override, LOG_DEBUG,
                   "Connecter_ssl: SSL_connect need more data" );
          */
          goto encore;
        }

       Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                 "Connecter_ssl: SSL_connect get error %d (%s)",
                 retour, ERR_error_string( ERR_get_error(), NULL ) );
       Log( "SSL_connect failed" );
     }
                                              /* Ici, la connexion a été effectuée, il faut maintenant tester les certificats */
    Client.srv_certif = SSL_get_peer_certificate( Client.connexion->ssl );               /* On prend le certificat du serveur */
    if (!Client.srv_certif)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING,
                "Connecter_ssl: no certificate received" );
       Log( "Aucun certificat serveur recu" );
       Deconnecter();
       return(FALSE);
     }
    Info_new( Config_cli.log, Config_cli.log_override, LOG_NOTICE,
             "Connecter_ssl: certificate received. Algo=%s, keylength=%d",
              (gchar *) SSL_get_cipher_name( Client.connexion->ssl ), SSL_get_cipher_bits( Client.connexion->ssl, NULL ) );

    retour = SSL_get_verify_result( Client.connexion->ssl );                                    /* Verification du certificat */
    if ( retour != X509_V_OK )                                                          /* Si erreur, on se deconnecte presto */
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_NOTICE,
                "Connecter_ssl: unauthorized certificate Error %02d", retour );
       Log( "Certificat serveur non valide" );
       Deconnecter();
       return(FALSE);
     }

    Info_new( Config_cli.log, Config_cli.log_override, LOG_NOTICE,
              "Connecter_ssl: partenaire %s, signataire %s",
              Nom_certif ( Client.srv_certif ), Nom_certif_signataire ( Client.srv_certif ) );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Identifier: Affiche la fenetre d'identification de l'utilisateur                                                           */
/* Entrée: rien                                                                                                               */
/* Sortie: kedal                                                                                                              */
/******************************************************************************************************************************/
 void Connecter ( void )
  { GtkWidget *table, *texte, *boite, *frame;
    GtkWidget *Entry_host, *Entry_nom, *Entry_code;
    gint retour;

    if (Client.connexion) return;
    F_ident = gtk_message_dialog_new ( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
                                       _("Identification required") );
    gtk_window_set_resizable (GTK_WINDOW (F_ident), FALSE);

    frame = gtk_frame_new( _("Put your ID and password") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ident)->vbox ), frame, TRUE, TRUE, 0 );

    boite = gtk_vbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(boite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), boite );

    table = gtk_table_new( 3, 3, TRUE );                                                      /* Table des entrys identifiant */
    gtk_box_pack_start( GTK_BOX(boite), table, TRUE, TRUE, 0 );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );

    texte = gtk_label_new( _("Serveur") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    Entry_host = gtk_entry_new();
    gtk_entry_set_text( GTK_ENTRY(Entry_host), Config_cli.host );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_host, 1, 3, 0, 1 );

    texte = gtk_label_new( _("Name") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Entry_nom = gtk_entry_new();
    gtk_entry_set_text( GTK_ENTRY(Entry_nom), Config_cli.user );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_nom, 1, 3, 1, 2 );

    texte = gtk_label_new( _("Password") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 2, 3 );
    Entry_code = gtk_entry_new();
    gtk_entry_set_visibility( GTK_ENTRY(Entry_code), FALSE );
    gtk_entry_set_text( GTK_ENTRY(Entry_code), Config_cli.passwd );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_code, 1, 3, 2, 3 );

    g_signal_connect_swapped( Entry_host, "activate", (GCallback)gtk_widget_grab_focus, Entry_nom );
    g_signal_connect_swapped( Entry_nom,  "activate", (GCallback)gtk_widget_grab_focus, Entry_code );

    gtk_widget_grab_focus( Entry_nom );
    gtk_widget_show_all( frame );
    retour = gtk_dialog_run( GTK_DIALOG(F_ident) );                                    /* Attente de reponse de l'utilisateur */

    if (retour == GTK_RESPONSE_CANCEL || retour == GTK_RESPONSE_DELETE_EVENT)                                /* Si annulation */
         { gtk_widget_destroy( F_ident );
           return;
         }
    else { g_snprintf( Client.host, sizeof(Client.host), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_host) ) );
           g_snprintf( Client.ident.nom, sizeof(Client.ident.nom), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_nom) ) );
           g_snprintf( Client.ident.passwd, sizeof(Client.ident.passwd), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_code) ) );

           gtk_widget_destroy( F_ident );                                                          /* Fermeture de la fenetre */
           if (Connecter_au_serveur())                                              /* Essai de connexion au serveur Watchdog */
            { Log( _("Waiting for connexion....") ); }
         }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
