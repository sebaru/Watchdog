/**********************************************************************************************************/
/* Client/liste_histo_msgs.c        Gestion de la page d'affichage des messages au fil de l'eau           */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mer 24 mar 2004 10:06:06 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste_histo_msgs.c
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
 
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern gint Nbr_message;                                                /* Nombre de message de Watchdog */
 extern GtkWidget *Label_message;                                  /* Pour afficher le nombre de messages */

 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 enum
  { COLONNE_ID,
    COLONNE_NUM,
    COLONNE_TYPE,
    COLONNE_DATE_CREATE,
    COLONNE_ACK,
    COLONNE_DATE_FIN,
    COLONNE_DUREE,
    COLONNE_GROUPE_PAGE,
    COLONNE_LIBELLE,
    COLONNE_COULEUR_FOND,
    COLONNE_COULEUR_TEXTE,
    NBR_COLONNE
  };

 extern GdkColor COULEUR_FOND[];
 extern GdkColor COULEUR_TEXTE[];

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"
 
 extern GdkBitmap *Rmask, *Bmask, *Vmask, *Omask, *Jmask;
 extern GdkPixmap *Rouge, *Bleue, *Verte, *Orange, *Jaune;

/* static GnomeUIInfo Menu_popup[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("synoptique..."), NULL, NULL, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };*/

/**********************************************************************************************************/
/* Chercher_histo_msgs: Envoie d'une requete au serveur                                                   */
/* Entrée: un pointeur sur la page en cours                                                               */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Envoi_requete_histo_msgs ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_HISTO_MSGS *infos;
    struct CMD_CRITERE_HISTO_MSGS requete;
    infos = (struct TYPE_INFO_HISTO_MSGS *)page->infos;

    if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_num) ) )               /* Tri par ID */
     { requete.num = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(infos->Spin_num) ); }
    else
     { requete.num = -1; }
    
    if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_type) ) )             /* Tri par ID */
     { requete.type = gtk_option_menu_get_history( GTK_OPTION_MENU(infos->Option_type) ); }
    else
     { requete.type = -1; }

    if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_nom_ack) ) )     /* Tri par nom_ack */
     { g_snprintf( requete.nom_ack, sizeof(requete.nom_ack), "%s",
                   gtk_entry_get_text( GTK_ENTRY(infos->Entry_nom_ack) ) );
     }
    else
     { memset( requete.nom_ack, 0, sizeof(requete.nom_ack) ); }

    if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_libelle) ) )     /* Tri par libelle */
     { g_snprintf( requete.libelle, sizeof(requete.libelle), "%s",
                   gtk_entry_get_text( GTK_ENTRY(infos->Entry_libelle) ) );
     }
    else
     { memset( requete.libelle, 0, sizeof(requete.libelle) ); }
  

    if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_groupage) ) )   /* Tri par groupage */
     { g_snprintf( requete.groupage, sizeof(requete.groupage), "%s",
                   gtk_entry_get_text( GTK_ENTRY(infos->Entry_groupage) ) );
     }
    else
     { memset( requete.groupage, 0, sizeof(requete.groupage) ); }

    if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_debut) ) )         /* Tri par debut */
     { requete.date_create_min = gnome_date_edit_get_time( GNOME_DATE_EDIT(infos->Date_debut) ); }
    else
     { requete.date_create_min=-1; }

    if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_fin) ) )           /* Tri par debut */
     { requete.date_create_max = gnome_date_edit_get_time( GNOME_DATE_EDIT(infos->Date_fin) ); }
    else
     { requete.date_create_max=-1; }

    requete.page_id = infos->page_id;    /* On indique au serveur que l'on veut la reponse sur cette page */ 
    printf("Envoi requete: page_id= %d\n", requete.page_id );
    Envoi_serveur( TAG_HISTO, SSTAG_CLIENT_REQUETE_HISTO_MSGS, (gchar *)&requete,
                   sizeof(struct CMD_CRITERE_HISTO_MSGS) );
 }
/**********************************************************************************************************/
/* Rafraichir_sensibilite: Grise ou non les champs de recherche de l'historique msgs                      */
/* Entrée: un pointeur sur la page en cours                                                               */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Rafraichir_sensibilite ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_HISTO_MSGS *infos;
    gboolean actif;
    infos = (struct TYPE_INFO_HISTO_MSGS *)page->infos;

    actif = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_num) );             /* Tri par ID */
    gtk_widget_set_sensitive( infos->Spin_num, actif );

    actif = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_nom_ack) );   /* Tri par nom ack */
    gtk_widget_set_sensitive( infos->Entry_nom_ack, actif );
    
    actif = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_libelle) );   /* Tri par libelle */
    gtk_widget_set_sensitive( infos->Entry_libelle, actif );

    actif = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_type) );         /* Tri par type */
    gtk_widget_set_sensitive( infos->Option_type, actif );

    actif = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_groupage) ); /* Tri par groupage */
    gtk_widget_set_sensitive( infos->Entry_groupage, actif );

    actif = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_debut) );        /* Tri par date */
    gtk_widget_set_sensitive( infos->Date_debut, actif );

    actif = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(infos->Check_fin) );          /* Tri par date */
    gtk_widget_set_sensitive( infos->Date_fin, actif );
 }
