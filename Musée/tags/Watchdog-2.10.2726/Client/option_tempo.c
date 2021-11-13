/**********************************************************************************************************/
/* Client/option_tempo.c        Addition/Edition d'un tempo Watchdog2.0                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 13 mars 2013 18:41:47 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * option_tempo.c
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

 static GtkWidget *Spin_delai_on;                                   /* Delai avant activation de la tempo */
 static GtkWidget *Spin_delai_off;                               /* Delai avant desactivation de la tempo */
 static GtkWidget *Spin_min_on;                                            /* Duree minimale d'activation */
 static GtkWidget *Spin_max_on;                          /* Duree maximale de l'activation (0 = illimité) */

/**********************************************************************************************************/
/* Get_options_Tempo: Rempli une structure mnemo depuis les informations de l'ihm                         */
/* Entrée: le mnemonique a remplir                                                                        */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 void Get_options_Tempo ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { struct CMD_TYPE_MNEMO_TEMPO *mnemo;
    mnemo = &mnemo_full->mnemo_tempo;
    mnemo->min_on    = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_min_on   ) );
    mnemo->max_on    = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_max_on   ) );
    mnemo->delai_on  = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_delai_on ) );
    mnemo->delai_off = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_delai_off) );
  }
/**********************************************************************************************************/
/* Get_options_Tempo_gtktable: Renvoie le widget lié à l'édition de la temporisation                      */
/* Entrée: rien                                                                                           */
/* sortie: le widget table                                                                                */
/**********************************************************************************************************/
 GtkWidget *Get_options_Tempo_gtktable ( void )
  { GtkWidget *table, *texte;
    gint i;

    table = gtk_table_new( 5, 2, TRUE );
    i = 0;
    texte = gtk_label_new( _("Options for Temporisation") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Delai avant activation") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_delai_on = gtk_spin_button_new_with_range( 0, +100000000, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_delai_on, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Duree minimale d'activation") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_min_on = gtk_spin_button_new_with_range( 0, +100000000, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_min_on, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Duree maximale d'activation") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_max_on = gtk_spin_button_new_with_range( 0, +100000000, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_max_on, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Delai avant desactivation") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_delai_off = gtk_spin_button_new_with_range( 0, +100000000, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_delai_off, 1, 2, i, i+1 );

    return(table);
  }
/**********************************************************************************************************/
/* Set_options_Tempo: Rempli l'ihm depuis une structure mnemo                                             */
/* Entrée : une structure representant l'AnalogInput                                                      */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 void Set_options_Tempo ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { struct CMD_TYPE_MNEMO_TEMPO *mnemo;
    mnemo = &mnemo_full->mnemo_tempo;
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_min_on),    mnemo->min_on );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_max_on),    mnemo->max_on );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_delai_on),  mnemo->delai_on );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_delai_off), mnemo->delai_off );
  }
/*--------------------------------------------------------------------------------------------------------*/
