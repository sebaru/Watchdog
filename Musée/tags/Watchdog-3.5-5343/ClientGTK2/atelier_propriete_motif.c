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

 #include <gnome.h>
 #include <string.h>
 #include <stdlib.h>

 #include "Reseaux.h"
 #include "trame.h"

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

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
/******************************************* Définitions des prototypes programme *********************************************/
 #include "protocli.h"

 extern GtkWidget *F_trame;                                           /* C'est bien le widget referencant la trame synoptique */

 static GtkWidget *F_propriete;                                                          /* Pour acceder la fenetre graphique */
 static GtkWidget *Combo_gestion;                                                                 /* Type de gestion du motif */
 static GtkWidget *Option_dialog_cde;                                          /* Type de boite de dialogue clic gauche motif */
 static GtkWidget *Spin_access_level;                                                       /* groupe d'appartenance du motif */
 static GtkWidget *Spin_rafraich;                                        /* Frequence de refraichissement d'un motif cyclique */
 static GtkWidget *Couleur_inactive;                                                           /* Parametres visuels du motif */
 static GtkWidget *Entry_libelle;                                                      /* Libelle du motif en cours d'edition */
 static GtkWidget *Entry_clic_tech_id;                                            /* tech_id dls a positionner si clic gauche */
 static GtkWidget *Entry_clic_acronyme;                                          /* acronyme dls a positionner si clic gauche */
 static GtkWidget *Entry_tech_id;                                                 /* tech_id dls a positionner si clic gauche */
 static GtkWidget *Entry_acronyme;                                               /* acronyme dls a positionner si clic gauche */
 static struct TRAME *Trame_preview0;                                                 /* Previsualisation du motif par défaut */
 static struct TRAME *Trame_preview1;                                                      /* Previsualisation du motif actif */
 static struct TRAME_ITEM_MOTIF *Trame_motif;                                                  /* Motif en cours de selection */
 static struct TRAME_ITEM_MOTIF *Trame_motif_p0;                                               /* Motif en cours de selection */
 static struct TRAME_ITEM_MOTIF *Trame_motif_p1;                                               /* Motif en cours de selection */
 static struct CMD_TYPE_MOTIF Motif_preview0;
 static struct CMD_TYPE_MOTIF Motif_preview1;
 static gint Tag_timer, ok_timer;                                                 /* Gestion des motifs cycliques/indicateurs */
 static GList *Liste_index_groupe;                     /* Pour correspondance index de l'option menu/Id du groupe en question */
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
                                            "lime" );                    /* frame numero ++ */
     }
    else if (Trame_motif->motif->type_gestion == TYPE_CYCLIQUE_0N ||
             Trame_motif->motif->type_gestion == TYPE_PROGRESSIF
            )
     { if (top >= Trame_motif->motif->rafraich)
        { Trame_choisir_frame( Trame_motif_p1, Trame_motif_p1->num_image+1,
                                               "lime" );                 /* frame numero ++ */
          top = 0;                                                            /* Raz pour prochaine frame */
        }
     }
    else if (Trame_motif->motif->type_gestion == TYPE_CYCLIQUE_1N)
     { if (top >= Trame_motif->motif->rafraich)
        { if (Trame_motif_p1->num_image == Trame_motif_p1->nbr_images-1)
           { Trame_choisir_frame( Trame_motif_p1, 1,
                                  "lime" );                              /* frame numero ++ */
           }
          else
           { Trame_choisir_frame( Trame_motif_p1, Trame_motif_p1->num_image+1,
                                  "lime" );                              /* frame numero ++ */
           }
          top = 0;                                                            /* Raz pour prochaine frame */
        }
     }
    else if (Trame_motif->motif->type_gestion == TYPE_CYCLIQUE_2N)
     { if (top >= Trame_motif->motif->rafraich)
        { if (Trame_motif_p1->num_image == Trame_motif_p1->nbr_images-1)
           { Trame_choisir_frame( Trame_motif_p1, 2,
                                  "lime" );                              /* frame numero ++ */
           }
          else
           { Trame_choisir_frame( Trame_motif_p1, Trame_motif_p1->num_image+1,
                                  "lime" );                              /* frame numero ++ */
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
    printf("Rafraichir_sensibilite1\n" );

    switch( Trame_motif->motif->type_gestion )
     { case TYPE_FOND:
       case TYPE_INERTE  :   Trame_choisir_frame( Trame_motif_p1, 0, "black" ); /* frame 0 */
                             ok_timer = TIMER_OFF;
                             break;
       case TYPE_STATIQUE:   Trame_choisir_frame( Trame_motif_p1, 0, "lime" );   /* frame 0 */
                             ok_timer = TIMER_OFF;
                             break;
       case TYPE_BOUTON    :
       case TYPE_PROGRESSIF:
       case TYPE_DYNAMIQUE : Trame_choisir_frame( Trame_motif_p1, 1, "lime" );   /* frame 1 */
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
     { case ACTION_SANS      : gtk_widget_set_sensitive( Entry_clic_tech_id, FALSE );
                               gtk_widget_set_sensitive( Entry_clic_acronyme, FALSE );
                               gtk_widget_set_sensitive( Spin_access_level, FALSE );
                               break;
       case ACTION_IMMEDIATE : gtk_widget_set_sensitive( Entry_clic_tech_id, TRUE );
                               gtk_widget_set_sensitive( Entry_clic_acronyme, TRUE );
                               gtk_widget_set_sensitive( Spin_access_level, TRUE );
                               break;
       case ACTION_CONFIRME  : gtk_widget_set_sensitive( Entry_clic_tech_id, TRUE );
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
#ifdef bouh
/**********************************************************************************************************/
/* Changer_couleur: Changement de la couleur du motif                                                     */
/* Entrée: widget, data = 0 pour un chgmt via propriete DLS, 1 pour chgmt via Couleur par Def             */
/* Sortie: la base de données est mise à jour                                                             */
/**********************************************************************************************************/
 static void Changer_couleur( GtkWidget *widget, gpointer data )
  { guint8 r, v, b;
printf("Changer_couleur %p\n", data);

    if (widget == Couleur_inactive)                                     /* Changement de couleur inactive */
     { gnome_color_picker_get_i8( GNOME_COLOR_PICKER(Couleur_inactive), &r, &v, &b, NULL );
       Trame_motif->motif->rouge0 = r;
       Trame_motif->motif->vert0  = v;
       Trame_motif->motif->bleu0  = b;
       /*Trame_peindre_motif( Trame_motif, r, v, b );
       Trame_peindre_motif( Trame_motif_p0, r, v, b );*/
     }
    else                                                                     /* Rafraichissement direct ? */
     { gdouble coul[8];
       gtk_color_selection_get_color( GTK_COLOR_SELECTION(data), &coul[0] );
       Trame_motif->motif->rouge0 = (guchar)(coul[0]*255.0);
       Trame_motif->motif->vert0  = (guchar)(coul[1]*255.0);
       Trame_motif->motif->bleu0  = (guchar)(coul[2]*255.0);
       /*Trame_peindre_motif( Trame_motif, Trame_motif->motif->rouge0,
                                         Trame_motif->motif->vert0,
                                         Trame_motif->motif->bleu0 );*/
     }
  }
/**********************************************************************************************************/
/* Changer_couleur_motif_directe: Changement de la couleur du motif en direct live                        */
/* Entrée: widget, data =0 pour inactive, 1 pour active                                                   */
/* Sortie: la base de données est mise à jour                                                             */
/**********************************************************************************************************/
 void Changer_couleur_motif_directe( struct TRAME_ITEM_MOTIF *trame_motif )
  { GtkWidget *fen, *choix;
    gdouble coul[8];

    Trame_motif = trame_motif;              /* On sauvegarde la reference pour peindre le motif plus tard */
    fen = gtk_dialog_new_with_buttons( _("Edit the default color"),
                                       GTK_WINDOW(F_client),
                                       GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_STOCK_OK, GTK_RESPONSE_OK,
                                       NULL);
    g_signal_connect_swapped( fen, "response",
                              G_CALLBACK(gtk_widget_destroy), fen );

    choix = gtk_color_selection_new();          /* Creation de la zone de saisie de la couleur par defaut */
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(fen)->vbox ), choix, TRUE, TRUE, 0 );

    coul[0] = trame_motif->motif->rouge0/255.0;
    coul[1] = trame_motif->motif->vert0/255.0;
    coul[2] = trame_motif->motif->bleu0/255.0;
    coul[3] = 0.0; coul[4] = 0.0;
    coul[5] = 0.0; coul[6] = 0.0;
    coul[7] = 0.0;

    gtk_color_selection_set_color( GTK_COLOR_SELECTION(choix), &coul[0] );
/*    g_signal_connect( G_OBJECT( choix ), "color_changed",
                      G_CALLBACK( Changer_couleur ), choix );*/
    gtk_widget_show_all( fen );
  }
#endif
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

/*    gnome_color_picker_set_i8 ( GNOME_COLOR_PICKER(Couleur_inactive),
                                motif->rouge0, motif->vert0, motif->bleu0, 0 );*/

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

    gtk_entry_set_text( GTK_ENTRY(Entry_tech_id), motif->tech_id );
    gtk_entry_set_text( GTK_ENTRY(Entry_acronyme), motif->acronyme );
    gtk_entry_set_text( GTK_ENTRY(Entry_clic_tech_id), motif->clic_tech_id );
    gtk_entry_set_text( GTK_ENTRY(Entry_clic_acronyme), motif->clic_acronyme );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_rafraich), motif->rafraich );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_access_level), motif->access_level );

    g_signal_handlers_block_by_func( G_OBJECT( GTK_COMBO_BOX(Combo_gestion) ),
                                     G_CALLBACK( Changer_gestion_motif ), NULL );
    for (i=0; i<NBR_TYPE_GESTION_MOTIF; i++) gtk_combo_box_remove_text( GTK_COMBO_BOX(Combo_gestion), 0 );
    printf("Nombre de images motif! %d....\n", trame_motif->nbr_images );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_INERTE   ) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_FOND     ) );
    gtk_combo_box_append_text( GTK_COMBO_BOX(Combo_gestion), Type_gestion_motif( TYPE_STATIQUE ) );

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

    Rafraichir_sensibilite();  /* test 18/01/2006 */
    printf("rafraichir_propriete Oktimer = %d\n", ok_timer );
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

    g_snprintf( Trame_motif->motif->tech_id, sizeof(Trame_motif->motif->tech_id),
               "%s", gtk_entry_get_text( GTK_ENTRY(Entry_tech_id) ) );
    g_strcanon( Trame_motif->motif->tech_id, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", '_' );
    g_snprintf( Trame_motif->motif->acronyme, sizeof(Trame_motif->motif->acronyme),
               "%s", gtk_entry_get_text( GTK_ENTRY(Entry_acronyme) ) );
    g_strcanon( Trame_motif->motif->acronyme, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", '_' );

    g_snprintf( Trame_motif->motif->clic_tech_id, sizeof(Trame_motif->motif->clic_tech_id),
               "%s", gtk_entry_get_text( GTK_ENTRY(Entry_clic_tech_id) ) );
    g_strcanon( Trame_motif->motif->clic_tech_id, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", '_' );
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

    Entry_tech_id = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_tech_id), TRUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_tech_id, 1, 2, 2, 3 );

    Entry_acronyme = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_acronyme), TRUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_acronyme, 2, 4, 2, 3 );

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

    texte = gtk_label_new( _("Clic Tech_ID/Acronyme (M)") );
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
/*    g_signal_connect( G_OBJECT(Couleur_inactive), "color_set",
                      G_CALLBACK(Changer_couleur), NULL );*/

    texte = gtk_label_new( _("Access Level") );                                  /* Combo du type d'acces */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 2, 8, 9 );
    Spin_access_level = gtk_spin_button_new_with_range( 0, 10, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_access_level, 2, 4, 8, 9 );

    ok_timer = TIMER_OFF;                                                       /* Timer = OFF par défaut */
    Tag_timer = gtk_timeout_add( RESOLUTION_TIMER, Timer_preview, NULL );      /* Enregistrement du timer */
    gtk_widget_show_all( Frame );                     /* On voit tout sauf la fenetre de plus haut niveau */
  }
/*--------------------------------------------------------------------------------------------------------*/
