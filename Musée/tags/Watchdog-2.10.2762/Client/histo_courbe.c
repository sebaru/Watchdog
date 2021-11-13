/**********************************************************************************************************/
/* Client/histo_courbe.c        Affichage des historiques des courbes synoptiques de supervision          */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 nov 2007 10:26:12 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * histo_courbe.c
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
 #include <sys/time.h>
 #include <gtkdatabox.h>
 #include <gtkdatabox_points.h>
 #include <gtkdatabox_lines.h>
 #include <gtkdatabox_grid.h>
 #include <gtkdatabox_markers.h>
 
 #include "Reseaux.h"
 #include "Config_cli.h"

 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static GtkWidget *F_source = NULL;
 static GtkWidget *Liste_source = NULL;
 enum
  {  COLONNE_ID,
     COLONNE_TYPE,
     COLONNE_TYPE_EA,
     COLONNE_OBJET,
     COLONNE_NUM,
     COLONNE_MIN,
     COLONNE_MAX,
     COLONNE_UNITE_STRING,
     COLONNE_LIBELLE,
     NBR_COLONNE
  };
 static GdkColor COULEUR_COURBE[NBR_MAX_COURBES]=
  { { 0x0, 0xFFFF, 0x0,    0x0    },
    { 0x0, 0x0,    0xFFFF, 0x0    },
    { 0x0, 0x0,    0x0,    0xFFFF },
    { 0x0, 0xFFFF, 0xFFFF, 0x0    },
    { 0x0, 0x0,    0xFFFF, 0xFFFF },
    { 0x0, 0xFFFF, 0x0,    0xFFFF },
  };

/**********************************************************************************************************/
/* CB_ajouter_editer_source: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_retirer_histo_courbe ( GtkDialog *dialog, gint reponse,
                                                   struct TYPE_INFO_COURBE *infos )
  { GtkTreeSelection *selection;
    struct COURBE *new_courbe;
    struct CMD_TYPE_COURBE rezo_courbe;
    gchar *libelle, *unite;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;


    if (reponse == GTK_RESPONSE_ACCEPT)
     {
       if( infos->Courbes[infos->slot_id].actif )                          /* Enleve la précédente courbe */
        { infos->Courbes[infos->slot_id].actif = FALSE;
          gtk_entry_set_text( GTK_ENTRY(infos->Entry[infos->slot_id]), "" );
          gtk_databox_graph_remove ( GTK_DATABOX(infos->Databox), infos->Courbes[infos->slot_id].index );
          gtk_databox_graph_remove ( GTK_DATABOX(infos->Databox), infos->Courbes[infos->slot_id].marker_select );
          gtk_widget_queue_draw (infos->Databox);
        }

       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_source) );
       store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_source) );

       nbr = gtk_tree_selection_count_selected_rows( selection );
       if (!nbr) return(FALSE);                                              /* Si rien n'est selectionné */

       lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );

       gtk_tree_model_get_iter( store, &iter, lignes->data );          /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_courbe.num, -1 );                /* Recup du id */
       gtk_tree_model_get( store, &iter, COLONNE_TYPE, &rezo_courbe.type, -1 );          /* Recup du type */
       gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );
       memcpy( &rezo_courbe.libelle, libelle, sizeof(rezo_courbe.libelle) );
       g_free( libelle );
       rezo_courbe.slot_id = infos->slot_id;

       new_courbe = &infos->Courbes[infos->slot_id];
       new_courbe->actif = TRUE;                                    /* Récupération des données EANA dans la structure COURBE */
       new_courbe->mnemo.mnemo_base.type = rezo_courbe.type;        /* Récupération des données EANA dans la structure COURBE */
       new_courbe->mnemo.mnemo_base.num  = rezo_courbe.num;
       gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );
       g_snprintf( new_courbe->mnemo.mnemo_base.libelle, sizeof(new_courbe->mnemo.mnemo_base.libelle), "%s", libelle );
       g_free(libelle);
       switch( new_courbe->mnemo.mnemo_base.type )
        { case MNEMO_ENTREE_ANA:
               new_courbe->mnemo.mnemo_ai.num = rezo_courbe.num;
               gtk_tree_model_get( store, &iter, COLONNE_TYPE_EA, &new_courbe->mnemo.mnemo_ai.type, -1 );
               gtk_tree_model_get( store, &iter, COLONNE_MIN, &new_courbe->mnemo.mnemo_ai.min, -1 );
               gtk_tree_model_get( store, &iter, COLONNE_MAX, &new_courbe->mnemo.mnemo_ai.max, -1 );
               gtk_tree_model_get( store, &iter, COLONNE_UNITE_STRING, &unite, -1 );
               g_snprintf( new_courbe->mnemo.mnemo_ai.unite,     sizeof(new_courbe->mnemo.mnemo_ai.unite),     "%s", unite );
               g_free(unite);
               break;
          case MNEMO_SORTIE:
          case MNEMO_ENTREE:
               break;
        }
                                                          /* Placement de la nouvelle courbe sur l'id gui */
       gtk_tree_selection_unselect_iter( selection, &iter );
       g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
       g_list_free (lignes);                                                        /* Liberation mémoire */

       Envoi_serveur( TAG_HISTO_COURBE, SSTAG_CLIENT_ADD_HISTO_COURBE,
                      (gchar *)&rezo_courbe, sizeof(struct CMD_TYPE_COURBE) );
     }
    else if (reponse == GTK_RESPONSE_REJECT)                                                /* On retire la courbe de la visu */
     { new_courbe = &infos->Courbes[infos->slot_id];
       new_courbe->actif = FALSE;                                   /* Récupération des données EANA dans la structure COURBE */
       new_courbe->mnemo.mnemo_base.type  = 0;                      /* Récupération des données EANA dans la structure COURBE */
       gtk_databox_graph_remove ( GTK_DATABOX(infos->Databox), new_courbe->index );
       gtk_databox_graph_remove ( GTK_DATABOX(infos->Databox), new_courbe->marker_select );
       gtk_widget_queue_draw (infos->Databox);
       gtk_entry_set_text( GTK_ENTRY(infos->Entry[infos->slot_id]), " -- no info --" );
     }
    gtk_widget_destroy(F_source);
    F_source = NULL;
    return(TRUE);
  }
