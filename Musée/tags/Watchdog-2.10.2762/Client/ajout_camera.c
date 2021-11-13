/**********************************************************************************************************/
/* Client/ajout_camera.c        Addition/Edition d'un camera Watchdog2.0                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 25 jun 2004 15:13:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_camera.c
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

 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static GtkWidget *Spin_bit_motion;              /* Numéro du bit bistable pour la detection de mouvement */
 static GtkWidget *Entry_bit_motion;                                          /* mnemonique du bit motion */
 static GtkWidget *Spin_num;                                                       /* Numéro de la camera */
 static GtkWidget *Entry_lib;                                                        /* Libelle du camera */
 static GtkWidget *Entry_location;                                               /* Location de la camera */
 static GtkWidget *Entry_objet;                                                     /* Objet de la camera */
 static GtkWidget *Combo_type;                                                             /* Type camera */
 static struct CMD_TYPE_CAMERA Camera;                                       /* Camera en cours d'édition */

/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Afficher_mnemo ( GtkWidget *widget )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    gint sstag;
    mnemo.num  = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(widget) );
    if ( widget == Spin_bit_motion )
     { mnemo.type = MNEMO_MONOSTABLE;
       sstag      = SSTAG_CLIENT_TYPE_NUM_MNEMO_MOTION;
     }
    else return;
    
    Envoi_serveur( TAG_CAMERA, sstag, (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Proto_afficher_mnemo_camera ( int tag, struct CMD_TYPE_MNEMO_BASE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );             /* Formatage */
    switch (tag)
     { case SSTAG_SERVEUR_TYPE_NUM_MNEMO_MOTION:
            gtk_entry_set_text( GTK_ENTRY(Entry_bit_motion), chaine );
            break;
     }
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_camera: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_camera ( GtkDialog *dialog, gint reponse, gboolean edition )
  { g_snprintf( Camera.libelle, sizeof(Camera.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
    g_snprintf( Camera.location, sizeof(Camera.location),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_location) ) );
    g_snprintf( Camera.objet, sizeof(Camera.objet),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_objet) ) );
    Camera.type = gtk_combo_box_get_active( GTK_COMBO_BOX(Combo_type) );
    Camera.bit = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bit_motion) );
    Camera.num = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_num) );
                  
    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_CAMERA, (edition ? SSTAG_CLIENT_VALIDE_EDIT_CAMERA
                                                   : SSTAG_CLIENT_ADD_CAMERA),
                              (gchar *)&Camera, sizeof( struct CMD_TYPE_CAMERA ) );
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_camera: Ajoute un camera au systeme                                                      */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_camera ( struct CMD_TYPE_CAMERA *edit_camera )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint cpt, ligne;

    if (edit_camera)
     { memcpy( &Camera, edit_camera, sizeof(struct CMD_TYPE_CAMERA) );
                                                                          /* Save pour utilisation future */
     }
    else memset (&Camera, 0, sizeof(struct CMD_TYPE_CAMERA) );             /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_camera ? _("Edit a camera") : _("Add a camera")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_camera),
                      GINT_TO_POINTER( (edit_camera ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 5, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    ligne = 0;
    texte = gtk_label_new( _("Numero") );              /* Id unique du entreeANA en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, ligne, ligne+1 );
    Spin_num = gtk_spin_button_new_with_range( 0, NBR_CAMERA, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_num, 1, 2, ligne, ligne+1 );

    texte = gtk_label_new( _("Type") );                                           /* Le type de la camera */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, ligne, ligne+1 );
    Combo_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_CAMERA; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_camera_vers_string(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_type, 3, 4, ligne, ligne+1 );

    ligne++;
    texte = gtk_label_new( _("Objet") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, ligne, ligne+1 );
    Entry_objet = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_objet), 128/*NBR_CARAC_OBJET_MNEMONIQUE*/ );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_objet, 1, 4, ligne, ligne+1 );

    ligne++;
    texte = gtk_label_new( _("Libelle") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, ligne, ligne+1 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, ligne, ligne+1 );

    ligne++;
    texte = gtk_label_new( _("Location") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, ligne, ligne+1 );
    Entry_location = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_location), NBR_CARAC_LOCATION_CAMERA );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_location, 1, 4, ligne, ligne+1 );

    ligne++;
    texte = gtk_label_new( _("Motion bit") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, ligne, ligne+1 );
    Spin_bit_motion = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    g_signal_connect( G_OBJECT(Spin_bit_motion), "changed",
                      G_CALLBACK(Afficher_mnemo), Spin_bit_motion );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bit_motion, 1, 2, ligne, ligne+1 );
    Entry_bit_motion = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bit_motion), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bit_motion, 2, 4, ligne, ligne+1 );

    if (edit_camera)                                                            /* Si edition d'un camera */
     { gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_camera->libelle );
       gtk_entry_set_text( GTK_ENTRY(Entry_objet), edit_camera->objet );
       gtk_entry_set_text( GTK_ENTRY(Entry_location), edit_camera->location );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_num), edit_camera->num );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bit_motion), edit_camera->bit );
       gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), edit_camera->type );
     }
    else gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 0 );

    Afficher_mnemo(Spin_bit_motion);                /* Demande l'affichage du mnemo associé au bit motion */

    gtk_widget_grab_focus( Entry_lib );
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
