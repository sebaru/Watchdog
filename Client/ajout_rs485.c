/**********************************************************************************************************/
/* Client/ajout_rs485.c        Addition/Edition d'un rs485 Watchdog2.0                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 20 nov 2004 13:47:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_rs485.c
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
 #include <sys/time.h>
 #include <unistd.h>


 #include "Reseaux.h"
/********************************* D�finitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static GtkWidget *Spin_num;                                  /* Num�ro du rs485 en cours d'�dition/ajout */
 static GtkWidget *Spin_bit;                                           /* Num�ro du but de comm du module */
 static GtkWidget *Entry_bit;                                                /* Mn�monique du bit de comm */
 static GtkWidget *Entry_lib;                                                         /* Libelle du rs485 */
 static GtkWidget *Check_actif;                                         /* Le module doit-il etre actif ? */
 static GtkWidget *Spin_e_min;                                        /* num�ro de l'entr�e minimum g�r�e */
 static GtkWidget *Spin_e_max;                                        /* num�ro de l'entr�e minimum g�r�e */
 static GtkWidget *Spin_ea_min;                                       /* num�ro de l'entr�e minimum g�r�e */
 static GtkWidget *Spin_ea_max;                                       /* num�ro de l'entr�e minimum g�r�e */
 static GtkWidget *Spin_s_min;                                        /* num�ro de l'entr�e minimum g�r�e */
 static GtkWidget *Spin_s_max;                                        /* num�ro de l'entr�e minimum g�r�e */
 static GtkWidget *Spin_sa_min;                                       /* num�ro de l'entr�e minimum g�r�e */
 static GtkWidget *Spin_sa_max;                                       /* num�ro de l'entr�e minimum g�r�e */
 static struct CMD_TYPE_RS485 Rs485;                                        /* Message en cours d'�dition */

/**********************************************************************************************************/
/* CB_ajouter_editer_rs485: Fonction appel�e qd on appuie sur un des boutons de l'interface               */
/* Entr�e: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_rs485 ( GtkDialog *dialog, gint reponse, gboolean edition )
  { g_snprintf( Rs485.libelle, sizeof(Rs485.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
    Rs485.actif      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_actif) );
    Rs485.num        = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_num) );
    Rs485.bit_comm   = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bit) );
    Rs485.e_min      = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_e_min) );
    Rs485.e_max      = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_e_max) );
    Rs485.ea_min     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_ea_min) );
    Rs485.ea_max     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_ea_max) );
    Rs485.s_min      = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_s_min) );
    Rs485.s_max      = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_s_max) );
    Rs485.sa_min     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_sa_min) );
    Rs485.sa_max     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_sa_max) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_RS485, (edition ? SSTAG_CLIENT_VALIDE_EDIT_RS485
                                                  : SSTAG_CLIENT_ADD_RS485),
                              (gchar *)&Rs485, sizeof( struct CMD_TYPE_RS485 ) );
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
 void Proto_afficher_mnemo_rs485 ( struct CMD_TYPE_MNEMONIQUE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );             /* Formatage */
    gtk_entry_set_text( GTK_ENTRY(Entry_bit), chaine );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Afficher_mnemo_rs485 ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    mnemo.type = MNEMO_MONOSTABLE;
    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_bit) );
    
    Envoi_serveur( TAG_RS485, SSTAG_CLIENT_TYPE_NUM_MNEMO_RS485,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Ajouter_rs485: Ajoute un rs485 au systeme                                                          */
/* Entr�e: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_rs485 ( struct CMD_TYPE_RS485 *edit_rs485 )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint i;

    if (edit_rs485)
     { memcpy( &Rs485, edit_rs485, sizeof(struct CMD_TYPE_RS485) );    /* Save pour utilisation future */
     }
    else memset (&Rs485, 0, sizeof(struct CMD_TYPE_RS485) );                  /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_rs485 ? _("Edit a rs485") : _("Add a rs485")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_rs485),
                      GINT_TO_POINTER( (edit_rs485 ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Cr�ation de l'interface graphique */
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
    Check_actif = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_actif, 0, 1, i, i+1 );

    texte = gtk_label_new( _("Rs485 Num") );                       /* Num�ro du module en cours d'edition */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 1, 2, i, i+1 );
    Spin_num = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_num, 2, 3, i, i+1 );

    i++;
    texte = gtk_label_new( _("E min") );                                    /* Num�ro de l'entr�e minimum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_e_min = gtk_spin_button_new_with_range( -1, NBR_ENTRE_TOR-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_e_min, 1, 2, i, i+1 );

    texte = gtk_label_new( _("E max") );                                    /* Num�ro de l'entr�e maximum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    Spin_e_max = gtk_spin_button_new_with_range( -1, NBR_ENTRE_TOR-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_e_max, 3, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("EA min") );                                   /* Num�ro de l'entr�e minimum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_ea_min = gtk_spin_button_new_with_range( -1, NBR_ENTRE_ANA-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_ea_min, 1, 2, i, i+1 );

    texte = gtk_label_new( _("EA max") );                                   /* Num�ro de l'entr�e maximum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    Spin_ea_max = gtk_spin_button_new_with_range( -1, NBR_ENTRE_ANA-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_ea_max, 3, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("S min") );                                   /* Num�ro de la sortie minimum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_s_min = gtk_spin_button_new_with_range( -1, NBR_SORTIE_TOR-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_s_min, 1, 2, i, i+1 );

    texte = gtk_label_new( _("S max") );                                   /* Num�ro de la sortie maximum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    Spin_s_max = gtk_spin_button_new_with_range( -1, NBR_SORTIE_TOR-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_s_max, 3, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("SA min") );                                  /* Num�ro de la sortie minimum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_sa_min = gtk_spin_button_new_with_range( -1, NBR_SORTIE_ANA-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_sa_min, 1, 2, i, i+1 );

    texte = gtk_label_new( _("SA max") );                                  /* Num�ro de la sortie maximum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    Spin_sa_max = gtk_spin_button_new_with_range( -1, NBR_SORTIE_ANA-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_sa_max, 3, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Bit de comm") );                           /* Num�ro du bit M a positionner */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_bit = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS-1, 1 );
    g_signal_connect( G_OBJECT(Spin_bit), "changed",
                      G_CALLBACK(Afficher_mnemo_rs485), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bit, 1, 2, i, i+1 );

    Entry_bit = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bit), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bit, 2, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Libelle") );                                        /* Le rs485 en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, i, i+1 );

    if (edit_rs485)                                                              /* Si edition d'un rs485 */
     { gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_rs485->libelle );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), edit_rs485->actif );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_num),    edit_rs485->num      );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bit),    edit_rs485->bit_comm );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_e_min),  edit_rs485->e_min    );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_e_max),  edit_rs485->e_max    );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_ea_min), edit_rs485->ea_min   );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_ea_max), edit_rs485->ea_max   );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_s_min),  edit_rs485->s_min    );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_s_max),  edit_rs485->s_max    );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_sa_min), edit_rs485->sa_min   );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_sa_max), edit_rs485->sa_max   );
       gtk_widget_grab_focus( Entry_lib );
     }
    else { gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), TRUE );
           gtk_widget_grab_focus( Spin_num );
         }
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface compl�te */
  }
/*--------------------------------------------------------------------------------------------------------*/
