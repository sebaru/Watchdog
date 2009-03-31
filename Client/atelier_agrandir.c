/**********************************************************************************************************/
/* Client/atelier_agrandir.c             gestion du resizing des motifs sur la trame                      */
/* Projet WatchDog version 1.6       Gestion d'habitat                       sam 20 déc 2003 17:13:29 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <gnome.h>
 #include <math.h>
 
 #include "trame.h"
 #include "motifs.h"

 #define PI 3.141592654

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* Agrandir_général: Fonction appelée quand on resize un motif                                            */
/* Entrée: une structure TRAME_ITEM_MOTIF                                                                 */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Agrandir_general ( struct TRAME_ITEM_MOTIF *trame_motif )
  { if (!(trame_motif && trame_motif->motif)) return;

    Trame_rafraichir_motif(trame_motif);
  }

/**********************************************************************************************************/
/* Agrandir_bd: Appelé quand un evenement est capté sur un carré de selection BD                          */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Agrandir_bd ( GooCanvasItem *widget, GooCanvasItem *target,
                    GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { static gdouble Clic_x, Clic_y;
    static gint Appui = 0;
    gdouble taille_x, taille_y, dx, dy;
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

       taille_x = trame_motif->motif->largeur + dx;
       taille_y = trame_motif->motif->hauteur + dy;
       if (   taille_x > (trame_motif->motif->gif_largeur>>1)
           && taille_y > (trame_motif->motif->gif_hauteur>>1)
           && taille_x < (gdouble)TAILLE_SYNOPTIQUE_X
           && taille_y < (gdouble)TAILLE_SYNOPTIQUE_Y )
        {
          trame_motif->motif->position_x += (dx_trame/2);         /* /2 car milieu de motif > dpl moindre */
          trame_motif->motif->position_y += (dy_trame/2);
          trame_motif->motif->largeur = taille_x;
          trame_motif->motif->hauteur = taille_y;

          Agrandir_general( trame_motif );         /* Met à jour les données des bases TRAME et visuelles */
        }
       Clic_x = event->motion.x_root;
       Clic_y = event->motion.y_root;
     }
  }
/**********************************************************************************************************/
/* Agrandir_bg: Appelé quand un evenement est capté sur un carré de selection BG                          */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Agrandir_bg ( GooCanvasItem *widget, GooCanvasItem *target,
                    GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { static gdouble Clic_x, Clic_y;
    static gint Appui = 0;
    gdouble taille_x, taille_y, dx, dy;
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

       taille_x = trame_motif->motif->largeur - dx;
       taille_y = trame_motif->motif->hauteur + dy;

       if (   taille_x > (trame_motif->motif->gif_largeur>>1)
           && taille_y > (trame_motif->motif->gif_hauteur>>1)
           && taille_x < (gdouble)TAILLE_SYNOPTIQUE_X
           && taille_y < (gdouble)TAILLE_SYNOPTIQUE_Y )
        {
          trame_motif->motif->position_x += (dx_trame/2);         /* /2 car milieu de motif > dpl moindre */
          trame_motif->motif->position_y += (dy_trame/2);
          trame_motif->motif->largeur = taille_x;
          trame_motif->motif->hauteur = taille_y;

          Agrandir_general( trame_motif );         /* Met à jour les données des bases TRAME et visuelles */
        }
       Clic_x = event->motion.x_root;
       Clic_y = event->motion.y_root;
     }
  }
/**********************************************************************************************************/
/* Agrandir_hg: Appelé quand un evenement est capté sur un carré de selection HG                          */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Agrandir_hg ( GooCanvasItem *widget, GooCanvasItem *target,
                    GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { static gdouble Clic_x, Clic_y;
    static gint Appui = 0;
    gdouble taille_x, taille_y, dx, dy;
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

       taille_x = trame_motif->motif->largeur - dx;
       taille_y = trame_motif->motif->hauteur - dy;

       if (   taille_x > (trame_motif->motif->gif_largeur>>1)
           && taille_y > (trame_motif->motif->gif_hauteur>>1)
           && taille_x < (gdouble)TAILLE_SYNOPTIQUE_X
           && taille_y < (gdouble)TAILLE_SYNOPTIQUE_Y )
        {
          trame_motif->motif->position_x += (dx_trame/2);         /* /2 car milieu de motif > dpl moindre */
          trame_motif->motif->position_y += (dy_trame/2);
          trame_motif->motif->largeur = taille_x;
          trame_motif->motif->hauteur = taille_y;

          Agrandir_general( trame_motif );         /* Met à jour les données des bases TRAME et visuelles */
        }
       Clic_x = event->motion.x_root;
       Clic_y = event->motion.y_root;
     }
  }
/**********************************************************************************************************/
/* Agrandir_hd: Appelé quand un evenement est capté sur un carré de selection HD                          */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Agrandir_hd ( GooCanvasItem *widget, GooCanvasItem *target,
                    GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { static gdouble Clic_x, Clic_y;
    static gint Appui = 0;
    gdouble taille_x, taille_y, dx, dy;
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

       taille_x = trame_motif->motif->largeur + dx;
       taille_y = trame_motif->motif->hauteur - dy;

       if (   taille_x > (trame_motif->motif->gif_largeur>>1)
           && taille_y > (trame_motif->motif->gif_hauteur>>1)
           && taille_x < (gdouble)TAILLE_SYNOPTIQUE_X
           && taille_y < (gdouble)TAILLE_SYNOPTIQUE_Y )
        {
          trame_motif->motif->position_x += (dx_trame/2);         /* /2 car milieu de motif > dpl moindre */
          trame_motif->motif->position_y += (dy_trame/2);
          trame_motif->motif->largeur = taille_x;
          trame_motif->motif->hauteur = taille_y;

          Agrandir_general( trame_motif );         /* Met à jour les données des bases TRAME et visuelles */
        }
       Clic_x = event->motion.x_root;
       Clic_y = event->motion.y_root;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
