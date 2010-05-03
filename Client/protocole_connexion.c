/**********************************************************************************************************/
/* Client/protocole_connexion.c    Gestion du protocole_connexion pour la connexion au serveur Watchdog   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 07 avr 2009 21:11:25 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_connexion.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CLIENT Client_en_cours;                           /* Identifiant de l'utilisateur en cours */
 extern GtkWidget *Barre_status;                                         /* Barre d'etat de l'application */
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_connexion ( struct CONNEXION *connexion )
  { GtkWidget *dialog;
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_REFUSE:
             { dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                                 GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                                                 _("You are not authorized\nto connect") );
               g_signal_connect_swapped( dialog, "response",
                                         G_CALLBACK(gtk_widget_destroy), dialog );
               gtk_widget_show_all(dialog);
               Deconnecter_sale();
             }
            break;
       case SSTAG_SERVEUR_ACCOUNT_DISABLED:
             { dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                                                 _("Your account is\ndisabled") );
               g_signal_connect_swapped( dialog, "response",
                                         G_CALLBACK(gtk_widget_destroy), dialog );
               gtk_widget_show_all(dialog);
               Deconnecter_sale();
             }
            break;
       case SSTAG_SERVEUR_ACCOUNT_EXPIRED:
             { dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                                                 _("Your account has\nexpired") );
               g_signal_connect_swapped( dialog, "response",
                                         G_CALLBACK(gtk_widget_destroy), dialog );
               gtk_widget_show_all(dialog);
               Deconnecter_sale();
             }
            break;
       case SSTAG_SERVEUR_CHANGEPASS:
             { struct CMD_UTIL_SETPASSWORD *util;
               util = (struct CMD_UTIL_SETPASSWORD *)connexion->donnees;
               Client_en_cours.id = util->id;
               if (!Changer_password()) Deconnecter();
             }
            break;

       case SSTAG_SERVEUR_AUTORISE:
             { struct REZO_SRV_IDENT *ident;
               gchar chaine[80];

               ident = (struct REZO_SRV_IDENT *)connexion->donnees;
               g_snprintf( chaine, sizeof(chaine), _("Connected to %s@%s:%d  %s"),
                           Client_en_cours.user, Client_en_cours.serveur, Client_en_cours.config.port, ident->comment );
               gnome_appbar_push( GNOME_APPBAR(Barre_status), chaine );
               Log( _("Connected") );
               Client_en_cours.mode = CONNECTE;
               Info( Client_en_cours.config.log, DEBUG_CONNEXION, "Client en mode CONNECTE" );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
