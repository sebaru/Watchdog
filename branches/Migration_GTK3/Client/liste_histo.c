/******************************************************************************************************************************/
/* client/liste_message.c        Gestion de la page d'affichage des messages au fil de l'eau                                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          mer 20 aoû 2003 18:19:00 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * liste_histo.c
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

 #include <gtk/gtk.h>
 #include <time.h>

 #include "Config_cli.h"
 #include "Reseaux.h"

 extern struct CONFIG Config;                                                              /* Configuration generale watchdog */

 enum
  { COLONNE_TECH_ID,
    COLONNE_ACRONYME,
    COLONNE_GROUPE_PAGE,
    COLONNE_TYPE,
    COLONNE_TYPE_PIXBUF,
    COLONNE_SYN_ID,
    COLONNE_DATE_CREATE,
    COLONNE_DLS_SHORTNAME,
    COLONNE_ACK,
    COLONNE_LIBELLE,
    COLONNE_COULEUR_FOND,
    COLONNE_COULEUR_TEXTE,
    NBR_COLONNE
  };
 GdkRGBA COULEUR_FOND[]=
  { { 0.0, 0.0, 0.0, 0.2 }, /* Info */
    { 1.0, 0.0, 0.0, 1.0 }, /* Alerte */
    { 1.0, 1.0, 0.0, 0.5 }, /* Trouble */
    { 1.0, 0.6, 1.0, 0.8 }, /* Alarme */
    { 0.0, 1.0, 0.0, 0.8 }, /* Veille */
    { 0.0, 0.0, 0.0, 0.5 }, /* Attente */
    { 1.0, 0.0, 0.0, 0.8 }, /* Danger */
    { 0.0, 1.0, 0.5, 0.5 }  /* Derangement */
  };
 GdkRGBA COULEUR_TEXTE[]=
  { { 0.0, 0.0, 0.0, 1.0 }, /* Info */
    { 0.0, 0.0, 0.0, 1.0 }, /* Alerte */
    { 0.0, 0.0, 0.0, 1.0 }, /* Trouble */
    { 0.0, 0.0, 0.0, 1.0 }, /* Alarme */
    { 0.0, 0.0, 0.0, 1.0 }, /* Veille */
    { 0.0, 0.0, 0.0, 1.0 }, /* Attente */
    { 0.0, 0.0, 0.0, 1.0 }, /* Danger */
    { 0.0, 0.0, 0.0, 1.0 }  /* Derangement */
  };
/**************************************** Définitions des prototypes programme ************************************************/
 #include "protocli.h"
 #include "client.h"

 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */

/******************************************************************************************************************************/
/* Type_vers_string: renvoie le type string associé                                                                           */
/* Entrée: le type numérique                                                                                                  */
/* Sortie: la chaine de caractère                                                                                             */
/******************************************************************************************************************************/
 gchar *Type_vers_string ( guint32 type )
  { switch (type)
     { case MSG_ETAT        : return( "Info        (I) " );
       case MSG_ALERTE      : return( "Alerte      (AK)" );
       case MSG_ALARME      : return( "Alarme      (AL)" );
       case MSG_DEFAUT      : return( "Trouble     (T) " );
       case MSG_VEILLE      : return( "Veille      (V) " );
       case MSG_ATTENTE     : return( "Attente     (A) " );
       case MSG_DANGER      : return( "Danger      (DA)" );
       case MSG_DERANGEMENT : return( "Derangement (DE)" );
     }
    return( "Unknown" );
  }
