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
    Client.mode = DISCONNECTED;
    Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO, "client en mode DISCONNECTED" );
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
    Envoyer_reseau( Client.connexion, TAG_CONNEXION, SSTAG_CLIENT_OFF, NULL, 0 );
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


 void soupCB (SoupSession *session, SoupMessage *msg, gpointer user_data)
{
  gint status_code;
  printf("test\n");
  g_object_get ( msg, "status-code",&status_code, NULL );
  printf("test\n");
  SoupBuffer *buf = soup_message_body_get_chunk ( msg->response_body, 0 );
  printf("test\n");
  printf("Recu Soup Message %d, %s\n", status_code, msg->response_body->data );
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

    Log( "Trying to connect" );

    SoupSession *session=soup_session_new();
    SoupMessage *msg= soup_message_new ( "GET", "http://wtd-ozoir.abls-habitat.fr/auth/login");

// soup_session_send_message (session, msg);
    soup_session_queue_message (session, msg, soupCB, NULL);              
    //g_object_unref (msg);
    return(TRUE);

    Raz_progress_pulse();

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    g_snprintf( service, sizeof(service), "%d", Config_cli.port_ihm );
    s = getaddrinfo( Client.host, service, &hints, &result);
    if (s != 0)
     { Log( "DNS failed" );
       Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING,
                 "Connecter_au_serveur: DNS failed %s(%s)", Client.host, gai_strerror(s) );
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
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR, "Connecter_au_serveur: cannot create new connexion" );
       Deconnecter();
       return(FALSE);
     }

    Client.mode = ATTENTE_INTERNAL;
    Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO, "client en mode ATTENTE_INTERNAL" );

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
    F_ident = gtk_message_dialog_new ( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
                                       "Identification required" );
    gtk_window_set_resizable (GTK_WINDOW (F_ident), FALSE);

    frame = gtk_frame_new( "Put your ID and password" );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX(gtk_dialog_get_content_area (GTK_DIALOG(F_ident))), frame, TRUE, TRUE, 0 );

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
            { Log( "Waiting for connexion...." ); }
         }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
