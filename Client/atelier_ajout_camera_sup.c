/**********************************************************************************************************/
/* Client/atelier_ajout_camera_sup.c     Gestion des camera_supaires pour Watchdog                        */
/* Projet WatchDog version 1.5     Gestion d'habitat                     dim. 13 sept. 2009 16:50:11 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * atelier_ajout_camera_sup.c
 * This file is part of <program name>
 *
 * Copyright (C) 2009 - sebastien
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #include <gnome.h>
 #include <string.h>
 #include <stdlib.h>

 #include "Reseaux.h"
 #include "trame.h"
 #include "Config_cli.h"

/********************************* D�finitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

 static GtkWidget *F_ajout_camera_sup = NULL;                 /* Fenetre graphique de choix de camera_sup */
 static GtkWidget *Spin_camsrc;
 static GtkWidget *Entry_camsrc;                                                /* Libelle proprement dit */

/**********************************************************************************************************/
/* Id_vers_trame_motif: Conversion d'un id motif en sa reference TRAME                                    */
/* Entr�e: Un id motif                                                                                    */
/* sortie: un struct TRAME_ITEM_MOTIF                                                                     */
/**********************************************************************************************************/
 struct TRAME_ITEM_CAMERA_SUP *Id_vers_trame_camera_sup ( struct TYPE_INFO_ATELIER *infos, gint id )
  { GList *liste;
    liste = infos->Trame_atelier->trame_items;
    while( liste )
     { if ( *(gint *)(liste->data) == TYPE_CAMERA_SUP &&
            ((struct TRAME_ITEM_CAMERA_SUP *)(liste->data))->camera_sup->id == id ) break;
       else liste = liste->next;
     }
    if (!liste)
     { printf("Id_vers_trame_camera_sup: item %d non trouv�\n", id );
       return(NULL);
     }
    return( (struct TRAME_ITEM_CAMERA_SUP *)(liste->data) );
  }
/**********************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appel�e qd on appuie sur un des boutons de l'interface              */
/* Entr�e: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_camera_sup ( GtkDialog *dialog, gint reponse,
                                                struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup )
  { struct CMD_TYPE_CAMERA_SUP add_camera_sup;
    struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    gchar *type;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return(TRUE);         /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    switch(reponse)
     { case GTK_RESPONSE_OK: if (!trame_camera_sup)                              /* Ajout d'un camera_sup */
                              { add_camera_sup.position_x = TAILLE_SYNOPTIQUE_X/2;
                                add_camera_sup.position_y = TAILLE_SYNOPTIQUE_Y/2;                            
                                add_camera_sup.syn_id  = infos->syn.id;
                                add_camera_sup.angle   = 0.0;
                                add_camera_sup.camera_src_id
                                         = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_camsrc) );

                                Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_CAMERA_SUP,
                                               (gchar *)&add_camera_sup, sizeof(struct CMD_TYPE_CAMERA_SUP) );
                                printf("Requete d'ajout de camera_supaire envoy�e au serveur....\n");
                                return(TRUE);                             /* On laisse la fenetre ouverte */
                              }
                             else                                                          /* Mise a jour */
                              { trame_camera_sup->camera_sup->camera_src_id
                                         = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_camsrc) );
                              }
                             break;
       case GTK_RESPONSE_CLOSE: break;
     }
    gtk_widget_destroy( F_ajout_camera_sup );
    F_ajout_camera_sup = NULL;
    return(TRUE);
  }
/**********************************************************************************************************/
/* Commenter: Met en route le processus permettant de camera_super un synoptique                             */
/* Entr�e: widget/data                                                                                    */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 void Menu_ajouter_editer_camera_sup ( struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup )
  { GtkWidget *hboite, *table, *label;
    GtkObject *adj;
    if (F_ajout_camera_sup) return;
    F_ajout_camera_sup = gtk_dialog_new_with_buttons( (trame_camera_sup ? _("Edit") : _("Add a Camera Sup")),
                                             GTK_WINDOW(F_client),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                             GTK_STOCK_OK, GTK_RESPONSE_OK,
                                             NULL);
    g_signal_connect( F_ajout_camera_sup, "response",
                      G_CALLBACK(CB_ajouter_editer_camera_sup), trame_camera_sup );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout_camera_sup)->vbox ), hboite, TRUE, TRUE, 0 );
    
    table = gtk_table_new( 1, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

/************************************* Entrys de commande *************************************************/
    label = gtk_label_new( _("Camera Number") );
    gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 1, 0, 1 );

    adj = gtk_adjustment_new( 0, 0, NBR_BIT_DLS-1, 1, 100, 0 );
    Spin_camsrc = gtk_spin_button_new( (GtkAdjustment *)adj, 0.5, 0.5);
