/******************************************************************************************************************************/
/* Client/liste_message.c        Gestion de la page d'affichage des messages au fil de l'eau                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          mer 20 aoû 2003 18:19:00 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * liste_histo.c
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
 #include <time.h>

 #include "Config_cli.h"
 #include "Reseaux.h"
 
 GtkWidget *Liste_histo;                                                 /* GtkTreeView pour la gestion des messages Watchdog */
 extern GList *Liste_pages;                                                       /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                                             /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG Config;                                                              /* Configuration generale watchdog */
 extern struct CLIENT Client;                                                        /* Identifiant de l'utilisateur en cours */

 enum
  { COLONNE_ID,
    COLONNE_NUM,
    COLONNE_GROUPE_PAGE,
    COLONNE_TYPE,
    COLONNE_SYN_ID,
    COLONNE_DATE_CREATE,
    COLONNE_DLS_SHORTNAME,
    COLONNE_ACK,
    COLONNE_LIBELLE,
    COLONNE_COULEUR_FOND,
    COLONNE_COULEUR_TEXTE,
    NBR_COLONNE
  };
 GdkColor COULEUR_FOND[]=
  { { 0x0, 0xAFFF, 0xAFFF, 0xAFFF }, /* Info */
    { 0x0, 0x7FFF, 0x0,    0x0    }, /* Alerte */
    { 0x0, 0xFFFF, 0xFFFF, 0x0    }, /* Trouble */
    { 0x0, 0xFFFF, 0x0,    0x0    }, /* Alarme */
    { 0x0, 0x0,    0xFFFF, 0x0    }, /* Veille */
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF }, /* Attente */
    { 0x0, 0x0   , 0x0   , 0xFFFF }  /* Danger */
  };
 GdkColor COULEUR_TEXTE[]=
  { { 0x0, 0x0,    0x0,    0x0    }, /* Info */
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF }, /* Alerte */
    { 0x0, 0x0,    0x0,    0x0    }, /* Trouble */
    { 0x0, 0x0,    0x0,    0x0    }, /* Alarme */
    { 0x0, 0x0,    0x0,    0x0    }, /* Veille */
    { 0x0, 0x0,    0x0,    0x0    }, /* Attente */
    { 0x0, 0xFFFF, 0xFFFF, 0xFFFF }  /* Danger */
  };
/**************************************** Définitions des prototypes programme ************************************************/
 #include "protocli.h"
 #include "client.h"
 
 extern struct CLIENT Client;                                                        /* Identifiant de l'utilisateur en cours */
 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */

 static void Menu_acquitter_histo ( void );
 static void Menu_go_to_syn ( void );

 static GnomeUIInfo Menu_popup[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Acknowledge"), NULL, Menu_acquitter_histo, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Go to Syn"), NULL, Menu_go_to_syn, GNOME_STOCK_PIXMAP_SEARCH ),
    GNOMEUIINFO_END
  };

