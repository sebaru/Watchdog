/**********************************************************************************************************/
/* Client/ajout_message.c        Addition/Edition d'un message Watchdog2.0                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 20 nov 2004 13:47:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static GtkWidget *Spin_num;                                /* Numéro du message en cours d'édition/ajout */
 static GtkWidget *Entry_lib;                                                       /* Libelle du message */
 static GtkWidget *Entry_lib_audio;                                           /* Libelle audio du message */
 static GtkWidget *Entry_lib_sms;                                               /* Libelle sms du message */
 static GtkWidget *Entry_objet;                                                       /* Objet du message */
 static GtkWidget *Combo_type;                                                  /* Type actuel du message */
 static GtkWidget *Combo_syn;                                                       /* Synoptique associé */
 static GtkWidget *Check_enable;                                  /* Le message est-il actif ou inhibe ?? */
 static GtkWidget *Combo_sms;                                 /* Le message doit-il etre envoyé par sms ? */
 static GtkWidget *Spin_bit_voc;                                                   /* Numéro du bit vocal */
 static GtkWidget *Entry_bit_voc;                                /* Mnémonique correspondant au bit vocal */
 static GtkWidget *Spin_vitesse_voc;                                 /* Vitesse de restitution de la voix */
 static GtkWidget *Spin_time_repeat;                                          /* Intervalle de repetition */
 static GtkWidget *Combo_type_voc;                               /* Type actuel de la voix de restitution */
 static GList *Liste_index_syn;
 static struct CMD_TYPE_MESSAGE Msg;                                        /* Message en cours d'édition */

/**********************************************************************************************************/
/* Jouer_message_audio : Joue le message audio en paramètre                                               */
/* Entrée : un texte à jouer                                                                              */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Jouer_message_audio ( const char *texte, gint vitesse, gint voix )
  { gchar nom_fichier[128], cible[128];
    gint fd_cible, pid;

    g_snprintf( nom_fichier, sizeof(nom_fichier), "test.pho" );
    g_snprintf( cible,       sizeof(cible),       "test.au" );
    unlink( nom_fichier );
    unlink( cible );

/***************************************** Création du PHO ************************************************/
    printf("AUDIO : Lancement de ESPEAK %s\n", texte );
    pid = fork();
    if (pid<0) return;
    else if (!pid)                                                 /* Création du .au en passant par .pho */
     { gchar chaine[30], chaine2[30];
       switch (voix)
        { case 0: g_snprintf( chaine, sizeof(chaine), "mb/mb-fr1" ); break;
          case 1: g_snprintf( chaine, sizeof(chaine), "mb/mb-fr4" ); break;
          case 2: g_snprintf( chaine, sizeof(chaine), "mb/mb-fr1" ); break;
          default:
          case 3: g_snprintf( chaine, sizeof(chaine), "mb/mb-fr4" ); break;
        }
       g_snprintf( chaine2, sizeof(chaine2), "%d", vitesse );
       fd_cible = open ( nom_fichier, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
       dup2( fd_cible, 1 );
       execlp( "espeak", "espeak", "-s", chaine2, "-v", chaine, texte, NULL );
       printf("AUDIO: Lancement espeak failed\n");
       _exit(0);
     }
    printf("AUDIO: waiting for espeak to finish pid\n");
    wait4(pid, NULL, 0, NULL );
    printf("AUDIO: espeak finished pid\n");

/****************************************** Création du AU ************************************************/
    printf("AUDIO : Lancement de MBROLA\n" );
    pid = fork();
    if (pid<0) return;
    else if (!pid)                                                 /* Création du .au en passant par .pho */
     { gchar chaine[30];
       switch (voix)
        { case 0: g_snprintf( chaine, sizeof(chaine), "fr1" ); break;
          case 1: g_snprintf( chaine, sizeof(chaine), "fr2" ); break;
          case 2: g_snprintf( chaine, sizeof(chaine), "fr6" ); break;
          default:
          case 3: g_snprintf( chaine, sizeof(chaine), "fr4" ); break;
        }
       execlp( "mbrola-linux-i386", "mbrola-linux-i386", chaine, nom_fichier, cible, NULL );
       printf("AUDIO: Lancement mbrola failed\n");
       _exit(0);
     }
    printf("AUDIO: waiting for mbrola to finish pid\n");
    wait4(pid, NULL, 0, NULL );
    printf("AUDIO: mbrola finished pid\n");

/****************************************** Lancement de l'audio ******************************************/
    printf("AUDIO : Lancement de APLAY\n");
    pid = fork();
    if (pid<0) return;
    else if (!pid)
     { execlp( "aplay", "aplay", "-R", "1", cible, NULL );
       printf("AUDIO: Lancement APLAY failed\n");
       _exit(0);
     }
/*    printf("AUDIO: waiting for APLAY to finish pid\n");
    wait4(pid, NULL, 0, NULL );
    printf("AUDIO: APLAY finished pid\n");*/
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_message: Fonction appelée qd on appuie sur un des boutons de l'interface             */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_message ( GtkDialog *dialog, gint reponse, gboolean edition )
  { gint index;

    g_snprintf( Msg.libelle, sizeof(Msg.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
    g_snprintf( Msg.libelle_audio, sizeof(Msg.libelle_audio),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib_audio) ) );
    g_snprintf( Msg.libelle_sms, sizeof(Msg.libelle_sms),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib_sms) ) );
    g_snprintf( Msg.objet, sizeof(Msg.objet),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_objet) ) );
    Msg.type       = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_type) );
    Msg.enable     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_enable) );
    Msg.sms        = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_sms) );
    Msg.num        = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_num) );
    index               = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_syn) );
    Msg.num_syn    = GPOINTER_TO_INT(g_list_nth_data( Liste_index_syn, index ) );
    Msg.bit_voc    = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bit_voc) );
    Msg.vitesse_voc= gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_vitesse_voc) );
    Msg.type_voc   = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_type_voc) );
    Msg.time_repeat= gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_time_repeat) );

    switch(reponse)
     { case GTK_RESPONSE_APPLY:
             { Jouer_message_audio ( gtk_entry_get_text( GTK_ENTRY(Entry_lib_audio) ),
                                     gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_vitesse_voc) ),
                                     gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_type_voc) )
                                   );
               return(TRUE);
             }
       case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_MESSAGE, (edition ? SSTAG_CLIENT_VALIDE_EDIT_MESSAGE
                                                    : SSTAG_CLIENT_ADD_MESSAGE),
                              (gchar *)&Msg, sizeof( struct CMD_TYPE_MESSAGE ) );
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
 void Proto_afficher_un_syn_for_message ( struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar chaine[256];
    g_snprintf( chaine, sizeof(chaine), "%s/%s", syn->mnemo, syn->libelle );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_syn), chaine );
    Liste_index_syn = g_list_append( Liste_index_syn, GINT_TO_POINTER(syn->id) );
    if (Msg.num_syn == syn->id)
     { gtk_combo_box_set_active ( GTK_COMBO_BOX (Combo_syn),
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
 void Menu_ajouter_editer_message ( struct CMD_TYPE_MESSAGE *edit_msg )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint cpt;

    if (edit_msg)
     { memcpy( &Msg, edit_msg, sizeof(struct CMD_TYPE_MESSAGE) );    /* Save pour utilisation future */
     }
    else memset (&Msg, 0, sizeof(struct CMD_TYPE_MESSAGE) );                  /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_msg ? _("Edit a message") : _("Add a message")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_MEDIA_PLAY, GTK_RESPONSE_APPLY,
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

    table = gtk_table_new( 9, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    Check_enable = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_enable, 0, 1, 0, 1 );

    Combo_sms = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_MSG_SMS; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_sms), Type_sms_vers_string(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_sms, 1, 2, 0, 1 );

    texte = gtk_label_new( _("MsgID") );                 /* Id unique du message en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Spin_num = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_num, 1, 2, 1, 2 );

    texte = gtk_label_new( _("Repeat") );                                     /* Répétition du message ?? */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 1, 2 );
    Spin_time_repeat = gtk_spin_button_new_with_range( 0, 60, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_time_repeat, 3, 4, 1, 2 );

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

    texte = gtk_label_new( _("Message") );                                      /* Le message en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 3, 4 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, 3, 4 );

    texte = gtk_label_new( _("Synopt.") );                       /* Choix du synoptique cible du messages */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 4, 5 );
    Combo_syn = gtk_combo_box_new_text();
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_syn, 1, 4, 4, 5 );
    Liste_index_syn = NULL;
    Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_WANT_SYN_FOR_MESSAGE, NULL, 0 );

