/**********************************************************************************************************/
/* Client/option_tempo.c        Addition/Edition d'un tempo Watchdog2.0                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 13 mars 2013 18:41:47 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * option_tempo.c
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

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static GtkWidget *Entry_num;                                 /* Numéro du tempo en cours d'édition/ajout */
 static GtkWidget *Entry_lib;                                                         /* Libelle du tempo */
 static GtkWidget *Spin_delai_on;                                   /* Delai avant activation de la tempo */
 static GtkWidget *Spin_delai_off;                               /* Delai avant desactivation de la tempo */
 static GtkWidget *Spin_min_on;                                            /* Duree minimale d'activation */
 static GtkWidget *Spin_max_on;                          /* Duree maximale de l'activation (0 = illimité) */
 static struct CMD_TYPE_OPTION_BIT_INTERNE Tempo;                                /* TR en cours d'édition */

/**********************************************************************************************************/
/* CB_ajouter_editer_tempo: Fonction appelée qd on appuie sur un des boutons de l'interface               */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_editer_option_tempo ( GtkDialog *dialog, gint reponse, gboolean edition )
  { Tempo.type = MNEMO_TEMPO;
    Tempo.tempo.min_on    = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_min_on   ) );
    Tempo.tempo.max_on    = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_max_on   ) );
    Tempo.tempo.delai_on  = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_delai_on ) );
    Tempo.tempo.delai_off = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_delai_off) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_VALIDE_EDIT_OPTION_BIT_INTERNE,
                              (gchar *)&Tempo, sizeof( struct CMD_TYPE_OPTION_BIT_INTERNE ) );
               break;
             }
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Proto_editer_option_tempo : Permet l'edition des options d'une temporisation                           */
/* Entrée: la Temporisation a éditer                                                                      */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_option_tempo ( struct CMD_TYPE_OPTION_BIT_INTERNE *edit_tempo )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint i;

    memcpy( &Tempo, edit_tempo, sizeof(struct CMD_TYPE_OPTION_ENTREEANA) );

    F_ajout = gtk_dialog_new_with_buttons( _("Option of tempo"),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_editer_option_tempo),
                      GINT_TO_POINTER( TRUE ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
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

    i=0;
    texte = gtk_label_new( _("T") );                       /* Id unique du tempo en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_num = gtk_entry_new ();
    gtk_entry_set_editable( GTK_ENTRY(Entry_num), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_num, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Delai avant activation") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_delai_on = gtk_spin_button_new_with_range( 0, +100000000, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_delai_on, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Duree minimale d'activation") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_min_on = gtk_spin_button_new_with_range( 0, +100000000, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_min_on, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Duree maximale d'activation") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_max_on = gtk_spin_button_new_with_range( 0, +100000000, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_max_on, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Delai avant desactivation") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_delai_off = gtk_spin_button_new_with_range( 0, +100000000, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_delai_off, 1, 2, i, i+1 );

    texte = gtk_label_new( _("Description") );                                    /* Le tempo en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 5, 6 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_lib), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 2, 5, 6 );

    if (edit_tempo)                                                              /* Si edition d'un tempo */
     { gchar chaine[32];
       gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_tempo->tempo.libelle );
       g_snprintf( chaine, sizeof(chaine), "%04d", edit_tempo->tempo.num );
       gtk_entry_set_text( GTK_ENTRY(Entry_num), chaine );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_min_on),    edit_tempo->tempo.min_on );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_max_on),    edit_tempo->tempo.max_on );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_delai_on),  edit_tempo->tempo.delai_on );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_delai_off), edit_tempo->tempo.delai_off );
       gtk_widget_grab_focus( Entry_lib );
     }
    else { gtk_widget_grab_focus( Entry_num );
         }
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
