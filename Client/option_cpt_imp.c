/**********************************************************************************************************/
/* Client/option_cpt_imp.c        Gestion des options des compteurs d'impulsions Watchdog2.0              */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 15 déc. 2010 11:38:05 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * option_cpt_imp.c
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
 static GtkWidget *Entry_num;                             /* Numéro du cpt_imp en cours d'édition/ajout */
 static GtkWidget *Option_unite;                                   /* Unite correspondante à l'entrée ana */
 static struct CMD_TYPE_OPTION_BIT_INTERNE Cpt;                                  /* EA en cours d'édition */

/**********************************************************************************************************/
/* CB_ajouter_editer_cpt_imp: Fonction appelée qd on appuie sur un des boutons de l'interface             */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_editer_option_cpt_imp ( GtkDialog *dialog, gint reponse, gboolean edition )
  { Cpt.eana.unite = gtk_combo_box_get_active( GTK_COMBO_BOX(Option_unite) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_VALIDE_EDIT_OPTION_BIT_INTERNE,
                              (gchar *)&Cpt, sizeof( struct CMD_TYPE_OPTION_BIT_INTERNE ) );
               break;
             }
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_cpt_imp: Ajoute un cpt_imp au systeme                                                          */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_option_cpt_imp ( struct CMD_TYPE_OPTION_BIT_INTERNE *edit_cpt_imp )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint cpt;

    memcpy( &Cpt, edit_cpt_imp, sizeof(struct CMD_TYPE_OPTION_COMPTEUR_IMP) );

    F_ajout = gtk_dialog_new_with_buttons( _("Option of cpt_imp"),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_editer_option_cpt_imp),
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

    texte = gtk_label_new( _("CI") );                    /* Id unique du cpt_imp en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    Entry_num = gtk_entry_new ();
    gtk_entry_set_editable( GTK_ENTRY(Entry_num), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_num, 1, 2, 0, 1 );

    texte = gtk_label_new( _("Unit") );                                              /* Unite du compteur */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 4, 5 );
    Option_unite = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_UNITE; cpt++ )
     { gtk_combo_box_insert_text( GTK_COMBO_BOX(Option_unite), cpt, Unite_vers_string(cpt) );
     }
    gtk_table_attach_defaults( GTK_TABLE(table), Option_unite, 1, 2, 4, 5 );

    if (edit_cpt_imp)                                                          /* Si edition d'un cpt_imp */
     { gchar chaine[32];
       Cpt.cpt_imp.id_mnemo = edit_cpt_imp->eana.id_mnemo;
       g_snprintf( chaine, sizeof(chaine), "%s%04d", Type_bit_interne_court(MNEMO_CPT_IMP), edit_cpt_imp->cpt_imp.num );
       gtk_entry_set_text( GTK_ENTRY(Entry_num), chaine );
       gtk_combo_box_set_active( GTK_COMBO_BOX(Option_unite), edit_cpt_imp->eana.unite );
     }
    else { gtk_widget_grab_focus( Entry_num );
         }
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
