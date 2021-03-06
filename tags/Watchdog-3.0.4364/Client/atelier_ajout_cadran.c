/******************************************************************************************************************************/
/* Client/atelier_ajout_cadran.c     Gestion des cadrans synoptiques pour Watchdog                                            */
/* Projet WatchDog version 1.5     Gestion d'habitat                                             dim 29 jan 2006 16:59:01 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_ajout_cadran.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sébastien Lefevre
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

/********************************************* Définitions des prototypes programme *******************************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */

 static GtkWidget *F_ajout_cadran = NULL;                                             /* Fenetre graphique de choix de cadran */
 static GtkWidget *Entry_bitctrl;                                                                   /* Libelle proprement dit */
 static GtkWidget *Entry_tech_id;                                                                   /* Libelle proprement dit */
 static GtkWidget *Entry_acronyme;                                                                  /* Libelle proprement dit */
 static GtkWidget *Spin_bitctrl;
 static GtkWidget *Spin_nb_decimal;
 static GtkWidget *Option_left;                                                /* Type de capteur (totalisateur/moyenneur/..) */
 static GtkWidget *Combo_type;

/******************************************************************************************************************************/
/* Id_vers_trame_motif: Conversion d'un id motif en sa reference TRAME                                                        */
/* Entrée: Un id motif                                                                                                        */
/* sortie: un struct TRAME_ITEM_MOTIF                                                                                         */
/******************************************************************************************************************************/
 struct TRAME_ITEM_CADRAN *Id_vers_trame_cadran ( struct TYPE_INFO_ATELIER *infos, gint id )
  { GList *liste;
    liste = infos->Trame_atelier->trame_items;
    while( liste )
     { if ( *(gint *)(liste->data) == TYPE_CADRAN &&
            ((struct TRAME_ITEM_CADRAN *)(liste->data))->cadran->id == id ) break;
       else liste = liste->next;
     }
    if (!liste)
     { printf("Id_vers_trame_cadran: item %d non trouvé\n", id );
       return(NULL);
     }
    return( (struct TRAME_ITEM_CADRAN *)(liste->data) );
  }
/******************************************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                                      */
/* Entre: widget, data.                                                                                                       */
/* Sortie: void                                                                                                               */
/******************************************************************************************************************************/
 void Proto_afficher_mnemo_cadran_atelier ( struct CMD_TYPE_MNEMO_BASE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );                                 /* Formatage */
    gtk_entry_set_text( GTK_ENTRY(Entry_bitctrl), chaine );
  }
