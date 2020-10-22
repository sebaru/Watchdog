/******************************************************************************************************************************/
/* Client/liste_plugin_dls.c        Configuration des plugin_dlss de Watchdog v2.0                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           ven 23 nov 2007 20:33:45 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

 #include <gtk/gtk.h>

 #include "Reseaux.h"

 enum
  {  COLONNE_ID,
     COLONNE_ACTIVE,
     COLONNE_PACKAGE,
     COLONNE_GROUPE_PAGE,
     COLONNE_SHORTNAME,
     COLONNE_TECH_ID,
     COLONNE_NOM,
     COLONNE_COMPIL_DATE,
     COLONNE_COMPIL_STATUS,
     COLONNE_COMPIL_NBR,
     COLONNE_NBR_LIGNE,
/*     COLONNE_COLOR_FOND,
     COLONNE_COLOR_TEXTE,*/
     NBR_COLONNE
  };
 static gchar *DLS_COMPIL_STATUS[]=                                          /* Status en français de la derniere compilation */
  { "Never compiled yet",
    "Export from Database failed",
    "Error loading source file",
    "Error loading log file",
    "Syntax error",
    "Error Fork GCC",
    "OK with Warnings",
    "OK",
    "Functions are missing. Need compiling again.",
    "Error, plugin is setting bits he does not own."
  };

/******************************************* Définitions des prototypes programme *********************************************/
 #include "protocli.h"

#ifdef bouh
 static void Menu_effacer_plugin_dls ( void );
 static void Menu_editer_mnemo ( void );
 static void Menu_editer_mnemo_new ( void );
 static void Menu_editer_source_dls ( void );
 static void Menu_editer_plugin_dls ( void );
 static void Menu_ajouter_plugin_dls ( void );
 static void Menu_refresh_plugin_dls ( void );
 static void Menu_editer_source_dls_new ( void );
 static void Menu_voir_le_run ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_plugin_dls, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit Source"), NULL, Menu_editer_source_dls, GNOME_STOCK_PIXMAP_BOOK_OPEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Properties"), NULL, Menu_editer_plugin_dls, GNOME_STOCK_PIXMAP_PROPERTIES ),
    GNOMEUIINFO_ITEM_STOCK ( N_("M_nemoniques"), N_("Edit mnemoniques"),
                             Menu_editer_mnemo, GNOME_STOCK_PIXMAP_BOOK_GREEN ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("_Edition new Interface"), N_("Editer dans la nouvelle interface"),
                             Menu_editer_source_dls_new, GNOME_STOCK_PIXMAP_BOOK_OPEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Mnemo new Interface"), N_("Editer les mnemoniques"),
                             Menu_editer_mnemo_new, GNOME_STOCK_PIXMAP_BOOK_GREEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Voir le run"), N_("Voir les bits internes"),
                             Menu_voir_le_run, GNOME_STOCK_PIXMAP_BOOK_YELLOW ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_plugin_dls, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_plugin_dls, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_END
  };
#endif
/******************************************************************************************************************************/
/* Dls_compil_status: Renvoie le statut en clair de la derniere compilation D.L.S                                             */
/* Entrée : le statut au format entier                                                                                        */
/* Sortie : le statut au format chaine                                                                                        */
/******************************************************************************************************************************/
 static gchar *Dls_compil_status ( guint status )
  { if (status >= NBR_DLS_COMPIL_STATUS)
         return("Unknown");
    else return ( DLS_COMPIL_STATUS[status] );
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Menu_want_mnemonique: l'utilisateur desire editer les mnemoniques d'un plugin DLS                                          */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Menu_editer_mnemo ( void )
  { struct CMD_TYPE_PLUGIN_DLS rezo_dls;
    gchar *nom, *tech_id, *package;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_plugin_dls) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_plugin_dls) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_dls.id, -1 );                                          /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, -1 );                                         /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_PACKAGE, &package, -1 );                                         /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_NOM, &nom, -1 );

    g_snprintf( rezo_dls.nom, sizeof(rezo_dls.nom), "%s", nom );
    g_snprintf( rezo_dls.tech_id, sizeof(rezo_dls.tech_id), "%s", tech_id );
    g_snprintf( rezo_dls.package, sizeof(rezo_dls.package), "%s", package );
    g_free( nom );
    g_free( tech_id );
    g_free( package );
    if (!Chercher_page_notebook (TYPE_PAGE_MNEMONIQUE, rezo_dls.id, TRUE))                    /* Page deja créé et affichée ? */
     { Creer_page_mnemonique ( &rezo_dls );
       Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_WANT_PAGE_MNEMONIQUE, (gchar *)&rezo_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
       Chercher_page_notebook ( TYPE_PAGE_MNEMONIQUE, rezo_dls.id, TRUE );
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);
  }
