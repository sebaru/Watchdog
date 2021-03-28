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

 #include "Reseaux.h"

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
  { { 0.687, 0.687, 0.687, 1.0 }, /* Info */
    { 0.5, 0.0, 0.0, 1.0 }, /* Alerte */
    { 1.0, 1.0, 0.0, 1.0 }, /* Trouble */
    { 1.0, 0.0, 0.0, 1.0 }, /* Alarme */
    { 0.0, 1.0, 0.0, 1.0 }, /* Veille */
    { 1.0, 1.0, 1.0, 1.0 }, /* Attente */
    { 0.0, 0.0, 1.0, 1.0 }, /* Danger */
    { 0.0, 0.0, 0.0, 1.0 }  /* Derangement */
  };
 GdkRGBA COULEUR_TEXTE[]=
  { { 0.0, 0.0, 0.0, 1.0 }, /* Info */
    { 1.0, 1.0, 1.0, 1.0 }, /* Alerte */
    { 0.0, 0.0, 0.0, 1.0 }, /* Trouble */
    { 0.0, 0.0, 0.0, 1.0 }, /* Alarme */
    { 0.0, 0.0, 0.0, 1.0 }, /* Veille */
    { 0.0, 0.0, 0.0, 1.0 }, /* Attente */
    { 1.0, 1.0, 1.0, 1.0 }, /* Danger */
    { 1.0, 1.0, 1.0, 1.0 }  /* Derangement */
  };
/**************************************** Définitions des prototypes programme ************************************************/
 #include "protocli.h"
 #include "client.h"

 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */
/******************************************************************************************************************************/
/* Typologie_vers_string: renvoie le type string associé                                                                           */
/* Entrée: le type numérique                                                                                                  */
/* Sortie: la chaine de caractère                                                                                             */
/******************************************************************************************************************************/
 gchar *Typologie_vers_string ( guint32 type )
  { switch (type)
     { case 0 : return( "Info        (I) " );
       case 1 : return( "Alerte      (AK)" );
       case 3 : return( "Défaut      (T) " );
       case 2 : return( "Alarme      (AL)" );
       case 4 : return( "Veille      (V) " );
       case 5 : return( "Attente     (A) " );
       case 6 : return( "Danger      (DA)" );
       case 7 : return( "Dérangement (DE)" );
     }
    return( "Unknown" );
  }
/******************************************************************************************************************************/
/* Menu_acquitter_histo: Acquittement d'un des messages histo                                                                 */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Acquitter_histo ( struct CLIENT *client )
  { GtkTreeSelection *selection;
    gchar *tech_id, *acronyme;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_histo) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_histo) );

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { gtk_tree_model_get_iter( store, &iter, lignes->data );          /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, COLONNE_ACRONYME, &acronyme, -1 );

       JsonBuilder *builder = Json_create ();
       if (builder == NULL) break;
       Json_add_string( builder, "tech_id", tech_id );
       Json_add_string( builder, "acronyme", acronyme );
       g_free(tech_id);
       g_free(acronyme);
       Envoi_json_au_serveur( client, "POST", builder, "/api/histo/ack", NULL );
       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/******************************************************************************************************************************/
/* Menu_go_to_syn: Affiche les synoptiques associés aux messages histo                                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Go_to_syn ( struct CLIENT *client )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gint syn_id;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_histo) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(client->Liste_histo) );

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { gtk_tree_model_get_iter( store, &iter, lignes->data );                              /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_SYN_ID, &syn_id, -1 );                                        /* Recup du id */

       Demander_synoptique_supervision ( client, syn_id );

       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/******************************************************************************************************************************/
