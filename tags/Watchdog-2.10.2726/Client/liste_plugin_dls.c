/**********************************************************************************************************/
/* Client/liste_plugin_dls.c        Configuration des plugin_dlss de Watchdog v2.0                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                       ven 23 nov 2007 20:33:45 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_plugin_dls.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2007 - Sébastien Lefevre
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

 static GtkWidget *Liste_plugin_dls;              /* GtkTreeView pour la gestion des plugin_dlss Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 enum
  {  COLONNE_ID,
     COLONNE_ACTIVE,
     COLONNE_TYPE,
     COLONNE_GROUPE_PAGE,
     COLONNE_NOM,
     COLONNE_COMPIL_DATE,
     COLONNE_COMPIL_STATUS,
     COLONNE_COLOR_FOND,
     COLONNE_COLOR_TEXTE,
     NBR_COLONNE
  };

 static GdkColor COULEUR_PLUGIN_FOND[]=
  { { 0x0, 0xFFFF, 0xFFFF, 0xFFFF }, /* Module */
    { 0x0, 0x0,    0x0,    0x7FFF }, /* Sous-groupe */
    { 0x0, 0x0,    0x7FFF, 0x0    }, /* Groupe */
    { 0x0, 0x7FFF, 0x0,    0x0    }, /* Top level */
  };
 static GdkColor COULEUR_PLUGIN_TEXTE[]=
  { { 0x0, 0x0,    0x0,    0x0    },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
  };

 static gchar *DLS_COMPIL_STATUS[]=                                          /* Status en français de la derniere compilation */
  { "Never compiled yet",
    "Error loading source file",
    "Error loading log file",
    "Syntax error",
    "Error Fork GCC",
    "OK with Warnings",
    "OK"
  };

/******************************************* Définitions des prototypes programme *********************************************/
 #include "protocli.h"

 static void Menu_effacer_plugin_dls ( void );
 static void Menu_editer_source_dls ( void );
 static void Menu_editer_plugin_dls ( void );
 static void Menu_ajouter_plugin_dls ( void );
 static void Menu_refresh_plugin_dls ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_plugin_dls, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit Source"), NULL, Menu_editer_source_dls, GNOME_STOCK_PIXMAP_BOOK_OPEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Properties"), NULL, Menu_editer_plugin_dls, GNOME_STOCK_PIXMAP_PROPERTIES ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_plugin_dls, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_plugin_dls, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_END
  };

/******************************************************************************************************************************/
/* Dls_compil_status: Renvoie le statut en clair de la derniere compilation D.L.S                                             */
/* Entrée : le statut au format entier                                                                                        */
/* Sortie : le statut au format chaine                                                                                        */
/******************************************************************************************************************************/
 static gchar *Dls_compil_status ( guint status )
  { if (status >= NBR_DLS_COMPIL_STATUS)
         return("Unknown");
    else return ( DLS_COMPIL_STATUS[status]);
  }
/******************************************************************************************************************************/
/* Menu_refresh_plugin_D.L.S: rafraichir la liste des plugins D.L.S                                                           */
/* Entrée : néant                                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Menu_refresh_plugin_dls ( void )
  { GtkListStore *store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_plugin_dls) ));      /* Récupération du model de data */
    gtk_list_store_clear ( store );
    Envoi_serveur( TAG_DLS, SSTAG_CLIENT_WANT_PAGE_DLS, NULL, 0 );
  }
