/******************************************************************************************************************************/
/* Client/atelier_clic_trame.c        Gestion des evenements sur la trame                                                     */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       sam. 19 sept. 2009 11:59:15 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_clic_trame.c
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

/******************************************* Définitions des prototypes programme *********************************************/
 #include "protocli.h"
/******************************************************************************************************************************/
/* Raise_to_top : Raise le item selectionné                                                                                   */
/* Entrée: Rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Raise_to_top ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    GList *liste;
    gint layer;

    if (!infos->Selection) return;
    switch ( *((gint *)(infos->Selection->data) ) )
     { case TYPE_MOTIF:
        { struct TRAME_ITEM_MOTIF *trame_motif = infos->Selection->data;
          goo_canvas_item_raise ( trame_motif->item_groupe, NULL );
          pthread_mutex_lock ( &infos->Trame_atelier->lock );
          liste = infos->Trame_atelier->trame_items;
          layer = 0;
          while (liste)
           { switch ( *((gint *)liste->data) )
              { case TYPE_MOTIF:
                 { struct TRAME_ITEM_MOTIF *item= liste->data;
                   gint item_layer = Json_get_int ( item->visuel, "layer" );
                   if (item_layer > layer) layer = item_layer;
                   break;
                 }
              }
             liste = liste->next;
           }
          pthread_mutex_unlock ( &infos->Trame_atelier->lock );
          Json_node_add_int ( trame_motif->visuel, "layer", layer + 1 );
          break;
        }
       case TYPE_CADRAN:
        { struct TRAME_ITEM_CADRAN *trame_cadran = infos->Selection->data;
          goo_canvas_item_raise ( trame_cadran->item_groupe, NULL );
          break;
        }
       case TYPE_PASSERELLE:
        { struct TRAME_ITEM_PASS *trame_pass = infos->Selection->data;
          goo_canvas_item_raise ( trame_pass->item_groupe, NULL );
          break;
        }
       default: printf("Raise_to_top: Type de selection inconnu\n");
     }
  }
/******************************************************************************************************************************/
/* Lower_to_bottom : Lower le motif au fond                                                                                   */
/* Entrée: Rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Lower_to_bottom ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    GList *liste;

    if (!infos->Selection) return;
    switch ( *((gint *)(infos->Selection->data) ) )
     { case TYPE_MOTIF:
        { struct TRAME_ITEM_MOTIF *trame_motif = infos->Selection->data;
          goo_canvas_item_lower ( trame_motif->item_groupe, NULL );
          pthread_mutex_lock ( &infos->Trame_atelier->lock );
          liste = infos->Trame_atelier->trame_items;
          while (liste)
           { switch ( *((gint *)liste->data) )
              { case TYPE_MOTIF:
                 { struct TRAME_ITEM_MOTIF *item= liste->data;
                   if ( Json_get_int ( item->visuel, "gestion" ) == TYPE_FOND)
                    { goo_canvas_item_lower ( item->item_groupe, NULL );
                      Json_node_add_int ( item->visuel, "layer", 0 );
                    }
                   else Json_node_add_int ( item->visuel, "layer", Json_get_int ( item->visuel, "layer" ) + 1 );
                   break;
                 }
              }
             liste = liste->next;
           }
          pthread_mutex_unlock ( &infos->Trame_atelier->lock );
          Json_node_add_int ( trame_motif->visuel, "layer", 1 );
         break;
        }
       case TYPE_CADRAN:
        { struct TRAME_ITEM_CADRAN *trame_cadran = infos->Selection->data;
          goo_canvas_item_lower ( trame_cadran->item_groupe, NULL );
          break;
        }
       case TYPE_PASSERELLE:
        { struct TRAME_ITEM_PASS *trame_pass = infos->Selection->data;
          goo_canvas_item_lower ( trame_pass->item_groupe, NULL );
          break;
        }
       default: printf("%s: Type de selection inconnu\n", __func__);
     }
  }
#ifdef bouh
/**********************************************************************************************************/
/* Afficher_propriete: Affiche les proprietes de l'objet                                                  */
/* Entrée: Rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Afficher_propriete ( void )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

printf("Afficher_propriete: debut\n");
    switch (infos->Selection.type)
     { case TYPE_MOTIF      : Editer_propriete_TOR( infos->Selection.trame_motif );        break;
       case TYPE_CADRAN     : Menu_ajouter_editer_cadran( infos->Selection.trame_cadran ); break;
       default: printf("Afficher_propriete: Type de selection inconnu\n");
     }
  }
#endif
/******************************************************************************************************************************/
/* Changer_couleur_directe: Change directement la couleur sans passer par toutes les fenetres de choix                        */
/* Entrée: Rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Changer_couleur_directe ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos = page->infos;

    if (!infos->Selection) return;
    switch ( *((gint *)(infos->Selection->data) ) )
     { case TYPE_MOTIF:
        { /*struct TRAME_ITEM_MOTIF *trame_motif = infos->Selection->data;
          Changer_couleur_motif_directe ( trame_motif );*/
          break;
        }
       default: printf("Changer_couleur_directe: Type de selection inconnu\n");
     }
  }
