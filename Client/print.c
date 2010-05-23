/**********************************************************************************************************/
/* Client/print.c        Configuration des impressions de Watchdog v2.0                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 23 fév 2008 11:39:41 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * print.c
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

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* begin_print: Prepare la pagination                                                                     */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Begin_print (GtkPrintOperation *operation,
                   GtkPrintContext   *context,
                   gpointer           user_data)    
  { guint nbr_msg, nbr_page;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;

    store  = gtk_tree_view_get_model ( user_data );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    nbr_msg = 0;
    while ( valide )                                                   /* Pour tous les objets du tableau */
     { valide = gtk_tree_model_iter_next( store, &iter );
       nbr_msg++;
     }
    nbr_page = 1 + nbr_msg / ((gtk_print_context_get_height(context)-2*PRINT_FONT_SIZE) / PRINT_FONT_SIZE);
    gtk_print_operation_set_n_pages ( operation, nbr_page );
  }
/**********************************************************************************************************/
/* New_print_job: Creer un job pour imprimer                                                              */
/* Entrée: Le nom du job                                                                                  */
/* Sortie: Le job                                                                                         */
/**********************************************************************************************************/
 GtkPrintOperation *New_print_job ( gchar *nom )
  { GtkPrintOperation *print;
    GtkPageSetup *default_page_setup;
    
    print = gtk_print_operation_new ();

    gtk_print_operation_set_unit (print, GTK_UNIT_PIXEL);
    gtk_print_operation_set_show_progress (print, TRUE);
    gtk_print_operation_set_job_name ( print, nom );

    default_page_setup = gtk_page_setup_new();
    gtk_page_setup_set_paper_size ( default_page_setup, gtk_paper_size_new (GTK_PAPER_NAME_A4) );
    gtk_page_setup_set_top_margin ( default_page_setup, 10.0, GTK_UNIT_MM );
    gtk_page_setup_set_bottom_margin ( default_page_setup, 10.0, GTK_UNIT_MM );
    gtk_page_setup_set_left_margin ( default_page_setup, 10.0, GTK_UNIT_MM );
    gtk_page_setup_set_right_margin ( default_page_setup, 10.0, GTK_UNIT_MM );
    gtk_page_setup_set_orientation ( default_page_setup, GTK_PAGE_ORIENTATION_LANDSCAPE );
    gtk_print_operation_set_default_page_setup ( print, default_page_setup );

    return( print );
  }
/*--------------------------------------------------------------------------------------------------------*/
