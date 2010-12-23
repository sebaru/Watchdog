/**********************************************************************************************************/
/* Client/liste_modbus.c        Configuration des modbuss de Watchdog v2.0                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 05 sept. 2010 16:47:20 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_modbus.c
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

 static GtkWidget *Liste_modbus;                      /* GtkTreeView pour la gestion des modbuss Watchdog */
 static GtkWidget *Liste_bornes_modbus;         /* GtkTreeView pour la gestion des bornes modbus Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

 enum                                                       /* Numéro des colonnes dans les listes modbus */
  {  COLONNE_ID,
     COLONNE_ACTIF,
     COLONNE_WATCHDOG,
     COLONNE_BIT,
     COLONNE_IP,
     COLONNE_LIBELLE,
     NBR_COLONNE
  };
 enum                                                       /* Numéro des colonnes dans les listes bornes */
  {  COLONNE_BORNE_ID,
     COLONNE_BORNE_MODULE,
     COLONNE_BORNE_TYPE,
     COLONNE_BORNE_TYPE_STRING,
     COLONNE_BORNE_ADRESSE,
     COLONNE_BORNE_MIN,
     COLONNE_BORNE_NBR,
     NBR_COLONNE_BORNE
  };

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_modbus ( void );
 static void Menu_editer_modbus ( void );
 static void Menu_ajouter_modbus ( void );
 static void Menu_exporter_modbus ( void );
 static void Menu_effacer_borne_modbus ( void );
 static void Menu_lister_borne_modbus ( void );
 static void Menu_ajouter_borne_modbus ( void );
 static void Menu_editer_borne_modbus ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_modbus, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_modbus, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit bornes"), NULL, Menu_lister_borne_modbus, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Print"), NULL, Menu_exporter_modbus, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_modbus, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_modbus, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Print"), NULL, Menu_exporter_modbus, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_select_borne[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_borne_modbus, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_borne_modbus, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_borne_modbus, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect_borne[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_borne_modbus, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_END
  };

 static gchar* Mode_borne [ NBR_MODE_BORNE ] =
  { "Input TOR",
    "Output TOR",
    "Input ANA",
    "Output ANA"
  };

/**********************************************************************************************************/
/* Type_ea_vers_string: renvoie le type d'ea sous forme de chaine de caractere                            */
/* Entrée: numéro du type d'entree ANA                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 gchar *Mode_borne_vers_string ( guint type )
  { if (type<NBR_MODE_BORNE) return( Mode_borne[type] );
                        else return( "Unknown" );
  }
/**********************************************************************************************************/
/* CB_effacer_modbus: Fonction appelée qd on appuie sur un des boutons de l'interface                     */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_modbus ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_MODBUS rezo_modbus;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_modbus) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_modbus) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_modbus.id, -1 );        /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

               memcpy( &rezo_modbus.libelle, libelle, sizeof(rezo_modbus.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_MODBUS, SSTAG_CLIENT_DEL_MODBUS,
                             (gchar *)&rezo_modbus, sizeof(struct CMD_TYPE_MODBUS) );
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
/* Menu_ajouter_modbus: Ajout d'un modbus                                                                 */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_modbus ( void )
  { Menu_ajouter_editer_modbus(NULL); }
/**********************************************************************************************************/
/* Menu_effacer_modbus: Retrait des modbuss selectionnés                                                  */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_modbus ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_modbus) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    printf("Menu effacer modbus: nbr=%d\n", nbr );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d module%c modbus ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_modbus), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_editer_modbus: Demande d'edition du modbus selectionné                                            */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_modbus ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_MODBUS rezo_modbus;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_modbus) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_modbus) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_modbus.id, -1 );                   /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_modbus.libelle, libelle, sizeof(rezo_modbus.libelle) );
    g_free( libelle );
    Envoi_serveur( TAG_MODBUS, SSTAG_CLIENT_EDIT_MODBUS,
                  (gchar *)&rezo_modbus, sizeof(struct CMD_TYPE_MODBUS) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* CB_effacer_borne_modbus: Fonction appelée qd on appuie sur un des boutons de l'interface               */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_borne_modbus ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_BORNE_MODBUS rezo_borne;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_bornes_modbus) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_bornes_modbus) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_BORNE_ID, &rezo_borne.id, -1 );   /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_BORNE_MODULE, &rezo_borne.module, -1 );/* Recup du id */

               Envoi_serveur( TAG_MODBUS, SSTAG_CLIENT_DEL_BORNE_MODBUS,
                             (gchar *)&rezo_borne, sizeof(struct CMD_TYPE_BORNE_MODBUS) );
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
/* Menu_ajouter_borne_modbus: Ajout d'une borne modbus                                                    */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_borne_modbus ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_BORNE_MODBUS rezo_borne;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_modbus) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_modbus) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_BORNE_MODULE, &rezo_borne.module, -1 );/* Recup du id module */

    Menu_ajouter_editer_borne_modbus( FALSE, &rezo_borne );
   }
