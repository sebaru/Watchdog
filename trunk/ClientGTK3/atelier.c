/******************************************************************************************************************************/
/* Client/atelier.c        Edition d'un synoptique de Watchdog v2.0                                                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          dim 29 mar 2009 09:54:12 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier.c
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

 #include <gtk/gtk.h>
/****************************************** Définitions des prototypes programme **********************************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Detruire_page_atelier: Destruction d'une page atelier                                                                      */
/* Entrée: la page en question                                                                                                */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Detruire_page_atelier ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    json_node_unref( infos->syn );
    //Trame_detruire_trame( infos->Trame_atelier );

    gint num = gtk_notebook_page_num( GTK_NOTEBOOK(page->client->Notebook), GTK_WIDGET(page->child) );
    gtk_notebook_remove_page( GTK_NOTEBOOK(page->client->Notebook), num );
    page->client->Liste_pages = g_slist_remove( page->client->Liste_pages, page );
    g_free(infos);                                                                     /* Libération des infos le cas échéant */
    g_free(page);
  }
/******************************************************************************************************************************/
/* Menu_grille_magnetique: Active ou non la sensibilité du Check_Grid                                                         */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_grille_magnetique ( struct PAGE_NOTEBOOK *page  )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    gtk_widget_set_sensitive( infos->Spin_grid, gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(infos->Check_grid) ) );
  }
/******************************************************************************************************************************/
/* Menu_enregistrer_synoptique: Envoi des données au serveur                                                                  */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_enregistrer_synoptique ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    if ( !(page && page->infos) )
     { printf("Erreur parametre Menu_enregistrer_synoptique\n"); return; }

     Envoi_json_au_serveur_new ( page->client, "POST", infos->syn, "/api/syn/save", NULL );
  }
/******************************************************************************************************************************/
/* Changer_option_zoom: Change le niveau de zoom du canvas                                                                    */
/* Entrée: la page en question                                                                                                */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Changer_option_zoom (GtkRange *range, struct PAGE_NOTEBOOK *page  )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    GtkAdjustment *adj;
    g_object_get( infos->Option_zoom, "adjustment", &adj, NULL );
    goo_canvas_set_scale ( GOO_CANVAS(infos->Trame_atelier->trame_widget), gtk_adjustment_get_value(adj) );
  }
/******************************************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Creer_page_atelier_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { GtkWidget *bouton, *boite, *hboite, *vboite, *scroll, *texte, *spin, *boite1, *separateur;
    struct CLIENT *client = user_data;
    struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    GBytes *response_brute;
    gchar *reason_phrase;
    GtkAdjustment *adj;
    gint status_code;
    gsize taille;

    printf("%s\n", __func__ );
    g_object_get ( msg, "response-body-data", &response_brute, NULL );
  /*  printf("Recu SYNS: %s %p\n", (gchar *)g_bytes_get_data ( response_brute, &taille ), client );*/

    g_object_get ( msg, "status-code", &status_code, "reason-phrase", &reason_phrase, NULL );
    if (status_code != 200)
     { gchar chaine[256];
       g_snprintf(chaine, sizeof(chaine), "Error loading synoptique: Code %d - %s", status_code, reason_phrase );
       Log(client, chaine);
       return;
     }

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    page->client = client;

    infos = page->infos = (struct TYPE_INFO_ATELIER *)g_try_malloc0( sizeof(struct TYPE_INFO_ATELIER) );
    if (!page->infos) { g_free(page); return; }

    page->type           = TYPE_PAGE_ATELIER;
    client->Liste_pages  = g_slist_append( client->Liste_pages, page );
    g_object_get ( msg, "response-body-data", &response_brute, NULL );
    infos->syn = Json_get_from_string ( g_bytes_get_data ( response_brute, &taille ) );

    hboite = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
/******************************************************* L'atelier ************************************************************/
    vboite = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), vboite, TRUE, TRUE, 0 );

    GtkWidget *control_box = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 6 );
    gtk_box_pack_start( GTK_BOX(vboite), control_box, FALSE, FALSE, 0 );

    texte = gtk_label_new( "Position X/Y" );                                                        /* Affichage des abcisses */
    gtk_box_pack_start( GTK_BOX(control_box), texte, FALSE, FALSE, 0 );

    infos->Entry_posxy = gtk_entry_new();
    gtk_editable_set_editable ( GTK_EDITABLE(infos->Entry_posxy), FALSE );
    gtk_widget_set_tooltip_text ( infos->Entry_posxy, "Position de l'élément sélectionné" );
    gtk_box_pack_start( GTK_BOX(control_box), infos->Entry_posxy, FALSE, FALSE, 0 );

    texte = gtk_label_new( "Rotate" );
    gtk_box_pack_start( GTK_BOX(control_box), texte, FALSE, FALSE, 0 );

    infos->Adj_angle = gtk_adjustment_new( 0.0, -180.0, +180.0, 1.0, 10.0, 0.0 );
    spin = gtk_spin_button_new(infos->Adj_angle, 1.0, 0);
    gtk_widget_set_tooltip_text ( spin, "Angle de l'élément sélectionné" );
    gtk_box_pack_start( GTK_BOX(control_box), spin, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(infos->Adj_angle), "value_changed", G_CALLBACK(Rotationner_selection), page );

    texte = gtk_label_new( "Scale" );
    gtk_box_pack_start( GTK_BOX(control_box), texte, FALSE, FALSE, 0 );

    infos->Adj_scale = gtk_adjustment_new( 1.0, 0.1, 5.0, 0.1, 1.0, 0.0 );
    spin = gtk_spin_button_new(infos->Adj_scale, 1.0, 1);
    gtk_widget_set_tooltip_text ( spin, "Zoom de l'élément sélectionné" );
    gtk_box_pack_start( GTK_BOX(control_box), spin, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(infos->Adj_scale), "value_changed", G_CALLBACK(Zoomer_selection), page );

    texte = gtk_label_new( "Description" );
    gtk_box_pack_start( GTK_BOX(control_box), texte, FALSE, FALSE, 0 );
    infos->Entry_libelle = gtk_entry_new();
    gtk_editable_set_editable ( GTK_EDITABLE(infos->Entry_libelle), FALSE );
    gtk_widget_set_tooltip_text ( infos->Entry_libelle, "Description de l'élément sélectionné" );
    gtk_box_pack_start( GTK_BOX(control_box), infos->Entry_libelle, TRUE, TRUE, 0 );