/**********************************************************************************************************/
/* Menu_ajouter_courbe: Demande au serveur de superviser une nouvelle courbe                              */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_changer_courbe ( struct TYPE_INFO_COURBE *infos, guint num )
  { GtkWidget *frame, *hboite, *scroll;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;

    if (F_source) return;
    F_source = gtk_dialog_new_with_buttons( _("View curves"),
                                               GTK_WINDOW(F_client),
                                               GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_REMOVE, GTK_RESPONSE_REJECT,
                                               GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT,
                                               NULL );
    g_signal_connect( F_source, "response",
                      G_CALLBACK(CB_ajouter_retirer_histo_courbe), infos );

    gtk_widget_set_size_request (F_source, 800, 600);

    frame = gtk_frame_new("Curves/EntreeANA");                       /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_source)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                    /* Id (num) */
                                              G_TYPE_UINT,                                        /* Type */
                                              G_TYPE_UINT,                                     /* Type EA */
                                              G_TYPE_STRING,                                     /* Objet */
                                              G_TYPE_STRING,                 /* Num (id en string "EAxxx" */
                                              G_TYPE_FLOAT,                                       /* min */
                                              G_TYPE_FLOAT,                                       /* max */
                                              G_TYPE_STRING,                              /* Unite_string */
                                              G_TYPE_STRING                                    /* libelle */
                               );

    Liste_source = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );/* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_source) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_source );

    renderer = gtk_cell_renderer_text_new();                                 /* Colonne de l'id du source */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Numéro"), renderer,
                                                         "text", COLONNE_NUM,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NUM);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_source), colonne );

    renderer = gtk_cell_renderer_text_new();                                 /* Colonne de l'id du source */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Objet"), renderer,
                                                         "text", COLONNE_OBJET,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_OBJET);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_source), colonne );

    renderer = gtk_cell_renderer_text_new();                              /* Colonne du libelle de source */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Min"), renderer,
                                                         "text", COLONNE_MIN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_MIN);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_source), colonne );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Max"), renderer,
                                                         "text", COLONNE_MAX,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_MAX);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_source), colonne );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Unit"), renderer,
                                                         "text", COLONNE_UNITE_STRING,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_UNITE_STRING);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_source), colonne );

    renderer = gtk_cell_renderer_text_new();                           /* Colonne du libelle de source */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Description"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_source), colonne );

    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_source), TRUE );              /* Pour faire beau */

    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
    gtk_widget_show_all(F_source);                                /* Affichage de l'interface complète */

    infos->slot_id = num;                /* Place de la prochaine courbe dans l'interface graphique */
    Envoi_serveur( TAG_HISTO_COURBE, SSTAG_CLIENT_WANT_PAGE_SOURCE_FOR_HISTO_COURBE, NULL, 0 );