/******************************************************************************************************************************/
/* Gerer_popup_message: Gestion du menu popup quand on clique droite sur la liste des messages                                */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                                                  */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Gerer_popup_histo ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { static GtkWidget *Popup=NULL;
    static GMenu *Model=NULL;
    struct CLIENT *client = data;
    GtkTreeSelection *selection;
    gboolean ya_selection;
    GtkTreePath *path;
    gint cellx, celly;
    if (!event) return(FALSE);

    if (!Model)
     { GtkWidget *item;
       Popup = gtk_menu_new();                                                                            /* Creation si besoin */
       item = gtk_menu_item_new_with_label ( "Acquitter le message" );
       g_signal_connect_swapped ( item, "activate", G_CALLBACK (Acquitter_histo), data );
       gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

       item = gtk_menu_item_new_with_label ( "Voir le synoptique" );
/*       g_signal_connect ( item, "activate", G_CALLBACK (Go_to_syn), NULL );*/
       gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
       gtk_widget_show_all(Popup);
     }

    ya_selection = FALSE;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_histo) );                        /* On recupere la selection */
    if (gtk_tree_selection_count_selected_rows(selection) == 0)
     { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(client->Liste_histo), event->x, event->y, &path, NULL, &cellx, &celly );
       if (path)
        { gtk_tree_selection_select_path( selection, path );
          gtk_tree_path_free( path );
          ya_selection = TRUE;
        }
     } else ya_selection = TRUE;                                                     /* ya bel et bien qqchose de selectionné */

    if ( event->button == 3 && ya_selection )                                                             /* Gestion du popup */
     { gtk_menu_popup_at_pointer ( GTK_MENU(Popup), (GdkEvent *)event );
       return(TRUE);
     }
    else if (event->type == GDK_2BUTTON_PRESS && event->button == 1 )                                       /* Double clic ?? */
     { /*Menu_go_to_syn();*/
       return(TRUE);
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Updater_histo_CB: Appeler suite a une requete d'acquit auprès du serveur                                                   */
/* Entrée: les parametres libsoup                                                                                             */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Updater_histo ( struct CLIENT *client, JsonNode *element )
  { gchar *tech_id, *acronyme;
    GtkTreeIter iter;

    gchar *acronyme_recu = Json_get_string ( element, "acronyme" );
    gchar *tech_id_recu  = Json_get_string ( element, "tech_id" );
    if (!acronyme || !tech_id) return;

    GtkTreeModel *store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(client->Liste_histo) );
    gboolean valide      = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )                                                    /* A la recherche de l'iter perdu */
     { gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, COLONNE_ACRONYME, &acronyme, -1 );
       if ( !strcmp(tech_id, tech_id_recu) && !strcmp(acronyme,acronyme_recu) )
        { gchar ack[80];
          g_snprintf( ack, sizeof(ack), "%s (%s)", Json_get_string(element, "date_fixe"), Json_get_string(element, "nom_ack") );
          gtk_list_store_set ( GTK_LIST_STORE(store), &iter, COLONNE_ACK, ack, -1 );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }
  }
/******************************************************************************************************************************/
/* Menu_acquitter_histo: Acquittement d'un des messages histo                                                                 */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Acquitter_histo ( struct CLIENT *client )
  { GtkTreeSelection *selection;
    gchar *tech_id, *acronyme;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_histo) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_histo) );

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { gsize taille_buf;
       gchar *buf;
       gtk_tree_model_get_iter( store, &iter, lignes->data );          /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, COLONNE_ACRONYME, &acronyme, -1 );

       JsonBuilder *builder = Json_create ();
       if (builder == NULL) break;
       json_builder_begin_object ( builder );
       Json_add_string( builder, "tech_id", tech_id );
       Json_add_string( builder, "acronyme", acronyme );
       json_builder_end_object (builder);                                                                        /* End Document */
       buf = Json_get_buf (builder, &taille_buf);
       Envoi_au_serveur( client, "POST", buf, taille_buf, "histo/ack", NULL );
       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
#ifdef bouh
/**********************************************************************************************************/
/* Menu_go_to_syn: Affiche les synoptiques associés aux messages histo                                    */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_go_to_syn ( void )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_histo) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_histo) );

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { guint id_syn;
       gtk_tree_model_get_iter( store, &iter, lignes->data );          /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_SYN_ID, &id_syn, -1 );                  /* Recup du id */

       Changer_vue_directe ( id_syn );

       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