/******************************************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                                      */
/* Entre: widget, data.                                                                                                       */
/* Sortie: void                                                                                                               */
/******************************************************************************************************************************/
 static void Afficher_mnemo_cadran_ctrl ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    gchar *type_char;

    type_char = gtk_combo_box_get_active_text( GTK_COMBO_BOX(Combo_type) );
    mnemo.type = Type_bit_interne_int( type_char );
    g_free(type_char);

    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_bitctrl) );

    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_EA,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/******************************************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appelée qd on appuie sur un des boutons de l'interface                                  */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                                                   */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_ajouter_editer_cadran ( GtkDialog *dialog, gint reponse,
                                             struct TRAME_ITEM_CADRAN *trame_cadran )
  { struct CMD_TYPE_CADRAN add_cadran;
    struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    gchar *type;

    page = Page_actuelle();                                                                   /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return(TRUE);                             /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;                             /* Pointeur sur les infos de la page atelier */

    switch(reponse)
     { case GTK_RESPONSE_OK:
        { if (!trame_cadran)                                                          /* Ajout d'un cadran */
           { gchar *type;
             add_cadran.position_x = TAILLE_SYNOPTIQUE_X/2;
             add_cadran.position_y = TAILLE_SYNOPTIQUE_Y/2;
             add_cadran.syn_id  = infos->syn.id;
             add_cadran.angle   = 0;
             add_cadran.fleche_left = gtk_combo_box_get_active( GTK_COMBO_BOX(Option_left) );
             add_cadran.nb_decimal  = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_nb_decimal) );
             type = gtk_combo_box_get_active_text( GTK_COMBO_BOX(Combo_type) );
             add_cadran.type = Type_bit_interne_int( type );
             g_free(type);
             add_cadran.bit_controle = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bitctrl) );
             g_snprintf( add_cadran.tech_id, sizeof(add_cadran.tech_id), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_tech_id) ) );
             g_snprintf( add_cadran.acronyme, sizeof(add_cadran.acronyme), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_acronyme) ) );
             if (strlen(add_cadran.tech_id)) add_cadran.bit_controle = -1;
             Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_CADRAN,
                            (gchar *)&add_cadran, sizeof(struct CMD_TYPE_CADRAN) );
             printf("Requete d'ajout de cadran envoyée au serveur....\n");
             return(TRUE);                                                                    /* On laisse la fenetre ouverte */
           }
          else                                                                              /* Mise a jour */
           { type = gtk_combo_box_get_active_text( GTK_COMBO_BOX(Combo_type) );
             trame_cadran->cadran->type = Type_bit_interne_int( type );
             g_free(type);
             trame_cadran->cadran->fleche_left = gtk_combo_box_get_active( GTK_COMBO_BOX(Option_left) );
             trame_cadran->cadran->nb_decimal  = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_nb_decimal) );
             trame_cadran->cadran->bit_controle = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bitctrl) );
             g_snprintf( trame_cadran->cadran->tech_id, sizeof(trame_cadran->cadran->tech_id),
                         "%s", gtk_entry_get_text( GTK_ENTRY(Entry_tech_id) ) );
             g_snprintf( trame_cadran->cadran->acronyme, sizeof(trame_cadran->cadran->acronyme),
                         "%s", gtk_entry_get_text( GTK_ENTRY(Entry_acronyme) ) );
             if (strlen(trame_cadran->cadran->tech_id)) trame_cadran->cadran->bit_controle = -1;
             printf("Maj cadran  type=%d\n", trame_cadran->cadran->type );
           }
          break;
        }
       case GTK_RESPONSE_CLOSE: break;
     }
    gtk_widget_destroy( F_ajout_cadran );
    F_ajout_cadran = NULL;
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Commenter: Met en route le processus permettant de cadraner un synoptique                                                  */
/* Entrée: widget/data                                                                                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Menu_ajouter_editer_cadran ( struct TRAME_ITEM_CADRAN *trame_cadran )
  { GtkWidget *hboite, *table, *label;
    GtkObject *adj;
    if (F_ajout_cadran) return;
    F_ajout_cadran = gtk_dialog_new_with_buttons( (trame_cadran ? _("Editer un Cadran") : _("Ajouter un Cadran")),
                                             GTK_WINDOW(F_client),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                             GTK_STOCK_OK, GTK_RESPONSE_OK,
                                             NULL);
    g_signal_connect( F_ajout_cadran, "response",
                      G_CALLBACK(CB_ajouter_editer_cadran), trame_cadran );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout_cadran)->vbox ), hboite, TRUE, TRUE, 0 );

    table = gtk_table_new( 6, 2, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

/**************************************************** Entrys de commande ******************************************************/
    Combo_type = gtk_combo_box_new_text();
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_ENTREE_ANA) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_ENTREE) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_BISTABLE) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_CPTH) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_CPT_IMP) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_type), Type_bit_interne(MNEMO_REGISTRE) );
    gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 0 );
    g_signal_connect( G_OBJECT(Combo_type), "changed",
                      G_CALLBACK(Afficher_mnemo_cadran_ctrl), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_type, 0, 1, 0, 1 );

    adj = gtk_adjustment_new( -1, -1, NBR_BIT_DLS-1, 1, 100, 0 );
    Spin_bitctrl = gtk_spin_button_new( (GtkAdjustment *)adj, 0.5, 0.5);
    g_signal_connect( G_OBJECT(Spin_bitctrl), "changed",
                      G_CALLBACK(Afficher_mnemo_cadran_ctrl), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bitctrl, 1, 2, 0, 1 );

    label = gtk_label_new( _("Tech_ID") );
    gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 1, 1, 2 );
    Entry_tech_id = gtk_entry_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_tech_id, 1, 2, 1, 2 );

    label = gtk_label_new( _("Acronyme") );
    gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 1, 2, 3 );
    Entry_acronyme = gtk_entry_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_acronyme, 1, 2, 2, 3 );

    Entry_bitctrl = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bitctrl), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bitctrl, 0, 2, 3, 4 );

