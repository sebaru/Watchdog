/**********************************************************************************************************/
/* Client/liste_icone.c        Configuration des icones de Watchdog v2.0                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 01 nov 2003 13:11:17 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_icone.c
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
 #include <sys/time.h>
 
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>
 
 #include "Reseaux.h"
 #include "client.h"
 #include "liste_icone.h"

 GtkWidget *Liste_classe;                             /* GtkTreeView pour la gestion des classes Watchdog */
 static GtkWidget *Liste_icone;                        /* GtkTreeView pour la gestion des icones Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern struct CLIENT Client;                           /* Identifiant de l'utilisateur en cours */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 enum
  {  COLONNE_ID,
     COLONNE_LIBELLE,
     COLONNE_CLASSE_ID_ICONE,
     NBR_COLONNE
  };
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_icone ( void );
 static void Menu_editer_icone ( void );
 static void Menu_ajouter_icone ( void );
 static void Menu_effacer_classe ( void );
 static void Menu_editer_classe ( void );
 static void Menu_ajouter_classe ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_icone, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_icone, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_icone, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_icone, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_classe_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_classe, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_classe, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_classe, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_classe_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_classe, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_END
  };
/**********************************************************************************************************/
/* CB_effacer_icone: Fonction appelée qd on appuie sur un des boutons de l'interface                 */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_icone ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_ICONE rezo_icone;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_icone) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_icone) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_icone.id, -1 );         /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

               memcpy( &rezo_icone.libelle, libelle, sizeof(rezo_icone.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_ICONE, SSTAG_CLIENT_DEL_ICONE,
                             (gchar *)&rezo_icone, sizeof(struct CMD_TYPE_ICONE) );
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
/* Menu_ajouter_icone: Ajout d'un icone                                                                   */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_icone ( void )
  { Menu_ajouter_editer_icone(NULL);
  }
/**********************************************************************************************************/
/* Menu_effacer_icone: Retrait des icones selectionnés                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_icone ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_icone) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    printf("Menu_effacer_icone: nbr=%d\n", nbr );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d icone%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_icone), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_editer_icone: Demande d'edition du icone selectionné                                              */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_icone ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_ICONE rezo_icone;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_icone) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_icone) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_icone.id, -1 );                    /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_icone.libelle, libelle, sizeof(rezo_icone.libelle) );
    g_free( libelle );
printf("on veut editer le icone %s\n", rezo_icone.libelle );
    Envoi_serveur( TAG_ICONE, SSTAG_CLIENT_EDIT_ICONE,
                  (gchar *)&rezo_icone, sizeof(struct CMD_TYPE_ICONE) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Gerer_popup_icone: Gestion du menu popup quand on clique droite sur la liste des icones                */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_icone ( GtkWidget *widget, GdkEventButton *event, gpointer data )
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
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_icone) );/* On recupere selection*/
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_icone), event->x, event->y,
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
     { Menu_editer_icone(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* CB_effacer_icone: Fonction appelée qd on appuie sur un des boutons de l'interface                 */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_classe ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_CLASSE rezo_classe;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;
    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_classe) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_classe) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_CLASSE_ID, &rezo_classe.id, -1 ); /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_CLASSE_LIBELLE, &libelle, -1 );

               memcpy( &rezo_classe.libelle, libelle, sizeof(rezo_classe.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_ICONE, SSTAG_CLIENT_DEL_CLASSE,
                             (gchar *)&rezo_classe, sizeof(struct CMD_TYPE_CLASSE) );
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
/* Menu_ajouter_icone: Ajout d'un icone                                                                   */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_classe ( void )
  { Menu_ajouter_editer_classe(NULL);
  }
/**********************************************************************************************************/
/* Menu_effacer_icone: Retrait des icones selectionnés                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_classe ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_classe) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d classe%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_classe), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_editer_icone: Demande d'edition du icone selectionné                                              */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_classe ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_CLASSE rezo_classe;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_classe) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_classe) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_CLASSE_ID, &rezo_classe.id, -1 );            /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_CLASSE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_classe.libelle, libelle, sizeof(rezo_classe.libelle) );
    g_free( libelle );
