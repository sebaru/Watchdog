/**********************************************************************************************************/
/* Client/liste_scenario.c        Configuration des scenarios de Watchdog v2.0                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 10 aoû 2008 11:13:33 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_scenario.c
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

 GtkWidget *Liste_scenario;                           /* GtkTreeView pour la gestion des scenarios Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

 enum
  {  COLONNE_ID,
     COLONNE_ACTIF,
     COLONNE_BITM,
     COLONNE_TS_JOUR,
     COLONNE_TS_MOIS,
     COLONNE_HEURE,
     COLONNE_MINUTE,
     COLONNE_LUNDI,
     COLONNE_MARDI,
     COLONNE_MERCREDI,
     COLONNE_JEUDI,
     COLONNE_VENDREDI,
     COLONNE_SAMEDI,
     COLONNE_DIMANCHE,
     COLONNE_JANVIER,
     COLONNE_FEVRIER,
     COLONNE_MARS,
     COLONNE_AVRIL,
     COLONNE_MAI,
     COLONNE_JUIN,
     COLONNE_JUILLET,
     COLONNE_AOUT,
     COLONNE_SEPTEMBRE,
     COLONNE_OCTOBRE,
     COLONNE_NOVEMBRE,
     COLONNE_DECEMBRE,
     COLONNE_LIBELLE,
     NBR_COLONNE
  };
 extern GdkColor COULEUR_FOND[];
 extern GdkColor COULEUR_TEXTE[];
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_scenario ( void );
 static void Menu_editer_scenario ( void );
 static void Menu_ajouter_scenario ( void );
 static void Menu_exporter_scenario ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_scenario, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_scenario, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Export"), NULL, Menu_exporter_scenario, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_scenario, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_scenario, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_exporter_scenario, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_END
  };

/**********************************************************************************************************/
/* CB_effacer_scenario: Fonction appelée qd on appuie sur un des boutons de l'interface                    */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_scenario ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_SCENARIO rezo_scenario;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_scenario) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_scenario) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_scenario.id, -1 );       /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

               memcpy( &rezo_scenario.libelle, libelle, sizeof(rezo_scenario.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_SCENARIO, SSTAG_CLIENT_DEL_SCENARIO,
                             (gchar *)&rezo_scenario, sizeof(struct CMD_TYPE_SCENARIO) );
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
/* Menu_ajouter_scenario: Ajout d'un scenario                                                               */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_scenario ( void )
  { Menu_ajouter_editer_scenario(NULL); }
/**********************************************************************************************************/
/* Menu_effacer_scenario: Retrait des scenarios selectionnés                                                */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_scenario ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_scenario) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    printf("Menu effacer scenario: nbr=%d\n", nbr );
    if (!nbr) return;                                                       /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d scenario%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_scenario), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_editer_scenario: Demande d'edition du scenario selectionné                                          */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_scenario ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_SCENARIO rezo_scenario;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_scenario) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_scenario) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_scenario.id, -1 );                  /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_scenario.libelle, libelle, sizeof(rezo_scenario.libelle) );
    g_free( libelle );
    Envoi_serveur( TAG_SCENARIO, SSTAG_CLIENT_EDIT_SCENARIO,
                  (gchar *)&rezo_scenario, sizeof(struct CMD_TYPE_SCENARIO) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* draw_page: Dessine une page pour l'envoyer sur l'imprimante                                            */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void draw_page (GtkPrintOperation *operation,
                        GtkPrintContext   *context,
                        gint               page_nr,
                        GtkTreeIter *iter)
  { GtkTreeModel *store;
    gboolean valide;
    cairo_t *cr;
    gdouble y;
    
    printf("Page_nr = %d\n", page_nr );
  
    cr = gtk_print_context_get_cairo_context (context);
  
    cairo_select_font_face (cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, PRINT_FONT_SIZE );

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_scenario) );
    valide = TRUE;
    y = 0.0;
    while ( valide && y<gtk_print_context_get_height (context) )      /* Pour tous les objets du tableau */
     { /*gtk_tree_model_get( store, iter, COLONNE_NOTINHIB, &enable, COLONNE_NUM, &num,
                           COLONNE_SMS, &sms,
                           COLONNE_TYPE_INT, &type_int, COLONNE_TYPE_STRING, &type_string,
                           COLONNE_OBJET, &objet, COLONNE_LIBELLE, &libelle, -1 );

       cairo_move_to( cr, 0.0*PRINT_FONT_SIZE, y );
       if (enable) { cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
                     cairo_show_text (cr, _("ON") );
                   }
       else        { cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
                     cairo_show_text (cr, _("OFF") );
                   }

       if (sms)    { cairo_move_to( cr, 3.0*PRINT_FONT_SIZE, y );
                     cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
                     cairo_show_text (cr, _("SMS") );
                   }

       cairo_move_to( cr, 6.0*PRINT_FONT_SIZE, y );
       cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
       cairo_show_text (cr, num );

       cairo_set_source_rgb (cr, (gdouble)COULEUR_FOND[type_int].red / 0xFFFF,
                                 (gdouble)COULEUR_FOND[type_int].green / 0xFFFF,
                                 (gdouble)COULEUR_FOND[type_int].blue / 0xFFFF );
       cairo_rectangle (cr, 11.0*PRINT_FONT_SIZE, y+1-PRINT_FONT_SIZE, 5.0*PRINT_FONT_SIZE, PRINT_FONT_SIZE);
       cairo_fill (cr);
  
       cairo_move_to( cr, 11.0*PRINT_FONT_SIZE, y );
       cairo_set_source_rgb (cr, (gdouble)COULEUR_TEXTE[type_int].red / 0xFFFF,
                                 (gdouble)COULEUR_TEXTE[type_int].green / 0xFFFF,
                                 (gdouble)COULEUR_TEXTE[type_int].blue / 0xFFFF );
       cairo_show_text (cr, type_string );

       cairo_move_to( cr, 17.0*PRINT_FONT_SIZE, y );
       cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
       cairo_show_text (cr, objet );

       cairo_move_to( cr, 33.0*PRINT_FONT_SIZE, y );
       cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
       cairo_show_text (cr, libelle );

       g_free(num);
       g_free(type_string);
       g_free(objet);
       g_free(libelle);*/

       valide = gtk_tree_model_iter_next( store, iter );
       y += PRINT_FONT_SIZE;
     }
  }
