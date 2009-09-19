/**********************************************************************************************************/
/* Client/atelier.c        Edition d'un synoptique de Watchdog v2.0                                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 29 mar 2009 09:54:12 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * atelier.c
 * This file is part of <Watchdog>
 *
 * Copyright (C) 2009 - sebastien
 *
 * <Watchdog> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <Watchdog> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <Watchdog>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #include <gnome.h>
 #include <sys/time.h>
 
 #include "Reseaux.h"
 #include "Config_cli.h"
 #include "trame.h"
 #include "motifs.h"

 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_effacer_motif ( void );
 static void Menu_ajouter_motif ( void );
 static void Menu_enregistrer_synoptique( struct TYPE_INFO_ATELIER *infos );
  
/**********************************************************************************************************/
/* Id_vers_trame_motif: Conversion d'un id motif en sa reference TRAME                                    */
/* Entrée: Un id motif                                                                                    */
/* sortie: un struct TRAME_ITEM_MOTIF                                                                     */
/**********************************************************************************************************/
 struct TRAME_ITEM_MOTIF *Id_vers_trame_motif ( struct TYPE_INFO_ATELIER *infos, gint id )
  { GList *liste;
    liste = infos->Trame_atelier->trame_items;
    while( liste )
     { if ( *(gint *)(liste->data) == TYPE_MOTIF &&
            ((struct TRAME_ITEM_MOTIF *)(liste->data))->motif->id == id ) break;
       else liste = liste->next;
     }
    if (!liste)
     { printf("Id_vers_trame_motif: item %d non trouvé\n", id );
       return(NULL);
     }
    return( (struct TRAME_ITEM_MOTIF *)(liste->data) );
  }
/**********************************************************************************************************/
/* Rechercher_infos_atelier_par_id_syn: Recherche une page synoptique par son numéro                      */
/* Entrée: Un numéro de synoptique                                                                        */
/* Sortie: Une référence sur les champs d'information de la page en question                              */
/**********************************************************************************************************/
 struct TYPE_INFO_ATELIER *Rechercher_infos_atelier_par_id_syn ( gint syn_id )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    GList *liste;
    liste = Liste_pages;
    infos = NULL;
    while( liste )
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if ( page->type == TYPE_PAGE_ATELIER )                        /* Est-ce bien une page d'atelier ?? */
        { infos = (struct TYPE_INFO_ATELIER *)page->infos;
          if (infos->syn.id == syn_id) break;                              /* Nous avons trouvé le syn !! */
        }
       liste = liste->next;                                                        /* On passe au suivant */
     }
    return(infos);
  }
/**********************************************************************************************************/
/* Detruire_page_atelier: Destruction d'une page atelier                                                  */
/* Entrée: la page en question                                                                            */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Detruire_page_atelier ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos;
printf("Detruire page atelier\n");
    infos = (struct TYPE_INFO_ATELIER *)page->infos;
printf("milieu Detruire page atelier\n");
    if ( Nbr_page_type( TYPE_PAGE_ATELIER ) == 1 )          /* S'il ne reste qu'1 page, c'est la derniere */
     { Detruire_fenetre_ajout_motif ();
       Detruire_fenetre_propriete_TOR ();
     }
printf("fin Detruire page atelier\n");
  }
/**********************************************************************************************************/
/* Menu_ajouter_message: Ajout d'un message                                                               */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_grille_magnetique ( struct TYPE_INFO_ATELIER *infos )
  { printf("Menu grille magnetique: %d\n", GTK_TOGGLE_BUTTON(infos->Check_grid)->active );
    gtk_widget_set_sensitive( infos->Spin_grid, GTK_TOGGLE_BUTTON(infos->Check_grid)->active );
  }
/**********************************************************************************************************/
/* Menu_ajouter_message: Ajout d'un message                                                               */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_motif ( void )
  { Choisir_motif_a_ajouter (); }
