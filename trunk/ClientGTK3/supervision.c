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

 #include <sys/time.h>

 #include "Reseaux.h"
 #include "trame.h"

/********************************************* Définitions des prototypes programme *******************************************/
 #include "protocli.h"

 enum
  {  COLONNE_HORLOGE_ID_MNEMO,
     COLONNE_HORLOGE_LIBELLE,
     NBR_COLONNE_HORLOGE
  };
/******************************************************************************************************************************/
/* Detruire_page_supervision: L'utilisateur veut fermer la page de supervision                                                */
/* Entrée: la page en question                                                                                                */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Detruire_page_supervision( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_SUPERVISION *infos = page->infos;

    soup_websocket_connection_close ( infos->ws_motifs, 0, "Thanks, Bye !" );
    json_node_unref( infos->syn );
    /*g_timeout_remove( infos->timer_id );*/
    Trame_detruire_trame( infos->Trame );

    gint num = gtk_notebook_page_num( GTK_NOTEBOOK(infos->client->Notebook), GTK_WIDGET(page->child) );
    gtk_notebook_remove_page( GTK_NOTEBOOK(infos->client->Notebook), num );
    infos->client->Liste_pages = g_slist_remove( infos->client->Liste_pages, page );
    g_free(infos);                                                                     /* Libération des infos le cas échéant */
    g_free(page);
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
 static void Menu_exporter_synoptique( struct CLIENT *client, struct TYPE_INFO_SUPERVISION *infos )
  { GtkPrintOperation *print;
    GError *error;

//    print = New_print_job ( "Print Synoptique" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (draw_page), infos );
    gtk_print_operation_set_n_pages ( print, 1 );

    gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                             GTK_WINDOW(client->window), &error);
  }
/******************************************************************************************************************************/
/* Menu_acquitter_synoptique: Envoi une commande d'acquit du synoptique en cours de visu                                      */
/* Entrée: La page d'information synoptique                                                                                   */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Menu_acquitter_synoptique( struct TYPE_INFO_SUPERVISION *infos )
  { //Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_ACQ_SYN, (gchar *)&infos->syn.id, sizeof(gint) );
  }
/******************************************************************************************************************************/
/* Menu_acquitter_synoptique: Envoi une commande d'acquit du synoptique en cours de visu                                      */
/* Entrée: La page d'information synoptique                                                                                   */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Menu_get_horloge_synoptique( struct TYPE_INFO_SUPERVISION *infos )
  { gchar chaine[80];
    printf("Get Horloge from %s\n", Json_get_string(infos->syn, "page") );
    g_snprintf(chaine, sizeof(chaine), "horloges/list/%s", Json_get_string(infos->syn, "page") );
//    Firefox_exec ( chaine );
  }
/******************************************************************************************************************************/
/* Afficher_un_motif: Ajoute un motif sur la trame de supervision                                                             */
/* Entrée: les parametres iteratif JSon                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Afficher_un_motif (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct TYPE_INFO_SUPERVISION *infos=user_data;
    struct TRAME_ITEM_MOTIF *trame_motif;
    struct CMD_TYPE_MOTIF *motif;

    if (!(infos && infos->Trame)) return;

    motif = (struct CMD_TYPE_MOTIF *)g_try_malloc0( sizeof(struct CMD_TYPE_MOTIF) );
    if (!motif) return;

    motif->position_x   = atoi(Json_get_string ( element, "posx" ));
    motif->position_y   = atoi(Json_get_string ( element, "posy" ));
    motif->largeur      = atoi(Json_get_string ( element, "larg" ));
    motif->hauteur      = atoi(Json_get_string ( element, "haut" ));
    motif->angle        = atoi(Json_get_string ( element, "angle" ));
    motif->icone_id     = atoi(Json_get_string ( element, "icone" ));
    motif->type_dialog  = atoi(Json_get_string ( element, "dialog" ));
    motif->type_gestion = atoi(Json_get_string ( element, "gestion" ));
    motif->rouge0       = atoi(Json_get_string ( element, "rouge" ));
    motif->vert0        = atoi(Json_get_string ( element, "vert" ));
    motif->bleu0        = atoi(Json_get_string ( element, "bleu" ));
    motif->layer        = atoi(Json_get_string ( element, "layer" ));
    //g_snprintf( motif->tech_id,       sizeof(motif->tech_id),       "%s", Json_get_string( element, "tech_id" ) );
    //g_snprintf( motif->acronyme,      sizeof(motif->acronyme),      "%s", Json_get_string( element, "acronyme" ) );
    g_snprintf( motif->clic_tech_id,  sizeof(motif->clic_tech_id),  "%s", Json_get_string( element, "clic_tech_id" ) );
    g_snprintf( motif->clic_acronyme, sizeof(motif->clic_acronyme), "%s", Json_get_string( element, "clic_acronyme" ) );
    g_snprintf( motif->libelle,       sizeof(motif->libelle),       "%s", Json_get_string( element, "libelle" ) );
    motif->access_level = atoi(Json_get_string ( element, "access_level" ));
    motif->bit_controle = atoi(Json_get_string ( element, "bitctrl" ));
    motif->bit_clic     = atoi(Json_get_string ( element, "bitclic" ));
    motif->rafraich     = atoi(Json_get_string ( element, "rafraich" ));
printf("%s, add motif %s\n", __func__, motif->libelle );
    trame_motif = Trame_ajout_motif ( FALSE, infos->Trame, motif );
    if (!trame_motif)
     { printf("Erreur creation d'un nouveau motif\n");
       return;                                                          /* Ajout d'un test anti seg-fault */
     }
  //  trame_motif->groupe_dpl = Nouveau_groupe();                   /* Numéro de groupe pour le deplacement */
    trame_motif->rouge  = motif->rouge0;                                         /* Sauvegarde etat motif */
    trame_motif->vert   = motif->vert0;                                          /* Sauvegarde etat motif */
    trame_motif->bleu   = motif->bleu0;                                          /* Sauvegarde etat motif */
    trame_motif->etat   = 0;                                                     /* Sauvegarde etat motif */
    trame_motif->cligno = 0;                                                     /* Sauvegarde etat motif */
   // g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-press-event",
   //                   G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
   // g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-release-event",
   //                   G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
  }
