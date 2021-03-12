/******************************************************************************************************************************/
/* Client/atelier_selection.c         gestion des selections synoptique                                                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       sam. 19 sept. 2009 13:22:22 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_selection.c
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
/* Deselectionner Deselectionne un item parmi tous ceux selectionnés                                                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Deselectionner ( struct TYPE_INFO_ATELIER *infos, struct TRAME_ITEM *item )
  { infos->Selection = g_slist_remove( infos->Selection, item ); }
/**********************************************************************************************************/
/* Tout_deselectionner: Deselectionne tous les motifs actuellement selectionnés                           */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Tout_deselectionner ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    struct TRAME_ITEM_MOTIF      *trame_motif;
    struct TRAME_ITEM_PASS       *trame_pass;
    struct TRAME_ITEM_COMMENT    *trame_comm;
    struct TRAME_ITEM_CADRAN     *trame_cadran;
    struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    GSList *objet;

    objet = infos->Selection;
    while (objet)
     { switch( *((gint *)objet->data) )
        { case TYPE_MOTIF:
               trame_motif = (struct TRAME_ITEM_MOTIF *)objet->data;
               g_object_set( trame_motif->select_hg, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
               g_object_set( trame_motif->select_hd, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
               g_object_set( trame_motif->select_bg, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
               g_object_set( trame_motif->select_bd, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
               trame_motif->selection = FALSE;
               break;
          case TYPE_COMMENTAIRE:
               trame_comm = (struct TRAME_ITEM_COMMENT *)objet->data;
               g_object_set( trame_comm->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
               trame_comm->selection = FALSE;
               break;
          case TYPE_PASSERELLE:
               trame_pass = (struct TRAME_ITEM_PASS *)objet->data;
               g_object_set( trame_pass->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
               trame_pass->selection = FALSE;
               break;
          case TYPE_CADRAN:
               trame_cadran = (struct TRAME_ITEM_CADRAN *)objet->data;
               g_object_set( trame_cadran->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
               trame_cadran->selection = FALSE;
               break;
          case TYPE_CAMERA_SUP:
               trame_camera_sup = (struct TRAME_ITEM_CAMERA_SUP *)objet->data;
               g_object_set( trame_camera_sup->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
               trame_camera_sup->selection = FALSE;
               break;
          default: printf("Tout_deselectionner: type inconnu\n" );
        }
       objet=g_slist_next(objet);
     }
    g_slist_free ( infos->Selection );
    infos->Selection = NULL;
    printf("Fin deselectionner\n");
  }
/******************************************************************************************************************************/
/* Selectionner: Incorpore le groupe en parametre dans la liste des motifs selectionnés                                       */
/* Entrée: un numero de groupe, deselect=1 si on doit deselectionner les motifs qui sont selectionnes                         */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Selectionner ( struct PAGE_NOTEBOOK *page, gint layer )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    struct TRAME_ITEM_MOTIF      *trame_motif;
    struct TRAME_ITEM_PASS       *trame_pass;
    struct TRAME_ITEM_COMMENT    *trame_comm;
    struct TRAME_ITEM_CADRAN     *trame_cadran;
    struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    GList *objet;
    printf("Selectionner : Selectionner layer %d\n", layer );
    objet = infos->Trame_atelier->trame_items;
    while (objet)
     { switch ( *((gint *)objet->data) )                             /* Test du type de données dans data */
        { case TYPE_MOTIF:
               trame_motif = (struct TRAME_ITEM_MOTIF *)objet->data;
               if (trame_motif->layer == layer)
                { if (!trame_motif->selection)
                   { g_object_set( trame_motif->select_hg, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL );
                     g_object_set( trame_motif->select_hd, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL );
                     g_object_set( trame_motif->select_bg, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL );
                     g_object_set( trame_motif->select_bd, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL );
                     trame_motif->selection = TRUE;
                     infos->Selection = g_slist_prepend( infos->Selection, objet->data );
                   }
                }
               break;
          case TYPE_PASSERELLE:
               trame_pass = (struct TRAME_ITEM_PASS *)objet->data;
               if (trame_pass->layer == layer)
                { if (!trame_pass->selection)
                   { g_object_set( trame_pass->select_mi, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL );
                     trame_pass->selection = TRUE;
                     infos->Selection = g_slist_prepend( infos->Selection, objet->data );
                   }
                }
               break;
          case TYPE_COMMENTAIRE:
               trame_comm = (struct TRAME_ITEM_COMMENT *)objet->data;
               if (trame_comm->layer == layer)
                { if (!trame_comm->selection)
                   { g_object_set( trame_comm->select_mi, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL );
                     trame_comm->selection = TRUE;
                     infos->Selection = g_slist_prepend( infos->Selection, objet->data );
                   }
                 }
                break;
           case TYPE_CADRAN:
                trame_cadran = (struct TRAME_ITEM_CADRAN *)objet->data;
                if (trame_cadran->layer == layer)
                 { if (!trame_cadran->selection)
                    { g_object_set( trame_cadran->select_mi, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL );
                      trame_cadran->selection = TRUE;
                      infos->Selection = g_slist_prepend( infos->Selection, objet->data );
                    }
                 }
                break;
           case TYPE_CAMERA_SUP:
                trame_camera_sup = (struct TRAME_ITEM_CAMERA_SUP *)objet->data;
                if (trame_camera_sup->layer == layer)
                 { if (!trame_camera_sup->selection)
                    { g_object_set( trame_camera_sup->select_mi, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL );
                      trame_camera_sup->selection = TRUE;
                      infos->Selection = g_slist_prepend( infos->Selection, objet->data );
                    }
                 }
               break;
          default: printf("Selectionner: type inconnu\n" );
        }
       objet=objet->next;
     }
    printf("Fin selectionner\n");
  }
/******************************************************************************************************************************/
/* Deplacer_selection: Deplace toute la selection sur le synoptique                                                           */
/* Entrée: Les variations en pixels                                                                                           */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Deplacer_selection ( struct PAGE_NOTEBOOK *page, gint deltax, gint deltay )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    struct TRAME_ITEM_MOTIF      *trame_motif;
    struct TRAME_ITEM_PASS       *trame_pass;
    struct TRAME_ITEM_COMMENT    *trame_comm;
    struct TRAME_ITEM_CADRAN     *trame_cadran;
    struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    GSList *selection;
    gint largeur_grille;
    gint dx, dy, new_x, new_y;

    largeur_grille = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(infos->Spin_grid) );
    selection = infos->Selection;                                                        /* Pour tous les objets selectionnés */
    while(selection)
     { dx = deltax; dy = deltay;
       switch ( *((gint *)selection->data) )
        { case TYPE_PASSERELLE:
               trame_pass = ((struct TRAME_ITEM_PASS *)(selection->data));
               new_x = trame_pass->pass->position_x+dx;
               new_y = trame_pass->pass->position_y+dy;

               if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(infos->Check_grid)))
                { new_x = new_x/largeur_grille * largeur_grille;
                  new_y = new_y/largeur_grille * largeur_grille;
                }

               if ( 0<new_x && new_x < TAILLE_SYNOPTIQUE_X ) { trame_pass->pass->position_x = new_x; }
               if ( 0<new_y && new_y < TAILLE_SYNOPTIQUE_Y ) { trame_pass->pass->position_y = new_y; }
               Trame_rafraichir_passerelle(trame_pass);                                                     /* Refresh visuel */
               break;

          case TYPE_COMMENTAIRE:
               trame_comm = ((struct TRAME_ITEM_COMMENT *)(selection->data));
               new_x = trame_comm->comment->position_x+dx;
               new_y = trame_comm->comment->position_y+dy;

               if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(infos->Check_grid)))
                { new_x = new_x/largeur_grille * largeur_grille;
                  new_y = new_y/largeur_grille * largeur_grille;
                }

               if ( 0<new_x && new_x < TAILLE_SYNOPTIQUE_X )
                { trame_comm->comment->position_x = new_x; }
               if ( 0<new_y && new_y < TAILLE_SYNOPTIQUE_Y )
                { trame_comm->comment->position_y = new_y; }
               Trame_rafraichir_comment(trame_comm);                                                        /* Refresh visuel */
               break;

          case TYPE_CADRAN:
               trame_cadran = ((struct TRAME_ITEM_CADRAN *)(selection->data));
               new_x = trame_cadran->cadran->position_x+dx;
               new_y = trame_cadran->cadran->position_y+dy;

               if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(infos->Check_grid)))
                { new_x = new_x/largeur_grille * largeur_grille;
                  new_y = new_y/largeur_grille * largeur_grille;
                }

               if ( 0<new_x && new_x < TAILLE_SYNOPTIQUE_X )
                { trame_cadran->cadran->position_x = new_x; }
               if ( 0<new_y && new_y < TAILLE_SYNOPTIQUE_Y )
                { trame_cadran->cadran->position_y = new_y; }

               Trame_rafraichir_cadran(trame_cadran);                                                       /* Refresh visuel */
               break;

          case TYPE_MOTIF:
               trame_motif = ((struct TRAME_ITEM_MOTIF *)(selection->data));
               new_x = trame_motif->motif->position_x+dx;
               new_y = trame_motif->motif->position_y+dy;

               if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(infos->Check_grid)))
                { new_x = new_x/largeur_grille * largeur_grille;
                  new_y = new_y/largeur_grille * largeur_grille;
                }
