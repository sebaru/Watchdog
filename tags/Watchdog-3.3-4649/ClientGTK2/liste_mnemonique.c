/******************************************************************************************************************************/
/* Client/liste_mnemonique.c        Configuration des mnemoniques de Watchdog v2.0                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           dim 05 déc 2004 14:20:57 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * liste_mnemonique.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sébastien Lefevre
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
 #include <fcntl.h>
 #include <sys/stat.h>
 
 #include "Reseaux.h"

 extern GList *Liste_pages;                                                       /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                                             /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG Config;                                                              /* Configuration generale watchdog */

 enum
  {  COLONNE_ID,
     COLONNE_TYPE_INT,
     COLONNE_TYPE,
     COLONNE_GROUPE_PAGE_DLS,
     COLONNE_NUM_PLUGIN,
     COLONNE_ACRONYME,
     COLONNE_TECH_ID,
     COLONNE_LIBELLE,
     COLONNE_EV_HOST,
     COLONNE_EV_THREAD,
     COLONNE_EV_TEXT,
     COLONNE_TABLEAU,
     COLONNE_COULEUR,
     COLONNE_COULEUR_TEXTE,
     NBR_COLONNE
  };
 static GdkColor COULEUR[NBR_TYPE_MNEMO]=
  { { 0x0, 0xAFFF, 0xAFFF, 0xAFFF },
    { 0x0, 0x9FFF, 0x0,    0x9FFF },
    { 0x0, 0x0,    0x7FFF, 0x7FFF },
    { 0x0, 0xCFFF, 0x4FFF, 0x0    },
    { 0x0, 0xAFFF, 0xAFFF, 0x0    },
    { 0x0, 0x0,    0xAFFF, 0x0    },
    { 0x0, 0x0,    0x0,    0xFFFF },
    { 0x0, 0x0,    0x7FFF, 0xCFFF },
    { 0x0, 0x0,    0x7FFF, 0x0    },
    { 0x0, 0xFFFF, 0xFFFF, 0x0    },
    { 0x0, 0x7FFF, 0x7FFF, 0x0    }, /* Registres */
    { 0x0, 0xAFFF, 0x0,    0x0    }, /* Horloge */
  };
 static GdkColor COULEUR_TEXTE[NBR_TYPE_MNEMO]=
  { { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF },
    { 0x0, 0x0,    0x0,    0x0    },
    { 0x0, 0x0,    0x0,    0x0    }, /* Registres */
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF }, /* Horloges */
  };
 static gchar *TYPE_BIT_INTERNE[ NBR_TYPE_MNEMO ]=          /* Type des différents bits internes utilisés */
  { "B  - Bistable",
    "M  - Monostable",
    "T  - Temporisation",
    "E  - Entree TOR",
    "A  - Sortie TOR",
    "EA - Entree ANA",
    "AA - Sortie ANA",
    "I  - Visuel Icone",
    "CH - Compteur H",
    "CI - Compteur IMP",
    "R  - Registre",
    "HOR- Horloge",
  };
 static gchar *TYPE_BIT_INTERNE_COURT[ NBR_TYPE_MNEMO ]=    /* Type des différents bits internes utilisés */
  { "B",
    "M",
    "T",
    "E",
    "A",
    "EA",
    "AA",
    "I",
    "CH",
    "CI",
    "R",
    "HOR",
  };
/******************************************** Définitions des prototypes programme ********************************************/
 #include "protocli.h"

 static void Menu_effacer_mnemonique ( void );
 static void Menu_editer_mnemonique ( void );
 static void Menu_ajouter_mnemonique ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_mnemonique, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_mnemonique, GNOME_STOCK_PIXMAP_PROPERTIES ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_mnemonique, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_mnemonique, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_END
  };
/******************************************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                                              */
/* Entrée: le numero du type                                                                                                  */
/* Sortie: le type                                                                                                            */
/******************************************************************************************************************************/
 static GdkColor *Couleur_bit_interne ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return( &COULEUR[0] );
    return( &COULEUR[num] );
  }
