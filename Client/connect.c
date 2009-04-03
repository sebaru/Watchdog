/**********************************************************************************************************/
/* Client/connect.c        Gestion du logon user sur module Client Watchdog                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 16 fév 2008 19:19:02 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * connect.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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
 #include "Cst_utilisateur.h"
 
 static GtkWidget *F_ident;                                  /* Fenetre d'identification de l'utilisateur */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *Barre_status;                                         /* Barre d'etat de l'application */
 extern struct CLIENT Client_en_cours;                           /* Identifiant de l'utilisateur en cours */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONNEXION *Connexion;                                              /* connexion au serveur */
 extern SSL_CTX *Ssl_ctx;                                                                 /* Contexte SSL */
/**********************************************************************************************************/
/* Changer_password: procedure de changement de password de l'utilisateur                                 */
/* Entrée: un identifiant utilisateur, et son ancien password                                             */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Changer_password ( void )
  { GtkWidget *dialog, *Entry_code1, *Entry_code2;
    GtkWidget *table, *texte, *boite, *frame;
    gint retour;

    dialog = gtk_message_dialog_new( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
                                     _("You have to change your password") );
    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

    frame = gtk_frame_new( _("Put your new password") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(dialog)->vbox ), frame, TRUE, TRUE, 0 );

    boite = gtk_vbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(boite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), boite );

    table = gtk_table_new( 2, 3, TRUE );                                  /* Table des entrys identifiant */
    gtk_box_pack_start( GTK_BOX(boite), table, TRUE, TRUE, 0 );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );

    texte = gtk_label_new( _("New password") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );

    Entry_code1 = gtk_entry_new();
    gtk_entry_set_visibility( GTK_ENTRY(Entry_code1), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_code1, 1, 3, 0, 1 );

    texte = gtk_label_new( _("New password\n(again)") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );

    Entry_code2 = gtk_entry_new();
    gtk_entry_set_visibility( GTK_ENTRY(Entry_code2), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_code2, 1, 3, 1, 2 );

    g_signal_connect_swapped( Entry_code1, "activate", (GCallback)gtk_widget_grab_focus, Entry_code2 );
one_again:
    gtk_widget_grab_focus( Entry_code1 );
    gtk_widget_show_all( frame );
    retour = gtk_dialog_run( GTK_DIALOG(dialog) );                 /* Attente de reponse de l'utilisateur */

    switch(retour)
     { case GTK_RESPONSE_CANCEL:                                                         /* Si annulation */
       default: gtk_widget_destroy( dialog );
                return(FALSE);
       case GTK_RESPONSE_OK:
        { gchar *code1, *code2;
          struct CMD_UTIL_SETPASSWORD util;

          code1 = (gchar *)gtk_entry_get_text( GTK_ENTRY(Entry_code1) );
          code2 = (gchar *)gtk_entry_get_text( GTK_ENTRY(Entry_code2) );

          if ( g_utf8_collate(code1, code2) )
           { GtkWidget *erreur;
             erreur = gtk_message_dialog_new( GTK_WINDOW(dialog), GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                              _("Passwords do not match") );
             gtk_dialog_run( GTK_DIALOG(erreur) );
             gtk_widget_destroy(erreur);
             goto one_again;
           }
      /*    if ( !g_utf8_collate(code1, Client_en_cours.password) )
           { GtkWidget *erreur;      A faire coté serveur !!!!
             erreur = gtk_message_dialog_new( GTK_WINDOW(dialog), GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                              _("Your REALLY have to\nchange your password") );
             gtk_dialog_run( GTK_DIALOG(erreur) );
             gtk_widget_destroy(erreur);
             goto one_again;
           }*/
          util.id = Client_en_cours.id;
          memcpy( util.code_en_clair, code1, sizeof(util.code_en_clair) );
          Envoi_serveur( TAG_CONNEXION, SSTAG_CLIENT_SETPASSWORD,
                        (gchar *)&util, sizeof(struct CMD_UTIL_SETPASSWORD) );
          gtk_widget_destroy( dialog );                                        /* Fermeture de la fenetre */
         }
      }
     return(TRUE);                                                            /* Normalement, ident=0 ici */
  }
/**********************************************************************************************************/
/* Deconnecter: libere la mémoire et deconnecte le client                                                 */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Deconnecter_sale ( void )
  { Fermer_connexion(Connexion);
    Connexion = NULL;
    Log ( _("Disconnected") );
    Client_en_cours.mode = INERTE;
    Info( Config_cli.log, DEBUG_CONNEXION, "client en mode INERTE" );
    gnome_appbar_clear_stack( GNOME_APPBAR(Barre_status) );
    Effacer_pages();                                                      /* Efface les pages du notebook */
    Raz_progress();
  }
/**********************************************************************************************************/
/* Deconnecter: libere la mémoire et deconnecte le client                                                 */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Deconnecter ( void )
  { if (!Connexion) return;
    Envoyer_reseau( Config_cli.log, Connexion, W_SERVEUR, TAG_CONNEXION, SSTAG_CLIENT_OFF, NULL, 0 );
    Deconnecter_sale();
  }
