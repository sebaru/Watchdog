/**********************************************************************************************************/
/* Client/ajout_onduleur.c        Addition/Edition d'un onduleur Watchdog2.0                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                   mer. 14 juil. 2010 22:00:27 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_onduleur.c
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
 static GtkWidget *Spin_bit_comm;                              /* Numéro du bit bistable de communication */
 static GtkWidget *Entry_bit_comm;                                                   /* Mnemo du bistable */
 static GtkWidget *Spin_real_power;                                                     /* Numéro de l'EA */
 static GtkWidget *Entry_real_power;                                                     /* Mnemo de l'EA */
 static GtkWidget *Spin_input_voltage;                                                  /* Numéro de l'EA */
 static GtkWidget *Entry_input_voltage;                                                  /* Mnemo de l'EA */
 static GtkWidget *Spin_load;                                                           /* Numéro de l'EA */
 static GtkWidget *Entry_load;                                                           /* Mnemo de l'EA */
 static GtkWidget *Spin_battery_charge;                                                 /* Numéro de l'EA */
 static GtkWidget *Entry_battery_charge;                                                 /* Mnemo de l'EA */
 static GtkWidget *Entry_lib;                                                    /* Libelle de l'onduleur */
 static GtkWidget *Entry_host;                                              /* Host hébergeant l'onduleur */
 static GtkWidget *Entry_ups;                                                             /* Nom de l'UPS */
 static GtkWidget *Check_actif;                                               /* Doit-il etre supervisé ? */
 static struct CMD_TYPE_ONDULEUR Edit_onduleur;                            /* Onduleur en cours d'édition */

