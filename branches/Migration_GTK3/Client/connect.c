/******************************************************************************************************************************/
/* Client/connect.c        Gestion du logon user sur module Client Watchdog                                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           sam 16 fév 2008 19:19:02 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * connect.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sébastien Lefevre
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

 #include <gtk/gtk.h>
 #include <libsoup/soup.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <fcntl.h>

 #include "Reseaux.h"
 #include "client.h"
 #include "Config_cli.h"

 static GtkWidget *fenetre;                                                      /* Fenetre d'identification de l'utilisateur */
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
  { soup_session_abort (Client.connexion);
    Client.connexion = NULL;
    /*Client.mode = DISCONNECTED;*/
    /*Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO, "client en mode DISCONNECTED" );*/
#ifdef bouh
    Effacer_pages();                                                                          /* Efface les pages du notebook */
#endif
  }
/******************************************************************************************************************************/
/* Deconnecter: libere la mémoire et deconnecte le client                                                                     */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Deconnecter ( void )
  { if (!Client.connexion) return;
    /*Envoyer_reseau( Client.connexion, TAG_CONNEXION, SSTAG_CLIENT_OFF, NULL, 0 );*/
    Deconnecter_sale();
    Log ( "Disconnected" );
  }
#ifdef bouh
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
#endif


/******************************************************************************************************************************/
/* Connecter_au_serveur_CB: Traite la reponse du serveur a la demande de connexionen                                          */
/* Entrée: les variables traditionnelles de libsous                                                                           */
/* Sortie: l'ihm est mise a jour et les ws sont activées                                                                      */
/******************************************************************************************************************************/
 static void Connecter_au_serveur_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { gchar *reason_phrase;
    gint status_code;
    GBytes *response;
    gsize taille;
    gchar *data;

    g_object_get ( msg, "status-code", &status_code, "reason-phrase", &reason_phrase, NULL );
    if (status_code != 200)
     { gchar chaine[256];
       g_snprintf(chaine, sizeof(chaine), "Error connecting to server : Code %d - %s", status_code, reason_phrase );
       Log(chaine);
       return;
     }
    g_object_get ( msg, "response-body-data", &response, NULL );
    data = g_bytes_unref_to_data ( response, &taille );
    printf("Recu Soup Message status %d, taille %d, %s\n", status_code, taille, data );
    g_free(data);
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 static void Connecter_au_serveur ( void )
  { gchar chaine[128];
    Log( "Trying to connect" );
    Raz_progress_pulse();
    Client.connexion = soup_session_new();
    SoupMessage *msg= soup_message_new ( "GET", "http://localhost:5560/dls/list");
    soup_session_queue_message (Client.connexion, msg, Connecter_au_serveur_CB, NULL);
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
    fenetre = gtk_message_dialog_new ( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
                                       "Identification required" );
    gtk_window_set_resizable (GTK_WINDOW (fenetre), FALSE);

    frame = gtk_frame_new( "Put your ID and password" );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX(gtk_dialog_get_content_area (GTK_DIALOG(fenetre))), frame, TRUE, TRUE, 0 );

    boite = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(boite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), boite );

    table = gtk_grid_new();                                                                   /* Table des entrys identifiant */
    gtk_box_pack_start( GTK_BOX(boite), table, TRUE, TRUE, 0 );
    //gtk_grid_set_row_homogeneous ( GTK_GRID(table), TRUE );
    //gtk_grid_set_column_homogeneous ( GTK_GRID(table), TRUE );
    gtk_grid_set_row_spacing( GTK_GRID(table), 5 );
    gtk_grid_set_column_spacing( GTK_GRID(table), 5 );

    texte = gtk_label_new( "Serveur" );
    gtk_grid_attach( GTK_GRID(table), texte, 0, 0, 1, 1 );
    Entry_host = gtk_entry_new();
    gtk_entry_set_text( GTK_ENTRY(Entry_host), Config_cli.host );
    gtk_grid_attach( GTK_GRID(table), Entry_host, 1, 0, 1, 1 );

    texte = gtk_label_new( "Name" );
    gtk_grid_attach( GTK_GRID(table), texte, 0, 1, 1, 1 );
    Entry_nom = gtk_entry_new();
    gtk_entry_set_text( GTK_ENTRY(Entry_nom), Config_cli.user );
    gtk_grid_attach( GTK_GRID(table), Entry_nom, 1, 1, 1, 1 );

    texte = gtk_label_new( "Password" );
    gtk_grid_attach( GTK_GRID(table), texte, 0, 2, 1, 1 );
    Entry_code = gtk_entry_new();
    gtk_entry_set_visibility( GTK_ENTRY(Entry_code), FALSE );
    gtk_entry_set_text( GTK_ENTRY(Entry_code), Config_cli.passwd );
    gtk_grid_attach( GTK_GRID(table), Entry_code, 1, 2, 1, 1 );

    g_signal_connect_swapped( Entry_host, "activate", (GCallback)gtk_widget_grab_focus, Entry_nom );
    g_signal_connect_swapped( Entry_nom,  "activate", (GCallback)gtk_widget_grab_focus, Entry_code );

    gtk_widget_grab_focus( Entry_nom );
    gtk_widget_show_all( frame );
    retour = gtk_dialog_run( GTK_DIALOG(fenetre) );                                    /* Attente de reponse de l'utilisateur */

    if (retour == GTK_RESPONSE_OK)
     { g_snprintf( Client.hostname, sizeof(Client.hostname), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_host) ) );
       g_snprintf( Client.username, sizeof(Client.username), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_nom) ) );
       g_snprintf( Client.password, sizeof(Client.password), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_code) ) );
       Connecter_au_serveur();                                                      /* Essai de connexion au serveur Watchdog */
     }
    gtk_widget_destroy( fenetre );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