printf("newx=%d, newy=%d\n", new_x, new_y);
               if ( 0<new_x && new_x<TAILLE_SYNOPTIQUE_X )
                { trame_motif->motif->position_x = new_x; }
               if ( 0<new_y && new_y<TAILLE_SYNOPTIQUE_Y )
                { trame_motif->motif->position_y = new_y; }
               Trame_rafraichir_motif(trame_motif);                                                         /* Refresh visuel */
               break;
          case TYPE_CAMERA_SUP:
               trame_camera_sup = ((struct TRAME_ITEM_CAMERA_SUP *)(selection->data));
               new_x = trame_camera_sup->camera_sup->posx+dx;
               new_y = trame_camera_sup->camera_sup->posy+dy;

               if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(infos->Check_grid)))
                { new_x = new_x/largeur_grille * largeur_grille;
                  new_y = new_y/largeur_grille * largeur_grille;
                }

               if ( 0<new_x && new_x<TAILLE_SYNOPTIQUE_X )
                { trame_camera_sup->camera_sup->posx = new_x; }
               if ( 0<new_y && new_y<TAILLE_SYNOPTIQUE_Y )
                { trame_camera_sup->camera_sup->posy = new_y; }
               Trame_rafraichir_camera_sup(trame_camera_sup);                                               /* Refresh visuel */
               break;
          default: printf("Deplacer_selection: type inconnu\n" );
        }
       selection = selection->next;
     }
  }