/* Gerer_popup_message: Gestion du menu popup quand on clique droite sur la liste des messages                                */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                                                  */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Gerer_popup_histo ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { GtkWidget *Popup, *item, *hbox;
    struct CLIENT *client = data;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    gint cellx, celly;
    if (!event) return(FALSE);

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_histo) );                /* On recupere la selection */
    if (gtk_tree_selection_count_selected_rows(selection) == 0)
     { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(client->Liste_histo), event->x, event->y, &path, NULL, &cellx, &celly );
       if (path)
        { gtk_tree_selection_select_path( selection, path );
          gtk_tree_path_free( path );
        }
     }

    if (event->type == GDK_2BUTTON_PRESS && event->button == 1 )                                            /* Double clic ?? */
     { Go_to_syn(client);
       return(TRUE);
     }

    if ( event->button != 3 ) return(FALSE);                                                              /* Gestion du popup */

    Popup = gtk_menu_new();

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "emblem-default", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Acquitter le message"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    g_signal_connect_swapped ( item, "activate", G_CALLBACK (Acquitter_histo), client );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "emblem-web", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Voir le synoptique"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    g_signal_connect_swapped ( item, "activate", G_CALLBACK (Go_to_syn), client );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    gtk_widget_show_all(Popup);

    gtk_menu_popup_at_pointer ( GTK_MENU(Popup), (GdkEvent *)event );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Reset_page_histo: Efface les enregistrements de la page histo                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Reset_page_histo( struct CLIENT *client )
  { GtkTreeModel *store;
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(client->Liste_histo) );
    gtk_list_store_clear( GTK_LIST_STORE(store) );
    if (client->websocket)
     { soup_websocket_connection_close ( client->websocket, 0, "Thanks" );
       client->websocket = NULL;
     }
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
again:
    valide = gtk_tree_model_get_iter_first( store, &iter );
    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, COLONNE_ACRONYME, &acronyme, -1 );
       if ( !strcmp(tech_id, Json_get_string(element, "tech_id")) && !strcmp(acronyme,Json_get_string(element, "acronyme")) )
        { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); goto again; }
       g_free(tech_id);
       g_free(acronyme);
       valide = gtk_tree_model_iter_next( store, &iter );
     }
  }
/******************************************************************************************************************************/
/* Afficher_un_histo: Ajoute un histo sur la page fil de l'eau                                                                */
/* Entrée: un jsonnode representant l'histo                                                                                   */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Afficher_un_histo (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { gchar groupe_page[512];
    GtkTreePath *path;
    GdkPixbuf *pixbuf;
    GtkTreeIter iter;
    struct CLIENT *client = user_data;
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(client->Liste_histo) ));
    gtk_list_store_append ( store, &iter );

    g_snprintf( groupe_page, sizeof(groupe_page), "%s/%s",
                Json_get_string(element, "syn_parent_page"), Json_get_string(element, "syn_page") );

    if (!Json_has_member(element, "nom_ack"))
     { gtk_list_store_set ( GTK_LIST_STORE(store), &iter, COLONNE_ACK, "None", -1 ); }
    else
     { gchar ack[128];
       g_snprintf( ack, sizeof(ack), "%s (%s)", Json_get_string(element, "date_fixe"), Json_get_string(element, "nom_ack") );
       gtk_list_store_set ( GTK_LIST_STORE(store), &iter, COLONNE_ACK, ack, -1 );
     }

    switch (Json_get_int(element, "typologie"))
     { case 1: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Bouclier2_rouge.svg", 30, 30, TRUE, NULL ); break;
       case 2: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Pignon_jaune.svg", 30, 30, TRUE, NULL ); break;
       case 3: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Pignon_orange.svg", 30, 30, TRUE, NULL ); break;
       case 4: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Bouclier2_vert.svg", 30, 30, TRUE, NULL ); break;
       case 6: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Croix_rouge_rouge.svg", 30, 30, TRUE, NULL ); break;
       case 7: pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Croix_route_orange.svg", 30, 30, TRUE, NULL ); break;
       default:
       case 5:
       case 0:  pixbuf = gdk_pixbuf_new_from_resource_at_scale ( "/fr/abls_habitat/watchdog/icons/Info.svg", 30, 30, TRUE, NULL ); break;
     }

    gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                         COLONNE_TECH_ID, Json_get_string(element, "tech_id"),
                         COLONNE_ACRONYME, Json_get_string(element, "acronyme"),
                         COLONNE_TYPE, Typologie_vers_string(Json_get_int(element, "typologie")),
                         COLONNE_TYPE_PIXBUF, pixbuf,
                         COLONNE_SYN_ID, Json_get_int(element, "syn_id"),
                         COLONNE_GROUPE_PAGE, groupe_page,
                         COLONNE_DATE_CREATE, Json_get_string(element, "date_create"),
                         COLONNE_DLS_SHORTNAME, Json_get_string(element, "dls_shortname"),
                         COLONNE_LIBELLE, Json_get_string(element, "libelle"),
                         COLONNE_COULEUR_FOND, &COULEUR_FOND[Json_get_int(element, "typologie")],
                         COLONNE_COULEUR_TEXTE, &COULEUR_TEXTE[Json_get_int(element, "typologie")],
                         -1
                       );
    path = gtk_tree_model_get_path ( GTK_TREE_MODEL(store), &iter );
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW(client->Liste_histo), path, NULL, FALSE, 0.0, 0.0 );
    gtk_tree_path_free( path );
  }
