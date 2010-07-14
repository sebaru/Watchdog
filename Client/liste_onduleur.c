/**********************************************************************************************************/
/* Client/liste_onduleur.c        Configuration des onduleurs de Watchdog v2.0                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 13:03:00 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_onduleur.c
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

 static GtkWidget *Liste_onduleur;                      /* GtkTreeView pour la gestion des onduleurs Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

 enum
  {  COLONNE_ID,
     COLONNE_ACTIF,
     COLONNE_HOST,
     COLONNE_UPS,
     COLONNE_BIT_COMM,
     COLONNE_EA_UPS_LOAD,
     COLONNE_EA_UPS_REAL_POWER,
     COLONNE_EA_BATTERY_CHARGE,
     COLONNE_EA_INPUT_VOLTAGE,
     COLONNE_LIBELLE,
     NBR_COLONNE
  };
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_onduleur ( void );
 static void Menu_editer_onduleur ( void );
 static void Menu_ajouter_onduleur ( void );
 static void Menu_exporter_onduleur ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_onduleur, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_onduleur, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Export"), NULL, Menu_exporter_onduleur, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_onduleur, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_onduleur, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Print"), NULL, Menu_exporter_onduleur, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_END
  };

/**********************************************************************************************************/
/* CB_effacer_onduleur: Fonction appelée qd on appuie sur un des boutons de l'interface                   */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_onduleur ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_ONDULEUR rezo_onduleur;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_onduleur) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_onduleur) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_onduleur.id, -1 );      /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

               memcpy( &rezo_onduleur.libelle, libelle, sizeof(rezo_onduleur.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_ONDULEUR, SSTAG_CLIENT_DEL_ONDULEUR,
                             (gchar *)&rezo_onduleur, sizeof(struct CMD_TYPE_ONDULEUR) );
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
/* Menu_ajouter_onduleur: Ajout d'un onduleur                                                             */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_onduleur ( void )
  { Menu_ajouter_editer_onduleur(NULL); }
/**********************************************************************************************************/
/* Menu_effacer_onduleur: Retrait des onduleurs selectionnés                                              */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_onduleur ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_onduleur) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    printf("Menu effacer onduleur: nbr=%d\n", nbr );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d onduleur%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_onduleur), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_editer_onduleur: Demande d'edition du onduleur selectionné                                        */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_onduleur ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_ONDULEUR rezo_onduleur;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_onduleur) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_onduleur) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_onduleur.id, -1 );                 /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_onduleur.libelle, libelle, sizeof(rezo_onduleur.libelle) );
    g_free( libelle );
    Envoi_serveur( TAG_ONDULEUR, SSTAG_CLIENT_EDIT_ONDULEUR,
                  (gchar *)&rezo_onduleur, sizeof(struct CMD_TYPE_ONDULEUR) );
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
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_onduleur) );
    valide = TRUE;
    y = 2 * PRINT_FONT_SIZE;
    while ( valide && y<gtk_print_context_get_height (context) )      /* Pour tous les objets du tableau */
     { gtk_tree_model_get( store, iter, COL_ONDULEUR_NOTINHIB, &enable, COL_ONDULEUR_NUM, &num,
                           COL_ONDULEUR_SMS, &sms,
                           COL_ONDULEUR_TYPE_INT, &type_int, COL_ONDULEUR_TYPE_STRING, &type_string,
                           COL_ONDULEUR_OBJET, &objet, COL_ONDULEUR_LIBELLE, &libelle, -1 );

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
/* Menu_exporter_onduleur: Exportation de la base dans un fichier texte                                    */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Menu_exporter_onduleur( void )
  { static GtkTreeIter iter;
    GtkPrintOperation *print;
    GtkPrintOperationResult res;
    GtkTreeModel *store;
    gboolean valide;
    GError *error;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_onduleur) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    if (!valide) return;

    print = New_print_job ( "Print Onduleur" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (draw_page), &iter );
    g_signal_connect (G_OBJECT(print), "begin-print",
                      G_CALLBACK (Begin_print), GTK_TREE_VIEW(Liste_onduleur) );

    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   GTK_WINDOW(F_client), &error);
  }
