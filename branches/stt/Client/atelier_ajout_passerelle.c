/**********************************************************************************************************/
/* Client/atelier_ajout_passerelle.c     Gestion des passerelles pour Watchdog                            */
/* Projet WatchDog version 1.5     Gestion d'habitat                         jeu 10 fév 2005 16:25:06 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * atelier_ajout_passerelle.c
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

/* a passer en local */
 static GtkWidget *F_ajout_pass = NULL;                         /* Fenetre graphique de choix de passaire */
 static GtkWidget *Liste_syn = NULL;                                 /* Liste des synoptiques disponibles */

 enum
  {  COLONNE_ID_SYN,
     COLONNE_MNEMO_SYN,
     COLONNE_LIBELLE_SYN,
     NBR_COLONNE_SYN
  };

/**********************************************************************************************************/
/* Id_vers_trame_pass: Conversion d'un id pass en sa reference TRAME                                      */
/* Entrée: Un id motif                                                                                    */
/* sortie: un struct TRAME_ITEM_MOTIF                                                                     */
/**********************************************************************************************************/
 struct TRAME_ITEM_PASS *Id_vers_trame_pass ( struct TYPE_INFO_ATELIER *infos, gint id )
  { GList *liste;
    liste = infos->Trame_atelier->trame_items;
    while( liste )
     { if ( *(gint *)(liste->data) == TYPE_PASSERELLE &&
            ((struct TRAME_ITEM_PASS *)(liste->data))->pass->id == id ) break;
       else liste = liste->next;
     }
    if (!liste)
     { printf("Id_vers_trame_pass: item %d non trouvé\n", id );
       return(NULL);
     }
    return( (struct TRAME_ITEM_PASS *)(liste->data) );
  }
/**********************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajout_pass ( GtkDialog *dialog, gint reponse )
  { struct CMD_TYPE_PASSERELLE add_pass;
    struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gint id;
   
    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return(TRUE);         /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_syn) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_syn) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            printf("lignes = %p\n", lignes );
            if (lignes)
             { gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID_SYN, &id, -1 );                /* Recup du id */
               g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
               g_list_free (lignes);                                                /* Liberation mémoire */

               add_pass.position_x = TAILLE_SYNOPTIQUE_X/2;
               add_pass.position_y = TAILLE_SYNOPTIQUE_Y/2;
               add_pass.syn_id = infos->syn.id;
               add_pass.syn_cible_id = id;                           /* Synoptique cible de la passerelle */
               add_pass.angle          = 0.0;

               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_PASS,
                              (gchar *)&add_pass, sizeof(struct CMD_TYPE_PASSERELLE) );
               printf("Requete d'ajout de passerelle envoyée au serveur....\n");
               return(TRUE);                                              /* On laisse la fenetre ouverte */
             }
       case GTK_RESPONSE_CLOSE:
            break;
     }
    gtk_widget_destroy( F_ajout_pass );
    F_ajout_pass = NULL;
    return(TRUE);
  }
/**********************************************************************************************************/
/* Commenter: Met en route le processus permettant de passer un synoptique                                */
/* Entrée: widget/data                                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Creer_fenetre_ajout_passerelle ( void )
  { GtkWidget *hboite, *scroll;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    if (F_ajout_pass) return;

    F_ajout_pass = gtk_dialog_new_with_buttons( _("Add a gateway"),
                                             GTK_WINDOW(F_client),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                             GTK_STOCK_ADD, GTK_RESPONSE_OK,
                                             NULL);
    g_signal_connect( F_ajout_pass, "response",
                      G_CALLBACK(CB_ajout_pass), NULL );
    gtk_widget_set_usize( F_ajout_pass, 600, 300 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout_pass)->vbox ), hboite, TRUE, TRUE, 0 );
    
/***************************************** La liste des classes *******************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE_SYN, G_TYPE_UINT,                               /* Id du syn */
                                                  G_TYPE_STRING,                                 /* Mnémo */
                                                  G_TYPE_STRING                                /* Libelle */
                               );

    Liste_syn = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );             /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_syn) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_syn );

    renderer = gtk_cell_renderer_text_new();                                  /* Colonne de l'id du icone */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Titre"), renderer,
                                                         "text", COLONNE_MNEMO_SYN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_MNEMO_SYN);              /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_syn), colonne );

    renderer = gtk_cell_renderer_text_new();                                  /* Colonne de l'id du icone */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Libelle"), renderer,
                                                         "text", COLONNE_LIBELLE_SYN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE_SYN);            /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_syn), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_icone), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_syn), TRUE );                    /* Pour faire beau */

    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
