/******************************************************************************************************************************/
/* Client/atelier_propriete_motif.c         gestion des selections synoptique                                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 04 avr 2009 13:43:20 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_propriete_motif.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - Sébastien Lefevre
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
/******************************************* Définitions des prototypes programme *********************************************/
 #include "protocli.h"

 static gchar *TYPE_GESTION_MOTIF[]=                      /* Type de gestion d'un motif sur un synoptique */
  { "Inerte (Mur)",
    "Fond d'ecran",
    "Actif (Lampe)",
    "Repos/Actif (Porte/Volet)",
    "Repos/Anime(0-n) (Moteur)",
    "Repos/Anime(1-n) (Moteur)",
    "Repos/Anime(2-n) (Moteur)",
    "Repos/Anime/Repos (Rideau)",
    "Inerte/Repos/Actif (Bouton)",
    NULL
  };

 static gchar *TYPE_DIALOG_CDE[]=                   /* Type de boite de dialogue lors d'un clic sur motif */
  { "Pas d'action",
    "immediate",
    "avec confirmation",
    NULL
  };

 #define RESOLUTION_TIMER        500
 #define TIMER_OFF     0
 #define TIMER_ON      1

 #define ROUGE1     0
 #define VERT1    255
 #define BLEU1      0

#ifdef bouh
/**********************************************************************************************************/
/* Type_gestion_motif: Renvoie le type correspondant au numero passé en argument                          */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 static gchar *Type_gestion_motif ( gint num )
  { if (num>sizeof(TYPE_GESTION_MOTIF)/sizeof(TYPE_GESTION_MOTIF[0])-1)
     return("Type_gestion_motif: Erreur interne");
    return( TYPE_GESTION_MOTIF[num] );
  }
/**********************************************************************************************************/
/* Type_dialog_cde: Renvoie le type correspondant au numero passé en argument                             */
/* Entrée: le numero du type                                                                              */
/* Sortie: le type                                                                                        */
/**********************************************************************************************************/
 static gchar *Type_dialog_cde ( gint num )
  { if (num>sizeof(TYPE_DIALOG_CDE)/sizeof(TYPE_DIALOG_CDE[0])-1)
     return("Type_dialog_cde: Erreur interne");
    return( TYPE_DIALOG_CDE[num] );
  }
