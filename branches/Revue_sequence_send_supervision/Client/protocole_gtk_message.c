/**********************************************************************************************************/
/* Client/protocole_gtk_message.c    Gestion du protocole_message pour la connexion au serveur Watchdog   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_gtk_message.c
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

 #include "Erreur.h"
 #include "Reseaux.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 static void Fenetre_erreur ( struct CMD_GTK_MESSAGE *erreur, gint type )
  { GtkWidget *dialog;
    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT,
                                      type, GTK_BUTTONS_CLOSE,
                                      erreur->message );

    g_signal_connect_swapped( GTK_OBJECT (dialog), "response",
                              G_CALLBACK (gtk_widget_destroy),
                              GTK_OBJECT (dialog));
    gtk_widget_show_all( dialog );                                           /* Visualisation de l'erreur */
  }
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_gtk_message ( struct CONNEXION *connexion )
  { switch ( Reseau_ss_tag(connexion) )
     { default:
       case SSTAG_SERVEUR_INFO   : Fenetre_erreur( (struct CMD_GTK_MESSAGE *)connexion->donnees,
                                                   GTK_MESSAGE_INFO );
                                   break;
       case SSTAG_SERVEUR_WARNING: Fenetre_erreur( (struct CMD_GTK_MESSAGE *)connexion->donnees,
                                                   GTK_MESSAGE_WARNING );
                                   break;
       case SSTAG_SERVEUR_ERREUR : Fenetre_erreur( (struct CMD_GTK_MESSAGE *)connexion->donnees,
                                                   GTK_MESSAGE_ERROR );
                                   break;
       case SSTAG_SERVEUR_NBR_ENREG:
             { struct CMD_ENREG *enreg;
               enreg = (struct CMD_ENREG *)connexion->donnees;
               Set_progress_text( enreg->comment, enreg->num );
             }
            return;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