#ifdef bouh
/**********************************************************************************************************/
/* Dupliquer_selection: Envoi au serveur une demande de création pour chacun des objets selectionnés      */
/* Entrée: Rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Dupliquer_selection ( void )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    struct TRAME_ITEM_MOTIF      *trame_motif;
    struct TRAME_ITEM_PASS       *trame_pass;
    struct TRAME_ITEM_COMMENT    *trame_comm;
    struct TRAME_ITEM_CADRAN     *trame_cadran;
    struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    GList *selection;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    selection = infos->Selection.items;
    while(selection)                                                 /* Pour tous les objets selectionnés */
     { switch ( *((gint *)selection->data) )
        { case TYPE_PASSERELLE:
               trame_pass = ((struct TRAME_ITEM_PASS *)selection->data);
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_PASS,
                              (gchar *)trame_pass->pass, sizeof( struct CMD_TYPE_PASSERELLE ) );
               break;
          case TYPE_COMMENTAIRE:
               trame_comm = ((struct TRAME_ITEM_COMMENT *)selection->data);
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_COMMENT,
                              (gchar *)trame_comm->comment, sizeof( struct CMD_TYPE_COMMENT ) );
               break;
          case TYPE_MOTIF:
               trame_motif = ((struct TRAME_ITEM_MOTIF *)selection->data);
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_MOTIF,
                              (gchar *)trame_motif->motif, sizeof( struct CMD_TYPE_MOTIF ) );
               break;
          case TYPE_CADRAN:
               trame_cadran = ((struct TRAME_ITEM_CADRAN *)selection->data);
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_CADRAN,
                              (gchar *)trame_cadran->cadran, sizeof( struct CMD_TYPE_CADRAN ) );
               break;
          case TYPE_CAMERA_SUP:
               trame_camera_sup = ((struct TRAME_ITEM_CAMERA_SUP *)selection->data);
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_CAMERA_SUP,
                              (gchar *)trame_camera_sup->camera_sup, sizeof( struct CMD_TYPE_CAMERASUP ) );
               break;
          default: /*Selection = g_list_remove( Selection, Selection->data );*/
                   printf("Dupliquer_selection: type inconnu\n" );
        }
       selection = selection->next;
     }
  }
