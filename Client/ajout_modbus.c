/**********************************************************************************************************/
/* Client/ajout_modbus.c        Addition/Edition d'un modbus Watchdog2.0                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 05 sept. 2010 16:58:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_modbus.c
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
 static GtkWidget *Spin_watchdog;                          /* Watchdig du modbus en cours d'édition/ajout */
 static GtkWidget *Spin_bit;                                           /* Numéro du bit de comm du module */
 static GtkWidget *Entry_bit;                                                /* Mnémonique du bit de comm */
 static GtkWidget *Entry_ip;                                                             /* @IP du module */
 static GtkWidget *Entry_lib;                                                        /* Libelle du modbus */
 static GtkWidget *Check_actif;                                         /* Le module doit-il etre actif ? */
 static struct CMD_TYPE_MODBUS Modbus;                                       /* Module en cours d'édition */

 static GtkWidget *F_borne;                                            /* Widget de l'interface graphique */
 static GtkWidget *Combo_borne_type;
 static GtkWidget *Spin_borne_adresse;
 static GtkWidget *Spin_borne_min;
 static GtkWidget *Spin_borne_nbr;
 static struct CMD_TYPE_BORNE_MODBUS Borne;                                   /* Borne en cours d'edition */

/**********************************************************************************************************/
/* CB_ajouter_editer_modbus: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_modbus ( GtkDialog *dialog, gint reponse, gboolean edition )
  { g_snprintf( Modbus.libelle, sizeof(Modbus.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
    g_snprintf( Modbus.ip, sizeof(Modbus.ip),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_ip) ) );
    Modbus.actif      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_actif) );
    Modbus.watchdog   = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_watchdog) );
    Modbus.bit        = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bit) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_MODBUS, (edition ? SSTAG_CLIENT_VALIDE_EDIT_MODBUS
                                                   : SSTAG_CLIENT_ADD_MODBUS),
                              (gchar *)&Modbus, sizeof( struct CMD_TYPE_MODBUS ) );
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
 void Proto_afficher_mnemo_modbus ( struct CMD_TYPE_MNEMONIQUE *mnemo )
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
 static void Afficher_mnemo_modbus ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    mnemo.type = MNEMO_BISTABLE;
    mnemo.num  = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_bit) );
    
    Envoi_serveur( TAG_MODBUS, SSTAG_CLIENT_TYPE_NUM_MNEMO_MODBUS,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Ajouter_modbus: Ajoute un modbus au systeme                                                            */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_modbus ( struct CMD_TYPE_MODBUS *edit_modbus )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint i;

    if (edit_modbus)
     { memcpy( &Modbus, edit_modbus, sizeof(struct CMD_TYPE_MODBUS) );    /* Save pour utilisation future */
     }
    else memset (&Modbus, 0, sizeof(struct CMD_TYPE_MODBUS) );                  /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_modbus ? _("Edit a modbus") : _("Add a modbus")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_modbus),
                      GINT_TO_POINTER( (edit_modbus ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 4, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    i=0;
    Check_actif = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_actif, 0, 1, i, i+1 );

    texte = gtk_label_new( _("Watchdog") );                           /* Temps de deconnexion des sorties */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    Spin_watchdog = gtk_spin_button_new_with_range( 0, 300, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_watchdog, 3, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Bit de comm") );                           /* Numéro du bit M a positionner */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_bit = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS-1, 1 );
    g_signal_connect( G_OBJECT(Spin_bit), "changed",
                      G_CALLBACK(Afficher_mnemo_modbus), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bit, 1, 2, i, i+1 );

    Entry_bit = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bit), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bit, 2, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Name/@IP") );                                     /* L'adresse IP du module */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_ip = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_ip), sizeof(Modbus.ip) );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_ip, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Libelle") );                                       /* Le modbus en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, i, i+1 );

    if (edit_modbus)                                                            /* Si edition d'un modbus */
     { gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_modbus->libelle );
       gtk_entry_set_text( GTK_ENTRY(Entry_ip),  edit_modbus->ip );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), edit_modbus->actif );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_watchdog),    edit_modbus->watchdog );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bit),         edit_modbus->bit );
       gtk_widget_grab_focus( Entry_lib );
     }
    else { gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), TRUE );
           gtk_widget_grab_focus( Spin_watchdog );
         }
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_borne_modbus: Fonction appelée qd on appuie sur un des boutons de l'interface        */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_borne_modbus ( GtkDialog *dialog, gint reponse, gboolean edition )
  { Borne.type    = gtk_combo_box_get_active (GTK_COMBO_BOX (Combo_borne_type) );
    Borne.adresse = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_borne_adresse) );
    Borne.min     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_borne_min) );
    Borne.nbr     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_borne_nbr) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_MODBUS, (edition ? SSTAG_CLIENT_VALIDE_EDIT_BORNE_MODBUS
                                                   : SSTAG_CLIENT_ADD_BORNE_MODBUS),
                              (gchar *)&Borne, sizeof( struct CMD_TYPE_BORNE_MODBUS ) );
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_borne);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Menu_ajouter_editer_borne_modbus: Ajoute une borne au systeme                                          */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_borne_modbus ( gboolean edition, struct CMD_TYPE_BORNE_MODBUS *edit_borne )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint i, cpt;

    memcpy( &Borne, edit_borne, sizeof(struct CMD_TYPE_BORNE_MODBUS) );   /* Save pour utilisation future */
    F_borne = gtk_dialog_new_with_buttons( (edition ? _("Edit a borne") : _("Add a borne")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_borne, "response",
                      G_CALLBACK(CB_ajouter_editer_borne_modbus),
                      GINT_TO_POINTER( edition ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_borne)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 4, 2, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    i=0;
    texte = gtk_label_new( _("Type") );                                      /* Numéro MODBUS de la borne */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );

    Combo_borne_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_MODE_BORNE; cpt++ )
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_borne_type), Mode_borne_vers_string(cpt) ); }
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_borne_type, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Adresse") );                                   /* Numéro MODBUS de la borne */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_borne_adresse = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_borne_adresse, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Bit interne min") );                                 /* Bit logique minimum */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_borne_min = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_borne_min, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Nombre d'I/O") );                               /* Nombre d'I/O de la borne */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_borne_nbr = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS-1, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_borne_nbr, 1, 2, i, i+1 );

    if (edition)                                                                /* Si edition d'un modbus */
     { gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_borne_type),        edit_borne->type );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_borne_adresse),    edit_borne->adresse );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_borne_min),        edit_borne->min     );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_borne_nbr),        edit_borne->nbr     );
     }
    else { gtk_combo_box_set_active (GTK_COMBO_BOX (Combo_borne_type), BORNE_INPUT_TOR );
           gtk_widget_grab_focus( Combo_borne_type );
         }
    gtk_widget_show_all(F_borne);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
