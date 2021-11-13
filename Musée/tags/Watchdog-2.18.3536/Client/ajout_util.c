/**********************************************************************************************************/
/* Client/Ajout_util.c        Addition/Eddition d'un utilisateur Watchdog2.0                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 20:39:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_util.c
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

 enum
  {  COLONNE_ID,
     COLONNE_NOM,
     COLONNE_COMMENTAIRE,
     NBR_COLONNE
  };
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"
 #include "client.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */
 extern struct CLIENT Client;                                    /* Identifiant de l'utilisateur en cours */

 static GtkWidget *Check_expire;                                         /* Le bouton d'expiration ou non */
 static GtkWidget *Calendar;                                 /* Le calendrier pour l'expiration du compte */
 static GtkWidget *Entry_nom;                                                  /* Le nom de l'utilisateur */
 static GtkWidget *Entry_last;                                                   /* L'id de l'utilisateur */
 static GtkWidget *Entry_comment;                                  /* Commentaire associé à l'utilisateur */
 static GtkWidget *Entry_sms_phone;                                   /* n° de telephone de l'utilisateur */
 static GtkWidget *Entry_imsg_jabberid;                 /* n° de messagerie instantannée de l'utilisateur */
 static GtkWidget *Check_enable;                                 /* Le compte utilisateur est-il enable ? */
 static GtkWidget *Check_cansetpwd;                       /* L'utilisateur peut-il changer son password ? */
 static GtkWidget *Check_setpwdnow;                                       /* Pour changer le password now */
 static GtkWidget *Check_mustchangepwd;/* L'utilisateur doit-il changer son password au prochain login ?? */
 static GtkWidget *Check_sms_enable;                                           /* Lui envoit-on des SMS ? */
 static GtkWidget *Check_sms_allow_cde;              /* Peut-on recevoir des sms de commande de sa part ? */
 static GtkWidget *Check_imsg_enable;                                         /* Lui envoit-on des IMSG ? */
 static GtkWidget *Check_imsg_allow_cde;            /* Peut-on recevoir des imsg de commande de sa part ? */
 static GtkWidget *Entry_pass1, *Entry_pass2;                            /* Acquisition des passwords ... */
 static GtkWidget *Spin_ssrv_bit_presence;                                       /* Bxxx de presence SSRV */
 static GtkWidget *Spin_access_level;                                              /* Niveau de clearance */
 static GtkWidget *Check_imsg_enable;                                         /* Lui envoit-on des IMSG ? */
 static GtkWidget *F_ajout;                                /* Widget visuel de la fenetre d'ajout/edition */
 static struct CMD_TYPE_UTILISATEUR Edit_util;                          /* Utilisateur en cours d'edition */
/**********************************************************************************************************/
/* Visu_groupe: Creation d'un GtkTreeView permettant la visu des données des groupes                      */
/* Entrées: kedal                                                                                         */
/* Sortie: un GtkTreeView                                                                                 */
/**********************************************************************************************************/
 static GtkWidget *Visu_groupe ( void )
  { GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkTreeModel *store;
    GtkWidget *vue;

    store = (GtkTreeModel *)gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,
                                                              G_TYPE_STRING,                    /* Nom */
                                                              G_TYPE_STRING             /* Commentaire */
                                               );

    vue = gtk_tree_view_new_with_model ( store );                                   /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(vue) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne de l'id de l'utilisateur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("GroupName"), renderer,
                                                         "text", COLONNE_NOM,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NOM);             /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW(vue), colonne );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne de l'id de l'utilisateur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Comment"), renderer,
                                                         "text", COLONNE_COMMENTAIRE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_COMMENTAIRE);            /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW(vue), colonne );

    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(vue), TRUE );                          /* Pour faire beau */
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */

    return( vue );
  }
/**********************************************************************************************************/
/* Changer_compte_expire: Appelé quand l'utilisateur clique sur le toggle expire                          */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 static void Changer_compte_expire ( void )
  { gtk_widget_set_sensitive( Calendar, GTK_TOGGLE_BUTTON(Check_expire)->active );
  }