/**********************************************************************************************************/
/* Envoi_serveur: Envoi d'un paquet au serveur                                                            */
/* Entrée: des infos sur le paquet à envoyer                                                              */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 gboolean Envoi_serveur ( gint tag, gint ss_tag, gchar *buffer, gint taille )
  { if ( Envoyer_reseau( Config_cli.log, Connexion, W_SERVEUR, tag, ss_tag, buffer, taille ) )
     { Info( Config_cli.log, DEBUG_CONNEXION, "Deconnexion sur erreur envoi au serveur" );
       Deconnecter_sale();
       Log ( _("Disconnected (server offline ?)") );
       return(FALSE);
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
 static gboolean Connecter_au_serveur ( void )
  { struct sockaddr_in src;                                            /* Données locales: pas le serveur */
    struct hostent *host;
    int connexion;

    if ( !(host = gethostbyname( Client_en_cours.serveur )) )                          /* On veut l'adresse IP */
     { Log( _("DNS failed") );
       Info_c( Config_cli.log, DEBUG_CONNEXION,
               _("Connecter_au_serveur: DNS failed"), Client_en_cours.serveur );
       return(FALSE);
     }

    src.sin_family = host->h_addrtype;
    memcpy( (char*)&src.sin_addr, host->h_addr, host->h_length );                 /* On recopie les infos */
    src.sin_port = htons( Config_cli.port );

    if ( (connexion = socket( AF_INET, SOCK_STREAM, 0)) == -1)                          /* Protocol = TCP */
     { Log( _("Socket creation failed") );
       Info( Config_cli.log, DEBUG_CONNEXION, _("Connecter_au_serveur: Socket creation failed") );
       return(FALSE);
     }

    if (connect (connexion, (struct sockaddr *)&src, sizeof(src)) == -1)
     { Info_c( Config_cli.log, DEBUG_CONNEXION, _("Connecter_au_serveur: connexion refused by server"),
               Config_cli.serveur );
       Log(_("connexion refused by server"));
       close(connexion);
       return(FALSE);
     }

    Connexion = Nouvelle_connexion( Config_cli.log, connexion,
                                    W_CLIENT, Config_cli.taille_bloc_reseau );
    if (!Connexion)
     { Info( Config_cli.log, DEBUG_CONNEXION, _("Connecter_au_serveur: cannot create new connexion") );
       Deconnecter();
       return(FALSE);       
     }

    Client_en_cours.mode = ATTENTE_CONNEXION_SSL;
    Info( Config_cli.log, DEBUG_CONNEXION, _("Connecter_au_serveur: client en mode ATTENTE_CONNEXION_SSL") );

    Connecter_ssl();                                                           /* Gere les parametres SSL */
    Envoyer_identification();                                        /* Envoi l'identification au serveur */

    return(TRUE);
  }
/**********************************************************************************************************/
/* Identifier: Affiche la fenetre d'identification de l'utilisateur                                       */
/* Entrée: rien                                                                                           */
/* Sortie: kedal                                                                                          */
/**********************************************************************************************************/
 void Connecter ( void )
  { GtkWidget *table, *texte, *boite, *frame;
    GtkWidget *Entry_serveur, *Entry_nom, *Entry_code;
    gint retour;

    if (Connexion) return;
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

    table = gtk_table_new( 3, 3, TRUE );                                  /* Table des entrys identifiant */
    gtk_box_pack_start( GTK_BOX(boite), table, TRUE, TRUE, 0 );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );

    texte = gtk_label_new( _("Serveur") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    Entry_serveur = gtk_entry_new();
#warning "To be cleaned UP"
    gtk_entry_set_text( GTK_ENTRY(Entry_serveur), Config_cli.serveur );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_serveur, 1, 3, 0, 1 );

    texte = gtk_label_new( _("Name") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Entry_nom = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_nom), NBR_CARAC_LOGIN );
#warning "To be cleaned UP"
    gtk_entry_set_text( GTK_ENTRY(Entry_nom), Config_cli.user );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_nom, 1, 3, 1, 2 );

    texte = gtk_label_new( _("Password") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 2, 3 );
    Entry_code = gtk_entry_new();
    gtk_entry_set_visibility( GTK_ENTRY(Entry_code), FALSE );
    gtk_entry_set_max_length( GTK_ENTRY(Entry_code), NBR_CARAC_LOGIN );
#warning "To be cleaned UP"
    gtk_entry_set_text( GTK_ENTRY(Entry_code), "bouh" );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_code, 1, 3, 2, 3 );

    g_signal_connect_swapped( Entry_nom, "activate", (GCallback)gtk_widget_grab_focus, Entry_code );

    gtk_widget_grab_focus( Entry_nom );
    gtk_widget_show_all( frame );
    retour = gtk_dialog_run( GTK_DIALOG(F_ident) );                /* Attente de reponse de l'utilisateur */

    if (retour == GTK_RESPONSE_CANCEL || retour == GTK_RESPONSE_DELETE_EVENT)            /* Si annulation */
         { gtk_widget_destroy( F_ident );
           return;
         }
    else { memcpy( Client_en_cours.user,    gtk_entry_get_text( GTK_ENTRY(Entry_nom) ),
                   sizeof(Client_en_cours.user) );
           memcpy( Client_en_cours.password,gtk_entry_get_text( GTK_ENTRY(Entry_code) ),
                   sizeof(Client_en_cours.password) );
           memcpy( Client_en_cours.serveur ,gtk_entry_get_text( GTK_ENTRY(Entry_serveur) ),
                   sizeof(Client_en_cours.serveur) );

           gtk_widget_destroy( F_ident );                                      /* Fermeture de la fenetre */
           if (Connecter_au_serveur())                          /* Essai de connexion au serveur Watchdog */
            { Log( _("Waiting for SSL handshake") ); }
         }
  }
/*--------------------------------------------------------------------------------------------------------*/
