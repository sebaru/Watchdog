/**********************************************************************************************************/
/* Client/liste_mnemonique.c        Configuration des mnemoniques de Watchdog v2.0                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 05 déc 2004 14:20:57 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_mnemonique.c
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
 #include <fcntl.h>
 #include <sys/stat.h>
 
 #include "Reseaux.h"

 static GtkWidget *Liste_mnemonique;              /* GtkTreeView pour la gestion des mnemoniques Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 enum
  {  COLONNE_ID,
     COLONNE_TYPE_INT,
     COLONNE_TYPE,
     COLONNE_OBJET,
     COLONNE_ACRONYME,
     COLONNE_LIBELLE,
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
  };
 static gchar *TYPE_BIT_INTERNE[ NBR_TYPE_MNEMO ]=          /* Type des différents bits internes utilisés */
  { "Bistable      B",
    "Monostable    M",
    "Temporisation TR",
    "Entree TOR    E",
    "Sortie TOR    A",
    "Entree ANA    EA",
    "Sortie ANA    AA",
    "Icone         I",
    "Compteur H    CH",
    "Compteur IMP  CI",
  };
 static gchar *TYPE_BIT_INTERNE_COURT[ NBR_TYPE_MNEMO ]=    /* Type des différents bits internes utilisés */
  { "B",
    "M",
    "TR",
    "E",
    "A",
    "EA",
    "AA",
    "I",
    "CH",
    "CI",
  };
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_mnemonique ( void );
 static void Menu_editer_mnemonique ( void );
 static void Menu_options_bit_interne ( void );
 static void Menu_ajouter_mnemonique ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_mnemonique, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_mnemonique, GNOME_STOCK_PIXMAP_PROPERTIES ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Options"), NULL, Menu_options_bit_interne, GNOME_STOCK_PIXMAP_INDEX ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_mnemonique, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_mnemonique, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_END
  };
/**********************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                          */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 static GdkColor *Couleur_bit_interne ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return( &COULEUR[0] );
    return( &COULEUR[num] );
  }
/**********************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                          */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 static GdkColor *Couleur_texte_bit_interne ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return( &COULEUR_TEXTE[0] );
    return( &COULEUR_TEXTE[num] );
  }
/**********************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                          */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 gchar *Type_bit_interne ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return("Type_bit_interne: Erreur interne");
    return( TYPE_BIT_INTERNE[num] );
  }
/**********************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                          */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 gint Type_bit_interne_int ( gchar *type )
  { gint cpt;
    for (cpt=0; cpt<NBR_TYPE_MNEMO; cpt++)
     { if ( !strcmp(type, TYPE_BIT_INTERNE[cpt]) ) return cpt; }
    printf("Type_bit_interne_int: pas trouvé\n");
    return(0);
  }
/**********************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                          */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 gchar *Type_bit_interne_court ( gint num )
  { if (num >= NBR_TYPE_MNEMO)
     return("Type_bit_interne_court: Erreur interne");
    return( TYPE_BIT_INTERNE_COURT[num] );
  }
/**********************************************************************************************************/
/* CB_effacer_mnemonique: Fonction appelée qd on appuie sur un des boutons de l'interface                 */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_mnemonique ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_MNEMONIQUE rezo_mnemonique;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_mnemonique) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_mnemonique) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_mnemonique.id, -1 );       /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

               memcpy( &rezo_mnemonique.libelle, libelle, sizeof(rezo_mnemonique.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_DEL_MNEMONIQUE,
                             (gchar *)&rezo_mnemonique, sizeof(struct CMD_TYPE_MNEMONIQUE) );
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
/* Menu_ajouter_mnemonique: Ajout d'un mnemonique                                                         */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_mnemonique ( void )
  { Menu_ajouter_editer_mnemonique(NULL);
  }
/**********************************************************************************************************/
/* Menu_effacer_mnemonique: Retrait des mnemoniques selectionnés                                          */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_mnemonique ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_mnemonique) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                       /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d mnemonique%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_mnemonique), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_options_bit_interne: Positionnement des optionslié au bit interne                                 */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_options_bit_interne ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_MNEMONIQUE rezo_mnemonique;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_mnemonique) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_mnemonique) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_mnemonique.id, -1 );               /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_TYPE_INT, &rezo_mnemonique.type, -1 );     /* Recup du type */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );
    memcpy( &rezo_mnemonique.libelle, libelle, sizeof(rezo_mnemonique.libelle) );
    g_free( libelle );
