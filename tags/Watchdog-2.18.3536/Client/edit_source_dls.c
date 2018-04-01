/******************************************************************************************************************************/
/* Client/edit_source_dls.c        Edition des modules D.L.S de watchdog v2.0                                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 30 oct 2004 14:26:13 CEST                     */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * edit_source_dls.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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
 #include <gtksourceview/gtksourceview.h>
 #include <gtksourceview/gtksourceprintcompositor.h>

 #include "Reseaux.h"

/***************************************** Définitions des prototypes programme ***********************************************/
 #include "Config_cli.h"
 #include "client.h"
 #include "protocli.h"

 static GtkWidget *F_send_dls;                                                             /* Fenetre de suivi de progression */
 static GtkWidget *Entry_compil_status;                                                /* Retour d'état de la compilation DLS */

 extern GtkWidget *Notebook;                                                             /* Le Notebook de controle du client */
 extern GList *Liste_pages;                                                       /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */
 extern struct CLIENT Client;                                                                         /* Structure de travail */

/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Proto_afficher_mnemo_dls ( struct CMD_TYPE_MNEMO_BASE *mnemo )
  { struct PAGE_NOTEBOOK *page;
    struct TYPE_INFO_SOURCE_DLS *infos;
    GtkTextBuffer *text_buffer;
    GtkTextIter iter;
    gchar chaine[256];

    snprintf( chaine, sizeof(chaine), " %s <->_%s%04d;  /* %s */\n",
              mnemo->acronyme, Type_bit_interne_court(mnemo->type), mnemo->num,
              mnemo->libelle );                                                              /* Formatage */

    page = Page_actuelle();
    infos = (struct TYPE_INFO_SOURCE_DLS *)page->infos;
    
    text_buffer = GTK_TEXT_BUFFER( infos->text );
    gtk_text_buffer_get_end_iter( text_buffer, &iter );
    gtk_text_buffer_insert_at_cursor( text_buffer, chaine, strlen(chaine) );
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_mnemonique: Fonction appelée qd on appuie sur un des boutons de l'interface          */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_mnemo ( GtkDialog *dialog, gint reponse, struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_SOURCE_DLS *infos;
    infos = (struct TYPE_INFO_SOURCE_DLS *)page->infos;

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { struct CMD_TYPE_NUM_MNEMONIQUE new_mnemo;
               new_mnemo.type = gtk_option_menu_get_history( GTK_OPTION_MENU(infos->Option_type) );
               new_mnemo.num = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(infos->Spin_num) );

                  Envoi_serveur( TAG_DLS, SSTAG_CLIENT_WANT_TYPE_NUM_MNEMO,
                                (gchar *)&new_mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(infos->F_mnemo);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_mnemonique: Ajoute un mnemonique au systeme                                                    */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Menu_ajouter_mnemo ( struct PAGE_NOTEBOOK *page )
  { GtkWidget *frame, *table, *texte, *hboite, *menu;
    struct TYPE_INFO_SOURCE_DLS *infos;
    int cpt;

    infos = (struct TYPE_INFO_SOURCE_DLS *)page->infos;
    infos->F_mnemo = gtk_dialog_new_with_buttons( _("Add a mnemonique"),
                                                  GTK_WINDOW(F_client),
                                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                  GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                  NULL);
    g_signal_connect( infos->F_mnemo, "response",
                      G_CALLBACK(CB_ajouter_mnemo), page );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(infos->F_mnemo)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 4, 1, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    texte = gtk_label_new( _("Type") );    /* Création de l'option menu pour le choix du type de mnemonique */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    infos->Option_type = gtk_option_menu_new();
    menu = gtk_menu_new();
    for ( cpt=0; cpt<NBR_TYPE_MNEMO; cpt++ )
     { gtk_menu_shell_append( GTK_MENU_SHELL(menu),
                              gtk_menu_item_new_with_label( Type_bit_interne(cpt) ) );
     }
    gtk_option_menu_set_menu( GTK_OPTION_MENU(infos->Option_type), menu );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Option_type, 1, 2, 0, 1 );

    texte = gtk_label_new( _("Num") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 0, 1 );
    infos->Spin_num = gtk_spin_button_new_with_range( 0.0, NBR_BIT_DLS, 1.0 );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Spin_num, 3, 4, 0, 1 );

    gtk_widget_show_all( infos->F_mnemo );
  }
