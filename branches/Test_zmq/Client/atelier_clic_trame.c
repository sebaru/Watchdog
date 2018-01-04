/**********************************************************************************************************/
/* Client/atelier_clic_trame.c        Gestion des evenements sur la trame                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                   sam. 19 sept. 2009 11:59:15 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * atelier_clic_trame.c
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
 
 #include "trame.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* Nouveau_groupe: Renvoie un numero de groupe unique                                                     */
/* Entrée: Rien                                                                                           */
/* Sortie: un gint                                                                                        */
/**********************************************************************************************************/
 gint Nouveau_groupe ( void )
  { static gint groupe = 0;
    groupe++;
    return(groupe);
  }
/**********************************************************************************************************/
/* Raise_to_top : Raise le item selectionné                                                               */
/* Entrée: Rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Raise_to_top ( void )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    GList *liste;
    gint layer;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    switch (infos->Selection.type)
     { case TYPE_MOTIF      : goo_canvas_item_raise ( infos->Selection.trame_motif->item_groupe, NULL );
                              liste = infos->Trame_atelier->trame_items;
                              layer = 0;
                              while (liste)
                               { struct TRAME_ITEM_MOTIF *trame_motif;
                                 switch ( *((gint *)liste->data) )
                                  { case TYPE_MOTIF:
                                         trame_motif = ((struct TRAME_ITEM_MOTIF *)liste->data);
                                         if (trame_motif->motif->layer > layer) layer = trame_motif->motif->layer;
                                         break;
                                  }
                                 liste = liste->next;
                               }
                              infos->Selection.trame_motif->motif->layer = layer + 1;
                              break;
       case TYPE_CADRAN    : goo_canvas_item_raise ( infos->Selection.trame_cadran->item_groupe, NULL );
                              break;
       case TYPE_PASSERELLE : goo_canvas_item_raise ( infos->Selection.trame_pass->item_groupe, NULL );
                              break;
       default: printf("Raise_to_top: Type de selection inconnu\n");
     }
  }
/**********************************************************************************************************/
/* Lower_to_bottom : Lower le motif au fond                                                               */
/* Entrée: Rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Lower_to_bottom ( void )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    GList *liste;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    switch (infos->Selection.type)
     { case TYPE_MOTIF      : goo_canvas_item_lower ( infos->Selection.trame_motif->item_groupe, NULL );
                              liste = infos->Trame_atelier->trame_items;
                              while (liste)
                               { struct TRAME_ITEM_MOTIF *trame_motif;
                                 switch ( *((gint *)liste->data) )
                                  { case TYPE_MOTIF:
                                         trame_motif = ((struct TRAME_ITEM_MOTIF *)liste->data);
                                         trame_motif->motif->layer++;
                                         break;
                                  }
                                 liste = liste->next;
                               }
                              infos->Selection.trame_motif->motif->layer = 0;
                              break;
       case TYPE_CADRAN    : goo_canvas_item_lower ( infos->Selection.trame_cadran->item_groupe, NULL );
                              break;
       case TYPE_PASSERELLE : goo_canvas_item_lower ( infos->Selection.trame_pass->item_groupe, NULL );
                              break;
       default: printf("Lower_to_bottom: Type de selection inconnu\n");
     }
  }
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
     { case TYPE_MOTIF      : Editer_propriete_TOR( infos->Selection.trame_motif );          break;
       case TYPE_CADRAN    : Menu_ajouter_editer_cadran( infos->Selection.trame_cadran ); break;
       case TYPE_PASSERELLE : Editer_propriete_pass( infos->Selection.trame_pass );          break;
       default: printf("Afficher_propriete: Type de selection inconnu\n");
     }
  }
/**********************************************************************************************************/
/* Changer_couleur_directe: Change directement la couleur sans passer par toutes les fenetres de choix    */
/* Entrée: Rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Changer_couleur_directe ( void )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    switch (infos->Selection.type)
     { case TYPE_MOTIF      : Changer_couleur_motif_directe ( infos->Selection.trame_motif ); break;
       default: printf("Changer_couleur_directe: Type de selection inconnu\n");
     }
  }
/**********************************************************************************************************/
/* Mettre_a_jour_position: S'occupe des rulers et des entry posxy pour affichage position souris/objet    */
/* Entrée: la nouvelle position X et Y                                                                    */
/* Sortie: sans                                                                                           */
/**********************************************************************************************************/
 static void Mettre_a_jour_position ( struct TYPE_INFO_ATELIER *infos, gint x, gint y, gfloat angle )
  { gchar chaine[30];

    snprintf( chaine, sizeof(chaine), "X = %d, Y = %d", x, y );
    gtk_entry_set_text( GTK_ENTRY(infos->Entry_posxy), chaine );

    g_signal_handlers_block_by_func( G_OBJECT(infos->Adj_angle),
                                     G_CALLBACK(Rotationner_selection), infos );
    gtk_adjustment_set_value ( infos->Adj_angle, (gdouble)angle );
    g_signal_handlers_unblock_by_func( G_OBJECT(infos->Adj_angle),
                                       G_CALLBACK(Rotationner_selection), infos );
  }
