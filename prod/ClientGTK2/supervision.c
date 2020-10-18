/******************************************************************************************************************************/
/* Client/supervision.c        Affichage du synoptique de supervision                                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          dim 29 mar 2009 09:54:22 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * supervision.c
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
 #include <sys/time.h>

 #include "Reseaux.h"
 #include "Config_cli.h"
 #include "trame.h"

 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

/********************************************* Définitions des prototypes programme *******************************************/
 #include "protocli.h"

 enum
  {  COLONNE_HORLOGE_ID_MNEMO,
     COLONNE_HORLOGE_LIBELLE,
     NBR_COLONNE_HORLOGE
  };

/******************************************************************************************************************************/
/* Rechercher_infos_supervision_par_id_syn: Recherche une page synoptique par son numéro                                      */
/* Entrée: Un numéro de synoptique                                                                                            */
/* Sortie: Une référence sur les champs d'information de la page en question                                                  */
/******************************************************************************************************************************/
 struct TYPE_INFO_SUPERVISION *Rechercher_infos_supervision_par_id_syn ( gint syn_id )
  { struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page;
    GList *liste;
    liste = Liste_pages;
    infos = NULL;
    while( liste )
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if ( page->type == TYPE_PAGE_SUPERVISION )                /* Est-ce bien une page d'supervision ?? */
        { infos = (struct TYPE_INFO_SUPERVISION *)page->infos;
          if (infos->syn.id == syn_id) break;                              /* Nous avons trouvé le syn !! */
        }
       liste = liste->next;                                                        /* On passe au suivant */
     }
    return(infos);
  }
/******************************************************************************************************************************/
/* Detruire_page_supervision: L'utilisateur veut fermer la page de supervision                                                */
/* Entrée: la page en question                                                                                                */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Detruire_page_supervision( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_SUPERVISION *infos;
    infos = (struct TYPE_INFO_SUPERVISION *)page->infos;
    /*g_timeout_remove( infos->timer_id );*/
    Trame_detruire_trame( infos->Trame );
  }
/******************************************************************************************************************************/
/* Detruire_page_supervision: L'utilisateur veut fermer la page de supervision                                                */
/* Entrée: la page en question                                                                                                */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Changer_option_zoom (GtkRange *range, struct TYPE_INFO_SUPERVISION *infos )
  { GtkAdjustment *adj;
    g_object_get( infos->Option_zoom, "adjustment", &adj, NULL );
    goo_canvas_set_scale ( GOO_CANVAS(infos->Trame->trame_widget), gtk_adjustment_get_value(adj) );
  }
/******************************************************************************************************************************/
/* draw_page: Dessine une page pour l'envoyer sur l'imprimante                                                                */
/* Entrée: néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void draw_page (GtkPrintOperation *operation,
                        GtkPrintContext   *context,
                        gint               page_nr,
                        struct TYPE_INFO_SUPERVISION *infos)
  { cairo_t *cr;
    cr = gtk_print_context_get_cairo_context (context);
    cairo_scale ( cr, 700.0/TAILLE_SYNOPTIQUE_X, 700.0/TAILLE_SYNOPTIQUE_X );
    goo_canvas_render ( GOO_CANVAS( infos->Trame->trame_widget ), cr, NULL, 1.0 );
  }
/******************************************************************************************************************************/
/* Menu_exporter_message: Exportation de la base dans un fichier texte                                                        */
/* Entrée: néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Menu_exporter_synoptique( struct TYPE_INFO_SUPERVISION *infos )
  { GtkPrintOperation *print;
    GError *error;

    print = New_print_job ( "Print Synoptique" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (draw_page), infos );
    gtk_print_operation_set_n_pages ( print, 1 );

    gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                             GTK_WINDOW(F_client), &error);
  }
/******************************************************************************************************************************/
/* Menu_acquitter_synoptique: Envoi une commande d'acquit du synoptique en cours de visu                                      */
/* Entrée: La page d'information synoptique                                                                                   */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Menu_acquitter_synoptique( struct TYPE_INFO_SUPERVISION *infos )
  { Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_ACQ_SYN, (gchar *)&infos->syn.id, sizeof(gint) ); }