/**********************************************************************************************************/
/* Menu_exporter_scenario: Exportation de la base dans un fichier texte                                    */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Menu_exporter_scenario( void )
  { static GtkTreeIter iter;
    GtkPrintOperation *print;
    GtkPrintOperationResult res;
    GtkTreeModel *store;
    gboolean valide;
    GError *error;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_scenario) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    if (!valide) return;

    print = New_print_job ( "Print Scenario" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (draw_page), &iter );
    g_signal_connect (G_OBJECT(print), "begin-print",
                      G_CALLBACK (Begin_print), GTK_TREE_VIEW(Liste_scenario) );

    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   GTK_WINDOW(F_client), &error);
  }
/**********************************************************************************************************/
/* Gerer_popup_scenario: Gestion du menu popup quand on clique droite sur la liste des scenarios            */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_scenario ( GtkWidget *widget, GdkEventButton *event, gpointer data )
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
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_scenario) );  /* On recupere selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_scenario), event->x, event->y,
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
     { Menu_editer_scenario(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_page_scenario: Creation de la page du notebook consacrée aux scenarios watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_scenario( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_SCENARIO;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des scenarios ******************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* Id */
                                              G_TYPE_BOOLEAN,                                    /* actif */
                                              G_TYPE_UINT,                                       /* bit_m */
                                              G_TYPE_BOOLEAN,                                  /* TS_JOUR */
                                              G_TYPE_BOOLEAN,                                  /* TS_MOIS */
                                              G_TYPE_UINT,                                       /* heure */
                                              G_TYPE_UINT,                                      /* minute */
                                              G_TYPE_BOOLEAN,                                    /* lundi */
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,                                  /* janvier */
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_BOOLEAN,
                                              G_TYPE_STRING                                    /* Libelle */
                               );

    Liste_scenario = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );        /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_scenario) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_scenario );

    renderer = gtk_cell_renderer_toggle_new();                            /* Colonne de l'id du scenario */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Enable"), renderer,
                                                         "active", COLONNE_ACTIF,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ACTIF);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_scenario), colonne );

    renderer = gtk_cell_renderer_text_new();                             /* Colonne du libelle de scenario */
    colonne = gtk_tree_view_column_new_with_attributes ( _("ScenarioId"), renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_scenario), colonne );


    renderer = gtk_cell_renderer_text_new();                             /* Colonne du libelle de scenario */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Scenario"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_scenario), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_scenario), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_scenario), TRUE );                /* Pour faire beau */

    g_signal_connect( G_OBJECT(Liste_scenario), "button_press_event",             /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_scenario), NULL );
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
                              G_CALLBACK(Menu_editer_scenario), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_scenario), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_scenario), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_scenario), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Edit Scenario") ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_scenario: Rafraichissement d'un scenario la liste à l'écran                              */
