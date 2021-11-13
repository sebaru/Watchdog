/**********************************************************************************************************/
/* Client/liste_camera.c        Configuration des cameras de Watchdog v2.0                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 13:03:00 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_camera.c
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

 static GtkWidget *Liste_camera;                      /* GtkTreeView pour la gestion des cameras Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_camera ( void );
 static void Menu_editer_camera ( void );
 static void Menu_ajouter_camera ( void );
 static void Menu_exporter_camera ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_camera, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_camera, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Export"), NULL, Menu_exporter_camera, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_camera, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_camera, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Print"), NULL, Menu_exporter_camera, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_END
  };

/**********************************************************************************************************/
/* CB_effacer_camera: Fonction appelée qd on appuie sur un des boutons de l'interface                     */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_camera ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_CAMERA rezo_camera;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_camera) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_camera) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COL_CAM_ID, &rezo_camera.id, -1 );        /* Recup du id */
               gtk_tree_model_get( store, &iter, COL_CAM_LIBELLE, &libelle, -1 );

               memcpy( &rezo_camera.libelle, libelle, sizeof(rezo_camera.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_CAMERA, SSTAG_CLIENT_DEL_CAMERA,
                             (gchar *)&rezo_camera, sizeof(struct CMD_TYPE_CAMERA) );
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
/* Menu_ajouter_camera: Ajout d'un camera                                                                 */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_camera ( void )
  { Menu_ajouter_editer_camera(NULL); }
/**********************************************************************************************************/
/* Menu_effacer_camera: Retrait des cameras selectionnés                                                  */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_camera ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_camera) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    printf("Menu effacer camera: nbr=%d\n", nbr );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d camera%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_camera), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Type_vers_string: renvoie le type string associé                                                       */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 gchar *Type_camera_vers_string ( guint32 type )
  { switch (type)
     { case CAMERA_MODE_INCRUSTATION: return( _("Incrustation") );
       case CAMERA_MODE_ICONE: return( _("Icone") );
     }
    return( _("Unknown") );
  }
/**********************************************************************************************************/
/* Menu_editer_camera: Demande d'edition du camera selectionné                                            */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_camera ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_CAMERA rezo_camera;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_camera) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_camera) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COL_CAM_ID, &rezo_camera.id, -1 );                   /* Recup du id */
    gtk_tree_model_get( store, &iter, COL_CAM_LIBELLE, &libelle, -1 );

    memcpy( &rezo_camera.libelle, libelle, sizeof(rezo_camera.libelle) );
    g_free( libelle );
    Envoi_serveur( TAG_CAMERA, SSTAG_CLIENT_EDIT_CAMERA,
                  (gchar *)&rezo_camera, sizeof(struct CMD_TYPE_CAMERA) );
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
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_camera) );
    valide = TRUE;
    y = 2 * PRINT_FONT_SIZE;
    while ( valide && y<gtk_print_context_get_height (context) )       /* Pour tous les objets du tableau */
     { gtk_tree_model_get( store, iter, COL_CAM_NOTINHIB, &enable, COL_CAM_NUM, &num,
                           COL_CAM_SMS, &sms,
                           COL_CAM_TYPE_INT, &type_int, COL_CAM_TYPE_STRING, &type_string,
                           COL_CAM_OBJET, &objet, COL_CAM_LIBELLE, &libelle, -1 );

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
/* Menu_exporter_camera: Exportation de la base dans un fichier texte                                     */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Menu_exporter_camera( void )
  { static GtkTreeIter iter;
    GtkPrintOperation *print;
    GtkPrintOperationResult res;
    GtkTreeModel *store;
    gboolean valide;
    GError *error;
#ifdef bouh
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_camera) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    if (!valide) return;

    print = New_print_job ( "Print Cameras" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (draw_page), &iter );
    g_signal_connect (G_OBJECT(print), "begin-print",
                      G_CALLBACK (Begin_print), GTK_TREE_VIEW(Liste_camera) );

    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   GTK_WINDOW(F_client), &error);
#endif
  }