/******************************************************************************************************************************/
/* Mettre_a_jour_position: S'occupe des rulers et des entry posxy pour affichage position souris/objet                        */
/* Entrée: la nouvelle position X et Y                                                                                        */
/* Sortie: sans                                                                                                               */
/******************************************************************************************************************************/
 static void Mettre_a_jour_position ( struct PAGE_NOTEBOOK *page, gint x, gint y, gint angle, gint scale )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    gchar chaine[30];

    snprintf( chaine, sizeof(chaine), "X = %d, Y = %d", x, y );
    gtk_entry_set_text( GTK_ENTRY(infos->Entry_posxy), chaine );

    g_signal_handlers_block_by_func( G_OBJECT(infos->Adj_angle), G_CALLBACK(Rotationner_selection), page );
    gtk_adjustment_set_value ( infos->Adj_angle, (gdouble)angle );
    g_signal_handlers_unblock_by_func( G_OBJECT(infos->Adj_angle), G_CALLBACK(Rotationner_selection), page );

    g_signal_handlers_block_by_func( G_OBJECT(infos->Adj_scale), G_CALLBACK(Zoomer_selection), page );
    gtk_adjustment_set_value ( infos->Adj_scale, (gdouble)scale );
    g_signal_handlers_unblock_by_func( G_OBJECT(infos->Adj_scale), G_CALLBACK(Zoomer_selection), page );
  }
/******************************************************************************************************************************/
/* Mettre_a_jour_description: S'occupe d'afficher la description de l'objet                                                   */
/* Entrée: la page et la description                                                                                          */
/* Sortie: sans                                                                                                               */
/******************************************************************************************************************************/
 static void Mettre_a_jour_description ( struct PAGE_NOTEBOOK *page, gint icone_id, gchar *description )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    gchar chaine[256];

    snprintf( chaine, sizeof(chaine), "%4d - %s", icone_id, description );
    gtk_entry_set_text( GTK_ENTRY(infos->Entry_libelle), chaine );
  }
/******************************************************************************************************************************/
/* Clic_sur_fond: Appelé quand un evenement est capté sur le fond synoptique                                                  */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 static void Clic_sur_fond ( struct PAGE_NOTEBOOK *page, GdkEvent *event, gpointer data )
  { switch (event->type)
     { case GDK_LEAVE_NOTIFY:  break;
       case GDK_ENTER_NOTIFY:
       case GDK_MOTION_NOTIFY: Mettre_a_jour_position( page, event->motion.x, event->motion.y, 0, 0 );
                               break;
       case GDK_BUTTON_PRESS:  if ( !(event->button.state & 0x4) )                         /* Si pas CTRL */
                                { Tout_deselectionner( page ); }
                               break;
       default: break;
     }
  }
