/**********************************************************************************************************/
/* Client/atelier_ajout_capteur.c     Gestion des capteurs synoptiques pour Watchdog                          */
/* Projet WatchDog version 1.5     Gestion d'habitat                         dim 29 jan 2006 16:59:01 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * atelier_ajout_capteur.c
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

 static GtkWidget *F_ajout_capteur = NULL;                         /* Fenetre graphique de choix de capteur */
 static GtkWidget *Entry_bitctrl;                                               /* Libelle proprement dit */
 static GtkWidget *Spin_bitctrl;
 static GtkWidget *Combo_type;

/**********************************************************************************************************/
/* Id_vers_trame_motif: Conversion d'un id motif en sa reference TRAME                                    */
/* Entrée: Un id motif                                                                                    */
/* sortie: un struct TRAME_ITEM_MOTIF                                                                     */
/**********************************************************************************************************/
 struct TRAME_ITEM_CAPTEUR *Id_vers_trame_capteur ( struct TYPE_INFO_ATELIER *infos, gint id )
  { GList *liste;
    liste = infos->Trame_atelier->trame_items;
    while( liste )
     { if ( *(gint *)(liste->data) == TYPE_CAPTEUR &&
            ((struct TRAME_ITEM_CAPTEUR *)(liste->data))->capteur->id == id ) break;
       else liste = liste->next;
     }
    if (!liste)
     { printf("Id_vers_trame_capteur: item %d non trouvé\n", id );
       return(NULL);
     }
    return( (struct TRAME_ITEM_CAPTEUR *)(liste->data) );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Proto_afficher_mnemo_capteur_atelier ( struct CMD_TYPE_MNEMO_BASE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );             /* Formatage */
    gtk_entry_set_text( GTK_ENTRY(Entry_bitctrl), chaine );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Afficher_mnemo_capteur_ctrl ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    gchar *type_char;

    type_char = gtk_combo_box_get_active_text( GTK_COMBO_BOX(Combo_type) );
    mnemo.type = Type_bit_interne_int( type_char );
    g_free(type_char);
                                
    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_bitctrl) );
    
    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_EA,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_capteur ( GtkDialog *dialog, gint reponse,
                                           struct TRAME_ITEM_CAPTEUR *trame_capteur )
  { struct CMD_TYPE_CAPTEUR add_capteur;
    struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    gchar *type;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return(TRUE);         /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    switch(reponse)
     { case GTK_RESPONSE_OK: if (!trame_capteur)                                        /* Ajout d'un capteur */
                              { gchar *type;
                                add_capteur.position_x = TAILLE_SYNOPTIQUE_X/2;
                                add_capteur.position_y = TAILLE_SYNOPTIQUE_Y/2;                            
                                add_capteur.syn_id  = infos->syn.id;
                                add_capteur.angle   = 0.0;
                                type = gtk_combo_box_get_active_text( GTK_COMBO_BOX(Combo_type) );
                                add_capteur.type = Type_bit_interne_int( type );
                                g_free(type);
                                add_capteur.bit_controle = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bitctrl) );

                                Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_CAPTEUR,
                                               (gchar *)&add_capteur, sizeof(struct CMD_TYPE_CAPTEUR) );
                                printf("Requete d'ajout de capteuraire envoyée au serveur....\n");
                                return(TRUE);                             /* On laisse la fenetre ouverte */
                              }
                             else                                                          /* Mise a jour */
                              { type = gtk_combo_box_get_active_text( GTK_COMBO_BOX(Combo_type) );
                                trame_capteur->capteur->type = Type_bit_interne_int( type );
                                g_free(type);
                                trame_capteur->capteur->bit_controle = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bitctrl) );
                                printf("Maj capteur  type=%d\n", trame_capteur->capteur->type );
                              }
                             break;
       case GTK_RESPONSE_CLOSE: break;
     }
    gtk_widget_destroy( F_ajout_capteur );
    F_ajout_capteur = NULL;
    return(TRUE);
  }