/**********************************************************************************************************/
/* Menu_effacer_borne_modbus: Retrait des bornes modbus selectionnés                                      */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_borne_modbus ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_bornes_modbus) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    printf("Menu effacer borne modbus: nbr=%d\n", nbr );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d borne%c modbus ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_borne_modbus), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_lister_borne_modbus: Liste les bornes d'un module modbus                                          */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_lister_borne_modbus ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_MODBUS rezo_modbus;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_bornes_modbus) );
    gtk_list_store_clear( GTK_LIST_STORE(store) );

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_modbus) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_modbus) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_modbus.id, -1 );                   /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_modbus.libelle, libelle, sizeof(rezo_modbus.libelle) );
    g_free( libelle );
    Envoi_serveur( TAG_MODBUS, SSTAG_CLIENT_WANT_BORNE_MODBUS,
                  (gchar *)&rezo_modbus, sizeof(struct CMD_TYPE_MODBUS) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Menu_editer_borne_modbus: Demande d'edition d'une borne d'un module modbus                             */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_borne_modbus ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_BORNE_MODBUS rezo_borne;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_bornes_modbus) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_bornes_modbus) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_BORNE_ID, &rezo_borne.id, -1 );              /* Recup du id */

    Envoi_serveur( TAG_MODBUS, SSTAG_CLIENT_EDIT_BORNE_MODBUS,
                  (gchar *)&rezo_borne, sizeof(struct CMD_TYPE_BORNE_MODBUS) );
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
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_modbus) );
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
/* Menu_exporter_modbus: Exportation de la base dans un fichier texte                                     */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Menu_exporter_modbus( void )
  { static GtkTreeIter iter;
    GtkPrintOperation *print;
    GtkPrintOperationResult res;
    GtkTreeModel *store;
    gboolean valide;
    GError *error;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_modbus) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    if (!valide) return;

    print = New_print_job ( "Print Cameras" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (draw_page), &iter );
    g_signal_connect (G_OBJECT(print), "begin-print",
                      G_CALLBACK (Begin_print), GTK_TREE_VIEW(Liste_modbus) );

    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   GTK_WINDOW(F_client), &error);
  }
