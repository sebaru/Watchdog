/**********************************************************************************************************/
/* Client/print.c        Configuration des impressions de Watchdog v2.0                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                    jeu. 28 août 2014 09:04:16 CEST */
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
/* draw_page: CB de dessin de la page nbr_page                                                            */
/* Entrée: le composeur                                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Print_draw_page ( GtkPrintOperation *operation,
                  GtkPrintContext   *context,
                  gint               page_nr,
                  gpointer           user_data )
  { GtkSourcePrintCompositor *compositor;
    compositor = GTK_SOURCE_PRINT_COMPOSITOR (user_data);
    gtk_source_print_compositor_draw_page ( compositor, context, page_nr );
  }
/**********************************************************************************************************/
/* begin_print: Prepare la pagination                                                                     */
/* Entrée: Prepare la pagination avec le composeur                                                        */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 gboolean Print_paginate ( GtkPrintOperation *operation,
                              GtkPrintContext   *context,
                              gpointer           user_data ) 
  { GtkSourcePrintCompositor *compositor;
    compositor = GTK_SOURCE_PRINT_COMPOSITOR (user_data);
    if (gtk_source_print_compositor_paginate (compositor, context))
     { gint n_pages;
       n_pages = gtk_source_print_compositor_get_n_pages (compositor);
printf("Print_paginate -> number page = %d\n", n_pages);
       gtk_print_operation_set_n_pages (operation, n_pages);
       return TRUE;
     }
    return FALSE;
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
