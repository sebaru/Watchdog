/******************************************************************************************************************************/
/* Client/atelier_ajout_motif.c         gestion des ajouts de motifs à la trame                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 08 mai 2004 11:13:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_ajout_motif.c
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

 #include <gtk/gtk.h>
 enum
  {  COLONNE_CLASSE_ID,
     COLONNE_CLASSE_LIBELLE,
     NBR_COLONNE_CLASSE
  };

 enum
  {  COLONNE_ICONE_ID,
     COLONNE_ICONE_LIBELLE,
     NBR_COLONNE
  };

/***************************************** Définitions des prototypes programme ***********************************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Atelier_Afficher_un_motif: Ajoute un motif sur la page atelier                                                             */
/* Entrée: les données JSON recu de la requete HTTP                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Afficher_un_motif (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct PAGE_NOTEBOOK *page = user_data;
    struct TRAME_ITEM_MOTIF *trame_motif;
    struct CMD_TYPE_MOTIF *motif;

    if (!page) return;

    motif = (struct CMD_TYPE_MOTIF *)g_try_malloc0( sizeof(struct CMD_TYPE_MOTIF) );
    if (!motif) return;

    motif->position_x   = Json_get_int ( element, "posx" );
    motif->position_y   = Json_get_int ( element, "posy" );
    motif->largeur      = Json_get_int ( element, "larg" );
    motif->hauteur      = Json_get_int ( element, "haut" );
    motif->angle        = Json_get_int ( element, "angle" );
    motif->icone_id     = Json_get_int ( element, "icone" );
    motif->type_dialog  = Json_get_int ( element, "dialog" );
    motif->type_gestion = Json_get_int ( element, "gestion" );
    motif->layer        = Json_get_int ( element, "layer" );
    g_snprintf( motif->def_color,     sizeof(motif->def_color),     "%s", Json_get_string( element, "def_color" ) );
    g_snprintf( motif->tech_id,       sizeof(motif->tech_id),       "%s", Json_get_string( element, "tech_id" ) );
    g_snprintf( motif->acronyme,      sizeof(motif->acronyme),      "%s", Json_get_string( element, "acronyme" ) );
    g_snprintf( motif->clic_tech_id,  sizeof(motif->clic_tech_id),  "%s", Json_get_string( element, "clic_tech_id" ) );
    g_snprintf( motif->clic_acronyme, sizeof(motif->clic_acronyme), "%s", Json_get_string( element, "clic_acronyme" ) );
    g_snprintf( motif->libelle,       sizeof(motif->libelle),       "%s", Json_get_string( element, "libelle" ) );
    motif->access_level = Json_get_int ( element, "access_level" );
    motif->rafraich     = Json_get_int ( element, "rafraich" );

    if (page->type == TYPE_PAGE_SUPERVISION)
     { struct TYPE_INFO_SUPERVISION *infos=page->infos;
       trame_motif = Trame_ajout_motif ( FALSE, infos->Trame, motif );
       if (!trame_motif)
        { printf("Erreur creation d'un nouveau motif\n");
          return;                                                          /* Ajout d'un test anti seg-fault */
        }
       g_snprintf( trame_motif->color, sizeof(trame_motif->color), "%s", motif->def_color );
       trame_motif->mode   = 0;                                                     /* Sauvegarde etat motif */
       trame_motif->cligno = 0;                                                     /* Sauvegarde etat motif */
       g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-press-event",
                         G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-release-event",
                         G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
       Updater_un_visuel ( trame_motif, element );
     }
    else if (page->type == TYPE_PAGE_ATELIER)
     { struct TYPE_INFO_ATELIER *infos=page->infos;
       trame_motif = Trame_ajout_motif ( TRUE, infos->Trame_atelier, motif );
       if (!trame_motif) { g_free(motif); return; }                                                            /* Si probleme */
       trame_motif->layer = infos->new_layer++;
       g_signal_connect( G_OBJECT(trame_motif->item), "button-press-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "button-release-event", G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "enter-notify-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "leave-notify-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "motion-notify-event",  G_CALLBACK(Clic_sur_motif), trame_motif );

       g_signal_connect( G_OBJECT(trame_motif->select_hg), "button-press-event",   G_CALLBACK(Agrandir_hg), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_hg), "button-release-event", G_CALLBACK(Agrandir_hg), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_hg), "motion-notify-event",  G_CALLBACK(Agrandir_hg), trame_motif );

       g_signal_connect( G_OBJECT(trame_motif->select_hd), "button-press-event",   G_CALLBACK(Agrandir_hd), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_hd), "button-release-event", G_CALLBACK(Agrandir_hd), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_hd), "motion-notify-event",  G_CALLBACK(Agrandir_hd), trame_motif );

       g_signal_connect( G_OBJECT(trame_motif->select_bg), "button-press-event",   G_CALLBACK(Agrandir_bg), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_bg), "button-release-event", G_CALLBACK(Agrandir_bg), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_bg), "motion-notify-event",  G_CALLBACK(Agrandir_bg), trame_motif );

       g_signal_connect( G_OBJECT(trame_motif->select_bd), "button-press-event",   G_CALLBACK(Agrandir_bd), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_bd), "button-release-event", G_CALLBACK(Agrandir_bd), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_bd), "motion-notify-event",  G_CALLBACK(Agrandir_bd), trame_motif );

     }
  }