printf("Envoie want page source for histo courbe\n");
  }
/**********************************************************************************************************/
/* Menu_changer_courbe1: Demande au serveur de changer la vue de la courbe 1                              */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_changer_courbe1 ( struct TYPE_INFO_COURBE *infos )
  { Menu_changer_courbe( infos, 0 ); }
 static void Menu_changer_courbe2 ( struct TYPE_INFO_COURBE *infos )
  { Menu_changer_courbe( infos, 1 ); }
 static void Menu_changer_courbe3 ( struct TYPE_INFO_COURBE *infos )
  { Menu_changer_courbe( infos, 2 ); }
 static void Menu_changer_courbe4 ( struct TYPE_INFO_COURBE *infos )
  { Menu_changer_courbe( infos, 3 ); }
 static void Menu_changer_courbe5 ( struct TYPE_INFO_COURBE *infos )
  { Menu_changer_courbe( infos, 4 ); }
 static void Menu_changer_courbe6 ( struct TYPE_INFO_COURBE *infos )
  { Menu_changer_courbe( infos, 5 ); }
/**********************************************************************************************************/
/* Menu_rescale: Demande de recadrage des courbes                                                         */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_rescale ( struct TYPE_INFO_COURBE *infos )
  { struct CMD_TYPE_COURBE rezo_courbe;
    guint cpt;
    for ( cpt=0; cpt<NBR_MAX_COURBES; cpt++ )
     { if (!infos->Courbes[cpt].actif) continue;

       rezo_courbe.slot_id = cpt;
       rezo_courbe.type = infos->Courbes[cpt].mnemo.mnemo_base.type;
       rezo_courbe.num  = infos->Courbes[cpt].mnemo.mnemo_base.num;

       printf("Envoi serveur TAG_CLIENT_ADD_HISTO_COURBE %d\n", rezo_courbe.slot_id );
       Envoi_serveur( TAG_HISTO_COURBE, SSTAG_CLIENT_ADD_HISTO_COURBE,
                      (gchar *)&rezo_courbe, sizeof(struct CMD_TYPE_COURBE) );
     }
  }
/**********************************************************************************************************/
/* Rafraichir_sensibilite: Grise ou non les champs de recherche de l'historique hard                      */
/* Entrée: un pointeur sur la page en cours                                                               */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Rafraichir_date_histo ( struct PAGE_NOTEBOOK *page )
  { struct CMD_HISTO_COURBE histo_courbe;
    struct TYPE_INFO_COURBE *infos;

    infos = (struct TYPE_INFO_COURBE *)page->infos;

    histo_courbe.date_first = gnome_date_edit_get_time( GNOME_DATE_EDIT(infos->Date_debut) );
    histo_courbe.date_last = gnome_date_edit_get_time( GNOME_DATE_EDIT(infos->Date_fin) );
printf(" Date first/last = %d / %d\n", histo_courbe.date_first, histo_courbe.date_last );

    Envoi_serveur( TAG_HISTO_COURBE, SSTAG_CLIENT_SET_DATE,
                   (gchar *)&histo_courbe, sizeof( struct CMD_HISTO_COURBE ) );

  }