/******************************************************************************************************************************/
/* Clic_general: Fonction generale de traitement du clic sur la trame                                                         */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 static void Clic_general ( struct PAGE_NOTEBOOK *page, GdkEvent *event, gpointer trame_item, gint groupe )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    gint x, y;

    if (!event) { printf("Clic_general: event=NULL\n"); return; }
    switch (event->type)
     { case GDK_BUTTON_RELEASE: infos->Appui = 0;
                                /* Update BD */
                                break;
       case GDK_BUTTON_PRESS:   infos->Clic_x = event->button.x_root;
                                infos->Clic_y = event->button.y_root;
                                printf("Clic_general:  Appui: ClicXY=%f %f  rootx=%f, rooty=%f\n",
                                       event->button.x, event->button.y,
                                       event->button.x_root, event->button.y_root );
                                infos->Appui = 1;
                                /*if (event->button.button == 1) */             /* Bouton gauche souris ? */
                                 { if (! (event->button.state & 0x4) )                         /* CTRL ?? */
                                    { Tout_deselectionner( page ); }
                                   Selectionner ( page, trame_item, groupe );
                                 }
                                break;
       case GDK_MOTION_NOTIFY:
                               if (infos->Appui)
                                { if (event->motion.state & 0x100)           /* Motion + Bouton gauche ?? */
                                   { gdouble posx, posy;
                                     posx = event->motion.x_root-infos->Clic_x;
                                     posy = event->motion.y_root-infos->Clic_y;
                                     printf("Clic_general: Appel a Deplacer_selection posx=%d, posy=%d\n", (gint)posx, (gint)posy );
                                     Deplacer_selection( page, (gint)posx, (gint)posy);
                                   }
                                  else if (event->motion.state & 0x200)      /* Motion + Bouton gauche ?? */
                                   { printf("Clic_general: bouh\n");
                                   }
                                }
                               if (!infos->Selection) { return; }
                               switch ( *((gint *)(infos->Selection->data)) )               /* Mise a jour des entrys positions */
                                { case TYPE_PASSERELLE:
                                        { struct TRAME_ITEM_PASS *trame_pass = infos->Selection->data;
                                          x = 1.0*Json_get_int ( trame_pass->pass, "posx" );
                                          y = 1.0*Json_get_int ( trame_pass->pass, "posy" );
                                        }
                                       break;
                                  case TYPE_COMMENTAIRE:
                                        { struct TRAME_ITEM_COMMENT *trame_comm = infos->Selection->data;
                                          x = 1.0*Json_get_int ( trame_comm->comment, "posx" );
                                          y = 1.0*Json_get_int ( trame_comm->comment, "posy" );
                                        }
                                       break;
                                  case TYPE_MOTIF:
                                        { struct TRAME_ITEM_MOTIF *trame_motif = infos->Selection->data;
                                          x = 1.0*Json_get_int ( trame_motif->visuel, "posx" );
                                          y = 1.0*Json_get_int ( trame_motif->visuel, "posy" );
                                        }
                                       break;
                                  case TYPE_CADRAN:
                                        { struct TRAME_ITEM_CADRAN *trame_cadran = infos->Selection->data;
                                          x = 1.0*Json_get_int ( trame_cadran->cadran, "posx" );
                                          y = 1.0*Json_get_int ( trame_cadran->cadran, "posy" );
                                        }
                                       break;
                                  default: printf("Clic_general: type inconnu\n" );
                                           x=-1; y=-1;
                                }
                               if (infos->Appui)
                                { infos->Clic_x = x;
                                  infos->Clic_y = y;
                                }
                               break;
       default: printf("Clic_general: event=%d\n", event->type ); return;
     }
  }
