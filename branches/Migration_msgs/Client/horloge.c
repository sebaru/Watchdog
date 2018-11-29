/******************************************************************************************************************************/
/* Client/horloge.c        Affichage des horloges en mode supervision                                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    13.08.2018 19:35:20 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * horloge.c
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
 
 #include "Reseaux.h"
 #include "Config_cli.h"
 #include "trame.h"

 extern GList *Liste_pages;                                                       /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                                             /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */

/********************************************* Définitions des prototypes programme *******************************************/
 #include "protocli.h"

 enum
  {  COLONNE_ID,
     COLONNE_HEURE,
     COLONNE_MINUTE,
     NBR_COLONNE
  };
  
/******************************************************************************************************************************/
/* Menu_ajouter_horloge: Ajout d'un horloge                                                                             */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_ajouter_horloge ( void )
  { struct PAGE_NOTEBOOK *page = Page_actuelle();
    struct TYPE_INFO_HORLOGE *infos;
    infos=page->infos;
    printf("%s: id=%d\n", __func__, infos->id_mnemo );
    if (page->type == TYPE_PAGE_HORLOGE) Menu_ajouter_editer_horloge(NULL, infos->id_mnemo);
  }
/******************************************************************************************************************************/
/* Menu_effacer_horloge: Retrait des horloges selectionnés                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_effacer_horloge ( void )
  { struct TYPE_INFO_HORLOGE *infos;
    struct PAGE_NOTEBOOK *page = Page_actuelle();
    struct CMD_TYPE_MNEMO_FULL mnemo;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;
    GtkWidget *dialog;
    guint nbr;

    infos = page->infos;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_horloge) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionné */

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(infos->Liste_horloge ) );
    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );

    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &mnemo.mnemo_horloge.id, -1 );                               /* Recup du id */
    mnemo.mnemo_base.id = infos->id_mnemo;
    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_DEL_HORLOGE, (gchar *)&mnemo, sizeof(struct CMD_TYPE_MNEMO_FULL) );
  }
/******************************************************************************************************************************/
/* Menu_editer_horloge: Demande d'edition du horloge selectionné                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_editer_horloge ( void )
  { GtkTreeSelection *selection;
    struct PAGE_NOTEBOOK *page = Page_actuelle();
    struct TYPE_INFO_HORLOGE *infos;
    struct CMD_TYPE_MNEMO_FULL mnemo;
    gint id_horloge;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    infos = page->infos;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_horloge) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(infos->Liste_horloge) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &mnemo.mnemo_horloge.id, -1 );                                           /* Recup du id */

    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_EDIT_HORLOGE, (gchar *)&mnemo, sizeof(struct CMD_TYPE_MNEMO_FULL) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                                               /* Liberation mémoire */
  }
/******************************************************************************************************************************/
/* Creer_page_horloge: Creation de la page du notebook consacrée aux horloges watchdog                                        */
/* Entrée: Le libelle a afficher dans le notebook et l'ID du  mnemo associé                                                   */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Creer_page_horloge ( gchar *libelle, guint id_mnemo )
  { GtkWidget *bouton, *boite, *hboite, *scroll, *frame, *label, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    struct TYPE_INFO_HORLOGE *infos;
    struct PAGE_NOTEBOOK *page;
    gchar titre[120];
    GtkListStore *store;

    printf("Creation page horloge %d-%s\n", id_mnemo, libelle );
    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->infos = (struct TYPE_INFO_HORLOGE *)g_try_malloc0( sizeof(struct TYPE_INFO_HORLOGE) );
    infos = (struct TYPE_INFO_HORLOGE *)page->infos;
    if (!page->infos) { g_free(page); return; }

    page->type   = TYPE_PAGE_HORLOGE;
    Liste_pages  = g_list_append( Liste_pages, page );
    infos->id_mnemo = id_mnemo;

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
/************************************************* La liste des horloges ***************************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                                              /* Id */
                                              G_TYPE_UINT,                                                           /* Heure */
                                              G_TYPE_UINT                                                           /* Minute */
                               );

    infos->Liste_horloge = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                      /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_horloge) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), infos->Liste_horloge );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de horloge */
    colonne = gtk_tree_view_column_new_with_attributes ( _("ID"), renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                                   /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_horloge), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de horloge */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Heure"), renderer,
                                                         "text", COLONNE_HEURE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_HEURE);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_horloge), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de horloge */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Minute"), renderer,
                                                         "text", COLONNE_MINUTE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_MINUTE);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_horloge), colonne );