/**********************************************************************************************************/
/* Menu_ajouter_message: Ajout d'un message                                                               */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_commentaire ( void )
  { Creer_fenetre_ajout_commentaire (); }
/**********************************************************************************************************/
/* Menu_effacer_message: Retrait des messages selectionnés                                                */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_effacer_motif ( void )
  { Effacer_selection(); }
/**********************************************************************************************************/
/* Menu_enregistrer_synoptique: Envoi des données au serveur                                              */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_enregistrer_synoptique ( struct TYPE_INFO_ATELIER *infos )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct CMD_EDIT_MOTIF edit_motif;
    struct MOTIF *motif;
    struct TRAME_ITEM_COMMENT *trame_comment;
    struct CMD_EDIT_COMMENT edit_comment;
    struct COMMENTAIRE *comment;
    struct TRAME_ITEM_PASS *trame_pass;
    struct CMD_EDIT_PASSERELLE edit_pass;
    struct PASSERELLE *pass;
    struct TRAME_ITEM_CAPTEUR *trame_capteur;
    struct CMD_EDIT_CAPTEUR edit_capteur;
    struct CAPTEUR *capteur;
    struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    struct CMD_TYPE_CAMERA_SUP *camera_sup, edit_camera_sup;

    GList *objet;
    if ( !(infos && infos->Trame_atelier && infos->Trame_atelier->trame_items) )
     { printf("Erreur parametre Menu_enregistrer_synoptique\n"); return; }

    objet = infos->Trame_atelier->trame_items;
    while (objet)
     { printf("Menu_enregistrer_synoptique type=%d\n", *((gint *)objet->data) );
       switch( *((gint *)objet->data) )
        { case TYPE_MOTIF:
               trame_motif = (struct TRAME_ITEM_MOTIF *)objet->data;
               motif = trame_motif->motif;
               edit_motif.id = motif->id;                                   /* Correspond a l'id du motif */
               edit_motif.icone_id = motif->icone_id;                       /* Correspond au fichier .gif */
               memcpy( edit_motif.libelle, motif->libelle, sizeof(edit_motif.libelle) );
               edit_motif.gid = motif->gid;                      /* Nom du groupe d'appartenance du motif */
               edit_motif.bit_controle = motif->bit_controle;                               /* Ixxx, Cxxx */
               edit_motif.bit_clic = motif->bit_clic;    /* Bit à activer quand clic bouton gauche souris */
               edit_motif.bit_clic2 = motif->bit_clic2;  /* Bit à activer quand clic bouton gauche souris */
               edit_motif.position_x = motif->position_x;                    /* en abscisses et ordonnées */
               edit_motif.position_y = motif->position_y;                    /* en abscisses et ordonnées */
               edit_motif.largeur = motif->largeur;                /* Taille de l'image sur le synoptique */
               edit_motif.hauteur = motif->hauteur;
               edit_motif.angle = motif->angle;
               edit_motif.type_dialog = motif->type_dialog;  /* Type de dialogue pour le clic de commande */
               edit_motif.rouge0 = motif->rouge0;
               edit_motif.vert0 = motif->vert0;
               edit_motif.bleu0 = motif->bleu0;
               edit_motif.type_gestion = motif->type_gestion;              /* Statique/dynamique/cyclique */
               printf("Update motif id=%d, libelle=%s, posx=%d, posy=%d\n", edit_motif.id, edit_motif.libelle, edit_motif.position_x, edit_motif.position_y );
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_EDIT_MOTIF,
                              (gchar *)&edit_motif, sizeof(struct CMD_EDIT_MOTIF) );
               break;

          case TYPE_COMMENTAIRE:
               trame_comment = (struct TRAME_ITEM_COMMENT *)objet->data;
               comment = trame_comment->comment;
               memcpy( edit_comment.libelle, comment->libelle, sizeof(edit_comment.libelle) );
               memcpy( edit_comment.font, comment->font, sizeof(edit_comment.font) );
               edit_comment.id         = comment->id;                     /* Correspond a l'id du comment */
               edit_comment.rouge      = comment->rouge;
               edit_comment.vert       = comment->vert;
               edit_comment.bleu       = comment->bleu;
               edit_comment.position_x = comment->position_x;                /* en abscisses et ordonnées */
               edit_comment.position_y = comment->position_y;                /* en abscisses et ordonnées */
               edit_comment.angle      = comment->angle;
               printf("Update comment id=%d, posx=%d, posy=%d\n", edit_comment.id, edit_comment.position_x, edit_comment.position_y );
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_EDIT_COMMENT,
                              (gchar *)&edit_comment, sizeof(struct CMD_EDIT_COMMENT) );
               break;

          case TYPE_PASSERELLE:
               trame_pass = (struct TRAME_ITEM_PASS *)objet->data;
               pass = trame_pass->pass;
               memcpy( edit_pass.libelle, pass->libelle, sizeof(edit_pass.libelle) );
               edit_pass.id         = pass->id;                           /* Correspond a l'id du comment */
               edit_pass.bit_controle = pass->bit_controle;
               edit_pass.bit_controle_1 = pass->bit_controle_1;
               edit_pass.bit_controle_2 = pass->bit_controle_2;
               edit_pass.position_x = pass->position_x;                      /* en abscisses et ordonnées */
               edit_pass.position_y = pass->position_y;                      /* en abscisses et ordonnées */
               edit_pass.angle      = pass->angle;
               printf("Update pass id=%d, posx=%d, posy=%d\n", edit_pass.id, edit_pass.position_x, edit_pass.position_y );
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_EDIT_PASS,
                              (gchar *)&edit_pass, sizeof(struct CMD_EDIT_PASSERELLE) );
               break;
          case TYPE_CAPTEUR:
               trame_capteur = (struct TRAME_ITEM_CAPTEUR *)objet->data;
               capteur = trame_capteur->capteur;
               memcpy( edit_capteur.libelle, capteur->libelle, sizeof(edit_capteur.libelle) );
               edit_capteur.id           = capteur->id;                   /* Correspond a l'id du comment */
               edit_capteur.type         = capteur->type;
               edit_capteur.bit_controle = capteur->bit_controle;
               edit_capteur.position_x   = capteur->position_x;              /* en abscisses et ordonnées */
               edit_capteur.position_y   = capteur->position_y;              /* en abscisses et ordonnées */
               edit_capteur.angle        = capteur->angle;
               printf("Update capteur id=%d, bitctrl=%d%d, posx=%d, posy=%d\n", edit_capteur.id, edit_capteur.type, edit_capteur.bit_controle, edit_capteur.position_x, edit_capteur.position_y );
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_EDIT_CAPTEUR,
                              (gchar *)&edit_capteur, sizeof(struct CMD_EDIT_CAPTEUR) );
               break;
          case TYPE_CAMERA_SUP:
               trame_camera_sup = (struct TRAME_ITEM_CAMERA_SUP *)objet->data;
               camera_sup = trame_camera_sup->camera_sup;
               edit_camera_sup.id           = camera_sup->id;           /* ID de la camera_sup a modifier */
               edit_camera_sup.position_x   = camera_sup->position_x;        /* en abscisses et ordonnées */
               edit_camera_sup.position_y   = camera_sup->position_y;        /* en abscisses et ordonnées */
               edit_camera_sup.angle        = camera_sup->angle;
               edit_camera_sup.largeur      = camera_sup->largeur; /* Taille de l'image sur le synoptique */
               edit_camera_sup.hauteur      = camera_sup->hauteur;

               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_EDIT_CAMERA_SUP,
                              (gchar *)&edit_camera_sup, sizeof(struct CMD_TYPE_CAMERA_SUP) );
               break;

          default: printf("Enregistrer_synoptique: type inconnu\n" );
        }
       objet=objet->next;
     }
  }