/******************************************************************************************************************************/
/* Clic_sur_motif: Appelé quand un evenement est capté sur un motif                                                           */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_motif ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { if (!(trame_motif && event)) return;

    struct PAGE_NOTEBOOK *page = trame_motif->page;

    if ( Json_get_int ( trame_motif->visuel, "gestion" ) == TYPE_FOND)
     { Clic_sur_fond( page, event, NULL );
       Mettre_a_jour_position( page, event->motion.x_root, event->motion.y_root, 0, 0 );
     }
    else
     { Clic_general( page, event, trame_motif, Json_get_int ( trame_motif->visuel, "groupe") );      /* Fonction de base clic */
       Mettre_a_jour_position( page, Json_get_int ( trame_motif->visuel, "posx" ),
                                     Json_get_int ( trame_motif->visuel, "posy" ),
                                     Json_get_int ( trame_motif->visuel, "angle" ),
                                     Json_get_int ( trame_motif->visuel, "scale" )
                             );
     }

    Mettre_a_jour_description( page,
                               Json_get_int ( trame_motif->visuel, "icone" ),
                               Json_get_string ( trame_motif->visuel, "libelle" ) );

  //  else if ( event->button.button == 1 &&                                       /* Double clic gauche ?? */
    //          event->type == GDK_2BUTTON_PRESS) Afficher_propriete();

    if (event->type == GDK_BUTTON_PRESS && event->button.button == 1)
     { if (trame_motif->select_hg) goo_canvas_item_raise( trame_motif->select_hg, NULL );
       if (trame_motif->select_hd) goo_canvas_item_raise( trame_motif->select_hd, NULL );
       if (trame_motif->select_hg) goo_canvas_item_raise( trame_motif->select_bg, NULL );
       if (trame_motif->select_bd) goo_canvas_item_raise( trame_motif->select_bd, NULL );
       return;
     }

    if ( ! (event->type == GDK_BUTTON_PRESS && event->button.button == 3)) return;
    GtkWidget *Popup = gtk_menu_new();
    GtkWidget *item, *submenu;

    item = Menu ( "Propriétés", "preferences-system" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Afficher_propriete), client );

    item = Menu ( "Couleur par défaut", "applications-graphics" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    /*g_signal_connect_swapped ( item, "activate", G_CALLBACK (Changer_couleur_directe), page );*/

    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), gtk_separator_menu_item_new () );

    item = Menu ( "Scale", "insert-link" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(item), submenu );

      item = Menu ( "Scale to 1:1", "zoom-original" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Mettre_echelle_selection_1_1), page );

      item = Menu ( "Scale to 1:Y", "object-flip-horizontal" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Mettre_echelle_selection_1_Y), page );

      item = Menu ( "Scale to X:1", "object-flip-vertical" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Mettre_echelle_selection_X_1), page );


    item = Menu ( "Raise/Lower", "insert-link" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(item), submenu );

      item = Menu ( "Raise to top", "go-top" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Raise_to_top), page );

      item = Menu ( "Lower to bottom", "go-bottom" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Lower_to_bottom), page );

    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), gtk_separator_menu_item_new () );

    item = Menu ( "Détacher la sélection", "view-restore" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    g_signal_connect_swapped ( item, "activate", G_CALLBACK (Detacher_selection), page );

    item = Menu ( "Fusionner la sélection", "view-fullscreen" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    g_signal_connect_swapped ( item, "activate", G_CALLBACK (Fusionner_selection), page );

    item = Menu ( "Dupliquer la sélection", "edit-copy" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Dupliquer_selection), client );

    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), gtk_separator_menu_item_new () );

    item = Menu ( "Supprimer la sélection", "edit-delete" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Effacer_selection), client );

    gtk_widget_show_all(Popup);
    gtk_menu_popup_at_pointer ( GTK_MENU(Popup), (GdkEvent *)event );
  }