/**********************************************************************************************************/
/* Detruire_page_supervision: L'utilisateur veut fermer la page de supervision                            */
/* Entrée: la page en question                                                                            */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Detruire_page_histo_courbe( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_COURBE *infos;
    infos = (struct TYPE_INFO_COURBE *)page->infos;
    gtk_databox_graph_remove_all ( GTK_DATABOX(infos->Databox) );
    gtk_widget_destroy(infos->Databox);
  }
/**********************************************************************************************************/
/* Creer_page_courbe: Creation d'une page du notebook pour les courbes watchdog                           */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_histo_courbe ( gchar *libelle )
  { GdkColor grille = { 0x0, 0x7FFF, 0x7FFF, 0x7FFF };
    GtkWidget *bouton, *boite, *hboite, *vboite, *table, *table2, *separateur;
    struct TYPE_INFO_COURBE *infos;
    struct PAGE_NOTEBOOK *page;
    GdkColor fond   = { 0x0, 0x0, 0x0, 0x0 };

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->infos = (struct TYPE_INFO_COURBE *)g_try_malloc0( sizeof(struct TYPE_INFO_COURBE) );
    infos = (struct TYPE_INFO_COURBE *)page->infos;
    if (!page->infos) { g_free(page); return; }

    page->type   = TYPE_PAGE_HISTO_COURBE;
    Liste_pages  = g_list_append( Liste_pages, page );
    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
 
    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), vboite, TRUE, TRUE, 0 );

    table = gtk_table_new ( 4, 6, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_EDIT );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_changer_courbe1), infos );
    gtk_table_attach( GTK_TABLE(table), bouton, 1, 2, 0, 1, 0, 0, 0, 0 );
    infos->Entry[0] = gtk_entry_new();
    gtk_widget_modify_base (infos->Entry[0], GTK_STATE_NORMAL, &COULEUR_COURBE[0]);
    gtk_editable_set_editable( GTK_EDITABLE(infos->Entry[0]), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry[0], 2, 3, 0, 1 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_EDIT );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_changer_courbe2), infos );
    gtk_table_attach( GTK_TABLE(table), bouton, 1, 2, 1, 2, 0, 0, 0, 0 );
    infos->Entry[1] = gtk_entry_new();
    gtk_widget_modify_base (infos->Entry[1], GTK_STATE_NORMAL, &COULEUR_COURBE[1]);
    gtk_editable_set_editable( GTK_EDITABLE(infos->Entry[1]), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry[1], 2, 3, 1, 2 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_EDIT );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_changer_courbe3), infos );
    gtk_table_attach( GTK_TABLE(table), bouton, 1, 2, 2, 3, 0, 0, 0, 0 );
    infos->Entry[2] = gtk_entry_new();
    gtk_widget_modify_base (infos->Entry[2], GTK_STATE_NORMAL, &COULEUR_COURBE[2]);
    gtk_editable_set_editable( GTK_EDITABLE(infos->Entry[2]), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry[2], 2, 3, 2, 3 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_EDIT );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_changer_courbe4), infos );
    gtk_table_attach( GTK_TABLE(table), bouton, 4, 5, 0, 1, 0, 0, 0, 0 );
    infos->Entry[3] = gtk_entry_new();
    gtk_widget_modify_base (infos->Entry[3], GTK_STATE_NORMAL, &COULEUR_COURBE[3]);
    gtk_editable_set_editable( GTK_EDITABLE(infos->Entry[3]), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry[3], 5, 6, 0, 1 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_EDIT );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_changer_courbe5), infos );
    gtk_table_attach( GTK_TABLE(table), bouton, 4, 5, 1, 2, 0, 0, 0, 0 );
    infos->Entry[4] = gtk_entry_new();
    gtk_widget_modify_base (infos->Entry[4], GTK_STATE_NORMAL, &COULEUR_COURBE[4]);
    gtk_editable_set_editable( GTK_EDITABLE(infos->Entry[4]), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry[4], 5, 6, 1, 2 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_EDIT );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_changer_courbe6), infos );
    gtk_table_attach( GTK_TABLE(table), bouton, 4, 5, 2, 3, 0, 0, 0, 0 );
    infos->Entry[5] = gtk_entry_new();
    gtk_widget_modify_base (infos->Entry[5], GTK_STATE_NORMAL, &COULEUR_COURBE[5]);
    gtk_editable_set_editable( GTK_EDITABLE(infos->Entry[5]), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry[5], 5, 6, 2, 3 );