/******************************************************************************************************************************/
/* CB_effacer_utilisateur: Fonction appelée qd on appuie sur un des boutons de l'interface                                    */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                                                   */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_effacer_plugin_dls ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_PLUGIN_DLS rezo_dls;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_plugin_dls) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_plugin_dls) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *nom;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_dls.id, -1 );        /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_NOM, &nom, -1 );

               memcpy( &rezo_dls.nom, nom, sizeof(rezo_dls.nom) );
               g_free( nom );

               Envoi_serveur( TAG_DLS, SSTAG_CLIENT_DEL_PLUGIN_DLS,
                              (gchar *)&rezo_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
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
 static void Menu_ajouter_plugin_dls ( void )
  { Menu_ajouter_editer_plugin_dls( NULL ); }
/**********************************************************************************************************/
/* Menu_effacer_plugin_dls: Retrait d'un plugin dls                                                       */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_plugin_dls ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_plugin_dls) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                       /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d plugin%c ?"), nbr, (nbr>1 ? 's' : ' ') );
     
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_plugin_dls), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_editer_source_dls: Demande d'edition du plugin_dls selectionné                                    */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_source_dls ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_PLUGIN_DLS rezo_dls;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *nom;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_plugin_dls) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_plugin_dls) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_dls.id, -1 );                      /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_NOM, &nom, -1 );

    memcpy( &rezo_dls.nom, nom, sizeof(rezo_dls.nom) );
    g_free( nom );
    if (!Chercher_page_notebook (TYPE_PAGE_SOURCE_DLS, rezo_dls.id, TRUE))/* Page deja créé et affichée ? */
     { Creer_page_source_dls ( &rezo_dls );
     Envoi_serveur( TAG_DLS, SSTAG_CLIENT_WANT_SOURCE_DLS,
                      (gchar *)&rezo_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);
  }
/**********************************************************************************************************/
/* Menu_editer_plugin_dls: Demande d'edition du plugin_dls selectionné                                    */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_plugin_dls ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_PLUGIN_DLS rezo_dls;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *nom;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_plugin_dls) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_plugin_dls) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_dls.id, -1 );               /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_NOM, &nom, -1 );

    memcpy( &rezo_dls.nom, nom, sizeof(rezo_dls.nom) );
    g_free( nom );
    Envoi_serveur( TAG_DLS, SSTAG_CLIENT_EDIT_PLUGIN_DLS,
                   (gchar *)&rezo_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);
  }
/**********************************************************************************************************/
/* Menu_exporter_message: Exportation de la base dans un fichier texte                                    */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Menu_exporter_plugin_dls( void )
  { GtkSourcePrintCompositor *compositor;
    GtkPrintOperation *print;
    GtkSourceBuffer *buffer;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    GError *error;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_plugin_dls) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    if (!valide) return;

    buffer = gtk_source_buffer_new( NULL );                               /* Création d'un nouveau buffer */
    gtk_text_buffer_create_tag ( GTK_TEXT_BUFFER(buffer), "active",
                                "background", "green", "foreground", "white", NULL
                               );
    gtk_text_buffer_create_tag ( GTK_TEXT_BUFFER(buffer), "down",
                                "background", "red",   "foreground", "white", NULL
                               );

    while ( valide  )                                            /* Pour tous les groupe_pages du tableau */
     { gchar pivot[80], *nom, chaine[128], *groupe_page;
       GtkTextIter iter_end;
       gint len, cpt, id;
       gboolean active;

       gtk_tree_model_get( store, &iter, COLONNE_ID, &id, COLONNE_ACTIVE, &active,
                                         COLONNE_NOM, &nom,
                                         COLONNE_GROUPE_PAGE, &groupe_page,
                           -1 );

       gtk_text_buffer_get_end_iter ( GTK_TEXT_BUFFER(buffer), &iter_end );
       if (active)
        { gtk_text_buffer_insert_with_tags_by_name ( GTK_TEXT_BUFFER(buffer), &iter_end, "--UP--", -1,
                                                     "active", NULL );
        }
       else
        { gtk_text_buffer_insert_with_tags_by_name ( GTK_TEXT_BUFFER(buffer), &iter_end, "-DOWN-", -1,
                                                     "down", NULL );
        }

       g_snprintf( chaine, sizeof(chaine), "- %04d - ", id );
       g_utf8_strncpy ( pivot, groupe_page, 30 );
       len = g_utf8_strlen( pivot, -1 );
       g_strlcat ( chaine, pivot, sizeof(chaine) );
       if ( len <= PRINT_NBR_CHAR_GROUPE_PAGE )
        { for(cpt=len; cpt<=PRINT_NBR_CHAR_GROUPE_PAGE; cpt++) g_strlcat( chaine, " ", sizeof(chaine) ); }
       g_strlcat( chaine, "- ", sizeof(chaine) );

       g_strlcat( chaine, nom, sizeof(chaine)-1 );
       g_strlcat( chaine, "\n", sizeof(chaine) );

       gtk_text_buffer_insert_at_cursor ( GTK_TEXT_BUFFER(buffer), chaine, -1 );
       g_free(nom);
       g_free(groupe_page);
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    compositor = gtk_source_print_compositor_new ( GTK_SOURCE_BUFFER(buffer) );
    gtk_source_print_compositor_set_print_line_numbers ( compositor, 5 );
    gtk_source_print_compositor_set_body_font_name ( compositor, PRINT_FONT_NAME );
    gtk_source_print_compositor_set_print_header ( compositor, TRUE );
    gtk_source_print_compositor_set_header_format ( compositor, TRUE,
                                                    "Plugin D.L.S",
                                                    "%F",
                                                    PRINT_HEADER_RIGHT);
    gtk_source_print_compositor_set_print_footer ( compositor, TRUE );
    gtk_source_print_compositor_set_footer_format ( compositor, TRUE,
                                                    PRINT_FOOTER_LEFT,
                                                    PRINT_FOOTER_CENTER,
                                                    PRINT_FOOTER_RIGHT);
    gtk_source_print_compositor_set_highlight_syntax ( compositor, TRUE );

    print = New_print_job ( "Print Plugin D.L.S" );
    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (Print_draw_page), compositor );
    g_signal_connect (G_OBJECT(print), "paginate",  G_CALLBACK (Print_paginate),  compositor );

    gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                             GTK_WINDOW(F_client), &error);
    g_object_unref( compositor);
    g_object_unref(buffer);
  }