/******************************************************************************************************************************/
/* Clic_sur_comment: Appelé quand un evenement est capté sur un commentaire                                                   */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_comment ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                         struct TRAME_ITEM_COMMENT *trame_comment )
  { if (!(trame_comment && event)) return;

    struct PAGE_NOTEBOOK *page = trame_comment->page;

    Clic_general( page, event, trame_comment, Json_get_int ( trame_comment->comment, "groupe") );    /* Fonction de base clic */
    Mettre_a_jour_position( page, Json_get_int ( trame_comment->comment, "posx" ),
                                  Json_get_int ( trame_comment->comment, "posy" ),
                                  Json_get_int ( trame_comment->comment, "angle" ),
                                  Json_get_int ( trame_comment->comment, "scale" )
                          );

    Mettre_a_jour_description( page,
                               Json_get_int ( trame_comment->comment, "id" ),
                               Json_get_string ( trame_comment->comment, "libelle" ) );

  //  else if ( event->button.button == 1 &&                                       /* Double clic gauche ?? */
    //          event->type == GDK_2BUTTON_PRESS) Afficher_propriete();

    if (event->type == GDK_BUTTON_PRESS && event->button.button == 1)
     { if (trame_comment->select_mi) goo_canvas_item_raise( trame_comment->select_mi, NULL );
       return;
     }

    if ( ! (event->type == GDK_BUTTON_PRESS && event->button.button == 3)) return;
    GtkWidget *Popup = gtk_menu_new();
    GtkWidget *item, *submenu;

    item = Menu ( "Propriétés", "preferences-system" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Afficher_propriete), client );

    item = Menu ( "Couleur par défaut", "applications-graphics" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    /*g_signal_connect_swapped ( item, "activate", G_CALLBACK (Changer_couleur_directe), page );*/

    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), gtk_separator_menu_item_new () );

    item = Menu ( "Scale", "insert-link" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(item), submenu );

      item = Menu ( "Scale to 1:1", "zoom-original" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Mettre_echelle_selection_1_1), page );

      item = Menu ( "Scale to 1:Y", "object-flip-horizontal" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Mettre_echelle_selection_1_Y), page );

      item = Menu ( "Scale to X:1", "object-flip-vertical" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Mettre_echelle_selection_X_1), page );


    item = Menu ( "Raise/Lower", "insert-link" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(item), submenu );

      item = Menu ( "Raise to top", "go-top" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Raise_to_top), page );

      item = Menu ( "Lower to bottom", "go-bottom" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Lower_to_bottom), page );

    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), gtk_separator_menu_item_new () );

    item = Menu ( "Détacher la sélection", "view-restore" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    g_signal_connect_swapped ( item, "activate", G_CALLBACK (Detacher_selection), page );

    item = Menu ( "Fusionner la sélection", "view-fullscreen" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    g_signal_connect_swapped ( item, "activate", G_CALLBACK (Fusionner_selection), page );

    item = Menu ( "Dupliquer la sélection", "edit-copy" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Dupliquer_selection), client );

    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), gtk_separator_menu_item_new () );

    item = Menu ( "Supprimer la sélection", "edit-delete" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Effacer_selection), client );

    gtk_widget_show_all(Popup);
    gtk_menu_popup_at_pointer ( GTK_MENU(Popup), (GdkEvent *)event );

  }
/******************************************************************************************************************************/
/* Clic_sur_pass: Appelé quand un evenement est capté sur une passerelle                                                      */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_pass ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                      struct TRAME_ITEM_PASS *trame_pass )
  {
/*    static GtkWidget *Popup = NULL;
    static GnomeUIInfo Popup_pass[]=
     { GNOMEUIINFO_ITEM_STOCK( N_("Properties"), NULL, Afficher_propriete, GNOME_STOCK_PIXMAP_PROPERTIES ),
       GNOMEUIINFO_SEPARATOR,
       /*GNOMEUIINFO_ITEM_STOCK( _("Duplicate item"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),*/
   /*    GNOMEUIINFO_ITEM_STOCK( N_("Detach from group"), NULL, Detacher_selection, GNOME_STOCK_PIXMAP_CUT ),
       GNOMEUIINFO_ITEM_STOCK( N_("Fusionner selection"), NULL, Fusionner_selection, GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST ),
       GNOMEUIINFO_ITEM_STOCK( N_("Duplicate selection"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),
       GNOMEUIINFO_SEPARATOR,
       GNOMEUIINFO_ITEM_STOCK( N_("Delete selection"), NULL, Effacer_selection, GNOME_STOCK_PIXMAP_TRASH ),
       GNOMEUIINFO_END
     };*/
    if (!(trame_pass && event)) return;

    struct PAGE_NOTEBOOK *page = trame_pass->page;

    Clic_general( page, event, trame_pass, Json_get_int ( trame_pass->pass, "groupe" ) );            /* Fonction de base clic */
    Mettre_a_jour_position( page, Json_get_int ( trame_pass->pass, "posx" ),
                                  Json_get_int ( trame_pass->pass, "posy" ),
                                  Json_get_int ( trame_pass->pass, "angle" ),
                                  Json_get_int ( trame_pass->pass, "scale" )
                          );
    Mettre_a_jour_description( page, 0, "Gateway" );
    if (event->type == GDK_BUTTON_PRESS)
     { if ( event->button.button == 1)
        { goo_canvas_item_raise( trame_pass->item_groupe, NULL ); }
       else if (event->button.button == 3)
        { /*if (!Popup) Popup = gnome_popup_menu_new( Popup_pass );                        /* Creation menu */
          /*gnome_popup_menu_do_popup_modal( Popup, NULL, NULL, (GdkEventButton *)event, NULL, F_client );*/

        }
     }
/*    else if ( event->button.button == 1 &&                                       /* Double clic gauche ?? */
  /*            event->type == GDK_2BUTTON_PRESS) Afficher_propriete();*/
  }
