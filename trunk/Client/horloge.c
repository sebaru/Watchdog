/******************************************************************************************************************************/
/* Client/horloge.c        Affichage des horloges en mode supervision                                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    13.08.2018 19:35:20 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * horloge.c
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
 #include <sys/time.h>
 
 #include "Reseaux.h"
 #include "Config_cli.h"
 #include "trame.h"

 extern GList *Liste_pages;                                                       /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                                             /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */

/********************************************* D�finitions des prototypes programme *******************************************/
 #include "protocli.h"

 enum
  {  COLONNE_ID,
     COLONNE_HEURE,
     COLONNE_MINUTE,
     NBR_COLONNE
  };
  
/******************************************************************************************************************************/
/* Rechercher_infos_supervision_par_id_syn: Recherche une page synoptique par son num�ro                                      */
/* Entr�e: Un num�ro de synoptique                                                                                            */
/* Sortie: Une r�f�rence sur les champs d'information de la page en question                                                  */
/******************************************************************************************************************************/
 struct TYPE_INFO_HORLOGE *Rechercher_infos_horloge_par_id_mnemo ( gint id_mnemo )
  { struct TYPE_INFO_HORLOGE *infos;
    struct PAGE_NOTEBOOK *page;
    GList *liste;
    liste = Liste_pages;
    infos = NULL;
    while( liste )
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if ( page->type == TYPE_PAGE_HORLOGE )                                            /* Est-ce bien une page d'horloge ?? */
        { infos = (struct TYPE_INFO_HORLOGE *)page->infos;
          if (infos->id_mnemo == id_mnemo) break;                                            /* Nous avons trouv� le mnemo !! */
        }
       liste = liste->next;                                                                            /* On passe au suivant */
     }
    return(infos);
  }
/******************************************************************************************************************************/
/* Menu_ajouter_horloge: Ajout d'un horloge                                                                             */
/* Entr�e: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_ajouter_horloge ( void )
  { struct PAGE_NOTEBOOK *page = Page_actuelle();
    struct TYPE_INFO_HORLOGE *infos;
    infos=page->infos;
    if (page->type == TYPE_PAGE_HORLOGE) Menu_ajouter_editer_horloge(NULL, infos->id_mnemo);
  }
/******************************************************************************************************************************/
/* Menu_effacer_horloge: Retrait des horloges selectionn�s                                                              */
/* Entr�e: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_effacer_horloge ( void )
  { struct TYPE_INFO_HORLOGE *infos;
    struct PAGE_NOTEBOOK *page = Page_actuelle();
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;
    GtkWidget *dialog;
    guint nbr, id;

    infos = page->infos;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_horloge) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionn� */

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(infos->Liste_horloge ) );
    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );

    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionn�e */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );                                                   /* Recup du id */

    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_DEL_HORLOGE, (gchar *)&id, sizeof(gint) );
  }
/******************************************************************************************************************************/
/* Menu_editer_horloge: Demande d'edition du horloge selectionn�                                                        */
/* Entr�e: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_editer_horloge ( void )
  { GtkTreeSelection *selection;
    struct PAGE_NOTEBOOK *page = Page_actuelle();
    struct TYPE_INFO_HORLOGE *infos;
    gint id_horloge;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    infos = page->infos;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_horloge) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(infos->Liste_horloge) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionn� */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionn�e */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &id_horloge, -1 );                                           /* Recup du id */

    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_EDIT_HORLOGE, (gchar *)&id_horloge, sizeof(gint) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                                               /* Liberation m�moire */
  }
/******************************************************************************************************************************/
/* Creer_page_horloge: Creation de la page du notebook consacr�e aux horloges watchdog                                        */
/* Entr�e: Le libelle a afficher dans le notebook et l'ID du  mnemo associ�                                                   */
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
                                              G_TYPE_STRING,                                                       /* Libelle */
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
                              G_CALLBACK(Menu_ajouter_editer_horloge), NULL );

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
/* Rafraichir_visu_horloge: Met � jour l'entr�e horloge correspondante                                                        */
/* Entr�e: une reference sur l'horloge                                                                                        */
/* Sortie: N�ant                                                                                                              */
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
/* Entr�e: une reference sur la page et sur l'horloge                                                                         */
/* Sortie: N�ant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_tick( struct TYPE_INFO_HORLOGE *infos, struct CMD_TYPE_MNEMO_FULL *mnemo )
  { GtkListStore *store;
    GtkTreeIter iter;
    printf(" print tick infos=%p\n", infos );
    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(infos->Liste_horloge) ));
    gtk_list_store_append ( store, &iter );                                                          /* Acquisition iterateur */
    Rafraichir_visu_tick ( store, &iter, mnemo );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