/******************************************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                                              */
/* Entrée: le numero du type                                                                                                  */
/* Sortie: le type                                                                                                            */
/******************************************************************************************************************************/
 static GdkColor *Couleur_texte_bit_interne ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return( &COULEUR_TEXTE[0] );
    return( &COULEUR_TEXTE[num] );
  }
/******************************************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                                              */
/* Entrée: le numero du type                                                                                                  */
/* Sortie: le type                                                                                                            */
/******************************************************************************************************************************/
 gchar *Type_bit_interne ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return("Type_bit_interne: Erreur interne");
    return( TYPE_BIT_INTERNE[num] );
  }
/******************************************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                                              */
/* Entrée: le numero du type                                                                                                  */
/* Sortie: le type                                                                                                            */
/******************************************************************************************************************************/
 gint Type_bit_interne_int ( gchar *type )
  { gint cpt;
    for (cpt=0; cpt<NBR_TYPE_MNEMO; cpt++)
     { if ( !strcmp(type, TYPE_BIT_INTERNE[cpt]) ) return cpt; }
    printf("Type_bit_interne_int: pas trouvé\n");
    return(0);
  }
/******************************************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                                              */
/* Entrée: le numero du type                                                                                                  */
/* Sortie: le type                                                                                                            */
/******************************************************************************************************************************/
 gchar *Type_bit_interne_court ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return("Type_bit_interne_court: Erreur interne");
    return( TYPE_BIT_INTERNE_COURT[num] );
  }
/******************************************************************************************************************************/
/* CB_effacer_mnemonique: Fonction appelée qd on appuie sur un des boutons de l'interface                                     */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                                                   */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_effacer_mnemonique ( GtkDialog *dialog, gint reponse, struct PAGE_NOTEBOOK *page )
  { struct CMD_TYPE_MNEMO_BASE rezo_mnemonique;
    struct TYPE_INFO_MNEMONIQUE *infos;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    infos = page->infos;
    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_mnemonique) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(infos->Liste_mnemonique) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );                      /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_mnemonique.id, -1 );                        /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_NUM_PLUGIN, &rezo_mnemonique.dls_id, -1 );        /* Recup du dls_id */
               gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );
               memcpy( &rezo_mnemonique.libelle, libelle, sizeof(rezo_mnemonique.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_DEL_MNEMONIQUE,
                             (gchar *)&rezo_mnemonique, sizeof(struct CMD_TYPE_MNEMO_BASE) );
               gtk_tree_selection_unselect_iter( selection, &iter );
               lignes = lignes->next;
             }
            g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
            g_list_free (lignes);                                                                       /* Liberation mémoire */
            break;
       default: break;
     }
    gtk_widget_destroy( GTK_WIDGET(dialog) );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Menu_ajouter_mnemonique: Ajout d'un mnemonique                                                                             */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_ajouter_mnemonique ( void )
  { struct PAGE_NOTEBOOK *page = Page_actuelle();
    struct TYPE_INFO_MNEMONIQUE *infos;
    infos=page->infos;
    if (page->type == TYPE_PAGE_MNEMONIQUE) Menu_ajouter_editer_mnemonique(NULL, infos->id);
  }
/******************************************************************************************************************************/
/* Menu_effacer_mnemonique: Retrait des mnemoniques selectionnés                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_effacer_mnemonique ( void )
  { struct TYPE_INFO_MNEMONIQUE *infos;
    struct PAGE_NOTEBOOK *page = Page_actuelle();
    GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    infos = page->infos;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_mnemonique) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d mnemonique%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_mnemonique), page );
    gtk_widget_show_all( dialog );
  }
/******************************************************************************************************************************/
/* Menu_editer_mnemonique: Demande d'edition du mnemonique selectionné                                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_editer_mnemonique ( void )
  { GtkTreeSelection *selection;
    struct PAGE_NOTEBOOK *page = Page_actuelle();
    struct TYPE_INFO_MNEMONIQUE *infos;
    struct CMD_TYPE_MNEMO_BASE rezo_mnemonique;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    infos = page->infos;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_mnemonique) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(infos->Liste_mnemonique) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_mnemonique.id, -1 );                                   /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_mnemonique.libelle, libelle, sizeof(rezo_mnemonique.libelle) );
    g_free( libelle );

    Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_EDIT_MNEMONIQUE,
                  (gchar *)&rezo_mnemonique, sizeof(struct CMD_TYPE_MNEMO_BASE) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                                               /* Liberation mémoire */
  }