/**********************************************************************************************************/
/* Effacer_selection: Efface tous les elements de la selection en cours                                   */
/* Entrée: Rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Effacer_selection ( void )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    struct TRAME_ITEM_MOTIF      *trame_motif;
    struct TRAME_ITEM_PASS       *trame_pass;
    struct TRAME_ITEM_COMMENT    *trame_comm;
    struct TRAME_ITEM_CADRAN     *trame_cadran;
    struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    struct CMD_TYPE_MOTIF id_motif;
    struct CMD_TYPE_COMMENT id_comment;
    struct CMD_TYPE_CADRAN id_cadran;
    struct CMD_TYPE_PASSERELLE id_pass;
    struct CMD_TYPE_CAMERASUP id_camera_sup;
    GList *selection;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    selection = infos->Selection.items;
    while(selection)                                                 /* Pour tous les objets selectionnés */
     { switch ( *((gint *)selection->data) )
        { case TYPE_PASSERELLE:
               trame_pass = ((struct TRAME_ITEM_PASS *)selection->data);
               id_pass.id = trame_pass->pass->id;
               memcpy( &id_pass.libelle, trame_pass->pass->libelle, sizeof(id_pass.libelle) );
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_DEL_PASS,
                              (gchar *)&id_pass, sizeof( struct CMD_TYPE_PASSERELLE ) );
               break;
          case TYPE_COMMENTAIRE:
               trame_comm = ((struct TRAME_ITEM_COMMENT *)selection->data);
               id_comment.id = trame_comm->comment->id;
               memcpy( &id_comment.libelle, trame_comm->comment->libelle, sizeof(id_comment.libelle) );
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_DEL_COMMENT,
                              (gchar *)&id_comment, sizeof( struct CMD_TYPE_COMMENT ) );
               break;
          case TYPE_MOTIF:
               trame_motif = ((struct TRAME_ITEM_MOTIF *)selection->data);
               id_motif.id = trame_motif->motif->id;
               memcpy( &id_motif.libelle, trame_motif->motif->libelle, sizeof(id_motif.libelle) );
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_DEL_MOTIF,
                              (gchar *)&id_motif, sizeof( struct CMD_TYPE_MOTIF ) );
               break;
          case TYPE_CADRAN:
               trame_cadran = ((struct TRAME_ITEM_CADRAN *)selection->data);
               id_cadran.id = trame_cadran->cadran->id;
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_DEL_CADRAN,
                              (gchar *)&id_cadran, sizeof( struct CMD_TYPE_CADRAN ) );
               break;
          case TYPE_CAMERA_SUP:
               trame_camera_sup = ((struct TRAME_ITEM_CAMERA_SUP *)selection->data);
               id_camera_sup.id = trame_camera_sup->camera_sup->id;
               memcpy( &id_camera_sup.libelle, trame_camera_sup->camera_sup->libelle,
                       sizeof(id_camera_sup.libelle) );
               Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_DEL_CAMERA_SUP,
                              (gchar *)&id_camera_sup, sizeof( struct CMD_TYPE_CAMERASUP ) );
               break;
          default: /*Selection = g_list_remove( Selection, Selection->data );*/
                   printf("Effacer_selection: type inconnu\n" );
        }
       selection = selection->next;
     }
  }
/**********************************************************************************************************/
/* Fusionner_selection: Fusionne les elements selectionnés dans un meme groupe                            */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Fusionner_selection ( void )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    GList *selection;
    gint new_groupe;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    selection = infos->Selection.items;                              /* Pour tous les objets selectionnés */
    new_groupe = Nouveau_groupe();
    while(selection)
     { switch ( *((gint *)selection->data) )
        { case TYPE_PASSERELLE:
               ((struct TRAME_ITEM_PASS *)selection->data)->groupe_dpl = new_groupe;
               break;
          case TYPE_COMMENTAIRE:
               ((struct TRAME_ITEM_COMMENT *)selection->data)->groupe_dpl = new_groupe;
               break;
          case TYPE_MOTIF:
               ((struct TRAME_ITEM_MOTIF *)selection->data)->groupe_dpl = new_groupe;
               break;
          case TYPE_CADRAN:
               ((struct TRAME_ITEM_CADRAN *)selection->data)->groupe_dpl = new_groupe;
               break;
          case TYPE_CAMERA_SUP:
               ((struct TRAME_ITEM_CAMERA_SUP *)selection->data)->groupe_dpl = new_groupe;
               break;
          default: printf("Fusionner_selection: type inconnu\n" );
        }
       selection = selection->next;
     }
    printf("Fin fusionner_selection\n");
  }
/**********************************************************************************************************/
/* Detacher_selection: Detache les elements d'un meme groupe de election                                  */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Detacher_selection ( void )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;               /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    switch ( infos->Selection.type )
       { case TYPE_PASSERELLE:
              ((struct TRAME_ITEM_PASS *)infos->Selection.trame_pass)->groupe_dpl = Nouveau_groupe();
              break;
         case TYPE_COMMENTAIRE:
              ((struct TRAME_ITEM_COMMENT *)infos->Selection.trame_comment)->groupe_dpl = Nouveau_groupe();
              break;
         case TYPE_MOTIF:
              ((struct TRAME_ITEM_MOTIF *)infos->Selection.trame_motif)->groupe_dpl = Nouveau_groupe();
              break;
         case TYPE_CADRAN:
              ((struct TRAME_ITEM_CADRAN *)infos->Selection.trame_cadran)->groupe_dpl = Nouveau_groupe();
              break;
         case TYPE_CAMERA_SUP:
              ((struct TRAME_ITEM_CAMERA_SUP *)infos->Selection.trame_camera_sup)->groupe_dpl = Nouveau_groupe();
              break;
         default: printf("Detacher_selection: type inconnu\n" );
       }
    printf("Fin detacher_selection\n");
  }
