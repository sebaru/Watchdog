/**********************************************************************************************************/
/* Client/atelier_ajout_palette.c     Gestion des palette pour Watchdog                                   */
/* Projet WatchDog version 1.5     Gestion d'habitat                        dim 22 mai 2005 18:10:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * atelier_ajout_palette.c
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
 #include <string.h>
 #include <stdlib.h>

 #include "Reseaux.h"
 #include "trame.h"
 #include "Config_cli.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

 enum
  {  COLONNE_SYN_ID,
     COLONNE_TITRE_SYN,
     COLONNE_LIBELLE_SYN,
     NBR_COLONNE_SYN
  };
 enum
  {  COLONNE_PALETTE_ID,
     COLONNE_SYN_CIBLE_ID,
     COLONNE_POSITION,
     COLONNE_TITRE_SYN_PALETTE,
     NBR_COLONNE_PALETTE
  };
/**********************************************************************************************************/
/* Visu_groupe: Creation d'un GtkTreeView permettant la visu des données des groupes                      */
/* Entrées: kedal                                                                                         */
/* Sortie: un GtkTreeView                                                                                 */
/**********************************************************************************************************/
 static GtkWidget *Visu_palette ( void )
  { GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkTreeModel *store;
    GtkWidget *vue;

    store = (GtkTreeModel *)gtk_list_store_new ( NBR_COLONNE_PALETTE,
                                                 G_TYPE_UINT,                               /* palette_ID */
                                                 G_TYPE_UINT,                             /* syn_cible_ID */
                                                 G_TYPE_UINT,                                 /* position */
                                                 G_TYPE_STRING                                     /* Nom */
                                               );

    vue = gtk_tree_view_new_with_model ( store );                                   /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(vue) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne de l'id de l'utilisateur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Description"), renderer,
                                                         "text", COLONNE_TITRE_SYN_PALETTE,
                                                         NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(vue), colonne );

    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(vue), TRUE );                          /* Pour faire beau */
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */

    return( vue );
  }
/**********************************************************************************************************/
/* Visu_groupe: Creation d'un GtkTreeView permettant la visu des données des groupes                      */
/* Entrées: kedal                                                                                         */
/* Sortie: un GtkTreeView                                                                                 */
/**********************************************************************************************************/
 static GtkWidget *Visu_synoptique ( void )
  { GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkTreeModel *store;
    GtkWidget *vue;

    store = (GtkTreeModel *)gtk_list_store_new ( NBR_COLONNE_SYN, G_TYPE_UINT,                  /* syn_ID */
                                                              G_TYPE_STRING,                       /* Nom */
                                                              G_TYPE_STRING                /* Commentaire */
                                               );

    vue = gtk_tree_view_new_with_model ( store );                                   /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(vue) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne de l'id de l'utilisateur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Titre"), renderer,
                                                         "text", COLONNE_TITRE_SYN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_TITRE_SYN);              /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW(vue), colonne );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne de l'id de l'utilisateur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Description"), renderer,
                                                         "text", COLONNE_LIBELLE_SYN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE_SYN);            /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW(vue), colonne );

    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(vue), TRUE );                          /* Pour faire beau */
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */

    return( vue );
  }
/**********************************************************************************************************/
/* Effacer_message: Retrait des messages selectionnés                                                     */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Ajouter_palettes ( struct TYPE_INFO_ATELIER *infos )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_PALETTE palette;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_syn) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(infos->Liste_syn) );

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { gtk_tree_model_get_iter( store, &iter, lignes->data );          /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_SYN_ID, &palette.syn_cible_id, -1 );      /* Recup du id */
       gtk_tree_model_get( store, &iter, COLONNE_TITRE_SYN, &palette.libelle, -1 );        /* Recup du id */

       palette.position = 0; /* Sera reintialisé par le serveur lors de l'ajout dans la BD */
       palette.syn_id = infos->syn.id;
       Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_PALETTE,
                      (gchar *)&palette, sizeof(struct CMD_TYPE_PALETTE) );
       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Effacer_message: Retrait des messages selectionnés                                                     */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Retirer_palettes ( struct TYPE_INFO_ATELIER *infos )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_PALETTE palette;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(infos->Liste_palette) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(infos->Liste_palette) );

printf("demande d'effacement palette\n" );
    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { gtk_tree_model_get_iter( store, &iter, lignes->data );          /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_PALETTE_ID, &palette.id, -1 );            /* Recup du id */
       gtk_tree_model_get( store, &iter, COLONNE_TITRE_SYN_PALETTE, &palette.libelle, -1 );/* Recup du id */

       palette.syn_id = infos->syn.id;
printf("Envoi demande d'effacement palette %d\n", palette.id );
       Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_DEL_PALETTE,
                      (gchar *)&palette, sizeof(struct CMD_TYPE_PALETTE) );
       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajout_palette ( GtkDialog *dialog, gint reponse, struct TYPE_INFO_ATELIER *infos )
  { if (!infos) return(FALSE);   

    gtk_widget_destroy( infos->F_ajout_palette );
    infos->F_ajout_palette = NULL;
    return(TRUE);
  }