/**********************************************************************************************************/
/* Gerer_popup_camera: Gestion du menu popup quand on clique droite sur la liste des cameras              */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_camera ( GtkWidget *widget, GdkEventButton *event, gpointer data )
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
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_camera) );   /* On recupere selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_camera), event->x, event->y,
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
     { Menu_editer_camera(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_liste_camera: Creation de la liste du notebook consacrée aux cameras watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_liste_camera( GtkWidget **Liste, GtkWidget **Scroll )
  { GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    GtkWidget *scroll, *liste;

    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    *Scroll = scroll;

    store = gtk_list_store_new ( NBR_COL_CAM, G_TYPE_UINT,                                          /* Id */
                                              G_TYPE_STRING,                                    /* Numéro */
                                              G_TYPE_STRING,                                      /* type */
                                              G_TYPE_STRING,                                       /* bit */
                                              G_TYPE_STRING,                                     /* objet */
                                              G_TYPE_STRING,                                   /* libelle */
                                              G_TYPE_STRING                                   /* location */
                               );

    liste = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                 /* Creation de la vue */
    *Liste = liste;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(liste) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), liste );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de camera */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Numero"), renderer,
                                                         "text", COL_CAM_NUM,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COL_CAM_NUM);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de camera */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Type"), renderer,
                                                         "text", COL_CAM_TYPE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COL_CAM_TYPE);                   /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de camera */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Motion bit"), renderer,
                                                         "text", COL_CAM_BIT,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COL_CAM_BIT);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de camera */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Objet"), renderer,
                                                         "text", COL_CAM_OBJET,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COL_CAM_OBJET);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de camera */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Libelle"), renderer,
                                                         "text", COL_CAM_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COL_CAM_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de camera */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Location"), renderer,
                                                         "text", COL_CAM_LOCATION,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COL_CAM_LOCATION);               /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_camera), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(liste), TRUE );                        /* Pour faire beau */
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
  }
/**********************************************************************************************************/
/* Creer_page_camera: Creation de la page du notebook consacrée aux cameras watchdog                      */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_camera( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_CAMERA;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des cameras *******************************************/
    Creer_liste_camera( &Liste_camera, &scroll );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    g_signal_connect( G_OBJECT(Liste_camera), "button_press_event",              /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_camera), NULL );
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
                              G_CALLBACK(Menu_editer_camera), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_camera), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_camera), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_camera), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Edit Camera") ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_camera: Rafraichissement d'un camera la liste à l'écran                                */
/* Entrée: une reference sur le camera                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Rafraichir_visu_camera( GtkListStore *store, GtkTreeIter *iter, struct CMD_TYPE_CAMERA *camera )
  { gchar chaine[24], bit[24];

    g_snprintf( chaine, sizeof(chaine), "%04d", camera->num );
    g_snprintf( bit, sizeof(bit), "%s%04d", Type_bit_interne_court(MNEMO_MONOSTABLE), camera->bit );
    gtk_list_store_set ( store, iter,
                         COL_CAM_ID, camera->id,
                         COL_CAM_NUM, chaine,
                         COL_CAM_OBJET, camera->objet,
                         COL_CAM_LIBELLE, camera->libelle,
                         COL_CAM_LOCATION, camera->location,
                         COL_CAM_TYPE, Type_camera_vers_string(camera->type),
                         COL_CAM_BIT, bit,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_camera: Ajoute un camera dans la liste des cameras                                         */
/* Entrée: une reference sur le camera                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_camera( struct CMD_TYPE_CAMERA *camera )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_CAMERA)) Creer_page_camera();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_camera) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_camera ( store, &iter, camera );
  }
/**********************************************************************************************************/
/* Cacher_un_camera: Enleve un camera de la liste des cameras                                             */
/* Entrée: une reference sur le camera                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_camera( struct CMD_TYPE_CAMERA *camera )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_camera) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COL_CAM_ID, &id, -1 );
       if ( id == camera->id )
        { printf("elimination camera %s\n", camera->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_camera: Rafraichissement du camera en parametre                                     */
/* Entrée: une reference sur le camera                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_camera( struct CMD_TYPE_CAMERA *camera )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_camera) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COL_CAM_ID, &id, -1 );
       if ( id == camera->id )
        { printf("maj camera %s\n", camera->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_camera( GTK_LIST_STORE(store), &iter, camera ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