/**********************************************************************************************************/
/* Fonction TIMER  appélée toutes les 10 millisecondes    pour rafraichissment visuel                     */
/**********************************************************************************************************/
 static gint Timer_preview ( gpointer data )
  { static gint top=0;

    if (!ok_timer) return(TRUE);

    if (Trame_motif->motif->type_gestion == TYPE_DYNAMIQUE ||
        Trame_motif->motif->type_gestion == TYPE_BOUTON )
     { Trame_choisir_frame( Trame_motif_p1, Trame_motif_p1->num_image+1,
                                            ROUGE1, VERT1, BLEU1 );                    /* frame numero ++ */
     }
    else if (Trame_motif->motif->type_gestion == TYPE_CYCLIQUE_0N ||
             Trame_motif->motif->type_gestion == TYPE_PROGRESSIF
            )
     { if (top >= Trame_motif->motif->rafraich)
        { Trame_choisir_frame( Trame_motif_p1, Trame_motif_p1->num_image+1,
                                               ROUGE1, VERT1, BLEU1 );                 /* frame numero ++ */
          top = 0;                                                            /* Raz pour prochaine frame */
        }
     }
    else if (Trame_motif->motif->type_gestion == TYPE_CYCLIQUE_1N)
     { if (top >= Trame_motif->motif->rafraich)
        { if (Trame_motif_p1->num_image == Trame_motif_p1->nbr_images-1)
           { Trame_choisir_frame( Trame_motif_p1, 1,
                                  ROUGE1, VERT1, BLEU1 );                              /* frame numero ++ */
           }
          else
           { Trame_choisir_frame( Trame_motif_p1, Trame_motif_p1->num_image+1,
                                  ROUGE1, VERT1, BLEU1 );                              /* frame numero ++ */
           }
          top = 0;                                                            /* Raz pour prochaine frame */
        }
     }
    else if (Trame_motif->motif->type_gestion == TYPE_CYCLIQUE_2N)
     { if (top >= Trame_motif->motif->rafraich)
        { if (Trame_motif_p1->num_image == Trame_motif_p1->nbr_images-1)
           { Trame_choisir_frame( Trame_motif_p1, 2,
                                  ROUGE1, VERT1, BLEU1 );                              /* frame numero ++ */
           }
          else
           { Trame_choisir_frame( Trame_motif_p1, Trame_motif_p1->num_image+1,
                                  ROUGE1, VERT1, BLEU1 );                              /* frame numero ++ */
           }
          top = 0;                                                            /* Raz pour prochaine frame */
        }
     }
    top += RESOLUTION_TIMER;                                           /* sinon, la prochaine sera +10 ms */
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Rafraichir_sensibilite: met a jour la sensibilite des widgets de la fenetre propriete                                      */
/* Entrée: void                                                                                                               */
/* Sortie: void                                                                                                               */
/******************************************************************************************************************************/
 void Rafraichir_sensibilite ( void )
  { gtk_widget_set_sensitive( Spin_rafraich, FALSE );
    gtk_widget_set_sensitive( Spin_bit_ctrl, TRUE );
    gtk_widget_set_sensitive( Entry_bit_ctrl, TRUE );
    printf("Rafraichir_sensibilite1\n" );

    switch( Trame_motif->motif->type_gestion )
     { case TYPE_FOND:
       case TYPE_INERTE  :   gtk_widget_set_sensitive( Spin_bit_ctrl, FALSE );                      /* Pas de bit de controle */
                             gtk_widget_set_sensitive( Entry_bit_ctrl, FALSE );
                             /*gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bit_ctrl), 0 );*/
                             Trame_choisir_frame( Trame_motif_p1, 0,
                                                                  Trame_motif->motif->rouge0,
                                                                  Trame_motif->motif->vert0,
                                                                  Trame_motif->motif->bleu0 ); /* frame 0 */
                             ok_timer = TIMER_OFF;
                             break;
       case TYPE_STATIQUE:   Trame_choisir_frame( Trame_motif_p1, 0, ROUGE1, VERT1, BLEU1 );   /* frame 0 */
                             ok_timer = TIMER_OFF;
                             break;
       case TYPE_BOUTON    :
       case TYPE_PROGRESSIF:
       case TYPE_DYNAMIQUE : Trame_choisir_frame( Trame_motif_p1, 1, ROUGE1, VERT1, BLEU1 );   /* frame 1 */
                             ok_timer = TIMER_ON;
                             break;
       case TYPE_CYCLIQUE_0N:
       case TYPE_CYCLIQUE_1N:
       case TYPE_CYCLIQUE_2N:gtk_widget_set_sensitive( Spin_rafraich, TRUE );
                             ok_timer = TIMER_ON;
                             printf("type cyclique: ok_timer = %d\n", ok_timer );
                             break;
       default: printf("Rafraichir_sensibilite: type_gestion inconnu\n" );
     }

    switch( Trame_motif->motif->type_dialog )
     { case ACTION_SANS      : gtk_widget_set_sensitive( Spin_bit_clic, FALSE );
                               gtk_widget_set_sensitive( Entry_bit_clic, FALSE );
                               gtk_widget_set_sensitive( Entry_clic_tech_id, FALSE );
                               gtk_widget_set_sensitive( Entry_clic_acronyme, FALSE );
                               gtk_widget_set_sensitive( Spin_access_level, FALSE );
                               break;
       case ACTION_IMMEDIATE : gtk_widget_set_sensitive( Spin_bit_clic, TRUE );
                               gtk_widget_set_sensitive( Entry_bit_clic, TRUE );
                               gtk_widget_set_sensitive( Entry_clic_tech_id, TRUE );
                               gtk_widget_set_sensitive( Entry_clic_acronyme, TRUE );
                               gtk_widget_set_sensitive( Spin_access_level, TRUE );
                               break;
       case ACTION_CONFIRME  : gtk_widget_set_sensitive( Spin_bit_clic, TRUE );
                               gtk_widget_set_sensitive( Entry_bit_clic, TRUE );
                               gtk_widget_set_sensitive( Entry_clic_tech_id, TRUE );
                               gtk_widget_set_sensitive( Entry_clic_acronyme, TRUE );
                               gtk_widget_set_sensitive( Spin_access_level, TRUE );
                               break;
     }
  }
/**********************************************************************************************************/
/* Changer_gestion_motif: Change le type de gestion de l'icone                                            */
/* Entrée: void                                                                                           */
/* Sortie: la base de données est mise à jour                                                             */
/**********************************************************************************************************/
 static void Changer_gestion_motif ( void )
  { Trame_motif->motif->type_gestion = gtk_combo_box_get_active( GTK_COMBO_BOX(Combo_gestion) );
    printf("Gestion = %s\n", Type_gestion_motif( Trame_motif->motif->type_gestion) );
    Rafraichir_sensibilite();                           /* Pour mettre a jour les sensibility des widgets */
  }
/**********************************************************************************************************/
/* Changer_dialog_cde: Change le type de dialogue clic gauche motif                                       */
/* Entrée: rien                                                                                           */
/* Sortie: la base de données est mise à jour                                                             */
/**********************************************************************************************************/
 static void Changer_dialog_cde ( void )
  { Trame_motif->motif->type_dialog = gtk_option_menu_get_history( GTK_OPTION_MENU(Option_dialog_cde) );
    printf("dialog = %s\n", Type_dialog_cde( Trame_motif->motif->type_gestion ) );
    Rafraichir_sensibilite();                           /* Pour mettre a jour les sensibility des widgets */
  }
/**********************************************************************************************************/
/* Changer_rafraich: Changement du taux de rafraichissement du motif                                      */
/* Entrée: widget, data                                                                                   */
/* Sortie: la base de données est mise à jour                                                             */
/**********************************************************************************************************/
 static void Changer_rafraich( GtkWidget *widget, gpointer data )
  { Trame_motif->motif->rafraich = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(Spin_rafraich));
  }
