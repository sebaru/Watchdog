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
 #include <gst/gst.h>
 #include <gst/interfaces/xoverlay.h>
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
    gst_element_set_state (infos->pipeline, GST_STATE_NULL);                    /* Extinction du pipeline */
    gst_object_unref( GST_OBJECT(infos->pipeline) );
  }


 void Test ( void )
{
struct CMD_TYPE_CAMERA camera={ 0, "testseb", "http://guest:guest@192.168.0.30/cgi/mjpg/mjpg.cgi", 0 };
Creer_page_supervision_camera( &camera );


}
/**********************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_supervision_camera ( struct CMD_TYPE_CAMERA *camera )
  { GtkWidget *bouton, *boite, *hboite;
    struct TYPE_INFO_CAMERA *infos;
    struct PAGE_NOTEBOOK *page;
    GstElement *source, *jpegdec, *ffmpeg, *sink;

    page = (struct PAGE_NOTEBOOK *)g_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;

    page->infos = (struct TYPE_INFO_CAMERA *)g_malloc0( sizeof(struct TYPE_INFO_CAMERA) );
    infos = (struct TYPE_INFO_CAMERA *)page->infos;
    if (!page->infos) { g_free(page); return; }

    page->type   = TYPE_PAGE_SUPERVISION_CAMERA;
    Liste_pages  = g_list_append( Liste_pages, page );
    memcpy( &infos->camera, camera, sizeof( struct CMD_TYPE_CAMERA) );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );

    infos->video_output = gtk_drawing_area_new ();
    gtk_box_pack_start (GTK_BOX (hboite), infos->video_output, TRUE, TRUE, 0);
    /*gtk_widget_set_size_request (infos->video_output, 640, 480);*/

/**************************************** Trame proprement dite *******************************************/
    
    /* Create gstreamer elements */
    infos->pipeline = gst_pipeline_new (NULL);

    source  = gst_element_factory_make ( "gnomevfssrc", NULL );
    g_object_set (G_OBJECT (source), "location", infos->camera.location, NULL);

    jpegdec  = gst_element_factory_make ("jpegdec", NULL);
    ffmpeg   = gst_element_factory_make ("ffmpegcolorspace", NULL );
    sink    = gst_element_factory_make ( "ximagesink", NULL );

    gst_bin_add_many (GST_BIN (infos->pipeline), source, jpegdec, ffmpeg, sink, NULL);
    gst_element_link_many (source, jpegdec, ffmpeg, sink, NULL);
 
    /* gst_x_overlay_handle_events (GST_X_OVERLAY (sink), FALSE);*/

/************************************ Les boutons de controles ********************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );


    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), page->child, gtk_label_new ( infos->camera.libelle ) );
    gtk_widget_show_all( page->child );
    gtk_widget_realize ( infos->video_output );
    gtk_main_iteration_do( TRUE );
    gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (sink), GDK_WINDOW_XWINDOW (infos->video_output->window));
    gst_element_set_state (infos->pipeline, GST_STATE_PLAYING);                   /* Allumage du pipeline */
 }
/*--------------------------------------------------------------------------------------------------------*/
