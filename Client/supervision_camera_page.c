/**********************************************************************************************************/
/* Client/supervision_page_camera.c          Affichage des cameras de supervision  en plien ecran         */
/* Projet WatchDog version 2.0       Gestion d'habitat                   sam. 12 sept. 2009 16:53:33 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * camera.c
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
 #include <gdk/gdkx.h>
 #include <gtk-libvlc-media-player.h>                                                       /* Player VLC */
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
/* Detruire_page_supervision: L'utilisateur veut fermer la page de supervision                            */
/* Entrée: la page en question                                                                            */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Detruire_page_supervision_camera( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_CAMERA *infos;
    infos = (struct TYPE_INFO_CAMERA *)page->infos;

    gtk_libvlc_media_player_stop(GTK_LIBVLC_MEDIA_PLAYER(infos->vlc)); 
    g_object_unref(G_OBJECT(infos->instance)); 
  }
/**********************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_supervision_camera ( struct CMD_TYPE_CAMERA_SUP *camera )
  { GtkWidget *bouton, *boite, *hboite;
    struct TYPE_INFO_CAMERA *infos;
    struct PAGE_NOTEBOOK *page;
    GtkLibVLCMedia *media;

    page = (struct PAGE_NOTEBOOK *)g_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;

    page->infos = (struct TYPE_INFO_CAMERA *)g_malloc0( sizeof(struct TYPE_INFO_CAMERA) );
    infos = (struct TYPE_INFO_CAMERA *)page->infos;
    if (!page->infos) { g_free(page); return; }

    page->type   = TYPE_PAGE_SUPERVISION_CAMERA;
    Liste_pages  = g_list_append( Liste_pages, page );
    memcpy( &infos->camera, camera, sizeof( struct CMD_TYPE_CAMERA_SUP) );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );

    infos->instance = gtk_libvlc_instance_new(NULL); 
    infos->vlc = gtk_libvlc_media_player_new(infos->instance);
    gtk_box_pack_start( GTK_BOX(hboite), infos->vlc, TRUE, TRUE, 0 );
    gtk_libvlc_media_player_set_volume (GTK_LIBVLC_MEDIA_PLAYER(infos->vlc), 0.0);

    media = gtk_libvlc_media_new(infos->camera.location);
    gtk_libvlc_media_player_add_media(GTK_LIBVLC_MEDIA_PLAYER(infos->vlc), media);
    g_object_unref(media);         

/************************************ Les boutons de controles ********************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );


    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), page->child, gtk_label_new ( infos->camera.libelle ) );
    gtk_widget_show_all( page->child );
    Chercher_page_notebook ( TYPE_PAGE_SUPERVISION_CAMERA, camera->camera_src_id, TRUE );
    gtk_libvlc_media_player_play(GTK_LIBVLC_MEDIA_PLAYER(infos->vlc), NULL); 
 }
/*--------------------------------------------------------------------------------------------------------*/