/**********************************************************************************************************/
/* Changer_option_zoom: Change le niveau de zoom du canvas                                                */
/* Entrée: la page en question                                                                            */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Changer_option_zoom (GtkRange *range, struct TYPE_INFO_ATELIER *infos )
  { GtkAdjustment *adj;
    g_object_get( infos->Option_zoom, "adjustment", &adj, NULL );
    goo_canvas_set_scale ( GOO_CANVAS(infos->Trame_atelier->trame_widget), gtk_adjustment_get_value(adj) );
  }
/**********************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_atelier( gint syn_id, gchar *libelle_syn )
  { GtkWidget *separateur, *frame, *bouton, *boite, *hboite, *table;
    GtkWidget *vboite, *capteur, *spin, *boite1, *scroll;
    GtkAdjustment *adj;
    struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    gchar libelle[50];

    page = (struct PAGE_NOTEBOOK *)g_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->type  = TYPE_PAGE_ATELIER;
    Liste_pages = g_list_append( Liste_pages, page );

    page->infos = (struct TYPE_INFO_ATELIER *)g_malloc0( sizeof(struct TYPE_INFO_ATELIER) );
    if (!page->infos) { g_free(page); return; }
    infos = (struct TYPE_INFO_ATELIER *)page->infos;
    infos->syn.id = syn_id;
    g_snprintf( infos->syn.libelle, sizeof(infos->syn.libelle), "%s", libelle_syn );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
/***************************************** L'atelier ******************************************************/
    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), vboite, TRUE, TRUE, 0 );
    table = gtk_table_new( 2, 5, TRUE );                                       /* Barre de controle trame */
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, FALSE, FALSE, 0 );
    
    capteur = gtk_label_new( _("Position X/Y") );                                 /* Affichage des abcisses */
    gtk_table_attach_defaults( GTK_TABLE(table), capteur, 0, 1, 0, 1 );
    infos->Entry_posxy = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(infos->Entry_posxy), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry_posxy, 1, 3, 0, 1 );

    capteur = gtk_label_new( _("Rotate") );
    gtk_table_attach( GTK_TABLE(table), capteur, 3, 4, 0, 1, 0, 0, 0, 0 );
    infos->Adj_angle = (GtkAdjustment *)gtk_adjustment_new( 0.0, -180.0, +180.0, 0.5, 10.0, 0.0 );
    spin = gtk_spin_button_new(infos->Adj_angle, 1.0, 1);
    gtk_table_attach_defaults( GTK_TABLE(table), spin, 4, 5, 0, 1 );
    g_signal_connect_swapped( G_OBJECT(infos->Adj_angle), "value_changed",
                              G_CALLBACK(Rotationner_selection), infos );

    capteur = gtk_label_new( _("Description") );
    gtk_table_attach_defaults( GTK_TABLE(table), capteur, 0, 1, 1, 2 );
    infos->Entry_libelle = gtk_entry_new();
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry_libelle, 1, 5, 1, 2 );
    gtk_entry_set_editable ( GTK_ENTRY(infos->Entry_libelle), FALSE );
    