/******************************************************************************************************************************/
/* Clic_sur_cadran: Appelé quand un evenement est capté sur un cadran                                                         */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_cadran ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                      struct TRAME_ITEM_CADRAN *trame_cadran )
  { gchar chaine[256];
    if (!(trame_cadran && event)) return;

    struct PAGE_NOTEBOOK *page = trame_cadran->page;
printf("%s: trame_cadran = %p\n", __func__, trame_cadran );

    Clic_general( page, event, trame_cadran, Json_get_int ( trame_cadran->cadran, "groupe" ) );      /* Fonction de base clic */
    Mettre_a_jour_position( page, Json_get_int ( trame_cadran->cadran, "posx" ),
                                  Json_get_int ( trame_cadran->cadran, "posy" ),
                                  Json_get_int ( trame_cadran->cadran, "angle" ),
                                  Json_get_int ( trame_cadran->cadran, "scale" ) );

    g_snprintf( chaine, sizeof(chaine), "%s:%s - %s",
                Json_get_string ( trame_cadran->cadran, "tech_id" ),
                Json_get_string ( trame_cadran->cadran, "acronyme" ),
                Json_get_string ( trame_cadran->cadran, "libelle" )
              );
    Mettre_a_jour_description( trame_cadran->page, -1, chaine );

  //  else if ( event->button.button == 1 &&                                       /* Double clic gauche ?? */
    //          event->type == GDK_2BUTTON_PRESS) Afficher_propriete();

    if (event->type == GDK_BUTTON_PRESS && event->button.button == 1)
     { if (trame_cadran->select_mi) goo_canvas_item_raise( trame_cadran->select_mi, NULL );
       return;
     }

    if ( ! (event->type == GDK_BUTTON_PRESS && event->button.button == 3)) return;
    GtkWidget *Popup = gtk_menu_new();
    GtkWidget *item, *submenu;

    item = Menu ( "Propriétés", "preferences-system" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Afficher_propriete), client );

    item = Menu ( "Couleur par défaut", "applications-graphics" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    /*g_signal_connect_swapped ( item, "activate", G_CALLBACK (Changer_couleur_directe), page );*/

    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), gtk_separator_menu_item_new () );

    item = Menu ( "Scale", "insert-link" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(item), submenu );

      item = Menu ( "Scale to 1:1", "zoom-original" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Mettre_echelle_selection_1_1), page );

      item = Menu ( "Scale to 1:Y", "object-flip-horizontal" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Mettre_echelle_selection_1_Y), page );

      item = Menu ( "Scale to X:1", "object-flip-vertical" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Mettre_echelle_selection_X_1), page );


    item = Menu ( "Raise/Lower", "insert-link" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM(item), submenu );

      item = Menu ( "Raise to top", "go-top" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Raise_to_top), page );

      item = Menu ( "Lower to bottom", "go-bottom" );
      gtk_menu_shell_append (GTK_MENU_SHELL(submenu), item);
      g_signal_connect_swapped ( item, "activate", G_CALLBACK (Lower_to_bottom), page );

    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), gtk_separator_menu_item_new () );

    item = Menu ( "Détacher la sélection", "view-restore" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Detacher_selection), client );

    item = Menu ( "Fusionner la sélection", "view-fullscreen" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Fusionner_selection), client );

    item = Menu ( "Dupliquer la sélection", "edit-copy" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Dupliquer_selection), client );

    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), gtk_separator_menu_item_new () );

    item = Menu ( "Supprimer la sélection", "edit-delete" );
    gtk_menu_shell_append (GTK_MENU_SHELL(Popup), item);
    //g_signal_connect_swapped ( item, "activate", G_CALLBACK (Effacer_selection), client );

    gtk_widget_show_all(Popup);
    gtk_menu_popup_at_pointer ( GTK_MENU(Popup), (GdkEvent *)event );