/***************************************** La liste des classes *******************************************/
    gtk_widget_show_all( F_ajout_pass );

    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER,
                   NULL, 0 );                                                    /* demande infos serveur */
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_une_passerelle_atelier( struct CMD_TYPE_PASSERELLE *rezo_pass )
  { struct TRAME_ITEM_PASS *trame_pass;
    struct TYPE_INFO_ATELIER *infos;
    struct CMD_TYPE_PASSERELLE *pass;
        
    infos = Rechercher_infos_atelier_par_id_syn ( rezo_pass->syn_id );
    pass = (struct CMD_TYPE_PASSERELLE *)g_try_malloc0( sizeof(struct CMD_TYPE_PASSERELLE) );
    if (!pass)
     { return;
     }

    memcpy( pass, rezo_pass, sizeof(struct CMD_TYPE_PASSERELLE) );

    trame_pass = Trame_ajout_passerelle ( TRUE, infos->Trame_atelier, pass );
    trame_pass->groupe_dpl = Nouveau_groupe();                    /* Numéro de groupe pour le deplacement */

    g_signal_connect( G_OBJECT(trame_pass->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_pass), trame_pass );
    g_signal_connect( G_OBJECT(trame_pass->item_groupe), "button-release-event",
                      G_CALLBACK(Clic_sur_pass), trame_pass );
    g_signal_connect( G_OBJECT(trame_pass->item_groupe), "enter-notify-event",
                      G_CALLBACK(Clic_sur_pass), trame_pass );
    g_signal_connect( G_OBJECT(trame_pass->item_groupe), "leave-notify-event",
                      G_CALLBACK(Clic_sur_pass), trame_pass );
    g_signal_connect( G_OBJECT(trame_pass->item_groupe), "motion-notify-event",
                      G_CALLBACK(Clic_sur_pass), trame_pass );
  }
/**********************************************************************************************************/
/* Cacher_un_message: Enleve un message de la liste des messages                                          */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_une_passerelle_atelier( struct CMD_TYPE_PASSERELLE *pass )
  { struct TRAME_ITEM_PASS *trame_pass;
    struct TYPE_INFO_ATELIER *infos;
        
    infos = Rechercher_infos_atelier_par_id_syn ( pass->syn_id );
    trame_pass = Id_vers_trame_pass( infos, pass->id );
    if (!trame_pass) return;
    Deselectionner( infos, (struct TRAME_ITEM *)trame_pass );   /* Au cas ou il aurait été selectionné... */
    goo_canvas_item_remove( trame_pass->item_groupe );
    if (trame_pass->pass) g_free(trame_pass->pass);                      /* On libere la mémoire associée */
    g_free(trame_pass);
    infos->Trame_atelier->trame_items = g_list_remove( infos->Trame_atelier->trame_items, trame_pass );
    printf("Proto_cacher_un_pass_atelier fin..\n");
  }
/**********************************************************************************************************/
/* Afficher_un_icone: Ajoute un icone dans la liste des icones                                            */
/* Entrée: une reference sur le icone                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_syn_for_passerelle_atelier( struct CMD_TYPE_SYNOPTIQUE *synoptique )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_ATELIER)) return;

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_syn) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                         COLONNE_ID_SYN, synoptique->id,
                         COLONNE_LIBELLE_SYN, synoptique->libelle,
                         COLONNE_MNEMO_SYN, synoptique->page,
                         -1
                       );
  }
/*--------------------------------------------------------------------------------------------------------*/
