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

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <fcntl.h>
 
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static GtkWidget *F_ident;                                  /* Fenetre d'identification de l'utilisateur */

 extern GtkWidget *Barre_status;                                         /* Barre d'etat de l'application */
 extern struct CLIENT Client_en_cours;                           /* Identifiant de l'utilisateur en cours */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
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
 void Deconnecter_sale ( void )
  { Fermer_connexion(Client_en_cours.connexion);
    Client_en_cours.connexion = NULL;
    Log ( _("Disconnected") );
    Client_en_cours.mode = INERTE;
    Info( Client_en_cours.config.log, DEBUG_CONNEXION, "client en mode INERTE" );
    gnome_appbar_clear_stack( GNOME_APPBAR(Barre_status) );
    Effacer_pages();                                                      /* Efface les pages du notebook */
    Raz_progress();
  }
/**********************************************************************************************************/
/* Deconnecter: libere la mémoire et deconnecte le client                                                 */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Deconnecter ( void )
  { if (!Client_en_cours.connexion) return;
    Envoyer_reseau( Client_en_cours.config.log, Client_en_cours.connexion,
                    W_SERVEUR, TAG_CONNEXION, SSTAG_CLIENT_OFF, NULL, 0 );
    Deconnecter_sale();
  }
/**********************************************************************************************************/
/* Envoi_serveur: Envoi d'un paquet au serveur                                                            */
/* Entrée: des infos sur le paquet à envoyer                                                              */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 gboolean Envoi_serveur ( gint tag, gint ss_tag, gchar *buffer, gint taille )
  { if ( Envoi_serveur_reel( &Client_en_cours, tag, ss_tag, buffer, taille ) == FALSE )
     { Deconnecter_sale();
       Log ( _("Disconnected (server offline ?)") );
       return(FALSE);
     }
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

    if (Client_en_cours.connexion) return;
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
    gtk_entry_set_text( GTK_ENTRY(Entry_serveur), Client_en_cours.config.serveur );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_serveur, 1, 3, 0, 1 );

    texte = gtk_label_new( _("Name") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Entry_nom = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_nom), NBR_CARAC_LOGIN );
    gtk_entry_set_text( GTK_ENTRY(Entry_nom), Client_en_cours.config.user );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_nom, 1, 3, 1, 2 );

    texte = gtk_label_new( _("Password") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 2, 3 );
    Entry_code = gtk_entry_new();
    gtk_entry_set_visibility( GTK_ENTRY(Entry_code), FALSE );
    gtk_entry_set_max_length( GTK_ENTRY(Entry_code), NBR_CARAC_LOGIN );
#warning "To be cleaned UP"
    gtk_entry_set_text( GTK_ENTRY(Entry_code), "bouh" );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_code, 1, 3, 2, 3 );

    g_signal_connect_swapped( Entry_serveur, "activate", (GCallback)gtk_widget_grab_focus, Entry_nom );
    g_signal_connect_swapped( Entry_nom, "activate", (GCallback)gtk_widget_grab_focus, Entry_code );

    gtk_widget_grab_focus( Entry_nom );
    gtk_widget_show_all( frame );
    retour = gtk_dialog_run( GTK_DIALOG(F_ident) );                /* Attente de reponse de l'utilisateur */

    if (retour == GTK_RESPONSE_CANCEL || retour == GTK_RESPONSE_DELETE_EVENT)            /* Si annulation */
         { gtk_widget_destroy( F_ident );
           return;
         }
    else { gint retour;
           memcpy( Client_en_cours.user,    gtk_entry_get_text( GTK_ENTRY(Entry_nom) ),
                   sizeof(Client_en_cours.user) );
           memcpy( Client_en_cours.password,gtk_entry_get_text( GTK_ENTRY(Entry_code) ),
                   sizeof(Client_en_cours.password) );
           memcpy( Client_en_cours.serveur ,gtk_entry_get_text( GTK_ENTRY(Entry_serveur) ),
                   sizeof(Client_en_cours.serveur) );

           gtk_widget_destroy( F_ident );                                      /* Fermeture de la fenetre */
           retour = Connecter_au_serveur( &Client_en_cours );   /* Essai de connexion au serveur Watchdog */
           switch ( retour )                       
            { case -1: Log( _("DNS failed") );
                       break;
              case -2: Log( _("Socket creation failed") );
                       break;
              case -3: Log( _("connexion refused by server"));
                       break;
              case -4: Log( _("memory problem"));
                       break;
              case -5: Log( _("Impossible de creer le contexte SSL") );
                       break;
              case 0:  Log( _("Waiting for authorization") );
                       break;
              default: Log( _("Unknown error") );
                       break;
            }
           if (retour) Deconnecter_sale();
         }
  }
/*--------------------------------------------------------------------------------------------------------*/
