/**********************************************************************************************************/
/* Client/ajout_synoptique.c        Addition/Edition d'un synoptique Watchdog2.0                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 13:54:48 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_synoptique.c
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
 static GtkWidget *Entry_id;                             /* Numéro du synoptique en cours d'édition/ajout */
 static GtkWidget *Entry_lib;                                                    /* Libelle du synoptique */
 static GtkWidget *Entry_page;                                        /* Mnemonique du synoptique */
 static GtkWidget *Entry_groupe;                                                  /* Groupe du synoptique */
 static GtkWidget *Combo_access_groupe;        /* Pour le choix d'appartenance du synoptique à tel ou tel groupe */
 static GList *Liste_index_groupe; /* Pour correspondance index de l'option menu/Id du groupe en question */
 static struct CMD_TYPE_SYNOPTIQUE Edit_syn;                                /* Message en cours d'édition */

/**********************************************************************************************************/
/* CB_ajouter_editer_synoptique: Fonction appelée qd on appuie sur un des boutons de l'interface          */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_synoptique ( GtkDialog *dialog, gint reponse, gboolean edition )
  { gint index_groupe;
    g_snprintf( Edit_syn.libelle, sizeof(Edit_syn.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
    g_snprintf( Edit_syn.page, sizeof(Edit_syn.page),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_page) ) );
    g_snprintf( Edit_syn.groupe, sizeof(Edit_syn.groupe),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_groupe) ) );
    index_groupe = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_access_groupe) );
    if (index_groupe == -1)
     { Edit_syn.access_groupe = 1; }
    else
     { Edit_syn.access_groupe = GPOINTER_TO_INT((g_list_nth( Liste_index_groupe, index_groupe ))->data); }
                  
    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_SYNOPTIQUE, (edition ? SSTAG_CLIENT_VALIDE_EDIT_SYNOPTIQUE
                                                       : SSTAG_CLIENT_ADD_SYNOPTIQUE),
                                (gchar *)&Edit_syn, sizeof( struct CMD_TYPE_SYNOPTIQUE ) );
               break;
             }
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    g_list_free( Liste_index_groupe );
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* CB_Valider: Simule l'appui sur le bouton OK                                                            */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void CB_valider ( void )
  { gtk_dialog_response( GTK_DIALOG(F_ajout), GTK_RESPONSE_OK ); } 
/**********************************************************************************************************/
/* Ajouter_synoptique: Ajoute un synoptique au systeme                                                    */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_synoptique ( struct CMD_TYPE_SYNOPTIQUE *edit_syn )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint i;

    if (edit_syn)
     { memcpy( &Edit_syn, edit_syn, sizeof(struct CMD_TYPE_SYNOPTIQUE) );    /* Save pour utilisation future */
     }
    else memset (&Edit_syn, 0, sizeof(struct CMD_TYPE_SYNOPTIQUE) );                  /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_syn ? _("Edit a synoptique") : _("Add a synoptique")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_synoptique),
                      GINT_TO_POINTER( (edit_syn ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
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

    i=0;
    texte = gtk_label_new( _("SynID") );              /* Id unique du synoptique en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_id = gtk_entry_new();
    gtk_editable_set_editable( GTK_EDITABLE(Entry_id), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_id, 1, 2, i, i+1 );

    texte = gtk_label_new( _("Access Group") ); /* Création de l'option menu pour le choix du type de synoptique */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    Combo_access_groupe = gtk_combo_box_new_text();
    Liste_index_groupe = NULL;
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_access_groupe, 3, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Groupe") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_groupe = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_groupe), NBR_CARAC_LIBELLE_SYNOPTIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_groupe, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Page") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_page = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_page), NBR_CARAC_PAGE_SYNOPTIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_page, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Name") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_SYNOPTIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, i, i+1 );

    g_signal_connect_swapped( Entry_lib, "activate", G_CALLBACK(gtk_widget_grab_focus), Entry_page );
    g_signal_connect_swapped( Entry_lib, "activate", G_CALLBACK(CB_valider), NULL );
    if (edit_syn)                                                              /* Si edition d'un synoptique */
     { gchar chaine[10];
       gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_syn->libelle );
       gtk_entry_set_text( GTK_ENTRY(Entry_page), edit_syn->page );
       gtk_entry_set_text( GTK_ENTRY(Entry_groupe), edit_syn->groupe );
       g_snprintf( chaine, sizeof(chaine), "%d", edit_syn->id );
       gtk_entry_set_text( GTK_ENTRY(Entry_id), chaine );
     }
    else { gtk_entry_set_text( GTK_ENTRY(Entry_id), _("?") );
           gtk_widget_set_sensitive( Entry_id, FALSE );
         }
    gtk_widget_grab_focus( Entry_lib );
    gtk_widget_show_all( F_ajout );
  }
/**********************************************************************************************************/
/* Proto_afficher_un_groupe_existant: ajoute un groupe dans la liste des groupes existants                */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_les_groupes_pour_synoptique ( GList *liste )
  { struct CMD_TYPE_GROUPE *groupe;

printf(" Flag afficher groupe pour syn \n" );

    while( liste )
     { groupe = (struct CMD_TYPE_GROUPE *)liste->data;
       gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_access_groupe), groupe->nom );
       Liste_index_groupe = g_list_append( Liste_index_groupe, GINT_TO_POINTER(groupe->id) );
       liste = liste->next;
     }
  }
/**********************************************************************************************************/
/* Proto_fin_affichage_groupes: Les groupes existants ont été envoyés par le serveur, nous pouvons faire  */
/*                              apparaitre les groupes de l'utilisateur dans l'interface                  */
/* les groupes de l'utilisateur en cours d'edition                                                        */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 void Proto_fin_affichage_groupes_pour_synoptique ( void )
  { GList *liste;
    gint cpt;

    cpt = 0;
printf("fin affichage groupe\n");
    liste = Liste_index_groupe; 
    while (liste)
     { if ( liste->data == GINT_TO_POINTER(Edit_syn.access_groupe) ) break;
       cpt++;
       liste = liste->next;
     }
    if (liste)
     { gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_access_groupe), cpt );
       printf("Set history %d\n", cpt );
     } else printf(" Groupe not %d found\n", Edit_syn.access_groupe );
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
