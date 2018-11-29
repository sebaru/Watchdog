/**********************************************************************************************************/
/* Client/liste_synoptique.c        Configuration des synoptiques de Watchdog v2.0                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 02 nov 2003 17:21:39 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_synoptique.c
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

 GtkWidget *Liste_synoptique;                     /* GtkTreeView pour la gestion des synoptiques Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 enum
  {  COLONNE_ID,
     COLONNE_GROUPE,
     COLONNE_PAGE,
     COLONNE_LIBELLE,
     NBR_COLONNE
  };
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_synoptique ( void );
 static void Menu_atelier_synoptique ( void );
 static void Menu_editer_synoptique ( void );
 static void Menu_ajouter_synoptique ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_synoptique, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Properties"), NULL, Menu_editer_synoptique, GNOME_STOCK_PIXMAP_PROPERTIES ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_atelier_synoptique, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_synoptique, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_synoptique, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_END
  };
/**********************************************************************************************************/
/* CB_effacer_synoptique: Fonction appelée qd on appuie sur un des boutons de l'interface                 */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_synoptique ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_SYNOPTIQUE rezo_synoptique;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_synoptique) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_synoptique) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_synoptique.id, -1 );       /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

               memcpy( &rezo_synoptique.libelle, libelle, sizeof(rezo_synoptique.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_SYNOPTIQUE, SSTAG_CLIENT_DEL_SYNOPTIQUE,
                             (gchar *)&rezo_synoptique, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
               gtk_tree_selection_unselect_iter( selection, &iter );
               lignes = lignes->next;
             }
            g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
            g_list_free (lignes);                                                   /* Liberation mémoire */
            break;
       default: break;
     }
    gtk_widget_destroy( GTK_WIDGET(dialog) );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Menu_ajouter_synoptique: Ajout d'un synoptique                                                         */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_synoptique ( void )
  { Menu_ajouter_editer_synoptique(NULL);
  }
/**********************************************************************************************************/
/* Menu_effacer_synoptique: Retrait des synoptiques selectionnés                                          */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_synoptique ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_synoptique) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                       /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d synoptique%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_synoptique), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_editer_synoptique: Demande d'edition du synoptique selectionné                                    */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_synoptique ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_SYNOPTIQUE rezo_synoptique;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_synoptique) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_synoptique) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_synoptique.id, -1 );               /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_synoptique.libelle, libelle, sizeof(rezo_synoptique.libelle) );
    g_free( libelle );

printf("on veut editer le synoptique num %d %s\n", rezo_synoptique.id, rezo_synoptique.libelle );
    Envoi_serveur( TAG_SYNOPTIQUE, SSTAG_CLIENT_EDIT_SYNOPTIQUE,
                  (gchar *)&rezo_synoptique, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Menu_atelier_synoptique: Edition physique d'un synoptique via l'atelier                                */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_atelier_synoptique ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_SYNOPTIQUE rezo_synoptique;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_synoptique) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_synoptique) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_synoptique.id, -1 );               /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */

    memcpy( &rezo_synoptique.libelle, libelle, sizeof(rezo_synoptique.libelle) );
    g_free( libelle );

    if (Chercher_page_notebook( TYPE_PAGE_ATELIER, rezo_synoptique.id, TRUE )) return;

printf("on veut editer(atelier) le synoptique %d, %s\n", rezo_synoptique.id, rezo_synoptique.libelle );
    Creer_page_atelier( rezo_synoptique.id, rezo_synoptique.libelle );
    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_SYNOPTIQUE,
                   (gchar *)&rezo_synoptique, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
  }
