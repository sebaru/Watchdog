/******************************************************************************************************************************/
/* Client/ajout_message.c        Addition/Edition d'un message Watchdog2.0                                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           sam 20 nov 2004 13:47:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ajout_message.c
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
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/wait.h>
 #include <fcntl.h>

 #include "Reseaux.h"
/********************************************** Définitions des prototypes programme *****************************************/
 #include "protocli.h"
 #include "client.h"
 #include "Config_cli.h"

 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                                                      /* Configuration generale watchdog */
 extern struct CLIENT Client;                                                        /* Identifiant de l'utilisateur en cours */

 static GtkWidget *F_ajout;                                                                /* Widget de l'interface graphique */
 static GtkWidget *Spin_num;                                                    /* Numéro du message en cours d'édition/ajout */
 static GtkWidget *Entry_lib;                                                                           /* Libelle du message */
 static GtkWidget *Check_persist;                                      /* Le message est-il persistent sur front descendant ? */
 static GtkWidget *Check_audio;                                                                 /* Le message est-il vocal ?? */
 static GtkWidget *Entry_lib_audio;                                                               /* Libelle audio du message */
 static GtkWidget *Entry_lib_sms;                                                                   /* Libelle sms du message */
 static GtkWidget *Entry_mp3;                                                                           /* nom de fichier mp3 */
 static GtkWidget *Combo_type;                                                                      /* Type actuel du message */
 static GtkWidget *Combo_dls;                                                                           /* Synoptique associé */
 static GtkWidget *Check_enable;                                                      /* Le message est-il actif ou inhibe ?? */
 static GtkWidget *Combo_sms;                                                     /* Le message doit-il etre envoyé par sms ? */
 static GtkWidget *Spin_bit_audio;                                                                     /* Numéro du bit vocal */
 static GtkWidget *Entry_bit_audio;                                                  /* Mnémonique correspondant au bit vocal */
 static GtkWidget *Spin_time_repeat;                                                              /* Intervalle de repetition */
 static GList *Liste_index_dls;
 static struct CMD_TYPE_MESSAGE Msg;                                                            /* Message en cours d'édition */

/******************************************************************************************************************************/
/* Valider_fichier_mp3: Confirmation et envoi du fichier mp3 au serveur                                                       */
/* Entrée: la page du notebook en cours d'edition                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Valider_fichier_mp3 ( struct CMD_TYPE_MESSAGE *msg, gchar *fichier )
  { struct CMD_TYPE_MESSAGE_MP3 *msg_mp3;
    gint taille, taille_max, id_source;
    gchar *buffer_envoi;

    id_source = open ( fichier, O_RDONLY, 0 );
    if (id_source<0) return;

    msg_mp3 = (struct CMD_TYPE_MESSAGE_MP3 *)g_try_malloc0( Client.connexion->taille_bloc );
    if (!msg_mp3) return;
    buffer_envoi     = (gchar *)msg_mp3 + sizeof(struct CMD_TYPE_MESSAGE_MP3);
    taille_max       = Client.connexion->taille_bloc - sizeof(struct CMD_TYPE_MESSAGE_MP3);
    msg_mp3->num     = msg->num;
    msg_mp3->taille  = 0;
                                                          /* Demande de suppression du fichier source MP3 */
    Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_VALIDE_EDIT_MP3_DEB,
                   (gchar *)msg_mp3, sizeof(struct CMD_TYPE_MESSAGE_MP3) );

    while( (taille = read ( id_source, buffer_envoi, taille_max ) ) > 0 )             /* Envoi du fichier */
     { msg_mp3->taille = taille;
       if (!Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_VALIDE_EDIT_MP3,
                           (gchar *)msg_mp3, taille + sizeof(struct CMD_TYPE_MESSAGE_MP3) ))
        { printf("erreur envoi au serveur\n"); }
       printf("Octets envoyés: %d\n", taille);
     }

    Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_VALIDE_EDIT_MP3_FIN,                     /* Fin du transfert */
                   (gchar *)msg_mp3, sizeof(struct CMD_TYPE_MESSAGE_MP3) );
    close(id_source);
    g_free(msg_mp3);
  }