/******************************************************************************************************************************/
/* Menu_editer_all_mnemo: Demande l'affichage de la page complete de tous les mnemoniques                                     */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Menu_editer_all_mnemo ( void )
  { if (!Chercher_page_notebook (TYPE_PAGE_ALL_MNEMONIQUE, 0, TRUE))                          /* Page deja créé et affichée ? */
     { Creer_page_all_mnemonique ();
       Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_WANT_PAGE_ALL_MNEMONIQUE, NULL, 0 );
       Chercher_page_notebook ( TYPE_PAGE_ALL_MNEMONIQUE, 0, TRUE );
     }
  }
/******************************************************************************************************************************/
/* Menu_refresh_plugin_D.L.S: rafraichir la liste des plugins D.L.S                                                           */
/* Entrée : néant                                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Menu_refresh_plugin_dls ( void )
  { GtkListStore *store;
    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(client->Liste_plugin_dls) ));      /* Récupération du model de data */
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
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_plugin_dls) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_plugin_dls) );
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
/******************************************************************************************************************************/
/* Menu_ajouter_synoptique: Ajout d'un synoptique                                                                             */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_ajouter_plugin_dls ( void )
  { Menu_ajouter_editer_plugin_dls( NULL ); }
/******************************************************************************************************************************/
/* Menu_effacer_plugin_dls: Retrait d'un plugin dls                                                                           */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_effacer_plugin_dls ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_plugin_dls) );

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
/******************************************************************************************************************************/
/* Menu_editer_source_dls: Demande d'edition du plugin_dls selectionné                                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_editer_source_dls ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_PLUGIN_DLS rezo_dls;
    gchar *nom, *tech_id, *package;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_plugin_dls) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_plugin_dls) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_dls.id, -1 );                                          /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, -1 );                                         /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_PACKAGE, &package, -1 );                                         /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_NOM, &nom, -1 );

    g_snprintf( rezo_dls.nom, sizeof(rezo_dls.nom), "%s", nom );
    g_snprintf( rezo_dls.tech_id, sizeof(rezo_dls.tech_id), "%s", tech_id );
    g_snprintf( rezo_dls.package, sizeof(rezo_dls.package), "%s", package );

    g_free( nom );
    g_free( tech_id );
    g_free( package );
    if (!Chercher_page_notebook (TYPE_PAGE_SOURCE_DLS, rezo_dls.id, TRUE))/* Page deja créé et affichée ? */
     { Creer_page_source_dls ( &rezo_dls );
     Envoi_serveur( TAG_DLS, SSTAG_CLIENT_WANT_SOURCE_DLS, (gchar *)&rezo_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);
  }
/******************************************************************************************************************************/
/* Menu_editer_source_dls: Demande d'edition du plugin_dls selectionné                                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_editer_source_dls_new ( void )
  { GtkTreeSelection *selection;
    gchar chaine[256], *tech_id;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_plugin_dls) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_plugin_dls) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (nbr!=1) return;                                                                           /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, -1 );                                         /* Recup du id */
    g_snprintf( chaine, sizeof(chaine), "admin/dls/sourceedit/%s", tech_id );
    g_free( tech_id );
    Firefox_exec( chaine );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);
  }
