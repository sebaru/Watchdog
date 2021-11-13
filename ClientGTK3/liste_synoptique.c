/******************************************************************************************************************************/
/* Client/liste_synoptique.c        Configuration des synoptiques de Watchdog v3.0                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           dim 02 nov 2003 17:21:39 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * liste_synoptique.c
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

 enum
  {  COLONNE_ID,
     COLONNE_ACCESS_LEVEL,
     COLONNE_PPAGE,
     COLONNE_PAGE,
     COLONNE_LIBELLE,
     NBR_COLONNE
  };
/***************************************** Définitions des prototypes programme ***********************************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Afficher_un_plugin: Rafraichissement d'un synoptique dans la liste à l'écran                                               */
/* Entrée: une reference sur le synoptique au format json                                                                     */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Afficher_un_synoptique (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct CLIENT *client=user_data;
    GtkTreeIter iter;

    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(client->Liste_synoptique) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */

    gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                         COLONNE_ID, Json_get_int( element, "id" ),
                         COLONNE_ACCESS_LEVEL, Json_get_int( element, "access_level" ),
                         COLONNE_PPAGE, Json_get_string( element, "ppage" ),
                         COLONNE_PAGE, Json_get_string( element, "page" ),
                         COLONNE_LIBELLE, Json_get_string( element, "libelle" ),
                         -1
                       );
  }
/******************************************************************************************************************************/
/* Synoptique_edited_CB: Appelé après la mise a jour/création d'un synoptique                                                 */
/* Entrée: les données issues de la librairie libsoup                                                                         */
/* sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Synoptique_edited_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { struct CLIENT *client = user_data;
    GBytes *response_brute;
    gchar *reason_phrase;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gint status_code;
    gboolean valide;
    gsize taille;
    gint id;
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

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(client->Liste_synoptique) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == Json_get_int(response, "id" ) ) break;
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                            COLONNE_ACCESS_LEVEL, Json_get_int( response, "access_level" ),
                            COLONNE_PPAGE, Json_get_string( response, "ppage" ),
                            COLONNE_PAGE, Json_get_string( response, "page" ),
                            COLONNE_LIBELLE, Json_get_string( response, "libelle" ),
                            -1
                          );
     }
    else Afficher_un_synoptique( NULL, 0, response, client );                     /* Sinon on en affiche un nouveau complet ! */
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Creer_Ajouter_Editer_synoptique: Affiche la fenetre d'edition des proprietes synoptique                                    */
/* Entrée: la page cliente et le synoptique au format JSON                                                                    */
/* sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Creer_Fenetre_Ajouter_Editer_synoptique ( struct CLIENT *client, JsonNode *syn, gchar *syn_parent )
  { gint i;
    GtkWidget *dialog = gtk_dialog_new_with_buttons( (syn ? "Editer le synoptique" : "Ajouter un synoptique"),
                                                     GTK_WINDOW(client->window),
                                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                     "Annuler", GTK_RESPONSE_CANCEL,
                                                     (syn ? "Valider" : "Ajouter"), GTK_RESPONSE_OK,
                                                     NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
    gtk_box_pack_start (GTK_BOX (content_area), hbox, FALSE, FALSE, 0);

    GtkWidget *image = gtk_image_new_from_icon_name ("preferences-system", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

    GtkWidget *table = gtk_grid_new();
    gtk_box_pack_start( GTK_BOX(hbox), table, TRUE, TRUE, 0 );
    gtk_grid_set_row_spacing( GTK_GRID(table), 5 );
    gtk_grid_set_column_spacing( GTK_GRID(table), 5 );
    /*gtk_grid_set_column_homogeneous ( GTK_GRID(table), TRUE );*/

    i=0;
    GtkWidget *texte = gtk_label_new( "Parent" );
    gtk_grid_attach( GTK_GRID(table), texte, 0, i, 1, 1 );
    GtkWidget *Entry_ppage = gtk_entry_new();
    gtk_editable_set_editable( GTK_EDITABLE(Entry_ppage), FALSE );
    gtk_grid_attach( GTK_GRID(table), Entry_ppage, 1, i, 2, 1 );

    i++;
    texte = gtk_label_new( "Page" );
    gtk_grid_attach( GTK_GRID(table), texte, 0, i, 1, 1 );
    GtkWidget *Entry_page = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_page), 32 );
    gtk_entry_set_placeholder_text ( GTK_ENTRY(Entry_page), "Entrez le nom court" );
    gtk_widget_set_tooltip_text ( Entry_page, "Entrez le nom court" );
    /*gtk_entry_set_progress_fraction (GTK_ENTRY(Entry_page), 0.5 );*/
    gtk_grid_attach( GTK_GRID(table), Entry_page, 1, i, 1, 1 );