/**********************************************************************************************************/
/* Proto_append_source_dls: Ajoute du texte dans le texte view associée au module DLS                     */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Proto_append_source_dls( struct CMD_TYPE_SOURCE_DLS *dls, gchar *buffer )
  { GtkTextBuffer *text_buffer;
    struct TYPE_INFO_SOURCE_DLS *infos;
    struct PAGE_NOTEBOOK *page;
    GtkTextIter iter;

    page = Chercher_page_notebook( TYPE_PAGE_SOURCE_DLS, dls->id, TRUE );         /* Recherche de la page */
    if (!page) return;
    infos = page->infos;
    if (!infos) return;
    text_buffer = GTK_TEXT_BUFFER( infos->text );
    gtk_text_buffer_get_end_iter( text_buffer, &iter );
    gtk_text_buffer_insert( text_buffer, &iter, buffer, dls->taille );
  }
/*****************************************************************************************************************************/
/* CB_valider_source_dls: Fonction appelée le client clique sur le bouton de la fenetre de statut de compilation              */
/* Entrée: la reponse de l'utilisateur                                                                                        */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 void Dls_set_compil_status ( gchar *chaine )
  { gtk_label_set_text ( GTK_LABEL(Entry_compil_status), chaine );
  }
/*****************************************************************************************************************************/
/* CB_valider_source_dls: Fonction appelée le client clique sur le bouton de la fenetre de statut de compilation              */
/* Entrée: la reponse de l'utilisateur                                                                                        */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_valider_source_dls ( GtkDialog *dialog, gint reponse, gpointer data )
  { switch(reponse)
     { case GTK_RESPONSE_OK:
             { break;
             }
       default: break;
     }
    gtk_widget_destroy(F_send_dls);
    F_send_dls = NULL;
    Entry_compil_status = NULL;
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Valider_source_dls: Confirmation de la source DLS et envoie au serveur                                                     */
/* Entrée: la page du notebook en cours d'edition                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Valider_source_dls ( struct PAGE_NOTEBOOK *page )
  { GtkWidget *vboite, *frame, *progress;
    GtkTextBuffer *text_buffer;
    GtkTextIter start, end;
    struct CMD_TYPE_SOURCE_DLS *edit_dls;
    gchar *Source, *Source_fin, *source, *buffer_envoi;
    gint taille_max, taille_source, taille_sent, nb_car;
    
    text_buffer = GTK_TEXT_BUFFER( ((struct TYPE_INFO_SOURCE_DLS *)page->infos)->text );               /* Recup du TextBuffer */
    gtk_text_buffer_get_start_iter( text_buffer, &start );                        /* Recup des iter de debut et fin de buffer */
    gtk_text_buffer_get_end_iter( text_buffer, &end );
    nb_car = gtk_text_buffer_get_char_count( text_buffer );                               /* Nombre de caractere a transférer */

    if (!nb_car) return;                                                              /* On ne compile pas des buffer vides ! */

    F_send_dls = gtk_dialog_new_with_buttons( "Apply a plugin",
                                              GTK_WINDOW(F_client),
                                              GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_STOCK_OK, GTK_RESPONSE_OK,
                                              NULL);
    g_signal_connect( F_send_dls, "response",
                      G_CALLBACK(CB_valider_source_dls), NULL );
    gtk_window_set_default_size(GTK_WINDOW(F_send_dls), 400, 100);

    frame = gtk_frame_new("Live Status");
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_send_dls)->vbox ), frame, TRUE, TRUE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );

/************************************************** Interface de progression **************************************************/
    progress = gtk_progress_bar_new();
    gtk_progress_bar_set_text( GTK_PROGRESS_BAR(progress), "Ready to send" );
    gtk_box_pack_start( GTK_BOX(vboite), progress, FALSE, FALSE, 0 );
    
    Entry_compil_status = gtk_label_new("");
    gtk_box_pack_start( GTK_BOX(vboite), Entry_compil_status, TRUE, TRUE, 0 );

    gtk_widget_show_all( F_send_dls );

    edit_dls = (struct CMD_TYPE_SOURCE_DLS *)g_try_malloc0( Client.connexion->taille_bloc );
    if (!edit_dls)
     { gtk_progress_bar_set_text( GTK_PROGRESS_BAR(progress), "Memory Error !" );
       return;
     }

    Source = gtk_text_buffer_get_text( text_buffer, &start, &end, FALSE );               /* Export du buffer au format chaine */

    buffer_envoi     = (gchar *)edit_dls + sizeof(struct CMD_TYPE_SOURCE_DLS);
    taille_max       = Client.connexion->taille_bloc - sizeof(struct CMD_TYPE_SOURCE_DLS);
    edit_dls->id     = ((struct TYPE_INFO_SOURCE_DLS *)page->infos)->id;
    edit_dls->taille = 0;
                                                                              /* Demande de suppression du fichier source DLS */
    Envoi_serveur( TAG_DLS, SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS_DEB,
                   (gchar *)edit_dls, sizeof(struct CMD_TYPE_SOURCE_DLS) );

    source = Source;
    Source_fin = g_utf8_offset_to_pointer( Source, nb_car );
    taille_sent = 0;
    taille_source = Source_fin - Source;                                                                /* Taill en octets !! */

    while( source != Source_fin )
     { gchar chaine[16];
       gint taille;
       taille = Source_fin-source;                                                                     /* Combien il reste ?? */
       if (taille>taille_max) taille = taille_max;                                   /* Borne supérieure lié au paquet reseau */
       memcpy( buffer_envoi, source, taille );                                                             /* Recopie mémoire */  
       edit_dls->taille = taille;
       if (!Envoi_serveur( TAG_DLS, SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS,
                           (gchar *)edit_dls, taille + sizeof(struct CMD_TYPE_SOURCE_DLS) ))
        { printf("erreur envoi au serveur\n"); }
       printf("Octets sent: %d / %d (taille bloc=%d, taille_max=%d) nb_car=%d\n",
               taille, taille_source, Client.connexion->taille_bloc, taille_max, nb_car );
       source += taille;
       taille_sent+=taille;
       gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR(progress), (gdouble)taille_sent/taille_source );
       g_snprintf( chaine, sizeof(chaine), "%3.1f%%", 100.0*taille_sent/taille_source );
       gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR(progress), (gdouble)taille_sent/taille_source );
       gtk_progress_bar_set_text( GTK_PROGRESS_BAR(progress), chaine );
       gtk_main_iteration_do(FALSE);
     }
    Envoi_serveur( TAG_DLS, SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS_FIN,
                   (gchar *)edit_dls, sizeof(struct CMD_TYPE_SOURCE_DLS) );
    Dls_set_compil_status( "Preparing compilation ... Please Wait ..." );
    g_free(Source);
    g_free(edit_dls);
  }
/**********************************************************************************************************/
/* Menu_exporter_message: Exportation de la base dans un fichier texte                                    */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Menu_exporter_source_dls( struct PAGE_NOTEBOOK *page )
  { GtkSourcePrintCompositor *compositor;
    struct TYPE_INFO_SOURCE_DLS *infos;
    GtkPrintOperation *print;
    GError *error;

    infos = (struct TYPE_INFO_SOURCE_DLS *)page->infos;

    compositor = gtk_source_print_compositor_new ( GTK_SOURCE_BUFFER(infos->text) );
    gtk_source_print_compositor_set_print_line_numbers ( compositor, 5 );
    gtk_source_print_compositor_set_body_font_name ( compositor, "Monospace 10");
    gtk_source_print_compositor_set_print_header ( compositor, TRUE );
    gtk_source_print_compositor_set_header_format ( compositor, TRUE,
                                                    "Module DLS",
                                                    "%F",
                                                    "page %N / %Q");
    gtk_source_print_compositor_set_print_footer ( compositor, TRUE );
    gtk_source_print_compositor_set_footer_format ( compositor, TRUE,
                                                    NULL,
                                                    NULL,
                                                    "Watchdog 2.0 ABLS");
    gtk_source_print_compositor_set_highlight_syntax ( compositor, TRUE );
    print = New_print_job ( "Print Souce D.L.S" );

    g_signal_connect (G_OBJECT(print), "draw-page", G_CALLBACK (Print_draw_page), compositor );
    g_signal_connect (G_OBJECT(print), "paginate",  G_CALLBACK (Print_paginate),  compositor );

    gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   GTK_WINDOW(F_client), &error);
    g_object_unref(compositor);
  }