/**************************************** Trame proprement dite *******************************************/

    boite1 = gtk_hbox_new( TRUE, 6 );                                         /* Barre de controle trame */
    gtk_box_pack_start( GTK_BOX(vboite), boite1, TRUE, TRUE, 0 );
    boite = gtk_vbox_new( TRUE, 6 );                                          /* Barre de controle trame */
    gtk_box_pack_start( GTK_BOX(boite1), boite, TRUE, TRUE, 0 );

    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start( GTK_BOX(boite), scroll, TRUE, TRUE, 0 );

    infos->Trame_atelier = Trame_creer_trame( TAILLE_SYNOPTIQUE_X, TAILLE_SYNOPTIQUE_Y, "darkgray", 20 );
    gtk_container_add( GTK_CONTAINER(scroll), infos->Trame_atelier->trame_widget );

  /*  g_signal_connect_swapped( G_OBJECT(infos->Trame_atelier->fond), "event",
                              G_CALLBACK(Clic_sur_fond), infos );*/
/**************************************** Boutons de controle *********************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );
    
    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_SAVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_enregistrer_synoptique), infos );

/*********************************************** Zoom *****************************************************/
    frame = gtk_frame_new ( _("Zoom/Grille") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    hboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );

    infos->Option_zoom = gtk_hscale_new_with_range ( 0.2, 5.0, 0.1 );
    gtk_box_pack_start( GTK_BOX(hboite), infos->Option_zoom, FALSE, FALSE, 0 );
    g_object_get( infos->Option_zoom, "adjustment", &adj, NULL );
    gtk_adjustment_set_value( adj, 1.0 );
    g_signal_connect( G_OBJECT( infos->Option_zoom ), "value-changed",
                      G_CALLBACK( Changer_option_zoom ), infos );

    infos->Check_grid = gtk_check_button_new_with_label( _("Grille magnetique") );
    gtk_box_pack_start( GTK_BOX(hboite), infos->Check_grid, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(infos->Check_grid), "toggled",
                              G_CALLBACK(Menu_grille_magnetique), infos );

    infos->Spin_grid = gtk_spin_button_new_with_range( 1.0, 20.0, 5.0 );
    gtk_box_pack_start( GTK_BOX(hboite), infos->Spin_grid, FALSE, FALSE, 0 );
    Menu_grille_magnetique( infos );

