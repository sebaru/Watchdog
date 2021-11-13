/******************************************************************************************************************************/
/* Client/ajout_plugin_dls.c        Configuration des plugin_dlss de Watchdog v2.0                                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         sam. 31 déc. 2011 17:34:23 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
 
 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG Config;                                                              /* Configuration generale watchdog */
/**************************************** Définitions des prototypes programme ************************************************/
 #include "protocli.h"

 static GtkWidget *F_ajout;                                   /* Widget de reference sur la fenetre d'ajout/edition du plugin */
 static GtkWidget *Entry_nom;                                                 /* Le nom en clair du plugin en cours d'edition */
 static GtkWidget *Entry_shortname;                                           /* Le nom en clair du plugin en cours d'edition */
 static GtkWidget *Entry_id;                                                        /* Le numéro de plugin en cours d'edition */
 static GtkWidget *Combo_syn;                                                                           /* Synoptique associé */
 static GtkWidget *Combo_type;                                                      /* Type du plugin (module, ssgrpupe, ...) */
 static GtkWidget *Check_actif;                                                      /* Le plugin est-il activé dans le dls ? */
 static struct CMD_TYPE_PLUGIN_DLS Edit_dls;                                                     /* Plugin en cours d'édition */
 static GList *Liste_index_syn;

/******************************************************************************************************************************/
/* Type_vers_string: renvoie le type string associé                                                                           */
/* Entrée: le type numérique                                                                                                  */
/* Sortie: la chaine de caractère                                                                                             */
/******************************************************************************************************************************/
 static gchar *Type_plugin_vers_string ( guint32 type )
  { switch (type)
     { case PLUGIN_MODULE   : return( _("Module") );
       case PLUGIN_SSGROUPE : return( _("Page") );
       case PLUGIN_GROUPE   : return( _("Groupe") );
       case PLUGIN_TOPLEVEL : return( _("Top Level") );
     }
    return( _("Unknown") );
  }
/******************************************************************************************************************************/
/* CB_ajouter_editer_groupe: Fonction appelée qd on appuie sur un des boutons de l'interface                                  */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                                                   */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_ajouter_editer_plugin_dls ( GtkDialog *dialog, gint reponse,
                                                gboolean edition )
  { gint index;

    g_snprintf( Edit_dls.nom, sizeof(Edit_dls.nom), "%s", (gchar *)gtk_entry_get_text( GTK_ENTRY(Entry_nom) ) );
    g_snprintf( Edit_dls.shortname, sizeof(Edit_dls.shortname), "%s", (gchar *)gtk_entry_get_text( GTK_ENTRY(Entry_shortname) ) );
    index             = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_syn) );
    Edit_dls.syn_id   = GPOINTER_TO_INT(g_list_nth_data( Liste_index_syn, index ) );
    Edit_dls.on       = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_actif) );
    Edit_dls.type     = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_type) );

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
    g_list_free( Liste_index_syn );
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Proto_afficher_un_groupe_existant: ajoute un groupe dans la liste des groupes existants                                    */
/* Entrée: rien                                                                                                               */
/* sortie: kedal                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_syn_for_plugin_dls ( struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar chaine[512];
    g_snprintf( chaine, sizeof(chaine), "%s/%s/%s", syn->parent_page, syn->page, syn->libelle );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_syn), chaine );
    Liste_index_syn = g_list_append( Liste_index_syn, GINT_TO_POINTER(syn->id) );
    if (Edit_dls.syn_id == syn->id)
     { gtk_combo_box_set_active ( GTK_COMBO_BOX (Combo_syn),
                                  g_list_index(Liste_index_syn, GINT_TO_POINTER(syn->id))
                                );
     }
  }
/******************************************************************************************************************************/
/* Menu_ajouter_plugin_dls: Ajoute un plugin_dls au systeme                                                                   */
/* Entrée: la structure d'edition du plugin                                                                                   */
/* sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Menu_ajouter_editer_plugin_dls ( struct CMD_TYPE_PLUGIN_DLS *edit_dls )
  { GtkWidget *frame, *vboite, *table, *texte;
    gint cpt, i;

    if (edit_dls)
     { memcpy( &Edit_dls, edit_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );                     /* Save pour utilisation future */
     }
    else memset (&Edit_dls, 0, sizeof(struct CMD_TYPE_PLUGIN_DLS) );                                   /* Sinon RAZ structure */

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

/************************************************** Paramètres du plugin_dls **************************************************/
    table = gtk_table_new( 5, 3, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, FALSE, FALSE, 0 );

    i = 0;
    texte = gtk_label_new( _("Plugin ID") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_id = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_id), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_id, 1, 2, i, i+1 );

    Check_actif = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_actif, 2, 3, i, i+1 );

    i++;
    texte = gtk_label_new( _("Type") );                         /* Création de l'option menu pour le choix du type de message */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );

    Combo_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_PLUGIN; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_plugin_vers_string(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_type, 1, 3, i, i+1 );

    i++;
    texte = gtk_label_new( _("Synoptique Name") );                                   /* Choix du synoptique cible du messages */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Combo_syn = gtk_combo_box_new_text();
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_syn, 1, 3, i, i+1 );
    Liste_index_syn = NULL;
    Envoi_serveur( TAG_DLS, SSTAG_CLIENT_WANT_SYN_FOR_PLUGIN_DLS, NULL, 0 );

    i++;
    texte = gtk_label_new( _("D.L.S Name") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_nom = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_nom), NBR_CARAC_PLUGIN_DLS );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_nom, 1, 3, i, i+1 );

    i++;
    texte = gtk_label_new( _("Shortname") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_shortname = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_shortname), NBR_CARAC_PLUGIN_DLS );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_shortname, 1, 3, i, i+1 );

    if (edit_dls)                                                     /* Si edition d'un message on pre-positionne les champs */
     { gchar id[32];
       g_snprintf( id, sizeof(id), "%04d", edit_dls->id );
       gtk_entry_set_text( GTK_ENTRY(Entry_id), id );
       gtk_entry_set_text( GTK_ENTRY(Entry_nom), edit_dls->nom );
       gtk_entry_set_text( GTK_ENTRY(Entry_shortname), edit_dls->shortname );
       gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_type), edit_dls->type );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), edit_dls->on );
     }
    else { gtk_entry_set_text( GTK_ENTRY(Entry_id), "new" );
           gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_type), 0 );
           gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), FALSE );
         }
    gtk_widget_grab_focus( Entry_nom );
    gtk_widget_show_all( F_ajout );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
