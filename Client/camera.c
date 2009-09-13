/**********************************************************************************************************/
/* Client/camera.c               Affichage des cameras de supervision                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 12 sept. 2009 16:53:33 CEST */
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

  GstElement *pipeline, *source, *jpegdec, *ffmpeg, *sink, *playbin;
GtkWidget *video_output;

 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

#ifdef bouh
/**********************************************************************************************************/
/* Detruire_page_supervision: L'utilisateur veut fermer la page de supervision                            */
/* Entrée: la page en question                                                                            */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Detruire_page_camera( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_SUPERVISION *infos;
    infos = (struct TYPE_INFO_SUPERVISION *)page->infos;
    /*g_timeout_remove( infos->timer_id );*/
    Trame_detruire_trame( infos->Trame );
  }
#endif
 void Test2 ( void )
{

  gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (sink), GDK_WINDOW_XWINDOW (video_output->window));

gst_element_set_state (pipeline, GST_STATE_PLAYING);

}
/**********************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Test ( void )
  { GtkWidget *bouton, *boite, *hboite, *scroll, *frame;
    GtkAdjustment *adj;
/*    struct TYPE_INFO_SUPERVISION *infos;*/
    struct PAGE_NOTEBOOK *page;
  GstBus *bus;
GMainLoop *loop;

    page = (struct PAGE_NOTEBOOK *)g_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;

  gst_init (NULL, NULL);

    
/*    page->infos = (struct TYPE_INFO_SUPERVISION *)g_malloc0( sizeof(struct TYPE_INFO_SUPERVISION) );
    infos = (struct TYPE_INFO_SUPERVISION *)page->infos;
    if (!page->infos) { g_free(page); return; }*/

/*    page->type   = TYPE_PAGE_SUPERVISION;*/
    Liste_pages  = g_list_append( Liste_pages, page );
/*    infos->syn_id = syn_id;*/

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );

    video_output = gtk_drawing_area_new ();
    gtk_box_pack_start (GTK_BOX (hboite), video_output, TRUE, TRUE, 0);
    gtk_widget_set_size_request (video_output, 640, 480);


/**************************************** Trame proprement dite *******************************************/
    
  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("camera-player");
printf("pipeline = %p\n", pipeline );
/*loop = g_main_loop_new (NULL, FALSE);*/

/*playbin = gst_element_factory_make("playbin", "playbin");*/
/*sink = gst_element_factory_make("ximagesink", "vsink");*/
/*g_object_set(G_OBJECT(playbin), "video-sink", sink, NULL);
g_object_set(G_OBJECT(playbin), "uri", "file:///mnt/SalleCTI-storage/Partage/Videos-clips/Ace_of_base_-_Always21.avi", NULL);
/* 
 http://guest:guest@192.168.0.30/cgi/mjpg/mjpg.cgi
file:///mnt/SalleCTI-storage/Partage/Videos-clips/Ace_of_base_-_Always21.avi 
*/

  source  = gst_element_factory_make ( "gnomevfssrc", NULL );
  g_object_set (G_OBJECT (source), "location", "http://guest:guest@192.168.0.30/cgi/mjpg/mjpg.cgi", NULL);

  jpegdec  = gst_element_factory_make ("jpegdec", NULL);
  ffmpeg   = gst_element_factory_make ("ffmpegcolorspace", NULL );
  sink    = gst_element_factory_make ( "ximagesink", NULL );


/* g_object_set (G_OBJECT (sink), "force-aspect-ratio", TRUE, NULL);*/


 gst_bin_add_many (GST_BIN (pipeline), source, jpegdec, ffmpeg, sink, NULL);
 gst_element_link_many (source, jpegdec, ffmpeg, sink, NULL);

/* gst_x_overlay_handle_events (GST_X_OVERLAY (sink), FALSE);*/

 gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), page->child, gtk_label_new ( "test camera" ) );
    gtk_widget_show_all( page->child );

 
/* gst_element_set_state (playbin, GST_STATE_PLAYING);*/

  /* Set up the pipeline */

  /* we set the input filename to the source element */

  /* we add a message handler */
/*  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);*/

  /* we add all elements into the pipeline */
  /* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */
/*  gst_bin_add_many (GST_BIN (pipeline),
                    source, jpegdec, ffmpeg, sink, NULL);

  /* we link the elements together */
  /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
/*  gst_element_link_many (source, jpegdec, ffmpeg, sink, NULL);
/*  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), decoder);*/

  /* note that the demuxer will be linked to the decoder dynamically.
     The reason is that Ogg may contain various streams (for example
     audio and video). The source pad(s) will be created at run time,
     by the demuxer when it detects the amount and nature of streams.
     Therefore we connect a callback function which will be executed
     when the "pad-added" is emitted.*/


  /* Set the pipeline to "playing" state*/
 
/*g_main_loop_run (loop);*/

/**************************************** Boutons de controle *********************************************/
/*    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );

/*    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_synoptique), infos );*/

 /*   infos->timer_id = g_timeout_add( 500, Timer, NULL );*/
  }
/*--------------------------------------------------------------------------------------------------------*/