/**********************************************************************************************************/
/* Preparer_requete_histo_msgs: Creation d'un fenetre de choix des criteres de recherches                 */
/* Entrée: un "infos" de type histo_msgs                                                                  */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static gboolean CB_preparer_requete_histo_msgs( GtkDialog *dialog, gint reponse, struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_HISTO_MSGS *infos;
    infos = (struct TYPE_INFO_HISTO_MSGS *)page->infos;

    switch(reponse)
     { case GTK_RESPONSE_OK: Envoi_requete_histo_msgs ( page );
                             break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(infos->F_histo_msgs);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Preparer_requete_histo_msgs: Creation d'un fenetre de choix des criteres de recherches                 */
/* Entrée: un "infos" de type histo_msgs                                                                  */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Preparer_requete_histo_msgs( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_HISTO_MSGS *infos;
    GtkWidget *frame, *hboite, *table, *menu;
    gint cpt;
    infos = (struct TYPE_INFO_HISTO_MSGS *)page->infos;

    infos->F_histo_msgs = gtk_dialog_new_with_buttons( _("Lookin for histo"),
                                                       GTK_WINDOW(F_client),
                                                       GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                       GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                       NULL);
    g_signal_connect( infos->F_histo_msgs, "response",
                      G_CALLBACK(CB_preparer_requete_histo_msgs), page );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(infos->F_histo_msgs)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 8, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    infos->Check_num = gtk_check_button_new_with_label( _("By num") );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Check_num, 0, 1, 0, 1 );
    g_signal_connect_swapped( G_OBJECT(infos->Check_num), "toggled",
                              G_CALLBACK(Rafraichir_sensibilite), page );
    
    infos->Spin_num = gtk_spin_button_new_with_range( 0.0, NBR_BIT_DLS, 1.0 );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Spin_num, 1, 4, 0, 1 );
    
    infos->Check_type = gtk_check_button_new_with_label( _("By type") );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Check_type, 0, 1, 1, 2 );
    g_signal_connect_swapped( G_OBJECT(infos->Check_type), "toggled",
                              G_CALLBACK(Rafraichir_sensibilite), page );

    infos->Option_type = gtk_option_menu_new();
    menu = gtk_menu_new();
    for ( cpt=0; cpt<NBR_TYPE_MSG; cpt++ )
     { gtk_menu_shell_append( GTK_MENU_SHELL(menu),
                              gtk_menu_item_new_with_label( Type_vers_string(cpt) ) );
     }
    gtk_option_menu_set_menu( GTK_OPTION_MENU(infos->Option_type), menu );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Option_type, 1, 4, 1, 2 );

    infos->Check_nom_ack = gtk_check_button_new_with_label( _("By name Ack") );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Check_nom_ack, 0, 1, 2, 3 );
    g_signal_connect_swapped( G_OBJECT(infos->Check_nom_ack), "toggled",
                              G_CALLBACK(Rafraichir_sensibilite), page );

    infos->Entry_nom_ack = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(infos->Entry_nom_ack), NBR_CARAC_LOGIN_UTF8 );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry_nom_ack, 1, 4, 2, 3 );
 
    infos->Check_libelle = gtk_check_button_new_with_label( _("By description") );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Check_libelle, 0, 1, 3, 4 );
    g_signal_connect_swapped( G_OBJECT(infos->Check_libelle), "toggled",
                              G_CALLBACK(Rafraichir_sensibilite), page );

    infos->Entry_libelle = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(infos->Entry_libelle), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry_libelle, 1, 4, 3, 4 );

    infos->Check_groupage = gtk_check_button_new_with_label( _("By groupe/page") );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Check_groupage, 0, 1, 4, 5 );
    g_signal_connect_swapped( G_OBJECT(infos->Check_groupage), "toggled",
                              G_CALLBACK(Rafraichir_sensibilite), page );

    infos->Entry_groupage = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(infos->Entry_groupage), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Entry_groupage, 1, 4, 4, 5 );

    infos->Check_debut = gtk_check_button_new_with_label( _("After") );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Check_debut, 0, 1, 5, 6 );
    g_signal_connect_swapped( G_OBJECT(infos->Check_debut), "toggled",
                              G_CALLBACK(Rafraichir_sensibilite), page );

    infos->Date_debut = gnome_date_edit_new ((time_t) 0, TRUE, TRUE);
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Date_debut, 1, 4, 5, 6 );

    infos->Check_fin = gtk_check_button_new_with_label( _("Before") );
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Check_fin, 0, 1, 6, 7 );
    g_signal_connect_swapped( G_OBJECT(infos->Check_fin), "toggled",
                              G_CALLBACK(Rafraichir_sensibilite), page );

    infos->Date_fin = gnome_date_edit_new ((time_t) 0, TRUE, TRUE);
    gtk_table_attach_defaults( GTK_TABLE(table), infos->Date_fin, 1, 4, 6, 7 );

    Rafraichir_sensibilite ( page );           /* Simule un evenement pour mettre a jour les sensibilites */
    gtk_widget_show_all( infos->F_histo_msgs );
  }
