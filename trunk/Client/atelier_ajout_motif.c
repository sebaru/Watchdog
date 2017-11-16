/**********************************************************************************************************/
/* Client/atelier_ajout_motif.c         gestion des ajouts de motifs à la trame                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 08 mai 2004 11:13:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * atelier_ajout_motif.c
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

 enum
  {  COLONNE_CLASSE_ID,
     COLONNE_CLASSE_LIBELLE,
     NBR_COLONNE_CLASSE
  };

 enum
  {  COLONNE_ID,
     COLONNE_LIBELLE,
     COLONNE_CLASSE_ID_ICONE,
     NBR_COLONNE
  };

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GdkBitmap *Rmask, *Bmask, *Vmask, *Omask, *Jmask;                         /* Des pitites boules ! */
 extern GdkPixmap *Rouge, *Bleue, *Verte, *Orange, *Jaune;
 extern GtkWidget *F_trame;                       /* C'est bien le widget referencant la trame synoptique */

 static GtkWidget *F_ajout_motif;                                    /* Pour acceder la fenetre graphique */
 static GtkWidget *Liste_classe;                      /* GtkTreeView pour la gestion des classes Watchdog */
 static GtkWidget *Liste_icone;                        /* GtkTreeView pour la gestion des icones Watchdog */
 static struct TRAME *Trame_preview0;                             /* Previsualisation du motif par défaut */
 static struct TRAME_ITEM_MOTIF *Trame_motif_p0;                           /* Motif en cours de selection */
 static struct CMD_TYPE_MOTIF Motif_preview0;

/**********************************************************************************************************/
/* Menu_editer_icone: Demande d'edition du icone selectionné                                              */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Clic_icone_atelier ( void )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr, icone_id;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_icone) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_icone) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &icone_id, -1 );                         /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */

    memset( &Motif_preview0, 0, sizeof(struct CMD_TYPE_MOTIF) );
    memcpy( &Motif_preview0.libelle, libelle, sizeof( Motif_preview0.libelle ) );
    g_free(libelle);
    Motif_preview0.icone_id = icone_id;
    Motif_preview0.position_x = TAILLE_ICONE_X/2;
    Motif_preview0.position_y = TAILLE_ICONE_Y/2;
    Motif_preview0.rouge0 = 0;
    Motif_preview0.vert0  = 255;
    Motif_preview0.bleu0  = 0;

    if (Trame_motif_p0)
     { goo_canvas_item_remove( Trame_motif_p0->item_groupe );
       Trame_preview0->trame_items = g_list_remove( Trame_preview0->trame_items,
                                                    Trame_motif_p0 );
       g_object_unref( Trame_motif_p0->pixbuf );
       g_free(Trame_motif_p0);
     }    
                                                                                   /* Affichage à l'ecran */
    Trame_motif_p0 = Trame_ajout_motif( TRUE, Trame_preview0, &Motif_preview0 );
    Reduire_en_vignette( &Motif_preview0 );
    Trame_rafraichir_motif ( Trame_motif_p0 );
  }
/**********************************************************************************************************/
/* Clic_classe_atelier: fonction appelée quand l'utilisateur appuie sur une des classes                   */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Clic_classe_atelier ( void )
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
    if (!nbr) return;                                                        /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_CLASSE_ID, &rezo_classe.id, -1 );            /* Recup du id */
    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_WANT_PAGE_ICONE_FOR_ATELIER,
                   (gchar *)&rezo_classe, sizeof(struct CMD_TYPE_CLASSE) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */

    if (Trame_motif_p0)                                         /* On efface la previsu de l'icone */
     { goo_canvas_item_remove( Trame_motif_p0->item_groupe );
       Trame_preview0->trame_items = g_list_remove( Trame_preview0->trame_items,
                                                           Trame_motif_p0 );
       g_object_unref( Trame_motif_p0->pixbuf );
       g_free(Trame_motif_p0);
       Trame_motif_p0 = NULL;
     }    
  }