/******************************************************************************************************************************/
/* Menu_editer_source_dls: Demande d'edition du plugin_dls selectionné                                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_editer_mnemo_new ( void )
  { GtkTreeSelection *selection;
    gchar chaine[256], *tech_id;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_plugin_dls) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_plugin_dls) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (nbr!=1) return;                                                                           /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, -1 );                                         /* Recup du id */
    g_snprintf( chaine, sizeof(chaine), "admin/mnemo/index/%s", tech_id );
    g_free( tech_id );
    Firefox_exec( chaine );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);
  }
/******************************************************************************************************************************/
/* Menu_editer_source_dls: Demande d'edition du plugin_dls selectionné                                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_voir_le_run ( void )
  { GtkTreeSelection *selection;
    gchar chaine[256], *tech_id;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_plugin_dls) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_plugin_dls) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (nbr!=1) return;                                                                           /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, -1 );                                         /* Recup du id */
    g_snprintf( chaine, sizeof(chaine), "admin/dls/run/%s", tech_id );
    g_free( tech_id );
    Firefox_exec( chaine );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);
  }
/******************************************************************************************************************************/
/* Menu_editer_plugin_dls: Demande d'edition du plugin_dls selectionné                                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_editer_plugin_dls ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_PLUGIN_DLS rezo_dls;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *nom;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_plugin_dls) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_plugin_dls) );

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
/******************************************************************************************************************************/
/* Menu_exporter_message: Exportation de la base dans un fichier texte                                                        */
/* Entrée: néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Menu_exporter_plugin_dls( void )
  { GtkSourcePrintCompositor *compositor;
    GtkPrintOperation *print;
    GtkSourceBuffer *buffer;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    GError *error;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(client->Liste_plugin_dls) );
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
#endif
/******************************************************************************************************************************/
/* Gerer_popup_plugin_dls: Gestion du menu popup quand on clique droite sur la liste des plugin_dls                           */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                                                  */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Gerer_popup_plugin_dls ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { GtkWidget *Popup, *item, *hbox;
    struct CLIENT *client = data;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    gint cellx, celly;

    if (!event) return(FALSE);

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_plugin_dls) );                        /* On recupere la selection */
    if (gtk_tree_selection_count_selected_rows(selection) == 0)
     { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(client->Liste_plugin_dls), event->x, event->y, &path, NULL, &cellx, &celly );
       if (path)
        { gtk_tree_selection_select_path( selection, path );
          gtk_tree_path_free( path );
        }
     }

    if (event->type == GDK_2BUTTON_PRESS && event->button == 1 )                                            /* Double clic ?? */
     { //Menu_editer_source_dls();
       return(TRUE);
     }

    if ( event->button != 3 ) return(FALSE);                                                              /* Gestion du popup */

    Popup = gtk_menu_new();

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "document-open", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Editer la source"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Acquitter_histo), client );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "preferences-system", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Propriétés"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Acquitter_histo), client );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "system-run", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Voir le RUN"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Acquitter_histo), client );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "list-add", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Ajouter"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Acquitter_histo), client );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    item = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "edit-delete", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Supprimer"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Acquitter_histo), client );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    gtk_widget_show_all(Popup);
    gtk_menu_popup_at_pointer ( GTK_MENU(Popup), (GdkEvent *)event );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Updater_synoptique_CB: Met à jour la liste synoptique en fonction des parametres recu du serveur                           */
