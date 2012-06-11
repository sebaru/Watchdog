/**********************************************************************************************************/
/* Client/liste_rfxcom.c        Configuration des rfxcoms de Watchdog v2.0                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                    lun. 11 juin 2012 19:29:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_rfxcom.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - Sébastien Lefevre
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

 static GtkWidget *Liste_rfxcom;                      /* GtkTreeView pour la gestion des rfxcoms Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

 enum                   /* Numéro des colonnes dans les listes CAM (liste_camera et atelier_ajout_camera) */
  {  COLONNE_ID,
     COLONNE_TYPE,
     COLONNE_CANAL,
     COLONNE_E_MIN,
     COLONNE_EA_MIN,
     COLONNE_A_MIN,
     COLONNE_LIBELLE,
     NBR_COLONNE
  };
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_rfxcom ( void );
 static void Menu_editer_rfxcom ( void );
 static void Menu_ajouter_rfxcom ( void );
 static void Menu_exporter_rfxcom ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_rfxcom, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_rfxcom, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Export"), NULL, Menu_exporter_rfxcom, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_rfxcom, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_rfxcom, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Print"), NULL, Menu_exporter_rfxcom, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_END
  };

/**********************************************************************************************************/
/* CB_effacer_rfxcom: Fonction appelée qd on appuie sur un des boutons de l'interface                     */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_rfxcom ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_RFXCOM rezo_rfxcom;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_rfxcom) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_rfxcom) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_rfxcom.id, -1 );        /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

               memcpy( &rezo_rfxcom.libelle, libelle, sizeof(rezo_rfxcom.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_RFXCOM, SSTAG_CLIENT_DEL_RFXCOM,
                             (gchar *)&rezo_rfxcom, sizeof(struct CMD_TYPE_RFXCOM) );
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
/* Menu_ajouter_rfxcom: Ajout d'un rfxcom                                                                 */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_rfxcom ( void )
  {/* Menu_ajouter_editer_rfxcom(NULL); */}
/**********************************************************************************************************/
/* Menu_effacer_rfxcom: Retrait des rfxcoms selectionnés                                                  */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_rfxcom ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_rfxcom) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    printf("Menu effacer rfxcom: nbr=%d\n", nbr );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d rfxcom%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_rfxcom), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_editer_rfxcom: Demande d'edition du rfxcom selectionné                                            */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_rfxcom ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_RFXCOM rezo_rfxcom;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_rfxcom) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_rfxcom) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_rfxcom.id, -1 );                   /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_rfxcom.libelle, libelle, sizeof(rezo_rfxcom.libelle) );
    g_free( libelle );
    Envoi_serveur( TAG_RFXCOM, SSTAG_CLIENT_EDIT_RFXCOM,
                  (gchar *)&rezo_rfxcom, sizeof(struct CMD_TYPE_RFXCOM) );
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
  { 
#ifdef bouh
    gchar *num, *type_string, *objet, *libelle, *date_create, titre[128], chaine[128];
    guint enable, type_int, sms;
    GtkTreeModel *store;
    gboolean valide;
    struct tm *temps;
    time_t timet;
    cairo_t *cr;
    gdouble y;
    printf("Page_nr = %d\n", page_nr );
  
    cr = gtk_print_context_get_cairo_context (context);
  
    cairo_select_font_face (cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 20.0 );

    cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);

    timet = time(NULL);
    temps = localtime( &timet );
    if (temps) { strftime( chaine, sizeof(chaine), "%F %T", temps ); }
    else       { g_snprintf( chaine, sizeof(chaine), _("Erreur") ); }

    date_create = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
    g_snprintf( titre, sizeof(titre), " Watchdog - Messages - %s - Page %d", date_create, page_nr );
    g_free( date_create );

    cairo_move_to( cr, 0.0, 0.0 );
    cairo_show_text (cr, titre );

    cairo_set_font_size (cr, PRINT_FONT_SIZE );
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_rfxcom) );
    valide = TRUE;
    y = 2 * PRINT_FONT_SIZE;
    while ( valide && y<gtk_print_context_get_height (context) )       /* Pour tous les objets du tableau */
     { gtk_tree_model_get( store, iter, COLONNE_NOTINHIB, &enable, COLONNE_NUM, &num,
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

       cairo_move_to( cr, 17.0*PRINT_FONT_SIZE, y );   /* Objet */
       cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
       cairo_show_text (cr, objet );

       cairo_move_to( cr, 38.0*PRINT_FONT_SIZE, y );  /* Libelle */
       cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
       cairo_show_text (cr, libelle );

       g_free(num);
       g_free(type_string);
       g_free(objet);
       g_free(libelle);

       valide = gtk_tree_model_iter_next( store, iter );
       y += PRINT_FONT_SIZE;
     }
#endif
  }