/*    texte = gtk_label_new( "Access Level" );                                       /* Création du spin du niveau de clearance */
/*    gtk_grid_attach( GTK_GRID(table), texte, 0, i, 1, 1 );*/
    GtkWidget *Spin_access_level = gtk_spin_button_new_with_range( 0, client->access_level-1, 1 );
    gtk_widget_set_tooltip_text ( Spin_access_level, "Niveau d'accès minimum" );
    gtk_grid_attach( GTK_GRID(table),  Spin_access_level, 2, i, 1, 1 );

    i++;
    texte = gtk_label_new( "Description" );
    gtk_grid_attach( GTK_GRID(table), texte, 0, i, 1, 1 );
    GtkWidget *Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), 128 );
    gtk_entry_set_placeholder_text ( GTK_ENTRY(Entry_lib), "Entrez la description du synoptique" );
    gtk_widget_set_tooltip_text ( Entry_lib, "Entrez la description du synoptique" );
    gtk_grid_attach( GTK_GRID(table), Entry_lib, 1, i, 2, 1 );

    g_signal_connect_swapped( Entry_lib, "activate", G_CALLBACK(gtk_widget_grab_focus), Entry_page );

    if (syn)                                                                                    /* Si edition d'un synoptique */
     { gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_access_level), Json_get_int ( syn, "access_level" ) );
       gtk_entry_set_text( GTK_ENTRY(Entry_lib), Json_get_string ( syn, "libelle" ) );
       gtk_entry_set_text( GTK_ENTRY(Entry_page), Json_get_string ( syn, "page" ) );
       gtk_entry_set_text( GTK_ENTRY(Entry_ppage), Json_get_string ( syn, "ppage" ) );
     }
    else
     { gtk_entry_set_text( GTK_ENTRY(Entry_ppage), syn_parent ); }

    gtk_widget_grab_focus( Entry_lib );
    gtk_widget_show_all( dialog );

    if (gtk_dialog_run( GTK_DIALOG(dialog) ) == GTK_RESPONSE_OK)                       /* Attente de reponse de l'utilisateur */
     { JsonBuilder *builder = Json_create ();
       if (builder)
        { Json_add_string ( builder, "libelle", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
          Json_add_string ( builder, "page", gtk_entry_get_text( GTK_ENTRY(Entry_page) ) );
          Json_add_string ( builder, "ppage", gtk_entry_get_text( GTK_ENTRY(Entry_ppage) ) );
          Json_add_int    ( builder, "access_level", gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_access_level) ) );
          if (syn) { Json_add_int ( builder, "id", Json_get_int( syn, "id" ) ); }
          Envoi_json_au_serveur( client, "POST", builder, "/api/syn/edit", Synoptique_edited_CB );
        }
     }
    gtk_widget_destroy( dialog );
  }