/******************************************************************************************************************************/
/* Menu_exporter_message: Exportation de la base dans un fichier texte                                                        */
/* Entrée: néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Menu_exporter_mnemonique( void )
  { struct PAGE_NOTEBOOK *page = Page_actuelle();
    struct TYPE_INFO_MNEMONIQUE *infos;
    GtkSourcePrintCompositor *compositor;
    GtkPrintOperation *print;
    GtkSourceBuffer *buffer;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    GError *error;
    gint cpt;

    infos = page->infos;
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(infos->Liste_mnemonique) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    if (!valide) return;

    buffer = gtk_source_buffer_new( NULL );                               /* Création d'un nouveau buffer */
    for ( cpt=0; cpt < NBR_TYPE_MNEMO; cpt++ )
     { gtk_text_buffer_create_tag ( GTK_TEXT_BUFFER(buffer), Type_bit_interne(cpt),
                                   "background-gdk", Couleur_bit_interne(cpt),
                                   "foreground-gdk", Couleur_texte_bit_interne(cpt),
                                   NULL );
     }

    while ( valide  )                                            /* Pour tous les groupe_pages du tableau */
     { gchar pivot[80], *type_string, *groupe_page, *libelle, *acronyme, chaine[128];
       GtkTextIter iter_end;
       gint len, cpt, type_int;

       gtk_tree_model_get( store, &iter, COLONNE_TYPE_INT, &type_int, COLONNE_TYPE, &type_string,
                           COLONNE_GROUPE_PAGE_DLS, &groupe_page,
                           COLONNE_ACRONYME, &acronyme, COLONNE_LIBELLE, &libelle, -1 );

       g_snprintf( chaine, sizeof(chaine), "%-6s", type_string );
       gtk_text_buffer_get_end_iter ( GTK_TEXT_BUFFER(buffer), &iter_end );
       gtk_text_buffer_insert_with_tags_by_name ( GTK_TEXT_BUFFER(buffer), &iter_end, chaine, -1,
                                                  Type_bit_interne(type_int), NULL );

       g_snprintf( chaine, sizeof(chaine), "- " );
       g_utf8_strncpy ( pivot, groupe_page, PRINT_NBR_CHAR_GROUPE_PAGE );
       len = g_utf8_strlen( pivot, -1 );
       g_strlcat ( chaine, pivot, sizeof(chaine) );
       if ( len <= PRINT_NBR_CHAR_GROUPE_PAGE )
        { for(cpt=len; cpt<=PRINT_NBR_CHAR_GROUPE_PAGE; cpt++) g_strlcat( chaine, " ", sizeof(chaine) ); }
       g_strlcat( chaine, "- ", sizeof(chaine) );

       g_utf8_strncpy ( pivot, acronyme, 20 );
       len = g_utf8_strlen( pivot, -1 );
       g_strlcat ( chaine, pivot, sizeof(chaine) );
       if ( len <= 20 ) { for(cpt=len; cpt<=20; cpt++) g_strlcat( chaine, " ", sizeof(chaine) ); }
       g_strlcat( chaine, "- ", sizeof(chaine) );

       g_strlcat( chaine, libelle, sizeof(chaine)-1 );
       g_strlcat( chaine, "\n", sizeof(chaine) );

       gtk_text_buffer_insert_at_cursor ( GTK_TEXT_BUFFER(buffer), chaine, -1 );
       g_free(type_string);
       g_free(groupe_page);
       g_free(acronyme);
       g_free(libelle);
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    compositor = gtk_source_print_compositor_new ( buffer );
    gtk_source_print_compositor_set_print_line_numbers ( compositor, 0 );
    gtk_source_print_compositor_set_body_font_name ( compositor, PRINT_FONT_NAME );
    gtk_source_print_compositor_set_print_header ( compositor, TRUE );
    gtk_source_print_compositor_set_header_format ( compositor, TRUE,
                                                    "Mnemoniques DLS",
                                                    "%F",
                                                    PRINT_HEADER_RIGHT);
    gtk_source_print_compositor_set_print_footer ( compositor, TRUE );
    gtk_source_print_compositor_set_footer_format ( compositor, TRUE,
                                                    PRINT_FOOTER_LEFT,
                                                    PRINT_FOOTER_CENTER,
                                                    PRINT_FOOTER_RIGHT);
    gtk_source_print_compositor_set_highlight_syntax ( compositor, TRUE );

    print = New_print_job ( "Print Mnemoniques" );
    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (Print_draw_page), compositor );
    g_signal_connect (G_OBJECT(print), "paginate",  G_CALLBACK (Print_paginate),  compositor );

    gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                             GTK_WINDOW(F_client), &error);
    g_object_unref(compositor);
    g_object_unref(buffer);
  }