/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Afficher_mnemo ( GtkWidget *widget )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    gint sstag;
printf("Afficher_mnemo\n");
    mnemo.num  = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(widget) );
    if ( widget == Spin_bit_comm )
     { mnemo.type = MNEMO_BISTABLE;
       sstag      = SSTAG_CLIENT_TYPE_NUM_MNEMO_BIT_COMM;
     }
    else if ( widget == Spin_load )
     { mnemo.type = MNEMO_ENTREE_ANA;
       sstag      = SSTAG_CLIENT_TYPE_NUM_MNEMO_EA_UPS_LOAD;
     }
    else if ( widget == Spin_real_power )
     { mnemo.type = MNEMO_ENTREE_ANA;
       sstag      = SSTAG_CLIENT_TYPE_NUM_MNEMO_EA_UPS_REAL_POWER;
     }
    else if ( widget == Spin_battery_charge )
     { mnemo.type = MNEMO_ENTREE_ANA;
       sstag      = SSTAG_CLIENT_TYPE_NUM_MNEMO_EA_BATTERY_CHARGE;
     }
    else if ( widget == Spin_input_voltage )
     { mnemo.type = MNEMO_ENTREE_ANA;
       sstag      = SSTAG_CLIENT_TYPE_NUM_MNEMO_EA_INPUT_VOLTAGE;
     }
    else return;

    Envoi_serveur( TAG_ONDULEUR, sstag, (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Proto_afficher_mnemo_onduleur ( int tag, struct CMD_TYPE_MNEMONIQUE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );             /* Formatage */
    switch (tag)
     { case SSTAG_SERVEUR_TYPE_NUM_MNEMO_BIT_COMM:
            gtk_entry_set_text( GTK_ENTRY(Entry_bit_comm), chaine );
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_EA_UPS_LOAD:
            gtk_entry_set_text( GTK_ENTRY(Entry_load), chaine );
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_EA_UPS_REAL_POWER:
            gtk_entry_set_text( GTK_ENTRY(Entry_real_power), chaine );
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_EA_INPUT_VOLTAGE:
            gtk_entry_set_text( GTK_ENTRY(Entry_input_voltage), chaine );
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_EA_BATTERY_CHARGE:
            gtk_entry_set_text( GTK_ENTRY(Entry_battery_charge), chaine );
            break;
     }
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_onduleur: Fonction appelée qd on appuie sur un des boutons de l'interface            */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_onduleur ( GtkDialog *dialog, gint reponse, gboolean edition )
  { switch(reponse)
     { case GTK_RESPONSE_OK:
        { if (edition)
           { g_snprintf( Edit_onduleur.libelle, sizeof(Edit_onduleur.libelle),
                         "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
             g_snprintf( Edit_onduleur.host, sizeof(Edit_onduleur.host),
                         "%s", gtk_entry_get_text( GTK_ENTRY(Entry_host) ) );
             g_snprintf( Edit_onduleur.ups, sizeof(Edit_onduleur.ups),
                         "%s", gtk_entry_get_text( GTK_ENTRY(Entry_ups) ) );
             Edit_onduleur.actif = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_actif) );
             Edit_onduleur.bit_comm = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bit_comm) );
             Edit_onduleur.ea_input_voltage  = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_input_voltage) );
             Edit_onduleur.ea_ups_load       = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_load) );
             Edit_onduleur.ea_ups_real_power = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_real_power) );
             Edit_onduleur.ea_battery_charge = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_battery_charge) );

             Envoi_serveur( TAG_ONDULEUR, SSTAG_CLIENT_VALIDE_EDIT_ONDULEUR,
                           (gchar *)&Edit_onduleur, sizeof( struct CMD_TYPE_ONDULEUR ) );
           }
          else
           { struct CMD_TYPE_ONDULEUR new_onduleur;
             g_snprintf( new_onduleur.libelle, sizeof(new_onduleur.libelle),
                         "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
             g_snprintf( new_onduleur.host, sizeof(new_onduleur.host),
                         "%s", gtk_entry_get_text( GTK_ENTRY(Entry_host) ) );
             g_snprintf( new_onduleur.ups, sizeof(new_onduleur.ups),
                         "%s", gtk_entry_get_text( GTK_ENTRY(Entry_ups) ) );
             new_onduleur.actif = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_actif) );
             new_onduleur.bit_comm = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bit_comm) );
             new_onduleur.ea_input_voltage  = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_input_voltage) );
             new_onduleur.ea_ups_load       = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_load) );
             new_onduleur.ea_ups_real_power = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_real_power) );
             new_onduleur.ea_battery_charge = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_battery_charge) );

             Envoi_serveur( TAG_ONDULEUR, SSTAG_CLIENT_ADD_ONDULEUR,
                           (gchar *)&new_onduleur, sizeof( struct CMD_TYPE_ONDULEUR ) );
           }
          break;
        }
       case GTK_RESPONSE_CANCEL:
       default: break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_onduleur: Ajoute un onduleur au systeme                                                      */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_onduleur ( struct CMD_TYPE_ONDULEUR *edit_onduleur )
  { GtkWidget *frame, *table, *texte, *hboite;
printf("Ajouter_editer_onduleur\n");
    if (edit_onduleur)
     { memcpy( &Edit_onduleur, edit_onduleur, sizeof(struct CMD_TYPE_ONDULEUR) );
                                                                          /* Save pour utilisation future */
     }
    else memset (&Edit_onduleur, 0, sizeof(struct CMD_TYPE_ONDULEUR) );             /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_onduleur ? _("Edit a UPS") : _("Add a UPS")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_onduleur),
                      GINT_TO_POINTER( (edit_onduleur ? TRUE : FALSE) ) );

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

    Check_actif = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_actif, 0, 1, 0, 1 );

    texte = gtk_label_new( _("Bit comm") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Spin_bit_comm = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    g_signal_connect( G_OBJECT(Spin_bit_comm), "changed",
                      G_CALLBACK(Afficher_mnemo), Spin_bit_comm );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bit_comm, 1, 2, 1, 2 );
    Entry_bit_comm = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bit_comm), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bit_comm, 2, 4, 1, 2 );

    texte = gtk_label_new( _("EA Load") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 2, 3 );
    Spin_load = gtk_spin_button_new_with_range( 0, NBR_ENTRE_ANA, 1 );
    g_signal_connect( G_OBJECT(Spin_load), "changed",
                      G_CALLBACK(Afficher_mnemo), Spin_load );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_load, 1, 2, 2, 3 );
    Entry_load = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_load), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_load, 2, 4, 2, 3 );

    texte = gtk_label_new( _("EA Real Power") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 3, 4 );
    Spin_real_power = gtk_spin_button_new_with_range( 0, NBR_ENTRE_ANA, 1 );
    g_signal_connect( G_OBJECT(Spin_real_power), "changed",
                      G_CALLBACK(Afficher_mnemo), Spin_real_power );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_real_power, 1, 2, 3, 4 );
    Entry_real_power = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_real_power), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_real_power, 2, 4, 3, 4 );

    texte = gtk_label_new( _("EA Battery Charge") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 4, 5 );
    Spin_battery_charge = gtk_spin_button_new_with_range( 0, NBR_ENTRE_ANA, 1 );
    g_signal_connect( G_OBJECT(Spin_battery_charge), "changed",
                      G_CALLBACK(Afficher_mnemo), Spin_battery_charge );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_battery_charge, 1, 2, 4, 5 );
    Entry_battery_charge = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_battery_charge), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_battery_charge, 2, 4, 4, 5 );

    texte = gtk_label_new( _("EA Input Voltage") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 5, 6 );
    Spin_input_voltage = gtk_spin_button_new_with_range( 0, NBR_ENTRE_ANA, 1 );
    g_signal_connect( G_OBJECT(Spin_input_voltage), "changed",
                      G_CALLBACK(Afficher_mnemo), Spin_input_voltage );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_input_voltage, 1, 2, 5, 6 );
    Entry_input_voltage = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_input_voltage), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_input_voltage, 2, 4, 5, 6 );
    Afficher_mnemo(Spin_input_voltage);                        /* Demande l'affichage du mnemo associé */

    if (edit_onduleur)                                                        /* Si edition d'un onduleur */
     { Edit_onduleur.id = edit_onduleur->id;
       gtk_entry_set_text( GTK_ENTRY(Entry_lib),  edit_onduleur->libelle);
       gtk_entry_set_text( GTK_ENTRY(Entry_host), edit_onduleur->host );
       gtk_entry_set_text( GTK_ENTRY(Entry_ups),  edit_onduleur->ups );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bit_comm), edit_onduleur->bit_comm );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_load), edit_onduleur->ea_ups_load );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_real_power), edit_onduleur->ea_ups_real_power );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_battery_charge), edit_onduleur->ea_battery_charge );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_input_voltage), edit_onduleur->ea_input_voltage );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), edit_onduleur->actif );
     }

    gtk_widget_grab_focus( Entry_lib );
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