/**********************************************************************************************************/
/* Proto_effacer_histo_msgs: Efface la liste de l'historique msgs                                         */
/* Entrée: un histo_msgs pour referencer la page à effacer                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_effacer_liste_histo_msgs( gint page_id )
  { struct PAGE_NOTEBOOK *page;
    GList *liste;
    
    liste = Liste_pages;                                /* Recherche de la page source dls correspondante */
    while(liste)
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type == TYPE_PAGE_HISTO_MSGS &&
           ((struct TYPE_INFO_HISTO_MSGS *)page->infos)->page_id == page_id) break;
       liste = liste->next;
     }
    if (!liste) return;                                                      /* Si pas trouvé, on s'en va */
    gtk_list_store_clear( GTK_LIST_STORE(((struct TYPE_INFO_HISTO_MSGS *)page->infos)->Liste_histo_msgs) );
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajout d'un message dans la liste des messages à l'écran                           */
/* Entrée: une reference sur le histo_msgs a afficherb                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_histo_msgs( struct CMD_RESPONSE_HISTO_MSGS *response )
  { gchar chaine[50], date[128], ack[128], *date_create, *date_fin, *duree, groupe_page[512];
    struct TYPE_INFO_HISTO_MSGS *infos;
    struct PAGE_NOTEBOOK *page;
    GtkListStore *store;
    GtkTreeIter iter;
    struct tm *temps;
    time_t chrono;
    time_t time;

    page = Chercher_page_notebook( TYPE_PAGE_HISTO_MSGS, response->page_id, FALSE );
    if (!page) return;

    infos = page->infos;
    if (!infos) return;

printf("Proto_afficher_histo_msgs 1\n");
    store = GTK_LIST_STORE( infos->Liste_histo_msgs );
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    
    time = response->histo.date_create_sec;
    temps = localtime( (time_t *)&time );
    if (temps) { strftime( chaine, sizeof(chaine), "%F %T", temps ); }
    else       { g_snprintf( chaine, sizeof(chaine), _("Erreur") ); }
    date_create = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
    g_snprintf( date, sizeof(date), "%s.%03d", date_create, (response->histo.date_create_usec/1000) );
    g_free( date_create );

    if (response->histo.date_fixe)
     { gchar *date_fixe;

       time = response->histo.date_fixe;
       temps = localtime( (time_t *)&time );
       if (temps) { strftime( chaine, sizeof(chaine), "%F %T", temps ); }
       else       { g_snprintf( chaine, sizeof(chaine), _("Erreur") ); }
       date_fixe = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
       g_snprintf( ack, sizeof(ack), "%s (%s)", date_fixe, response->histo.nom_ack );
       g_free( date_fixe );
     }
    else
     { g_snprintf( ack, sizeof(ack), _("no") ); }

    time = response->histo.date_fin;
    temps = localtime( (time_t *)&time );
    if (temps) { strftime( chaine, sizeof(chaine), "%F %T", temps ); }
    else       { g_snprintf( chaine, sizeof(chaine), _("Erreur") ); }
    date_fin = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );

    chrono = (time_t)response->histo.date_fin - (time_t)response->histo.date_create_sec;
    temps = gmtime( &chrono );
    snprintf( chaine, sizeof(chaine), _("%2d day%c %02d:%02d:%02d"),
              temps->tm_yday, (temps->tm_yday>1 ? 's' : ' '),
              temps->tm_hour, temps->tm_min, temps->tm_sec );
    duree = g_strdup( chaine );

    g_snprintf( groupe_page, sizeof(groupe_page), "%s/%s", response->histo.msg.groupe, response->histo.msg.page );

    gtk_list_store_set ( store, &iter,
                         COLONNE_ID, response->histo.id,
                         COLONNE_NUM, response->histo.msg.num,
                         COLONNE_TYPE, Type_vers_string(response->histo.msg.type),
                         COLONNE_DATE_CREATE, date,
                         COLONNE_DATE_FIN, date_fin,
                         COLONNE_DUREE, duree,
                         COLONNE_ACK, ack,
                         COLONNE_GROUPE_PAGE, groupe_page,
                         COLONNE_LIBELLE, response->histo.msg.libelle,
                         COLONNE_COULEUR_FOND, &COULEUR_FOND[response->histo.msg.type],
                         COLONNE_COULEUR_TEXTE, &COULEUR_TEXTE[response->histo.msg.type],
                         -1
                       );
    g_free( date_fin );
    g_free( duree );
  }

/**********************************************************************************************************/
/* Creer_page_message: Creation de la page du notebook consacrée aux messages watchdog                    */
/* Entrée: rien                                                                                           */
/* Sortie: un widget boite                                                                                */
/**********************************************************************************************************/
 void Creer_page_liste_histo_msgs( void )
  { GtkWidget *boite, *scroll, *hboite, *bouton;
    GtkWidget *Liste_histo_msgs;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;
    struct PAGE_NOTEBOOK *page;
    struct TYPE_INFO_HISTO_MSGS *infos;
    static gint page_id = 0;

    page = (struct PAGE_NOTEBOOK *)g_try_malloc0( sizeof(struct PAGE_NOTEBOOK) );
    if (!page) return;
    
    page->infos = (struct TYPE_INFO_HISTO_MSGS *)g_try_malloc0( sizeof(struct TYPE_INFO_HISTO_MSGS) );
    if (!page->infos) { g_free(page); return; }
    infos = (struct TYPE_INFO_HISTO_MSGS *)page->infos;
    infos->page_id = page_id++;

    page->type  = TYPE_PAGE_HISTO_MSGS;
    Liste_pages = g_list_append( Liste_pages, page );

    hboite = gtk_hbox_new( FALSE, 6 );
    page->child = hboite;
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    
/***************************************** La liste des groupes *******************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* id */
                                              G_TYPE_UINT,                                         /* num */
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              GDK_TYPE_COLOR,      /* Couleur de fond de l'enregistrement */
                                              GDK_TYPE_COLOR      /* Couleur de fond de l'enregistrement */
                               );
    ((struct TYPE_INFO_HISTO_MSGS *)page->infos)->Liste_histo_msgs = store;
    Liste_histo_msgs = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );      /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_histo_msgs) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_histo_msgs );

    renderer = gtk_cell_renderer_text_new();                                    /* Colonne de l'id du msg */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("MsgID"), renderer,
                                                         "text", COLONNE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ID);                     /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo_msgs), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Type"), renderer,
                                                         "text", COLONNE_TYPE,
                                                         "background-gdk", COLONNE_COULEUR_FOND,
                                                         "foreground-gdk", COLONNE_COULEUR_TEXTE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_TYPE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo_msgs), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Beginning"), renderer,
                                                         "text", COLONNE_DATE_CREATE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_DATE_CREATE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo_msgs), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("End"), renderer,
                                                         "text", COLONNE_DATE_FIN,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_DATE_FIN);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo_msgs), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Duration"), renderer,
                                                         "text", COLONNE_DUREE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_DUREE);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo_msgs), colonne );

    renderer = gtk_cell_renderer_text_new();                                        /* Colonne du libelle */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Groupe/Page"), renderer,
                                                         "text", COLONNE_GROUPE_PAGE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_GROUPE_PAGE);            /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo_msgs), colonne );

    renderer = gtk_cell_renderer_text_new();                                        /* Colonne du libelle */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Message"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo_msgs), colonne );

    renderer = gtk_cell_renderer_text_new();                                     /* Colonne du synoptique */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("Name Ack"), renderer,
                                                         "text", COLONNE_ACK,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (colonne, COLONNE_ACK);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_histo_msgs), colonne );

#ifdef bouh
    g_signal_connect( G_OBJECT(Liste_histo_msgs), "button_press_event",               /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_histo), NULL );
#endif
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
    
/************************************ Les boutons de controles ********************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Detruire_page), page );

    bouton = gtk_button_new_from_stock( GTK_STOCK_FIND );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Preparer_requete_histo_msgs), page );

    gtk_widget_show_all( hboite );
    gtk_notebook_append_page( GTK_NOTEBOOK(Notebook), hboite, gtk_label_new ( _("Historique") ) );
  }
/*--------------------------------------------------------------------------------------------------------*/