/******************************************************************************************************************************/
/* CB_ajouter_editer_message: Fonction appelée qd on appuie sur un des boutons de l'interface                                 */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                                                   */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_ajouter_editer_message ( GtkDialog *dialog, gint reponse, gboolean edition )
  { gint index;

    g_snprintf( Msg.libelle, sizeof(Msg.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
    g_snprintf( Msg.libelle_audio, sizeof(Msg.libelle_audio),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib_audio) ) );
    g_snprintf( Msg.libelle_sms, sizeof(Msg.libelle_sms),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib_sms) ) );

    Msg.type       = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_type) );
    Msg.enable     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_enable) );
    Msg.audio      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_audio) );
    Msg.persist    = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_persist) );
    Msg.sms        = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_sms) );
    Msg.num        = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_num) );
    index               = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_dls) );
    Msg.dls_id   = GPOINTER_TO_INT(g_list_nth_data( Liste_index_dls, index ) );
    if (Msg.dls_id == 0) Msg.dls_id = 1;                                           /* Par défaut, pointe sur le premier D.L.S */
    Msg.bit_audio  = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bit_audio) );
    Msg.time_repeat= gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_time_repeat) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_MESSAGE, (edition ? SSTAG_CLIENT_VALIDE_EDIT_MESSAGE
                                                    : SSTAG_CLIENT_ADD_MESSAGE),
                              (gchar *)&Msg, sizeof( struct CMD_TYPE_MESSAGE ) );
               Valider_fichier_mp3 ( &Msg,
                                     gnome_file_entry_get_full_path ( GNOME_FILE_ENTRY(Entry_mp3), TRUE )
                                   );
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    g_list_free( Liste_index_dls );
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Proto_afficher_un_groupe_existant: ajoute un groupe dans la liste des groupes existants                                    */
/* Entrée: rien                                                                                                               */
/* sortie: kedal                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_dls_for_message ( struct CMD_TYPE_PLUGIN_DLS *dls )
  { gchar chaine[512];
    g_snprintf( chaine, sizeof(chaine), "%s/%s/%s", dls->syn_groupe, dls->syn_page, dls->shortname );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_dls), chaine );
    Liste_index_dls = g_list_append( Liste_index_dls, GINT_TO_POINTER(dls->id) );
    if (Msg.dls_id == dls->id)
     { gtk_combo_box_set_active ( GTK_COMBO_BOX (Combo_dls),
                                  g_list_index(Liste_index_dls, GINT_TO_POINTER(dls->id))
                                );
     }
    else if (Msg.dls_id == 0)
     { gtk_combo_box_set_active ( GTK_COMBO_BOX (Combo_dls), 0 );
     }
  }
/******************************************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                                      */
/* Entre: widget, data.                                                                                                       */
/* Sortie: void                                                                                                               */
/******************************************************************************************************************************/
 void Proto_afficher_mnemo_voc_message ( struct CMD_TYPE_MNEMO_BASE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );                                 /* Formatage */
    gtk_entry_set_text( GTK_ENTRY(Entry_bit_audio), chaine );
  }
/******************************************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                                      */
/* Entre: widget, data.                                                                                                       */
/* Sortie: void                                                                                                               */
/******************************************************************************************************************************/
 static void Afficher_mnemo_voc ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    mnemo.type = MNEMO_MONOSTABLE;
    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_bit_audio) );
    
    Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_TYPE_NUM_MNEMO_VOC,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Rafraichir_sensibilite: met a jour la sensibilite des widgets de la fenetre propriete                  */
/* Entrée: void                                                                                           */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Rafraichir_sensibilite_msg ( void )
  { gboolean enable, audio;
    enable = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_enable) );
    audio  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_audio) );
    gtk_widget_set_sensitive( Check_persist, enable );
    gtk_widget_set_sensitive( Spin_num, enable );
    gtk_widget_set_sensitive( Spin_time_repeat, enable );
    gtk_widget_set_sensitive( Entry_lib, enable );
    gtk_widget_set_sensitive( Entry_lib_sms, enable );
    gtk_widget_set_sensitive( Combo_type, enable );
    gtk_widget_set_sensitive( Combo_dls, enable );
    gtk_widget_set_sensitive( Combo_sms, enable );
    gtk_widget_set_sensitive( Check_audio, enable );
    gtk_widget_set_sensitive( Entry_lib_audio, enable & audio);
    gtk_widget_set_sensitive( Entry_mp3, enable & audio);
    gtk_widget_set_sensitive( Spin_bit_audio, enable & audio);
    gtk_widget_set_sensitive( Entry_bit_audio, enable & audio);
  }