/**********************************************************************************************************/
/* Menu_exporter_rfxcom: Exportation de la base dans un fichier texte                                     */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Menu_exporter_rfxcom( void )
  { static GtkTreeIter iter;
    GtkPrintOperation *print;
    GtkPrintOperationResult res;
    GtkTreeModel *store;
    gboolean valide;
    GError *error;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_rfxcom) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    if (!valide) return;

    print = New_print_job ( "Print Cameras" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (draw_page), &iter );
    g_signal_connect (G_OBJECT(print), "begin-print",
                      G_CALLBACK (Begin_print), GTK_TREE_VIEW(Liste_rfxcom) );

    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   GTK_WINDOW(F_client), &error);
  }
/**********************************************************************************************************/
/* Gerer_popup_rfxcom: Gestion du menu popup quand on clique droite sur la liste des rfxcoms              */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_rfxcom ( GtkWidget *widget, GdkEventButton *event, gpointer data )
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
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_rfxcom) );   /* On recupere selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_rfxcom), event->x, event->y,
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
     { Menu_editer_rfxcom(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_liste_rfxcom: Creation de la liste du notebook consacrée aux rfxcoms watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_liste_rfxcom( GtkWidget **Liste, GtkWidget **Scroll )
  { GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    GtkWidget *scroll, *liste;

    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    *Scroll = scroll;

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* Id */
                                              G_TYPE_UINT,                                        /* type */
                                              G_TYPE_UINT,                                       /* canal */
                                              G_TYPE_INT,                                        /* E_MIN */
                                              G_TYPE_INT,                                       /* EA_MIN */
                                              G_TYPE_INT,                                        /* A_MIN */
                                              G_TYPE_STRING                                    /* libelle */
                               );

    liste = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                 /* Creation de la vue */
    *Liste = liste;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(liste) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), liste );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de rfxcom */
    g_object_set ( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("ID"), renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                     /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de rfxcom */
    g_object_set ( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Type"), renderer,
                                                         "text", COLONNE_TYPE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_TYPE);                   /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de rfxcom */
    g_object_set ( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Canal"), renderer,
                                                         "text", COLONNE_CANAL,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_CANAL);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                               /* Colonne du libelle de rfxcom */
    colonne = gtk_tree_view_column_new_with_attributes ( _("E min"), renderer,
                                                         "text", COLONNE_E_MIN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_E_MIN);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                               /* Colonne du libelle de rfxcom */
    colonne = gtk_tree_view_column_new_with_attributes ( _("EA min"), renderer,
                                                         "text", COLONNE_EA_MIN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_EA_MIN);                 /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                               /* Colonne du libelle de rfxcom */
    colonne = gtk_tree_view_column_new_with_attributes ( _("A min"), renderer,
                                                         "text", COLONNE_A_MIN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_A_MIN);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de rfxcom */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Libelle"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_rfxcom), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(liste), TRUE );                        /* Pour faire beau */
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
  }
/**********************************************************************************************************/
/* Creer_page_rfxcom: Creation de la page du notebook consacrée aux rfxcoms watchdog                      */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_rfxcom( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_RFXCOM;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des rfxcoms *******************************************/
    Creer_liste_rfxcom( &Liste_rfxcom, &scroll );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    g_signal_connect( G_OBJECT(Liste_rfxcom), "button_press_event",              /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_rfxcom), NULL );
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
                              G_CALLBACK(Menu_editer_rfxcom), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
/*    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_rfxcom), NULL );
*/
    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_rfxcom), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_rfxcom), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Edit RFXCOM") ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_rfxcom: Rafraichissement d'un rfxcom la liste à l'écran                                */
/* Entrée: une reference sur le rfxcom                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Rafraichir_visu_rfxcom( GtkListStore *store, GtkTreeIter *iter, struct CMD_TYPE_RFXCOM *rfxcom )
  { gtk_list_store_set ( store, iter,
                         COLONNE_ID, rfxcom->id,
                         COLONNE_TYPE, "test", /*rfxcom->actif,*/
                         COLONNE_CANAL, rfxcom->canal,
                         COLONNE_LIBELLE, rfxcom->libelle,
                         COLONNE_EA_MIN, rfxcom->ea_min,
                         COLONNE_E_MIN, rfxcom->e_min,
                         COLONNE_A_MIN, rfxcom->a_min,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_rfxcom: Ajoute un rfxcom dans la liste des rfxcoms                                         */
/* Entrée: une reference sur le rfxcom                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_rfxcom( struct CMD_TYPE_RFXCOM *rfxcom )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_RFXCOM)) Creer_page_rfxcom();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_rfxcom) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_rfxcom ( store, &iter, rfxcom );
  }
/**********************************************************************************************************/
/* Cacher_un_rfxcom: Enleve un rfxcom de la liste des rfxcoms                                             */
/* Entrée: une reference sur le rfxcom                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_rfxcom( struct CMD_TYPE_RFXCOM *rfxcom )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_rfxcom) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == rfxcom->id )
        { printf("elimination rfxcom %s\n", rfxcom->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_rfxcom: Rafraichissement du rfxcom en parametre                                     */
/* Entrée: une reference sur le rfxcom                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_rfxcom( struct CMD_TYPE_RFXCOM *rfxcom )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_rfxcom) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == rfxcom->id )
        { printf("maj rfxcom %s\n", rfxcom->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_rfxcom( GTK_LIST_STORE(store), &iter, rfxcom ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
