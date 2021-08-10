/******************************************************************************************************************************/
/* Client/supervision_cadran.c        Affichage des cadrans synoptique de supervision                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           mer 01 fév 2006 18:41:37 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * supervision_cadran.c
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

 #include "trame.h"
/***************************************** Définitions des prototypes programme ***********************************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Clic_sur_camera_supervision: Appelé quand un evenement est capté sur une camera de supervision                             */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_cadran_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                    GdkEvent *event, struct TRAME_ITEM_CADRAN *trame_cadran )
  { if ( !(event->button.button == 1 &&                                                                     /* clic gauche ?? */
           event->type == GDK_BUTTON_PRESS)
       ) return;
    if (!trame_cadran->page->client->connexion) return;

    GtkWidget *fenetre = gtk_dialog_new_with_buttons ( "Changer un registre", GTK_WINDOW(trame_cadran->page->client->window),
                                                       GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                       "Annuler", GTK_RESPONSE_CANCEL, "Valider", GTK_RESPONSE_OK, NULL );

    GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (fenetre));

    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
    gtk_box_pack_start (GTK_BOX (content_area), hbox, FALSE, FALSE, 0);

    GtkWidget *image = gtk_image_new_from_icon_name ("dialog-question", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

    GtkWidget *table = gtk_grid_new();                                                                   /* Table des entrys identifiant */
    gtk_box_pack_start( GTK_BOX(hbox), table, TRUE, TRUE, 0 );
    gtk_grid_set_row_spacing( GTK_GRID(table), 1 );
    gtk_grid_set_column_spacing( GTK_GRID(table), 2 );

    GtkWidget *texte = gtk_label_new( "Nouvelle valeur" );
    gtk_grid_attach( GTK_GRID(table), texte, 0, 0, 1, 1 );
    GtkWidget *Entry_valeur = gtk_entry_new();
    gtk_entry_set_placeholder_text ( GTK_ENTRY(Entry_valeur), "valeur choisie" );
    gtk_widget_set_tooltip_text ( Entry_valeur, "Entrez la nouvelle valeur du registre" );
    /*gtk_entry_set_icon_from_icon_name ( GTK_ENTRY(Entry_valeur), GTK_ENTRY_ICON_PRIMARY, "system-run" );*/
    gtk_grid_attach( GTK_GRID(table), Entry_valeur, 1, 0, 1, 1 );

    gtk_widget_grab_focus( Entry_valeur );
    gtk_widget_show_all( hbox );

    if (gtk_dialog_run( GTK_DIALOG(fenetre) ) == GTK_RESPONSE_OK)                      /* Attente de reponse de l'utilisateur */
     { gdouble new_valeur = g_strtod ( gtk_entry_get_text( GTK_ENTRY(Entry_valeur) ), NULL );
       JsonBuilder *RootNode = Json_create();
       Json_add_double ( RootNode, "valeur", new_valeur );
       Json_add_string ( RootNode, "classe",   "REGISTRE" );
       Json_add_string ( RootNode, "tech_id",  Json_get_string( trame_cadran->cadran, "tech_id" ) );
       Json_add_string ( RootNode, "acronyme", Json_get_string( trame_cadran->cadran, "acronyme" ) );
       Envoi_json_au_serveur ( trame_cadran->page->client, "PUT", RootNode, "/api/dls/set", NULL );

     }
    gtk_widget_destroy( fenetre );
  }
/******************************************************************************************************************************/
/* Met a jour le libelle d'un cadran                                                                                          */
/******************************************************************************************************************************/
 static void Updater_un_cadran ( struct TRAME_ITEM_CADRAN *trame_cadran, JsonNode *cadran )
  { gchar libelle[25];
    gdouble valeur = Json_get_double ( cadran, "valeur" );
    gchar *classe  = Json_get_string ( trame_cadran->cadran, "classe" );

    if (!strcasecmp ( classe, "DI" ) || !strcasecmp ( classe, "BI" ) )
     { g_snprintf( libelle, sizeof(libelle), "%s", (valeur ? "TRUE" : "FALSE") ); }
    else if (!strcasecmp ( classe, "REGISTRE" ) || !strcasecmp ( classe, "AI" ) )
     { if (!Json_get_bool(cadran,"in_range")) g_snprintf(libelle, sizeof(libelle), "not in range" );
       else
        { gchar *digit, format[24];
          if(-1000000.0<valeur && valeur<1000000.0) digit = "%6"; else digit="%8";
          g_snprintf( format, sizeof(format), "%s.%df %%s", digit, Json_get_int ( trame_cadran->cadran, "nb_decimal" ) );
          g_snprintf( libelle, sizeof(libelle), format, valeur, Json_get_string(cadran, "unite") );
        }
     }
    else if (!strcasecmp ( classe, "CH" ) )
     { if (valeur < 3600)
        { g_snprintf( libelle, sizeof(libelle), "%02dm%02ds", (int)valeur/60, ((int)valeur%60) ); }
       else
        { g_snprintf( libelle, sizeof(libelle), "%04dh%02dm", (int)valeur/3600, ((int)valeur%3600)/60 ); }
     }
    else if (!strcasecmp ( classe, "CI" ) )
     { g_snprintf( libelle, sizeof(libelle), "%8.2f %s", valeur, Json_get_string(cadran, "unite") ); }
    else if (!strcasecmp ( classe, "TEMPO" ) )
     { gint src, heure, minute, seconde;
       src = valeur/10;
       heure = src / 3600;
       minute = (src - heure*3600) / 60;
       seconde = src - heure*3600 - minute*60;
       g_snprintf( libelle, sizeof(libelle), "%02d:%02d:%02d", heure, minute, seconde );
     }
    else { g_snprintf( libelle, sizeof(libelle), "unknown" ); }
    /*printf("%s: update cadran %s:%s to %s\n", __func__,
           trame_cadran->cadran->tech_id, trame_cadran->cadran->acronyme, libelle );*/
    g_object_set( trame_cadran->item_entry, "text", libelle, NULL );
  }
/******************************************************************************************************************************/
/* Proto_changer_etat_cadran: Rafraichissement du visuel cadran sur parametre                                                 */
/* Entrée: une reference sur le cadran                                                                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_les_cadrans( struct PAGE_NOTEBOOK *page, JsonNode *cadran )
  { struct TYPE_INFO_SUPERVISION *infos = page->infos;
    GList *liste_cadrans;
    gint cpt;

    pthread_mutex_lock ( &infos->Trame->lock );
    liste_cadrans = infos->Trame->trame_items;                                     /* On parcours tous les cadrans de la page */
    while (liste_cadrans)
     { switch( *((gint *)liste_cadrans->data) )
        { case TYPE_CADRAN    :
           { cpt++;
             struct TRAME_ITEM_CADRAN *trame_cadran = liste_cadrans->data;
             if ( !strcmp( Json_get_string(cadran,"tech_id"), Json_get_string ( trame_cadran->cadran, "tech_id" ) ) &&
                  !strcmp( Json_get_string(cadran,"acronyme"), Json_get_string ( trame_cadran->cadran, "acronyme") )
                )
              { Updater_un_cadran ( trame_cadran, cadran ); }
             break;
           }
          default: break;
        }
       liste_cadrans=liste_cadrans->next;
     }
    pthread_mutex_unlock ( &infos->Trame->lock );
    if (!cpt)                                       /* Si nous n'avons rien mis à jour, alors nous demandons le desabonnement */
     { /*Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_CHANGE_CADRAN_UNKNOWN,
                      (gchar *)etat_cadran, sizeof(struct CMD_ETAT_BIT_CADRAN) );*/
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