/**********************************************************************************************************/
/* Choisir_motif_a_ajouter: Affichage de la fenetre de choix du motif a ajouter                           */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Choisir_motif_a_ajouter ( void )
  { if (!F_ajout_motif) Creer_fenetre_ajout_motif ();
    Clic_classe_atelier();
    gtk_widget_show_all( F_ajout_motif );
  }
/**********************************************************************************************************/
/* Detruire_page_propriete_TOR: Destruction de la page de parametres DLS                                  */
/* Entrée: rien                                                                                           */
/* Sortie: toute trace de la fenetre est eliminée                                                         */
/**********************************************************************************************************/
 void Detruire_fenetre_ajout_motif ( void )
  { printf("Detruire_fenetre_ajout_motif: debut\n");
    if (Trame_preview0) { Trame_detruire_trame( Trame_preview0 );
                          Trame_preview0 = NULL;
                        }
    printf("Detruire_fenetre_ajout_motif: milieu\n");
    if (F_ajout_motif) gtk_widget_destroy( F_ajout_motif );
    printf("Detruire_fenetre_ajout_motif: fin\n");
    Trame_motif_p0 = NULL;
    F_ajout_motif = NULL;
  }
/**********************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajout_motif ( GtkDialog *dialog, gint reponse )
  { struct CMD_TYPE_MOTIF add_motif;
    struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return(TRUE);         /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    switch(reponse)
     { case GTK_RESPONSE_OK:     if (!Trame_motif_p0) return(TRUE);
                                 add_motif.icone_id = Trame_motif_p0->motif->icone_id;/* Correspond au .gif*/
                                 g_snprintf( add_motif.libelle, sizeof(add_motif.libelle),
                                             "%s" , Motif_preview0.libelle );
                                 add_motif.syn_id = infos->syn.id;
                                 add_motif.access_level = 0;     /* Nom du groupe d'appartenance du motif */
                                 add_motif.bit_controle = 0;                                /* Ixxx, Cxxx */
                                 add_motif.bit_clic = 0;        /* Bit à activer quand clic gauche souris */
                                 add_motif.bit_clic2 = 0;       /* Bit à activer quand clic gauche souris */
                                 add_motif.angle = 0.0; /*infos->Adj_angle->value;*/
                                 add_motif.type_dialog = 0;               /* Type de la boite de dialogue */
                                 add_motif.type_gestion = 0;
                                 add_motif.position_x = TAILLE_SYNOPTIQUE_X/2;
                                 add_motif.position_y = TAILLE_SYNOPTIQUE_Y/2;
                                 add_motif.largeur = Trame_motif_p0->gif_largeur;
                                 add_motif.hauteur = Trame_motif_p0->gif_hauteur;
                                 add_motif.rouge0 = 255;
                                 add_motif.vert0 = 255;
                                 add_motif.bleu0 = 255;
                                 Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_MOTIF,
                                                (gchar *)&add_motif, sizeof(struct CMD_TYPE_MOTIF) );
                                 printf("Requete envoyée au serveur....\n");
                                 return(TRUE);                            /* On laisse la fenetre ouverte */
                                 break;
       case GTK_RESPONSE_CLOSE: break;
     }
    gtk_widget_hide( F_ajout_motif );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Creer_page_propriete_TOR: Creation de la fenetre d'edition des proprietes TOR                          */
/* Entrée: niet                                                                                           */
/* Sortie: niet                                                                                           */
/**********************************************************************************************************/
 void Creer_fenetre_ajout_motif ( void )
  { GtkWidget *table, *scroll, *hboite, *vboite;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;

    F_ajout_motif = gtk_dialog_new_with_buttons( _("Add a icon"),
                                                        GTK_WINDOW(F_client),
                                                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                        GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                                        GTK_STOCK_ADD, GTK_RESPONSE_OK,
                                                        NULL);
    gtk_widget_set_size_request (F_ajout_motif, 800, 600);

    g_signal_connect( F_ajout_motif, "response",
                      G_CALLBACK(CB_ajout_motif), NULL );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout_motif)->vbox ), hboite, TRUE, TRUE, 0 );
    