/**********************************************************************************************************/
/* Gerer_popup_plugin_dls: Gestion du menu popup quand on clique droite sur la liste des plugin_dls       */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_plugin_dls ( GtkWidget *widget, GdkEventButton *event, gpointer data )
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
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_plugin_dls) );
                                                                              /* On recupere la selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_plugin_dls), event->x, event->y,
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
     { Menu_editer_source_dls(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_page_plugin_dls: Creation de la page du notebook consacrée aux plugins plugin_dlss watchdog      */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_plugin_dls( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_PLUGIN_DLS;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/**************************************************** La liste des plugin_dlss ************************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                                    /* Id du plugin */
                                              G_TYPE_BOOLEAN,                                                     /* Activé ? */
                                              G_TYPE_UINT,                                                            /* Type */
                                              G_TYPE_STRING,                                                   /* Groupe/Page */
                                              G_TYPE_STRING,                                                           /* Nom */
                                              G_TYPE_STRING,                                                   /* Compil_date */
                                              G_TYPE_STRING,                                                 /* Compil_status */
                                              GDK_TYPE_COLOR,                                                   /* Color_fond */
                                              GDK_TYPE_COLOR                                                   /* Color_texte */
                                );

    Liste_plugin_dls = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                          /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_plugin_dls) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_plugin_dls );

    renderer = gtk_cell_renderer_toggle_new();                                                  /* Colonne de l'id du message */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Enable"), renderer,
                                                         "active", COLONNE_ACTIVE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ACTIVE);                                     /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( _("ID"), renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                                         /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Groupe/Page"), renderer,
                                                         "text", COLONNE_GROUPE_PAGE,
                                                         "background-gdk", COLONNE_COLOR_FOND,
                                                         "foreground-gdk", COLONNE_COLOR_TEXTE,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_GROUPE_PAGE);                                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( _("D.L.S Name"), renderer,
                                                         "text", COLONNE_NOM,
                                                         "background-gdk", COLONNE_COLOR_FOND,
                                                         "foreground-gdk", COLONNE_COLOR_TEXTE,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NOM);                                        /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Compilation Date"), renderer,
                                                         "text", COLONNE_COMPIL_DATE,
                                                         "background-gdk", COLONNE_COLOR_FOND,
                                                         "foreground-gdk", COLONNE_COLOR_TEXTE,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_COMPIL_DATE);                                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Compil Status"), renderer,
                                                         "text", COLONNE_COMPIL_STATUS,
                                                         "background-gdk", COLONNE_COLOR_FOND,
                                                         "foreground-gdk", COLONNE_COLOR_TEXTE,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_COMPIL_STATUS);                              /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_plugin_dls), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_plugin_dls), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_plugin_dls), TRUE );                                 /* Pour faire beau */

    g_signal_connect( G_OBJECT(Liste_plugin_dls), "button_press_event",                              /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_plugin_dls), NULL );
    g_object_unref (G_OBJECT (store));                                            /* nous n'avons plus besoin de notre modele */
    
