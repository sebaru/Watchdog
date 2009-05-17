/**********************************************************************************************************/
/* Client/supervision.c        Affichage du synoptique de supervision                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 29 mar 2009 09:54:22 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * supervision.c
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

/**********************************************************************************************************/
/* Rechercher_infos_supervision_par_id_syn: Recherche une page synoptique par son numéro                  */
/* Entrée: Un numéro de synoptique                                                                        */
/* Sortie: Une référence sur les champs d'information de la page en question                              */
/**********************************************************************************************************/
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
          if (infos->syn_id == syn_id) break;                              /* Nous avons trouvé le syn !! */
        }
       liste = liste->next;                                                        /* On passe au suivant */
     }
    return(infos);
  }
/**********************************************************************************************************/
/* Detruire_page_supervision: L'utilisateur veut fermer la page de supervision                            */
/* Entrée: la page en question                                                                            */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Detruire_page_supervision( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_SUPERVISION *infos;
    infos = (struct TYPE_INFO_SUPERVISION *)page->infos;
    /*g_timeout_remove( infos->timer_id );*/
    Trame_detruire_trame( infos->Trame );
  }
/**********************************************************************************************************/
/* Detruire_page_supervision: L'utilisateur veut fermer la page de supervision                            */
/* Entrée: la page en question                                                                            */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Changer_option_zoom (GtkRange *range, struct TYPE_INFO_SUPERVISION *infos )
  { GtkAdjustment *adj;
    g_object_get( infos->Option_zoom, "adjustment", &adj, NULL );
    goo_canvas_set_scale ( GOO_CANVAS(infos->Trame->trame_widget), gtk_adjustment_get_value(adj) );
  }
/**********************************************************************************************************/
/* draw_page: Dessine une page pour l'envoyer sur l'imprimante                                            */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void draw_page (GtkPrintOperation *operation,
                        GtkPrintContext   *context,
                        gint               page_nr,
                        struct TYPE_INFO_SUPERVISION *infos)
  { cairo_t *cr;
    cr = gtk_print_context_get_cairo_context (context);
    cairo_scale ( cr, 0.85, 0.85 );
    goo_canvas_render ( GOO_CANVAS( infos->Trame->trame_widget ), cr, NULL, 1.0 );
  }
/**********************************************************************************************************/
/* Menu_exporter_message: Exportation de la base dans un fichier texte                                    */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Menu_exporter_synoptique( struct TYPE_INFO_SUPERVISION *infos )
  { GtkPrintOperation *print;
    GtkPrintOperationResult res;
    GError *error;

    print = New_print_job ( "Print Synoptique" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (draw_page), infos );
    gtk_print_operation_set_n_pages ( print, 1 );

    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   GTK_WINDOW(F_client), &error);
  }
/**********************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_supervision ( gchar *libelle, guint syn_id )
  { GtkWidget *bouton, *boite, *hboite, *scroll, *frame;
    GtkAdjustment *adj;
    struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page;
    static gint init_timer;

    page = (struct PAGE_NOTEBOOK *)g_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->infos = (struct TYPE_INFO_SUPERVISION *)g_malloc0( sizeof(struct TYPE_INFO_SUPERVISION) );
    infos = (struct TYPE_INFO_SUPERVISION *)page->infos;
    if (!page->infos) { g_free(page); return; }

    page->type   = TYPE_PAGE_SUPERVISION;
    Liste_pages  = g_list_append( Liste_pages, page );
    infos->syn_id = syn_id;

    if (!init_timer) { g_timeout_add( 500, Timer, NULL ); init_timer = 1; }

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
/**************************************** Trame proprement dite *******************************************/
    
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    infos->Trame = Trame_creer_trame( TAILLE_SYNOPTIQUE_X, TAILLE_SYNOPTIQUE_Y, "darkgray", 0 );
    gtk_container_add( GTK_CONTAINER(scroll), infos->Trame->trame_widget );

/**************************************** Boutons de controle *********************************************/
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

/*********************************************** Zoom *****************************************************/
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

/************************************************* Palettes ***********************************************/
    frame = gtk_frame_new( _("Palette") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    infos->Box_palette = gtk_vbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(infos->Box_palette), 6 );
    gtk_container_add( GTK_CONTAINER(frame), infos->Box_palette );

    gtk_widget_show_all( page->child );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), page->child, gtk_label_new ( libelle ) );
 /*   infos->timer_id = g_timeout_add( 500, Timer, NULL );*/
  }
