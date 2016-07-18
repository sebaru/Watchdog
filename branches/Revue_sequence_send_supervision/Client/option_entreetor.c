/**********************************************************************************************************/
/* Client/option_entreetor.c        Addition/Edition d'un entreetor Watchdog2.0                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 15 d�c. 2010 11:37:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * option_entreetor.c
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
 
 #include "Reseaux.h"

/********************************* D�finitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 static GtkWidget *Check_DI_furtif;                                              /* Option DI - furtivit� */

/**********************************************************************************************************/
/* Get_options_DI: Rempli une structure mnemo depuis les informations de l'ihm                            */
/* Entr�e: le mnemonique a remplir                                                                        */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 void Get_options_DI ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { struct CMD_TYPE_MNEMO_DI *mnemo;
    mnemo = &mnemo_full->mnemo_di;
    mnemo->furtif = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_DI_furtif));
  }
/**********************************************************************************************************/
/* Get_options_DI_gtktable: Renvoie le widget li� � l'�dition du mnemonique DI                            */
/* Entr�e: rien                                                                                           */
/* sortie: le widget table                                                                                */
/**********************************************************************************************************/
 GtkWidget *Get_options_DI_gtktable ( void )
  { GtkWidget *table, *texte;
    gint i;

    table = gtk_table_new( 5, 2, TRUE );
    i = 0;
    texte = gtk_label_new( _("Options for Digital Inputs") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 2, i, i+1 );

    i++;
    Check_DI_furtif = gtk_check_button_new_with_label( _("Reserved") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_DI_furtif, 0, 1, i, i+1 );

    return(table);
  }
/**********************************************************************************************************/
/* Set_options_DI: positionne dans l'IHM les diff�rents champs de la Digital Input parametre              */
/* Entr�e : une structure representant l'AnalogInput                                                      */
/* Sortie : n�ant                                                                                         */
/**********************************************************************************************************/
 void Set_options_DI ( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { struct CMD_TYPE_MNEMO_DI *mnemo;
    mnemo = &mnemo_full->mnemo_di;
    
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_DI_furtif), mnemo->furtif );
  }
/*--------------------------------------------------------------------------------------------------------*/
