/**********************************************************************************************************/
/* Client/ajout_camera.c        Addition/Edition d'un camera Watchdog2.0                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 25 jun 2004 15:13:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_camera.c
 * This file is part of <Watchdog>
 *
 * Copyright (C) 2009 - 
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
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
 static GtkWidget *Entry_num;                              /* Numéro du entreeANA en cours d'édition/ajout */
 static GtkWidget *Entry_lib;                                                        /* Libelle du camera */
 static GtkWidget *Entry_location;                                               /* Location de la camera */
 static GtkWidget *Combo_type;                                                             /* Type camera */
 static struct CMD_TYPE_CAMERA Edit_camera;                                  /* Camera en cours d'édition */

/**********************************************************************************************************/
/* CB_ajouter_editer_camera: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_camera ( GtkDialog *dialog, gint reponse, gboolean edition )
  { switch(reponse)
     { case GTK_RESPONSE_OK:
             { if (edition)
                { Edit_camera.type = gtk_combo_box_get_active( GTK_COMBO_BOX(Combo_type) );
                  g_snprintf( Edit_camera.libelle, sizeof(Edit_camera.libelle),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
                  g_snprintf( Edit_camera.location, sizeof(Edit_camera.location),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_location) ) );
                  
                  Envoi_serveur( TAG_CAMERA, SSTAG_CLIENT_VALIDE_EDIT_CAMERA,
                                (gchar *)&Edit_camera, sizeof( struct CMD_TYPE_CAMERA ) );
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
/* Ajouter_camera: Ajoute un camera au systeme                                                      */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_camera ( struct CMD_TYPE_CAMERA *edit_camera )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint cpt;

    if (edit_camera)
     { memcpy( &Edit_camera, edit_camera, sizeof(struct CMD_TYPE_CAMERA) );
                                                                          /* Save pour utilisation future */
     }
    else memset (&Edit_camera, 0, sizeof(struct CMD_TYPE_CAMERA) );             /* Sinon RAZ structure */

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

    table = gtk_table_new( 3, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    texte = gtk_label_new( _("Numero") );              /* Id unique du entreeANA en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    Entry_num = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_num), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_num, 1, 2, 0, 1 );

    texte = gtk_label_new( _("Type") );                                           /* Le type de la camera */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 0, 1 );
    Combo_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_CAMERA; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_camera_vers_string(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_type, 3, 4, 0, 1 );

    texte = gtk_label_new( _("Libelle") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_lib), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, 1, 2 );

    texte = gtk_label_new( _("Location") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 2, 3 );
    Entry_location = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_location), NBR_CARAC_LOCATION_CAMERA );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_location, 1, 4, 2, 3 );

    if (edit_camera)                                                       /* Si edition d'un camera */
     { gchar chaine[32];
       g_snprintf( chaine, sizeof(chaine), "%s%04d", Type_bit_interne_court(MNEMO_CAMERA), edit_camera->num );
       Edit_camera.id_mnemo = edit_camera->id_mnemo;
       gtk_entry_set_text( GTK_ENTRY(Entry_num), chaine);
       gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_camera->libelle );
       gtk_entry_set_text( GTK_ENTRY(Entry_location), edit_camera->location );
       gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), edit_camera->type );
     }
    else gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 0 );

    gtk_widget_grab_focus( Entry_lib );
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