/**********************************************************************************************************/
/* Gerer_popup_modbus: Gestion du menu popup quand on clique droite sur la liste des modbuss              */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_modbus ( GtkWidget *widget, GdkEventButton *event, gpointer data )
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
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_modbus) );   /* On recupere selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_modbus), event->x, event->y,
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
     { Menu_editer_modbus(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Gerer_popup_borne_modbus: Gestion du menu popup quand on clique droite sur la liste des bornes modbus  */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_borne_modbus ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { static GtkWidget *Popup_select=NULL, *Popup_nonselect=NULL;
    GtkTreeSelection *selection;
    gboolean ya_selection;
    GtkTreePath *path;
    gint cellx, celly;
    if (!event) return(FALSE);

    if ( event->button == 3 )                                                         /* Gestion du popup */
     { if (!Popup_select)    Popup_select = gnome_popup_menu_new( Menu_popup_select_borne );
       if (!Popup_nonselect) Popup_nonselect = gnome_popup_menu_new( Menu_popup_nonselect_borne );

       ya_selection = FALSE;
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_bornes_modbus) );/* On recupere selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_bornes_modbus), event->x, event->y,
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
     { Menu_editer_borne_modbus(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_liste_modbus: Creation de la liste du notebook consacrée aux modbuss watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_liste_modbus( GtkWidget **Liste, GtkWidget **Scroll )
  { GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    GtkWidget *scroll, *liste;

    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    *Scroll = scroll;

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* Id */
                                              G_TYPE_BOOLEAN,                                    /* actif */
                                              G_TYPE_UINT,                                    /* Watchdog */
                                              G_TYPE_UINT,                                         /* bit */
                                              G_TYPE_STRING,                                        /* IP */
                                              G_TYPE_STRING                                    /* libelle */
                               );

    liste = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                 /* Creation de la vue */
    *Liste = liste;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(liste) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), liste );

    renderer = gtk_cell_renderer_toggle_new();                              /* Colonne de l'id du message */
    colonne = gtk_tree_view_column_new_with_attributes ( _("ON"), renderer,
                                                         "active", COLONNE_ACTIF,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ACTIF);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_modbus), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de modbus */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Name/IP"), renderer,
                                                         "text", COLONNE_IP,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_IP);                     /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de modbus */
    g_object_set ( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Bit Comm"), renderer,
                                                         "text", COLONNE_BIT,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_BIT );                   /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de modbus */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Watchdog"), renderer,
                                                         "text", COLONNE_WATCHDOG,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_WATCHDOG );              /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de modbus */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Libelle"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_modbus), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(liste), TRUE );                        /* Pour faire beau */
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
  }
/**********************************************************************************************************/
/* Creer_page_modbus: Creation de la page du notebook consacrée aux modbuss watchdog                      */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_modbus( void )
  { GtkWidget *boite, *vboite, *scroll, *hboite, *bouton, *separateur;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_MODBUS;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), vboite, TRUE, TRUE, 0 );

/***************************************** La liste des modbuss *******************************************/
    Creer_liste_modbus( &Liste_modbus, &scroll );
    gtk_box_pack_start( GTK_BOX(vboite), scroll, TRUE, TRUE, 0 );
    g_signal_connect( G_OBJECT(Liste_modbus), "button_press_event",              /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_modbus), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(vboite), separateur, FALSE, FALSE, 0 );

    Creer_liste_bornes_modbus( &Liste_bornes_modbus, &scroll );
    gtk_box_pack_start( GTK_BOX(vboite), scroll, TRUE, TRUE, 0 );
    g_signal_connect( G_OBJECT(Liste_bornes_modbus), "button_press_event",       /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_borne_modbus), NULL );

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
                              G_CALLBACK(Menu_editer_modbus), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_modbus), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_modbus), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_modbus), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Edit MODBUS") ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_modbus: Rafraichissement d'un modbus la liste à l'écran                                */
/* Entrée: une reference sur le modbus                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Rafraichir_visu_modbus( GtkListStore *store, GtkTreeIter *iter, struct CMD_TYPE_MODBUS *modbus )
  { gtk_list_store_set ( store, iter,
                         COLONNE_ID, modbus->id,
                         COLONNE_ACTIF, modbus->actif,
                         COLONNE_LIBELLE, modbus->libelle,
                         COLONNE_BIT, modbus->bit,
                         COLONNE_IP, modbus->ip,
                         COLONNE_WATCHDOG, modbus->watchdog,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_modbus: Ajoute un modbus dans la liste des modbuss                                         */
/* Entrée: une reference sur le modbus                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_modbus( struct CMD_TYPE_MODBUS *modbus )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_MODBUS)) Creer_page_modbus();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_modbus) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_modbus ( store, &iter, modbus );
  }
/**********************************************************************************************************/
/* Cacher_un_modbus: Enleve un modbus de la liste des modbuss                                             */
/* Entrée: une reference sur le modbus                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_modbus( struct CMD_TYPE_MODBUS *modbus )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_modbus) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == modbus->id )
        { printf("elimination modbus %s\n", modbus->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_modbus: Rafraichissement du modbus en parametre                                     */
