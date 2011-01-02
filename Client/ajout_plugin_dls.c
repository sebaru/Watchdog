/**********************************************************************************************************/
/* Client/ajout_plugin_dls.c        Configuration des plugin_dlss de Watchdog v2.0                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 05 avr 2009 20:49:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_plugin_dls.c
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
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static GtkWidget *F_ajout;               /* Widget de reference sur la fenetre d'ajout/edition du plugin */
 static GtkWidget *Entry_nom;                             /* Le nom en clair du plugin en cours d'edition */
 static GtkWidget *Combo_type;                                  /* Type du plugin (module, ssgrpupe, ...) */
 static GtkWidget *Check_actif;                                  /* Le plugin est-il activé dans le dls ? */
 static struct CMD_TYPE_PLUGIN_DLS Edit_dls;                                /* Message en cours d'édition */

/**********************************************************************************************************/
/* Type_vers_string: renvoie le type string associé                                                       */
/* Entrée: le type numérique                                                                              */
/* Sortie: la chaine de caractère                                                                         */
/**********************************************************************************************************/
 static gchar *Type_plugin_vers_string ( guint32 type )
  { switch (type)
     { case PLUGIN_MODULE   : return( _("Module") );
       case PLUGIN_SSGROUPE : return( _("Sous-Groupe") );
       case PLUGIN_GROUPE   : return( _("Groupe") );
       case PLUGIN_TOPLEVEL : return( _("Top Level") );
     }
    return( _("Unknown") );
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_groupe: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_plugin_dls ( GtkDialog *dialog, gint reponse,
                                                gboolean edition )
  { g_snprintf( Edit_dls.nom, sizeof(Edit_dls.nom),
                "%s", (gchar *)gtk_entry_get_text( GTK_ENTRY(Entry_nom) ) );
    Edit_dls.on   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_actif) );
    Edit_dls.type = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_type) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_DLS, (edition ? SSTAG_CLIENT_VALIDE_EDIT_PLUGIN_DLS
                                                : SSTAG_CLIENT_ADD_PLUGIN_DLS),
                                (gchar *)&Edit_dls, sizeof( struct CMD_TYPE_PLUGIN_DLS ) );
               break;
             }
       case GTK_RESPONSE_CANCEL:
       default:                  break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Menu_ajouter_plugin_dls: Ajoute un plugin_dls au systeme                                               */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_plugin_dls ( struct CMD_TYPE_PLUGIN_DLS *edit_dls )
  { GtkWidget *frame, *vboite, *table, *texte;
    gint cpt;

    if (edit_dls)
     { memcpy( &Edit_dls, edit_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) ); /* Save pour utilisation future */
     }
    else memset (&Edit_dls, 0, sizeof(struct CMD_TYPE_PLUGIN_DLS) );               /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_dls ? _("Edit a plugin") : _("Add a plugin")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_plugin_dls),
                      GINT_TO_POINTER( (edit_dls ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );

/******************************************** Paramètres du plugin_dls ************************************/
    table = gtk_table_new( 2, 3, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, FALSE, FALSE, 0 );

    texte = gtk_label_new( _("Plugin name") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    Entry_nom = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_nom), NBR_CARAC_PLUGIN_DLS );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_nom, 1, 3, 0, 1 );

    Check_actif = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_actif, 0, 1, 1, 2 );

    texte = gtk_label_new( _("Type") );     /* Création de l'option menu pour le choix du type de message */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 1, 2, 1, 2 );

    Combo_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_PLUGIN; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_plugin_vers_string(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_type, 2, 3, 1, 2 );


    if (edit_dls)                                                              /* Si edition d'un message */
     { gtk_entry_set_text( GTK_ENTRY(Entry_nom), edit_dls->nom );
       gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_type), edit_dls->type );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), edit_dls->on );
     }
    else { gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_type), 0 );
           gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), FALSE );
         }
    gtk_widget_grab_focus( Entry_nom );
    gtk_widget_show_all( F_ajout );
  }
/*--------------------------------------------------------------------------------------------------------*/