/* Entrée: une reference sur le scenario                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_scenario( GtkTreeIter *iter, struct CMD_TYPE_SCENARIO *scenario )
  { GtkTreeModel *store;
    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_scenario) );             /* Acquisition du modele */
printf("Rafraichir visu_scenario %d\n", scenario->id);
    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, scenario->id,
                         COLONNE_ACTIF, scenario->actif,
                         COLONNE_BITM, scenario->bit_m,
                         COLONNE_TS_JOUR, scenario->ts_jour,
                         COLONNE_TS_MOIS, scenario->ts_mois,
                         COLONNE_HEURE, scenario->heure,
                         COLONNE_MINUTE, scenario->minute,
                         COLONNE_LIBELLE, scenario->libelle,
                         COLONNE_LUNDI, scenario->lundi,
                         COLONNE_MARDI, scenario->mardi,
                         COLONNE_MERCREDI, scenario->mercredi,
                         COLONNE_JEUDI, scenario->jeudi,
                         COLONNE_VENDREDI, scenario->vendredi,
                         COLONNE_SAMEDI, scenario->samedi,
                         COLONNE_DIMANCHE, scenario->dimanche,
                         COLONNE_JANVIER, scenario->janvier,
                         COLONNE_FEVRIER, scenario->fevrier,
                         COLONNE_MARS, scenario->mars,
                         COLONNE_AVRIL, scenario->avril,
                         COLONNE_MAI, scenario->mai,
                         COLONNE_JUIN, scenario->juin,
                         COLONNE_JUILLET, scenario->juillet,
                         COLONNE_AOUT, scenario->aout,
                         COLONNE_SEPTEMBRE, scenario->septembre,
                         COLONNE_OCTOBRE, scenario->octobre,
                         COLONNE_NOVEMBRE, scenario->novembre,
                         COLONNE_DECEMBRE, scenario->decembre,
                         -1
                       );
printf("Rafraichir visu_scenario fin\n");
  }
/**********************************************************************************************************/
/* Afficher_un_scenario: Ajoute un scenario dans la liste des scenarios                                   */
/* Entrée: une reference sur le scenario                                                                  */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_scenario( struct CMD_TYPE_SCENARIO *scenario )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_SCENARIO)) Creer_page_scenario();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_scenario) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_scenario ( &iter, scenario );
  }
/**********************************************************************************************************/
/* Cacher_un_scenario: Enleve un scenario de la liste des scenarios                                       */
/* Entrée: une reference sur le scenario                                                                  */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_scenario( struct CMD_TYPE_SCENARIO *scenario )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_scenario) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == scenario->id )
        { printf("elimination scenario %s\n", scenario->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_scenario: Rafraichissement du scenario en parametre                                 */
/* Entrée: une reference sur le scenario                                                                  */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_scenario( struct CMD_TYPE_SCENARIO *scenario )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_scenario) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == scenario->id )
        { printf("maj scenario %s\n", scenario->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_scenario( &iter, scenario ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