/**********************************************************************************************************/
/* Proto_afficher_un_motif_supervision: Ajoute un motif sur la trame de supervision                       */
/* Entrée: une reference sur le motif                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_motif_supervision( struct CMD_SHOW_MOTIF *rezo_motif )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct TYPE_INFO_SUPERVISION *infos;
    struct MOTIF *motif;

    infos = Rechercher_infos_supervision_par_id_syn ( rezo_motif->syn_id );
    if (!(infos && infos->Trame)) return;
    motif = (struct MOTIF *)g_malloc0( sizeof(struct MOTIF) );
    if (!motif)
     { Info( Config_cli.log, DEBUG_MEM, "Afficher_motif_supervision: not enought memory" );
       return;
     }

    motif->id = rezo_motif->id;                                     /* Numero du motif dans la DBWatchdog */
    motif->icone_id = rezo_motif->icone_id;                                 /* Correspond au fichier .gif */
    printf("Libelle motif=%s id %d coul %d %d %d\n", rezo_motif->libelle, rezo_motif->id,
                                                     rezo_motif->rouge0,
                                                     rezo_motif->vert0,
                                                     rezo_motif->bleu0 );
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

    trame_motif = Trame_ajout_motif ( FALSE, infos->Trame, motif );
    trame_motif->groupe_dpl = Nouveau_groupe();                   /* Numéro de groupe pour le deplacement */
    trame_motif->rouge  = motif->rouge0;                                         /* Sauvegarde etat motif */
    trame_motif->vert   = motif->vert0;                                          /* Sauvegarde etat motif */
    trame_motif->bleu   = motif->bleu0;                                          /* Sauvegarde etat motif */
    trame_motif->etat   = 0;                                                     /* Sauvegarde etat motif */
    trame_motif->cligno = 0;                                                     /* Sauvegarde etat motif */
    g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-release-event",
                      G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
  }
/**********************************************************************************************************/
/* Changer_etat_motif: Changement d'etat d'un motif                                                       */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Changer_etat_motif( struct TRAME_ITEM_MOTIF *trame_motif, struct CMD_ETAT_BIT_CTRL *etat_motif )
  { printf("Changer_etat_motif: %s %d = %d %d %d etat %d (type %d) cligno=%d\n",
            trame_motif->motif->libelle,
            trame_motif->motif->bit_controle,
            etat_motif->rouge,
            etat_motif->vert,
            etat_motif->bleu, etat_motif->etat, trame_motif->motif->type_gestion, etat_motif->cligno );
    trame_motif->rouge  = etat_motif->rouge;                                     /* Sauvegarde etat motif */
    trame_motif->vert   = etat_motif->vert;                                      /* Sauvegarde etat motif */
    trame_motif->bleu   = etat_motif->bleu;                                      /* Sauvegarde etat motif */
    trame_motif->etat   = etat_motif->etat;                                      /* Sauvegarde etat motif */
    trame_motif->cligno = etat_motif->cligno;                                    /* Sauvegarde etat motif */
    switch( trame_motif->motif->type_gestion )
     { case TYPE_INERTE: break;                          /* Si le motif est inerte, nous n'y touchons pas */
       case TYPE_STATIQUE:
            Trame_choisir_frame( trame_motif, 0, etat_motif->rouge,
                                                 etat_motif->vert,
                                                 etat_motif->bleu );                           /* frame 1 */
            break;
       case TYPE_BOUTON:
       case TYPE_DYNAMIQUE: 
            if (etat_motif->etat)
             { Trame_choisir_frame( trame_motif, 1, etat_motif->rouge,
                                                    etat_motif->vert,
                                                    etat_motif->bleu );                        /* frame 1 */
             }
            else
             { Trame_choisir_frame( trame_motif, 0, etat_motif->rouge,
                                                    etat_motif->vert,
                                                    etat_motif->bleu );
             }
            break;
       case TYPE_PROGRESSIF:
            Trame_peindre_motif ( trame_motif, etat_motif->rouge,
                                               etat_motif->vert,
                                               etat_motif->bleu );
            break;
       case TYPE_INDICATEUR:
            Trame_choisir_frame( trame_motif, etat_motif->etat, etat_motif->rouge,
                                                                etat_motif->vert,
                                                                etat_motif->bleu );            /* frame 1 */
            break;
       default: printf("Changer_etat_motif: type gestion non géré %d bit_ctrl=%d\n",
                        trame_motif->motif->type_gestion, trame_motif->motif->bit_controle );
     }
    printf("Changer_etat_motif: sortie\n");
    
  }
