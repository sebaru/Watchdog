/**********************************************************************************************************/
/* Client/ajout_groupe.c        Configuration des groupes de Watchdog v2.0                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 12:43:26 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_groupe.c
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

 #include <gnome.h>
 
 #include "Reseaux.h"
 
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static GtkWidget *F_ajout;             /* Widget de reference sur la fenetre d'ajout/edition des groupes */
 static GtkWidget *Entry_nom;                             /* Le nom en clair du groupe en cours d'edition */
 static GtkWidget *Entry_comment;                                   /* Le commentaire associé à ce groupe */
 static struct CMD_TYPE_GROUPE Groupe;                                       /* Groupe en cours d'edition */
/**********************************************************************************************************/
/* CB_ajouter_editer_groupe: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_groupe ( GtkDialog *dialog, gint reponse,
                                            gboolean edition )
  { g_snprintf( Groupe.nom, sizeof(Groupe.nom),
                "%s", (gchar *)gtk_entry_get_text( GTK_ENTRY(Entry_nom) ) );
    g_snprintf( Groupe.commentaire, sizeof(Groupe.commentaire),
                "%s", (gchar *)gtk_entry_get_text( GTK_ENTRY(Entry_comment) ) );
    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_UTILISATEUR, (edition ? SSTAG_CLIENT_VALIDE_EDIT_GROUPE
                                                        : SSTAG_CLIENT_ADD_GROUPE),
                                 (gchar *)&Groupe, sizeof(struct CMD_TYPE_GROUPE) );
               break;
             }
       case GTK_RESPONSE_CANCEL:
       default:                  break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Menu_ajouter_groupe: Ajoute un groupe au systeme                                                       */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_groupe ( struct CMD_TYPE_GROUPE *edit_groupe )
  { GtkWidget *frame, *vboite, *table, *texte;

    if (edit_groupe)
     { memcpy( &Groupe, edit_groupe, sizeof(struct CMD_TYPE_GROUPE) ); /*Save pour utilisation future*/
     }
    else memset (&Groupe, 0, sizeof(struct CMD_TYPE_GROUPE) );                /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_groupe ? _("Edit a group") : _("Add a group")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_groupe),
                      GINT_TO_POINTER( (edit_groupe ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );

/******************************************** Paramètres du groupe ****************************************/
    table = gtk_table_new( 2, 3, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, FALSE, FALSE, 0 );

    texte = gtk_label_new( _("Groupname") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    Entry_nom = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_nom), NBR_CARAC_LOGIN );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_nom, 1, 3, 0, 1 );

    texte = gtk_label_new( _("Comment") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Entry_comment = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_comment), NBR_CARAC_COMMENTAIRE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_comment, 1, 3, 1, 2 );

    gtk_widget_show_all( F_ajout );

    if (edit_groupe)
     { gtk_entry_set_text( GTK_ENTRY(Entry_nom), edit_groupe->nom );
       gtk_editable_set_editable( GTK_EDITABLE(Entry_nom), FALSE );
       gtk_entry_set_text( GTK_ENTRY(Entry_comment), edit_groupe->commentaire );
       gtk_widget_grab_focus( Entry_comment );
     }
    else gtk_widget_grab_focus( Entry_nom );
  }
/*--------------------------------------------------------------------------------------------------------*/