/******************************************************************************************************************************/
/* Changer_couleur: Changement de la couleur du motif                                                                         */
/* Entrée: widget, data = 0 pour un chgmt via propriete DLS, 1 pour chgmt via Couleur par Def                                 */
/* Sortie: la base de données est mise à jour                                                                                 */
/******************************************************************************************************************************/
 static void Changer_default_couleur ( GtkColorChooser *chooser, GdkRGBA *color, gpointer user_data)
  { struct TRAME_ITEM_MOTIF *trame_motif = user_data;
printf("%s\n", __func__ );
    //g_snprintf( trame_motif->motif->def_color, sizeof(trame_motif->motif->def_color), "%s", gdk_rgba_to_string (color) );
    trame_motif->motif->rouge0 = color->red*255.0;
    trame_motif->motif->vert0  = color->green*255.0;
    trame_motif->motif->bleu0  = color->blue*255.0;
    Trame_peindre_motif( trame_motif, trame_motif->motif->rouge0,
                                      trame_motif->motif->vert0,
                                      trame_motif->motif->bleu0 );
  }
#endif
#ifdef bouh
/******************************************************************************************************************************/
/* Changer_couleur_motif_directe: Changement de la couleur du motif en direct live                                            */
/* Entrée: widget, data =0 pour inactive, 1 pour active                                                                       */
/* Sortie: la base de données est mise à jour                                                                                 */
/******************************************************************************************************************************/
 void Changer_couleur_motif_directe( struct TRAME_ITEM_MOTIF *trame_motif )
  { GtkWidget *dialog = gtk_dialog_new_with_buttons( "Choisir la couleur par défaut",
                                                     GTK_WINDOW(trame_motif->page->client),
                                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                     "Annuler", GTK_RESPONSE_CANCEL,
                                                     "Valider", GTK_RESPONSE_OK,
                                                     NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    GtkWidget *choix = gtk_color_chooser_widget_new();              /* Creation de la zone de saisie de la couleur par defaut */
    gtk_box_pack_start (GTK_BOX (content_area), choix, TRUE, TRUE, 5);

    GdkRGBA color;
    //gdk_rgba_parse ( &color, trame_motif->motif->def_color );
    color.red   = trame_motif->motif->rouge0/255.0;
    color.green = trame_motif->motif->vert0/255.0;
    color.blue  = trame_motif->motif->bleu0/255.0;
    color.alpha = 1.0;
    gtk_color_chooser_set_rgba ( GTK_COLOR_CHOOSER(choix), &color );
    gtk_color_chooser_set_use_alpha ( GTK_COLOR_CHOOSER(choix), TRUE );
    g_signal_connect( G_OBJECT( choix ), "color-activated", G_CALLBACK( Changer_default_couleur ), trame_motif );
    gtk_widget_show_all ( dialog );
    if (gtk_dialog_run( GTK_DIALOG(dialog) ) == GTK_RESPONSE_OK)                       /* Attente de reponse de l'utilisateur */
     { gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(choix), &color); }
    Changer_default_couleur ( GTK_COLOR_CHOOSER(choix), &color, trame_motif );
    gtk_widget_destroy ( dialog );
  }
