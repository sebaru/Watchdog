/**********************************************************************************************************/
/* Client/ajout_rfxcom.c        Addition/Edition d'un rfxcom Watchdog2.0                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                    lun. 11 juin 2012 19:38:07 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_rfxcom.c
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
 #include <unistd.h>


 #include "Reseaux.h"
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static GtkWidget *Combo_type;                                                  /* Type du capteur rfxcom */
 static GtkWidget *Spin_canal;                                                 /* Canal du capteur rfxcom */
 static GtkWidget *Entry_lib;                                                        /* Libelle du rfxcom */
 static GtkWidget *Spin_e_min;                                        /* numéro de l'entrée minimum gérée */
 static GtkWidget *Spin_ea_min;                                       /* numéro de l'entrée minimum gérée */
 static GtkWidget *Spin_a_min;                                        /* numéro de l'entrée minimum gérée */
 static struct CMD_TYPE_RFXCOM Rfxcom;                                        /* Message en cours d'édition */

/**********************************************************************************************************/
/* CB_ajouter_editer_rfxcom: Fonction appelée qd on appuie sur un des boutons de l'interface               */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_rfxcom ( GtkDialog *dialog, gint reponse, gboolean edition )
  { g_snprintf( Rfxcom.libelle, sizeof(Rfxcom.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
    Rfxcom.e_min      = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_e_min) );
    Rfxcom.ea_min     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_ea_min) );
    Rfxcom.a_min      = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_a_min) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_RFXCOM, (edition ? SSTAG_CLIENT_VALIDE_EDIT_RFXCOM
                                                  : SSTAG_CLIENT_ADD_RFXCOM),
                              (gchar *)&Rfxcom, sizeof( struct CMD_TYPE_RFXCOM ) );
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Proto_afficher_mnemo_rfxcom ( struct CMD_TYPE_MNEMONIQUE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );             /* Formatage */
/*    gtk_entry_set_text( GTK_ENTRY(Entry_bit), chaine );*/
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Afficher_mnemo_rfxcom ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    mnemo.type = MNEMO_MONOSTABLE;
/*    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_bit) );*/
    
    Envoi_serveur( TAG_RFXCOM, SSTAG_CLIENT_TYPE_NUM_MNEMO_RFXCOM,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Ajouter_rfxcom: Ajoute un rfxcom au systeme                                                          */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_rfxcom ( struct CMD_TYPE_RFXCOM *edit_rfxcom )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint i;

    if (edit_rfxcom)
     { memcpy( &Rfxcom, edit_rfxcom, sizeof(struct CMD_TYPE_RFXCOM) );    /* Save pour utilisation future */
     }
    else memset (&Rfxcom, 0, sizeof(struct CMD_TYPE_RFXCOM) );                  /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_rfxcom ? _("Edit a rfxcom") : _("Add a rfxcom")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_rfxcom),
                      GINT_TO_POINTER( (edit_rfxcom ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 7, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    i=0;
/*    Check_actif = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_actif, 0, 1, i, i+1 );
*/
    texte = gtk_label_new( _("Rfxcom Num") );                       /* Numéro du module en cours d'edition */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 1, 2, i, i+1 );
  /*  Spin_num = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_num, 2, 3, i, i+1 );
*/
    i++;
    texte = gtk_label_new( _("E min") );                                    /* Numéro de l'entrée minimum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_e_min = gtk_spin_button_new_with_range( -1, NBR_ENTRE_TOR-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_e_min, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("EA min") );                                   /* Numéro de l'entrée minimum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_ea_min = gtk_spin_button_new_with_range( -1, NBR_ENTRE_ANA-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_ea_min, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("A min") );                                   /* Numéro de la sortie minimum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_a_min = gtk_spin_button_new_with_range( -1, NBR_SORTIE_TOR-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_a_min, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Libelle") );                                        /* Le rfxcom en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8 );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, i, i+1 );

    if (edit_rfxcom)                                                              /* Si edition d'un rfxcom */
     { gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_rfxcom->libelle );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_e_min),  edit_rfxcom->e_min    );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_ea_min), edit_rfxcom->ea_min   );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_a_min),  edit_rfxcom->a_min    );
       gtk_widget_grab_focus( Entry_lib );
     }
    else { gtk_widget_grab_focus( Entry_lib );
         }
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