/**********************************************************************************************************/
/* Mettre_a_jour_position: S'occupe des rulers et des entry posxy pour affichage position souris/objet    */
/* Entrée: la nouvelle position X et Y                                                                    */
/* Sortie: sans                                                                                           */
/**********************************************************************************************************/
 static void Mettre_a_jour_description ( struct TYPE_INFO_ATELIER *infos, gint icone_id, gchar *description )
  { gchar chaine[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];

    snprintf( chaine, sizeof(chaine), "%4d - %s", icone_id, description );
    gtk_entry_set_text( GTK_ENTRY(infos->Entry_libelle), chaine );
  }
/**********************************************************************************************************/
/* Clic_sur_fond: Appelé quand un evenement est capté sur le fond synoptique                              */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Clic_sur_fond ( struct TYPE_INFO_ATELIER *infos, GdkEvent *event, gpointer data )
  { switch (event->type)
     { case GDK_LEAVE_NOTIFY:  break;
       case GDK_ENTER_NOTIFY:
       case GDK_MOTION_NOTIFY: Mettre_a_jour_position( infos, event->motion.x, event->motion.y, 0.0 );
                               break;
       case GDK_BUTTON_PRESS:  if ( !(event->button.state & 0x4) )                         /* Si pas CTRL */
                                { Tout_deselectionner( infos );
                                  infos->Selection.type = TYPE_RIEN;
                                  infos->Selection.trame_motif = NULL;
                                  infos->Selection.groupe = 0;
                                }
                               break;
       default: break;
     }
  }
