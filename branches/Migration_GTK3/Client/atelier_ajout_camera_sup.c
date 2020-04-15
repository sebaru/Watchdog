/******************************************************************************************************************************/
/* Client/atelier_ajout_camera_sup.c     Gestion des camera_supaires pour Watchdog                                            */
/* Projet WatchDog version 1.5     Gestion d'habitat                                         dim. 13 sept. 2009 16:50:11 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_ajout_camera_sup.c
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

 #include <gnome.h>
 #include <string.h>
 #include <stdlib.h>

 #include "Reseaux.h"
 #include "trame.h"
 #include "Config_cli.h"

/******************************************** Définitions des prototypes programme ********************************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */

 static GtkWidget *F_ajout_camera_sup = NULL;                                     /* Fenetre graphique de choix de camera_sup */
 static GtkWidget *Liste_camera;                                          /* GtkTreeView pour la gestion des cameras Watchdog */

/******************************************************************************************************************************/
/* Id_vers_trame_motif: Conversion d'un id motif en sa reference TRAME                                                        */
/* Entrée: Un id motif                                                                                                        */
/* sortie: un struct TRAME_ITEM_MOTIF                                                                                         */
/******************************************************************************************************************************/
 struct TRAME_ITEM_CAMERA_SUP *Id_vers_trame_camera_sup ( struct TYPE_INFO_ATELIER *infos, gint id )
  { GList *liste;
    liste = infos->Trame_atelier->trame_items;
    while( liste )
     { if ( *(gint *)(liste->data) == TYPE_CAMERA_SUP &&
            ((struct TRAME_ITEM_CAMERA_SUP *)(liste->data))->camera_sup->id == id ) break;
       else liste = liste->next;
     }
    if (!liste)
     { printf("Id_vers_trame_camera_sup: item %d non trouvé\n", id );
       return(NULL);
     }
    return( (struct TRAME_ITEM_CAMERA_SUP *)(liste->data) );
  }
/******************************************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appelée qd on appuie sur un des boutons de l'interface                                  */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                                                   */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_ajouter_camera_sup ( GtkDialog *dialog, gint reponse, gpointer data )
  { struct CMD_TYPE_CAMERASUP add_camera_sup;
    struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gint id;

    page = Page_actuelle();                                                                   /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return(TRUE);                             /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;                             /* Pointeur sur les infos de la page atelier */

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_camera) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_camera) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            printf("lignes = %p\n", lignes );
            if (lignes)
             { gtk_tree_model_get_iter( store, &iter, lignes->data );                      /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COL_CAM_ID, &id, -1 );                                       /* Recup du num */
               g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
               g_list_free (lignes);                                                                    /* Liberation mémoire */

               add_camera_sup.posx   = TAILLE_SYNOPTIQUE_X/2;
               add_camera_sup.posy   = TAILLE_SYNOPTIQUE_Y/2;                            
               add_camera_sup.syn_id = infos->syn.id;
               add_camera_sup.camera_src_id = id;

               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_CAMERA_SUP,
                              (gchar *)&add_camera_sup, sizeof(struct CMD_TYPE_CAMERASUP) );
             }
            return(TRUE);                                                                     /* On laisse la fenetre ouverte */
       case GTK_RESPONSE_CLOSE:
            break;
     }
    gtk_widget_destroy( F_ajout_camera_sup );
    F_ajout_camera_sup = NULL;
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Commenter: Met en route le processus permettant de camera_super un synoptique                                              */
/* Entrée: widget/data                                                                                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Menu_ajouter_camera_sup ( void )
  { GtkWidget *hboite, *scroll;
    if (F_ajout_camera_sup) return;
    F_ajout_camera_sup = gtk_dialog_new_with_buttons( _("Add a Camera Sup" ),
                                             GTK_WINDOW(F_client),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                             GTK_STOCK_OK, GTK_RESPONSE_OK,
                                             NULL);
    g_signal_connect( F_ajout_camera_sup, "response",
                      G_CALLBACK(CB_ajouter_camera_sup), NULL );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout_camera_sup)->vbox ), hboite, TRUE, TRUE, 0 );
    
/**************************************************** La liste des cameras ****************************************************/
    Creer_liste_camera ( &Liste_camera, &scroll );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_WANT_PAGE_CAMERA_FOR_ATELIER,
                   NULL, 0 );                                                                        /* demande infos serveur */
    gtk_widget_show_all( F_ajout_camera_sup );
  }
/******************************************************************************************************************************/
/* Afficher_un_camera: Ajoute un camera dans la liste des cameras                                                             */
/* Entrée: une reference sur le camera                                                                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_camera_for_atelier( struct CMD_TYPE_CAMERA *camera )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_ATELIER)) return;
    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_camera) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_camera ( store, &iter, camera );
  }
/******************************************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                                          */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_camera_sup_atelier( struct CMD_TYPE_CAMERASUP *rezo_camera_sup )
  { struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    struct TYPE_INFO_ATELIER *infos;
    struct CMD_TYPE_CAMERASUP *camera_sup;
        
    infos = Rechercher_infos_atelier_par_id_syn ( rezo_camera_sup->syn_id );
    camera_sup = (struct CMD_TYPE_CAMERASUP *)g_try_malloc0( sizeof(struct CMD_TYPE_CAMERASUP) );
    if (!camera_sup)
     { return;
     }

    memcpy ( camera_sup, rezo_camera_sup, sizeof(struct CMD_TYPE_CAMERASUP) );

    trame_camera_sup = Trame_ajout_camera_sup ( TRUE, infos->Trame_atelier, camera_sup );
    if (!trame_camera_sup) return;
    trame_camera_sup->groupe_dpl = Nouveau_groupe();              /* Numéro de groupe pour le deplacement */

    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_camera_sup), trame_camera_sup );
    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "button-release-event",
                      G_CALLBACK(Clic_sur_camera_sup), trame_camera_sup );
    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "enter-notify-event",
                      G_CALLBACK(Clic_sur_camera_sup), trame_camera_sup );
    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "leave-notify-event",
                      G_CALLBACK(Clic_sur_camera_sup), trame_camera_sup );
    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "motion-notify-event",
                      G_CALLBACK(Clic_sur_camera_sup), trame_camera_sup );
  }
/******************************************************************************************************************************/
/* Cacher_un_message: Enleve un message de la liste des messages                                                              */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_cacher_un_camera_sup_atelier( struct CMD_TYPE_CAMERASUP *camera_sup )
  { struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    struct TYPE_INFO_ATELIER *infos;
        
    infos = Rechercher_infos_atelier_par_id_syn ( camera_sup->syn_id );
    trame_camera_sup = Id_vers_trame_camera_sup( infos, camera_sup->id );
    printf("Proto_cacher_un_camera_sup_atelier debut: ID=%d %p\n", camera_sup->id, trame_camera_sup );
    if (!trame_camera_sup) return;
    Deselectionner( infos, (struct TRAME_ITEM *)trame_camera_sup );                 /* Au cas ou il aurait été selectionné... */
    Trame_del_camera_sup( trame_camera_sup );
    g_free(trame_camera_sup);
    infos->Trame_atelier->trame_items = g_list_remove( infos->Trame_atelier->trame_items, trame_camera_sup );
    printf("Proto_cacher_un_camera_sup_atelier fin..\n");
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
