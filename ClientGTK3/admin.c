/**********************************************************************************************************/
/* Client/admin.c        Administration du serveur Watchdog v2.0                                          */
/* Projet WatchDog version 3.0       Gestion d'habitat                     lun. 24 déc. 2012 13:08:00 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin.c
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

 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 static GtkWidget *Text_buffer;                                        /* Le Buffer de reponse du serveur */
 static GtkWidget *Entry_request;                                           /* Entry de request du client */

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* Menu_valider_request: Envoi une request d'admin au serveur                                             */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_valider_request ( void )
  { struct CMD_TYPE_ADMIN Admin;
    g_snprintf( Admin.buffer, sizeof(Admin.buffer),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_request) ) );
    Envoi_serveur( TAG_ADMIN, SSTAG_CLIENT_REQUEST,
                   (gchar *)&Admin, sizeof( struct CMD_TYPE_ADMIN ) );
    gtk_entry_set_text( GTK_ENTRY(Entry_request), "" );                           /* RAZ du entry request */
  }
/**********************************************************************************************************/
/* Creer_page_admin: Creation de la page du notebook consacrée aux admins watchdog                        */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_admin( void )
  { GtkWidget *boite, *hboite, *bouton, *separateur, *scroll;
    PangoFontDescription *font_desc;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_ADMIN;
    Liste_pages = g_list_append( Liste_pages, page );
printf("Creer_page_admin !\n");
    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des admins ***************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, TRUE, TRUE, 0 );

    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(boite), scroll, TRUE, TRUE, 0 );

    Text_buffer = gtk_text_view_new();
    gtk_text_view_set_editable ( GTK_TEXT_VIEW(Text_buffer), FALSE );
    gtk_text_view_set_cursor_visible ( GTK_TEXT_VIEW(Text_buffer), FALSE );
    font_desc = pango_font_description_from_string("Monospace 10");
    gtk_widget_modify_font( Text_buffer, font_desc );
    pango_font_description_free(font_desc);
    gtk_container_add( GTK_CONTAINER(scroll), Text_buffer );


    Entry_request = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_request), NBR_CARAC_BUFFER_ADMIN );
    g_signal_connect_swapped( G_OBJECT(Entry_request), "activate",
                              G_CALLBACK(Menu_valider_request), NULL );
    gtk_box_pack_start( GTK_BOX(boite), Entry_request, FALSE, FALSE, 0 );

/************************************ Les boutons de controles ********************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_EXECUTE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_valider_request), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Admin.") ) );
  }
/**********************************************************************************************************/
/* Afficher_un_admin: Ajoute un admin dans la liste des admins                                            */
/* Entrée: une reference sur le admin                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_admin( struct CMD_TYPE_ADMIN *admin )
  { GtkTextIter iter;
    GtkTextMark *tmp_textMark = NULL;

    if (!Tester_page_notebook(TYPE_PAGE_ADMIN)) Creer_page_admin();

    gtk_text_buffer_get_end_iter ( gtk_text_view_get_buffer (GTK_TEXT_VIEW(Text_buffer)), &iter );
    gtk_text_buffer_insert ( gtk_text_view_get_buffer (GTK_TEXT_VIEW(Text_buffer)), &iter,
                             admin->buffer, -1 );

    gtk_text_buffer_get_end_iter ( gtk_text_view_get_buffer (GTK_TEXT_VIEW(Text_buffer)), &iter );
    
    tmp_textMark = gtk_text_buffer_create_mark(gtk_text_view_get_buffer (GTK_TEXT_VIEW(Text_buffer)), NULL, &iter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(Text_buffer), tmp_textMark, 0, FALSE, 0, 0);
  }
/*--------------------------------------------------------------------------------------------------------*/