/******************************************************************************************************************************/
/* Gerer_popup_mnemonique: Gestion du menu popup quand on clique droite sur la liste des mnemoniques                          */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                                                  */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Gerer_popup_mnemonique ( GtkWidget *widget, GdkEventButton *event, struct PAGE_NOTEBOOK *page )
  { static GtkWidget *Popup_select=NULL, *Popup_nonselect=NULL;
    struct TYPE_INFO_MNEMONIQUE *infos;
    GtkTreeSelection *selection;
    gboolean ya_selection;
    GtkTreePath *path;
    gint cellx, celly;

    if (!event) return(FALSE);

    infos = page->infos;
    if (!infos) return(FALSE);

    if ( event->button == 3 )                                                                             /* Gestion du popup */
     { if (!Popup_select)    Popup_select = gnome_popup_menu_new( Menu_popup_select );
       if (!Popup_nonselect) Popup_nonselect = gnome_popup_menu_new( Menu_popup_nonselect );

       ya_selection = FALSE;
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_mnemonique) );             /* On recupere selection*/
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(infos->Liste_mnemonique), event->x, event->y,
                                          &path, NULL, &cellx, &celly );
          
          if (path)
           { gtk_tree_selection_select_path( selection, path );
             gtk_tree_path_free( path );
             ya_selection = TRUE;
           }
        } else ya_selection = TRUE;                                                  /* ya bel et bien qqchose de selectionné */

       gnome_popup_menu_do_popup_modal( (ya_selection ? Popup_select : Popup_nonselect),
                                        NULL, NULL, event, NULL, F_client );
       return(TRUE);
     }
    else if (event->type == GDK_2BUTTON_PRESS && event->button == 1 )                                       /* Double clic ?? */
     { Menu_editer_mnemonique(); }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Creer_page_mnemonique: Creation de la page du notebook consacrée aux mnemoniques watchdog                                  */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Creer_page_mnemonique( struct CMD_TYPE_PLUGIN_DLS *plugin )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    struct TYPE_INFO_MNEMONIQUE *infos;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    gchar titre[120];
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->infos = (struct TYPE_INFO_MNEMONIQUE *)g_try_malloc0( sizeof(struct TYPE_INFO_MNEMONIQUE) );
    if (!page->infos) { g_free(page); return; }
    infos = (struct TYPE_INFO_MNEMONIQUE *)page->infos;
    if (plugin)
     { infos->id   = plugin->id;
       page->type  = TYPE_PAGE_MNEMONIQUE;
     }
    else page->type = TYPE_PAGE_ALL_MNEMONIQUE;
    
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/************************************************* La liste des mnemoniques ***************************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                                              /* Id */
                                              G_TYPE_UINT,                                                   /* Type (entier) */
                                              G_TYPE_STRING,                                         /* Type ("bistable"... ) */
                                              G_TYPE_STRING,                                                   /* Groupe_page */
                                              G_TYPE_UINT,                                                      /* Num_plugin */
                                              G_TYPE_STRING,                                                      /* Acronyme */
                                              G_TYPE_STRING,                                                       /* Tech_ID */
                                              G_TYPE_STRING,                                                       /* libellé */
                                              G_TYPE_STRING,                                                          /* host */
                                              G_TYPE_STRING,                                                        /* thread */
                                              G_TYPE_STRING,                                                  /* Command_text */
                                              G_TYPE_STRING,                                                       /* Tableau */
                                              GDK_TYPE_COLOR,                                                 /* couleur fond */
                                              GDK_TYPE_COLOR                                                 /* couleur texte */
                               );

    infos->Liste_mnemonique = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                   /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_mnemonique) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), infos->Liste_mnemonique );

    renderer = gtk_cell_renderer_text_new();                                                        /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Type/Num"), renderer,
                                                         "text", COLONNE_TYPE,
                                                         "background-gdk", COLONNE_COULEUR,
                                                         "foreground-gdk", COLONNE_COULEUR_TEXTE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_TYPE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Groupe/Page/D.L.S Name"), renderer,
                                                         "text", COLONNE_GROUPE_PAGE_DLS,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_GROUPE_PAGE_DLS);                            /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Acronyme"), renderer,
                                                         "text", COLONNE_ACRONYME,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ACRONYME);                                   /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Tech ID"), renderer,
                                                         "text", COLONNE_TECH_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_TECH_ID);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Description"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Event Host"), renderer,
                                                         "text", COLONNE_EV_HOST,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_EV_HOST);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Event Thread"), renderer,
                                                         "text", COLONNE_EV_THREAD,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_EV_THREAD);                                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Event Text"), renderer,
                                                         "text", COLONNE_EV_TEXT,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_EV_TEXT);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Tableau"), renderer,
                                                         "text", COLONNE_TABLEAU,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_TABLEAU);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (infos->Liste_mnemonique), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_mnemonique), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(infos->Liste_mnemonique), TRUE );                          /* Pour faire beau */

    g_signal_connect( G_OBJECT(infos->Liste_mnemonique), "button_press_event",                       /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_mnemonique), page );
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
                              G_CALLBACK(Menu_editer_mnemonique), NULL );

    if (plugin)
     { bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
       gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
       g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                                 G_CALLBACK(Menu_ajouter_mnemonique), NULL );
     }

    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_mnemonique), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_mnemonique), NULL );

    gtk_widget_show_all( hboite );
    if (plugin) g_snprintf( titre, sizeof(titre), "%s Mnemos", plugin->tech_id );
           else g_snprintf( titre, sizeof(titre), "Dictionnaire" );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( titre ) );
  }