/**********************************************************************************************************/
/* Gerer_popup_onduleur: Gestion du menu popup quand on clique droite sur la liste des onduleurs            */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_onduleur ( GtkWidget *widget, GdkEventButton *event, gpointer data )
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
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_onduleur) );  /* On recupere selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_onduleur), event->x, event->y,
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
     { Menu_editer_onduleur(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_page_onduleur: Creation de la page du notebook consacrée aux onduleurs watchdog                  */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_onduleur( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_ONDULEUR;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des onduleurs *****************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* id */
                                              G_TYPE_BOOLEAN,                                    /* actif */
                                              G_TYPE_STRING,                                      /* host */
                                              G_TYPE_STRING,                                       /* ups */
                                              G_TYPE_UINT,                                    /* bit_comm */
                                              G_TYPE_UINT,                                        /* load */
                                              G_TYPE_UINT,                                   /* real powa */
                                              G_TYPE_UINT,                              /* battery charge */
                                              G_TYPE_UINT,                               /* input voltage */
                                              G_TYPE_STRING                                    /* libelle */
                               );

    Liste_onduleur = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );        /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_onduleur) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_onduleur );

    renderer = gtk_cell_renderer_toggle_new();                             /* Colonne de l'id du onduleur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("ON"), renderer,
                                                         "active", COLONNE_ACTIF,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ACTIF);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_onduleur), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de onduleur */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("ID"), renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                     /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_onduleur), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de onduleur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Host"), renderer,
                                                         "text", COLONNE_HOST,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_HOST);                   /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_onduleur), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de onduleur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Name"), renderer,
                                                         "text", COLONNE_UPS,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_UPS);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_onduleur), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de onduleur */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Comm."), renderer,
                                                         "text", COLONNE_BIT_COMM,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_BIT_COMM);               /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_onduleur), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de onduleur */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("EA Load"), renderer,
                                                         "text", COLONNE_EA_UPS_LOAD,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_EA_UPS_LOAD);            /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_onduleur), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de onduleur */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("EA Real Power"), renderer,
                                                         "text", COLONNE_EA_UPS_REAL_POWER,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_EA_UPS_REAL_POWER);      /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_onduleur), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de onduleur */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("EA Batt.Charge"), renderer,
                                                         "text", COLONNE_EA_BATTERY_CHARGE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_EA_BATTERY_CHARGE);      /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_onduleur), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de onduleur */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("EA Input Volt."), renderer,
                                                         "text", COLONNE_EA_INPUT_VOLTAGE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_EA_INPUT_VOLTAGE);       /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_onduleur), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de onduleur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Libelle"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_onduleur), colonne );

    g_signal_connect( G_OBJECT(Liste_onduleur), "button_press_event",             /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_onduleur), NULL );
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
                              G_CALLBACK(Menu_editer_onduleur), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_onduleur), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_onduleur), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_onduleur), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Edit UPS") ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_onduleur: Rafraichissement d'un onduleur la liste à l'écran                            */
/* Entrée: une reference sur le onduleur                                                                  */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_onduleur( GtkTreeIter *iter, struct CMD_TYPE_ONDULEUR *onduleur )
  { GtkTreeModel *store;
    gchar bit_comm[10], load[10], real_power[10], battery_charge[10], input_voltage[10];

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_onduleur) );            /* Acquisition du modele */

    g_snprintf( bit_comm, sizeof(bit_comm), "%s%04d",
                Type_bit_interne_court(MNEMO_BISTABLE), onduleur->bit_comm );
    g_snprintf( load, sizeof(load), "%s%04d",
                Type_bit_interne_court(MNEMO_ENTREE_ANA), onduleur->ea_ups_load );
    g_snprintf( real_power, sizeof(real_power), "%s%04d",
                Type_bit_interne_court(MNEMO_ENTREE_ANA), onduleur->ea_ups_real_power );
    g_snprintf( battery_charge, sizeof(battery_charge), "%s%04d",
                Type_bit_interne_court(MNEMO_ENTREE_ANA), onduleur->ea_battery_charge );
    g_snprintf( input_voltage, sizeof(input_voltage), "%s%04d",
                Type_bit_interne_court(MNEMO_ENTREE_ANA), onduleur->ea_input_voltage );

    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, onduleur->id,
                         COLONNE_ACTIF, onduleur->actif,
                         COLONNE_HOST, onduleur->host,
                         COLONNE_UPS, onduleur->ups,
                         COLONNE_BIT_COMM, bit_comm,
                         COLONNE_EA_UPS_LOAD, load,
                         COLONNE_EA_UPS_REAL_POWER, real_power,
                         COLONNE_EA_BATTERY_CHARGE, battery_charge,
                         COLONNE_EA_INPUT_VOLTAGE, input_voltage,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_onduleur: Ajoute un onduleur dans la liste des onduleurs                                      */
/* Entrée: une reference sur le onduleur                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_onduleur( struct CMD_TYPE_ONDULEUR *onduleur )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_ONDULEUR)) Creer_page_onduleur();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_onduleur) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_onduleur ( &iter, onduleur );
  }
/**********************************************************************************************************/
/* Cacher_un_onduleur: Enleve un onduleur de la liste des onduleurs                                          */
/* Entrée: une reference sur le onduleur                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_onduleur( struct CMD_TYPE_ONDULEUR *onduleur )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_onduleur) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == onduleur->id )
        { printf("elimination onduleur %s\n", onduleur->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_onduleur: Rafraichissement du onduleur en parametre                                   */
/* Entrée: une reference sur le onduleur                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_onduleur( struct CMD_TYPE_ONDULEUR *onduleur )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_onduleur) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == onduleur->id )
        { printf("maj onduleur %s\n", onduleur->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_onduleur( &iter, onduleur ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