/******************************************************************************************************************************/
/* Show_Proprietes_synoptique: Edite les propriétés d'un synoptique                                                           */
/* Entrée: les données issues de libsoup                                                                                      */
/* sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Show_Proprietes_synoptique_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
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
    Creer_Fenetre_Ajouter_Editer_synoptique ( client, response, NULL );
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Menu_editer_synoptique: Demande d'edition du synoptique selectionné                                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_Want_Proprietes_synoptique ( struct PAGE_NOTEBOOK *page )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr, id;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(page->client->Liste_synoptique) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(page->client->Liste_synoptique) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );                                                   /* Recup du id */

    gchar chaine[64];
    g_snprintf ( chaine, sizeof(chaine), "/api/syn/get?syn_id=%d", id  );
    Envoi_json_au_serveur( page->client, "GET", NULL, chaine, Show_Proprietes_synoptique_CB );

    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/******************************************************************************************************************************/
/* Synoptique_deleted_CB: Met à jour la liste synoptique en fonction des parametres recu du serveur                           */
/* Entrée: les données issues de la librairie libsoup                                                                         */
/* sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Synoptique_deleted_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { struct CLIENT *client = user_data;
    GBytes *response_brute;
    gchar *reason_phrase;
    GtkTreeModel *store;
    GtkTreeIter iter;
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

    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(client->Liste_synoptique) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == Json_get_int(response, "id" ) ) break;
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }

    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Send_Effacer_synoptique: Fonction appelée qd on appuie sur le bouton de suppression et que l'ona validé l'ordre            */
/* Entrée: la page source                                                                                                     */
/* sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Send_Effacer_synoptique ( struct PAGE_NOTEBOOK *page )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(page->client->Liste_synoptique) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(page->client->Liste_synoptique) );
    lignes    = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { gchar chaine[80];
       gint id;
       gtk_tree_model_get_iter( store, &iter, lignes->data );                              /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );                                                /* Recup du id */
       g_snprintf(chaine, sizeof(chaine), "syn/del/%d", id );
       JsonBuilder *builder = Json_create ();
       if (builder)
        { Json_add_int ( builder, "syn_id", id );
          Envoi_json_au_serveur ( page->client, "DELETE", NULL, chaine, Synoptique_deleted_CB );
        }
       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                                               /* Liberation mémoire */
  }
/******************************************************************************************************************************/
/* Menu_Want_Effacer_synoptique: Retrait des synoptiques selectionnés                                                         */
/* Entrée: la page du client                                                                                                  */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_Want_Effacer_synoptique ( struct PAGE_NOTEBOOK *page )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    gboolean retour;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(page->client->Liste_synoptique) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                       /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(page->client->window),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
                                      "Do you want to delete %d synoptique%c ?", nbr, (nbr>1 ? 's' : ' ') );
    gtk_widget_show_all( dialog );
    retour = gtk_dialog_run( GTK_DIALOG(dialog) );                                     /* Attente de reponse de l'utilisateur */
    gtk_widget_destroy(dialog);
    if (retour == GTK_RESPONSE_OK) Send_Effacer_synoptique(page);
  }
/******************************************************************************************************************************/
/* Menu_Want_Add_synoptique: Demande un ajout de synoptique                                                                   */
/* Entrée: la page du client                                                                                                  */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_Want_Add_synoptique ( struct PAGE_NOTEBOOK *page )
  { GtkTreeIter iter;
    gchar *syn_page;
    GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(page->client->Liste_synoptique) );
    GtkTreeModel     *store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(page->client->Liste_synoptique) );
    GList            *lignes    = gtk_tree_selection_get_selected_rows ( selection, NULL );
    if (!lignes) return;

    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_PAGE, &syn_page, -1 );                                           /* Recup du id */
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);

    Creer_Fenetre_Ajouter_Editer_synoptique ( page->client, NULL, syn_page );
    g_free(syn_page);
 }
