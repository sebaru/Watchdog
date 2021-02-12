/******************************************************************************************************************************/
/* Client/option_entreeANA.c        Addition/Edition d'un entreeANA Watchdog2.0                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mer. 15 déc. 2010 11:37:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * option_entreeana.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sébastien Lefevre
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

 gchar *TYPE_EA[NBR_TYPE_ENTREEANA] =
  { "Non Interprete",
    "4/20 mA 12 bits",
    "4/20 mA 10 bits",
    "WAGO 4/20 mA 750-455",
    "WAGO PT100 750-461",
  };

/******************************************** Définitions des prototypes programme ********************************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG Config;                                                              /* Configuration generale watchdog */

 static GtkWidget *Spin_min;                                                  /* Numéro du entreeANA en cours d'édition/ajout */
 static GtkWidget *Spin_max;                                                  /* Numéro du entreeANA en cours d'édition/ajout */
 static GtkWidget *Entry_unite;                                                        /* Unite correspondante à l'entrée ana */
 static GtkWidget *Option_type;                                                                    /* Type de gestion de l'ea */

/******************************************************************************************************************************/
/* Type_ea_vers_string: renvoie le type d'ea sous forme de chaine de caractere                                                */
/* Entrée: numéro du type d'entree ANA                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Type_ea_vers_string ( guint type )
  { if (type<NBR_TYPE_ENTREEANA) return( TYPE_EA[type] );
                            else return ( "Unknown" );
  }
/******************************************************************************************************************************/
/* CB_ajouter_editer_entreeANA: Fonction appelée qd on appuie sur un des boutons de l'interface                               */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                                                   */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 void Get_options_AI ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { struct CMD_TYPE_MNEMO_AI *mnemo;
    mnemo = &mnemo_full->mnemo_ai;
    g_snprintf( mnemo->unite, sizeof(mnemo->unite),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_unite) ) );
    mnemo->min  = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_min) );
    mnemo->max  = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_max) );
    mnemo->type = gtk_combo_box_get_active( GTK_COMBO_BOX(Option_type) );
  }
/******************************************************************************************************************************/
/* Ajouter_entreeANA: Ajoute un entreeANA au systeme                                                                          */
/* Entrée: rien                                                                                                               */
/* sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 GtkWidget *Get_options_AI_gtktable ( void )
  { GtkWidget *table, *texte;
    gint cpt, i;

    table = gtk_table_new( 5, 4, TRUE );

    i=0;
    texte = gtk_label_new( _("Options for Analog Inputs") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Type") );                                                            /* Type de gestion de l'EA */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Option_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_ENTREEANA; cpt++ )
     { gtk_combo_box_insert_text( GTK_COMBO_BOX(Option_type), cpt, Type_ea_vers_string(cpt) );
     }
    gtk_table_attach_defaults( GTK_TABLE(table), Option_type, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Unit") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_unite = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_unite), NBR_CARAC_UNITE_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_unite, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("min") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_min = gtk_spin_button_new_with_range( -1000, +1000, 0.5 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_min, 1, 2, i, i+1 );

    texte = gtk_label_new( _("max") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, i, i+1 );
    Spin_max = gtk_spin_button_new_with_range( -1000, +1000, 0.5 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_max, 3, 4, i, i+1 );

    gtk_widget_grab_focus( Entry_unite );
    return(table);
  }
/******************************************************************************************************************************/
/* Set_options_AI: positionne dans l'IHM les différents champs de l'AnalogInput en parametre                                  */
/* Entrée : une structure representant l'AnalogInput                                                                          */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Set_options_AI ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { struct CMD_TYPE_MNEMO_AI *mnemo;
    if (mnemo_full)
     { mnemo = &mnemo_full->mnemo_ai;
       gtk_entry_set_text( GTK_ENTRY(Entry_unite), mnemo->unite );
       gtk_combo_box_set_active( GTK_COMBO_BOX(Option_type), mnemo->type );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_min), mnemo->min );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_max), mnemo->max );
     }
    else
     { gtk_entry_set_text( GTK_ENTRY(Entry_unite), "m/s" );
       gtk_combo_box_set_active( GTK_COMBO_BOX(Option_type), 0 );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_min), 0 );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_max), 100 );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