/**********************************************************************************************************/
/* Changer_setpwdnow: Appelé quand l'utilisateur clique sur le toggle setpasswordnow                     */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 static void Changer_setpwdnow ( void )
  { gboolean enable;
    enable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_setpwdnow));
    gtk_widget_set_sensitive( Entry_pass1, enable );
    gtk_widget_set_sensitive( Entry_pass2, enable );
  }
/**********************************************************************************************************/
/* Changer_enabled: Appelé quand l'utilisateur clique sur le toggle enabled                               */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 static void Changer_enabled ( void )
  { gboolean enable;
    enable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_enable));
    gtk_widget_set_sensitive( Entry_nom, enable );
    gtk_widget_set_sensitive( Entry_comment, enable );
  }
/**********************************************************************************************************/
/* Changer_sms_enable: Appelé quand l'utilisateur clique sur le toggle sms_enable                         */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 static void Changer_sms_enable ( void )
  { gboolean enable;
    enable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_sms_enable));
    gtk_widget_set_sensitive( Entry_sms_phone, enable );
    gtk_widget_set_sensitive( Check_sms_allow_cde, enable );
  }
/**********************************************************************************************************/
/* Changer_sms_enable: Appelé quand l'utilisateur clique sur le toggle sms_enable                         */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 static void Changer_imsg_enable ( void )
  { gboolean enable;
    enable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_imsg_enable));
    gtk_widget_set_sensitive( Entry_imsg_jabberid, enable );
    gtk_widget_set_sensitive( Check_imsg_allow_cde, enable );
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_utilisateur: Fonction appelée qd on appuie sur un des boutons de l'interface         */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_utilisateur ( GtkDialog *dialog, gint reponse,
                                                 gboolean edition )
  { 
    g_snprintf( Edit_util.nom, sizeof(Edit_util.nom),
                "%s", gtk_entry_get_text(GTK_ENTRY(Entry_nom) ) );
    g_snprintf( Edit_util.commentaire, sizeof(Edit_util.commentaire),
                "%s", gtk_entry_get_text(GTK_ENTRY(Entry_comment) ) );
    g_snprintf( Edit_util.sms_phone, sizeof(Edit_util.sms_phone),
                "%s", gtk_entry_get_text(GTK_ENTRY(Entry_sms_phone) ) );
    g_snprintf( Edit_util.imsg_jabberid, sizeof(Edit_util.imsg_jabberid),
                "%s", gtk_entry_get_text(GTK_ENTRY(Entry_imsg_jabberid) ) );

    Edit_util.date_expire       = gnome_date_edit_get_time( GNOME_DATE_EDIT(Calendar) );
    Edit_util.expire            = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_expire));
    Edit_util.mustchangepwd     = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_mustchangepwd));
    Edit_util.enable            = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_enable));
    Edit_util.cansetpwd         = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_cansetpwd));
    Edit_util.sms_enable        = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_sms_enable));
    Edit_util.sms_allow_cde     = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_sms_allow_cde));
    Edit_util.imsg_enable       = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_imsg_enable));
    Edit_util.imsg_allow_cde    = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_imsg_allow_cde));
    Edit_util.ssrv_bit_presence = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_ssrv_bit_presence) );
    Edit_util.access_level      = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_access_level) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_setpwdnow)))
             { if (g_utf8_collate( gtk_entry_get_text(GTK_ENTRY(Entry_pass1) ),
                                   gtk_entry_get_text(GTK_ENTRY(Entry_pass2) )
                                 )
                  )
                { GtkWidget *dialog;
                  dialog = gtk_message_dialog_new ( GTK_WINDOW(F_ajout),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                    GTK_MESSAGE_ERROR,
                                                    GTK_BUTTONS_CLOSE, _("Passwords do not match") );
                  g_signal_connect_swapped( dialog, "response",
                                            G_CALLBACK(gtk_widget_destroy), dialog );
                  return(TRUE);
                }
               g_snprintf ( Edit_util.hash, sizeof(Edit_util.hash), "%s", /* le champ Hash contient le code en clair ! */
                            gtk_entry_get_text(GTK_ENTRY(Entry_pass1) ) );
               Envoi_serveur( TAG_UTILISATEUR, SSTAG_CLIENT_CHANGE_PASSWORD,
                              (gchar *)&Client.util, sizeof(struct CMD_TYPE_UTILISATEUR) );
             }
           Envoi_serveur( TAG_UTILISATEUR, (edition ? SSTAG_CLIENT_VALIDE_EDIT_UTIL
                                                    : SSTAG_CLIENT_ADD_UTIL),
                              (gchar *)&Edit_util, sizeof(struct CMD_TYPE_UTILISATEUR) );
           break;
      case GTK_RESPONSE_CANCEL :
       default                 : break;
     }

    gtk_widget_destroy( F_ajout );                                           /* Destruction de la fenetre */
    return(TRUE);                                      /* Pour eviter la propagation du signal 'response' */
  }
