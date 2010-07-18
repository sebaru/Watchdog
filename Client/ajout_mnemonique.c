/**********************************************************************************************************/
/* Client/ajout_mnemonique.c        Addition/Edition d'un mnemonique Watchdog2.0                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 05 d�c 2004 14:21:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_mnemonique.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - S�bastien Lefevre
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
 
/********************************* D�finitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static GtkWidget *Option_type;         /* Pour le choix d'appartenance du mnemonique � tel ou tel groupe */
 static GtkWidget *Spin_num;
 static GtkWidget *Entry_lib;                                                    /* Libelle du mnemonique */
 static GtkWidget *Entry_acro;                                                  /* Acronyme du mnemonique */
 static GtkWidget *Entry_objet;                                                    /* Objet du mnemonique */
 static struct CMD_TYPE_MNEMONIQUE Edit_mnemo;                              /* Message en cours d'�dition */

/**********************************************************************************************************/
/* CB_ajouter_editer_mnemonique: Fonction appel�e qd on appuie sur un des boutons de l'interface          */
/* Entr�e: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_mnemonique ( GtkDialog *dialog, gint reponse, gboolean edition )
  { g_snprintf( Edit_mnemo.libelle, sizeof(Edit_mnemo.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
    g_snprintf( Edit_mnemo.objet, sizeof(Edit_mnemo.objet),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_objet) ) );
    g_snprintf( Edit_mnemo.acronyme, sizeof(Edit_mnemo.acronyme),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_acro) ) );
    Edit_mnemo.type = gtk_combo_box_get_active( GTK_COMBO_BOX(Option_type) );
    Edit_mnemo.num = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_num) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_MNEMONIQUE, (edition ? SSTAG_CLIENT_VALIDE_EDIT_MNEMONIQUE
                                                       : SSTAG_CLIENT_ADD_MNEMONIQUE),
                              (gchar *)&Edit_mnemo, sizeof( struct CMD_TYPE_MNEMONIQUE ) );
               break;
             }
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* CB_Valider: Simule l'appui sur le bouton OK                                                            */
/* Entr�e: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void CB_valider ( void )
  { gtk_dialog_response( GTK_DIALOG(F_ajout), GTK_RESPONSE_OK ); } 
/**********************************************************************************************************/
/* Set_max_bit: Met a jour la range Spinbutton selon le type de mnemonique                                */
/* Entr�e : Rien                                                                                          */
/* Sortie : Rien                                                                                          */
/**********************************************************************************************************/
 static void Set_max_bit ( void )
  { switch ( gtk_combo_box_get_active ( GTK_COMBO_BOX(Option_type) ) )
     { case MNEMO_BISTABLE:
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, (gdouble) NBR_BIT_BISTABLE );
            break;
       case MNEMO_MONOSTABLE:
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, (gdouble) NBR_BIT_MONOSTABLE );
            break;
       case MNEMO_TEMPO:
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, (gdouble) NBR_TEMPO );
            break;
       case MNEMO_ENTREE:
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, (gdouble) NBR_ENTRE_TOR );
            break;
       case MNEMO_SORTIE:
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, (gdouble) NBR_SORTIE_TOR );
            break;
       case MNEMO_ENTREE_ANA:
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, (gdouble) NBR_ENTRE_ANA );
            break;
       case MNEMO_SORTIE_ANA:
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, (gdouble) NBR_ENTRE_ANA );
            break;
       case MNEMO_MOTIF:
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, (gdouble) NBR_BIT_CONTROLE );
            break;
       case MNEMO_CPTH:
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, (gdouble) NBR_COMPTEUR_H );
            break;
       default: 
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, 1.0 );
            break;
     }
  }
/**********************************************************************************************************/
/* Ajouter_mnemonique: Ajoute un mnemonique au systeme                                                    */
/* Entr�e: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_mnemonique ( struct CMD_TYPE_MNEMONIQUE *edit_mnemo )
  { GtkWidget *frame, *table, *texte, *hboite;
    int cpt;

    if (edit_mnemo)                                                       /* Save pour utilisation future */
     { memcpy( &Edit_mnemo, edit_mnemo, sizeof(struct CMD_TYPE_MNEMONIQUE) );
     }
    else memset (&Edit_mnemo, 0, sizeof(struct CMD_TYPE_MNEMONIQUE) );             /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_mnemo ? _("Edit a mnemonique") : _("Add a mnemonique")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_mnemonique),
                      GINT_TO_POINTER( (edit_mnemo ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Cr�ation de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 4, 4, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    texte = gtk_label_new( _("Type") );    /* Cr�ation de l'option menu pour le choix du type de mnemonique */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    Option_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_MNEMO; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Option_type),Type_bit_interne(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Option_type, 1, 2, 0, 1 );
    g_signal_connect_swapped( G_OBJECT(Option_type), "changed", G_CALLBACK(Set_max_bit), NULL );

    texte = gtk_label_new( _("Num") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 0, 1 );
    Spin_num = gtk_spin_button_new_with_range( 0.0, NBR_BIT_DLS, 1.0 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_num, 3, 4, 0, 1 );

    texte = gtk_label_new( _("Object") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Entry_objet = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_objet), NBR_CARAC_OBJET_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_objet, 1, 4, 1, 2 );

    texte = gtk_label_new( _("Acronyme") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 2, 3 );
    Entry_acro = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_acro), NBR_CARAC_ACRONYME_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_acro, 1, 4, 2, 3 );

    texte = gtk_label_new( _("Description") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 3, 4 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, 3, 4 );

    Set_max_bit();
    g_signal_connect_swapped( Entry_lib, "activate", G_CALLBACK(CB_valider), NULL );
    if (edit_mnemo)                                                          /* Si edition d'un mnemonique */
     { gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_mnemo->libelle );
       gtk_entry_set_text( GTK_ENTRY(Entry_acro), edit_mnemo->acronyme );
       gtk_entry_set_text( GTK_ENTRY(Entry_objet), edit_mnemo->objet );
       gtk_combo_box_set_active( GTK_COMBO_BOX(Option_type), edit_mnemo->type );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_num), (double)edit_mnemo->num );
     }
    gtk_widget_grab_focus( Entry_lib );
    gtk_widget_show_all( F_ajout );
  }

/*--------------------------------------------------------------------------------------------------------*/