/******************************************* Ajout de motifs **********************************************/
    frame = gtk_frame_new( _("Motifs") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(vboite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_motif), infos );

    bouton = gtk_button_new_from_stock( GTK_STOCK_DELETE );
    gtk_box_pack_start( GTK_BOX(vboite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_effacer_motif), infos );

/******************************************* Ajout de comments ********************************************/
    frame = gtk_frame_new( _("Comments") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(vboite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_commentaire), infos );
/******************************************* Ajout de passerelles *****************************************/
    frame = gtk_frame_new( _("Gateways") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(vboite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Creer_fenetre_ajout_passerelle), infos );
/******************************************* Ajout de palettes ********************************************/
    frame = gtk_frame_new( _("Palettes") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );

    bouton = gtk_button_new_with_label( _("Manage") );
    gtk_box_pack_start( GTK_BOX(vboite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Creer_fenetre_ajout_palette), infos );
/******************************************* Ajout de capteurs **********************************************/
    frame = gtk_frame_new( _("Capteur") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(vboite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_editer_capteur), NULL );

/******************************************** Camera de supervision ***************************************/
    frame = gtk_frame_new( _("Camera") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(vboite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_camera_sup), NULL );

/************************************************* fin ****************************************************/
    gtk_widget_show_all( page->child );
    g_snprintf( libelle, sizeof(libelle), "Atelier/%s", libelle_syn );

    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), page->child, gtk_label_new ( libelle ) );
    Creer_fenetre_propriete_TOR ( infos );
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_motif_atelier( struct CMD_SHOW_MOTIF *rezo_motif )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct TYPE_INFO_ATELIER *infos;
    struct MOTIF *motif;
        
    infos = Rechercher_infos_atelier_par_id_syn ( rezo_motif->syn_id );
    if (!infos) return;
    motif = (struct MOTIF *)g_malloc0( sizeof(struct MOTIF) );
    if (!motif)
     { Info( Config_cli.log, DEBUG_MEM, "Afficher_motif_atelier: not enought memory" );
       return;
     }

    motif->id = rezo_motif->id;                                     /* Numero du motif dans la DBWatchdog */
    motif->icone_id = rezo_motif->icone_id;                                 /* Correspond au fichier .gif */
    memcpy( motif->libelle, rezo_motif->libelle, sizeof(motif->libelle) );
    motif->gid = rezo_motif->gid;                                /* Nom du groupe d'appartenance du motif */
    motif->bit_controle = rezo_motif->bit_controle;                                         /* Ixxx, Cxxx */
    motif->bit_clic = rezo_motif->bit_clic;   /* Bit à activer quand on clic avec le bouton gauche souris */
    motif->bit_clic2 = rezo_motif->bit_clic2; /* Bit à activer quand on clic avec le bouton gauche souris */
    motif->position_x = rezo_motif->position_x;                              /* en abscisses et ordonnées */
    motif->position_y = rezo_motif->position_y;
    motif->largeur = rezo_motif->largeur;                          /* Taille de l'image sur le synoptique */
    motif->hauteur = rezo_motif->hauteur;
    motif->angle = rezo_motif->angle;
    motif->type_dialog = rezo_motif->type_dialog;/* Type de la boite de dialogue pour le clic de commande */
    motif->rouge0 = rezo_motif->rouge0;
    motif->vert0 = rezo_motif->vert0;
    motif->bleu0 = rezo_motif->bleu0;
    motif->type_gestion = rezo_motif->type_gestion;                        /* Statique/dynamique/cyclique */

    trame_motif = Trame_ajout_motif ( TRUE, infos->Trame_atelier, motif );
    trame_motif->groupe_dpl = Nouveau_groupe();                   /* Numéro de groupe pour le deplacement */

    g_signal_connect( G_OBJECT(trame_motif->item), "button-press-event",
                      G_CALLBACK(Clic_sur_motif), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->item), "button-release-event",
                      G_CALLBACK(Clic_sur_motif), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->item), "enter-notify-event",
                      G_CALLBACK(Clic_sur_motif), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->item), "leave-notify-event",
                      G_CALLBACK(Clic_sur_motif), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->item), "motion-notify-event",
                      G_CALLBACK(Clic_sur_motif), trame_motif );

    g_signal_connect( G_OBJECT(trame_motif->select_hg), "button-press-event",
                      G_CALLBACK(Agrandir_hg), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->select_hg), "button-release-event",
                      G_CALLBACK(Agrandir_hg), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->select_hg), "motion-notify-event",
                      G_CALLBACK(Agrandir_hg), trame_motif );

    g_signal_connect( G_OBJECT(trame_motif->select_hd), "button-press-event",
                      G_CALLBACK(Agrandir_hd), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->select_hd), "button-release-event",
                      G_CALLBACK(Agrandir_hd), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->select_hd), "motion-notify-event",
                      G_CALLBACK(Agrandir_hd), trame_motif );

    g_signal_connect( G_OBJECT(trame_motif->select_bg), "button-press-event",
                      G_CALLBACK(Agrandir_bg), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->select_bg), "button-release-event",
                      G_CALLBACK(Agrandir_bg), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->select_bg), "motion-notify-event",
                      G_CALLBACK(Agrandir_bg), trame_motif );

    g_signal_connect( G_OBJECT(trame_motif->select_bd), "button-press-event",
                      G_CALLBACK(Agrandir_bd), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->select_bd), "button-release-event",
                      G_CALLBACK(Agrandir_bd), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->select_bd), "motion-notify-event",
                      G_CALLBACK(Agrandir_bd), trame_motif );
  }
/**********************************************************************************************************/
/* Cacher_un_message: Enleve un message de la liste des messages                                          */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_motif_atelier( struct CMD_ID_MOTIF *motif )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct TYPE_INFO_ATELIER *infos;
        
    infos = Rechercher_infos_atelier_par_id_syn ( motif->syn_id );
    trame_motif = Id_vers_trame_motif( infos, motif->id );
    if (!trame_motif) return;
    Deselectionner( infos, (struct TRAME_ITEM *)trame_motif );  /* Au cas ou il aurait été selectionné... */
    Trame_del_item( trame_motif );
    infos->Trame_atelier->trame_items = g_list_remove( infos->Trame_atelier->trame_items, trame_motif );
    g_free(trame_motif);
    printf("Proto_cacher_un_motif_atelier fin..\n");
  }
/*--------------------------------------------------------------------------------------------------------*/