/**********************************************************************************************************/
/* Rafraichir_propriete: Rafraichit les données de la fenetre d'edition des proprietes du motif           */
/* Entrée: Le trame_motif souhaité                                                                        */
/* Sortie: niet                                                                                           */
/**********************************************************************************************************/
 static void Rafraichir_propriete ( struct TRAME_ITEM_MOTIF *trame_motif )
  { struct CMD_TYPE_MOTIF *motif;
    gint i;

    Trame_motif = trame_motif;                  /* Sauvegarde pour les futurs changements d'environnement */

    if (!F_propriete) return;
    motif = Trame_motif->motif;

    gnome_color_picker_set_i8 ( GNOME_COLOR_PICKER(Couleur_inactive),
                                motif->rouge0, motif->vert0, motif->bleu0, 0 );

    memcpy( &Motif_preview0, trame_motif->motif, sizeof(struct CMD_TYPE_MOTIF) );/* Recopie et ajustement */
    memcpy( &Motif_preview1, trame_motif->motif, sizeof(struct CMD_TYPE_MOTIF) );/* Recopie et ajustement */
    Reduire_en_vignette( &Motif_preview0 );
    Reduire_en_vignette( &Motif_preview1 );
    Motif_preview0.position_x = Motif_preview1.position_x = TAILLE_ICONE_X/2;
    Motif_preview0.position_y = Motif_preview1.position_y = TAILLE_ICONE_Y/2;

    if (Trame_motif_p0)
     { goo_canvas_item_remove( Trame_motif_p0->item_groupe );
       Trame_preview0->trame_items = g_list_remove( Trame_preview0->trame_items, Trame_motif_p0 );
       g_object_unref( Trame_motif_p0->pixbuf );
       g_free(Trame_motif_p0);
     }
    if (Trame_motif_p1)
     { goo_canvas_item_remove( Trame_motif_p1->item_groupe );
       Trame_preview1->trame_items = g_list_remove( Trame_preview1->trame_items, Trame_motif_p1 );
       g_object_unref( Trame_motif_p1->pixbuf );
       g_free(Trame_motif_p1);
     }
    Trame_motif_p0 = Trame_ajout_motif( TRUE, Trame_preview0, &Motif_preview0 );   /* Affichage à l'ecran */
    Trame_motif_p1 = Trame_ajout_motif( TRUE, Trame_preview1, &Motif_preview1 );

    gtk_entry_set_text( GTK_ENTRY(Entry_libelle), motif->libelle );

    printf("Rafraichir_proprietes1:  ctrl=%d clic=%d\n", motif->bit_controle, motif->bit_clic );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bit_ctrl), motif->bit_controle );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bit_clic), motif->bit_clic );
    gtk_entry_set_text( GTK_ENTRY(Entry_clic_tech_id), motif->clic_tech_id );
    gtk_entry_set_text( GTK_ENTRY(Entry_clic_acronyme), motif->clic_acronyme );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_rafraich), motif->rafraich );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_access_level), motif->access_level );
    printf("Rafraichir_proprietes2:  ctrl=%d clic=%d\n", motif->bit_controle, motif->bit_clic );

    g_signal_handlers_block_by_func( G_OBJECT( GTK_COMBO_BOX(Combo_gestion) ),
                                     G_CALLBACK( Changer_gestion_motif ), NULL );
    for (i=0; i<NBR_TYPE_GESTION_MOTIF; i++) gtk_combo_box_remove_text( GTK_COMBO_BOX(Combo_gestion), 0 );
    printf("Nombre de images motif! %d....\n", trame_motif->nbr_images );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_INERTE   ) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_FOND     ) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_STATIQUE ) );
    printf("Rafraichir_proprietes3:  ctrl=%d clic=%d\n", motif->bit_controle, motif->bit_clic );

    if (trame_motif->nbr_images >= 2)                                    /* Sensibilite des boutons radio */
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_DYNAMIQUE   ) );
       gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_CYCLIQUE_0N ) );
     }
    if (trame_motif->nbr_images >= 3)
     { gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_CYCLIQUE_1N ) );
       gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_CYCLIQUE_2N ) );
       gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_PROGRESSIF  ) );
       gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_BOUTON      ) );
     }
    gtk_widget_show_all(Combo_gestion);
    g_signal_handlers_unblock_by_func( G_OBJECT( GTK_COMBO_BOX(Combo_gestion) ),
                                       G_CALLBACK( Changer_gestion_motif ), NULL );

    gtk_combo_box_set_active( GTK_COMBO_BOX(Combo_gestion), motif->type_gestion );

    gtk_option_menu_set_history( GTK_OPTION_MENU(Option_dialog_cde), motif->type_dialog );
    printf("Rafraichir_proprietes8:  ctrl=%d clic=%d\n", motif->bit_controle, motif->bit_clic );

    Rafraichir_sensibilite();  /* test 18/01/2006 */
    printf("rafraichir_propriete Oktimer = %d\n", ok_timer );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Proto_afficher_mnemo_atelier ( int tag, struct CMD_TYPE_MNEMO_BASE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );             /* Formatage */
    switch (tag)
     { case SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC:
            gtk_entry_set_text( GTK_ENTRY(Entry_bit_clic), chaine );
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_CTRL:
            gtk_entry_set_text( GTK_ENTRY(Entry_bit_ctrl), chaine );
            break;
     }
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Afficher_mnemo_clic ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;

    mnemo.type = MNEMO_MONOSTABLE;
    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_bit_clic) );
    Trame_motif->motif->bit_clic = mnemo.num;

    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_TYPE_NUM_MNEMO_CLIC,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Afficher_mnemo_ctrl ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    mnemo.type = MNEMO_VISUEL;
    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_bit_ctrl) );
    Trame_motif->motif->bit_controle = mnemo.num;

    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_TYPE_NUM_MNEMO_CTRL,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Editer_propriete_TOR: Mise à jour des parametres de la fenetre edition motif et du motif proprement dit*/