/**********************************************************************************************************/
/* Menu_ajouter_editer_utilisateur: Met a jour ou ajout un utilisateur au systeme                         */
/* Entrée: l'id et le nom de l'utilisateur                                                                */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_utilisateur ( struct CMD_TYPE_UTILISATEUR *edit_util )
  { GtkWidget *bouton, *frame, *vboite, *table, *texte, *scroll, *separateur;
    time_t temps;
    guint i;

    if (edit_util)
     { memcpy( &Edit_util, edit_util, sizeof(struct CMD_TYPE_UTILISATEUR) );/*Save pour utilisation future*/
     }
    else memset (&Edit_util, 0, sizeof(struct CMD_TYPE_UTILISATEUR) );             /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_util ? _("Edit a user") : _("Add a user")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_utilisateur),
                      GINT_TO_POINTER( (edit_util ? TRUE : FALSE) ) );

    frame = gtk_frame_new( _("Settings") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );

/******************************************** Paramètres de l'utilisateur *********************************/
    table = gtk_table_new( 7, 4, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, FALSE, FALSE, 0 );

    i=0;
    Check_enable = gtk_check_button_new_with_label ( _("Account enabled") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_enable, 0, 1, i, i+1 );
    g_signal_connect( G_OBJECT(Check_enable), "clicked",
                      G_CALLBACK(Changer_enabled), NULL );
    Entry_nom = gtk_entry_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_nom, 1, 2, i, i+1 );
    gtk_entry_set_max_length( GTK_ENTRY(Entry_nom), NBR_CARAC_LOGIN );

    Entry_comment = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_comment), NBR_CARAC_COMMENTAIRE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_comment, 2, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Last change") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    gtk_misc_set_alignment( GTK_MISC(texte), 1.0, 0.5 );
    Entry_last = gtk_entry_new();
    gtk_editable_set_editable( GTK_EDITABLE(Entry_last), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_last, 1, 4, i, i+1 );


    i++;
    Check_cansetpwd = gtk_check_button_new_with_label ( _("Can change his password") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_cansetpwd, 2, 4, i, i+1 );

    Check_mustchangepwd = gtk_check_button_new_with_label ( _("Have to change his password at logon") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mustchangepwd, 0, 2, i, i+1 );

    i++;
    Check_setpwdnow = gtk_check_button_new_with_label ( _("Set password now") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_setpwdnow, 0, 1, i, i+1 );
    g_signal_connect( G_OBJECT(Check_setpwdnow), "clicked",
                      G_CALLBACK(Changer_setpwdnow), NULL );

    Entry_pass1 = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_pass1), NBR_CARAC_LOGIN );
    gtk_entry_set_visibility( GTK_ENTRY(Entry_pass1), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_pass1, 1, 2, i, i+1 );
    gtk_widget_set_sensitive( Entry_pass1, FALSE );

    texte = gtk_label_new( _("Password again") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    gtk_misc_set_alignment( GTK_MISC(texte), 1.0, 0.5 );
    Entry_pass2 = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_pass2), NBR_CARAC_LOGIN );
    gtk_entry_set_visibility( GTK_ENTRY(Entry_pass2), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_pass2, 3, 4, i, i+1 );
    gtk_widget_set_sensitive( Entry_pass2, FALSE );

    i++;
    Check_expire = gtk_check_button_new_with_label ( _("Account expire") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_expire, 0, 1, i, i+1 );
    g_signal_connect( G_OBJECT(Check_expire), "clicked",
                      G_CALLBACK(Changer_compte_expire), NULL );

    Calendar = gnome_date_edit_new ((time_t) 0, TRUE, TRUE);
    gtk_table_attach_defaults( GTK_TABLE(table), Calendar, 1, 4, i, i+1 );

    i++;
    Check_sms_enable = gtk_check_button_new_with_label ( _("Send SMS to") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_sms_enable, 0, 1, i, i+1 );
    g_signal_connect( G_OBJECT(Check_sms_enable), "clicked",
                      G_CALLBACK(Changer_sms_enable), NULL );
    Entry_sms_phone = gtk_entry_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_sms_phone, 1, 3, i, i+1 );
    gtk_entry_set_max_length( GTK_ENTRY(Entry_sms_phone), 80 );

    Check_sms_allow_cde = gtk_check_button_new_with_label ( _("Allow CDE") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_sms_allow_cde, 3, 4, i, i+1 );

    i++;
    Check_imsg_enable = gtk_check_button_new_with_label ( _("Send IMSG to") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_imsg_enable, 0, 1, i, i+1 );
    g_signal_connect( G_OBJECT(Check_imsg_enable), "clicked",
                      G_CALLBACK(Changer_imsg_enable), NULL );
    Entry_imsg_jabberid = gtk_entry_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_imsg_jabberid, 1, 3, i, i+1 );
    gtk_entry_set_max_length( GTK_ENTRY(Entry_imsg_jabberid), 80 );

    Check_imsg_allow_cde = gtk_check_button_new_with_label ( _("Allow CDE") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_imsg_allow_cde, 3, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("SSRV_bit_presence") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_ssrv_bit_presence = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_ssrv_bit_presence, 1, 2, i, i+1 );;

    texte = gtk_label_new( _("Access Level") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    Spin_access_level = gtk_spin_button_new_with_range( 0, 10, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_access_level, 3, 4, i, i+1 );

    if (edit_util)                                                           /* Est-on en mode edition ?? */
     { gchar chaine[80], *date;
       gtk_entry_set_text( GTK_ENTRY(Entry_nom), edit_util->nom );
       gtk_editable_set_editable( GTK_EDITABLE( Entry_nom ), FALSE );

       g_snprintf( chaine, sizeof(chaine), "%d", edit_util->id );
       gtk_entry_set_text( GTK_ENTRY(Entry_comment), edit_util->commentaire );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_access_level), edit_util->access_level );
       gtk_entry_set_text( GTK_ENTRY(Entry_sms_phone), edit_util->sms_phone );
       gtk_entry_set_text( GTK_ENTRY(Entry_imsg_jabberid), edit_util->imsg_jabberid );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_enable),        edit_util->enable );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mustchangepwd), edit_util->mustchangepwd );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_cansetpwd),     edit_util->cansetpwd );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_expire),        edit_util->expire );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_sms_enable),    edit_util->sms_enable );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_sms_allow_cde), edit_util->sms_allow_cde );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_imsg_enable),   edit_util->imsg_enable );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_imsg_allow_cde),edit_util->imsg_allow_cde );

       temps = edit_util->date_modif;
       strftime( chaine, sizeof(chaine), "%c", localtime((time_t *)&temps) );
       date = g_locale_to_utf8( chaine, -1, NULL, NULL, 0 ); 
       gtk_entry_set_text( GTK_ENTRY(Entry_last), date );
       g_free(date);
       gnome_date_edit_set_time( GNOME_DATE_EDIT(Calendar), edit_util->date_expire );
     }
    else
     { gtk_entry_set_text( GTK_ENTRY(Entry_last), "?" );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( Check_enable ), TRUE );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( Check_mustchangepwd ), TRUE );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( Check_cansetpwd ), TRUE );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( Check_expire ), TRUE );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( Check_setpwdnow ), TRUE );
       gnome_date_edit_set_time( GNOME_DATE_EDIT(Calendar), time(NULL) + 86400*31*2 );  /* Valable 2 mois */
     }
    Changer_compte_expire();
    Changer_setpwdnow();
    Changer_enabled();
    Changer_sms_enable ();
    Changer_imsg_enable ();
    gtk_widget_show_all( F_ajout );
  }
/*--------------------------------------------------------------------------------------------------------*/
