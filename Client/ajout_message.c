/**********************************************************************************************************/
/* Client/ajout_message.c        Addition/Edition d'un message Watchdog2.0                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 20 nov 2004 13:47:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_message.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
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
 static GtkWidget *Spin_num;                                /* Numéro du message en cours d'édition/ajout */
 static GtkWidget *Entry_lib;                                                       /* Libelle du message */
 static GtkWidget *Entry_objet;                                                       /* Objet du message */
 static GtkWidget *Combo_type;                                                  /* Type actuel du message */
 static GtkWidget *Combo_syn;                                                       /* Synoptique associé */
 static GtkWidget *Check_not_inhibe;                              /* Le message est-il actif ou inhibe ?? */
 static GtkWidget *Check_sms;                                 /* Le message doit-il etre envoyé par sms ? */
 static GtkWidget *Spin_bit_voc;                                                   /* Numéro du bit vocal */
 static GtkWidget *Entry_bit_voc;                                /* Mnémonique correspondant au bit vocal */
 static GList *Liste_index_syn;
 static struct CMD_EDIT_MESSAGE Edit_msg;                                   /* Message en cours d'édition */

/**********************************************************************************************************/
/* CB_ajouter_editer_message: Fonction appelée qd on appuie sur un des boutons de l'interface             */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_message ( GtkDialog *dialog, gint reponse, gboolean edition )
  { gint index;
    switch(reponse)
     { case GTK_RESPONSE_OK:
             { if (edition)
                { g_snprintf( Edit_msg.libelle, sizeof(Edit_msg.libelle),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
                  g_snprintf( Edit_msg.objet, sizeof(Edit_msg.objet),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_objet) ) );
                  Edit_msg.type       = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_type) );
                  Edit_msg.not_inhibe = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_not_inhibe) );
                  Edit_msg.sms        = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_sms) );
                  Edit_msg.num        = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_num) );
                  index               = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_syn) );
                  Edit_msg.num_syn    = GPOINTER_TO_INT(g_list_nth_data( Liste_index_syn, index ) );
                  Edit_msg.num_voc    = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bit_voc) );
                  Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_VALIDE_EDIT_MESSAGE,
                                (gchar *)&Edit_msg, sizeof( struct CMD_EDIT_MESSAGE ) );
                }
               else
                { struct CMD_ADD_MESSAGE new_msg;
                  g_snprintf( new_msg.libelle, sizeof(new_msg.libelle),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
                  g_snprintf( new_msg.objet, sizeof(new_msg.objet),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_objet) ) );
                  new_msg.type       = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_type) );
                  new_msg.not_inhibe = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_not_inhibe) );
                  new_msg.sms        = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_sms) );
                  new_msg.num        = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_num) );
                  index              = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_syn) );
                  new_msg.num_syn    = GPOINTER_TO_INT(g_list_nth_data( Liste_index_syn, index ) );
                  new_msg.num_voc    = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bit_voc) );

                  Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_ADD_MESSAGE,
                                (gchar *)&new_msg, sizeof( struct CMD_EDIT_MESSAGE ) );
                }
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    g_list_free( Liste_index_syn );
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Proto_afficher_un_groupe_existant: ajoute un groupe dans la liste des groupes existants                */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_syn_for_message ( struct CMD_SHOW_SYNOPTIQUE *syn )
  { gchar chaine[256];
    g_snprintf( chaine, sizeof(chaine), "%s/%s", syn->mnemo, syn->libelle );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_syn), chaine );
    Liste_index_syn = g_list_append( Liste_index_syn, GINT_TO_POINTER(syn->id) );
    if (Edit_msg.num_syn == syn->id)
     { gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_syn),
                                 g_list_index(Liste_index_syn, GINT_TO_POINTER(syn->id))
                                );
     }
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Proto_afficher_mnemo_voc_message ( struct CMD_TYPE_MNEMONIQUE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );             /* Formatage */
    gtk_entry_set_text( GTK_ENTRY(Entry_bit_voc), chaine );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Afficher_mnemo_voc ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    mnemo.type = MNEMO_MONOSTABLE;
    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_bit_voc) );
    
    Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_TYPE_NUM_MNEMO_VOC,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Ajouter_message: Ajoute un message au systeme                                                          */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_message ( struct CMD_EDIT_MESSAGE *edit_msg )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint cpt;

    if (edit_msg)
     { memcpy( &Edit_msg, edit_msg, sizeof(struct CMD_EDIT_MESSAGE) );    /* Save pour utilisation future */
     }
    else memset (&Edit_msg, 0, sizeof(struct CMD_EDIT_MESSAGE) );                  /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_msg ? _("Edit a message") : _("Add a message")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_message),
                      GINT_TO_POINTER( (edit_msg ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 6, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    Check_not_inhibe = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_not_inhibe, 0, 1, 0, 1 );

    Check_sms = gtk_check_button_new_with_label( _("Sms") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_sms, 1, 2, 0, 1 );

    texte = gtk_label_new( _("MsgID") );                 /* Id unique du message en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Spin_num = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_num, 1, 2, 1, 2 );

    texte = gtk_label_new( _("Type") );     /* Création de l'option menu pour le choix du type de message */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 0, 1 );

    Combo_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_MSG; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_vers_string(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_type, 3, 4, 0, 1 );

    texte = gtk_label_new( _("Object") );                                       /* Le message en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 2, 3 );
    Entry_objet = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_objet), NBR_CARAC_OBJET_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_objet, 1, 4, 2, 3 );

    texte = gtk_label_new( _("Profil Audio") );                          /* Numéro du bit M a positionner */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 3, 4 );
    Spin_bit_voc = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    g_signal_connect( G_OBJECT(Spin_bit_voc), "changed",
                      G_CALLBACK(Afficher_mnemo_voc), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bit_voc, 1, 2, 3, 4 );

    Entry_bit_voc = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bit_voc), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bit_voc, 2, 4, 3, 4 );

    texte = gtk_label_new( _("Synopt.") );                       /* Choix du synoptique cible du messages */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 4, 5 );
    Combo_syn = gtk_combo_box_new_text();
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_syn, 1, 4, 4, 5 );
    Liste_index_syn = NULL;
    Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_WANT_SYN_FOR_MESSAGE, NULL, 0 );

    texte = gtk_label_new( _("Message") );                                      /* Le message en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 5, 6 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, 5, 6 );

    if (edit_msg)                                                              /* Si edition d'un message */
     { /*syn = Rechercher_syn_par_num( Nom_syn, msg->num_synoptique );*/
       Edit_msg.id = edit_msg->id;
       Edit_msg.num_syn = edit_msg->num_syn;
       Edit_msg.num_voc = edit_msg->num_voc;
       gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_msg->libelle );
       gtk_entry_set_text( GTK_ENTRY(Entry_objet), edit_msg->objet );
       gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_type), edit_msg->type );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_not_inhibe), edit_msg->not_inhibe );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_sms), edit_msg->sms );

       /*gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(Combo_syn)->entry), syn->libelle );*/
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_num), edit_msg->num );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bit_voc), edit_msg->num_voc );
       gtk_widget_grab_focus( Entry_lib );
       /*for (cpt=0; cpt<MAX_HP; cpt++)
        { gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(Check_hp[cpt]), (msg.hps & (1<<cpt)) ); }*/
     }
    else { gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_not_inhibe), TRUE );
           gtk_widget_grab_focus( Spin_num );
         }
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
