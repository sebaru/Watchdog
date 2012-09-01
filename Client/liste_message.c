/**********************************************************************************************************/
/* Client/liste_message.c        Configuration des messages de Watchdog v2.0                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 23 fév 2008 11:25:00 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_message.c
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

 GtkWidget *Liste_message;                           /* GtkTreeView pour la gestion des messages Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

 enum
  {  COLONNE_NOTINHIB,
     COLONNE_SMS,
     COLONNE_ID,
     COLONNE_AUDIO,
     COLONNE_REPEAT,
     COLONNE_NUM,
     COLONNE_TYPE_INT,
     COLONNE_TYPE_STRING,
     COLONNE_GROUPE_PAGE,
     COLONNE_LIBELLE,
     COLONNE_LIBELLE_AUDIO,
     COLONNE_LIBELLE_SMS,
     COLONNE_COULEUR_FOND,
     COLONNE_COULEUR_TEXTE,
     NBR_COLONNE
  };
 extern GdkColor COULEUR_FOND[];
 extern GdkColor COULEUR_TEXTE[];
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_message ( void );
 static void Menu_editer_message ( void );
 static void Menu_ajouter_message ( void );
 static void Menu_exporter_message ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_message, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_editer_message, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Export"), NULL, Menu_exporter_message, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_effacer_message, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add"), NULL, Menu_ajouter_message, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Print"), NULL, Menu_exporter_message, GNOME_STOCK_PIXMAP_PRINT ),
    GNOMEUIINFO_END
  };

/**********************************************************************************************************/
/* Type_vers_string: renvoie le type string associé                                                       */
/* Entrée: le type numérique                                                                              */
/* Sortie: la chaine de caractère                                                                         */
/**********************************************************************************************************/
 gchar *Type_vers_string ( guint32 type )
  { switch (type)
     { case MSG_ETAT     : return( _("Info    (I)") );
       case MSG_ALERTE   : return( _("Alerte  (AK)") );
       case MSG_ALARME   : return( _("Alarme  (AL)") );
       case MSG_DEFAUT   : return( _("Trouble (T)") );
       case MSG_VEILLE   : return( _("Veille  (V)") );
       case MSG_ATTENTE  : return( _("Attente (A)") );
       case MSG_DANGER   : return( _("Danger  (D)") );
     }
    return( _("Unknown") );
  }
/**********************************************************************************************************/
/* Type_sms_vers_string: renvoie le type de sms associé                                                   */
/* Entrée: le type numérique                                                                              */
/* Sortie: la chaine de caractère                                                                         */
/**********************************************************************************************************/
 gchar *Type_sms_vers_string ( guint32 type )
  { switch (type)
     { case MSG_SMS_NONE  : return( _("-- Aucun SMS --") );
       case MSG_SMS_GSM   : return( _("- SMS via GSM- ") );
       case MSG_SMS_SMSBOX: return( _("SMS via SMSBOX ") );
     }
    return( _("Unknown") );
  }