/*********************************************** Les boutons de controles *****************************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REFRESH );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_refresh_plugin_dls), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_OPEN );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_editer_source_dls), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_plugin_dls), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_plugin_dls), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_plugin_dls), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Plugins D.L.S") ) );
  }
/******************************************************************************************************************************/
/* Rafraichir_visu_plugin_dls: Rafraichissement d'un plugin_dls la liste à l'écran                                            */
/* Entrée: une reference sur le plugin_dls                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Rafraichir_visu_plugin_dls( GtkTreeIter *iter, struct CMD_TYPE_PLUGIN_DLS *plugin_dls )
  { gchar chaine[128], groupe_page[512], *date_compil;
    GtkTreeModel *store;
    struct tm *temps;
    time_t time;

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_plugin_dls) );                              /* Acquisition du modele */

    g_snprintf( groupe_page, sizeof(groupe_page), "%s/%s", plugin_dls->groupe, plugin_dls->page );
    time = plugin_dls->compil_date;
    temps = localtime( (time_t *)&time );
    if (temps) { strftime( chaine, sizeof(chaine), "%F %T", temps ); }
    else       { g_snprintf( chaine, sizeof(chaine), _("Erreur") ); }
    date_compil = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );

    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, plugin_dls->id,
                         COLONNE_ACTIVE, plugin_dls->on,
                         COLONNE_TYPE, plugin_dls->type,
                         COLONNE_GROUPE_PAGE, groupe_page,
                         COLONNE_NOM, plugin_dls->nom,
                         COLONNE_COMPIL_DATE, date_compil,
                         COLONNE_COMPIL_STATUS, Dls_compil_status(plugin_dls->compil_status),
                         COLONNE_COLOR_FOND, &COULEUR_PLUGIN_FOND[plugin_dls->type],
                         COLONNE_COLOR_TEXTE, &COULEUR_PLUGIN_TEXTE[plugin_dls->type],
                          -1
                       );
    g_free( date_compil );
  }
/**********************************************************************************************************/
/* Afficher_un_plugin_dls: Ajoute un plugin_dls dans la liste des plugin_dlss                             */
/* Entrée: une reference sur le plugin_dls                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *plugin_dls )
  { GtkListStore *store;
    GtkTreeIter iter;
    if (!Tester_page_notebook(TYPE_PAGE_PLUGIN_DLS)) Creer_page_plugin_dls();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_plugin_dls) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_plugin_dls ( &iter, plugin_dls );
  }
/**********************************************************************************************************/
/* Cacher_un_plugin_dls: Enleve un plugin_dls de la liste des plugin_dlss                                 */
/* Entrée: une reference sur le plugin_dls                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *plugin_dls )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_plugin_dls) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == plugin_dls->id )
        { printf("elimination plugin_dls %s\n", plugin_dls->nom );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_plugin_dls: Rafraichissement du plugin_dls en parametre                             */
/* Entrée: une reference sur le plugin_dls                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *plugin_dls )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_plugin_dls) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == plugin_dls->id )
        { printf("maj plugin_dls %s\n", plugin_dls->nom );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_plugin_dls( &iter, plugin_dls ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