#ifdef bouh
/**********************************************************************************************************/
/* Menu_editer_icone: Demande d'edition du icone selectionné                                              */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Clic_icone_atelier ( void )
  { GtkTreeSelection *selection;
    gchar *icone_id,*libelle;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_icone) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_icone) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ICONE_ID, &icone_id,
                                      COLONNE_ICONE_LIBELLE, &libelle, -1 );                                   /* Recup du id */

    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                                               /* Liberation mémoire */

    memset( &Motif_preview0, 0, sizeof(struct CMD_TYPE_MOTIF) );
    memcpy( &Motif_preview0.libelle, libelle, sizeof( Motif_preview0.libelle ) );
    Motif_preview0.icone_id = atoi(icone_id);
    Motif_preview0.position_x = TAILLE_ICONE_X/2;
    Motif_preview0.position_y = TAILLE_ICONE_Y/2;
    Motif_preview0.rouge0 = 0;
    Motif_preview0.vert0  = 255;
    Motif_preview0.bleu0  = 0;
    g_free(icone_id);
    g_free(libelle);

    if (Trame_motif_p0)
     { goo_canvas_item_remove( Trame_motif_p0->item_groupe );
       Trame_preview0->trame_items = g_list_remove( Trame_preview0->trame_items,
                                                    Trame_motif_p0 );
       g_object_unref( Trame_motif_p0->pixbuf );
       g_free(Trame_motif_p0);
     }
                                                                                   /* Affichage à l'ecran */
    Trame_motif_p0 = Trame_ajout_motif( TRUE, Trame_preview0, &Motif_preview0 );
    Reduire_en_vignette( &Motif_preview0 );
    Trame_rafraichir_motif ( Trame_motif_p0 );
  }
/******************************************************************************************************************************/
/* CB_Receive_package_data : Recoit une partie du fichier package                                                             */
/* Entrée : Les informations à sauvegarder                                                                                    */
/******************************************************************************************************************************/
 static size_t CB_Receive_package_data( char *ptr, size_t size, size_t nmemb, void *userdata )
  { gchar *new_buffer;
    Info_new( Config_cli.log, FALSE, LOG_DEBUG,
              "%s: Récupération de %d*%d octets depuis le cloud (actual size=%d)", __func__, size, nmemb, Package_received_size );
    new_buffer = g_try_realloc ( Package_received_buffer, Package_received_size + size*nmemb + 1);
    if (!new_buffer)                                                                     /* Si erreur, on arrete le transfert */
     { Info_new( Config_cli.log, FALSE, LOG_ERR,
                "%s: Memory Error realloc (%s).", __func__, strerror(errno) );
       g_free(Package_received_buffer);
       Package_received_buffer = NULL;
       return(-1);
     } else Package_received_buffer = new_buffer;
    memcpy( Package_received_buffer + Package_received_size, ptr, size*nmemb );
    Package_received_size += size*nmemb;
    Package_received_buffer[Package_received_size] = 0;                                              /* Caractere nul d'arret */
    return(size*nmemb);
  }
