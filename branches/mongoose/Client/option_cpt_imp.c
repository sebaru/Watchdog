/**********************************************************************************************************/
/* Client/option_cpt_imp.c        Gestion des options des compteurs d'impulsions Watchdog2.0              */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 15 déc. 2010 11:38:05 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * option_cpt_imp.c
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

 gchar *TYPE_CI[NBR_TYPE_CI] =
  { "Totalisateur",
    "Moyenneur (s)",
    "Moyenneur (min)"
  };

 static GtkWidget *Entry_unite;                                    /* Unite correspondante à l'entrée ana */
 static GtkWidget *Spin_multi;                                              /* Multiplicateur d'affichage */
 static GtkWidget *Option_type;                            /* Type de capteur (totalisateur/moyenneur/..) */

/**********************************************************************************************************/
/* Type_ci_vers_string: renvoie le type d'ea sous forme de chaine de caractere                            */
/* Entrée: numéro du type de CI                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gchar *Type_ci_vers_string ( guint type )
  { if (type<NBR_TYPE_CI) return( TYPE_CI[type] );
                     else return ( "Unknown" );
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_cpt_imp: Fonction appelée qd on appuie sur un des boutons de l'interface             */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 void Get_options_CPTIMP ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { struct CMD_TYPE_MNEMO_CPT_IMP *mnemo;
    mnemo = &mnemo_full->mnemo_cptimp;

    g_snprintf( mnemo->unite, sizeof(mnemo->unite),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_unite) ) );
    mnemo->type  = gtk_combo_box_get_active( GTK_COMBO_BOX(Option_type ) );
    mnemo->multi = gtk_spin_button_get_value( GTK_SPIN_BUTTON(Spin_multi) );
  }
/**********************************************************************************************************/
/* Ajouter_cpt_imp: Ajoute un cpt_imp au systeme                                                          */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 GtkWidget *Get_options_CPTIMP_gtktable ( void )
  { GtkWidget *table, *texte;
    gint cpt, i;

    table = gtk_table_new( 5, 4, TRUE );

    i=0;
    texte = gtk_label_new( _("Options for Count Impulse") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Type") );                                              /* Unite du compteur */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Option_type = gtk_combo_box_new_text();
    for ( cpt=0; cpt<NBR_TYPE_CI; cpt++ )
     { gtk_combo_box_insert_text( GTK_COMBO_BOX(Option_type), cpt, Type_ci_vers_string(cpt) );
     }
    gtk_table_attach_defaults( GTK_TABLE(table), Option_type, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Unit") );                                              /* Unite du compteur */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_unite = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_unite), NBR_CARAC_UNITE_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_unite, 1, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Multi") );                                              /* Unite du compteur */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_multi = gtk_spin_button_new_with_range( 0.001, 1000.0, 0.001 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_multi, 1, 4, i, i+1 );

    gtk_widget_grab_focus( Entry_unite );
    return(table);
  }
/**********************************************************************************************************/
/* Set_options_AI: positionne dans l'IHM les différents champs de l'AnalogInput en parametre              */
/* Entrée : une structure representant l'AnalogInput                                                      */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 void Set_options_CPTIMP ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { struct CMD_TYPE_MNEMO_CPT_IMP *mnemo;
    mnemo = &mnemo_full->mnemo_cptimp;

    gtk_entry_set_text( GTK_ENTRY(Entry_unite), mnemo->unite );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_multi), mnemo->multi );
    gtk_combo_box_set_active( GTK_COMBO_BOX(Option_type),  mnemo->type  );
  }
/*--------------------------------------------------------------------------------------------------------*/