/******************************************************************************************************************************/
/* Jouer_remote_mp3 : Joue un fichier mp3 du serveur                                                                          */
/* Entrée : le message à jouer au format file, ou id si file=NULL                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Jouer_remote_mp3 ( gchar *file, gint id )
  { gchar url[256];
    gint pid;

    if (file)
     { g_snprintf( url, sizeof(url), "http://%s/ws/audio/%s", Client.host, file ); }
    else
     { g_snprintf( url, sizeof(url), "http://%s/ws/audio/%d", Client.host, id ); }

    pid = fork();
    if (pid<0)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                 "Jouer_remote_mp3: '%s' fork failed pid=%d (%s)", url, id, pid, strerror(errno) );
     }
    else if (!pid)
     { execlp( "mpg123", "mpg123", "-v", url, NULL );
       Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                "Jouer_remote_mp3: '%s' exec failed pid=%d (%s)", url, pid, strerror( errno ) );
       _exit(0);
     }
  }
/**********************************************************************************************************/
/* Menu_acquitter_histo: Acquittement d'un des messages histo                                             */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_acquitter_histo ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_HISTO histo;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_histo) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_histo) );

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { gtk_tree_model_get_iter( store, &iter, lignes->data );          /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_ID, &histo.id, -1 );                      /* Recup du id */
       gtk_tree_model_get( store, &iter, COLONNE_NUM, &histo.msg.num, -1 );               /* Recup du num */

       Envoi_serveur( TAG_HISTO, SSTAG_CLIENT_ACK_HISTO, (gchar *)&histo, sizeof(struct CMD_TYPE_HISTO) );
       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Menu_go_to_syn: Affiche les synoptiques associés aux messages histo                                    */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_go_to_syn ( void )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_histo) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_histo) );

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    while ( lignes )
     { guint id_syn;
       gtk_tree_model_get_iter( store, &iter, lignes->data );          /* Recuperation ligne selectionnée */
       gtk_tree_model_get( store, &iter, COLONNE_SYN_ID, &id_syn, -1 );                  /* Recup du id */

       Changer_vue_directe ( id_syn );

       gtk_tree_selection_unselect_iter( selection, &iter );
       lignes = lignes->next;
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Gerer_popup_message: Gestion du menu popup quand on clique droite sur la liste des messages            */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_histo ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { static GtkWidget *Popup=NULL;
    GtkTreeSelection *selection;
    gboolean ya_selection;
    GtkTreePath *path;
    gint cellx, celly;
    if (!event) return(FALSE);

    if (!Popup)    Popup = gnome_popup_menu_new( Menu_popup );                        /*Creation si besoin*/
    ya_selection = FALSE;
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_histo) );    /* On recupere la selection */
    if (gtk_tree_selection_count_selected_rows(selection) == 0)
     { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_histo), event->x, event->y,
                                       &path, NULL, &cellx, &celly );
          
       if (path)
        { gtk_tree_selection_select_path( selection, path );
          gtk_tree_path_free( path );
          ya_selection = TRUE;
        }
     } else ya_selection = TRUE;                                 /* ya bel et bien qqchose de selectionné */

    if ( event->button == 3 && ya_selection )                                         /* Gestion du popup */
     { gnome_popup_menu_do_popup_modal( Popup, NULL, NULL, event, NULL, F_client );
       return(TRUE);
     }
    else if (event->type == GDK_2BUTTON_PRESS && event->button == 1 )                   /* Double clic ?? */
     { Menu_go_to_syn();
       return(TRUE);
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Rafraichir_visu_message: Rafraichissement du message à la position iter de la liste                    */
/* Entrée: une reference sur l'utilisateur                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_histo( GtkTreeIter *iter, struct CMD_TYPE_HISTO *histo )
  { GtkTreeModel *store;
    gchar chaine[128], date[128], ack[128], *date_create, groupe_page[512];
    struct tm *temps;
    time_t time;

    time = histo->date_create_sec;
    temps = localtime( (time_t *)&time );
    if (temps) { strftime( chaine, sizeof(chaine), "%F %T", temps ); }
    else       { g_snprintf( chaine, sizeof(chaine), _("Erreur") ); }
    date_create = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );

    g_snprintf( date, sizeof(date), "%s.%03d", date_create, ((int)histo->date_create_usec/1000) );
    g_free( date_create );

    g_snprintf( groupe_page, sizeof(groupe_page), "%s/%s", histo->msg.syn_groupe, histo->msg.syn_page );

    if (histo->date_fixe)
     { gchar *date_fixe;

       time = histo->date_fixe;
       temps = localtime( (time_t *)&time );
       if (temps) { strftime( chaine, sizeof(chaine), "%F %T", temps ); }
       else       { g_snprintf( chaine, sizeof(chaine), _("Erreur") ); }
       date_fixe = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );

       g_snprintf( ack, sizeof(ack), "%s (%s)", date_fixe, histo->nom_ack );
       g_free( date_fixe );
     }
    else
     { g_snprintf( ack, sizeof(ack), "(%s)", histo->nom_ack ); }

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_histo) );               /* Acquisition du modele */
    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, histo->id,
                         COLONNE_NUM, histo->msg.num,
                         COLONNE_SYN_ID, histo->msg.syn_id,
                         COLONNE_GROUPE_PAGE, groupe_page,
                         COLONNE_TYPE, Type_vers_string(histo->msg.type),
                         COLONNE_DATE_CREATE, date,
                         COLONNE_DLS_SHORTNAME, histo->msg.dls_shortname,
                         COLONNE_ACK, ack,
                         COLONNE_LIBELLE, histo->msg.libelle,
                         COLONNE_COULEUR_FOND, &COULEUR_FOND[histo->msg.type],
                         COLONNE_COULEUR_TEXTE, &COULEUR_TEXTE[histo->msg.type],
                         -1
                       );
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_histo: Rafraichissement de l'histo en parametre                                     */
/* Entrée: une reference sur le groupe                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_rafraichir_un_histo( struct CMD_TYPE_HISTO *histo )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    guint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_histo) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )                                                    /* A la recherche de l'iter perdu */
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == histo->id )
        { Rafraichir_visu_histo( &iter, histo );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajout d'un message dans la liste des messages à l'écran                           */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_histo( struct CMD_TYPE_HISTO *histo )
  { GtkListStore *store;
    GtkTreePath *path;
    GtkTreeIter iter;

    if (!Tester_page_notebook(TYPE_PAGE_HISTO))
     { printf("Creation page histo\n");
       Creer_page_histo();
       printf("Fin Creation page histo\n");
     }

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_histo) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_histo ( &iter, histo );
    path = gtk_tree_model_get_path ( GTK_TREE_MODEL(store), &iter );
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW(Liste_histo), path, NULL, FALSE, 0.0, 0.0 );
    gtk_tree_path_free( path );
  }