/**********************************************************************************************************/
/* Clic_general: Fonction generale de traitement du clic sur la trame                                     */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 static void Clic_general ( struct TYPE_INFO_ATELIER *infos, GdkEvent *event )
  { static gdouble Clic_x, Clic_y;
    static gint Appui=0;
    gfloat angle;
    gint x, y;

    if (!event) { printf("Clic_general: event=NULL\n"); return; }
    switch (event->type)
     { case GDK_BUTTON_RELEASE: Appui = 0;
                                /* Update BD */
                                break;
       case GDK_BUTTON_PRESS:   Clic_x = event->button.x_root;
                                Clic_y = event->button.y_root;
                                printf(" Appui: ClicXY=%f %f  rootx=%f, rooty=%f\n",
                                       event->button.x, event->button.y,
                                       event->button.x_root, event->button.y_root );
                                Appui = 1;
                                /*if (event->button.button == 1) */             /* Bouton gauche souris ? */
                                 { if ( event->button.state & 0x4 )                           /* CTRL ?? */
                                    { Selectionner ( infos, infos->Selection.groupe, TRUE ); }
                                   else
                                    { if ( !Tester_selection( infos, infos->Selection.groupe ) ) 
                                       { Tout_deselectionner( infos ); }
                                      Selectionner ( infos, infos->Selection.groupe, FALSE );
                                    }
                                 }
                                break;
       case GDK_MOTION_NOTIFY: 
                               if (Appui)
                                { if (event->motion.state & 0x100)           /* Motion + Bouton gauche ?? */
                                   { gdouble posx, posy;
                                     posx = event->motion.x_root-Clic_x;
                                     posy = event->motion.y_root-Clic_y;
                                     printf("posx=%f, posy=%f\n", posx, posy );
                                     Deplacer_selection( infos, (gint)posx, (gint)posy);
                                   }
                                  else if (event->motion.state & 0x200)      /* Motion + Bouton gauche ?? */
                                   { printf("bouh\n");
                                   }
                                }
                               switch (infos->Selection.type)        /* Mise a jour des entrys positions */
                                { case TYPE_PASSERELLE:
                                       x = infos->Selection.trame_pass->pass->position_x;
                                       y = infos->Selection.trame_pass->pass->position_y;
                                       angle = infos->Selection.trame_pass->pass->angle;
                                       break;
                                  case TYPE_COMMENTAIRE:
                                       x = infos->Selection.trame_comment->comment->position_x;
                                       y = infos->Selection.trame_comment->comment->position_y;
                                       angle = infos->Selection.trame_comment->comment->angle;
                                       break;
                                  case TYPE_MOTIF:
                                       x = infos->Selection.trame_motif->motif->position_x;
                                       y = infos->Selection.trame_motif->motif->position_y;
                                       angle = infos->Selection.trame_motif->motif->angle;
                                       break;
                                  case TYPE_CADRAN:
                                       x = infos->Selection.trame_cadran->cadran->position_x;
                                       y = infos->Selection.trame_cadran->cadran->position_y;
                                       angle = infos->Selection.trame_cadran->cadran->angle;
                                       break;
                                  case TYPE_CAMERA_SUP:
                                       x = infos->Selection.trame_camera_sup->camera_sup->posx;
                                       y = infos->Selection.trame_camera_sup->camera_sup->posy;
                                       angle = 0.0;
                                       break;
                                  case TYPE_SCENARIO:
                                       x = infos->Selection.trame_scenario->scenario->posx;
                                       y = infos->Selection.trame_scenario->scenario->posy;
                                       angle = 0.0;
                                       break;
                                  default: printf("Clic_general: type inconnu %d\n", infos->Selection.type );
                                           x=-1; y=-1; angle = 0.0;
                                }
                               if (Appui)
                                { Clic_x = x;
                                  Clic_y = y;
                                }
                               Mettre_a_jour_position( infos, x, y, angle );
                               break;
       default: printf("Clic_general: event=%d\n", event->type ); return;
     }
  }