printf("on veut editer le classe %s\n", rezo_classe.libelle );
    Envoi_serveur( TAG_ICONE, SSTAG_CLIENT_EDIT_CLASSE,
                  (gchar *)&rezo_classe, sizeof(struct CMD_TYPE_CLASSE) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Menu_editer_icone: Demande d'edition du icone selectionné                                              */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Clic_classe ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_CLASSE rezo_classe;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_icone) );
    gtk_list_store_clear( GTK_LIST_STORE(store) );

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_classe) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_classe) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    printf("nbr=%d\n", nbr );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */
printf("bouh\n");
    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    printf("lignes = %p\n", lignes );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_CLASSE_ID, &rezo_classe.id, -1 );            /* Recup du id */
    printf("on veut printer la classe %d\n", rezo_classe.id );
    Envoi_serveur( TAG_ICONE, SSTAG_CLIENT_WANT_PAGE_ICONE,
                   (gchar *)&rezo_classe, sizeof(struct CMD_TYPE_CLASSE) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Gerer_popup_icone: Gestion du menu popup quand on clique droite sur la liste des icones                */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_classe ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { static GtkWidget *Popup_select=NULL, *Popup_nonselect=NULL;
    GtkTreeSelection *selection;
    gboolean ya_selection;
    GtkTreePath *path;
    gint cellx, celly;
    if (!event) return(FALSE);

    if ( event->button == 3 )                                                         /* Gestion du popup */
     { if (!Popup_select)    Popup_select = gnome_popup_menu_new( Menu_classe_popup_select );
       if (!Popup_nonselect) Popup_nonselect = gnome_popup_menu_new( Menu_classe_popup_nonselect );

       ya_selection = FALSE;
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_classe) );    /* On recupere selection*/
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_classe), event->x, event->y,
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
     { Menu_editer_classe(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_page_icone: Creation de la page du notebook consacrée aux icones watchdog                        */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_icone( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_ICONE;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des classes *******************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE_CLASSE, G_TYPE_UINT,                      /* Id de la classe */
                                                     G_TYPE_STRING                              /* Classe */
                               );

    Liste_classe = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );          /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_classe) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_classe );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("ClassId"), renderer,
                                                         "text", COLONNE_CLASSE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_CLASSE_ID);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_classe), colonne );

    renderer = gtk_cell_renderer_text_new();                                  /* Colonne de l'id du icone */
    colonne = gtk_tree_view_column_new_with_attributes ( _("ClassName"), renderer,
                                                         "text", COLONNE_CLASSE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_CLASSE_LIBELLE);         /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_classe), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_icone), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_classe), TRUE );                 /* Pour faire beau */

    g_signal_connect( G_OBJECT(Liste_classe), "button_press_event",              /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_classe), NULL );
    g_signal_connect( G_OBJECT(selection), "changed",                            /* Gestion du menu popup */
                      G_CALLBACK(Clic_classe), NULL );
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */

/***************************************** La liste des icones ********************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* Id */
                                              G_TYPE_STRING,                                   /* libellé */
                                              G_TYPE_UINT                              /* Classe id icone */
                               );

    Liste_icone = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );           /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_icone) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_icone );

    renderer = gtk_cell_renderer_text_new();                                  /* Colonne de l'id du icone */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("IconeId"), renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                     /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_icone), colonne );

    renderer = gtk_cell_renderer_text_new();                               /* Colonne du libelle de icone */
    colonne = gtk_tree_view_column_new_with_attributes ( _("IconName"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_icone), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_icone), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_icone), TRUE );                  /* Pour faire beau */

    g_signal_connect( G_OBJECT(Liste_icone), "button_press_event",               /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_icone), NULL );
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
                              G_CALLBACK(Menu_editer_icone), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_icone), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_icone), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_with_label( _("Add class") );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_classe), NULL );

    bouton = gtk_button_new_with_label( _("Edit class") );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_editer_classe), NULL );

    bouton = gtk_button_new_with_label( _("Del class") );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_classe), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Outils") ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_icone: Rafraichissement d'un icone la liste à l'écran                                  */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_icone( GtkTreeIter *iter, struct CMD_TYPE_ICONE *icone )
  { GtkTreeModel *store;
    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_icone) );          /* Acquisition du modele */
    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, icone->id,
                         COLONNE_LIBELLE, icone->libelle,
                         COLONNE_CLASSE_ID_ICONE, icone->id_classe,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_icone: Ajoute un icone dans la liste des icones                                            */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_icone( struct CMD_TYPE_ICONE *icone )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gint id, nbr;

    if (!Tester_page_notebook(TYPE_PAGE_ICONE)) Creer_page_icone();

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_classe) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_classe) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */
    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_CLASSE_ID, &id, -1 );                        /* Recup du id */

    if(id!=icone->id_classe) return;

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_icone) );
    gtk_list_store_append ( GTK_LIST_STORE(store), &iter );                      /* Acquisition iterateur */

    Rafraichir_visu_icone ( &iter, icone );
  }