printf("on veut les options du bit_interne %d %s\n", rezo_mnemonique.type, rezo_mnemonique.libelle );

    Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_EDIT_OPTION_BIT_INTERNE,
                  (gchar *)&rezo_mnemonique, sizeof(struct CMD_TYPE_MNEMONIQUE) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Menu_editer_mnemonique: Demande d'edition du mnemonique selectionné                                    */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_mnemonique ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_MNEMONIQUE rezo_mnemonique;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_mnemonique) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_mnemonique) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_mnemonique.id, -1 );               /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_mnemonique.libelle, libelle, sizeof(rezo_mnemonique.libelle) );
    g_free( libelle );

    Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_EDIT_MNEMONIQUE,
                  (gchar *)&rezo_mnemonique, sizeof(struct CMD_TYPE_MNEMONIQUE) );
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
  { gchar *type_string, *objet, *libelle, *acronyme, *date_create, titre[128], chaine[128];
    GtkTreeModel *store;
    struct tm *temps;
    time_t timet;
    GdkColor *color;
    gboolean valide;
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
    g_snprintf( titre, sizeof(titre), " Watchdog - Mnemonique - %s - Page %d", date_create, page_nr );
    g_free( date_create );

    cairo_move_to( cr, 0.0, 0.0 );
    cairo_show_text (cr, titre );

    cairo_set_font_size (cr, PRINT_FONT_SIZE );
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_mnemonique) );
    valide = TRUE;
    y = 2 * PRINT_FONT_SIZE;
    while ( valide && y<gtk_print_context_get_height (context) )      /* Pour tous les objets du tableau */
     { gtk_tree_model_get( store, iter, COLONNE_TYPE, &type_string, COLONNE_OBJET, &objet,
                           COLONNE_ACRONYME, &acronyme, COLONNE_LIBELLE, &libelle,
                           COLONNE_COULEUR, &color, -1 );

       cairo_move_to( cr, 0.0*PRINT_FONT_SIZE, y );
       cairo_set_source_rgb (cr, color->red/65536.0, color->green/65536.0, color->blue/65536.0);
       cairo_show_text (cr, type_string );

       cairo_move_to( cr, 5.0*PRINT_FONT_SIZE, y );
       cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
       cairo_show_text (cr, objet );

       cairo_move_to( cr, 23.0*PRINT_FONT_SIZE, y );
       cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
       cairo_show_text (cr, acronyme );

       cairo_move_to( cr, 35.0*PRINT_FONT_SIZE, y );
       cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
       cairo_show_text (cr, libelle );

       g_free(type_string);
       g_free(objet);
       g_free(acronyme);
       g_free(libelle);

       valide = gtk_tree_model_iter_next( store, iter );
       y += PRINT_FONT_SIZE;
     }
  }
/**********************************************************************************************************/
/* Menu_exporter_message: Exportation de la base dans un fichier texte                                    */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Menu_exporter_mnemonique( void )
  { static GtkTreeIter iter;
    GtkPrintOperation *print;
    GtkPrintOperationResult res;
    GtkTreeModel *store;
    gboolean valide;
    GError *error;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_mnemonique) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    if (!valide) return;

    print = New_print_job ( "Print Mnemoniques" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (draw_page), &iter );
    g_signal_connect (G_OBJECT(print), "begin-print",
                      G_CALLBACK (Begin_print), GTK_TREE_VIEW(Liste_mnemonique) );

    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   GTK_WINDOW(F_client), &error);
  }