/**********************************************************************************************************/
/* Commenter: Met en route le processus permettant de capteurer un synoptique                             */
/* Entrée: widget/data                                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Menu_ajouter_editer_capteur ( struct TRAME_ITEM_CAPTEUR *trame_capteur )
  { GtkWidget *hboite, *table, *label;
    GtkObject *adj;
    if (F_ajout_capteur) return;
    F_ajout_capteur = gtk_dialog_new_with_buttons( (trame_capteur ? _("Edit") : _("Add a Texte")),
                                             GTK_WINDOW(F_client),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                             GTK_STOCK_OK, GTK_RESPONSE_OK,
                                             NULL);
    g_signal_connect( F_ajout_capteur, "response",
                      G_CALLBACK(CB_ajouter_editer_capteur), trame_capteur );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout_capteur)->vbox ), hboite, TRUE, TRUE, 0 );
    
    table = gtk_table_new( 3, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

/************************************* Entrys de commande *************************************************/
    label = gtk_label_new( _("Type Control bit") );
    gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 2, 0, 1 );
    Combo_type = gtk_combo_box_new_text();
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_ENTREE_ANA) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_ENTREE) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_BISTABLE) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_CPTH) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_CPT_IMP) );
    gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 0 );
    g_signal_connect( G_OBJECT(Combo_type), "changed",
                      G_CALLBACK(Afficher_mnemo_capteur_ctrl), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_type, 2, 4, 0, 1 );

    label = gtk_label_new( _("Control bit") );
    gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 2, 1, 2 );

    adj = gtk_adjustment_new( 0, 0, NBR_BIT_DLS-1, 1, 100, 0 );
    Spin_bitctrl = gtk_spin_button_new( (GtkAdjustment *)adj, 0.5, 0.5);
    g_signal_connect( G_OBJECT(Spin_bitctrl), "changed",
                      G_CALLBACK(Afficher_mnemo_capteur_ctrl), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bitctrl, 2, 4, 1, 2 );

    Entry_bitctrl = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bitctrl), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bitctrl, 0, 4, 2, 3 );

    if (trame_capteur)
     { gtk_entry_set_text( GTK_ENTRY(Entry_bitctrl), trame_capteur->capteur->libelle );
       switch(trame_capteur->capteur->type)
        { default:
          case MNEMO_ENTREE_ANA: gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 0 ); break;
          case MNEMO_ENTREE    : gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 1 ); break;
          case MNEMO_BISTABLE  : gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 2 ); break;
          case MNEMO_CPTH      : gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 3 ); break;
          case MNEMO_CPT_IMP   : gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 4 ); break;
        }
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bitctrl), trame_capteur->capteur->bit_controle );
     }
    Afficher_mnemo_capteur_ctrl();                              /* Pour mettre a jour le mnemonique associé */
    gtk_widget_show_all( F_ajout_capteur );
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_capteur_atelier( struct CMD_TYPE_CAPTEUR *rezo_capteur )
  { struct TRAME_ITEM_CAPTEUR *trame_capteur;
    struct TYPE_INFO_ATELIER *infos;
    struct CMD_TYPE_CAPTEUR *capteur;
        
    infos = Rechercher_infos_atelier_par_id_syn ( rezo_capteur->syn_id );
    capteur = (struct CMD_TYPE_CAPTEUR *)g_try_malloc0( sizeof(struct CMD_TYPE_CAPTEUR) );
    if (!capteur)
     { return;
     }

    memcpy ( capteur, rezo_capteur, sizeof( struct CMD_TYPE_CAPTEUR ) );

    trame_capteur = Trame_ajout_capteur ( TRUE, infos->Trame_atelier, capteur );
    trame_capteur->groupe_dpl = Nouveau_groupe();                 /* Numéro de groupe pour le deplacement */

    g_signal_connect( G_OBJECT(trame_capteur->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_capteur), trame_capteur );
    g_signal_connect( G_OBJECT(trame_capteur->item_groupe), "button-release-event",
                      G_CALLBACK(Clic_sur_capteur), trame_capteur );
    g_signal_connect( G_OBJECT(trame_capteur->item_groupe), "enter-notify-event",
                      G_CALLBACK(Clic_sur_capteur), trame_capteur );
    g_signal_connect( G_OBJECT(trame_capteur->item_groupe), "leave-notify-event",
                      G_CALLBACK(Clic_sur_capteur), trame_capteur );
    g_signal_connect( G_OBJECT(trame_capteur->item_groupe), "motion-notify-event",
                      G_CALLBACK(Clic_sur_capteur), trame_capteur );
  }
/**********************************************************************************************************/
/* Cacher_un_message: Enleve un message de la liste des messages                                          */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_capteur_atelier( struct CMD_TYPE_CAPTEUR *capteur )
  { struct TRAME_ITEM_CAPTEUR *trame_capteur;
    struct TYPE_INFO_ATELIER *infos;
        
    infos = Rechercher_infos_atelier_par_id_syn ( capteur->syn_id );
    trame_capteur = Id_vers_trame_capteur( infos, capteur->id );
    printf("Proto_cacher_un_capteur_atelier debut: ID=%d %p\n", capteur->id, trame_capteur );
    if (!trame_capteur) return;
    Deselectionner( infos, (struct TRAME_ITEM *)trame_capteur );/* Au cas ou il aurait été selectionné... */
    Trame_del_capteur( trame_capteur );
    g_free(trame_capteur);
    infos->Trame_atelier->trame_items = g_list_remove( infos->Trame_atelier->trame_items, trame_capteur );
    printf("Proto_cacher_un_capteur_atelier fin..\n");
  }
/*--------------------------------------------------------------------------------------------------------*/