/******************************************************************************************************************************/
/* Remplir_liste_icone: envoie une requete dans le cloud pour récupérer la liste des icones de classe en parametre            */
/* Entrée / Sortie : Rien                                                                                                     */
/******************************************************************************************************************************/
 static void Remplir_liste_icone ( gchar *classe_id )
  { gchar erreur[CURL_ERROR_SIZE+1], url[256];
    gint http_response, res;
    GtkTreeModel *store;
    CURL *curl;

    Package_received_buffer = NULL;                                                     /* Init du tampon de reception à NULL */
    Package_received_size = 0;                                                          /* Init du tampon de reception à NULL */
    http_response = 0;

    curl = curl_easy_init();
    if (!curl)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR, "%s: cURL init failed", __func__ );
       return;
     }

    g_snprintf( url, sizeof(url), "https://icons.abls-habitat.fr/icons/icon_list/%s", classe_id );
    Info_new( Config_cli.log, Config_cli.log_override, LOG_DEBUG, "%s: Trying to get %s", __func__, url );

    curl_easy_setopt(curl, CURLOPT_URL, url );
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, erreur );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1 );
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CB_Receive_package_data );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, Config_cli.log_override );
    curl_easy_setopt(curl, CURLOPT_USERAGENT, WATCHDOG_USER_AGENT );
    res = curl_easy_perform(curl);
    if (res)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING,
                "%s: Could not connect to %s (%s)", __func__, url, erreur);
     }
    else if (curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &http_response ) != CURLE_OK)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING, "%s: Wrong Response code for %s", __func__, url );
     }
    else
     { JsonNode *Response;
       JsonArray *Icones;
       gint cpt;
       Response = json_from_string ( Package_received_buffer, NULL );
       if(!Response)
        { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR, "%s: Wrong Response JSON for %s", __func__, url );
          return;
        }

       store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_icone) );
       Icones = json_node_get_array (Response);
       for (cpt=0; cpt < json_array_get_length ( Icones ); cpt++ )
        { GtkTreeIter iter;
          JsonNode *icone_node = json_array_get_element ( Icones, cpt );
          JsonArray *icone     = json_node_get_array(icone_node);
          const gchar *icone_id      = json_node_get_string ( json_array_get_element(icone, 0) );
          const gchar *icone_libelle = json_node_get_string ( json_array_get_element(icone, 1) );
          gtk_list_store_append ( GTK_LIST_STORE(store), &iter );                                    /* Acquisition iterateur */
          gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                               COLONNE_ICONE_ID, icone_id,
                               COLONNE_ICONE_LIBELLE, icone_libelle,
                               -1
                             );
        }
       json_node_unref(Response);
       g_free(Package_received_buffer);                                                           /* On libere le tampon reçu */
       Package_received_buffer = NULL;
     }
    curl_easy_cleanup(curl);
  }
/******************************************************************************************************************************/
/* Clic_classe_atelier: fonction appelée quand l'utilisateur appuie sur une des classes                                       */
/* Entrée: rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Clic_classe_atelier ( void )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gchar *classe_id;
    GList *lignes;
    guint nbr;

    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_icone) );
    gtk_list_store_clear( GTK_LIST_STORE(store) );

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_classe) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_classe) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                                            /* Si rien n'est selectionné */

    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );                                 /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_CLASSE_ID, &classe_id, -1 );                                     /* Recup du id */
    Remplir_liste_icone ( classe_id );
    g_free(classe_id);
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                                               /* Liberation mémoire */

    if (Trame_motif_p0)                                                                    /* On efface la previsu de l'icone */
     { goo_canvas_item_remove( Trame_motif_p0->item_groupe );
       Trame_preview0->trame_items = g_list_remove( Trame_preview0->trame_items,
                                                           Trame_motif_p0 );
       g_object_unref( Trame_motif_p0->pixbuf );
       g_free(Trame_motif_p0);
       Trame_motif_p0 = NULL;
     }
  }
/**********************************************************************************************************/
/* Choisir_motif_a_ajouter: Affichage de la fenetre de choix du motif a ajouter                           */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Choisir_motif_a_ajouter ( void )
  { if (!F_ajout_motif) Creer_fenetre_ajout_motif ();
    Clic_classe_atelier();
    gtk_widget_show_all( F_ajout_motif );
  }
/**********************************************************************************************************/
/* Detruire_page_propriete_TOR: Destruction de la page de parametres DLS                                  */
/* Entrée: rien                                                                                           */
/* Sortie: toute trace de la fenetre est eliminée                                                         */
/**********************************************************************************************************/
 void Detruire_fenetre_ajout_motif ( void )
  { if (Trame_preview0) { Trame_detruire_trame( Trame_preview0 );
                          Trame_preview0 = NULL;
                        }
    if (F_ajout_motif) gtk_widget_destroy( F_ajout_motif );
    Trame_motif_p0 = NULL;
    F_ajout_motif = NULL;
  }