/**********************************************************************************************************/
/* Gerer_popup_mnemonique: Gestion du menu popup quand on clique droite sur la liste des mnemoniques      */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_mnemonique ( GtkWidget *widget, GdkEventButton *event, gpointer data )
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
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_mnemonique) );/* On recupere selection*/
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_mnemonique), event->x, event->y,
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
     { Menu_editer_mnemonique(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_page_mnemonique: Creation de la page du notebook consacrée aux mnemoniques watchdog              */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_mnemonique( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_MNEMONIQUE;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des mnemoniques ***************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* Id */
                                              G_TYPE_UINT,                               /* Type (entier) */
                                              G_TYPE_STRING,                     /* Type ("bistable"... ) */
                                              G_TYPE_STRING,                                     /* Objet */
                                              G_TYPE_STRING,                                  /* Acronyme */
                                              G_TYPE_STRING,                                   /* libellé */
                                              GDK_TYPE_COLOR,                             /* couleur fond */
                                              GDK_TYPE_COLOR                             /* couleur texte */
                               );

    Liste_mnemonique = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );      /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_mnemonique) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_mnemonique );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Type/Num"), renderer,
                                                         "text", COLONNE_TYPE,
                                                         "background-gdk", COLONNE_COULEUR,
                                                         "foreground-gdk", COLONNE_COULEUR_TEXTE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_TYPE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Object"), renderer,
                                                         "text", COLONNE_OBJET,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_OBJET);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Acronyme"), renderer,
                                                         "text", COLONNE_ACRONYME,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ACRONYME);               /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_mnemonique), colonne );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne du libelle de mnemonique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Description"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_mnemonique), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_mnemonique), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_mnemonique), TRUE );             /* Pour faire beau */

    g_signal_connect( G_OBJECT(Liste_mnemonique), "button_press_event",          /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_mnemonique), NULL );
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
                              G_CALLBACK(Menu_editer_mnemonique), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_mnemonique), NULL );

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
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Mnemoniques") ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_mnemonique: Rafraichissement d'un mnemonique la liste à l'écran                        */
/* Entrée: une reference sur le mnemonique                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_mnemonique( GtkTreeIter *iter, struct CMD_TYPE_MNEMONIQUE *mnemonique )
  { GtkTreeModel *store;
    gchar chaine[60];
       
    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_mnemonique) );          /* Acquisition du modele */
    g_snprintf( chaine, sizeof(chaine), "%s%04d",
                Type_bit_interne_court( mnemonique->type ), mnemonique->num );

    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID,       mnemonique->id,
                         COLONNE_TYPE_INT, mnemonique->type,
                         COLONNE_TYPE,     chaine,
                         COLONNE_OBJET,    mnemonique->objet,
                         COLONNE_ACRONYME, mnemonique->acronyme,
                         COLONNE_LIBELLE,  mnemonique->libelle,
                         COLONNE_COULEUR,  Couleur_bit_interne( mnemonique->type ),
                         COLONNE_COULEUR_TEXTE, Couleur_texte_bit_interne( mnemonique->type ),
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_mnemonique: Ajoute un mnemonique dans la liste des mnemoniques                             */
/* Entrée: une reference sur le mnemonique                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_mnemonique( struct CMD_TYPE_MNEMONIQUE *mnemonique )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_MNEMONIQUE)) Creer_page_mnemonique();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_mnemonique) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_mnemonique ( &iter, mnemonique );
  }
/**********************************************************************************************************/
/* Cacher_un_mnemonique: Enleve un mnemonique de la liste des mnemoniques                                 */
/* Entrée: une reference sur le mnemonique                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_mnemonique( struct CMD_TYPE_MNEMONIQUE *mnemonique )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_mnemonique) );
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
/**********************************************************************************************************/
/* Proto_rafrachir_un_mnemonique: Rafraichissement du mnemonique en parametre                             */
/* Entrée: une reference sur le mnemonique                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_mnemonique( struct CMD_TYPE_MNEMONIQUE *mnemonique )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_mnemonique) );
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
     { Rafraichir_visu_mnemonique( &iter, mnemonique ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