/******************************************************************************************************************************/
/* Creer_page_all_mnemonique: Demande la creation de la page de tous les mnemoniques                                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Creer_page_all_mnemonique( void )
  { Creer_page_mnemonique(NULL); }
 /******************************************************************************************************************************/
/* Rafraichir_visu_mnemonique: Rafraichissement d'un mnemonique la liste à l'écran                                            */
/* Entrée: une reference sur le mnemonique                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Rafraichir_visu_mnemonique( GtkListStore *store, GtkTreeIter *iter, struct CMD_TYPE_MNEMO_BASE *mnemonique )
  { gchar chaine[60], groupe_page[512];
       
    if (mnemonique->num != -1)
     { g_snprintf( chaine, sizeof(chaine), "%s%04d", Type_bit_interne_court( mnemonique->type ), mnemonique->num ); }
    else
     { g_snprintf( chaine, sizeof(chaine), "%s", Type_bit_interne_court( mnemonique->type ) ); }

    g_snprintf( groupe_page, sizeof(groupe_page), "%s/%s/%s",
                mnemonique->syn_parent_page, mnemonique->syn_page, mnemonique->dls_shortname );

    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID,              mnemonique->id,
                         COLONNE_TYPE_INT,        mnemonique->type,
                         COLONNE_TYPE,            chaine,
                         COLONNE_GROUPE_PAGE_DLS, groupe_page,
                         COLONNE_NUM_PLUGIN,      mnemonique->dls_id,
                         COLONNE_ACRONYME,        mnemonique->acronyme,
                         COLONNE_TECH_ID,         mnemonique->dls_tech_id,
                         COLONNE_LIBELLE,         mnemonique->libelle,
                         COLONNE_EV_HOST,         mnemonique->ev_host,
                         COLONNE_EV_THREAD,       mnemonique->ev_thread,
                         COLONNE_EV_TEXT,         mnemonique->ev_text,
                         COLONNE_TABLEAU,         mnemonique->tableau,
                         COLONNE_COULEUR,         Couleur_bit_interne( mnemonique->type ),
                         COLONNE_COULEUR_TEXTE,   Couleur_texte_bit_interne( mnemonique->type ),
                         -1
                       );
  }
/******************************************************************************************************************************/
/* Afficher_un_mnemonique: Ajoute un mnemonique dans la liste des mnemoniques                                                 */
/* Entrée: une reference sur le mnemonique                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_mnemonique( struct CMD_TYPE_MNEMO_BASE *mnemonique )
  { struct TYPE_INFO_MNEMONIQUE *infos;
    struct PAGE_NOTEBOOK *page;
    GtkListStore *store;
    GtkTreeIter iter;

    page = Chercher_page_notebook( TYPE_PAGE_MNEMONIQUE, mnemonique->dls_id, FALSE );
    if (!page) return;

    infos = page->infos;
    if (!infos) return;

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(infos->Liste_mnemonique) ));
    gtk_list_store_append ( store, &iter );                                                          /* Acquisition iterateur */
    Rafraichir_visu_mnemonique ( store, &iter, mnemonique );
  }