/*    g_signal_connect( G_OBJECT(Spin_camsrc), "changed",
                      G_CALLBACK(Afficher_mnemo_camera_sup), NULL );*/
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_camsrc, 1, 2, 0, 1 );

/*    Entry_camsrc = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_camsrc), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_camsrc, 2, 4, 0, 1 );*/

    if (trame_camera_sup)
     { /*gtk_entry_set_text( GTK_ENTRY(Entry_camsrc), trame_camera_sup->camera_sup->libelle );*/
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_camsrc), trame_camera_sup->camera_sup->camera_src_id );
     }
    gtk_widget_show_all( F_ajout_camera_sup );
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entr�e: une reference sur le message                                                                   */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_camera_sup_atelier( struct CMD_TYPE_CAMERA_SUP *rezo_camera_sup )
  { struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    struct TYPE_INFO_ATELIER *infos;
    struct CMD_TYPE_CAMERA_SUP *camera_sup;
        
    infos = Rechercher_infos_atelier_par_id_syn ( rezo_camera_sup->syn_id );
    camera_sup = (struct CMD_TYPE_CAMERA_SUP *)g_malloc0( sizeof(struct CMD_TYPE_CAMERA_SUP) );
    if (!camera_sup)
     { Info( Config_cli.log, DEBUG_MEM, "Afficher_camera_sup_atelier: not enought memory" );
       return;
     }

    memcpy ( camera_sup, rezo_camera_sup, sizeof(struct CMD_TYPE_CAMERA_SUP) );

    trame_camera_sup = Trame_ajout_camera_sup ( TRUE, infos->Trame_atelier, camera_sup );
    trame_camera_sup->groupe_dpl = Nouveau_groupe();              /* Num�ro de groupe pour le deplacement */

/*    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_camera_sup), trame_camera_sup );
    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "button-release-event",
                      G_CALLBACK(Clic_sur_camera_sup), trame_camera_sup );
    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "enter-notify-event",
                      G_CALLBACK(Clic_sur_camera_sup), trame_camera_sup );
    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "leave-notify-event",
                      G_CALLBACK(Clic_sur_camera_sup), trame_camera_sup );
    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "motion-notify-event",
                      G_CALLBACK(Clic_sur_camera_sup), trame_camera_sup );*/
  }
/**********************************************************************************************************/
/* Cacher_un_message: Enleve un message de la liste des messages                                          */
/* Entr�e: une reference sur le message                                                                   */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_camera_sup_atelier( struct CMD_TYPE_CAMERA_SUP *camera_sup )
  { struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    struct TYPE_INFO_ATELIER *infos;
        
    infos = Rechercher_infos_atelier_par_id_syn ( camera_sup->syn_id );
    trame_camera_sup = Id_vers_trame_camera_sup( infos, camera_sup->id );
    printf("Proto_cacher_un_camera_sup_atelier debut: ID=%d %p\n", camera_sup->id, trame_camera_sup );
    if (!trame_camera_sup) return;
    Deselectionner( infos, (struct TRAME_ITEM *)trame_camera_sup );/* Au cas ou il aurait �t� selectionn�... */
    Trame_del_camera_sup( trame_camera_sup );
    g_free(trame_camera_sup);
    infos->Trame_atelier->trame_items = g_list_remove( infos->Trame_atelier->trame_items, trame_camera_sup );
    printf("Proto_cacher_un_camera_sup_atelier fin..\n");
  }
/*--------------------------------------------------------------------------------------------------------*/