/**********************************************************************************************************/
/* Gerer_popup_synoptique: Gestion du menu popup quand on clique droite sur la liste des synoptiques      */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_synoptique ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { static GtkWidget *Popup_select=NULL, *Popup_nonselect=NULL;
    GtkTreeSelection *selection;
    gboolean ya_selection;
    GtkTreePath *path;
    gint cellx, celly;
    if (!event) return(FALSE);

    if ( event->button == 3 )                                                         /* Gestion du popup */
     { if (!Popup_select)    Popup_select = gnome_popup_menu_new( Menu_popup_select );
       if (!Popup_nonselect) Popup_nonselect = gnome_popup_menu_new( Menu_popup_nonselect );

       ya_selection = FALSE;
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_synoptique) );/* On recupere selection*/
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_synoptique), event->x, event->y,
                                          &path, NULL, &cellx, &celly );
          
          if (path)
           { gtk_tree_selection_select_path( selection, path );
             gtk_tree_path_free( path );
             ya_selection = TRUE;
           }
        } else ya_selection = TRUE;                              /* ya bel et bien qqchose de selectionné */

       gnome_popup_menu_do_popup_modal( (ya_selection ? Popup_select : Popup_nonselect),
                                        NULL, NULL, event, NULL, F_client );
       return(TRUE);
     }
    else if (event->type == GDK_2BUTTON_PRESS && event->button == 1 )                   /* Double clic ?? */
     { Menu_atelier_synoptique(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_page_synoptique: Creation de la page du notebook consacrée aux synoptiques watchdog              */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_synoptique( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_SYNOPTIQUE;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des synoptiques ***************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* Id */
                                              G_TYPE_STRING,                                    /* Groupe */
                                              G_TYPE_STRING,                                      /* Page */
                                              G_TYPE_STRING                                       /* Name */
                               );

    Liste_synoptique = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );      /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_synoptique) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_synoptique );

    renderer = gtk_cell_renderer_text_new();                             /* Colonne de l'id du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("SynId"), renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                     /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_synoptique), colonne );


    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Groupe"), renderer,
                                                         "text", COLONNE_GROUPE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_GROUPE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_synoptique), colonne );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Page"), renderer,
                                                         "text", COLONNE_PAGE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_PAGE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_synoptique), colonne );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne du libelle de synoptique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Synoptic Name"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_synoptique), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_synoptique), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_synoptique), TRUE );             /* Pour faire beau */

    g_signal_connect( G_OBJECT(Liste_synoptique), "button_press_event",          /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_synoptique), NULL );
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
    
/************************************ Les boutons de controles ********************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_PROPERTIES );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_editer_synoptique), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_synoptique), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
/*    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_synoptique), NULL );*/

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_synoptique), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Atelier") ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_synoptique: Rafraichissement d'un synoptique la liste à l'écran                        */
/* Entrée: une reference sur le synoptique                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_synoptique( GtkTreeIter *iter, struct CMD_TYPE_SYNOPTIQUE *synoptique )
  { GtkTreeModel *store;
       
    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_synoptique) );          /* Acquisition du modele */

    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, synoptique->id,
                         COLONNE_GROUPE, synoptique->parent_page,
                         COLONNE_PAGE, synoptique->page,
                         COLONNE_LIBELLE, synoptique->libelle,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_synoptique: Ajoute un synoptique dans la liste des synoptiques                             */
/* Entrée: une reference sur le synoptique                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_synoptique( struct CMD_TYPE_SYNOPTIQUE *synoptique )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_SYNOPTIQUE)) Creer_page_synoptique();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_synoptique) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_synoptique ( &iter, synoptique );
  }
/**********************************************************************************************************/
/* Cacher_un_synoptique: Enleve un synoptique de la liste des synoptiques                                 */
/* Entrée: une reference sur le synoptique                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_synoptique( struct CMD_TYPE_SYNOPTIQUE *synoptique )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_synoptique) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == synoptique->id )
        { printf("elimination synoptique %s\n", synoptique->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_synoptique: Rafraichissement du synoptique en parametre                             */
/* Entrée: une reference sur le synoptique                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_synoptique( struct CMD_TYPE_SYNOPTIQUE *synoptique )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_synoptique) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == synoptique->id )
        { printf("maj synoptique %s\n", synoptique->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_synoptique( &iter, synoptique ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