/**********************************************************************************************************/
/* Cacher_un_icone: Enleve un icone de la liste des icones                                                */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_icone( struct CMD_TYPE_ICONE *icone )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_icone) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == icone->id )
        { printf("elimination icone %s\n", icone->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_icone: Rafraichissement du icone en parametre                                       */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_icone( struct CMD_TYPE_ICONE *icone )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id, classe_id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_icone) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == icone->id )
        { gtk_tree_model_get( store, &iter, COLONNE_CLASSE_ID_ICONE, &classe_id, -1 );
          if (icone->id_classe != classe_id)                       /* L'icone a-t-il changer de classe ?? */
           { printf("changement de classe!\n");
             gtk_list_store_remove( GTK_LIST_STORE(store), &iter );
             return;
           }
          else break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_icone( &iter, icone ); }
  }
/**********************************************************************************************************/
/* Rafraichir_visu_classe: Rafraichissement d'une classe la liste à l'écran                               */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_classe( GtkTreeIter *iter, struct CMD_TYPE_CLASSE *classe )
  { GtkTreeModel *store;
    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_classe) );          /* Acquisition du modele */
    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_CLASSE_ID, classe->id,
                         COLONNE_CLASSE_LIBELLE, classe->libelle,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_icone: Ajoute un icone dans la liste des icones                                            */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_une_classe( struct CMD_TYPE_CLASSE *classe )
  { GtkTreeModel *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_ICONE)) Creer_page_icone();

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_classe) );

    gtk_list_store_append ( GTK_LIST_STORE(store), &iter );                      /* Acquisition iterateur */
    Rafraichir_visu_classe ( &iter, classe );
  }
/**********************************************************************************************************/
/* Cacher_un_icone: Enleve un icone de la liste des icones                                                */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_une_classe( struct CMD_TYPE_CLASSE *classe )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_classe) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_CLASSE_ID, &id, -1 );
       if ( id == classe->id )
        { printf("elimination classe %s\n", classe->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_icone: Rafraichissement du icone en parametre                                       */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_une_classe( struct CMD_TYPE_CLASSE *classe )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_classe) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == classe->id )
        { break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_classe( &iter, classe ); }
  }
/**********************************************************************************************************/
/* Proto_envoyer_gif: Procedure d'envoi d'un fichier gif au serveur                                       */
/* Entrée: la requete serveur sous forme de struct CMD_TYPE_ICONE                                          */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Proto_envoyer_gif ( struct CMD_TYPE_ICONE *icone )
  { gchar from_fichier[256], to_fichier[256];
    gint source, cible, taille;
    gchar buffer[1024];
    gchar fichier[256];
    guint cpt_frame;

    memcpy ( &fichier, icone->nom_fichier, sizeof(fichier) );

    for (cpt_frame=0; ; cpt_frame++)
     {
       if (cpt_frame==0)
        { g_snprintf( from_fichier, sizeof(from_fichier), "%s", fichier );
          g_snprintf( to_fichier, sizeof(to_fichier), "%d.gif", icone->id );
        }
       else
        { g_snprintf( from_fichier, sizeof(from_fichier), "%s.%02d", fichier, cpt_frame );
          g_snprintf( to_fichier, sizeof(to_fichier), "%d.gif.%02d", icone->id, cpt_frame );
        }

       printf("Proto_envoyer_gif1: id=%d, nom_fichier=%s to_fichier=%s\n",
               icone->id, from_fichier, to_fichier );
       source = open( from_fichier, O_RDONLY );
       if (source<0) return;


       unlink(to_fichier);
       cible = open( to_fichier, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR );
       if (cible<0)
        { close(source); printf(" Erreur création fichier\n"); return; }
       while( (taille = read( source, buffer, sizeof(buffer) )) )
        { write( cible, buffer, taille ); }
       close(source);
       close(cible);

       memcpy ( &icone->nom_fichier, to_fichier, sizeof(icone->nom_fichier) );
       memcpy ( buffer, icone, sizeof(struct CMD_TYPE_ICONE) );
       source = open( to_fichier, O_RDONLY );
       Envoi_serveur( TAG_ICONE, SSTAG_CLIENT_ADD_ICONE_DEB_FILE,
                      (gchar *)icone, sizeof(struct CMD_TYPE_ICONE) );

       while ( (taille = read( source, buffer + sizeof(struct CMD_TYPE_ICONE),
                                sizeof(buffer) - sizeof(struct CMD_TYPE_ICONE) ) ) > 0 )
        { Envoi_serveur( TAG_ICONE, SSTAG_CLIENT_ADD_ICONE_FILE,
                         (gchar *)&buffer, taille + sizeof(struct CMD_TYPE_ICONE) );
          printf("Envoi données gif au serveur: %d\n", taille );
        }
       close(source);
       Envoi_serveur( TAG_ICONE, SSTAG_CLIENT_ADD_ICONE_FIN_FILE,
                      (gchar *)&buffer, sizeof(struct CMD_TYPE_ICONE) );
       Client.mode = VALIDE;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