/******************************************************************************************************************************/
/* Updater_histo_CB: Appeler suite a une requete d'acquit auprès du serveur                                                   */
/* Entrée: les parametres libsoup                                                                                             */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Updater_histo ( struct CLIENT *client, JsonNode *element )
  { GtkTreeIter iter;

    gchar *acronyme_recu = Json_get_string ( element, "acronyme" );
    gchar *tech_id_recu  = Json_get_string ( element, "tech_id" );

    if (!acronyme_recu || !tech_id_recu) return;

    if (Json_has_member(element,"alive") && Json_get_bool(element,"alive") == FALSE) { Cacher_un_histo ( client, element ); return; }

    GtkTreeModel *store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(client->Liste_histo) );
    gboolean valide      = gtk_tree_model_get_iter_first( store, &iter );
    gboolean found = FALSE;
    while ( valide )                                              /* A la recherche de l'iter perdu. Si trouvé, on met a jour */
     { gchar *tech_id, *acronyme;
       gtk_tree_model_get( store, &iter, COLONNE_TECH_ID, &tech_id, COLONNE_ACRONYME, &acronyme, -1 );
       if ( !strcmp(tech_id, tech_id_recu) && !strcmp(acronyme, acronyme_recu) )
        { gchar ack[80];
          found = TRUE;
          if(Json_has_member(element, "nom_ack"))
           { g_snprintf( ack, sizeof(ack), "%s (%s)", Json_get_string(element, "date_fixe"), Json_get_string(element, "nom_ack") );
             gtk_list_store_set ( GTK_LIST_STORE(store), &iter, COLONNE_ACK, ack, -1 );
           }
          if(Json_has_member(element, "date_create"))
           { gtk_list_store_set ( GTK_LIST_STORE(store), &iter, COLONNE_DATE_CREATE, Json_get_string(element, "date_create"), -1 ); }
        }
       g_free(tech_id);
       g_free(acronyme);
       valide = gtk_tree_model_iter_next( store, &iter );
     }
    if (!found) Afficher_un_histo( NULL, 0, element, client );                    /* Sinon on en affiche un nouveau complet ! */
  }
/******************************************************************************************************************************/
/* Traiter_reception_websocket_CB: Opere le traitement d'un message recu par la WebSocket MSGS                                  */
/* Entrée: rien                                                                                                               */
/* Sortie: un widget boite                                                                                                    */
/******************************************************************************************************************************/
 static void Traiter_histo_ws_CB ( SoupWebsocketConnection *self, gint type, GBytes *message_brut, gpointer user_data )
  { gsize taille;
    struct CLIENT *client = user_data;
    printf("%s: Recu WS: %s %p\n", __func__, g_bytes_get_data ( message_brut, &taille ), client );
    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response) return;

    gchar *zmq_tag = Json_get_string( response, "zmq_tag" );
    if (zmq_tag)
     {      if(!strcasecmp(zmq_tag,"DLS_HISTO"))         { Updater_histo( client, response ); }
       else if(!strcasecmp(zmq_tag,"PULSE"))             { Set_progress_pulse( client ); }
       else printf("%s: tag '%s' inconnu\n", __func__, zmq_tag );
     }
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Traiter_reception_websocket_CB: Opere le traitement d'un message recu par la WebSocket MSGS                                  */
/* Entrée: rien                                                                                                               */
/* Sortie: un widget boite                                                                                                    */
/******************************************************************************************************************************/
 static void Traiter_histo_ws_on_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { printf("%s\n", __func__ );
  }
 static void Traiter_histo_ws_on_error  ( SoupWebsocketConnection *connexion, GError *error, gpointer user_data )
  { struct CLIENT *client = user_data;
    printf("%s: WebSocket Error '%s' received !\n", __func__, error->message );
    Log( client, error->message );
  }
