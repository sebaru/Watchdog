/******************************************************************************************************************************/
/* Client/atelier_agrandir.c             gestion du resizing des motifs sur la trame                                          */
/* Projet WatchDog version 1.6       Gestion d'habitat                                           sam 20 déc 2003 17:13:29 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_agrandir.c
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
 #include <math.h>
/****************************************** Définitions des prototypes programme **********************************************/
 #include "protocli.h"

 #define PI 3.141592654

/******************************************************************************************************************************/
/* Agrandir_general_motif: Mise a jour des données de base                                                                    */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 static void Agrandir_general_motif ( struct TRAME_ITEM_MOTIF *trame_motif, gdouble dx, gdouble dy,
                                      gdouble dposx, gdouble dposy )
  {
       if (   trame_motif->motif->largeur + dx > (trame_motif->gif_largeur>>1)
           && trame_motif->motif->hauteur + dy > (trame_motif->gif_hauteur>>1)
           && trame_motif->motif->largeur + dx < (gdouble)TAILLE_SYNOPTIQUE_X
           && trame_motif->motif->hauteur + dy < (gdouble)TAILLE_SYNOPTIQUE_Y )
        {
          trame_motif->motif->position_x += (dposx/2);            /* /2 car milieu de motif > dpl moindre */
          trame_motif->motif->position_y += (dposy/2);
          trame_motif->motif->largeur    += dx;
          trame_motif->motif->hauteur    += dy;

          Trame_rafraichir_motif(trame_motif);
        }
  }
/******************************************************************************************************************************/
/* Agrandir_bd: Appelé quand un evenement est capté sur un carré de selection BD                                              */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Agrandir_bd ( GooCanvasItem *widget, GooCanvasItem *target,
                    GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { static gdouble Clic_x, Clic_y;
    static gint Appui = 0;
    gdouble dx, dy;
    gdouble dx_trame, dy_trame;
    gdouble cosinus, sinus;

    if (!(trame_motif && event)) return;
    if (event->type == GDK_BUTTON_PRESS)
     { Clic_x = event->button.x_root;
       Clic_y = event->button.y_root;
       Appui = 1;
     }
    else if (event->type == GDK_BUTTON_RELEASE)
     { Appui = 0;
     }
    else if (event->type == GDK_MOTION_NOTIFY && (event->motion.state & 0x100) && Appui)
     { dx_trame = (event->motion.x_root - Clic_x);
       dy_trame = (event->motion.y_root - Clic_y);
       cosinus  = cos(trame_motif->motif->angle*PI/180);
       sinus    = sin(trame_motif->motif->angle*PI/180);

       dx = dx_trame*cosinus + dy_trame*sinus;
       dy =-dx_trame*sinus   + dy_trame*cosinus;

       Agrandir_general_motif( trame_motif, dx, dy, dx_trame, dy_trame );
       Clic_x = event->motion.x_root;
       Clic_y = event->motion.y_root;
     }
  }
/******************************************************************************************************************************/
/* Agrandir_bg: Appelé quand un evenement est capté sur un carré de selection BG                                              */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Agrandir_bg ( GooCanvasItem *widget, GooCanvasItem *target,
                    GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { static gdouble Clic_x, Clic_y;
    static gint Appui = 0;
    gdouble dx, dy;
    gdouble dx_trame, dy_trame;
    gdouble cosinus, sinus;

    if (!(trame_motif && event)) return;

    if (event->type == GDK_BUTTON_PRESS)
     { Clic_x = event->button.x_root;
       Clic_y = event->button.y_root;
       Appui = 1;
     }
    else if (event->type == GDK_BUTTON_RELEASE)
     { Appui = 0;
     }
    else if (event->type == GDK_MOTION_NOTIFY && (event->motion.state & 0x100) && Appui)
     { dx_trame = (event->motion.x_root - Clic_x);
       dy_trame = (event->motion.y_root - Clic_y);
       cosinus  = cos(trame_motif->motif->angle*PI/180);
       sinus    = sin(trame_motif->motif->angle*PI/180);

       dx = dx_trame*cosinus + dy_trame*sinus;
       dy =-dx_trame*sinus   + dy_trame*cosinus;

       Agrandir_general_motif( trame_motif, -dx, dy, dx_trame, dy_trame );
       Clic_x = event->motion.x_root;
       Clic_y = event->motion.y_root;
     }
  }
/******************************************************************************************************************************/
/* Agrandir_hg: Appelé quand un evenement est capté sur un carré de selection HG                                              */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Agrandir_hg ( GooCanvasItem *widget, GooCanvasItem *target,
                    GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { static gdouble Clic_x, Clic_y;
    static gint Appui = 0;
    gdouble dx, dy;
    gdouble dx_trame, dy_trame;
    gdouble cosinus, sinus;

    if (!(trame_motif && event)) return;

    if (event->type == GDK_BUTTON_PRESS)
     { Clic_x = event->button.x_root;
       Clic_y = event->button.y_root;
       Appui = 1;
     }
    else if (event->type == GDK_BUTTON_RELEASE)
     { Appui = 0;
     }
    else if (event->type == GDK_MOTION_NOTIFY && (event->motion.state & 0x100) && Appui)
     { dx_trame = (event->motion.x_root - Clic_x);
       dy_trame = (event->motion.y_root - Clic_y);
       cosinus  = cos(trame_motif->motif->angle*PI/180);
       sinus    = sin(trame_motif->motif->angle*PI/180);

       dx = dx_trame*cosinus + dy_trame*sinus;
       dy =-dx_trame*sinus   + dy_trame*cosinus;

       Agrandir_general_motif( trame_motif, -dx, -dy, dx_trame, dy_trame );
       Clic_x = event->motion.x_root;
       Clic_y = event->motion.y_root;
     }
  }
/******************************************************************************************************************************/
/* Agrandir_hd: Appelé quand un evenement est capté sur un carré de selection HD                                              */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Agrandir_hd ( GooCanvasItem *widget, GooCanvasItem *target,
                    GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { static gdouble Clic_x, Clic_y;
    static gint Appui = 0;
    gdouble dx, dy;
    gdouble dx_trame, dy_trame;
    gdouble cosinus, sinus;

    if (!(trame_motif && event)) return;

    if (event->type == GDK_BUTTON_PRESS)
     { Clic_x = event->button.x_root;
       Clic_y = event->button.y_root;
       Appui = 1;
     }
    else if (event->type == GDK_BUTTON_RELEASE)
     { Appui = 0;
     }
    else if (event->type == GDK_MOTION_NOTIFY && (event->motion.state & 0x100) && Appui)
     { dx_trame = (event->motion.x_root - Clic_x);
       dy_trame = (event->motion.y_root - Clic_y);
       cosinus  = cos(trame_motif->motif->angle*PI/180);
       sinus    = sin(trame_motif->motif->angle*PI/180);

       dx = dx_trame*cosinus + dy_trame*sinus;
       dy =-dx_trame*sinus   + dy_trame*cosinus;

       Agrandir_general_motif( trame_motif, dx, -dy, dx_trame, dy_trame );
       Clic_x = event->motion.x_root;
       Clic_y = event->motion.y_root;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