#endif
/******************************************************************************************************************************/
/* Reset_page_histo: Efface les enregistrements de la page histo                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Reset_page_histo( struct CLIENT *client )
  { GtkTreeModel *store;
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(client->Liste_histo) );
    gtk_list_store_clear( GTK_LIST_STORE(store) );
  }
/******************************************************************************************************************************/
/* Cacher_un_histo: Enleve un histo de la liste fil de l'eau                                                                  */
/* Entrée: l'element a cacher                                                                                                 */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Cacher_un_histo( struct CLIENT *client, JsonNode *element )
  { gchar *tech_id, *acronyme;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(client->Liste_histo) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, COLONNE_ACRONYME, &acronyme, -1 );
       if ( !strcmp(tech_id, Json_get_string(element, "tech_id")) && !strcmp(acronyme,Json_get_string(element, "acronyme")) )
        { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); break; }
       valide = gtk_tree_model_iter_next( store, &iter );
     }
  }
/******************************************************************************************************************************/
/* Afficher_un_histo: Ajoute un histo sur la page fil de l'eau                                                                */
/* Entrée: un jsonnode representant l'histo                                                                                   */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Afficher_un_histo (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { gchar ack[128], groupe_page[512];
    GtkTreePath *path;
    GdkPixbuf *pixbuf;
    GtkTreeIter iter;
    struct CLIENT *client = user_data;
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(client->Liste_histo) ));
    gtk_list_store_append ( store, &iter );

    g_snprintf( groupe_page, sizeof(groupe_page), "%s/%s",
                Json_get_string(element, "syn_parent_page"), Json_get_string(element, "syn_page") );

    if (strcmp(Json_get_string(element, "nom_ack"),"None"))
     { g_snprintf( ack, sizeof(ack), "%s (%s)", Json_get_string(element, "date_fixe"), Json_get_string(element, "nom_ack") ); }
    else
     { g_snprintf( ack, sizeof(ack), "(%s)", Json_get_string(element, "nom_ack" ) ); }

    switch (Json_get_int(element, "type"))
     { case MSG_ALARME: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Pignon_orange.svg", 30, 30, TRUE, NULL ); break;
       case MSG_ALERTE: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Bouclier2_rouge.svg", 30, 30, TRUE, NULL ); break;
       case MSG_DANGER: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Croix_rouge_rouge.svg", 30, 30, TRUE, NULL ); break;
       case MSG_DEFAUT: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Pignon_jaune.svg", 30, 30, TRUE, NULL ); break;
       case MSG_DERANGEMENT: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Croix_route_orange.svg", 30, 30, TRUE, NULL ); break;
       default:
       case MSG_ATTENTE:
       case MSG_ETAT:  pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Info.svg", 30, 30, TRUE, NULL ); break;
     }

    gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                         COLONNE_TECH_ID, Json_get_string(element, "tech_id"),
                         COLONNE_ACRONYME, Json_get_string(element, "acronyme"),
                         COLONNE_TYPE, Type_vers_string(Json_get_int(element, "type")),
                         COLONNE_TYPE_PIXBUF, pixbuf,
                         COLONNE_SYN_ID, Json_get_int(element, "syn_id"),
                         COLONNE_GROUPE_PAGE, groupe_page,
                         COLONNE_DATE_CREATE, Json_get_string(element, "date_create"),
                         COLONNE_DLS_SHORTNAME, Json_get_string(element, "dls_shortname"),
                         COLONNE_ACK, ack,
                         COLONNE_LIBELLE, Json_get_string(element, "libelle"),
                         COLONNE_COULEUR_FOND, &COULEUR_FOND[Json_get_int(element, "type")],
                         COLONNE_COULEUR_TEXTE, &COULEUR_TEXTE[Json_get_int(element, "type")],
                         -1
                       );
    path = gtk_tree_model_get_path ( GTK_TREE_MODEL(store), &iter );
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW(client->Liste_histo), path, NULL, FALSE, 0.0, 0.0 );
    gtk_tree_path_free( path );
  }