/**********************************************************************************************************/
/* CB_effacer_message: Fonction appelée qd on appuie sur un des boutons de l'interface                    */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_message ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_MESSAGE rezo_message;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_message) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_message) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_message.id, -1 );       /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

               memcpy( &rezo_message.libelle, libelle, sizeof(rezo_message.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_DEL_MESSAGE,
                             (gchar *)&rezo_message, sizeof(struct CMD_TYPE_MESSAGE) );
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
/* Menu_ajouter_message: Ajout d'un message                                                               */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_message ( void )
  { Menu_ajouter_editer_message(NULL); }
/**********************************************************************************************************/
/* Menu_effacer_message: Retrait des messages selectionnés                                                */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_message ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_message) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    printf("Menu effacer message: nbr=%d\n", nbr );
    if (!nbr) return;                                                       /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d message%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_message), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_editer_message: Demande d'edition du message selectionné                                          */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_editer_message ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_MESSAGE rezo_message;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_message) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_message) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_message.id, -1 );                  /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_message.libelle, libelle, sizeof(rezo_message.libelle) );
    g_free( libelle );
    Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_EDIT_MESSAGE,
                  (gchar *)&rezo_message, sizeof(struct CMD_TYPE_MESSAGE) );
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
  { gchar *num, *type_string, *groupe_page, *libelle, *date_create, titre[128], chaine[128];
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
    g_snprintf( titre, sizeof(titre), " Watchdog - Messages - %s - Page %d", date_create, page_nr+1 );
    g_free( date_create );

    cairo_move_to( cr, 0.0, 0.0 );
    cairo_show_text (cr, titre );

    cairo_set_font_size (cr, PRINT_FONT_SIZE );
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_message) );
    valide = TRUE;
    y = 2 * PRINT_FONT_SIZE;
    while ( valide && y<gtk_print_context_get_height (context) )      /* Pour tous les groupe_pages du tableau */
     { gtk_tree_model_get( store, iter, COLONNE_NOTINHIB, &enable, COLONNE_NUM, &num,
                           COLONNE_SMS, &sms,
                           COLONNE_TYPE_INT, &type_int, COLONNE_TYPE_STRING, &type_string,
                           COLONNE_GROUPE_PAGE, &groupe_page, COLONNE_LIBELLE, &libelle, -1 );

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
       cairo_show_text (cr, groupe_page );

       cairo_move_to( cr, 38.0*PRINT_FONT_SIZE, y );  /* Libelle */
       cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
       cairo_show_text (cr, libelle );

       g_free(num);
       g_free(type_string);
       g_free(groupe_page);
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
 static void Menu_exporter_message( void )
  { static GtkTreeIter iter;
    GtkPrintOperation *print;
    GtkPrintOperationResult res;
    GtkTreeModel *store;
    gboolean valide;
    GError *error;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_message) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    if (!valide) return;

    print = New_print_job ( "Print Messages" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (draw_page), &iter );
    g_signal_connect (G_OBJECT(print), "begin-print",
                      G_CALLBACK (Begin_print), GTK_TREE_VIEW(Liste_message) );

    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   GTK_WINDOW(F_client), &error);
  }