/**********************************************************************************************************/
/* Changer_etat_motif: Changement d'etat d'un motif                                                       */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Changer_etat_pass_1( struct TRAME_ITEM_PASS *trame_pass, struct CMD_ETAT_BIT_CTRL *etat_motif )
  { printf("Changer_etat_pass !\n");
    trame_pass->rouge1  = etat_motif->rouge;                                     /* Sauvegarde etat motif */
    trame_pass->vert1   = etat_motif->vert;                                      /* Sauvegarde etat motif */
    trame_pass->bleu1   = etat_motif->bleu;                                      /* Sauvegarde etat motif */
    trame_pass->cligno1  = etat_motif->cligno;                                   /* Sauvegarde etat motif */

    Trame_peindre_pass_1 ( trame_pass, etat_motif->rouge,
                                     etat_motif->vert,
                                     etat_motif->bleu );
    printf("Changer_etat_pass: sortie\n");
    
  }
/**********************************************************************************************************/
/* Changer_etat_motif: Changement d'etat d'un motif                                                       */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Changer_etat_pass_2( struct TRAME_ITEM_PASS *trame_pass, struct CMD_ETAT_BIT_CTRL *etat_motif )
  { printf("Changer_etat_pass !\n");
    trame_pass->rouge2  = etat_motif->rouge;                                     /* Sauvegarde etat motif */
    trame_pass->vert2   = etat_motif->vert;                                      /* Sauvegarde etat motif */
    trame_pass->bleu2   = etat_motif->bleu;                                      /* Sauvegarde etat motif */
    trame_pass->cligno2  = etat_motif->cligno;                                   /* Sauvegarde etat motif */

    Trame_peindre_pass_2 ( trame_pass, etat_motif->rouge,
                                     etat_motif->vert,
                                     etat_motif->bleu );
    printf("Changer_etat_pass_2: sortie\n");
    
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_message: Rafraichissement du message en parametre                                   */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_changer_etat_motif( struct CMD_ETAT_BIT_CTRL *etat_motif )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct TRAME_ITEM_PASS *trame_pass;
    struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page;
    GList *liste_motifs;
    GList *liste;
    gint cpt;

printf("Recu changement etat motif: %d = r%d v%d b%d\n", etat_motif->num, etat_motif->rouge,
                                                         etat_motif->vert, etat_motif->bleu );
    cpt = 0;                                                 /* Nous n'avons encore rien fait au debut !! */
    liste = Liste_pages;
    while(liste)                                              /* On parcours toutes les pages SUPERVISION */
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type != TYPE_PAGE_SUPERVISION) { liste = liste->next; continue; }

       infos = (struct TYPE_INFO_SUPERVISION *)page->infos;

       liste_motifs = infos->Trame->trame_items;            /* On parcours tous les motifs de chaque page */
       while (liste_motifs)
        { switch( *((gint *)liste_motifs->data) )
           { case TYPE_MOTIF      : trame_motif = (struct TRAME_ITEM_MOTIF *)liste_motifs->data;
                                    if (trame_motif->motif->bit_controle == etat_motif->num)
                                     { Changer_etat_motif( trame_motif, etat_motif );
                                       cpt++;                         /* Nous updatons un motif de plus ! */ 
                                     }
                                    break;
             case TYPE_COMMENTAIRE: break;
             case TYPE_PASSERELLE : trame_pass = (struct TRAME_ITEM_PASS *)liste_motifs->data;
                                    if (trame_pass->pass->bit_controle_1 == etat_motif->num)
                                     { Changer_etat_pass_1( trame_pass, etat_motif );
                                       cpt++;                         /* Nous updatons un motif de plus ! */ 
                                     }
                                    else if (trame_pass->pass->bit_controle_2 == etat_motif->num)
                                     { Changer_etat_pass_2( trame_pass, etat_motif );
                                       cpt++;                         /* Nous updatons un motif de plus ! */ 
                                     }
                                    break;
             default: break;
           }
          liste_motifs=liste_motifs->next;
        }
       liste = liste->next;
     }
    if (!cpt)             /* Si nous n'avons rien mis à jour, c'est que le bit Ixxx ne nous est pas utile */
     { Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_CHANGE_MOTIF_UNKNOWN,
                      (gchar *)etat_motif, sizeof(struct CMD_ETAT_BIT_CTRL) ); 
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