/******************************************************************************************************************************/
/* Menu_acquitter_synoptique: Envoi une commande d'acquit du synoptique en cours de visu                                      */
/* Entrée: La page d'information synoptique                                                                                   */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Menu_get_horloge_synoptique( struct TYPE_INFO_SUPERVISION *infos )
  { gchar chaine[80];
    printf("Get Horloge from %s\n", infos->syn.page );
    g_snprintf(chaine, sizeof(chaine), "horloges/list/%s", infos->syn.page );
    Firefox_exec ( chaine );
  }
/******************************************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                                        */
/* Entrée: Le libelle a afficher dans le notebook et l'ID du synoptique                                                       */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Creer_page_supervision ( struct CMD_TYPE_SYNOPTIQUE *syn )
  { GtkWidget *bouton, *boite, *hboite, *scroll, *frame, *label;
    GtkAdjustment *adj;
    struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page;
    static gint init_timer;
    GdkColor color;

    printf("Creation page synoptique %d\n", syn->id );
    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;

    page->infos = (struct TYPE_INFO_SUPERVISION *)g_try_malloc0( sizeof(struct TYPE_INFO_SUPERVISION) );
    infos = (struct TYPE_INFO_SUPERVISION *)page->infos;
    if (!page->infos) { g_free(page); return; }

    page->type   = TYPE_PAGE_SUPERVISION;
    Liste_pages  = g_list_append( Liste_pages, page );
    memcpy(&infos->syn, syn, sizeof(struct CMD_TYPE_SYNOPTIQUE) );

    if (!init_timer) { g_timeout_add( 500, Timer, NULL ); init_timer = 1; }

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
/**************************************************** Trame proprement dite ***************************************************/

    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    infos->Trame = Trame_creer_trame( TAILLE_SYNOPTIQUE_X, TAILLE_SYNOPTIQUE_Y, "darkgray", 0 );
    gtk_container_add( GTK_CONTAINER(scroll), infos->Trame->trame_widget );

/************************************************** Boutons de controle *******************************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );

    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_synoptique), infos );

/********************************************************** Zoom **************************************************************/
    frame = gtk_frame_new ( _("Zoom") );
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

/************************************************************* Palettes *******************************************************/
    frame = gtk_frame_new( _("Palette") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    infos->Box_palette = gtk_vbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(infos->Box_palette), 6 );
    gtk_container_add( GTK_CONTAINER(frame), infos->Box_palette );

/******************************************************* Acquitter ************************************************************/
    infos->bouton_acq = gtk_button_new_with_label( "Acquitter" );
    gtk_box_pack_start( GTK_BOX(boite), infos->bouton_acq, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(infos->bouton_acq), "clicked",
                              G_CALLBACK(Menu_acquitter_synoptique), infos );

/******************************************************* Horloges *************************************************************/
    bouton = gtk_button_new_with_label( "Horloges" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_get_horloge_synoptique), infos );

    gtk_widget_show_all( page->child );

    label = gtk_event_box_new ();
    gtk_container_add( GTK_CONTAINER(label), gtk_label_new ( syn->libelle ) );
    gdk_color_parse ("cyan", &color);
    gtk_widget_modify_bg ( label, GTK_STATE_NORMAL, &color );
    gtk_widget_modify_bg ( label, GTK_STATE_ACTIVE, &color );
    gtk_widget_show_all( label );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), page->child, label );
  }
/**********************************************************************************************************/
/* Proto_afficher_un_motif_supervision: Ajoute un motif sur la trame de supervision                       */
/* Entrée: une reference sur le motif                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_motif_supervision( struct CMD_TYPE_MOTIF *rezo_motif )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct TYPE_INFO_SUPERVISION *infos;
    struct CMD_TYPE_MOTIF *motif;

    infos = Rechercher_infos_supervision_par_id_syn ( rezo_motif->syn_id );
    if (!(infos && infos->Trame)) return;
    motif = (struct CMD_TYPE_MOTIF *)g_try_malloc0( sizeof(struct CMD_TYPE_MOTIF) );
    if (!motif) return;

    memcpy( motif, rezo_motif, sizeof(struct CMD_TYPE_MOTIF) );
    trame_motif = Trame_ajout_motif ( FALSE, infos->Trame, motif );
    if (!trame_motif)
     { printf("Erreur creation d'un nouveau motif\n");
       return;                                                          /* Ajout d'un test anti seg-fault */
     }
    trame_motif->groupe_dpl = Nouveau_groupe();                   /* Numéro de groupe pour le deplacement */
