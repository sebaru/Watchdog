/**********************************************************************************************************/
/* Client/ajout_mnemonique.c        Addition/Edition d'un mnemonique Watchdog2.0                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 05 déc 2004 14:21:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_mnemonique.c
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
 static GtkWidget *Option_type;         /* Pour le choix d'appartenance du mnemonique à tel ou tel groupe */
 static GtkWidget *Spin_num;
 static GtkWidget *Entry_lib;                                                    /* Libelle du mnemonique */
 static GtkWidget *Entry_acro;                                                  /* Acronyme du mnemonique */
 static GtkWidget *Entry_command;                                          /* Commande_text du mnemonique */
 static GtkWidget *Entry_tableau;                                 /* Tableau d'affichage pour les courbes */
 static GtkWidget *Combo_dls;                                                       /* Synoptique associé */
 static struct CMD_TYPE_MNEMO_FULL Option_mnemo;                       /* Mnemonique en cours d'édition */
 static GList *Liste_index_dls;

/**********************************************************************************************************/
/* CB_ajouter_editer_mnemonique: Fonction appelée qd on appuie sur un des boutons de l'interface          */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_mnemonique ( GtkDialog *dialog, gint reponse, gboolean edition )
  { gint index;
    g_snprintf( Option_mnemo.mnemo_base.libelle, sizeof(Option_mnemo.mnemo_base.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
    g_snprintf( Option_mnemo.mnemo_base.acronyme, sizeof(Option_mnemo.mnemo_base.acronyme),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_acro) ) );
    g_snprintf( Option_mnemo.mnemo_base.command_text, sizeof(Option_mnemo.mnemo_base.command_text),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_command) ) );
    g_snprintf( Option_mnemo.mnemo_base.tableau, sizeof(Option_mnemo.mnemo_base.tableau),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_tableau) ) );
    index                              = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_dls) );
    Option_mnemo.mnemo_base.num_plugin = GPOINTER_TO_INT(g_list_nth_data( Liste_index_dls, index ) );
    Option_mnemo.mnemo_base.type       = gtk_combo_box_get_active( GTK_COMBO_BOX(Option_type) );
    Option_mnemo.mnemo_base.num        = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_num) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_ADD_MNEMONIQUE,
                              (gchar *)&Option_mnemo.mnemo_base, sizeof( struct CMD_TYPE_MNEMO_BASE ) );
               break;
             }
       case GTK_RESPONSE_APPLY:
             { Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_VALIDE_EDIT_MNEMONIQUE,
                              (gchar *)&Option_mnemo.mnemo_base, sizeof( struct CMD_TYPE_MNEMO_BASE ) );
               break;
             }
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    g_list_free( Liste_index_dls );
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Proto_afficher_un_groupe_existant: ajoute un groupe dans la liste des groupes existants                */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_dls_for_mnemonique ( struct CMD_TYPE_PLUGIN_DLS *dls )
  { gchar chaine[512];
    g_snprintf( chaine, sizeof(chaine), "%s/%s/%s", dls->groupe, dls->page, dls->nom );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_dls), chaine );
    Liste_index_dls = g_list_append( Liste_index_dls, GINT_TO_POINTER(dls->id) );
    if (Option_mnemo.mnemo_base.num_plugin == dls->id)
     { gtk_combo_box_set_active ( GTK_COMBO_BOX (Combo_dls),
                                  g_list_index(Liste_index_dls, GINT_TO_POINTER(dls->id))
                                );
     }
  }
/**********************************************************************************************************/
/* CB_Valider: Simule l'appui sur le bouton OK                                                            */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void CB_valider ( void )
  { gtk_dialog_response( GTK_DIALOG(F_ajout), GTK_RESPONSE_OK ); } 
/**********************************************************************************************************/
/* Set_max_bit: Met a jour la range Spinbutton selon le type de mnemonique                                */
/* Entrée : Rien                                                                                          */
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
       case MNEMO_CPT_IMP:
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, (gdouble) NBR_COMPTEUR_IMP );
            break;
       default: 
            gtk_spin_button_set_range (GTK_SPIN_BUTTON(Spin_num), 0.0, 1.0 );
            break;
     }
  }
/**********************************************************************************************************/
/* Ajouter_mnemonique: Ajoute un mnemonique au systeme                                                    */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_mnemonique ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { GtkWidget *frame, *table, *texte, *hboite;
    int cpt, i;

    if (mnemo_full)                                                     /* Save pour utilisation future */
     { memcpy( &Option_mnemo, mnemo_full, sizeof(struct CMD_TYPE_MNEMO_FULL) );
     }
    else memset (&Option_mnemo, 0, sizeof(struct CMD_TYPE_MNEMO_FULL) );         /* Sinon RAZ structure */

    if (mnemo_full)
     { F_ajout = gtk_dialog_new_with_buttons( _("Edit a mnemonic"),
                                              GTK_WINDOW(F_client),
                                              GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_STOCK_COPY, GTK_RESPONSE_OK,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
                                              NULL);
     }
    else
     { F_ajout = gtk_dialog_new_with_buttons( _("Add a mnemonic"),
                                              GTK_WINDOW(F_client),
                                              GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_ADD, GTK_RESPONSE_OK,
                                              NULL);
     }
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_mnemonique),
                      GINT_TO_POINTER( (mnemo_full ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 5, 4, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    i=0;
    texte = gtk_label_new( _("Type") );  /* Création de l'option menu pour le choix du type de mnemonique */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Option_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_MNEMO; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Option_type),Type_bit_interne(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Option_type, 1, 2, i, i+1 );
    g_signal_connect_swapped( G_OBJECT(Option_type), "changed", G_CALLBACK(Set_max_bit), NULL );

    texte = gtk_label_new( _("Num") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    Spin_num = gtk_spin_button_new_with_range( 0.0, NBR_BIT_DLS, 1.0 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_num, 3, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Groupe/Page/Module DLS") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Combo_dls = gtk_combo_box_new_text();
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_dls, 1, 4, i, i+1 );
    Liste_index_dls = NULL;
    Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_WANT_DLS_FOR_MNEMO, NULL, 0 );

    i++;
    texte = gtk_label_new( _("Acronyme") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_acro = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_acro), NBR_CARAC_ACRONYME_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_acro, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Command text") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_command = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_command), NBR_CARAC_LIBELLE_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_command, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Description") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Tableau") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_tableau = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_tableau), NBR_CARAC_LIBELLE_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_tableau, 1, 4, i, i+1 );

    Set_max_bit();
    g_signal_connect_swapped( Entry_lib, "activate", G_CALLBACK(CB_valider), NULL );
    if (mnemo_full)                                                          /* Si edition d'un mnemonique */
     { gtk_entry_set_text( GTK_ENTRY(Entry_lib),     mnemo_full->mnemo_base.libelle );
       gtk_entry_set_text( GTK_ENTRY(Entry_acro),    mnemo_full->mnemo_base.acronyme );
       gtk_entry_set_text( GTK_ENTRY(Entry_command), mnemo_full->mnemo_base.command_text );
       gtk_entry_set_text( GTK_ENTRY(Entry_tableau), mnemo_full->mnemo_base.tableau );
       gtk_combo_box_set_active( GTK_COMBO_BOX(Option_type), mnemo_full->mnemo_base.type );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_num), (double)mnemo_full->mnemo_base.num );
     }
    gtk_widget_grab_focus( Entry_lib );
    gtk_widget_show_all( F_ajout );
  }
/*--------------------------------------------------------------------------------------------------------*/