/*    g_signal_connect( G_OBJECT(infos->Liste_horloge), "button_press_event",                       /* Gestion du menu popup */
  /*                    G_CALLBACK(Gerer_popup_horloge), page );*/
    g_object_unref (G_OBJECT (store));                                            /* nous n'avons plus besoin de notre modele */
    
/************************************************* Les boutons de controles ***************************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_OPEN );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_editer_horloge), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_horloge), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_horloge), NULL );

    gtk_widget_show_all( hboite );
    g_snprintf( titre, sizeof(titre), "Horloge %s", libelle );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( titre ) );
  }
/******************************************************************************************************************************/
/* Rafraichir_visu_horloge: Met à jour l'entrée horloge correspondante                                                        */
/* Entrée: une reference sur l'horloge                                                                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Rafraichir_visu_tick( GtkListStore *store, GtkTreeIter *iter, struct CMD_TYPE_MNEMO_FULL *mnemo )
  { gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, mnemo->mnemo_horloge.id,
                         COLONNE_HEURE, mnemo->mnemo_horloge.heure,
                         COLONNE_MINUTE, mnemo->mnemo_horloge.minute,
                         -1
                       );
  }
/******************************************************************************************************************************/
/* Afficher_une_horloge: Ajoute une horloge dans la liste des horloges du synoptiques                                         */
/* Entrée: une reference sur la page et sur l'horloge                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_tick( struct CMD_TYPE_MNEMO_FULL *mnemo )
  { struct PAGE_NOTEBOOK *page;
    struct TYPE_INFO_HORLOGE *infos;
    GtkListStore *store;
    GtkTreeIter iter;

    page = Chercher_page_notebook ( TYPE_PAGE_HORLOGE, mnemo->mnemo_base.id, TRUE );
    if (!page) return;

    infos = page->infos;
    if (!infos) return;

    printf(" print tick infos=%p\n", infos );
    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(infos->Liste_horloge) ));
    gtk_list_store_append ( store, &iter );                                                          /* Acquisition iterateur */
    Rafraichir_visu_tick ( store, &iter, mnemo );
  }
/******************************************************************************************************************************/
/* Cacher_un_mnemonique: Enleve un mnemonique de la liste des mnemoniques                                                     */
/* Entrée: une reference sur le mnemonique                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_cacher_un_tick( struct CMD_TYPE_MNEMO_FULL *mnemo )
  { struct TYPE_INFO_HORLOGE *infos;
    struct PAGE_NOTEBOOK *page;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    page = Chercher_page_notebook( TYPE_PAGE_HORLOGE, mnemo->mnemo_base.id, FALSE );
    if (!page) return;

    infos = page->infos;
    if (!infos) return;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(infos->Liste_horloge) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == mnemo->mnemo_horloge.id )
        { printf("elimination horloge %d\n", mnemo->mnemo_horloge.id );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/******************************************************************************************************************************/
/* Proto_rafrachir_un_mnemonique: Rafraichissement du mnemonique en parametre                                                 */
/* Entrée: une reference sur le mnemonique                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_rafraichir_un_tick( struct CMD_TYPE_MNEMO_FULL *mnemo )
  { struct TYPE_INFO_HORLOGE *infos;
    struct PAGE_NOTEBOOK *page;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    page = Chercher_page_notebook( TYPE_PAGE_HORLOGE, mnemo->mnemo_base.id, FALSE );
    if (!page) return;

    infos = page->infos;
    if (!infos) return;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(infos->Liste_horloge) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == mnemo->mnemo_horloge.id )
        { printf("maj horloge %d\n", mnemo->mnemo_horloge.id );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_tick( GTK_LIST_STORE(store), &iter, mnemo ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