/**********************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajout_motif ( GtkDialog *dialog, gint reponse )
  { struct CMD_TYPE_MOTIF add_motif;
    struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return(TRUE);         /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    switch(reponse)
     { case GTK_RESPONSE_OK:     if (!Trame_motif_p0) return(TRUE);
                                 memset ( &add_motif, 0, sizeof(add_motif) );
                                 add_motif.icone_id = Trame_motif_p0->motif->icone_id;/* Correspond au .gif*/
                                 g_snprintf( add_motif.libelle, sizeof(add_motif.libelle),
                                             "%s" , Motif_preview0.libelle );
                                 g_snprintf( add_motif.clic_tech_id, sizeof(add_motif.clic_tech_id), "NONE" );
                                 g_snprintf( add_motif.clic_acronyme, sizeof(add_motif.clic_acronyme), "NONE" );
                                 add_motif.syn_id = infos->syn.id;
                                 add_motif.access_level = 0;     /* Nom du groupe d'appartenance du motif */
                                 add_motif.bit_controle = 0;                                /* Ixxx, Cxxx */
                                 add_motif.angle = 0.0; /*infos->Adj_angle->value;*/
                                 add_motif.type_dialog = 0;               /* Type de la boite de dialogue */
                                 add_motif.type_gestion = 0;
                                 /*add_motif.position_x et posy positionné par le serveur */
                                 add_motif.largeur = Trame_motif_p0->gif_largeur;
                                 add_motif.hauteur = Trame_motif_p0->gif_hauteur;
                                 add_motif.rouge0 = 255;
                                 add_motif.vert0 = 255;
                                 add_motif.bleu0 = 255;
                                 Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_MOTIF,
                                                (gchar *)&add_motif, sizeof(struct CMD_TYPE_MOTIF) );
                                 printf("Requete envoyée au serveur....\n");
                                 return(TRUE);                            /* On laisse la fenetre ouverte */
                                 break;
       case GTK_RESPONSE_CLOSE: break;
     }
    gtk_widget_hide( F_ajout_motif );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Remplir_liste_classe: envoie une requete dans le cloud pour récupérer la liste des classes                                 */
/* Entrée / Sortie : Rien                                                                                                     */
/******************************************************************************************************************************/
 static void Remplir_liste_classe ( void )
  { gchar erreur[CURL_ERROR_SIZE+1], url[256];
    gint http_response, res;
    GtkTreeModel *store;
    CURL *curl;

    Package_received_buffer = NULL;                                                     /* Init du tampon de reception à NULL */
    Package_received_size = 0;                                                          /* Init du tampon de reception à NULL */
    http_response = 0;

    curl = curl_easy_init();
    if (!curl)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR, "%s: cURL init failed", __func__ );
       return;
     }

    g_snprintf( url, sizeof(url), "https://icons.abls-habitat.fr/icons/class_list" );
    Info_new( Config_cli.log, Config_cli.log_override, LOG_DEBUG, "%s: Trying to get %s", __func__, url );

    curl_easy_setopt(curl, CURLOPT_URL, url );
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, erreur );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1 );
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CB_Receive_package_data );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, Config_cli.log_override );
    curl_easy_setopt(curl, CURLOPT_USERAGENT, WATCHDOG_USER_AGENT );
    res = curl_easy_perform(curl);
    if (res)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING,
                "%s: Could not connect to %s (%s)", __func__, url, erreur);
     }
    else if (curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &http_response ) != CURLE_OK)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING, "%s: Wrong Response code for %s", __func__, url );
     }
    else
     { JsonNode *Response;
       JsonArray *Classes;
       gint cpt;
       Response = json_from_string ( Package_received_buffer, NULL );
       if(!Response)
        { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR, "%s: Wrong Response JSON for %s", __func__, url );
          return;
        }

       store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_classe) );
       Classes = json_node_get_array (Response);
       for (cpt=0; cpt < json_array_get_length ( Classes ); cpt++ )
        { GtkTreeIter iter;
          JsonNode *classe_node = json_array_get_element ( Classes, cpt );
          JsonArray *classe     = json_node_get_array(classe_node);
          const gchar *classe_id      = json_node_get_string ( json_array_get_element(classe, 0) );
          const gchar *classe_libelle = json_node_get_string ( json_array_get_element(classe, 1) );
          gtk_list_store_append ( GTK_LIST_STORE(store), &iter );                      /* Acquisition iterateur */
          gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                               COLONNE_CLASSE_ID, classe_id,
                               COLONNE_CLASSE_LIBELLE, classe_libelle,
                               -1
                             );
        }
       json_node_unref(Response);
       g_free(Package_received_buffer);                                                           /* On libere le tampon reçu */
       Package_received_buffer = NULL;
     }
    curl_easy_cleanup(curl);
  }