/**********************************************************************************************************/
/* Menu_source_dls_save_as: Exportation dans un fichier du plugin en cours d'edition                      */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Menu_source_dls_save_as( struct PAGE_NOTEBOOK *page )
 { struct TYPE_INFO_SOURCE_DLS *infos;
   gchar fichier[80];
   GtkWidget *dialog;

   infos = (struct TYPE_INFO_SOURCE_DLS *)page->infos;

   dialog = gtk_file_chooser_dialog_new ( "Save File", GTK_WINDOW(F_client),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                          NULL);
   gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

   g_snprintf( fichier, sizeof(fichier), "%04d-%s.dls", infos->id, infos->plugin_name );
   gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER (dialog), g_get_home_dir() );
   gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER (dialog), fichier );

   if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
     { GtkTextBuffer *text_buffer;
       GtkTextIter start, end;
       gchar *filename, *Source;

       gtk_widget_set_sensitive ( infos->text, FALSE);
       text_buffer = GTK_TEXT_BUFFER( infos->text );
       gtk_text_buffer_get_start_iter( text_buffer, &start );
       gtk_text_buffer_get_end_iter( text_buffer, &end );

       Source = gtk_text_buffer_get_text ( text_buffer, &start, &end, FALSE);       
       gtk_text_buffer_set_modified ( text_buffer, FALSE);

       filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
       g_file_set_contents ( filename, Source, -1, NULL );
       g_free (filename);
       gtk_widget_set_sensitive (infos->text, TRUE);
     }
   gtk_widget_destroy (dialog);
 }
/**********************************************************************************************************/
/* Creer_page_plugin_dls: Creation de la page du notebook consacrée aux plugins plugin_dlss watchdog      */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_page_source_dls( struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { GtkWidget *boite, *scroll, *hboite, *bouton, *separateur, *label;
    struct TYPE_INFO_SOURCE_DLS *infos;
    struct PAGE_NOTEBOOK *page;
    PangoFontDescription *font;
    GtkWidget *text, *buffer;
    gchar titre[NBR_CARAC_PLUGIN_DLS_UTF8 + 10];
    GdkColor color;

    if ( Chercher_page_notebook ( TYPE_PAGE_SOURCE_DLS, rezo_dls->id, TRUE ) ) return; /* Page deja créé ? */

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    page->infos = (struct TYPE_INFO_SOURCE_DLS *)g_try_malloc0( sizeof(struct TYPE_INFO_SOURCE_DLS) );
    if (!page->infos) { g_free(page); return; }
    infos = (struct TYPE_INFO_SOURCE_DLS *)page->infos;
    
    page->type = TYPE_PAGE_SOURCE_DLS;
    infos->id = rezo_dls->id;
    g_snprintf( infos->plugin_name, sizeof(infos->plugin_name), "%s", rezo_dls->nom );
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );                           /* Initialisation des parametres de page */
    page->child = hboite;                                           /* Sauvegarde pour destruction future */
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des plugin_dls ****************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    buffer = (GtkWidget *)gtk_source_buffer_new( NULL );                  /* Création d'un nouveau buffer */
    infos->text = buffer;                                                                   /* Sauvegarde */
    text = gtk_source_view_new_with_buffer( GTK_SOURCE_BUFFER(buffer) );   /* Création de la vue associée */

    gtk_source_view_set_show_line_numbers( (GtkSourceView *)text, TRUE );
    gtk_container_add( GTK_CONTAINER(scroll), text );

  /* Change default font throughout the widget */
    font = pango_font_description_from_string ("Monospace 10");
    gtk_widget_modify_font(text, font);
    pango_font_description_free(font);

/************************************ Les boutons de controles ********************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_APPLY );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Valider_source_dls), page );

    bouton = gtk_button_new_from_stock( GTK_STOCK_PRINT );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_exporter_source_dls), page );

    bouton = gtk_button_new_from_stock( GTK_STOCK_SAVE_AS );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_source_dls_save_as), page );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_ajouter_mnemo), page );

    gtk_widget_show_all( hboite );
    g_snprintf( titre, sizeof(titre), "Dls: %s", rezo_dls->nom );
    label = gtk_event_box_new ();
    gtk_container_add( GTK_CONTAINER(label), gtk_label_new(titre) );
    gdk_color_parse ("red", &color);
    gtk_widget_modify_bg ( label, GTK_STATE_NORMAL, &color );
    gtk_widget_modify_bg ( label, GTK_STATE_ACTIVE, &color );
    gtk_widget_show_all( label );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, label );
  }
/*--------------------------------------------------------------------------------------------------------*/