/* Entrée: une structure referencant le motif à editer                                                    */
/* Sortie: niet                                                                                           */
/**********************************************************************************************************/
 void Editer_propriete_TOR ( struct TRAME_ITEM_MOTIF *trame_motif )
  { Rafraichir_propriete( trame_motif );             /* On rafraichit les données visuelles de la fenetre */
    gtk_widget_show( F_propriete );
    ok_timer = TIMER_ON;                                                                /* Arret du timer */
  }
/******************************************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appelée qd on appuie sur un des boutons de l'interface                                  */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                                                   */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_editer_propriete_TOR ( GtkDialog *dialog, gint reponse,
                                           gboolean edition )
  { switch(reponse)
     { case GTK_RESPONSE_OK:
       case GTK_RESPONSE_CANCEL: break;
     }
    g_snprintf( Trame_motif->motif->libelle, sizeof(Trame_motif->motif->libelle),
               "%s", gtk_entry_get_text( GTK_ENTRY(Entry_libelle) ) );
    g_snprintf( Trame_motif->motif->clic_tech_id, sizeof(Trame_motif->motif->clic_tech_id),
               "%s", gtk_entry_get_text( GTK_ENTRY(Entry_clic_tech_id) ) );
    g_snprintf( Trame_motif->motif->clic_acronyme, sizeof(Trame_motif->motif->clic_acronyme),
               "%s", gtk_entry_get_text( GTK_ENTRY(Entry_clic_acronyme) ) );
    g_strcanon( Trame_motif->motif->clic_acronyme, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", '_' );
    ok_timer = TIMER_OFF;                                                                                   /* Arret du timer */
    gtk_widget_hide( F_propriete );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Detruire_fenetre_propriete_TOR: Destruction de la fenetre de parametres DLS                                                */
/* Entrée: rien                                                                                                               */
/* Sortie: toute trace de la fenetre est eliminée                                                                             */
/******************************************************************************************************************************/
 void Detruire_fenetre_propriete_TOR ( void )
  {
    ok_timer = TIMER_OFF;
    gtk_timeout_remove( Tag_timer );                                                      /* On le vire des fonctions actives */
    if (Trame_preview0) Trame_detruire_trame( Trame_preview0 );
    if (Trame_preview1) Trame_detruire_trame( Trame_preview1 );
    gtk_widget_destroy( F_propriete );
    g_list_free( Liste_index_groupe );
    Trame_motif_p0 = NULL;
    Trame_motif_p1 = NULL;
    F_propriete = NULL;
  }
/******************************************************************************************************************************/
/* Creer_fenetre_propriete_TOR: Creation de la fenetre d'edition des proprietes TOR                                           */
/* Entrée: niet                                                                                                               */
/* Sortie: niet                                                                                                               */
/******************************************************************************************************************************/
 void Creer_fenetre_propriete_TOR ( struct TYPE_INFO_ATELIER *infos )
  { GtkWidget *texte, *table, *bouton, *separator;
    GtkWidget *boite, *Frame, *hboite, *menu;
    GtkObject *adj;
    gint cpt;

    F_propriete = gtk_dialog_new_with_buttons( _("Edit a item"),
                                               GTK_WINDOW(F_client),
                                               GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_STOCK_OK, GTK_RESPONSE_OK,
                                               NULL);
    g_signal_connect( F_propriete, "response",     G_CALLBACK(CB_editer_propriete_TOR), FALSE );
    g_signal_connect( F_propriete, "delete-event", G_CALLBACK(CB_editer_propriete_TOR), FALSE );

/****************************************** Frame de representation du motif actif ********************************************/
    Frame = gtk_frame_new( _("Properties") );
    gtk_frame_set_label_align( GTK_FRAME(Frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_propriete)->vbox ), Frame, TRUE, TRUE, 0 );

    boite = gtk_vbox_new( FALSE, 6 );
    gtk_container_add( GTK_CONTAINER(Frame), boite );
    gtk_container_set_border_width( GTK_CONTAINER(boite), 6 );

    Trame_preview0 = Trame_creer_trame( TAILLE_ICONE_X, TAILLE_ICONE_Y, "darkgray", 0 );
    gtk_widget_set_usize( Trame_preview0->trame_widget, TAILLE_ICONE_X, TAILLE_ICONE_Y );
    Trame_preview1 = Trame_creer_trame( TAILLE_ICONE_X, TAILLE_ICONE_Y, "darkgray", 0 );
    gtk_widget_set_usize( Trame_preview1->trame_widget, TAILLE_ICONE_X, TAILLE_ICONE_Y );
printf("Creer_fenetre_propriete_TOR: trame_p0=%p, trame_p1=%p\n", Trame_preview0, Trame_preview1 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(boite), hboite, FALSE, FALSE, 0 );

    bouton = gtk_button_new();
    gtk_container_add( GTK_CONTAINER(bouton), Trame_preview0->trame_widget );
    gtk_box_pack_start( GTK_BOX(hboite), bouton, TRUE, FALSE, 0 );

    bouton = gtk_button_new();
    gtk_container_add( GTK_CONTAINER(bouton), Trame_preview1->trame_widget );
    gtk_box_pack_start( GTK_BOX(hboite), bouton, TRUE, FALSE, 0 );

    separator = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separator, FALSE, FALSE, 0 );

/********************************************** Proprietes DLS ********************************************/
    table = gtk_table_new(9, 4, TRUE);
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(boite), table, TRUE, TRUE, 0 );

    texte = gtk_label_new( _("Description") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );

    Entry_libelle = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_libelle), NBR_CARAC_LIBELLE_MOTIF );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_libelle, 1, 4, 0, 1 );

    texte = gtk_label_new( _("Type of response") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 2, 1, 2 );

    Combo_gestion = gtk_combo_box_new_text();
    gtk_table_attach_defaults( GTK_TABLE(table), Combo_gestion, 2, 4, 1, 2 );
    g_signal_connect( G_OBJECT( GTK_OPTION_MENU(Combo_gestion) ), "changed",
                      G_CALLBACK( Changer_gestion_motif ), NULL );

    texte = gtk_label_new( _("Control bit (I)") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 2, 3 );

    Spin_bit_ctrl = gtk_spin_button_new_with_range( -1, NBR_BIT_DLS, 1 );
    g_signal_connect( G_OBJECT(Spin_bit_ctrl), "value-changed",
                      G_CALLBACK(Afficher_mnemo_ctrl), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bit_ctrl, 1, 2, 2, 3 );

    Entry_bit_ctrl = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bit_ctrl), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bit_ctrl, 2, 4, 2, 3 );

    texte = gtk_label_new( _("Type of dialog box") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 2, 3, 4 );

    Option_dialog_cde = gtk_option_menu_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Option_dialog_cde, 2, 4, 3, 4 );
    menu = gtk_menu_new();
    for (cpt=0; cpt<ACTION_NBR_ACTION; cpt++)
     { gtk_menu_shell_append( GTK_MENU_SHELL(menu),
                              gtk_menu_item_new_with_label( Type_dialog_cde( cpt ) ) );
     }
    gtk_option_menu_set_menu( GTK_OPTION_MENU(Option_dialog_cde), menu );
    g_signal_connect( G_OBJECT( GTK_OPTION_MENU(Option_dialog_cde) ), "changed",
                      G_CALLBACK( Changer_dialog_cde ), NULL );

    texte = gtk_label_new( _("Action bit (M)") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 4, 5 );

    Spin_bit_clic = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    g_signal_connect( G_OBJECT(Spin_bit_clic), "value-changed",
                      G_CALLBACK(Afficher_mnemo_clic), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bit_clic, 1, 2, 4, 5 );

    Entry_bit_clic = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_bit_clic), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bit_clic, 2, 4, 4, 5 );

    texte = gtk_label_new( _("Tech_ID/Acronyme (M)") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 5, 6 );

    Entry_clic_tech_id = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_clic_tech_id), TRUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_clic_tech_id, 1, 2, 5, 6 );

    Entry_clic_acronyme = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_clic_acronyme), TRUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_clic_acronyme, 2, 4, 5, 6 );

    texte = gtk_label_new( _("Delai between frame") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 2, 6, 7 );

    adj = gtk_adjustment_new( RESOLUTION_TIMER, RESOLUTION_TIMER, 10*RESOLUTION_TIMER,
                              RESOLUTION_TIMER, 10*RESOLUTION_TIMER, 0 );
    Spin_rafraich = gtk_spin_button_new( (GtkAdjustment *)adj, 0.5, 0.5);
    gtk_signal_connect( GTK_OBJECT(Spin_rafraich), "changed", GTK_SIGNAL_FUNC(Changer_rafraich), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_rafraich, 2, 4, 6, 7 );

    texte = gtk_label_new( _("Default color") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 2, 7, 8 );

    Couleur_inactive = gnome_color_picker_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Couleur_inactive, 2, 4, 7, 8 );
    g_signal_connect( G_OBJECT(Couleur_inactive), "color_set",
                      G_CALLBACK(Changer_couleur), NULL );

    texte = gtk_label_new( _("Access Level") );                                  /* Combo du type d'acces */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 2, 8, 9 );
    Spin_access_level = gtk_spin_button_new_with_range( 0, 10, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_access_level, 2, 4, 8, 9 );

    ok_timer = TIMER_OFF;                                                       /* Timer = OFF par défaut */
    Tag_timer = gtk_timeout_add( RESOLUTION_TIMER, Timer_preview, NULL );      /* Enregistrement du timer */
    gtk_widget_show_all( Frame );                     /* On voit tout sauf la fenetre de plus haut niveau */
  }
#endif
/*--------------------------------------------------------------------------------------------------------*/