/**********************************************************************************************************/
/* Clic_sur_motif: Appelé quand un evenement est capté sur un motif                                       */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Clic_sur_motif ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                       struct TRAME_ITEM_MOTIF *trame_motif )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    static GtkWidget *Popup = NULL;
    static GnomeUIInfo Popup_scale[]=                                      /*!< Définition du popup scale */
     { GNOMEUIINFO_ITEM_STOCK( N_("Scale to 1:1"), NULL, Mettre_echelle_selection_1_1, GNOME_STOCK_PIXMAP_ALIGN_JUSTIFY ),
       GNOMEUIINFO_ITEM_STOCK( N_("Scale to 1:Y"), NULL, Mettre_echelle_selection_1_Y, GNOME_STOCK_PIXMAP_ALIGN_JUSTIFY ),
       GNOMEUIINFO_ITEM_STOCK( N_("Scale to X:1"), NULL, Mettre_echelle_selection_X_1, GNOME_STOCK_PIXMAP_ALIGN_JUSTIFY ),
       GNOMEUIINFO_END
     };
    static GnomeUIInfo Popup_raise[]=                                      /*!< Définition du popup raise */
     { GNOMEUIINFO_ITEM_STOCK( N_("Raise to top"), NULL, Raise_to_top, GNOME_STOCK_PIXMAP_TOP ),
       GNOMEUIINFO_ITEM_STOCK( N_("Lower to bottom"), NULL, Lower_to_bottom, GNOME_STOCK_PIXMAP_BOTTOM ),
       GNOMEUIINFO_END
     };
    static GnomeUIInfo Popup_motif[]=
     { GNOMEUIINFO_ITEM_STOCK( N_("DLS properties"), NULL, Afficher_propriete, GNOME_STOCK_PIXMAP_PROPERTIES ),
       GNOMEUIINFO_ITEM_STOCK( N_("Default color"), NULL,
                               Changer_couleur_directe, GNOME_STOCK_PIXMAP_COLORSELECTOR ),
       GNOMEUIINFO_SUBTREE(N_("_Scale"), Popup_scale),
       GNOMEUIINFO_SUBTREE(N_("_Raise/Lower"), Popup_raise),
       GNOMEUIINFO_SEPARATOR,
       GNOMEUIINFO_ITEM_STOCK( N_("Detach from group"), NULL, Detacher_selection, GNOME_STOCK_PIXMAP_CUT ),
       GNOMEUIINFO_ITEM_STOCK( N_("Fusionner selection"), NULL, Fusionner_selection, GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST ),
       GNOMEUIINFO_ITEM_STOCK( N_("Duplicate selection"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),
       GNOMEUIINFO_SEPARATOR,
       GNOMEUIINFO_ITEM_STOCK( N_("Delete selection"), NULL, Effacer_selection, GNOME_STOCK_PIXMAP_TRASH ),
       GNOMEUIINFO_END
     };

    if (!(trame_motif && event)) return;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    infos->Selection.type = TYPE_MOTIF;
    infos->Selection.groupe = trame_motif->groupe_dpl;
    infos->Selection.trame_motif = trame_motif;

    if (trame_motif->motif->type_gestion == TYPE_FOND)
     { Clic_sur_fond( infos, event, NULL ); }
    else
     { Clic_general( infos, event );                                             /* Fonction de base clic */
     }

    Mettre_a_jour_description( infos, trame_motif->motif->icone_id, trame_motif->motif->libelle );
    if (event->type == GDK_BUTTON_PRESS)
     { if ( event->button.button == 1)
        { goo_canvas_item_raise( trame_motif->select_hg, NULL );
          goo_canvas_item_raise( trame_motif->select_hd, NULL );
          goo_canvas_item_raise( trame_motif->select_bg, NULL );
          goo_canvas_item_raise( trame_motif->select_bd, NULL );
        }
       else if (event->button.button == 3)
        { if (!Popup) Popup = gnome_popup_menu_new( Popup_motif );                       /* Creation menu */
          gnome_popup_menu_do_popup_modal( Popup, NULL, NULL, (GdkEventButton *)event, NULL, F_client );

        }
     }
    else if ( event->button.button == 1 &&                                       /* Double clic gauche ?? */
              event->type == GDK_2BUTTON_PRESS) Afficher_propriete();
  }
/**********************************************************************************************************/
/* Clic_sur_motif: Appelé quand un evenement est capté sur un motif                                       */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Clic_sur_comment ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                       struct TRAME_ITEM_COMMENT *trame_comment )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    static GtkWidget *Popup = NULL;
    static GnomeUIInfo Popup_comment[]=
     { GNOMEUIINFO_ITEM_STOCK( N_("Properties"), NULL, Afficher_propriete, GNOME_STOCK_PIXMAP_PROPERTIES ),
       GNOMEUIINFO_SEPARATOR,
       /*GNOMEUIINFO_ITEM_STOCK( _("Duplicate item"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),*/
       GNOMEUIINFO_ITEM_STOCK( N_("Detach from group"), NULL, Detacher_selection, GNOME_STOCK_PIXMAP_CUT ),
       GNOMEUIINFO_ITEM_STOCK( N_("Fusionner selection"), NULL, Fusionner_selection, GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST ),
       GNOMEUIINFO_ITEM_STOCK( N_("Duplicate selection"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),
       GNOMEUIINFO_SEPARATOR,
       GNOMEUIINFO_ITEM_STOCK( N_("Delete selection"), NULL, Effacer_selection, GNOME_STOCK_PIXMAP_TRASH ),
       GNOMEUIINFO_END
     };
    if (!(trame_comment && event)) return;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    infos->Selection.type = TYPE_COMMENTAIRE;
    infos->Selection.groupe = trame_comment->groupe_dpl;
    infos->Selection.trame_comment = trame_comment;

    Clic_general( infos, event );                                                /* Fonction de base clic */

    Mettre_a_jour_description( infos, trame_comment->comment->id, trame_comment->comment->libelle );
    if (event->type == GDK_BUTTON_PRESS)
     { if ( event->button.button == 1)
        { goo_canvas_item_raise( trame_comment->select_mi, NULL );
        }
       else if (event->button.button == 3)
        { if (!Popup) Popup = gnome_popup_menu_new( Popup_comment );                     /* Creation menu */
          gnome_popup_menu_do_popup_modal( Popup, NULL, NULL, (GdkEventButton *)event, NULL, F_client );

        }
     }
/*    else if ( event->button.button == 1 &&                                       /* Double clic gauche ?? */
/*              event->type == GDK_2BUTTON_PRESS) Afficher_propriete();*/
  }