/******************************************************************************************************************************/
/* Traiter_reception_ws_msgs_CB: Opere le traitement d'un message recu par la WebSocket MOTIF                                 */
/* Entrée: les parametres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Traiter_reception_ws_motifs_CB ( SoupWebsocketConnection *self, gint type, GBytes *message_brut, gpointer user_data )
  { struct TYPE_INFO_SUPERVISION *infos = user_data;
    gsize taille;
    printf("%s\n", __func__ );
    printf("Recu MOTIFS: %s :\n", g_bytes_get_data ( message_brut, &taille ) );
    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response) return;


  }
/******************************************************************************************************************************/
/* Traiter_reception_ws_msgs_CB: Opere le traitement d'un message recu par la WebSocket MSGS                                  */
/* Entrée: rien                                                                                                               */
/* Sortie: un widget boite                                                                                                    */
/******************************************************************************************************************************/
 static void Traiter_reception_ws_motifs_on_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { printf("%s\n", __func__ );
  }
 static void Traiter_reception_ws_motifs_on_error  ( SoupWebsocketConnection *connexion, GError *error, gpointer user_data )
  { printf("%s: WebSocket Error '%s' received !\n", __func__, error->message );
  }
/******************************************************************************************************************************/
/* Traiter_connect_ws_CB: Termine la creation de la connexion websocket MSGS et raccorde le signal handler                    */
/* Entrée: les variables traditionnelles de libsous                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Traiter_connect_ws_motifs_CB (GObject *source_object, GAsyncResult *res, gpointer user_data )
  { struct TYPE_INFO_SUPERVISION *infos = user_data;
    GError *error = NULL;
    gsize taille_buf;
    printf("%s\n", __func__ );
    infos->ws_motifs = soup_session_websocket_connect_finish ( infos->client->connexion, res, &error );
    if (!infos->ws_motifs)                                                                    /* No limit on incoming packet ! */
     { printf("%s: Error opening Websocket '%s' !\n", __func__, error->message);
       g_error_free (error);
       return;
     }
    g_object_set ( G_OBJECT(infos->ws_motifs), "max-incoming-payload-size", G_GINT64_CONSTANT(0), NULL );
    g_signal_connect ( infos->ws_motifs, "message", G_CALLBACK(Traiter_reception_ws_motifs_CB), infos );
    g_signal_connect ( infos->ws_motifs, "closed",  G_CALLBACK(Traiter_reception_ws_motifs_on_closed), infos );
    g_signal_connect ( infos->ws_motifs, "error",   G_CALLBACK(Traiter_reception_ws_motifs_on_error), infos );
    JsonBuilder *builder = Json_create ();
    if (builder == NULL) return;
    Json_add_array ( builder, "cadrans" );
    GList *liste = infos->Trame->trame_items;
    while (liste)
     { struct TRAME_ITEM *item = liste->data;
       switch( *(gint *)item )
        { case TYPE_CADRAN:
           { struct TRAME_ITEM_CADRAN *trame_cadran = liste->data;
             printf("%s: abonnement cadran to %d %s:%s\n", __func__,
                    trame_cadran->cadran->type, trame_cadran->cadran->tech_id, trame_cadran->cadran->acronyme );
             Json_add_object ( builder, NULL );
             Json_add_string ( builder, "type_abonnement", "cadran" );
             Json_add_int    ( builder, "type", trame_cadran->cadran->type );
             Json_add_string ( builder, "tech_id", trame_cadran->cadran->tech_id );
             Json_add_string ( builder, "acronyme", trame_cadran->cadran->acronyme );
             Json_end_object ( builder );
             break;
           }
        }
       liste = g_list_next ( liste );
     }
    Json_end_array ( builder );
    gchar *buf = Json_get_buf (builder, &taille_buf);
    GBytes *gbytes = g_bytes_new_take ( buf, taille_buf );
    soup_websocket_connection_send_message (infos->ws_motifs, SOUP_WEBSOCKET_DATA_TEXT, gbytes );
    g_bytes_unref( gbytes );
  }