/* Entrée: les données issues de la librairie libsoup                                                                         */
/* sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Updater_un_plugin_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { struct CLIENT *client = user_data;
    GBytes *response_brute;
    gchar *reason_phrase;
    gint status_code;
    gsize taille;
    printf("%s\n", __func__ );
    g_object_get ( msg, "status-code", &status_code, "reason-phrase", &reason_phrase, NULL );
    if (status_code != 200)
     { GtkWidget *dialog = gtk_message_dialog_new ( GTK_WINDOW(client->window), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error Code %d: %s",
                                                    status_code, reason_phrase);
       gtk_dialog_run (GTK_DIALOG (dialog));
       gtk_widget_destroy (dialog);
       return;
     }
    g_object_get ( msg, "response-body-data", &response_brute, NULL );
    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( response_brute, &taille ) );
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Effacer_synoptique: Fonction appelée qd on appuie sur le bouton de suppression et que l'ona validé l'ordre                 */
/* Entrée: la page source                                                                                                     */
/* sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Effacer_un_plugin ( struct PAGE_NOTEBOOK *page )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(page->client->Liste_plugin_dls) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(page->client->Liste_plugin_dls) );
    lignes    = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { gint id;
       gtk_tree_model_get_iter( store, &iter, lignes->data );                              /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );                                                /* Recup du id */
       JsonBuilder *builder = Json_create ();
       if (builder)
        { Json_add_int ( builder, "dls_id", id );
          Envoi_json_au_serveur ( page->client, "DELETE", builder, "/api/dls/del", Updater_un_plugin_CB );
        }
       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                                               /* Liberation mémoire */
  }
/******************************************************************************************************************************/
/* Menu_effacer_synoptique: Retrait des synoptiques selectionnés                                                              */
/* Entrée: la page du client                                                                                                  */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_effacer_un_plugin ( struct PAGE_NOTEBOOK *page )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    gboolean retour;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(page->client->Liste_plugin_dls) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                       /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(page->client->window),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
                                      "Do you want to delete %d plugin%c ?", nbr, (nbr>1 ? 's' : ' ') );
    gtk_widget_show_all( dialog );
    retour = gtk_dialog_run( GTK_DIALOG(dialog) );                                     /* Attente de reponse de l'utilisateur */
    gtk_widget_destroy(dialog);
    if (retour == GTK_RESPONSE_OK) Effacer_un_plugin(page);
  }
/******************************************************************************************************************************/
/* Afficher_un_plugin: Rafraichissement d'un plugin_dls la liste à l'écran                                                    */
/* Entrée: une reference sur le plugin_dls                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Afficher_un_plugin (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct PAGE_NOTEBOOK *page=user_data;
    gchar groupe_page[512];
    GtkTreeIter iter;


    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(page->client->Liste_plugin_dls) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */

    g_snprintf( groupe_page, sizeof(groupe_page), "%s/%s", Json_get_string(element, "ppage"), Json_get_string(element, "page") );

    gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                         COLONNE_ID, Json_get_int ( element, "id" ),
                         COLONNE_ACTIVE, Json_get_bool ( element, "actif" ),
                         COLONNE_PACKAGE, Json_get_string( element, "package" ),
                         COLONNE_GROUPE_PAGE, groupe_page,
                         COLONNE_NOM, Json_get_string( element, "name" ),
                         COLONNE_SHORTNAME, Json_get_string( element, "shortname" ),
                         COLONNE_TECH_ID, Json_get_string( element, "tech_id" ),
                         COLONNE_COMPIL_DATE, Json_get_string( element, "compil_date" ),
                         COLONNE_COMPIL_STATUS, Dls_compil_status(Json_get_int ( element, "compil_status")),
                         COLONNE_COMPIL_NBR, Json_get_int( element, "nbr_compil" ),
                         COLONNE_NBR_LIGNE, Json_get_int( element, "nbr_ligne" ),
                          -1
                       );
  }
/******************************************************************************************************************************/
/* Detruire_page_plugin_dls: L'utilisateur veut fermer la page de plugin dls                                                  */
/* Entrée: la page en question                                                                                                */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Detruire_page_plugin_dls( struct PAGE_NOTEBOOK *page )
  { gtk_widget_destroy ( page->child );
    page->client->Liste_pages = g_slist_remove( page->client->Liste_pages, page );
    g_free(page);
  }