/**********************************************************************************************************/
/* Gerer_popup_message: Gestion du menu popup quand on clique droite sur la liste des messages            */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_message ( GtkWidget *widget, GdkEventButton *event, gpointer data )
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
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_message) );  /* On recupere selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_message), event->x, event->y,
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
     { Menu_editer_message(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_message( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_MESSAGE;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des messages ******************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_BOOLEAN,                               /* Not_inhibe */
                                              G_TYPE_STRING,                                       /* SMS */
                                              G_TYPE_UINT,                                          /* Id */
                                              G_TYPE_STRING,                                     /* Audio */
                                              G_TYPE_UINT,                                      /* Repeat */
                                              G_TYPE_STRING,                                       /* Num */
                                              G_TYPE_UINT,                                 /* Int du type */
                                              G_TYPE_STRING,                            /* String du Type */
                                              G_TYPE_STRING,                               /* Groupe Page */
                                              G_TYPE_STRING,                                   /* Libelle */
                                              G_TYPE_STRING,                             /* Libelle_audio */
                                              G_TYPE_STRING,                               /* Libelle_sms */
                                              GDK_TYPE_COLOR,      /* Couleur de fond de l'enregistrement */
                                              GDK_TYPE_COLOR      /* Couleur du texte de l'enregistrement */
                               );

    Liste_message = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );         /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_message) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_message );

    renderer = gtk_cell_renderer_toggle_new();                              /* Colonne de l'id du message */
    colonne = gtk_tree_view_column_new_with_attributes ( _("ON"), renderer,
                                                         "active", COLONNE_NOTINHIB,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NOTINHIB);               /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_message), colonne );

    renderer = gtk_cell_renderer_text_new();                                /* Colonne de l'id du message */
    colonne = gtk_tree_view_column_new_with_attributes ( _("SMS"), renderer,
                                                         "text", COLONNE_SMS,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_SMS);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_message), colonne );

    renderer = gtk_cell_renderer_text_new();                             /* Colonne du libelle de message */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Msg"), renderer,
                                                         "text", COLONNE_NUM,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NUM);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_message), colonne );

    renderer = gtk_cell_renderer_text_new();                             /* Colonne du libelle de message */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Audio"), renderer,
                                                         "text", COLONNE_AUDIO,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_AUDIO);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_message), colonne );

    renderer = gtk_cell_renderer_text_new();                             /* Colonne du libelle de message */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Repeat"), renderer,
                                                         "text", COLONNE_REPEAT,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_REPEAT);                 /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_message), colonne );

    renderer = gtk_cell_renderer_text_new();                             /* Colonne du libelle de message */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Type"), renderer,
                                                         "text", COLONNE_TYPE_STRING,
                                                         "background-gdk", COLONNE_COULEUR_FOND,
                                                         "foreground-gdk", COLONNE_COULEUR_TEXTE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_TYPE_STRING);                   /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_message), colonne );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Groupe / Page"), renderer,
                                                         "text", COLONNE_GROUPE_PAGE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_GROUPE_PAGE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_message), colonne );

    renderer = gtk_cell_renderer_text_new();                             /* Colonne du libelle de message */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Message"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_message), colonne );

    renderer = gtk_cell_renderer_text_new();                             /* Colonne du libelle de message */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Message Audio"), renderer,
                                                         "text", COLONNE_LIBELLE_AUDIO,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE_AUDIO);          /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_message), colonne );

    renderer = gtk_cell_renderer_text_new();                             /* Colonne du libelle de message */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Message SMS"), renderer,
                                                         "text", COLONNE_LIBELLE_SMS,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE_SMS);            /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_message), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_message), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_message), TRUE );                /* Pour faire beau */

    g_signal_connect( G_OBJECT(Liste_message), "button_press_event",             /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_message), NULL );
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
                              G_CALLBACK(Menu_editer_message), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_message), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_message), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_message), NULL );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Edit Msg") ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_message: Rafraichissement d'un message la liste à l'écran                              */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_message( GtkTreeIter *iter, struct CMD_TYPE_MESSAGE *message )
  { GtkTreeModel *store;
    gchar chaine[10], audio[10], groupe_page[512];

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_message) );             /* Acquisition du modele */

    g_snprintf( chaine, sizeof(chaine), "%04d", message->num);
    if (message->bit_voc)
     { g_snprintf( audio, sizeof(audio), "%s%04d", Type_bit_interne_court(MNEMO_MONOSTABLE), message->bit_voc ); }
    else
     { g_snprintf( audio, sizeof(audio), "- no -"); }

    g_snprintf( groupe_page, sizeof(groupe_page), "%s/%s", message->groupe, message->page );

    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_NOTINHIB, message->enable,
                         COLONNE_SMS, Type_sms_vers_string(message->sms),
                         COLONNE_ID, message->id,
                         COLONNE_NUM, chaine,
                         COLONNE_REPEAT, message->time_repeat,
                         COLONNE_AUDIO, audio,
                         COLONNE_TYPE_INT, message->type,
                         COLONNE_TYPE_STRING, Type_vers_string(message->type),
                         COLONNE_GROUPE_PAGE, groupe_page,
                         COLONNE_LIBELLE, message->libelle,
                         COLONNE_LIBELLE_AUDIO, message->libelle_audio,
                         COLONNE_LIBELLE_SMS, message->libelle_sms,
                         COLONNE_COULEUR_FOND, &COULEUR_FOND[message->type],
                         COLONNE_COULEUR_TEXTE, &COULEUR_TEXTE[message->type],
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_message( struct CMD_TYPE_MESSAGE *message )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_MESSAGE)) Creer_page_message();

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_message) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_message ( &iter, message );
  }
/**********************************************************************************************************/
/* Cacher_un_message: Enleve un message de la liste des messages                                          */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_message( struct CMD_TYPE_MESSAGE *message )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_message) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == message->id )
        { printf("elimination message %s\n", message->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_message: Rafraichissement du message en parametre                                   */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_message( struct CMD_TYPE_MESSAGE *message )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_message) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == message->id )
        { printf("maj message %s\n", message->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_message( &iter, message ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