/******************************************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                                        */
/* Entrée: Le libelle a afficher dans le notebook et l'ID du synoptique                                                       */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Creer_page_supervision_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { GtkWidget *bouton, *boite, *hboite, *scroll, *frame, *label;
    struct CLIENT *client = user_data;
    GtkAdjustment *adj;
    struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page;
    static gint init_timer;
    GBytes *response_brute;
    gchar *reason_phrase;
    gint status_code;
    gchar chaine[128];
    gsize taille;

    printf("%s\n", __func__ );
    g_object_get ( msg, "response-body-data", &response_brute, NULL );
    printf("Recu SYNS: %s %p\n", g_bytes_get_data ( response_brute, &taille ), client );

    g_object_get ( msg, "status-code", &status_code, "reason-phrase", &reason_phrase, NULL );
    if (status_code != 200)
     { gchar chaine[256];
       g_snprintf(chaine, sizeof(chaine), "Error loading synoptique: Code %d - %s", status_code, reason_phrase );
       Log(client, chaine);
       return;
     }

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;

    page->infos = (struct TYPE_INFO_SUPERVISION *)g_try_malloc0( sizeof(struct TYPE_INFO_SUPERVISION) );
    infos = (struct TYPE_INFO_SUPERVISION *)page->infos;
    if (!page->infos) { g_free(page); return; }
    infos->client = client;

    page->type   = TYPE_PAGE_SUPERVISION;
    client->Liste_pages  = g_slist_append( client->Liste_pages, page );
    g_object_get ( msg, "response-body-data", &response_brute, NULL );
    infos->syn = Json_get_from_string ( g_bytes_get_data ( response_brute, &taille ) );

    //if (!init_timer) { g_timeout_add( 500, Timer, NULL ); init_timer = 1; }

    hboite = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
/**************************************************** Trame proprement dite ***************************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    infos->Trame = Trame_creer_trame( TAILLE_SYNOPTIQUE_X, TAILLE_SYNOPTIQUE_Y, "darkgray", 0 );
    gtk_container_add( GTK_CONTAINER(scroll), infos->Trame->trame_widget );

/************************************************** Boutons de controle *******************************************************/
    boite = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_with_label( "Fermer" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Detruire_page_supervision), page );

    bouton = gtk_button_new_with_label( "Imprimer" );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    //g_signal_connect_swapped( G_OBJECT(bouton), "clicked", G_CALLBACK(Menu_exporter_synoptique), infos );

/********************************************************** Zoom **************************************************************/
    frame = gtk_frame_new ( "Zoom" );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    hboite = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );

    infos->Option_zoom = gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, 0.2, 5.0, 0.1 );
    gtk_box_pack_start( GTK_BOX(hboite), infos->Option_zoom, FALSE, FALSE, 0 );
    g_object_get( infos->Option_zoom, "adjustment", &adj, NULL );
    gtk_adjustment_set_value( adj, 1.0 );
    g_signal_connect( G_OBJECT( infos->Option_zoom ), "value-changed",
                      G_CALLBACK( Changer_option_zoom ), infos );

/************************************************************* Palettes *******************************************************/
    frame = gtk_frame_new( "Palette" );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX(boite), frame, FALSE, FALSE, 0 );

    infos->Box_palette = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
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
    gtk_container_add( GTK_CONTAINER(label), gtk_label_new ( Json_get_string( infos->syn, "libelle" ) ) );