/******************************************************************************************************************************/
/* Creer_page_plugin_dls: Creation de la page du notebook consacrée aux plugins plugin_dlss watchdog                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Creer_page_plugin_dls_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    struct CLIENT *client = user_data;
    struct PAGE_NOTEBOOK *page;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    GBytes *response_brute;
    gchar *reason_phrase;
    JsonNode *response;
    gint status_code;
    gsize taille;

    g_object_get ( msg, "status-code", &status_code, "reason-phrase", &reason_phrase, NULL );
    if (status_code != 200)
     { gchar chaine[256];
       g_snprintf(chaine, sizeof(chaine), "Error loading plugins DLS: Code %d - %s", status_code, reason_phrase );
       Log(client, chaine);
       return;
     }

    g_object_get ( msg, "response-body-data", &response_brute, NULL );
    printf("Recu SYNS: %s %p\n", (gchar *)g_bytes_get_data ( response_brute, &taille ), client );

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;

    page->type   = TYPE_PAGE_PLUGIN_DLS;
    page->client = client;
    client->Liste_pages = g_slist_append( client->Liste_pages, page );

    g_object_get ( msg, "response-body-data", &response_brute, NULL );
    response = Json_get_from_string ( g_bytes_get_data ( response_brute, &taille ) );

    hboite = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );

/**************************************************** La liste des plugin_dlss ************************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                                    /* Id du plugin */
                                              G_TYPE_BOOLEAN,                                                     /* Activé ? */
                                              G_TYPE_STRING,                                                       /* Package */
                                              G_TYPE_STRING,                                                   /* Groupe/Page */
                                              G_TYPE_STRING,                                                     /* Shortname */
                                              G_TYPE_STRING,                                                       /* Tech_id */
                                              G_TYPE_STRING,                                                           /* Nom */
                                              G_TYPE_STRING,                                                   /* Compil_date */
                                              G_TYPE_STRING,                                                 /* Compil_status */
                                              G_TYPE_UINT,                                                      /* Compil_nbr */
                                              G_TYPE_UINT                                                        /* nbr ligne */
                                );
    client->Liste_plugin_dls = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                  /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_plugin_dls) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), client->Liste_plugin_dls );

    renderer = gtk_cell_renderer_toggle_new();                                                  /* Colonne de l'id du message */
    colonne = gtk_tree_view_column_new_with_attributes ( "Enable", renderer,
                                                         "active", COLONNE_ACTIVE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ACTIVE);                                     /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( "ID", renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                                         /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( "Tech_ID", renderer,
                                                         "text", COLONNE_TECH_ID,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_TECH_ID);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( "Groupe/Page", renderer,
                                                         "text", COLONNE_GROUPE_PAGE,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_GROUPE_PAGE);                                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( "Shortname", renderer,
                                                         "text", COLONNE_SHORTNAME,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_SHORTNAME);                                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( "Package", renderer,
                                                         "text", COLONNE_PACKAGE,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_PACKAGE);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( "D.L.S Name", renderer,
                                                         "text", COLONNE_NOM,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NOM);                                        /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( "Compilation Date", renderer,
                                                         "text", COLONNE_COMPIL_DATE,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_COMPIL_DATE);                                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( "Compil Status", renderer,
                                                         "text", COLONNE_COMPIL_STATUS,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_COMPIL_STATUS);                              /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( "# of Compil", renderer,
                                                         "text", COLONNE_COMPIL_NBR,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_COMPIL_NBR);                                 /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    renderer = gtk_cell_renderer_text_new();                                                  /* Colonne du nom de plugin_dls */
    colonne = gtk_tree_view_column_new_with_attributes ( "# of lines", renderer,
                                                         "text", COLONNE_NBR_LIGNE,
                                                         NULL);
    gtk_tree_view_column_set_reorderable(colonne, TRUE);                                       /* On peut deplacer la colonne */
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NBR_LIGNE);                                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_plugin_dls), colonne );

    g_signal_connect( G_OBJECT(client->Liste_plugin_dls), "button_press_event",                      /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_plugin_dls), client );
    g_object_unref (G_OBJECT (store));                                            /* nous n'avons plus besoin de notre modele */