/******************************************************************************************************************************/
/* Menu_editer_synoptique: Demande d'edition du synoptique selectionné                                                        */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Menu_Want_Atelier_synoptique ( struct PAGE_NOTEBOOK *page )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(page->client->Liste_synoptique) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(page->client->Liste_synoptique) );
    lignes    = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { gint id;
       gtk_tree_model_get_iter( store, &iter, lignes->data );                              /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );                                                /* Recup du id */
       gchar chaine[64];
       g_snprintf( chaine, sizeof(chaine), "/api/syn/show?syn_id=%d", id  );
       Envoi_json_au_serveur ( page->client, "GET", NULL, chaine, Creer_page_atelier_CB );
       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                                         /* Liberation mémoire */
  }
/******************************************************************************************************************************/
/* Gerer_popup_plugin_dls: Gestion du menu popup quand on clique droite sur la liste des plugin_dls                           */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                                                  */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Gerer_popup_synoptique ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { GtkWidget *Popup, *item, *hbox;
    struct PAGE_NOTEBOOK *page = data;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    gint cellx, celly;

    if (!event) return(FALSE);

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(page->client->Liste_synoptique) );     /* On recupere la selection */
    if (gtk_tree_selection_count_selected_rows(selection) == 0)
     { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(page->client->Liste_synoptique), event->x, event->y, &path, NULL, &cellx, &celly );
       if (path)
        { gtk_tree_selection_select_path( selection, path );
          gtk_tree_path_free( path );
        }
     }

    if (event->type == GDK_2BUTTON_PRESS && event->button == 1 )                                            /* Double clic ?? */
     { Menu_Want_Atelier_synoptique ( page );
       return(TRUE);
     }

    if ( event->button != 3 ) return(FALSE);                                                              /* Gestion du popup */

    Popup = gtk_menu_new();

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "document-open", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Editer le synoptique"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    g_signal_connect_swapped ( item, "activate", G_CALLBACK (Menu_Want_Atelier_synoptique), page );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "preferences-system", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Propriétés"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    g_signal_connect_swapped ( item, "activate", G_CALLBACK (Menu_Want_Proprietes_synoptique), page );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "list-add", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Ajouter"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    g_signal_connect_swapped ( item, "activate", G_CALLBACK (Menu_Want_Add_synoptique), page );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    item = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    item = gtk_menu_item_new();
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( "edit-delete", GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new("Supprimer"), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    g_signal_connect_swapped ( item, "activate", G_CALLBACK (Menu_Want_Effacer_synoptique), page );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);

    gtk_widget_show_all(Popup);
    gtk_menu_popup_at_pointer ( GTK_MENU(Popup), (GdkEvent *)event );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Detruire_page_liste_synoptique: L'utilisateur veut fermer la page de plugin dls                                            */
/* Entrée: la page en question                                                                                                */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Detruire_page_liste_synoptique( struct PAGE_NOTEBOOK *page )
  { gtk_widget_destroy ( page->child );
    page->client->Liste_pages = g_slist_remove( page->client->Liste_pages, page );
    g_free(page);
  }
/******************************************************************************************************************************/
/* Creer_page_synoptique: Creation de la page du notebook consacrée aux synoptiques watchdog                                  */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Creer_page_liste_synoptique_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { GtkWidget *scroll, *boite, *hboite, *separateur, *bouton;
    struct CLIENT *client = user_data;
    struct PAGE_NOTEBOOK *page;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GBytes *response_brute;
    gchar *reason_phrase;
    GtkListStore *store;
    JsonNode *response;
    gint status_code;
    gsize taille;

    printf("%s\n", __func__ );
    g_object_get ( msg, "response-body-data", &response_brute, NULL );
    printf("Recu SYNS: %s %p\n", (gchar *)g_bytes_get_data ( response_brute, &taille ), client );

    g_object_get ( msg, "status-code", &status_code, "reason-phrase", &reason_phrase, NULL );
    if (status_code != 200)
     { gchar chaine[256];
       g_snprintf(chaine, sizeof(chaine), "Error loading liste synoptique: Code %d - %s", status_code, reason_phrase );
       Log(client, chaine);
       return;
     }

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;

    page->type   = TYPE_PAGE_SYNOPTIQUE;
    page->client = client;
    client->Liste_pages = g_slist_append( client->Liste_pages, page );

    g_object_get ( msg, "response-body-data", &response_brute, NULL );
    response = Json_get_from_string ( g_bytes_get_data ( response_brute, &taille ) );

    hboite = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );

/***************************************************** La liste des synoptiques ***********************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                                              /* Id */
                                              G_TYPE_UINT,                                                    /* Access_level */
                                              G_TYPE_STRING,                                                         /* PPage */
                                              G_TYPE_STRING,                                                          /* Page */
                                              G_TYPE_STRING                                                           /* Name */
                               );

    client->Liste_synoptique = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );                  /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(client->Liste_synoptique) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), client->Liste_synoptique );

    renderer = gtk_cell_renderer_text_new();                                                 /* Colonne de l'id du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( "SynId", renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                                         /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_synoptique), colonne );

    renderer = gtk_cell_renderer_text_new();                                                 /* Colonne de l'id du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( "Access Level", renderer,
                                                         "text", COLONNE_ACCESS_LEVEL,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ACCESS_LEVEL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_synoptique), colonne );

    renderer = gtk_cell_renderer_text_new();                                                        /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( "Groupe", renderer,
                                                         "text", COLONNE_PPAGE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_PPAGE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_synoptique), colonne );

    renderer = gtk_cell_renderer_text_new();                                                        /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( "Page", renderer,
                                                         "text", COLONNE_PAGE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_PAGE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_synoptique), colonne );

    renderer = gtk_cell_renderer_text_new();                                              /* Colonne du libelle de synoptique */
    colonne = gtk_tree_view_column_new_with_attributes ( "Synoptic Name", renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (client->Liste_synoptique), colonne );

    g_signal_connect( G_OBJECT(client->Liste_synoptique), "button_press_event",                      /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_synoptique), page );
    g_object_unref (G_OBJECT (store));                                            /* nous n'avons plus besoin de notre modele */

/************************************************ Les boutons de controles ****************************************************/
    boite = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_with_label( "Fermer" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "window-close", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Fermer la page" );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Detruire_page_liste_synoptique), page );

    separateur = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_with_label( "Ouvrir" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "document-open", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Editer le synoptique dans l'atelier" );
    g_signal_connect_swapped ( bouton, "clicked", G_CALLBACK (Menu_Want_Atelier_synoptique), page );

    bouton = gtk_button_new_with_label( "Propriétés" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "preferences-system", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Editer les propriétés du synoptique" );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_Want_Proprietes_synoptique), page );

    bouton = gtk_button_new_with_label( "Ajouter" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "list-add", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Ajouter un synoptique fils" );
    g_signal_connect_swapped ( bouton, "clicked", G_CALLBACK (Menu_Want_Add_synoptique), page );

/*    bouton = gtk_button_new_with_label( "Imprimer" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "document-print", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Imprimer la liste des synoptiques" );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_exporter_plugin_dls), NULL );*/

    separateur = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_with_label( "Supprimer" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( "edit-delete", GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, "Supprimer un synoptique et toutes ses dependances" );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_Want_Effacer_synoptique), page );

    json_array_foreach_element ( Json_get_array ( response, "synoptiques" ), Afficher_un_synoptique, client );
    json_node_unref ( response );
    gtk_widget_show_all( hboite );
    gint page_num = gtk_notebook_append_page( GTK_NOTEBOOK(client->Notebook), page->child, gtk_label_new("Synoptiques") );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK(client->Notebook), page_num );
  }
/******************************************************************************************************************************/
/* Menu_want_supervision: l'utilisateur desire voir le synoptique supervision                                                 */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Menu_want_liste_synoptique ( struct CLIENT *client )
  { if (Chercher_page_notebook( client, TYPE_PAGE_ATELIER, 1, TRUE )) return;
    Envoi_json_au_serveur( client, "GET", NULL, "/api/syn/list", Creer_page_liste_synoptique_CB );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
