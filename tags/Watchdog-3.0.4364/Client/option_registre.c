/******************************************************************************************************************************/
/* Client/option_registre.c        Addition/Edition d'un mnemonique de type Registre Watchdog2.0                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    21.03.2017 08:02:54 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * option_registre.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sébastien Lefevre
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

/******************************************** Définitions des prototypes programme ********************************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG Config;                                                              /* Configuration generale watchdog */

 static GtkWidget *Entry_unite;                                                        /* Unite correspondante à l'entrée ana */

/******************************************************************************************************************************/
/* Get_options_registre: Récupère les options d'un mnemo Registre et les applique a la structure en parametre                 */
/* Entrée: la structure destinatrice                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Get_options_Registre ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { struct CMD_TYPE_MNEMO_REGISTRE *mnemo;
    mnemo = &mnemo_full->mnemo_r;
    g_snprintf( mnemo->unite, sizeof(mnemo->unite),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_unite) ) );
  }
/******************************************************************************************************************************/
/* Ajouter_entreeANA: Ajoute un entreeANA au systeme                                                                          */
/* Entrée: rien                                                                                                               */
/* sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 GtkWidget *Get_options_Registre_gtktable ( void )
  { GtkWidget *table, *texte;
    gint i;

    table = gtk_table_new( 2, 4, TRUE );

    i=0;
    texte = gtk_label_new( _("Options for Registers") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 4, i, i+1 );

    i++;
    texte = gtk_label_new( _("Unit") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Entry_unite = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_unite), NBR_CARAC_UNITE_MNEMONIQUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_unite, 1, 4, i, i+1 );
    gtk_widget_grab_focus( Entry_unite );
    return(table);
  }
/******************************************************************************************************************************/
/* Set_options_AI: positionne dans l'IHM les différents champs de l'AnalogInput en parametre                                  */
/* Entrée : une structure representant l'AnalogInput                                                                          */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Set_options_Registre ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { struct CMD_TYPE_MNEMO_REGISTRE *mnemo;
    if (mnemo_full)
     { mnemo = &mnemo_full->mnemo_r;
       gtk_entry_set_text( GTK_ENTRY(Entry_unite), mnemo->unite );
     }
    else
     { gtk_entry_set_text( GTK_ENTRY(Entry_unite), "m/s" );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