/*************************************************** Trame proprement dite ****************************************************/

    boite1 = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );                                                              /* Barre de controle trame */
    gtk_box_pack_start( GTK_BOX(vboite), boite1, TRUE, TRUE, 0 );
    boite = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );                                                               /* Barre de controle trame */
    gtk_box_pack_start( GTK_BOX(boite1), boite, TRUE, TRUE, 0 );

    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start( GTK_BOX(boite), scroll, TRUE, TRUE, 0 );

    infos->Trame_atelier = Trame_creer_trame( page, TAILLE_SYNOPTIQUE_X, TAILLE_SYNOPTIQUE_Y, "darkgray", 20 );
    gtk_container_add( GTK_CONTAINER(scroll), infos->Trame_atelier->trame_widget );

  /*  g_signal_connect_swapped( G_OBJECT(infos->Trame_atelier->fond), "event",
                              G_CALLBACK(Clic_sur_fond), infos );*/
/*************************************************** Boutons de controle ******************************************************/
    boite = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_with_label( "Fermer" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "window-close", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Fermer la page" );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Detruire_page_atelier), page );

    bouton = gtk_button_new_with_label( "Sauvegarder" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "document-save", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Sauvegarder les modifications" );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_enregistrer_synoptique), page );

    separateur = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

/******************************************************** Zoom ****************************************************************/
    texte = gtk_label_new ( "Zoom" );
    gtk_box_pack_start( GTK_BOX(boite), texte, FALSE, FALSE, 0 );

    infos->Option_zoom = gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, 0.2, 5.0, 0.1 );
    gtk_box_pack_start( GTK_BOX(boite), infos->Option_zoom, FALSE, FALSE, 0 );
    g_object_get( infos->Option_zoom, "adjustment", &adj, NULL );
    gtk_adjustment_set_value( adj, 1.0 );
    g_signal_connect( G_OBJECT( infos->Option_zoom ), "value-changed", G_CALLBACK( Changer_option_zoom ), page );

    separateur = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    texte = gtk_label_new ( "Grille" );
    gtk_box_pack_start( GTK_BOX(boite), texte, FALSE, FALSE, 0 );

    infos->Check_grid = gtk_check_button_new_with_label( "Aimantée" );
    gtk_box_pack_start( GTK_BOX(boite), infos->Check_grid, FALSE, FALSE, 0 );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(infos->Check_grid), TRUE );
    gtk_widget_set_tooltip_text ( infos->Check_grid, "Activer ou non la\ngrille magnétique" );
    g_signal_connect_swapped( G_OBJECT(infos->Check_grid), "toggled", G_CALLBACK(Menu_grille_magnetique), page );

    infos->Spin_grid = gtk_spin_button_new_with_range( 1.0, 20.0, 5.0 );
    gtk_box_pack_start( GTK_BOX(boite), infos->Spin_grid, FALSE, FALSE, 0 );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(infos->Spin_grid), 10.0 );
    gtk_widget_set_tooltip_text ( infos->Spin_grid, "Taille des maillons d'aimantage" );
    Menu_grille_magnetique( page );

    separateur = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

/***************************************************** Ajout de motifs ********************************************************/
    texte = gtk_label_new ( "Ajouts" );
    gtk_box_pack_start( GTK_BOX(boite), texte, FALSE, FALSE, 0 );

    bouton = gtk_button_new_with_label( "Motifs" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "list-add", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Ajouter un motif dans la page" );
    //g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_ajouter_motif), page );
/******************************************************** fin *****************************************************************/
    gtk_widget_show_all( page->child );
    gchar titre[80];
    g_snprintf( titre, sizeof(titre), "Atelier/%s", Json_get_string( infos->syn, "libelle" ) );
    gint page_num = gtk_notebook_append_page( GTK_NOTEBOOK(client->Notebook), page->child, gtk_label_new ( titre ) );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK(client->Notebook), page_num );

    Json_node_foreach_array_element ( infos->syn, "visuels",     Afficher_un_motif, page );
    Json_node_foreach_array_element ( infos->syn, "passerelles", Afficher_une_passerelle, page );
    Json_node_foreach_array_element ( infos->syn, "comments",    Afficher_un_commentaire, page );
    Json_node_foreach_array_element ( infos->syn, "cameras",     Afficher_une_camera, page );
    Json_node_foreach_array_element ( infos->syn, "cadrans",     Afficher_un_cadran, page );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