printf("New motif %s:%s\n", motif->tech_id, motif->acronyme);
    g_snprintf( trame_motif->color, sizeof(trame_motif->color), "%s", motif->def_color );
    trame_motif->etat   = 0;                                                     /* Sauvegarde etat motif */
    trame_motif->cligno = 0;                                                     /* Sauvegarde etat motif */
    g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-release-event",
                      G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
  }
/******************************************************************************************************************************/
/* Changer_etat_motif: Changement d'etat d'un motif                                                                           */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Changer_etat_motif( struct TRAME_ITEM_MOTIF *trame_motif, struct CMD_ETAT_BIT_CTRL *etat_motif )
  { printf("Changer_etat_motif: %s:%s = %s etat %d (type %d) cligno=%d %s\n",
            trame_motif->motif->tech_id,
            trame_motif->motif->acronyme, etat_motif->color,
            etat_motif->etat, trame_motif->motif->type_gestion, etat_motif->cligno, trame_motif->motif->libelle );
    g_snprintf( trame_motif->color, sizeof(trame_motif->color), "%s", etat_motif->color );
    trame_motif->etat   = etat_motif->etat;                                      /* Sauvegarde etat motif */
    trame_motif->cligno = etat_motif->cligno;                                    /* Sauvegarde etat motif */

    switch( trame_motif->motif->type_gestion )
     { case TYPE_INERTE: break;                          /* Si le motif est inerte, nous n'y touchons pas */
       case TYPE_STATIQUE:
            Trame_choisir_frame( trame_motif, 0, etat_motif->color );                           /* frame 1 */
            break;
       case TYPE_BOUTON:
            if ( ! (etat_motif->etat % 2) )
             { Trame_choisir_frame( trame_motif, 3*(etat_motif->etat/2),
                                    etat_motif->color );
             } else
             { Trame_choisir_frame( trame_motif, 3*(etat_motif->etat/2) + 1,
                                    etat_motif->color );
             }
            break;
       case TYPE_DYNAMIQUE:
            Trame_choisir_frame( trame_motif, etat_motif->etat, etat_motif->color );            /* frame 1 */
       case TYPE_PROGRESSIF:
            Trame_peindre_motif ( trame_motif, etat_motif->color );
            break;
       default: printf("Changer_etat_motif: type gestion non géré %d %s:%s\n",
                        trame_motif->motif->type_gestion, trame_motif->motif->tech_id, trame_motif->motif->acronyme );
     }
  }
/******************************************************************************************************************************/
/* Changer_etat_passerelle: Changement d'etat d'une passerelle (toutes les vignettes)                                         */
/* Entrée: une reference sur la passerelle, l'etat attendu                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Changer_etat_passerelle( struct TRAME_ITEM_PASS *trame_pass, struct CMD_TYPE_SYN_VARS *vars )
  { printf("Changer_etat_passerelle syn %d!\n", vars->syn_id );

    if (vars->bit_comm_out == TRUE)  /****************************  Vignette Activite *****************************************/
     { Trame_set_svg ( trame_pass->item_1, "kaki", 0, TRUE ); }
    else if (vars->bit_alarme == TRUE)
     { Trame_set_svg ( trame_pass->item_1, "rouge", 0, TRUE ); }
    else if (vars->bit_alarme_fixe == TRUE)
     { Trame_set_svg ( trame_pass->item_1, "rouge", 0, FALSE ); }
    else if (vars->bit_defaut == TRUE)
     { Trame_set_svg ( trame_pass->item_1, "jaune", 0, TRUE ); }
    else if (vars->bit_defaut_fixe == TRUE)
     { Trame_set_svg ( trame_pass->item_1, "jaune", 0, FALSE ); }
    else
     { Trame_set_svg ( trame_pass->item_1, "vert", 0, FALSE ); }


    if (vars->bit_comm_out == TRUE) /******************************* Vignette Securite des Personnes **************************/
     { Trame_set_svg ( trame_pass->item_2, "kaki", 0, TRUE ); }
    else if (vars->bit_alerte == TRUE)
     { Trame_set_svg ( trame_pass->item_2, "rouge", 0, TRUE ); }
    else if (vars->bit_alerte_fixe == TRUE)
     { Trame_set_svg ( trame_pass->item_2, "rouge", 0, FALSE ); }
    else if (vars->bit_veille_totale == TRUE)
     { Trame_set_svg ( trame_pass->item_2, "vert", 0, FALSE ); }
    else if (vars->bit_veille_partielle == TRUE)
     { Trame_set_svg ( trame_pass->item_2, "orange", 0, FALSE ); }
    else
     { Trame_set_svg ( trame_pass->item_2, "blanc", 0, FALSE ); }

    if (vars->bit_comm_out == TRUE) /******************************* Vignette Securite des Biens ******************************/
     { Trame_set_svg ( trame_pass->item_3, "kaki", 0, TRUE ); }
    else if (vars->bit_danger == TRUE)
     { Trame_set_svg ( trame_pass->item_3, "rouge", 0, TRUE ); }
    else if (vars->bit_danger_fixe == TRUE)
     { Trame_set_svg ( trame_pass->item_3, "rouge", 0, FALSE ); }
    else if (vars->bit_derangement == TRUE)
     { Trame_set_svg ( trame_pass->item_3, "jaune", 0, TRUE ); }
    else if (vars->bit_derangement_fixe == TRUE)
     { Trame_set_svg ( trame_pass->item_3, "jaune", 0, FALSE ); }
    else
     { Trame_set_svg ( trame_pass->item_3, "vert", 0, FALSE ); }
 }