/*********************************************** Les boutons de controles *****************************************************/
    boite = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_with_label( "Fermer" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "window-close", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Fermer la page" );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Detruire_page_plugin_dls), page );

    bouton = gtk_button_new_with_label( "Refresh" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "document-revert", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Recharger la liste" );
    //g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_refresh_plugin_dls), NULL );

    separateur = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_with_label( "Ouvrir" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "document-open", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Ouvrir le code source" );
    //g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_editer_source_dls), NULL );

    bouton = gtk_button_new_with_label( "Ajouter" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "list-add", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Ajouter un module DLS" );
    //g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_ajouter_editer_plugin_dls), NULL );

    bouton = gtk_button_new_with_label( "Imprimer" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "document-print", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Imprimer la liste des plugins DLS" );
    //g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_exporter_plugin_dls), NULL );

    separateur = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_with_label( "Supprimer" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "edit-delete", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Supprimer un plugin et ses dependances" );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_effacer_un_plugin), page );

    json_array_foreach_element ( Json_get_array ( response, "plugins" ), Afficher_un_plugin, page );
    json_node_unref ( response );
//    json_array_foreach_element ( Json_get_array ( infos->syn, "passerelles" ), Afficher_une_passerelle, infos );
//    json_array_foreach_element ( Json_get_array ( infos->syn, "comments" ),    Afficher_un_commentaire, infos );
//    json_array_foreach_element ( Json_get_array ( infos->syn, "cameras" ),     Afficher_une_camera, infos );
//    json_array_foreach_element ( Json_get_array ( infos->syn, "cadrans" ),     Afficher_un_cadran, infos );
    gtk_widget_show_all( hboite );
    gint page_num = gtk_notebook_append_page( GTK_NOTEBOOK(client->Notebook), page->child, gtk_label_new("Plugins D.L.S") );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK(client->Notebook), page_num );
  }
#ifdef bouh

/******************************************************************************************************************************/
/* Afficher_un_plugin_dls: Ajoute un plugin_dls dans la liste des plugin_dls                                                  */
/* Entrée: une reference sur le plugin_dls                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *plugin_dls )
  { GtkListStore *store;
    GtkTreeIter iter;
    if (!Tester_page_notebook(TYPE_PAGE_PLUGIN_DLS)) Creer_page_plugin_dls();

    Rafraichir_visu_plugin_dls ( &iter, plugin_dls );
  }
/******************************************************************************************************************************/
/* Cacher_un_plugin_dls: Enleve un plugin_dls de la liste des plugin_dls                                                      */
/* Entrée: une reference sur le plugin_dls                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_cacher_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *plugin_dls )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(client->Liste_plugin_dls) );
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
/******************************************************************************************************************************/
/* Proto_rafrachir_un_plugin_dls: Rafraichissement du plugin_dls en parametre                                                 */
/* Entrée: une reference sur le plugin_dls                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_rafraichir_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *plugin_dls )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(client->Liste_plugin_dls) );
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
#endif
/******************************************************************************************************************************/
/* Menu_want_supervision: l'utilisateur desire voir le synoptique supervision                                                 */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Menu_want_edition_DLS ( struct CLIENT *client )
  { if (Chercher_page_notebook( client, TYPE_PAGE_PLUGIN_DLS, 1, TRUE )) return;
    Envoi_json_au_serveur( client, "GET", NULL, "/api/dls/list", Creer_page_plugin_dls_CB );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
