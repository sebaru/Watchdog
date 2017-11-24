/**********************************************************************************************************/
/* Client/liste_util.c        Configuration des utilisateurs de Watchdog v2.0                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 14:50:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_util.c
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
 
 #include "Reseaux.h"

 static GtkWidget *Liste_util;                        /* GtkTreeView pour la gestion des groupes Watchdog */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 enum
  {  COLONNE_ID,
     COLONNE_ACCESS_LEVEL,
     COLONNE_NOM,
     COLONNE_COMMENTAIRE,
     NBR_COLONNE
  };
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_utilisateur ( void );
 static void Menu_editer_utilisateur ( void );
 static void Menu_ajouter_utilisateur ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_utilisateur, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_utilisateur, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_utilisateur, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_utilisateur, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_END
  };
  
/**********************************************************************************************************/
/* CB_effacer_utilisateur: Fonction appelée qd on appuie sur un des boutons de l'interface                */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_utilisateur ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_UTILISATEUR rezo_util;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_util) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_util) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *nom;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_util.id, -1 );          /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_NOM, &nom, -1 );

               memcpy( &rezo_util.nom, nom, sizeof(rezo_util.nom) );
               g_free( nom );

               Envoi_serveur( TAG_UTILISATEUR, SSTAG_CLIENT_DEL_UTIL,
                              (gchar *)&rezo_util, sizeof(struct CMD_TYPE_UTILISATEUR) );
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
/* Menu_editer_utilisateur: Demande d'edition de l'utilisateur selectionné                                */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_utilisateur ( void )
  { Menu_ajouter_editer_utilisateur( NULL );
  }
/**********************************************************************************************************/
/* Menu_effacer_utilisateur: Retrait des utilisateurs selectionnés                                        */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_utilisateur ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_util) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                       /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d user%c ?"), nbr, (nbr>1 ? 's' : ' ') );
     
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_utilisateur), NULL );
    gtk_widget_show_all(dialog);
  }
/**********************************************************************************************************/
/* Menu_editer_utilisateur: Demande d'edition de l'utilisateur selectionné                                */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_utilisateur ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_UTILISATEUR rezo_util;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *nom;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_util) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_util) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_util.id, -1 );                     /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_NOM, &nom, -1 );
printf("Want edit user %s %d\n", nom, rezo_util.id );
    memcpy( &rezo_util.nom, nom, sizeof(rezo_util.nom) );
    g_free( nom );
    Envoi_serveur( TAG_UTILISATEUR, SSTAG_CLIENT_EDIT_UTIL,
                   (gchar *)&rezo_util, sizeof(struct CMD_TYPE_UTILISATEUR) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Gerer_popup_utilisateur: Gestion du menu popup quand on clique droite sur la liste des users           */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_utilisateur ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { static GtkWidget *Popup_select=NULL, *Popup_nonselect=NULL;
    GtkTreeSelection *selection;
    gboolean ya_selection;
    GtkTreePath *path;
    gint cellx, celly;
    if (!event) return(FALSE);

    if ( event->button == 3 )                                                         /* Gestion du popup */
     { if (!Popup_select)    Popup_select = gnome_popup_menu_new( Menu_popup_select );/*Creation si besoin*/
       if (!Popup_nonselect) Popup_nonselect = gnome_popup_menu_new( Menu_popup_nonselect );

       ya_selection = FALSE;
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_util) );  /* On recupere la selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_util), event->x, event->y,
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
     { Menu_editer_utilisateur(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_page_utilisateur: Creation de la page du notebook consacrée aux utilisateurs watchdog            */
/* Entrée: rien                                                                                           */
/* Sortie: un widget boite                                                                                */
/**********************************************************************************************************/
 void Creer_page_utilisateur( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_UTIL;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des utilisateurs **************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                         /* Id de l'utilisateur */
                                              G_TYPE_UINT,                                /* Access level */
                                              G_TYPE_STRING,                                       /* Nom */
                                              G_TYPE_STRING                                /* Commentaire */
                               );

    Liste_util = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );            /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_util) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_util );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne de l'id de l'utilisateur */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("UserId"), renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                     /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_util), colonne );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne de l'id de l'access level */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Access Level"), renderer,
                                                         "text", COLONNE_ACCESS_LEVEL,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ACCESS_LEVEL);           /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_util), colonne );

    renderer = gtk_cell_renderer_text_new();                           /* Colonne du nom de l'utilisateur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Username"), renderer,
                                                         "text", COLONNE_NOM,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NOM);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_util), colonne );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Comment"), renderer,
                                                         "text", COLONNE_COMMENTAIRE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_COMMENTAIRE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_util), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_util), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_util), TRUE );                   /* Pour faire beau */

    g_signal_connect( G_OBJECT(Liste_util), "button_press_event",                /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_utilisateur), NULL );
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
                              G_CALLBACK(Menu_editer_utilisateur), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_utilisateur), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_utilisateur), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Users") )  );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_util: Rafraichissement d'un utilisateur de la liste à l'écran                          */
/* Entrée: une reference sur l'utilisateur                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Rafraichir_visu_utilisateur( GtkTreeIter *iter, struct CMD_TYPE_UTILISATEUR *util )
  { GtkTreeModel *store;

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_util) );                /* Acquisition du modele */
printf(" new user : %d %s %s\n", util->id, util->nom, util->commentaire );

    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, util->id,
                         COLONNE_NOM, util->nom,
                         COLONNE_COMMENTAIRE, util->commentaire,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_groupe: Ajoute un groupe dans la liste des groupes                                         */
/* Entrée: une reference sur le groupe                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_utilisateur( struct CMD_TYPE_UTILISATEUR *util )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_UTIL)) Creer_page_utilisateur();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_util) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_utilisateur ( &iter, util );
  }
/**********************************************************************************************************/
/* Cacher_un_utilisateur: Enleve un groupe de la liste des utilisateurs                                   */
/* Entrée: une reference sur l'utilisateur                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_utilisateur( struct CMD_TYPE_UTILISATEUR *util )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_util) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == util->id )
        { printf("elimination util %s\n", util->nom );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_utilisateur: Rafraichissement de l'utilisateur en parametre                         */
/* Entrée: une reference sur l'utilisateur                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_utilisateur( struct CMD_TYPE_UTILISATEUR *util )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_util) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == util->id )
        { printf("maj utilisateur %s\n", util->nom );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_utilisateur( &iter, util ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