/******************************************************************************************************************************/
/* Traiter_reception_ws_msgs_CB: Opere le traitement d'un message recu par la WebSocket MSGS                                  */
/* Entrée: rien                                                                                                               */
/* Sortie: un widget boite                                                                                                    */
/******************************************************************************************************************************/
 void Traiter_reception_ws_msgs_CB ( SoupWebsocketConnection *self, gint type, GBytes *message_brut, gpointer user_data )
  { gsize taille;
    struct CLIENT *client = user_data;
    printf("Recu MSGS: %s %p\n", g_bytes_get_data ( message_brut, &taille ), client );
    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response) return;

    gchar *zmq_type = Json_get_string( response, "zmq_type" );
    if (zmq_type)
     { if (!strcmp(zmq_type,"load_histo_alive"))
        { json_array_foreach_element ( Json_get_array(response, "enregs"), Afficher_un_histo, client ); }
       else if(!strcmp(zmq_type,"insert_or_delete_histo"))
        { if (Json_get_bool(response,"alive"))
           { Afficher_un_histo( NULL, 0, response, client ); }
          else { Cacher_un_histo ( client, response ); }
        }
       else if(!strcmp(zmq_type,"update_histo"))
        { Updater_histo( client, response ); }
       else if(!strcmp(zmq_type,"pulse"))
        { Set_progress_pulse( client ); }
     }
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: un widget boite                                                                                                    */
/******************************************************************************************************************************/
 GtkWidget *Creer_page_histo( struct CLIENT *client )
  { GtkTreeSelection *selection;
    GtkWidget *scroll, *hboite;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) { printf("Creer_page_histo: page = NULL !\n"); return(NULL); }

    page->type  = TYPE_PAGE_HISTO;
    client->Liste_pages = g_slist_append( client->Liste_pages, page );

    hboite = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
/***************************************** La liste des groupes *******************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_STRING,                                                       /* tech_ID */
                                              G_TYPE_STRING,                                                      /* Acronyme */
                                              G_TYPE_STRING,                                                   /* Groupe page */
                                              G_TYPE_STRING,                                                   /* Type String */
                                              GDK_TYPE_PIXBUF,
                                              G_TYPE_UINT,                                                         /* Num_syn */
                                              G_TYPE_STRING,                                                   /* date create */
                                              G_TYPE_STRING,                                                 /* DLS Shortname */
                                              G_TYPE_STRING,                                                           /* ACK */
                                              G_TYPE_STRING,                                                       /* Libelle */
                                              gdk_rgba_get_type(),                     /* Couleur de fond de l'enregistrement */
                                              gdk_rgba_get_type()                     /* Couleur du texte de l'enregistrement */
                               );

    client->Liste_histo = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                               /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_histo) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), client->Liste_histo );

    renderer = gtk_cell_renderer_text_new();                                                         /* Colonne du synoptique */
    colonne = gtk_tree_view_column_new_with_attributes ( "Groupe/Page", renderer,
                                                         "text", COLONNE_GROUPE_PAGE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_GROUPE_PAGE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                                         /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( "Type", renderer,
                                                         "text", COLONNE_TYPE,
                                                         "background-rgba", COLONNE_COULEUR_FOND,
                                                         "foreground-rgba", COLONNE_COULEUR_TEXTE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_TYPE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    renderer = gtk_cell_renderer_pixbuf_new();                                                       /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( "Type", renderer,
                                                         "pixbuf", COLONNE_TYPE_PIXBUF,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_TYPE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( "Timestamp", renderer,
                                                         "text", COLONNE_DATE_CREATE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_DATE_CREATE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( "DLS Shortname", renderer,
                                                         "text", COLONNE_DLS_SHORTNAME,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_DLS_SHORTNAME);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( "Ack", renderer,
                                                         "text", COLONNE_ACK,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_ACK);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                        /* Colonne du libelle */
    colonne = gtk_tree_view_column_new_with_attributes ( "Message", renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    g_signal_connect( G_OBJECT(client->Liste_histo), "button_press_event",               /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_histo), client );
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */

/************************************ Les boutons de controles ********************************************/
/*    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

/*    bouton = Bobouton( Verte, Vmask, _("Acknowledge") );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect( G_OBJECT(bouton), "clicked",
                      G_CALLBACK(Menu_acquitter_histo), NULL );
*/
    gtk_widget_show_all( hboite );
    return(hboite);
  }
/*--------------------------------------------------------------------------------------------------------*/