/******************************************************************************************************************************/
/* Traiter_connect_ws_CB: Termine la creation de la connexion websocket MSGS et raccorde le signal handler                    */
/* Entrée: les variables traditionnelles de libsous                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Traiter_histo_ws_on_connect (GObject *source_object, GAsyncResult *res, gpointer user_data )
  { struct CLIENT *client = user_data;
    GError *error = NULL;
    printf("%s\n", __func__ );
    client->websocket = soup_session_websocket_connect_finish ( client->connexion, res, &error );
    if (client->websocket)                                                                   /* No limit on incoming packet ! */
     { g_object_set ( G_OBJECT(client->websocket), "max-incoming-payload-size", G_GINT64_CONSTANT(0), NULL );
       g_signal_connect ( client->websocket, "message", G_CALLBACK(Traiter_histo_ws_CB), client );
       g_signal_connect ( client->websocket, "closed",  G_CALLBACK(Traiter_histo_ws_on_closed), client );
       g_signal_connect ( client->websocket, "error",   G_CALLBACK(Traiter_histo_ws_on_error), client );
     }
    else { printf("%s: Error opening Websocket '%s' !\n", __func__, error->message);
           g_error_free (error);
         }
  }
/******************************************************************************************************************************/
/* Afficher_histo_alive_CB: appeler par libsoup lorsque la requete de recuperation des histo alive a terminé                  */
/* Entrée: les parametres libsoup                                                                                             */
/* Sortie: un widget boite                                                                                                    */
/******************************************************************************************************************************/
 void Afficher_histo_alive_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { struct CLIENT *client = user_data;
    GBytes *response_brute;
    gchar *reason_phrase;
    gint status_code;
    gsize taille;
    printf("%s\n", __func__ );

    g_object_get ( msg, "status-code", &status_code, "reason-phrase", &reason_phrase, NULL );
    if (status_code != 200)
     { gchar chaine[256];
       g_snprintf(chaine, sizeof(chaine), "Error with get histo alive %s: Code %d - %s", client->hostname, status_code, reason_phrase );
       printf(chaine);
       return;
     }
    g_object_get ( msg, "response-body-data", &response_brute, NULL );

    printf("Recu MSGS: %s %p\n", (gchar *)g_bytes_get_data ( response_brute, &taille ), client );
    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( response_brute, &taille ) );
    if (!response || Json_has_member ( response, "enregs" ) == FALSE ) return;
    json_array_foreach_element ( Json_get_array(response, "enregs"), Afficher_un_histo, client );

    gchar chaine[256];
    g_snprintf(chaine, sizeof(chaine), "wss://%s:5560/api/live-motifs", client->hostname );
    soup_session_websocket_connect_async ( client->connexion, soup_message_new ( "GET", chaine ),
                                           NULL, NULL, g_cancellable_new(), Traiter_histo_ws_on_connect, client );
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

    hboite = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
/************************************************ La liste des groupes ********************************************************/
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

    client->Liste_histo = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                       /* Creation de la vue */
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

    renderer = gtk_cell_renderer_text_new();                                                         /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( "Timestamp", renderer,
                                                         "text", COLONNE_DATE_CREATE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_DATE_CREATE);
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE(store), COLONNE_DATE_CREATE, GTK_SORT_ASCENDING );
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                                         /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL ); 
    colonne = gtk_tree_view_column_new_with_attributes ( "DLS Shortname", renderer,
                                                         "text", COLONNE_DLS_SHORTNAME,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_DLS_SHORTNAME);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                                         /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( "Ack", renderer,
                                                         "text", COLONNE_ACK,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_ACK);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                                            /* Colonne du libelle */
    colonne = gtk_tree_view_column_new_with_attributes ( "Message", renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_histo), colonne );

    g_signal_connect( G_OBJECT(client->Liste_histo), "button_press_event",                           /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_histo), client ); 
    g_object_unref (G_OBJECT (store));                                            /* nous n'avons plus besoin de notre modele */

/******************************************** Les boutons de controles ********************************************************/
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
/*----------------------------------------------------------------------------------------------------------------------------*/