/* Entrée: une reference sur le modbus                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_modbus( struct CMD_TYPE_MODBUS *modbus )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_modbus) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == modbus->id )
        { printf("maj modbus %s\n", modbus->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_modbus( GTK_LIST_STORE(store), &iter, modbus ); }
  }
/**********************************************************************************************************/
/* Creer_liste_bornes_modbus: Creation de la liste des bornes modbus                                      */
/* Entrée: Le retour de la liste, le scroll qui va bien                                                   */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_liste_bornes_modbus( GtkWidget **Liste, GtkWidget **Scroll )
  { GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    GtkWidget *scroll, *liste;

    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    *Scroll = scroll;

    store = gtk_list_store_new ( NBR_COLONNE_BORNE,
                                                    G_TYPE_UINT,                                    /* Id */
                                                    G_TYPE_UINT,                                /* Module */
                                                    G_TYPE_UINT,                                  /* Type */
                                                    G_TYPE_STRING,                         /* Type String */
                                                    G_TYPE_UINT,                               /* Adresse */
                                                    G_TYPE_UINT,                                   /* Min */
                                                    G_TYPE_UINT                                    /* Nbr */
                               );

    liste = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                 /* Creation de la vue */
    *Liste = liste;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(liste) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), liste );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de modbus */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Mode"), renderer,
                                                         "text", COLONNE_BORNE_TYPE_STRING,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_BORNE_TYPE_STRING);      /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de modbus */
    g_object_set ( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Adress"), renderer,
                                                         "text", COLONNE_BORNE_ADRESSE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_BORNE_ADRESSE );         /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de modbus */
    colonne = gtk_tree_view_column_new_with_attributes ( _("First I/O bit"), renderer,
                                                         "text", COLONNE_BORNE_MIN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_BORNE_MIN );             /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de modbus */
    colonne = gtk_tree_view_column_new_with_attributes ( _("I/O Number"), renderer,
                                                         "text", COLONNE_BORNE_NBR,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_BORNE_NBR );             /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (liste), colonne );

    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(liste), TRUE );                        /* Pour faire beau */
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
  }
/**********************************************************************************************************/
/* Rafraichir_visu_borne_modbus: Rafraichissement d'une borne modbus à l'écran                            */
/* Entrée: une reference sur la borne                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Rafraichir_visu_borne_modbus( GtkListStore *store, GtkTreeIter *iter, struct CMD_TYPE_BORNE_MODBUS *borne )
  { gtk_list_store_set ( store, iter,
                         COLONNE_BORNE_ID, borne->id,
                         COLONNE_BORNE_MODULE, borne->module,
                         COLONNE_BORNE_TYPE, borne->type,
                         COLONNE_BORNE_TYPE_STRING, Mode_borne_vers_string(borne->type),
                         COLONNE_BORNE_ADRESSE, borne->adresse,
                         COLONNE_BORNE_MIN, borne->min,
                         COLONNE_BORNE_NBR, borne->nbr,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Proto_afficher_une_borne_modbus: Affiche une nouvelle borne dans la liste                              */
/* Entrée: une reference sur le modbus                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_une_borne_modbus( struct CMD_TYPE_BORNE_MODBUS *borne )
  { GtkListStore *store;
    GtkTreeIter iter;

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_bornes_modbus) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_borne_modbus ( store, &iter, borne );
  }
/**********************************************************************************************************/
/* Cacher_une_borne_modbus: Enleve une borne modbus de la liste des modbuss                               */
/* Entrée: une reference sur le modbus                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_une_borne_modbus( struct CMD_TYPE_BORNE_MODBUS *borne )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_bornes_modbus) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == borne->id ) break;
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_une_borne_modbus: Rafraichissement de la borne modbus en parametre                     */
/* Entrée: une reference sur la borne                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_une_borne_modbus( struct CMD_TYPE_BORNE_MODBUS *borne )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_bornes_modbus) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == borne->id ) break;
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_borne_modbus( GTK_LIST_STORE(store), &iter, borne ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