/**********************************************************************************************************/
/* Clic_sur_pass: Appelé quand un evenement est capté sur une passerelle                                  */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Clic_sur_pass ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                      struct TRAME_ITEM_PASS *trame_pass )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    static GtkWidget *Popup = NULL;
    static GnomeUIInfo Popup_pass[]=
     { GNOMEUIINFO_ITEM_STOCK( N_("Properties"), NULL, Afficher_propriete, GNOME_STOCK_PIXMAP_PROPERTIES ),
       GNOMEUIINFO_SEPARATOR,
       /*GNOMEUIINFO_ITEM_STOCK( _("Duplicate item"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),*/
       GNOMEUIINFO_ITEM_STOCK( N_("Detach from group"), NULL, Detacher_selection, GNOME_STOCK_PIXMAP_CUT ),
       GNOMEUIINFO_ITEM_STOCK( N_("Fusionner selection"), NULL, Fusionner_selection, GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST ),
       GNOMEUIINFO_ITEM_STOCK( N_("Duplicate selection"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),
       GNOMEUIINFO_SEPARATOR,
       GNOMEUIINFO_ITEM_STOCK( N_("Delete selection"), NULL, Effacer_selection, GNOME_STOCK_PIXMAP_TRASH ),
       GNOMEUIINFO_END
     };
    if (!(trame_pass && event)) return;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;                             /* Pointeur sur les infos de la page atelier */

    infos->Selection.type = TYPE_PASSERELLE;
    infos->Selection.groupe = trame_pass->groupe_dpl;
    infos->Selection.trame_pass = trame_pass;

    Clic_general( infos, event );                                                /* Fonction de base clic */

    Mettre_a_jour_description( infos, 0, _("Gateway") );
    if (event->type == GDK_BUTTON_PRESS)
     { if ( event->button.button == 1)
        { goo_canvas_item_raise( trame_pass->item_groupe, NULL ); }
       else if (event->button.button == 3)
        { if (!Popup) Popup = gnome_popup_menu_new( Popup_pass );                        /* Creation menu */
          gnome_popup_menu_do_popup_modal( Popup, NULL, NULL, (GdkEventButton *)event, NULL, F_client );

        }
     }
    else if ( event->button.button == 1 &&                                       /* Double clic gauche ?? */
              event->type == GDK_2BUTTON_PRESS) Afficher_propriete();
  }