/**********************************************************************************************************/
/* Commenter: Met en route le processus permettant de passer un synoptique                                */
/* Entrée: widget/data                                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Creer_fenetre_ajout_palette ( struct TYPE_INFO_ATELIER *infos )
  { GtkWidget *hboite, *scroll, *table, *texte, *bouton;
    if (infos->F_ajout_palette) return;

    infos->F_ajout_palette = gtk_dialog_new_with_buttons( _("Add a palette"),
                                                          GTK_WINDOW(F_client),
                                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                          GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                                          NULL);
    gtk_widget_set_size_request (infos->F_ajout_palette, 600, 400);
    g_signal_connect( infos->F_ajout_palette, "response",
                      G_CALLBACK(CB_ajout_palette), infos );
printf("creer fenetre palette 1\n");
    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(infos->F_ajout_palette)->vbox ), hboite, TRUE, TRUE, 0 );
    
/***************************************** La liste des classes *******************************************/
    table = gtk_table_new( 3, 3, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    texte = gtk_label_new( _("Palettes SYN") );
    gtk_table_attach( GTK_TABLE(table), texte, 0, 1, 0, 1, 0, 0, 0, 0 );

    texte = gtk_label_new( _("All palettes") );
    gtk_table_attach( GTK_TABLE(table), texte, 2, 3, 0, 1, 0, 0, 0, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_table_attach( GTK_TABLE(table), bouton, 1, 2, 1, 2, 0, 0, 0, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Ajouter_palettes), infos );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_table_attach( GTK_TABLE(table), bouton, 1, 2, 2, 3, 0, 0, 0, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Retirer_palettes), infos );
printf("creer fenetre palette 2\n");

/***************************************** Gestion des groupes existants **********************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_table_attach_defaults( GTK_TABLE(table), scroll, 2, 3, 1, 3 );

    infos->Liste_syn = Visu_synoptique ();
printf("creer: infos=%p infos->Liste_syn = %p\n", infos, infos->Liste_syn );
    gtk_container_add( GTK_CONTAINER(scroll), infos->Liste_syn );

/***************************************** Gestion des groupes de l'utilisateur ***************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_table_attach_defaults( GTK_TABLE(table), scroll, 0, 1, 1, 3 );
printf("creer fenetre palette 3\n");

    infos->Liste_palette = Visu_palette();                                          /* Creation de la vue */
    gtk_container_add( GTK_CONTAINER(scroll), infos->Liste_palette );

    gtk_widget_show_all( infos->F_ajout_palette );
    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER_PALETTE,
                   (char *)&infos->syn, sizeof(struct CMD_TYPE_SYNOPTIQUE) );       /*demande infos serveur */
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_une_palette_atelier( struct CMD_TYPE_PALETTE *rezo_palette )
  { struct TYPE_INFO_ATELIER *infos;
    GtkListStore *store;
    GtkTreeIter iter;

printf("New Palette debut: \n" );
    infos = Rechercher_infos_atelier_par_id_syn ( rezo_palette->syn_id );
    if (!infos) return;
printf("New Palette atelier: %d %s\n", rezo_palette->id, rezo_palette->libelle );
    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(infos->Liste_palette) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                         COLONNE_PALETTE_ID, rezo_palette->id,
                         COLONNE_SYN_CIBLE_ID, rezo_palette->syn_cible_id,
                         COLONNE_POSITION, rezo_palette->position,
                         COLONNE_TITRE_SYN_PALETTE, rezo_palette->libelle,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Cacher_un_message: Enleve un message de la liste des messages                                          */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_une_palette_atelier( struct CMD_TYPE_PALETTE *palette )
  { struct TYPE_INFO_ATELIER *infos;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    infos = Rechercher_infos_atelier_par_id_syn ( palette->syn_id );
    if (!infos) return;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(infos->Liste_palette) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_PALETTE_ID, &id, -1 );
       if ( id == palette->id ) break;
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
    else { printf("Proto_cacher_une_palette_atelier: non trouvé\n"); }

  }
/**********************************************************************************************************/
/* Afficher_un_icone: Ajoute un icone dans la liste des icones                                            */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_syn_for_palette_atelier( struct CMD_TYPE_SYNOPTIQUE *synoptique )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    GtkListStore *store;
    GtkTreeIter iter;

    page = Page_actuelle();
    if (page->type!=TYPE_PAGE_ATELIER) return;

    infos = page->infos;
    if (!infos) return;
printf("afficher: infos=%p infos->Liste_syn = %p   syn_id=%d\n", infos, infos->Liste_syn, synoptique->id );
    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(infos->Liste_syn) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                         COLONNE_SYN_ID, synoptique->id,
                         COLONNE_LIBELLE_SYN, synoptique->libelle,
                         COLONNE_TITRE_SYN, synoptique->page,
                         -1
                       );
  }
/*--------------------------------------------------------------------------------------------------------*/