/***************************************** La liste des classes *******************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE_CLASSE, G_TYPE_UINT,                      /* Id de la classe */
                                                     G_TYPE_STRING                              /* Classe */
                               );

    Liste_classe = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );   /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_classe) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_classe );

#ifdef bouh
    renderer = gtk_cell_renderer_text_new();                                    /* Colonne du commentaire */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("ClassId"), renderer,
                                                         "text", COLONNE_CLASSE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_CLASSE_ID);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_classe), colonne );
#endif
    renderer = gtk_cell_renderer_text_new();                                  /* Colonne de l'id du icone */
    colonne = gtk_tree_view_column_new_with_attributes ( _("ClassName"), renderer,
                                                         "text", COLONNE_CLASSE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_CLASSE_LIBELLE);         /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_classe), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_icone), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_classe), TRUE );          /* Pour faire beau */

    g_signal_connect_swapped( G_OBJECT(selection), "changed",                    /* Gestion du menu popup */
                              G_CALLBACK(Clic_classe_atelier), NULL );
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */

/***************************************** La liste des icones ********************************************/
    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), vboite, TRUE, TRUE, 0 );
    
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(vboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* Id */
                                              G_TYPE_STRING,                                   /* libellé */
                                              G_TYPE_UINT                              /* Classe id icone */
                               );

    Liste_icone = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );    /* Creation de la vue */
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
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_icone), TRUE );           /* Pour faire beau */

    g_signal_connect_swapped( G_OBJECT(selection), "changed",                    /* Gestion du menu popup */
                              G_CALLBACK(Clic_icone_atelier), NULL );
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
/*********************************************** Preview et controle **************************************/
    table = gtk_table_new( 3, 3, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, FALSE, FALSE, 0 );

    Trame_preview0 = Trame_creer_trame( TAILLE_ICONE_X, TAILLE_ICONE_Y, "darkgray", 0 );
    gtk_widget_set_usize( Trame_preview0->trame_widget, TAILLE_ICONE_X, TAILLE_ICONE_Y );

    gtk_table_attach_defaults( GTK_TABLE(table), Trame_preview0->trame_widget, 0, 1, 0, 3 );
    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_WANT_PAGE_CLASSE_FOR_ATELIER,
                   NULL, 0 );                                            /* On veut les données classes ! */
  }
/**********************************************************************************************************/
/* Rafraichir_visu_classe: Rafraichissement d'une classe la liste à l'écran                               */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_classe_atelier( GtkTreeIter *iter, struct CMD_TYPE_CLASSE *classe )
  { GtkTreeModel *store;
    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_classe) );       /* Acquisition du modele */
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
 void Proto_afficher_une_classe_atelier( struct CMD_TYPE_CLASSE *classe )
  { GtkTreeModel *store;
    GtkTreeIter iter;

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_classe) );

    gtk_list_store_append ( GTK_LIST_STORE(store), &iter );                      /* Acquisition iterateur */
    Rafraichir_visu_classe_atelier ( &iter, classe );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_icone: Rafraichissement d'un icone la liste à l'écran                                  */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_icone_atelier( GtkTreeIter *iter, struct CMD_TYPE_ICONE *icone )
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
 void Proto_afficher_un_icone_atelier( struct CMD_TYPE_ICONE *icone )
  { GtkTreeModel *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_ATELIER)) return;

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_icone) );
    gtk_tree_model_get_iter_first( store, &iter );

    gtk_list_store_append ( GTK_LIST_STORE(store), &iter );                      /* Acquisition iterateur */
    Rafraichir_visu_icone_atelier ( &iter, icone );
  }
/*--------------------------------------------------------------------------------------------------------*/