/**********************************************************************************************************/
/* Cacher_un_utilisateur: Enleve un groupe de la liste des utilisateurs                                   */
/* Entrée: une reference sur l'utilisateur                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_histo( struct CMD_TYPE_HISTO *histo )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    guint num;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_histo) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_NUM, &num, -1 );
/* printf("Del_histo: id = %d, cible = %d\n", id, histo->id); */
       if ( num == histo->msg.num ) 
        { if (gtk_list_store_remove( GTK_LIST_STORE(store), &iter )) continue; }
       valide = gtk_tree_model_iter_next( store, &iter );
     }
  }
/**********************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: un widget boite                                                                                */
/**********************************************************************************************************/
 void Reset_page_histo( void )
  { GtkTreeModel *store;
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_histo) );
    gtk_list_store_clear( GTK_LIST_STORE(store) ); 
  }
/**********************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: un widget boite                                                                                */
/**********************************************************************************************************/
 void Creer_page_histo( void )
  { GtkWidget *boite, *scroll, *hboite;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) { printf("Creer_page_histo: page = NULL !\n"); return; }
    
    page->type  = TYPE_PAGE_HISTO;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des groupes *******************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* ID */
                                              G_TYPE_UINT,                                         /* NUM */
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,                               /* Groupe page */
                                              G_TYPE_UINT,                                     /* Num_syn */
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,                             /* DLS Shortname */
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              GDK_TYPE_COLOR,      /* Couleur de fond de l'enregistrement */
                                              GDK_TYPE_COLOR      /* Couleur du texte de l'enregistrement */
                               );
    Liste_histo = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );           /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_histo) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_histo );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne de l'id du msg */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Num"), renderer,
                                                         "text", COLONNE_NUM,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NUM);                    /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Groupe/Page"), renderer,
                                                         "text", COLONNE_GROUPE_PAGE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_GROUPE_PAGE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Type"), renderer,
                                                         "text", COLONNE_TYPE,
                                                         "background-gdk", COLONNE_COULEUR_FOND,
                                                         "foreground-gdk", COLONNE_COULEUR_TEXTE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_TYPE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Timestamp"), renderer,
                                                         "text", COLONNE_DATE_CREATE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_DATE_CREATE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("DLS Shortname"), renderer,
                                                         "text", COLONNE_DLS_SHORTNAME,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_DLS_SHORTNAME);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Ack"), renderer,
                                                         "text", COLONNE_ACK,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_ACK);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo), colonne );

    renderer = gtk_cell_renderer_text_new();                                        /* Colonne du libelle */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Message"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo), colonne );

    g_signal_connect( G_OBJECT(Liste_histo), "button_press_event",               /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_histo), NULL );
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
    
/************************************ Les boutons de controles ********************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

/*    bouton = Bobouton( Verte, Vmask, _("Acknowledge") );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect( G_OBJECT(bouton), "clicked",
                      G_CALLBACK(Menu_acquitter_histo), NULL );
*/
    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Messages") ) );
  }
/*--------------------------------------------------------------------------------------------------------*/