/******************************************************************************************************************************/
/* Proto_afficher_tous_mnemonique: Ajoute un mnemonique dans la liste de tous les mnemoniques                                 */
/* Entrée: une reference sur le mnemonique                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_tous_mnemonique( struct CMD_TYPE_MNEMO_BASE *mnemonique )
  { struct TYPE_INFO_MNEMONIQUE *infos;
    struct PAGE_NOTEBOOK *page;
    GtkListStore *store;
    GtkTreeIter iter;

    page = Chercher_page_notebook( TYPE_PAGE_ALL_MNEMONIQUE, 0, FALSE );
    if (!page) return;

    infos = page->infos;
    if (!infos) return;

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(infos->Liste_mnemonique) ));
    gtk_list_store_append ( store, &iter );                                                          /* Acquisition iterateur */
    Rafraichir_visu_mnemonique ( store, &iter, mnemonique );
  }
/******************************************************************************************************************************/
/* Cacher_un_mnemonique: Enleve un mnemonique de la liste des mnemoniques                                                     */
/* Entrée: une reference sur le mnemonique                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_cacher_un_mnemonique( struct CMD_TYPE_MNEMO_BASE *mnemonique )
  { struct TYPE_INFO_MNEMONIQUE *infos;
    struct PAGE_NOTEBOOK *page;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    page = Chercher_page_notebook( TYPE_PAGE_MNEMONIQUE, mnemonique->dls_id, FALSE );
    if (!page) return;

    infos = page->infos;
    if (!infos) return;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(infos->Liste_mnemonique) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == mnemonique->id )
        { printf("elimination mnemonique %s\n", mnemonique->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/******************************************************************************************************************************/
/* Proto_rafrachir_un_mnemonique: Rafraichissement du mnemonique en parametre                                                 */
/* Entrée: une reference sur le mnemonique                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_rafraichir_un_mnemonique( struct CMD_TYPE_MNEMO_BASE *mnemonique )
  { struct TYPE_INFO_MNEMONIQUE *infos;
    struct PAGE_NOTEBOOK *page;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    page = Chercher_page_notebook( TYPE_PAGE_MNEMONIQUE, mnemonique->dls_id, FALSE );
    if (!page) return;

    infos = page->infos;
    if (!infos) return;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(infos->Liste_mnemonique) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == mnemonique->id )
        { printf("maj mnemonique %s\n", mnemonique->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_mnemonique( GTK_LIST_STORE(store), &iter, mnemonique ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