/**********************************************************************************************************/
/* Creer_page_propriete_TOR: Creation de la fenetre d'edition des proprietes TOR                          */
/* Entrée: niet                                                                                           */
/* Sortie: niet                                                                                           */
/**********************************************************************************************************/
 void Creer_fenetre_ajout_motif ( void )
  { GtkWidget *table, *scroll, *hboite, *vboite;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;

    F_ajout_motif = gtk_dialog_new_with_buttons( _("Add a icon"),
                                                 GTK_WINDOW(F_client),
                                                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                                 GTK_STOCK_ADD, GTK_RESPONSE_OK,
                                                 NULL);
    gtk_widget_set_size_request (F_ajout_motif, 800, 600);

    g_signal_connect( F_ajout_motif, "response", G_CALLBACK(CB_ajout_motif), NULL );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout_motif)->vbox ), hboite, TRUE, TRUE, 0 );

/************************************************ La liste des classes ********************************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE_CLASSE, G_TYPE_STRING,                                        /* Id de la classe */
                                                     G_TYPE_STRING                                                  /* Classe */
                               );

    Liste_classe = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );   /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_classe) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_BROWSE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_classe );

    renderer = gtk_cell_renderer_text_new();                                  /* Colonne de l'id du icone */
    colonne = gtk_tree_view_column_new_with_attributes ( _("ClassName"), renderer,
                                                         "text", COLONNE_CLASSE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_CLASSE_LIBELLE);         /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_classe), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_icone), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_classe), TRUE );          /* Pour faire beau */

    g_signal_connect_swapped( G_OBJECT(selection), "changed",                    /* Gestion du menu popup */
                              G_CALLBACK(Clic_classe_atelier), NULL );
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */

/***************************************** La liste des icones ********************************************/
    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), vboite, TRUE, TRUE, 0 );

    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(vboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_STRING,                                                            /* Id */
                                              G_TYPE_STRING                                                        /* libellé */
                               );

    Liste_icone = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );    /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_icone) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_icone );

    renderer = gtk_cell_renderer_text_new();                                                      /* Colonne de l'id du icone */
    g_object_set( renderer, "xalign", 0.5, NULL );
    colonne = gtk_tree_view_column_new_with_attributes ( _("IconeId"), renderer,
                                                         "text", COLONNE_ICONE_ID,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ICONE_ID);                                   /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_icone), colonne );

    renderer = gtk_cell_renderer_text_new();                                                   /* Colonne du libelle de icone */
    colonne = gtk_tree_view_column_new_with_attributes ( _("IconName"), renderer,
                                                         "text", COLONNE_ICONE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ICONE_LIBELLE);                              /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_icone), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_icone), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_icone), TRUE );                                      /* Pour faire beau */

    g_signal_connect_swapped( G_OBJECT(selection), "changed",                    /* Gestion du menu popup */
                              G_CALLBACK(Clic_icone_atelier), NULL );
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
/*********************************************** Preview et controle **************************************/
    table = gtk_table_new( 3, 3, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, FALSE, FALSE, 0 );

    Trame_preview0 = Trame_creer_trame( TAILLE_ICONE_X, TAILLE_ICONE_Y, "darkgray", 0 );
    gtk_widget_set_usize( Trame_preview0->trame_widget, TAILLE_ICONE_X, TAILLE_ICONE_Y );

    gtk_table_attach_defaults( GTK_TABLE(table), Trame_preview0->trame_widget, 0, 1, 0, 3 );
    Remplir_liste_classe();                                  /* Récupération de la liste des classes et affichage sur l'ecran */
  }
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