/******************************************************************************************************************************/
/* Clic_sur_cadran: Appelé quand un evenement est capté sur un cadran                                                         */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_cadran ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                      struct TRAME_ITEM_CADRAN *trame_cadran )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    static GtkWidget *Popup = NULL;
    static GnomeUIInfo Popup_cadran[]=
     { GNOMEUIINFO_ITEM_STOCK( N_("Properties"), NULL, Afficher_propriete, GNOME_STOCK_PIXMAP_PROPERTIES ),
       GNOMEUIINFO_SEPARATOR,
       /*GNOMEUIINFO_ITEM_STOCK( _("Duplicate item"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),*/
       GNOMEUIINFO_ITEM_STOCK( N_("Detach from group"), NULL, Detacher_selection, GNOME_STOCK_PIXMAP_CUT ),
       GNOMEUIINFO_ITEM_STOCK( N_("Fusionner selection"), NULL, Fusionner_selection, GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST ),
       GNOMEUIINFO_ITEM_STOCK( N_("Duplicate selection"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),
       GNOMEUIINFO_SEPARATOR,
       GNOMEUIINFO_ITEM_STOCK( N_("Delete selection"), NULL, Effacer_selection, GNOME_STOCK_PIXMAP_TRASH ),
       GNOMEUIINFO_END
     };
    if (!(trame_cadran && event)) return;

    page = Page_actuelle();                                                                   /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;                                   /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;                             /* Pointeur sur les infos de la page atelier */

    infos->Selection.type = TYPE_CADRAN;
    infos->Selection.groupe = trame_cadran->groupe_dpl;
    infos->Selection.trame_cadran = trame_cadran;

    Clic_general( infos, event );                                                                    /* Fonction de base clic */

    Mettre_a_jour_description( infos, 0, trame_cadran->cadran->libelle );
    if (event->type == GDK_BUTTON_PRESS)
     { if ( event->button.button == 1)
        { goo_canvas_item_raise( trame_cadran->select_mi, NULL );
        }
       else if (event->button.button == 3)
        { if (!Popup) Popup = gnome_popup_menu_new( Popup_cadran );                       /* Creation menu */
          gnome_popup_menu_do_popup_modal( Popup, NULL, NULL, (GdkEventButton *)event, NULL, F_client );

        }
     }
    else if ( event->button.button == 1 &&                                       /* Double clic gauche ?? */
              event->type == GDK_2BUTTON_PRESS) Afficher_propriete();
  }
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
/******************************************************************************************************************************/
/* Clic_sur_camera_sup: Appelé quand un evenement est capté sur une camera de supervision                                     */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_scenario ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                          struct TRAME_ITEM_SCENARIO *trame_scenario )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    static GtkWidget *Popup = NULL;
    static GnomeUIInfo Popup_scenario[]=
     { /*GNOMEUIINFO_ITEM_STOCK( _("Duplicate item"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),*/
       GNOMEUIINFO_ITEM_STOCK( N_("Detach from group"), NULL, Detacher_selection, GNOME_STOCK_PIXMAP_CUT ),
       GNOMEUIINFO_ITEM_STOCK( N_("Fusionner selection"), NULL, Fusionner_selection, GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST ),
       GNOMEUIINFO_ITEM_STOCK( N_("Duplicate selection"), NULL, Dupliquer_selection, GNOME_STOCK_PIXMAP_COPY ),
       GNOMEUIINFO_SEPARATOR,
       GNOMEUIINFO_ITEM_STOCK( N_("Delete selection"), NULL, Effacer_selection, GNOME_STOCK_PIXMAP_TRASH ),
       GNOMEUIINFO_END
     };

    if (!(trame_scenario && event)) return;

    page = Page_actuelle();                                                                   /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;                                   /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;                             /* Pointeur sur les infos de la page atelier */

    infos->Selection.type = TYPE_SCENARIO;
    infos->Selection.groupe = trame_scenario->groupe_dpl;
    infos->Selection.trame_scenario = trame_scenario;

    Clic_general( infos, event );                                                                    /* Fonction de base clic */

    Mettre_a_jour_description( infos, trame_scenario->scenario->num, "Scenario" );
    if (event->type == GDK_BUTTON_PRESS)
     { if ( event->button.button == 1)
        { goo_canvas_item_raise( trame_scenario->select_mi, NULL ); }
       else if (event->button.button == 3)
        { if (!Popup) Popup = gnome_popup_menu_new( Popup_scenario );                                      /* Creation menu */
          gnome_popup_menu_do_popup_modal( Popup, NULL, NULL, (GdkEventButton *)event, NULL, F_client );

        }
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
