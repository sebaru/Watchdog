/**********************************************************************************************************/
/* Client/liste_entreeANA.c        Configuration des entreeANAs de Watchdog v2.0                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 25 jun 2004 15:04:52 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_entreeana.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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

 static GtkWidget *Liste_entreeANA;                /* GtkTreeView pour la gestion des entreeANAs Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

 enum
  {  COLONNE_ID,
     COLONNE_NUM,
     COLONNE_TYPE,
     COLONNE_MIN,
     COLONNE_MAX,
     COLONNE_UNITE,
     COLONNE_LIBELLE,
     NBR_COLONNE
  };
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_editer_entreeANA ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_entreeANA, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_END
  };
/**********************************************************************************************************/
/* Menu_editer_entreeANA: Demande d'edition du entreeANA selectionné                                      */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_entreeANA ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_ENTREEANA rezo_entreeANA;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_entreeANA) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_entreeANA) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_entreeANA.id_mnemo, -1 );          /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_entreeANA.libelle, libelle, sizeof(rezo_entreeANA.libelle) );
    g_free( libelle );
printf("on veut editer le entreeANA %s %d\n", rezo_entreeANA.libelle, rezo_entreeANA.id_mnemo );
    Envoi_serveur( TAG_ENTREEANA, SSTAG_CLIENT_EDIT_ENTREEANA,
                  (gchar *)&rezo_entreeANA, sizeof(struct CMD_TYPE_ENTREEANA) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Gerer_popup_entreeANA: Gestion du menu popup quand on clique droite sur la liste des entreeANAs        */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_entreeANA ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { static GtkWidget *Popup_select=NULL;
    GtkTreeSelection *selection;
    gboolean ya_selection;
    GtkTreePath *path;
    gint cellx, celly;
    if (!event) return(FALSE);

    if ( event->button == 3 )                                                         /* Gestion du popup */
     { if (!Popup_select)    Popup_select = gnome_popup_menu_new( Menu_popup_select );

       ya_selection = FALSE;
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_entreeANA) );/* On recupere selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_entreeANA), event->x, event->y,
                                          &path, NULL, &cellx, &celly );
          
          if (path)
           { gtk_tree_selection_select_path( selection, path );
             gtk_tree_path_free( path );
             ya_selection = TRUE;
           }
        } else ya_selection = TRUE;                              /* ya bel et bien qqchose de selectionné */

       if (ya_selection) gnome_popup_menu_do_popup_modal( Popup_select, NULL, NULL, event, NULL, F_client );
       return(TRUE);
     }
    else if (event->type == GDK_2BUTTON_PRESS && event->button == 1 )                   /* Double clic ?? */
     { Menu_editer_entreeANA(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_page_entreeANA: Creation de la page du notebook consacrée aux entreeANAs watchdog                */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_entreeANA( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_ENTREEANA;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des entreeANAs ****************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* id */
                                              G_TYPE_STRING,                                       /* Num */
                                              G_TYPE_STRING,                               /* type_string */
                                              G_TYPE_DOUBLE,                                       /* min */
                                              G_TYPE_DOUBLE,                                       /* max */
                                              G_TYPE_STRING,                                     /* Unite */
                                              G_TYPE_STRING                                    /* libelle */
                               );

    Liste_entreeANA = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );       /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_entreeANA) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_entreeANA );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne de l'id du entreeANA */
    colonne = gtk_tree_view_column_new_with_attributes ( _("EA"), renderer,
                                                         "text", COLONNE_NUM,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NUM);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_entreeANA), colonne );

    renderer = gtk_cell_renderer_text_new();                           /* Colonne du libelle de entreeANA */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Type"), renderer,
                                                         "text", COLONNE_TYPE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_TYPE);                   /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_entreeANA), colonne );

    renderer = gtk_cell_renderer_text_new();                           /* Colonne du libelle de entreeANA */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Min"), renderer,
                                                         "text", COLONNE_MIN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_MIN);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_entreeANA), colonne );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Max"), renderer,
                                                         "text", COLONNE_MAX,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_MAX);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_entreeANA), colonne );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Unit"), renderer,
                                                         "text", COLONNE_UNITE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_UNITE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_entreeANA), colonne );

    renderer = gtk_cell_renderer_text_new();                           /* Colonne du libelle de entreeANA */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Description"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_entreeANA), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_entreeANA), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_entreeANA), TRUE );              /* Pour faire beau */

    g_signal_connect( G_OBJECT(Liste_entreeANA), "button_press_event",           /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_entreeANA), NULL );
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

    bouton = gtk_button_new_from_stock( GTK_STOCK_OPEN );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_editer_entreeANA), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Edit EA") ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_entreeANA: Rafraichissement d'un entreeANA la liste à l'écran                          */
/* Entrée: une reference sur le entreeANA                                                                 */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_entreeANA( GtkTreeIter *iter, struct CMD_TYPE_ENTREEANA *entreeANA )
  { GtkTreeModel *store;
    gchar chaine[20];
       
    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_entreeANA) );           /* Acquisition du modele */

    printf("maj entreeANA %s unite %d %s id_mnemo %d\n", entreeANA->libelle,
           entreeANA->unite, Unite_vers_string(entreeANA->unite), entreeANA->id_mnemo );
          
    g_snprintf( chaine, sizeof(chaine), "%s%04d", Type_bit_interne_court(MNEMO_ENTREE_ANA), entreeANA->num );
    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, entreeANA->id_mnemo,
                         COLONNE_NUM, chaine,
                         COLONNE_TYPE, Type_ea_vers_string(entreeANA->type),
                         COLONNE_MIN, entreeANA->min,
                         COLONNE_MAX, entreeANA->max,
                         COLONNE_UNITE, Unite_vers_string(entreeANA->unite),
                         COLONNE_LIBELLE, entreeANA->libelle,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_entreeANA: Ajoute un entreeANA dans la liste des entreeANAs                                */
/* Entrée: une reference sur le entreeANA                                                                 */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_une_entreeANA( struct CMD_TYPE_ENTREEANA *entreeANA )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_ENTREEANA)) Creer_page_entreeANA();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_entreeANA) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_entreeANA ( &iter, entreeANA );
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_entreeANA: Rafraichissement du entreeANA en parametre                               */
/* Entrée: une reference sur le entreeANA                                                                 */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_une_entreeANA( struct CMD_TYPE_ENTREEANA *entreeANA )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_entreeANA) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == entreeANA->id_mnemo ) break;

       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_entreeANA( &iter, entreeANA ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