/*    static GnomeUIInfo Popup_cadran[]=
     { GNOMEUIINFO_ITEM_STOCK( N_("Properties"), NULL, Afficher_propriete, GNOME_STOCK_PIXMAP_PROPERTIES ),
       GNOMEUIINFO_SEPARATOR,
       /*GNOMEUIINFO_ITEM_STOCK( _("Duplicate item"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),*/
       /*GNOMEUIINFO_ITEM_STOCK( N_("Detach from group"), NULL, Detacher_selection, GNOME_STOCK_PIXMAP_CUT ),
       GNOMEUIINFO_ITEM_STOCK( N_("Fusionner selection"), NULL, Fusionner_selection, GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST ),
       GNOMEUIINFO_ITEM_STOCK( N_("Duplicate selection"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),
       GNOMEUIINFO_SEPARATOR,
       GNOMEUIINFO_ITEM_STOCK( N_("Delete selection"), NULL, Effacer_selection, GNOME_STOCK_PIXMAP_TRASH ),
       GNOMEUIINFO_END
     };*/

/*    else if ( event->button.button == 1 &&                                       /* Double clic gauche ?? */
  /*            event->type == GDK_2BUTTON_PRESS) Afficher_propriete();*/
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Clic_sur_camera_sup: Appelé quand un evenement est capté sur une camera de supervision                                     */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_camera_sup ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                            struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    static GtkWidget *Popup = NULL;
    static GnomeUIInfo Popup_camera_sup[]=
     { /*GNOMEUIINFO_ITEM_STOCK( _("Duplicate item"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),*/
       GNOMEUIINFO_ITEM_STOCK( N_("Detach from group"), NULL, Detacher_selection, GNOME_STOCK_PIXMAP_CUT ),
       GNOMEUIINFO_ITEM_STOCK( N_("Fusionner selection"), NULL, Fusionner_selection, GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST ),
       GNOMEUIINFO_ITEM_STOCK( N_("Duplicate selection"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),
       GNOMEUIINFO_SEPARATOR,
       GNOMEUIINFO_ITEM_STOCK( N_("Delete selection"), NULL, Effacer_selection, GNOME_STOCK_PIXMAP_TRASH ),
       GNOMEUIINFO_END
     };

    if (!(trame_camera_sup && event)) return;

    page = Page_actuelle();                                                                   /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;                                   /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;                             /* Pointeur sur les infos de la page atelier */

    infos->Selection.type = TYPE_CAMERA_SUP;
    infos->Selection.groupe = trame_camera_sup->groupe_dpl;
    infos->Selection.trame_camera_sup = trame_camera_sup;

    Clic_general( infos, event );                                                                    /* Fonction de base clic */

    Mettre_a_jour_description( infos, trame_camera_sup->camera_sup->camera_src_id,
                                      trame_camera_sup->camera_sup->libelle );
    if (event->type == GDK_BUTTON_PRESS)
     { if ( event->button.button == 1)
        { goo_canvas_item_raise( trame_camera_sup->select_mi, NULL ); }
       else if (event->button.button == 3)
        { if (!Popup) Popup = gnome_popup_menu_new( Popup_camera_sup );                                      /* Creation menu */
          gnome_popup_menu_do_popup_modal( Popup, NULL, NULL, (GdkEventButton *)event, NULL, F_client );

        }
     }
  }
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