/****************************************** Paragraphe Voix ***********************************************/
    texte = gtk_label_new( _("Profil Audio") );                          /* Numéro du bit M a positionner */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 5, 6 );
    Spin_bit_voc = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    g_signal_connect( G_OBJECT(Spin_bit_voc), "changed",
                      G_CALLBACK(Afficher_mnemo_voc), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bit_voc, 1, 2, 5, 6 );

    Entry_bit_voc = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bit_voc), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bit_voc, 2, 4, 5, 6 );

    texte = gtk_label_new( _("Type Voix") );   /* Création de l'option menu pour le choix du type de voix */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 6, 7 );

    Combo_type_voc = gtk_combo_box_new_text();
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type_voc), "Homme 1" );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type_voc), "Femme 1" );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type_voc), "Homme 2" );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type_voc), "Femme 2" );
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_type_voc, 1, 2, 6, 7 );

    texte = gtk_label_new( _("Vitesse Voix") );      /* Création du spon pour le choix de la vitesse voix */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 6, 7 );

    Spin_vitesse_voc = gtk_spin_button_new_with_range( 50, 500, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_vitesse_voc, 3, 4, 6, 7 );

    texte = gtk_label_new( _("Message Audio") );                                /* Le message en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 7, 8 );
    Entry_lib_audio = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib_audio), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib_audio, 1, 4, 7, 8 );

    texte = gtk_label_new( _("Message SMS") );                                  /* Le message en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 8, 9 );
    Entry_lib_sms = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib_sms), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib_sms, 1, 4, 8, 9 );

    if (edit_msg)                                                              /* Si edition d'un message */
     { Msg.id = edit_msg->id;
       Msg.num_syn = edit_msg->num_syn;
       Msg.bit_voc = edit_msg->bit_voc;
       Msg.vitesse_voc = edit_msg->vitesse_voc;
       gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_msg->libelle );
       gtk_entry_set_text( GTK_ENTRY(Entry_lib_audio), edit_msg->libelle_audio );
       gtk_entry_set_text( GTK_ENTRY(Entry_lib_sms), edit_msg->libelle_sms );
       gtk_entry_set_text( GTK_ENTRY(Entry_objet), edit_msg->objet );
       gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_type), edit_msg->type );
       gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_type_voc), edit_msg->type_voc );
       gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_sms), edit_msg->sms );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_enable), edit_msg->enable );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_num), edit_msg->num );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bit_voc), edit_msg->bit_voc );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_vitesse_voc), edit_msg->vitesse_voc );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_time_repeat), edit_msg->time_repeat );
       gtk_widget_grab_focus( Entry_lib );
     }
    else { gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_enable), TRUE );
           gtk_widget_grab_focus( Spin_num );
         }
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
