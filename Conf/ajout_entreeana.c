/**********************************************************************************************************/
/* Client/ajout_entreeANA.c        Addition/Edition d'un entreeANA Watchdog2.0                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 25 jun 2004 15:13:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_entreeana.c
 * This file is part of <Watchdog>
 *
 * Copyright (C) 2009 - 
 *
 * <Watchdog> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <Watchdog> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <Watchdog>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

 #include <gnome.h>
 
 #include "Reseaux.h"

/********************************* D�finitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static GtkWidget *Entry_num;                              /* Num�ro du entreeANA en cours d'�dition/ajout */
 static GtkWidget *Entry_lib;                                                     /* Libelle du entreeANA */
 static GtkWidget *Spin_min;                              /* Num�ro du entreeANA en cours d'�dition/ajout */
 static GtkWidget *Spin_max;                              /* Num�ro du entreeANA en cours d'�dition/ajout */
 static GtkWidget *Option_unite;                                   /* Unite correspondante � l'entr�e ana */
 static GtkWidget *Option_type;                                                /* Type de gestion de l'ea */
 static struct CMD_TYPE_ENTREEANA Edit_entree;                                   /* EA en cours d'�dition */

/**********************************************************************************************************/
/* CB_ajouter_editer_entreeANA: Fonction appel�e qd on appuie sur un des boutons de l'interface           */
/* Entr�e: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_entreeANA ( GtkDialog *dialog, gint reponse, gboolean edition )
  { switch(reponse)
     { case GTK_RESPONSE_OK:
             { if (edition)
                { Edit_entree.min = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_min) );
                  Edit_entree.max = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_max) );
                  Edit_entree.type  = gtk_combo_box_get_active( GTK_COMBO_BOX(Option_type) );
                  Edit_entree.unite = gtk_combo_box_get_active( GTK_COMBO_BOX(Option_unite) );

                  Envoi_serveur( TAG_ENTREEANA, SSTAG_CLIENT_VALIDE_EDIT_ENTREEANA,
                                (gchar *)&Edit_entree, sizeof( struct CMD_TYPE_ENTREEANA ) );
                }
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_entreeANA: Ajoute un entreeANA au systeme                                                      */
/* Entr�e: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_entreeANA ( struct CMD_TYPE_ENTREEANA *edit_entree )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint cpt;

    if (edit_entree)
     { memcpy( &Edit_entree, edit_entree, sizeof(struct CMD_TYPE_ENTREEANA) );
                                                                          /* Save pour utilisation future */
     }
    else memset (&Edit_entree, 0, sizeof(struct CMD_TYPE_ENTREEANA) );             /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_entree ? _("Edit a entreeANA") : _("Add a entreeANA")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_entreeANA),
                      GINT_TO_POINTER( (edit_entree ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Cr�ation de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 6, 2, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    texte = gtk_label_new( _("EA") );                 /* Id unique du entreeANA en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    Entry_num = gtk_entry_new ();
    gtk_entry_set_editable( GTK_ENTRY(Entry_num), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_num, 1, 2, 0, 1 );

    texte = gtk_label_new( _("Type") );                                        /* Type de gestion de l'EA */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Option_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_ENTREEANA; cpt++ )
     { gtk_combo_box_insert_text( GTK_COMBO_BOX(Option_type), cpt, Type_ea_vers_string(cpt) );
     }
    gtk_table_attach_defaults( GTK_TABLE(table), Option_type, 1, 2, 1, 2 );

    texte = gtk_label_new( _("min") );                /* Id unique du entreeANA en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 2, 3 );
    Spin_min = gtk_spin_button_new_with_range( -1000, +1000, 0.5 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_min, 1, 2, 2, 3 );

    texte = gtk_label_new( _("max") );                /* Id unique du entreeANA en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 3, 4 );
    Spin_max = gtk_spin_button_new_with_range( -1000, +1000, 0.5 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_max, 1, 2, 3, 4 );

    texte = gtk_label_new( _("Unit") );               /* Id unique du entreeANA en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 4, 5 );
    Option_unite = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_UNITE; cpt++ )
     { gtk_combo_box_insert_text( GTK_COMBO_BOX(Option_unite), cpt, Unite_vers_string(cpt) );
     }
    gtk_table_attach_defaults( GTK_TABLE(table), Option_unite, 1, 2, 4, 5 );

    texte = gtk_label_new( _("Description") );                               /* Le entreeANA en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 5, 6 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_lib), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 2, 5, 6 );

    if (edit_entree)                                                       /* Si edition d'un entreeANA */
     { gchar chaine[32];
       Edit_entree.id_mnemo = edit_entree->id_mnemo;
       gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_entree->libelle );
       g_snprintf( chaine, sizeof(chaine), "%s%04d", Type_bit_interne_court(MNEMO_ENTREE_ANA), edit_entree->num );
       gtk_entry_set_text( GTK_ENTRY(Entry_num), chaine );
       gtk_combo_box_set_active( GTK_COMBO_BOX(Option_unite), edit_entree->unite );
       gtk_combo_box_set_active( GTK_COMBO_BOX(Option_type), edit_entree->type );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_min), edit_entree->min );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_max), edit_entree->max );
       gtk_widget_grab_focus( Entry_lib );
     }
    else { gtk_widget_grab_focus( Entry_num );
         }
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface compl�te */
  }
/*--------------------------------------------------------------------------------------------------------*/