/****************************************** L'entry pour la date select ***********************************/
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, FALSE, FALSE, 0 );

    gtk_table_attach_defaults( GTK_TABLE(table), gtk_label_new( _("Date de debut") ), 0, 1, 0, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), gtk_label_new( _("Date de fin") ), 1, 2, 0, 1 );

    infos->Date_debut = gnome_date_edit_new ((time_t) 0, TRUE, TRUE);
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Date_debut, 0, 1, 1, 2 );
    g_signal_connect_swapped( G_OBJECT(infos->Date_debut), "date-changed",
                              G_CALLBACK(Rafraichir_date_histo), page );
    g_signal_connect_swapped( G_OBJECT(infos->Date_debut), "time-changed",
                              G_CALLBACK(Rafraichir_date_histo), page );

    infos->Date_fin = gnome_date_edit_new ((time_t) 0, TRUE, TRUE);
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Date_fin, 1, 2, 1, 2 );
    g_signal_connect_swapped( G_OBJECT(infos->Date_fin), "date-changed",
                              G_CALLBACK(Rafraichir_date_histo), page );
    g_signal_connect_swapped( G_OBJECT(infos->Date_fin), "time-changed",
                              G_CALLBACK(Rafraichir_date_histo), page );

/****************************************** L'entry pour la date select ***********************************/
    infos->Entry_date_select = gtk_entry_new();
    gtk_editable_set_editable( GTK_EDITABLE(infos->Entry_date_select), FALSE );
    gtk_box_pack_start( GTK_BOX(vboite), infos->Entry_date_select, FALSE, FALSE, 0 );

/****************************************** La databox ****************************************************/
    gtk_databox_create_box_with_scrollbars_and_rulers ( &infos->Databox, &table2, TRUE, TRUE, FALSE, FALSE );
    gtk_box_pack_start( GTK_BOX(vboite), table2, TRUE, TRUE, 0 );

    gtk_databox_set_scale_type_x ( GTK_DATABOX (infos->Databox), GTK_DATABOX_SCALE_LINEAR );
    gtk_databox_set_scale_type_y ( GTK_DATABOX (infos->Databox), GTK_DATABOX_SCALE_LINEAR );
    gtk_widget_modify_bg (infos->Databox, GTK_STATE_NORMAL, &fond);

    g_signal_connect_swapped( G_OBJECT(infos->Databox), "motion_notify_event",
                              G_CALLBACK(CB_deplacement_databox), infos );
    gtk_widget_add_events( GTK_WIDGET(infos->Databox), GDK_LEAVE_NOTIFY_MASK );
    g_signal_connect_swapped( G_OBJECT(infos->Databox), "leave_notify_event",
                              G_CALLBACK(CB_sortir_databox), infos );

    infos->index_grille = gtk_databox_grid_new ( 3, 10, &grille, 1 );
    gtk_databox_graph_add (GTK_DATABOX (infos->Databox), infos->index_grille);

/**************************************** Boutons de controle *********************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_EDIT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
/*    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_retirer_courbe), infos );*/

    bouton = gtk_button_new_with_label( _("Rescale") );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_rescale), infos );

    gtk_widget_show_all( page->child );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), page->child, gtk_label_new ( libelle ) );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_source: Rafraichissement d'un source la liste à l'écran                                */