/******************************************************************************************************************************/
/* Changer_etat_passerelle: Changement d'etat d'une passerelle (toutes les vignettes)                                         */
/* Entrée: une reference sur la passerelle, l'etat attendu                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Changer_etat_etiquette( struct TYPE_INFO_SUPERVISION *infos, struct CMD_TYPE_SYN_VARS *vars )
  { printf("Changer_etat_etiquette syn %d!\n", vars->syn_id );
    if (vars->bit_comm_out == TRUE)  /****************************  Vignette Activite *****************************************/
     { Trame_set_svg ( infos->Trame->Vignette_activite, "kaki", 0, TRUE ); }
    else if (vars->bit_alarme == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_activite, "rouge", 0, TRUE ); }
    else if (vars->bit_alarme_fixe == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_activite, "rouge", 0, FALSE ); }
    else if (vars->bit_defaut == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_activite, "jaune", 0, TRUE ); }
    else if (vars->bit_defaut_fixe == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_activite, "jaune", 0, FALSE ); }
    else
     { Trame_set_svg ( infos->Trame->Vignette_activite, "vert", 0, FALSE ); }

    if (vars->bit_comm_out == TRUE) /******************************* Vignette Securite des Biens ******************************/
     { Trame_set_svg ( infos->Trame->Vignette_secu_bien, "kaki", 0, TRUE ); }
    else if (vars->bit_alerte == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_secu_bien, "rouge", 0, TRUE ); }
    else if (vars->bit_alerte_fixe == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_secu_bien, "rouge", 0, FALSE ); }
    else if (vars->bit_veille_totale == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_secu_bien, "vert", 0, FALSE ); }
    else if (vars->bit_veille_partielle == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_secu_bien, "orange", 0, FALSE ); }
    else
     { Trame_set_svg ( infos->Trame->Vignette_secu_bien, "blanc", 0, FALSE ); }

    if (vars->bit_comm_out == TRUE) /******************************* Vignette Securite des Personnes **************************/
     { Trame_set_svg ( infos->Trame->Vignette_secu_personne, "kaki", 0, TRUE ); }
    else if (vars->bit_danger == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_secu_personne, "rouge", 0, TRUE ); }
    else if (vars->bit_danger_fixe == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_secu_personne, "rouge", 0, FALSE ); }
    else if (vars->bit_derangement == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_secu_personne, "jaune", 0, TRUE ); }
    else if (vars->bit_derangement_fixe == TRUE)
     { Trame_set_svg ( infos->Trame->Vignette_secu_personne, "jaune", 0, FALSE ); }
    else
     { Trame_set_svg ( infos->Trame->Vignette_secu_personne, "vert", 0, FALSE ); }
 }
/******************************************************************************************************************************/
/* Proto_rafrachir_un_message: Rafraichissement du message en parametre                                                       */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_changer_etat_motif( struct CMD_ETAT_BIT_CTRL *etat_motif )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page;
    GList *liste_motifs;
    GList *liste;
    gint cpt;