//    gdk_color_parse ("cyan", &color);
//    gtk_widget_modify_bg ( label, GTK_STATE_NORMAL, &color );
//    gtk_widget_modify_bg ( label, GTK_STATE_ACTIVE, &color );
    gtk_widget_show_all( label );
    gint page_num = gtk_notebook_append_page( GTK_NOTEBOOK(client->Notebook), page->child, label );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK(client->Notebook), page_num );
    json_array_foreach_element ( Json_get_array ( infos->syn, "motifs" ),      Afficher_un_motif, infos );
    json_array_foreach_element ( Json_get_array ( infos->syn, "passerelles" ), Afficher_une_passerelle, infos );
    json_array_foreach_element ( Json_get_array ( infos->syn, "comments" ),    Afficher_un_commentaire, infos );
    json_array_foreach_element ( Json_get_array ( infos->syn, "cameras" ),     Afficher_une_camera, infos );
    json_array_foreach_element ( Json_get_array ( infos->syn, "cadrans" ),     Afficher_un_cadran, infos );

    g_snprintf(chaine, sizeof(chaine), "ws://%s:5560/ws/live-motifs", client->hostname );
    soup_session_websocket_connect_async ( client->connexion, soup_message_new ( "GET", chaine ),
                                           NULL, NULL, g_cancellable_new(), Traiter_connect_ws_motifs_CB, infos );
  }

#ifdef bouh
/******************************************************************************************************************************/
/* Changer_etat_motif: Changement d'etat d'un motif                                                                           */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Changer_etat_motif( struct TRAME_ITEM_MOTIF *trame_motif, struct CMD_ETAT_BIT_CTRL *etat_motif )
  { printf("Changer_etat_motif: %d = %d %d %d etat %d (type %d) cligno=%d %s\n",
            trame_motif->motif->bit_controle,
            etat_motif->rouge,
            etat_motif->vert,
            etat_motif->bleu, etat_motif->etat, trame_motif->motif->type_gestion, etat_motif->cligno, trame_motif->motif->libelle );
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
            if ( ! (etat_motif->etat % 2) )
             { Trame_choisir_frame( trame_motif, 3*(etat_motif->etat/2),
                                    etat_motif->rouge, etat_motif->vert, etat_motif->bleu );
             } else
             { Trame_choisir_frame( trame_motif, 3*(etat_motif->etat/2) + 1,
                                    etat_motif->rouge, etat_motif->vert, etat_motif->bleu );
             }
            break;
       case TYPE_DYNAMIQUE:
            Trame_choisir_frame( trame_motif, etat_motif->etat, etat_motif->rouge,
                                                                etat_motif->vert,
                                                                etat_motif->bleu );            /* frame 1 */
       case TYPE_PROGRESSIF:
            Trame_peindre_motif ( trame_motif, etat_motif->rouge,
                                               etat_motif->vert,
                                               etat_motif->bleu );
            break;
       default: printf("Changer_etat_motif: type gestion non géré %d bit_ctrl=%d\n",
                        trame_motif->motif->type_gestion, trame_motif->motif->bit_controle );
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

printf("Recu changement etat motif: %d = %d r%d v%d b%d\n", etat_motif->num, etat_motif->etat, etat_motif->rouge,
                                                         etat_motif->vert, etat_motif->bleu );
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
                                    if (trame_motif->motif->bit_controle == etat_motif->num)
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
    liste = client->Liste_pages;
    while(liste)                                                                  /* On parcours toutes les pages SUPERVISION */
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type != TYPE_PAGE_SUPERVISION) { liste = liste->next; continue; }

       infos = (struct TYPE_INFO_SUPERVISION *)page->infos;

       if (infos->syn.id == syn_vars->syn_id)
        {                                                     /* Positionnement de la couleur du bouton d'acquit du synotique */
          if (syn_vars->bit_defaut || syn_vars->bit_alarme || syn_vars->bit_alerte ||
              syn_vars->bit_derangement || syn_vars->bit_danger)
           { //gdk_color_parse ("blue", &color);
             //gtk_widget_override_background_color ( infos->bouton_acq, GTK_STATE_NORMAL, &color );
           } else
           { //gtk_widget_override_background_color ( infos->bouton_acq, GTK_STATE_NORMAL, NULL );
           }

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
#endif
/******************************************************************************************************************************/
/* Demander_synoptique_supervision: Envoie une demande d'affichage synoptique au serveur                                      */
/* Entrée/Sortie: l'instance cliente, l'id du synoptique a demander                                                           */
/******************************************************************************************************************************/
 void Demander_synoptique_supervision ( struct CLIENT *client, gint id )
  { gchar chaine[80];
    g_snprintf( chaine, sizeof(chaine), "syn/get/%d", id );
    Envoi_au_serveur( client, "GET", NULL, 0, chaine, Creer_page_supervision_CB );
  }
/**********************************************************************************************************/
/* Menu_want_supervision: l'utilisateur desire voir le synoptique supervision                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_supervision_accueil ( struct CLIENT *client )
  { if (Chercher_page_notebook( client, TYPE_PAGE_SUPERVISION, 1, TRUE )) return;
    Demander_synoptique_supervision ( client, 1 );
  }

/*----------------------------------------------------------------------------------------------------------------------------*/