/* Entrée: une reference sur le source                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_source_EA_histo( GtkTreeIter *iter, struct CMD_TYPE_MNEMO_FULL *source )
  { struct PAGE_NOTEBOOK *page;
    GtkTreeModel *store;
    gchar chaine[20];

    page = Page_actuelle();
    if (page->type != TYPE_PAGE_HISTO_COURBE) return;                                      /* Bon type ?? */

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_source) );              /* Acquisition du modele */

    g_snprintf( chaine, sizeof(chaine), "%s%04d", Type_bit_interne_court(MNEMO_ENTREE_ANA), source->mnemo_base.num );
    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, source->mnemo_base.num,
                         COLONNE_TYPE, MNEMO_ENTREE_ANA,
                         COLONNE_TYPE_EA, source->mnemo_ai.type,
                         COLONNE_OBJET, "a venir",
                         COLONNE_NUM, chaine,
                         COLONNE_MIN, source->mnemo_ai.min,
                         COLONNE_MAX, source->mnemo_ai.max,
                         COLONNE_UNITE_STRING, source->mnemo_ai.unite,
                         COLONNE_LIBELLE, source->mnemo_base.libelle,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_source: Rafraichissement d'un source la liste à l'écran                                */
/* Entrée: une reference sur le source                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_source_histo( GtkTreeIter *iter, struct CMD_TYPE_MNEMO_BASE *source )
  { struct PAGE_NOTEBOOK *page;
    GtkTreeModel *store;
    gchar chaine[20], groupe_page[512];

    page = Page_actuelle();
    if (page->type != TYPE_PAGE_HISTO_COURBE) return;                                      /* Bon type ?? */

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_source) );              /* Acquisition du modele */

    g_snprintf( chaine, sizeof(chaine), "%s%04d", Type_bit_interne_court(source->type), source->num );
    g_snprintf( groupe_page, sizeof(groupe_page), "%s/%s/%s",
                source->groupe, source->page, source->plugin_dls );

    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, source->num,
                         COLONNE_TYPE, source->type,
                         COLONNE_TYPE_EA, 0,
                         COLONNE_OBJET, groupe_page,
                         COLONNE_NUM, chaine,
                         COLONNE_MIN, 0.0,
                         COLONNE_MAX, 1.0,
                         COLONNE_UNITE_STRING, "On/Off",
                         COLONNE_LIBELLE, source->libelle,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Afficher_un_source: Ajoute un source dans la liste des sources                                         */
/* Entrée: une reference sur le source                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_une_source_EA_for_histo_courbe( struct CMD_TYPE_MNEMO_FULL *source )
  { GtkListStore *store;
    GtkTreeIter iter;

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_source) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_source_EA_histo ( &iter, source );
  }
/**********************************************************************************************************/
/* Afficher_un_source: Ajoute un source dans la liste des sources                                         */
/* Entrée: une reference sur le source                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_une_source_for_histo_courbe( struct CMD_TYPE_MNEMO_BASE *source )
  { GtkListStore *store;
    GtkTreeIter iter;

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_source) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_source_histo ( &iter, source );
  }
/**********************************************************************************************************/
/* Proto_ajouter_courbe: Appeler lorsque le client recoit la reponse d'ajout de courbe par le serveur     */
/* Entrée: une reference sur le source                                                                 */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_ajouter_histo_courbe( struct CMD_TYPE_COURBE *courbe )
  { struct PAGE_NOTEBOOK *page;
    struct TYPE_INFO_COURBE *infos;

    page = Chercher_page_notebook( TYPE_PAGE_HISTO_COURBE, 0, TRUE );         /* Récupération page courbe */
    if (!page) return;
    infos = page->infos;
    if (!infos) return;

    Ajouter_courbe ( courbe, infos, FALSE );
  }
/**********************************************************************************************************/
/* Proto_start_courbe: Appeler lorsque le client recoit un premier bloc de valeur a afficher              */
/* Entrée: une reference sur la courbe                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_start_histo_courbe( struct CMD_START_COURBE *start_courbe )
  { struct PAGE_NOTEBOOK *page;
    struct TYPE_INFO_COURBE *infos;

    page = Chercher_page_notebook( TYPE_PAGE_HISTO_COURBE, 0, TRUE );         /* Récupération page courbe */
    if (!page) return;
    infos = page->infos;
    if (!infos) return;

    Afficher_courbe ( start_courbe, infos );
    gtk_databox_auto_rescale( GTK_DATABOX(infos->Databox), 0.1 );
  }
/*--------------------------------------------------------------------------------------------------------*/