/**************************************************** Entrys de commande ******************************************************/
    label = gtk_label_new( _("Position Flèche") );
    gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 1, 4, 5 );
    Option_left = gtk_combo_box_new_text();
    gtk_combo_box_append_text( GTK_COMBO_BOX(Option_left), "Droite" );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Option_left), "Gauche" );
    gtk_combo_box_set_active( GTK_COMBO_BOX(Option_left), 0 );
    gtk_table_attach_defaults( GTK_TABLE(table), Option_left, 1, 2, 4, 5 );

    label = gtk_label_new( _("Nombre de décimales") );
    gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 1, 5, 6 );
    adj = gtk_adjustment_new( 0, 0, 2, 1, 1, 0 );
    Spin_nb_decimal = gtk_spin_button_new( (GtkAdjustment *)adj, 0.5, 0.5);
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_nb_decimal, 1, 2, 5, 6 );

    if (trame_cadran)
     { switch(trame_cadran->cadran->type)
        { default:
          case MNEMO_ENTREE_ANA: gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 0 ); break;
          case MNEMO_ENTREE    : gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 1 ); break;
          case MNEMO_BISTABLE  : gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 2 ); break;
          case MNEMO_CPTH      : gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 3 ); break;
          case MNEMO_CPT_IMP   : gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 4 ); break;
          case MNEMO_REGISTRE  : gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_type), 5 ); break;
        }
       gtk_combo_box_set_active( GTK_COMBO_BOX(Option_left), trame_cadran->cadran->fleche_left );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_nb_decimal), trame_cadran->cadran->nb_decimal );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bitctrl), trame_cadran->cadran->bit_controle );
       gtk_entry_set_text( GTK_ENTRY(Entry_tech_id), trame_cadran->cadran->tech_id );
       gtk_entry_set_text( GTK_ENTRY(Entry_acronyme), trame_cadran->cadran->acronyme );
     }
    Afficher_mnemo_cadran_ctrl();                                                 /* Pour mettre a jour le mnemonique associé */
    gtk_widget_show_all( F_ajout_cadran );
  }
/******************************************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                                          */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_cadran_atelier( struct CMD_TYPE_CADRAN *rezo_cadran )
  { struct TRAME_ITEM_CADRAN *trame_cadran;
    struct TYPE_INFO_ATELIER *infos;
    struct CMD_TYPE_CADRAN *cadran;

    infos = Rechercher_infos_atelier_par_id_syn ( rezo_cadran->syn_id );
    cadran = (struct CMD_TYPE_CADRAN *)g_try_malloc0( sizeof(struct CMD_TYPE_CADRAN) );
    if (!cadran) { printf("Cadran == NULL\n"); return; }

    memcpy ( cadran, rezo_cadran, sizeof( struct CMD_TYPE_CADRAN ) );

    trame_cadran = Trame_ajout_cadran ( TRUE, infos->Trame_atelier, cadran );
    trame_cadran->groupe_dpl = Nouveau_groupe();                                     /* Numéro de groupe pour le deplacement */

    g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_cadran), trame_cadran );
    g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "button-release-event",
                      G_CALLBACK(Clic_sur_cadran), trame_cadran );
    g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "enter-notify-event",
                      G_CALLBACK(Clic_sur_cadran), trame_cadran );
    g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "leave-notify-event",
                      G_CALLBACK(Clic_sur_cadran), trame_cadran );
    g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "motion-notify-event",
                      G_CALLBACK(Clic_sur_cadran), trame_cadran );
  }
/******************************************************************************************************************************/
/* Cacher_un_message: Enleve un message de la liste des messages                                                              */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_cacher_un_cadran_atelier( struct CMD_TYPE_CADRAN *cadran )
  { struct TRAME_ITEM_CADRAN *trame_cadran;
    struct TYPE_INFO_ATELIER *infos;

    infos = Rechercher_infos_atelier_par_id_syn ( cadran->syn_id );
    trame_cadran = Id_vers_trame_cadran( infos, cadran->id );
    printf("Proto_cacher_un_cadran_atelier debut: ID=%d %p\n", cadran->id, trame_cadran );
    if (!trame_cadran) return;
    Deselectionner( infos, (struct TRAME_ITEM *)trame_cadran );/* Au cas ou il aurait été selectionné... */
    Trame_del_cadran( trame_cadran );
    g_free(trame_cadran);
    infos->Trame_atelier->trame_items = g_list_remove( infos->Trame_atelier->trame_items, trame_cadran );
    printf("Proto_cacher_un_cadran_atelier fin..\n");
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