printf("Recu changement etat motif: %s:%s = %d color=%s\n", etat_motif->tech_id, etat_motif->acronyme, etat_motif->etat, etat_motif->color );
    cpt = 0;                                                                     /* Nous n'avons encore rien fait au debut !! */
    liste = Liste_pages;
    while(liste)                                                                  /* On parcours toutes les pages SUPERVISION */
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type != TYPE_PAGE_SUPERVISION) { liste = liste->next; continue; }

       infos = (struct TYPE_INFO_SUPERVISION *)page->infos;

       liste_motifs = infos->Trame->trame_items;                                /* On parcours tous les motifs de chaque page */
       while (liste_motifs)
        { switch( *((gint *)liste_motifs->data) )
           { case TYPE_MOTIF      : trame_motif = (struct TRAME_ITEM_MOTIF *)liste_motifs->data;
                                    printf("Searching for %s:%s with %s:%s\n",trame_motif->motif->tech_id, trame_motif->motif->acronyme,
                                           etat_motif->tech_id, etat_motif->acronyme );
                                    if (!strcasecmp(trame_motif->motif->tech_id, etat_motif->tech_id) &&
                                        !strcasecmp(trame_motif->motif->acronyme, etat_motif->acronyme) )
                                     { Changer_etat_motif( trame_motif, etat_motif );
                                       cpt++;                                             /* Nous updatons un motif de plus ! */
                                     }
                                    break;
             case TYPE_COMMENTAIRE: break;
             default: break;
           }
          liste_motifs=liste_motifs->next;
        }
       liste = liste->next;
     }
    if (!cpt)                                 /* Si nous n'avons rien mis à jour, c'est que le bit Ixxx ne nous est pas utile */
     { Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_CHANGE_MOTIF_UNKNOWN, (gchar *)etat_motif, sizeof(struct CMD_ETAT_BIT_CTRL) );
     }
  }
/******************************************************************************************************************************/
/* Proto_set_syn_vars: Mets a jour les variables de run d'un synoptique                                                       */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_set_syn_vars( struct CMD_TYPE_SYN_VARS *syn_vars )
  { struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page;
    GdkColor color;
    GList *objet;
    GList *liste;
    gint cpt;

printf("Recu set syn_vars %d  comm_out=%d, def=%d, ala=%d, vp=%d, vt=%d, ale=%d, der=%d, dan=%d\n",syn_vars->syn_id,
                   syn_vars->bit_comm_out, syn_vars->bit_defaut, syn_vars->bit_alarme,
                   syn_vars->bit_veille_partielle, syn_vars->bit_veille_totale, syn_vars->bit_alerte,
                   syn_vars->bit_derangement, syn_vars->bit_danger );
    cpt = 0;                                                                     /* Nous n'avons encore rien fait au debut !! */
    liste = Liste_pages;
    while(liste)                                                                  /* On parcours toutes les pages SUPERVISION */
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type != TYPE_PAGE_SUPERVISION) { liste = liste->next; continue; }

       infos = (struct TYPE_INFO_SUPERVISION *)page->infos;

       if (infos->syn.id == syn_vars->syn_id)
        {                                                     /* Positionnement de la couleur du bouton d'acquit du synotique */
          if (syn_vars->bit_defaut || syn_vars->bit_alarme || syn_vars->bit_alerte ||
              syn_vars->bit_derangement || syn_vars->bit_danger)
           { gdk_color_parse ("blue", &color);
             gtk_widget_modify_bg ( infos->bouton_acq, GTK_STATE_NORMAL, &color );
           } else
           { gtk_widget_modify_bg ( infos->bouton_acq, GTK_STATE_NORMAL, NULL ); }

           Changer_etat_etiquette( infos, syn_vars );                          /* Positionnement des vignettes du synoptiques */
        }

       objet = infos->Trame->trame_items;
       while (objet)
        { switch ( *((gint *)objet->data) )                             /* Test du type de données dans data */
           { case TYPE_PASSERELLE:
                   { struct TRAME_ITEM_PASS *trame_pass = objet->data;
                     if (trame_pass->pass->syn_cible_id == syn_vars->syn_id)
                      { Changer_etat_passerelle( trame_pass, syn_vars );
                        cpt++;                                                            /* Nous updatons un motif de plus ! */
                      }
                   }
                  break;
             default: break;
           }
          objet=objet->next;
        }
       liste = liste->next;
     }

    if (!cpt)                                 /* Si nous n'avons rien mis à jour, c'est que le bit Ixxx ne nous est pas utile */
     { Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_SET_SYN_VARS_UNKNOWN, (gchar *)syn_vars, sizeof(struct CMD_TYPE_SYN_VARS) );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