/******************************************************************************************************************************/
/* Menu_ajouter_editer_message: Ajoute ou edite un message au systeme                                                         */
/* Entrée: La structure du message a editer ou a creer                                                                        */
/* sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Menu_ajouter_editer_message ( struct CMD_TYPE_MESSAGE *edit_msg )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint cpt, i;

    if (edit_msg)
     { memcpy( &Msg, edit_msg, sizeof(struct CMD_TYPE_MESSAGE) );                             /* Save pour utilisation future */
     }
    else memset (&Msg, 0, sizeof(struct CMD_TYPE_MESSAGE) );                                           /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_msg ? _("Edit a message") : _("Add a message")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_message),
                      GINT_TO_POINTER( (edit_msg ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                                                   /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 8, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    i = 0;
    Check_enable = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_enable, 0, 1, i, i+1 );
    g_signal_connect( G_OBJECT( GTK_CHECK_BUTTON(Check_enable) ), "clicked",
                      G_CALLBACK( Rafraichir_sensibilite_msg ), NULL );
    Check_persist = gtk_check_button_new_with_label( _("Persistent") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_persist, 1, 2, i, i+1 );
    texte = gtk_label_new( _("Type") );     /* Création de l'option menu pour le choix du type de message */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );

    Combo_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_MSG; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_vers_string(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_type, 3, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("MsgID") );                 /* Id unique du message en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_num = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_num, 1, 2, i, i+1 );

    texte = gtk_label_new( _("Repeat (min)") );                               /* Répétition du message ?? */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    Spin_time_repeat = gtk_spin_button_new_with_range( 0, 60, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_time_repeat, 3, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Message") );                                      /* Le message en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Groupe/Page/DLS") );                                           /* Choix du DLS cible du message */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Combo_dls = gtk_combo_box_new_text();
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_dls, 1, 4, i, i+1 );
    Liste_index_dls = NULL;
    Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_WANT_DLS_FOR_MESSAGE, NULL, 0 );

/******************************************************** Paragraphe SMS ******************************************************/
    i++;
    Combo_sms = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_MSG_SMS; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_sms), Type_sms_vers_string(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_sms, 0, 1, i, i+1 );

    Entry_lib_sms = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib_sms), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib_sms, 1, 4, i, i+1 );

/******************************************************** Paragraphe Voix *****************************************************/
    i++;
    Check_audio = gtk_check_button_new_with_label( _("Message Vocal") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_audio, 0, 1, i, i+1 );
    g_signal_connect( G_OBJECT( GTK_CHECK_BUTTON(Check_audio) ), "clicked",
                      G_CALLBACK( Rafraichir_sensibilite_msg ), NULL );

    Spin_bit_audio = gtk_spin_button_new_with_range( 30, 59, 1 );                                     /* Range M0030 -> M0059 */
    g_signal_connect( G_OBJECT(Spin_bit_audio), "changed",
                      G_CALLBACK(Afficher_mnemo_voc), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bit_audio, 1, 2, i, i+1 );;

    Entry_bit_audio = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bit_audio), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bit_audio, 2, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Message Audio") );                                                    /* Le message en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_lib_audio = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib_audio), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib_audio, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Mp3 upload") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );

    Entry_mp3 = gnome_file_entry_new("Mp3Filename", _("Select a file for mp3") );
    gnome_file_entry_set_modal( GNOME_FILE_ENTRY(Entry_mp3), TRUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_mp3, 1, 4, i, i+1 );

    if (edit_msg)                                                                                  /* Si edition d'un message */
     { gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_msg->libelle );
       gtk_entry_set_text( GTK_ENTRY(Entry_lib_audio), edit_msg->libelle_audio );
       gtk_entry_set_text( GTK_ENTRY(Entry_lib_sms), edit_msg->libelle_sms );
       gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_type), edit_msg->type );
       gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_sms), edit_msg->sms );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_enable), edit_msg->enable );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_persist), edit_msg->persist );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_audio), edit_msg->audio );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_num), edit_msg->num );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bit_audio), edit_msg->bit_audio );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_time_repeat), edit_msg->time_repeat );
       gtk_widget_grab_focus( Entry_lib );
     }
    else { gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_enable), TRUE );
           gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_audio), FALSE );
           gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_persist), FALSE );
           gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_type), 0 );
           gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_sms), 0 );
           gtk_widget_grab_focus( Spin_num );
         }
    Rafraichir_sensibilite_msg();
    gtk_widget_show_all(F_ajout);                                                        /* Affichage de l'interface complète */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