#endif
/******************************************************************************************************************************/
/* Rotationner_selection: Fait tourner la selection                                                                           */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Rotationner_selection ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    GSList *selection;
    gfloat angle;

    angle = (gfloat) gtk_adjustment_get_value ( infos->Adj_angle );

    selection = infos->Selection;                                                        /* Pour tous les objets selectionnés */
    while(selection)
     { switch ( *((gint *)selection->data) )
       { case TYPE_MOTIF:
          { struct TRAME_ITEM_MOTIF *trame_motif = selection->data;
            trame_motif->motif->angle = angle;
            Trame_rafraichir_motif(trame_motif);
            break;
          }
         case TYPE_PASSERELLE:
          { struct TRAME_ITEM_PASS *trame_pass = selection->data;
            trame_pass->pass->angle = angle;
            Trame_rafraichir_passerelle(trame_pass);
            break;
          }
         case TYPE_COMMENTAIRE:
          { struct TRAME_ITEM_COMMENT *trame_comm = selection->data;
            trame_comm->comment->angle = angle;
            Trame_rafraichir_comment(trame_comm);
            break;
          }
         case TYPE_CADRAN:
          { struct TRAME_ITEM_CADRAN *trame_cadran = selection->data;
            trame_cadran->cadran->angle = angle;
            Trame_rafraichir_cadran(trame_cadran);
            break;
          }
         case TYPE_CAMERA_SUP: break;
         default: printf("Rotationner_selection: type inconnu\n" );
       }
      selection = g_slist_next(selection);
     }
  }
/******************************************************************************************************************************/
/* Mettre_echelle_selection_1_1 : Mise à l'echelle 1 des motifs selectionnés                                                  */
/* Entrée: Rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Mettre_echelle_selection_1_1 ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    GSList *selection;

    selection = infos->Selection;                                                        /* Pour tous les objets selectionnés */
    while(selection)
     { switch ( *((gint *)selection->data) )
        { case TYPE_MOTIF:
           { struct TRAME_ITEM_MOTIF *trame_motif = selection->data;
             trame_motif->motif->largeur = (gfloat)trame_motif->gif_largeur;
             trame_motif->motif->hauteur = (gfloat)trame_motif->gif_hauteur;
             Trame_rafraichir_motif(trame_motif);
             break;
           }
          default: printf("Mettre_echelle_selection_1_1: type non inconnu\n" );
        }
       selection = g_slist_next(selection);
     }
  }
/**********************************************************************************************************/
/* Mettre_echelle_selection_1_1 : Mise à l'echelle 1 des motifs selectionnés                              */
/* Entrée: Rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Mettre_echelle_selection_1_Y ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    GSList *selection;

    selection = infos->Selection;                                                        /* Pour tous les objets selectionnés */
    while(selection)
     { switch ( *((gint *)selection->data) )
        { case TYPE_MOTIF:
           { struct TRAME_ITEM_MOTIF *trame_motif = selection->data;
             trame_motif->motif->largeur = (gfloat)trame_motif->gif_largeur;
             Trame_rafraichir_motif(trame_motif);
             break;
           }
          default: printf("Mettre_echelle_selection_1_Y: type non inconnu\n" );
        }
       selection = selection->next;
     }
  }
/**********************************************************************************************************/
/* Mettre_echelle_selection_1_1 : Mise à l'echelle 1 des motifs selectionnés uniquement sur la hauteur    */
/* Entrée: Rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Mettre_echelle_selection_X_1 ( struct PAGE_NOTEBOOK *page )
  { struct TYPE_INFO_ATELIER *infos = page->infos;
    GSList *selection;

    selection = infos->Selection;                                                        /* Pour tous les objets selectionnés */
    while(selection)
     { switch ( *((gint *)selection->data) )
        { case TYPE_MOTIF:
           { struct TRAME_ITEM_MOTIF *trame_motif = selection->data;
             trame_motif->motif->hauteur = (gfloat)trame_motif->gif_hauteur;
             Trame_rafraichir_motif(trame_motif);
             break;
           }
          default: printf("Mettre_echelle_selection_X_1: type non inconnu\n" );
        }
       selection = selection->next;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
