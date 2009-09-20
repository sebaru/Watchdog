/**********************************************************************************************************/
/* Client/trame.c         gestion de la trame du synoptique                                               */
/* Projet WatchDog version 2.0      Gestion d'habitat                       dim 28 sep 2003 16:44:11 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * trame.c
 * This file is part of Watxhdog
 *
 * Copyright (C) 2008 - Sébastien Lefevre
 *
 * Watxhdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watxhdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watxhdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

 #define FACTEUR_PI 3.141592654/180.0

 #include <gnome.h>                                                             /* Bibliothèque graphique */
 #include <gdk-pixbuf/gdk-pixbuf.h>                                          /* Gestion des images/motifs */
 #include <gdk-pixbuf/gdk-pixdata.h>                                         /* Gestion des images/motifs */
 #include <gdk/gdkx.h>
 #include <gst/gst.h>
 #include <gst/interfaces/xoverlay.h>
 #include <goocanvas.h>                                                            /* Interface GooCanvas */
 #include <gif_lib.h>
 #include <string.h>
 #include <stdlib.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>

 #include "trame.h"
 #include "motifs.h"

#define DEBUG_TRAME
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/****************************** Inclusion des images XPM pour les menus ***********************************/
 extern GdkBitmap *Rmask, *Bmask, *Vmask, *Omask, *Jmask;
 extern GdkPixmap *Rouge, *Bleue, *Verte, *Orange, *Jaune;

/**********************************************************************************************************/
/* Reduire_en_vignette: Met un motif aux dimensions de vignette                                           */
/* Entrée: Le trame_motif souhaité                                                                        */
/* Sortie: niet                                                                                           */
/**********************************************************************************************************/
 void Reduire_en_vignette ( struct MOTIF *motif )
  { if ( motif && (motif->largeur>TAILLE_ICONE_X || motif->hauteur>TAILLE_ICONE_Y ) )
     { double facteur;
       facteur = (gdouble)motif->hauteur/motif->largeur;
       printf("Reduire en vignette: facteur = %f\n", facteur );
       if (facteur>=1.0)
        { motif->largeur = (guint)((double)TAILLE_ICONE_Y/facteur);
          motif->hauteur = TAILLE_ICONE_Y;
        }
       else
        { motif->largeur = TAILLE_ICONE_X;
          motif->hauteur = (guint)((double)TAILLE_ICONE_X*facteur);
        }
       printf("Reduire en vignette : new=%f, new_y=%f\n", motif->largeur, motif->hauteur );
     }
  }
/**********************************************************************************************************/
/* Trame_new_item: Renvoi un nouveau item, completement vierge                                            */
/* Entrée: kedal                                                                                          */
/* Sortie: une structure TRAME_ITEM_MOTIF                                                                 */
/**********************************************************************************************************/
 struct TRAME_ITEM_MOTIF *Trame_new_item ( void )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    trame_motif = g_malloc0( sizeof(struct TRAME_ITEM_MOTIF) );
    trame_motif->num_image  = 0;
    trame_motif->cligno     = 0;                                        /* Par defaut, on ne clignote pas */
    trame_motif->images     = NULL;
    return(trame_motif);
  }
/**********************************************************************************************************/
/* Trame_del_item: Renvoi un nouveau item, completement vierge                                            */
/* Entrée: un item                                                                                        */
/* Sortie: rieng                                                                                          */
/**********************************************************************************************************/
 void Trame_del_item ( struct TRAME_ITEM_MOTIF *trame_motif )
  { if (trame_motif->item_groupe) goo_canvas_item_remove( trame_motif->item_groupe );
    if (trame_motif->select_hd) goo_canvas_item_remove( trame_motif->select_hd );
    if (trame_motif->select_hg) goo_canvas_item_remove( trame_motif->select_hg );
    if (trame_motif->select_bd) goo_canvas_item_remove( trame_motif->select_bd );
    if (trame_motif->select_bg) goo_canvas_item_remove( trame_motif->select_bg );
    g_list_foreach( trame_motif->images, (GFunc) gdk_pixbuf_unref /*g_free*/, NULL );
    g_list_free( trame_motif->images );
    if (trame_motif->pixbuf) gdk_pixbuf_unref(trame_motif->pixbuf);
  }
/**********************************************************************************************************/
/* Trame_del_item: Renvoi un nouveau item, completement vierge                                            */
/* Entrée: un item                                                                                        */
/* Sortie: rieng                                                                                          */
/**********************************************************************************************************/
 void Trame_del_camera_sup ( struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup )
  {
    if (trame_camera_sup->pipeline)
     { gst_element_set_state (trame_camera_sup->pipeline, GST_STATE_NULL);      /* Extinction du pipeline */
       gst_object_unref( GST_OBJECT(trame_camera_sup->pipeline) );
     }
    if (trame_camera_sup->item_groupe) goo_canvas_item_remove( trame_camera_sup->item_groupe );
    if (trame_camera_sup->select_mi) goo_canvas_item_remove( trame_camera_sup->select_mi );
  }
/**********************************************************************************************************/
/* Trame_del_item: Renvoi un nouveau item, completement vierge                                            */
/* Entrée: un item                                                                                        */
/* Sortie: rieng                                                                                          */
/**********************************************************************************************************/
 void Trame_del_capteur ( struct TRAME_ITEM_CAPTEUR *trame_capteur )
  { if (trame_capteur->item_groupe) goo_canvas_item_remove( trame_capteur->item_groupe );
  }
/**********************************************************************************************************/
/* Trame_del_item: Renvoi un nouveau item, completement vierge                                            */
/* Entrée: un item                                                                                        */
/* Sortie: rieng                                                                                          */
/**********************************************************************************************************/
 void Trame_del_passerelle ( struct TRAME_ITEM_PASS *trame_pass )
  { if (trame_pass->item_groupe) goo_canvas_item_remove( trame_pass->item_groupe );
  }
/**********************************************************************************************************/
/* Trame_del_item: Renvoi un nouveau item, completement vierge                                            */
/* Entrée: un item                                                                                        */
/* Sortie: rieng                                                                                          */
/**********************************************************************************************************/
 void Trame_del_commentaire ( struct TRAME_ITEM_COMMENT *trame_comm )
  { if (trame_comm->item_groupe) goo_canvas_item_remove( trame_comm->item_groupe );
  }
/**********************************************************************************************************/
/* Trame_rafraichir_motif: remet à jour la position, rotation, echelle du motif en parametre              */
/* Entrée: la structure graphique TRAME_MOTIF                                                             */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Trame_rafraichir_motif ( struct TRAME_ITEM_MOTIF *trame_motif )
  { if (!(trame_motif && trame_motif->motif && trame_motif->image)) return;

    cairo_matrix_init_identity ( &trame_motif->transform );
    cairo_matrix_translate ( &trame_motif->transform,
                             (gdouble)trame_motif->motif->position_x,
                             (gdouble)trame_motif->motif->position_y
                           );

    cairo_matrix_rotate ( &trame_motif->transform, (gdouble)trame_motif->motif->angle*FACTEUR_PI );
    cairo_matrix_scale  ( &trame_motif->transform,
                           (gdouble)trame_motif->motif->largeur/trame_motif->motif->gif_largeur,
                           (gdouble)trame_motif->motif->hauteur/trame_motif->motif->gif_hauteur
                        );

    goo_canvas_item_set_transform ( trame_motif->item_groupe, &trame_motif->transform );

    if ( trame_motif->select_hd)
     {
       cairo_matrix_init_identity ( &trame_motif->transform_hd );
       cairo_matrix_translate ( &trame_motif->transform_hd,
                                (gdouble)trame_motif->motif->position_x,
                                (gdouble)trame_motif->motif->position_y
                              );
       cairo_matrix_rotate ( &trame_motif->transform_hd, (gdouble)trame_motif->motif->angle*FACTEUR_PI );
       cairo_matrix_translate ( &trame_motif->transform_hd,
                                ((gdouble)trame_motif->motif->largeur/2),
                                -((gdouble)trame_motif->motif->hauteur/2) - 9
                              );
       goo_canvas_item_set_transform ( trame_motif->select_hd, &trame_motif->transform_hd );

       cairo_matrix_init_identity ( &trame_motif->transform_bd );
       cairo_matrix_translate ( &trame_motif->transform_bd,
                                (gdouble)trame_motif->motif->position_x,
                                (gdouble)trame_motif->motif->position_y
                              );
       cairo_matrix_rotate ( &trame_motif->transform_bd, (gdouble)trame_motif->motif->angle*FACTEUR_PI );
       cairo_matrix_translate ( &trame_motif->transform_bd,
                                ((gdouble)trame_motif->motif->largeur/2),
                                ((gdouble)trame_motif->motif->hauteur/2)
                              );
       goo_canvas_item_set_transform ( trame_motif->select_bd, &trame_motif->transform_bd );

       cairo_matrix_init_identity ( &trame_motif->transform_hg );
       cairo_matrix_translate ( &trame_motif->transform_hg,
                                (gdouble)trame_motif->motif->position_x,
                                (gdouble)trame_motif->motif->position_y
                              );
       cairo_matrix_rotate ( &trame_motif->transform_hg, (gdouble)trame_motif->motif->angle*FACTEUR_PI );
       cairo_matrix_translate ( &trame_motif->transform_hg,
                                -((gdouble)trame_motif->motif->largeur/2) - 9,
                                -((gdouble)trame_motif->motif->hauteur/2) - 9
                              );
       goo_canvas_item_set_transform ( trame_motif->select_hg, &trame_motif->transform_hg );

       cairo_matrix_init_identity ( &trame_motif->transform_bg );
       cairo_matrix_translate ( &trame_motif->transform_bg,
                                (gdouble)trame_motif->motif->position_x,
                                (gdouble)trame_motif->motif->position_y
                              );
       cairo_matrix_rotate ( &trame_motif->transform_bg, (gdouble)trame_motif->motif->angle*FACTEUR_PI );
       cairo_matrix_translate ( &trame_motif->transform_bg,
                                -((gdouble)trame_motif->motif->largeur/2) - 9,
                                ((gdouble)trame_motif->motif->hauteur/2)
                              );
       goo_canvas_item_set_transform ( trame_motif->select_bg, &trame_motif->transform_bg );
     }
  }
/**********************************************************************************************************/
/* Trame_rafraichir_motif: remet à jour la position, rotation, echelle du motif en parametre              */
/* Entrée: la structure graphique TRAME_MOTIF                                                             */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Trame_rafraichir_camera_sup ( struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup )
  { if (!(trame_camera_sup && trame_camera_sup->camera_sup)) return;

    cairo_matrix_init_identity ( &trame_camera_sup->transform );
    cairo_matrix_translate ( &trame_camera_sup->transform,
                             (gdouble)trame_camera_sup->camera_sup->position_x,
                             (gdouble)trame_camera_sup->camera_sup->position_y
                           );

    cairo_matrix_rotate ( &trame_camera_sup->transform, 0.0 );
    cairo_matrix_scale  ( &trame_camera_sup->transform, 1.0, 1.0 );

    goo_canvas_item_set_transform ( trame_camera_sup->item_groupe, &trame_camera_sup->transform );
  }
/**********************************************************************************************************/
/* Trame_rafraichir_motif: remet à jour la position, rotation, echelle du motif en parametre              */
/* Entrée: la structure graphique TRAME_MOTIF                                                             */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Trame_rafraichir_comment ( struct TRAME_ITEM_COMMENT *trame_comment )
  { if (!(trame_comment && trame_comment->comment)) return;

    cairo_matrix_init_identity ( &trame_comment->transform );
    cairo_matrix_translate ( &trame_comment->transform,
                             (gdouble)trame_comment->comment->position_x,
                             (gdouble)trame_comment->comment->position_y
                           );

    cairo_matrix_rotate ( &trame_comment->transform, (gdouble)trame_comment->comment->angle*FACTEUR_PI );
    cairo_matrix_scale  ( &trame_comment->transform, 1.0, 1.0 );

    goo_canvas_item_set_transform ( trame_comment->item_groupe, &trame_comment->transform );
  }
/**********************************************************************************************************/
/* Trame_rafraichir_motif: remet à jour la position, rotation, echelle du motif en parametre              */
/* Entrée: la structure graphique TRAME_MOTIF                                                             */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Trame_rafraichir_passerelle ( struct TRAME_ITEM_PASS *trame_pass )
  { if (!(trame_pass && trame_pass->pass)) return;

    cairo_matrix_init_identity ( &trame_pass->transform );
    cairo_matrix_translate ( &trame_pass->transform,
                             (gdouble)trame_pass->pass->position_x,
                             (gdouble)trame_pass->pass->position_y
                           );

    cairo_matrix_rotate ( &trame_pass->transform, (gdouble)trame_pass->pass->angle*FACTEUR_PI );
    cairo_matrix_scale  ( &trame_pass->transform, 1.0, 1.0 );

    goo_canvas_item_set_transform ( trame_pass->item_groupe, &trame_pass->transform );
  }
/**********************************************************************************************************/
/* Trame_peindre_motif: Peint un motif de la couleur selectionnée                                         */
/* Entrée: une structure TRAME_ITEM_MOTIF, la couleur de reference                                        */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Trame_peindre_pass_1 ( struct TRAME_ITEM_PASS *trame_pass, guchar r, guchar v, guchar b )
  { guint couleur;

    if (!(trame_pass && trame_pass->pass)) return;
    couleur = ((guint)r<<24) + ((guint)v<<16) + ((guint)b<<8) + 0xFF;
    g_object_set( G_OBJECT(trame_pass->item_rectangle_1), "fill_color_rgba", couleur, NULL );

    trame_pass->en_cours_rouge1 = r;                        /* Sauvegarde de la couleur actuelle du motif */
    trame_pass->en_cours_vert1  = v;
    trame_pass->en_cours_bleu1  = b;
  }
/**********************************************************************************************************/
/* Trame_peindre_motif: Peint un motif de la couleur selectionnée                                         */
/* Entrée: une structure TRAME_ITEM_MOTIF, la couleur de reference                                        */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Trame_peindre_pass_2 ( struct TRAME_ITEM_PASS *trame_pass, guchar r, guchar v, guchar b )
  { guint couleur;

    if (!(trame_pass && trame_pass->pass)) return;
    couleur = ((guint)r<<24) + ((guint)v<<16) + ((guint)b<<8) + 0xFF;
    g_object_set( G_OBJECT(trame_pass->item_rectangle_2), "fill_color_rgba", couleur, NULL );

    trame_pass->en_cours_rouge2 = r;                        /* Sauvegarde de la couleur actuelle du motif */
    trame_pass->en_cours_vert2  = v;
    trame_pass->en_cours_bleu2  = b;
  }
/**********************************************************************************************************/
/* Trame_rafraichir_capteur: remet à jour la position, rotation, echelle du capteur en parametre              */
/* Entrée: la structure graphique TRAME_ITEM_CAPTEUR                                                        */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Trame_rafraichir_capteur ( struct TRAME_ITEM_CAPTEUR *trame_capteur )
  { if (!(trame_capteur && trame_capteur->capteur)) return;

    cairo_matrix_init_identity ( &trame_capteur->transform );
    cairo_matrix_translate ( &trame_capteur->transform,
                             (gdouble)trame_capteur->capteur->position_x,
                             (gdouble)trame_capteur->capteur->position_y
                           );

    cairo_matrix_rotate ( &trame_capteur->transform, (gdouble)trame_capteur->capteur->angle*FACTEUR_PI );
    cairo_matrix_scale  ( &trame_capteur->transform, 1.0, 1.0 );

    goo_canvas_item_set_transform ( trame_capteur->item_groupe, &trame_capteur->transform );
  }
/**********************************************************************************************************/
/* Trame_peindre_motif: Peint un motif de la couleur selectionnée                                         */
/* Entrée: une structure TRAME_ITEM_MOTIF, la couleur de reference                                        */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Trame_peindre_motif ( struct TRAME_ITEM_MOTIF *trame_motif, guchar r, guchar v, guchar b )
  { guint x, max, base;
    guchar *buffer;

    if (!(trame_motif && trame_motif->motif && trame_motif->image)) return;
    max = trame_motif->motif->gif_largeur*trame_motif->motif->gif_hauteur;
#ifdef DEBUG_TRAME
    printf("Trame_peindre_motif: gif_largeur=%d  gif_hauteur=%d  r=%d,v=%d,b=%d\n",
            trame_motif->motif->gif_largeur, trame_motif->motif->gif_hauteur, r, v, b );
#endif       

    if (trame_motif->pixbuf) gdk_pixbuf_unref(trame_motif->pixbuf);
    trame_motif->pixbuf = gdk_pixbuf_copy( (GdkPixbuf *)(trame_motif->image->data) );
    buffer = gdk_pixbuf_get_pixels( trame_motif->pixbuf );

    if ( gdk_pixbuf_get_has_alpha( trame_motif->pixbuf ) )                  /* y a-t-il un canal alpha ?? */
     { for (x=0; x<max; x++)
        { base = x<<2;
          if (buffer[ base+0 ] == buffer[ base+1 ] &&
              buffer[ base+1 ] == buffer[ base+2 ] &&
              buffer[ base+2 ] == COULEUR_ACTIVE) 
           {  buffer[ base+0 ] = r;
              buffer[ base+1 ] = v;
              buffer[ base+2 ] = b;
           }
        }
     }
    else
     { for (x=0; x<max; x++)
        { base = x*3;
          if (buffer[ base+0 ] == buffer[ base+1 ] &&
              buffer[ base+1 ] == buffer[ base+2 ] &&
              buffer[ base+2 ] == COULEUR_ACTIVE) 
           {  buffer[ base+0 ] = r;
              buffer[ base+1 ] = v;
              buffer[ base+2 ] = b;
           }
        }
     }

    if (trame_motif->item) g_object_set( trame_motif->item, "pixbuf", trame_motif->pixbuf, NULL );
    trame_motif->en_cours_rouge = r;                        /* Sauvegarde de la couleur actuelle du motif */
    trame_motif->en_cours_vert  = v;
    trame_motif->en_cours_bleu  = b;
  }
/**********************************************************************************************************/
/* Trame_choisir_frame: Choisit une frame parmi celles du motif                                           */
/* Entrée: une structure TRAME_ITEM_MOTIF, le numero de frame voulue                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Trame_choisir_frame ( struct TRAME_ITEM_MOTIF *trame_motif, gint num, guchar r, guchar v, guchar b )
  { GList *frame;
    
    if (!(trame_motif && trame_motif->motif)) { printf ("Niet\n"); return; }

    if (trame_motif->num_image == num )                                         /* Frame deja affichée ?? */
     { Trame_peindre_motif( trame_motif, r, v, b );
       return;
     }

    frame = g_list_nth( trame_motif->images, num );
    if (!frame) { frame = trame_motif->images;                                      /* Bouclage si erreur */
                  num = 0;
                }

    trame_motif->image = frame;
    trame_motif->num_image = num;
    Trame_peindre_motif( trame_motif, r, v, b );
  }

/**********************************************************************************************************/
/* Charger_pixbuf: Tente de charger un ensemble de pixbuf representant un icone                           */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference         */
/* Sortie: reussite                                                                                       */
/**********************************************************************************************************/
 void Charger_pixbuf_file ( struct TRAME_ITEM_MOTIF *trame_item, gchar *fichier )
  { gint cpt_frame, source;
    gchar from_fichier[80];
    GdkPixbuf *pixbuf;

    trame_item->images = NULL;

    pixbuf = gdk_pixbuf_new_from_file ( fichier, NULL );                            /* Creation du pixbuf */
    if (!pixbuf)                                                   /* Si chargement impossible, on arrete */
     { pixbuf = gdk_pixbuf_new_from_file ( "default.gif", NULL ); }                 /* Creation du pixbuf */

printf("Charger_pixbuf_file: %s\n", fichier );

    trame_item->motif->gif_largeur = gdk_pixbuf_get_width ( pixbuf );
    trame_item->motif->gif_hauteur = gdk_pixbuf_get_height (pixbuf );

    trame_item->images = g_list_append( trame_item->images, pixbuf );        /* Et ajout dans la liste */
    trame_item->image  = trame_item->images;                             /* Synchro sur image numero 1 */
    trame_item->nbr_images = 1;

    for (cpt_frame=1; ; cpt_frame++)
     {
       g_snprintf( from_fichier, sizeof(from_fichier), "%s.%02d", fichier, cpt_frame );
printf("Charger_pixbuf_file: test ouverture %s\n", from_fichier );

       source = open( from_fichier, O_RDONLY );
       if (source<0) return;
       else { close(source);
              trame_item->nbr_images++;
            }
     }
  }
/**********************************************************************************************************/
/* Charger_pixbuf: Tente de charger un ensemble de pixbuf representant un icone                           */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference         */
/* Sortie: reussite                                                                                       */
/**********************************************************************************************************/
 static void Charger_pixbuf_id ( struct TRAME_ITEM_MOTIF *trame_item, guint icone_id )
  { guint i;

    trame_item->image  = NULL;
    trame_item->images = NULL;
    trame_item->nbr_images = 0;

    trame_item->motif->gif_largeur = 0;
    trame_item->motif->gif_hauteur = 0;

    for ( i=0; ; i++ )
     { char nom_fichier[80];
       GdkPixbuf *pixbuf;

       if (i) g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.gif.%02d", icone_id, i );
       else   g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.gif", icone_id );
     
#ifdef DEBUG_TRAME
printf("Charger_pixbuf: %s\n", nom_fichier );
#endif

       pixbuf = gdk_pixbuf_new_from_file ( nom_fichier, NULL );                     /* Creation du pixbuf */
#ifdef DEBUG_TRAME
printf("Charger_pixbuf: %s -> pixbuf = %p\n", nom_fichier, pixbuf );
#endif
       if (!pixbuf)
        { if (i == 0)                                              /* Si chargement impossible, on arrete */
           { pixbuf = gdk_pixbuf_new_from_file ( "default.gif", NULL ); }           /* Creation du pixbuf */
          else break;
        }

       trame_item->motif->gif_largeur = gdk_pixbuf_get_width ( pixbuf );
       trame_item->motif->gif_hauteur = gdk_pixbuf_get_height( pixbuf );

#ifdef DEBUG_TRAME
printf("Charger_pixbuf: w/h %d, %d alpha %d\n", trame_item->motif->gif_largeur,
                                                trame_item->motif->gif_hauteur,
                                                gdk_pixbuf_get_has_alpha(pixbuf) );
#endif

       trame_item->images = g_list_append( trame_item->images, pixbuf );        /* Et ajout dans la liste */
       trame_item->image  = trame_item->images;                             /* Synchro sur image numero 1 */
       trame_item->nbr_images++;
     }
  }
/**********************************************************************************************************/
/* Trame_ajout_motif: Ajoute un motif sur le visuel                                                       */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference         */
/* Sortie: reussite                                                                                       */
/**********************************************************************************************************/
 struct TRAME_ITEM_MOTIF *Trame_ajout_motif ( gint flag, struct TRAME *trame,
                                              struct MOTIF *motif )
  { struct TRAME_ITEM_MOTIF *trame_motif;

    if (!(trame && motif)) return(NULL);
    trame_motif = Trame_new_item();
    if (!trame_motif) return(NULL);

    trame_motif->motif = motif;

    Charger_pixbuf_id( trame_motif, motif->icone_id );
/*    Charger_gif( trame_motif, nom_fichier ); */

    if (!trame_motif->images)                                              /* En cas de probleme, on sort */
     { Trame_del_item(trame_motif);
       g_free(trame_motif);
       return(NULL);
     }
    Trame_peindre_motif( trame_motif, motif->rouge0, motif->vert0, motif->bleu0 );
#ifdef DEBUG_TRAME
printf("New motif: largeur %f haut%f\n", motif->largeur, motif->hauteur );
#endif
    trame_motif->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );         /* Groupe MOTIF */
    trame_motif->item = goo_canvas_image_new ( trame_motif->item_groupe,
                                               trame_motif->pixbuf,
                                               (-(gdouble)(motif->gif_largeur/2)),
                                               (-(gdouble)(motif->gif_hauteur/2)),
                                               NULL );

    if (!motif->largeur) motif->largeur = motif->gif_largeur;
    if (!motif->hauteur) motif->hauteur = motif->gif_hauteur;

    if ( flag )
     { GdkPixbuf *pixbuf;

       pixbuf = gdk_pixbuf_new_from_file( "fleche_hg.gif", NULL );
       trame_motif->select_hg = goo_canvas_image_new ( trame->canvas_root,
                                                       pixbuf, 0.0, 0.0,
                                                       NULL );
       gdk_pixbuf_unref(pixbuf);

       pixbuf = gdk_pixbuf_new_from_file( "fleche_hd.gif", NULL );
       trame_motif->select_hd = goo_canvas_image_new ( trame->canvas_root,
                                                       pixbuf, 0.0, 0.0,
                                                       NULL );
       gdk_pixbuf_unref(pixbuf);

       pixbuf = gdk_pixbuf_new_from_file( "fleche_bg.gif", NULL );
       trame_motif->select_bg = goo_canvas_image_new ( trame->canvas_root,
                                                       pixbuf, 0.0, 0.0,
                                                       NULL );
       gdk_pixbuf_unref(pixbuf);

       pixbuf = gdk_pixbuf_new_from_file( "fleche_bd.gif", NULL );
       trame_motif->select_bd = goo_canvas_image_new ( trame->canvas_root,
                                                       pixbuf, 0.0, 0.0,
                                                       NULL );
       gdk_pixbuf_unref(pixbuf);

       g_object_set( trame_motif->select_hg, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
       g_object_set( trame_motif->select_hd, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
       g_object_set( trame_motif->select_bg, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
       g_object_set( trame_motif->select_bd, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
     }

    Trame_rafraichir_motif ( trame_motif );

    trame_motif->type = TYPE_MOTIF;
    trame->trame_items = g_list_append( trame->trame_items, trame_motif );
    if (trame_motif->motif->type_gestion == TYPE_FOND)
     { goo_canvas_item_lower( trame_motif->item, NULL );
       goo_canvas_item_lower( trame->fond, NULL );
     }
    return(trame_motif);
  }
/**********************************************************************************************************/
/* Trame_ajout_camera_sup: Ajoute un camera_sup sur le visuel                                             */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference         */
/* Sortie: reussite                                                                                       */
/**********************************************************************************************************/
 struct TRAME_ITEM_CAMERA_SUP *Trame_ajout_camera_sup ( gint flag, struct TRAME *trame,
                                                        struct CMD_TYPE_CAMERA_SUP *camera_sup )
  { struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    GstElement *source, *jpegdec, *ffmpeg, *sink, *scale;

    if (!(trame && camera_sup)) return(NULL);

    trame_camera_sup = g_malloc0( sizeof(struct TRAME_ITEM_CAMERA_SUP) );
    if (!trame_camera_sup) return(NULL);

    trame_camera_sup->camera_sup = camera_sup;

    trame_camera_sup->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );    /* Groupe MOTIF */

    if ( flag )
     { gchar chaine[256];
       if ( camera_sup->type == CAMERA_MODE_ICONE )
        { trame_camera_sup->item = goo_canvas_rect_new( trame_camera_sup->item_groupe,
                                                        -55.0, -15.0, 110.0, 30.0,
                                                        "fill-color", "blue",
                                                        "stroke-color", "yellow",
                                                        NULL);

        }
       else if (camera_sup->type == CAMERA_MODE_INCRUSTATION )
        { trame_camera_sup->item = goo_canvas_rect_new( trame_camera_sup->item_groupe,
                                                        -DEFAULT_CAMERA_LARGEUR/2, -DEFAULT_CAMERA_HAUTEUR/2,
                                                        DEFAULT_CAMERA_LARGEUR, DEFAULT_CAMERA_HAUTEUR,
                                                        "fill-color", "blue",
                                                        "stroke-color", "yellow",
                                                        NULL);

        }
       g_snprintf( chaine, sizeof(chaine), "CAM%03d", trame_camera_sup->camera_sup->camera_src_id );
       goo_canvas_text_new ( trame_camera_sup->item_groupe, chaine, 0.0, 0.0,
                                                         -1, GTK_ANCHOR_CENTER,
                                                         "fill-color", "yellow",
                                                         "font", "arial bold 14",
                                                         NULL);
       trame_camera_sup->select_mi = goo_canvas_rect_new (trame_camera_sup->item_groupe,
                                                          -5.0, -5.0, 10.0, 10.0,
                                                          "fill_color", "green",
                                                          "stroke_color", "black", NULL);

       g_object_set( trame_camera_sup->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
     }
    else
     { gchar chaine[256];
       if ( camera_sup->type == CAMERA_MODE_ICONE )
        { trame_camera_sup->item = goo_canvas_rect_new( trame_camera_sup->item_groupe,
                                                        -55.0, -15.0, 110.0, 30.0,
                                                        "fill-color", "blue",
                                                        "stroke-color", "yellow",
                                                        NULL);

          g_snprintf( chaine, sizeof(chaine), "CAM%03d", trame_camera_sup->camera_sup->camera_src_id );
          goo_canvas_text_new ( trame_camera_sup->item_groupe, chaine, 0.0, 0.0,
                                                            -1, GTK_ANCHOR_CENTER,
                                                            "fill-color", "yellow",
                                                            "font", "arial bold 14",
                                                         NULL);
        }
       else if (camera_sup->type == CAMERA_MODE_INCRUSTATION )
        { trame_camera_sup->video_output = gtk_drawing_area_new ();
          trame_camera_sup->item = goo_canvas_widget_new( trame_camera_sup->item_groupe,
                                                          trame_camera_sup->video_output,
                                                          -DEFAULT_CAMERA_LARGEUR/2.0,
                                                          -DEFAULT_CAMERA_HAUTEUR/2.0,
                                                           DEFAULT_CAMERA_LARGEUR*1.0,
                                                           DEFAULT_CAMERA_HAUTEUR*1.0,
                                                          NULL);
          /* Create gstreamer elements */
          trame_camera_sup->pipeline = gst_pipeline_new (NULL);

          source  = gst_element_factory_make ( "gnomevfssrc", NULL );
          g_object_set (G_OBJECT (source), "location", trame_camera_sup->camera_sup->location, NULL);

          jpegdec  = gst_element_factory_make ("jpegdec", NULL);
          ffmpeg   = gst_element_factory_make ("ffmpegcolorspace", NULL );
          scale    = gst_element_factory_make ("videoscale", NULL );
          sink     = gst_element_factory_make ("ximagesink", NULL );

          gst_bin_add_many (GST_BIN (trame_camera_sup->pipeline), source, jpegdec, ffmpeg, scale, sink, NULL);
          gst_element_link_many (source, jpegdec, ffmpeg, scale, sink, NULL);
 
          gst_x_overlay_handle_events (GST_X_OVERLAY (sink), FALSE);
          gtk_widget_realize ( trame_camera_sup->video_output );
          gtk_main_iteration_do( TRUE );
          gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (sink),
                                        GDK_WINDOW_XWINDOW (trame_camera_sup->video_output->window));
        }
     }

    Trame_rafraichir_camera_sup ( trame_camera_sup );

    trame_camera_sup->type = TYPE_CAMERA_SUP;
    trame->trame_items = g_list_append( trame->trame_items, trame_camera_sup );
    return(trame_camera_sup);
  }
/**********************************************************************************************************/
/* Trame_ajout_motif: Ajoute un motif sur le visuel                                                       */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference         */
/* Sortie: reussite                                                                                       */
/**********************************************************************************************************/
 void Trame_ajout_motif_par_item ( struct TRAME *trame,
                                   struct TRAME_ITEM_MOTIF *trame_motif )
  { trame_motif->image = trame_motif->images;

    Trame_choisir_frame( trame_motif, 0, trame_motif->motif->rouge0,
                                      trame_motif->motif->vert0,
                                      trame_motif->motif->bleu0 );
#ifdef DEBUG_TRAME
printf("New motif par item: %f %f\n", trame_motif->motif->largeur, trame_motif->motif->hauteur );
#endif
    trame_motif->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );         /* Groupe MOTIF */

    trame_motif->item = goo_canvas_image_new ( trame_motif->item_groupe,
                                               trame_motif->pixbuf,
                                               (-(gdouble)(trame_motif->motif->gif_largeur/2)),
                                               (-(gdouble)(trame_motif->motif->gif_hauteur/2)),
                                               NULL );

    trame_motif->type = TYPE_MOTIF;                                  /* Il s'agit d'un item de type motif */
    Trame_rafraichir_motif ( trame_motif );                                    /* Rafraichissement visuel */

    trame->trame_items = g_list_append( trame->trame_items, trame_motif );
  }
/**********************************************************************************************************/
/* Trame_ajout_commentaire: Ajoute un commentaire sur le visuel                                           */
/* Entrée: une structure commentaire, la trame de reference                                               */
/* Sortie: reussite                                                                                       */
/**********************************************************************************************************/
 struct TRAME_ITEM_COMMENT *Trame_ajout_commentaire ( gint flag, struct TRAME *trame, struct COMMENTAIRE *comm )
  { struct TRAME_ITEM_COMMENT *trame_comm;
    guint couleur;

    if (!(trame && comm)) return(NULL);
    trame_comm = g_malloc0( sizeof(struct TRAME_ITEM_COMMENT) );
    if (!trame_comm) return(NULL);
    couleur = ((guint)comm->rouge<<24) + ((guint)comm->vert<<16) + ((guint)comm->bleu<<8) + 0xFF;

#ifdef DEBUG_TRAME
printf("New comment %s %s \n", comm->libelle, comm->font );
#endif
    trame_comm->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );        /* Groupe COMMENT */

    trame_comm->item = goo_canvas_text_new ( trame_comm->item_groupe,
                                             comm->libelle, 0.0, 0.0, -1, GTK_ANCHOR_CENTER,
                                               "font", comm->font,
                                               "fill_color_rgba", couleur,
                                               NULL );
    trame_comm->comment = comm;
    trame_comm->type = TYPE_COMMENTAIRE;
    trame->trame_items = g_list_append( trame->trame_items, trame_comm );

    if ( flag )
     { trame_comm->select_mi = goo_canvas_rect_new (trame_comm->item_groupe,
                                                    -5.0, -5.0, 10.0, 10.0,
                                                    "fill_color", "green",
                                                    "stroke_color", "black", NULL);
       g_object_set( trame_comm->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
     }

    Trame_rafraichir_comment ( trame_comm );
    return(trame_comm);
  }
/**********************************************************************************************************/
/* Trame_ajout_passerelle: Ajoute une passerelle sur le visuel                                            */
/* Entrée: une structure passerelle, la trame de reference                                                */
/* Sortie: reussite                                                                                       */
/**********************************************************************************************************/
 struct TRAME_ITEM_PASS *Trame_ajout_passerelle ( gint flag, struct TRAME *trame, struct PASSERELLE *pass )
  { struct TRAME_ITEM_PASS *trame_pass;
    gdouble taillex, tailley;
       
    if (!(trame && pass)) return(NULL);
    trame_pass = g_malloc0( sizeof(struct TRAME_ITEM_PASS) );
    if (!trame_pass) return(NULL);

    trame_pass->pass = pass;

    trame_pass->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );     /* Groupe PASSERELLE */
    trame_pass->item_texte = goo_canvas_text_new( trame_pass->item_groupe,
                                                  pass->libelle, 10.0, 0.0,
                                                  -1, GTK_ANCHOR_WEST,
                                                  "font", "fixed bold 14",
                                                  "fill-color", "white",
                                                  NULL);

    trame_pass->item_rectangle_1 = goo_canvas_rect_new( trame_pass->item_groupe,
                                                        -7.0, -8.0, 12.0, 6.0,
                                                        "fill-color", "green",
                                                        "stroke-color", "yellow",
                                                        NULL);

    trame_pass->item_rectangle_2 = goo_canvas_rect_new( trame_pass->item_groupe,
                                                        -7.0, +2.0, 12.0, 6.0,
                                                        "fill-color", "red",
                                                        "stroke-color", "yellow",
                                                        NULL);

/*    g_object_get( G_OBJECT(trame_pass->item_texte), "height", &tailley, "width", &taillex, NULL );
printf(" dX = %d dY = %d \n", taillex, tailley );*/
    tailley = 15;
    taillex = strlen(pass->libelle) * 11;
    trame_pass->item_fond = goo_canvas_rect_new( trame_pass->item_groupe,
                                                 (double)-15.0,
                                                 (double)-(tailley/2+5.0),
                                                 (double)+taillex+30.0,
                                                 (double)(tailley+10),
                                                 "stroke-color", "yellow",
                                                 "fill-color", "blue",
                                                 NULL);

    goo_canvas_item_lower ( trame_pass->item_fond, NULL );

    if ( flag )
     { trame_pass->select_mi = goo_canvas_rect_new (trame_pass->item_groupe,
                                                    +10.0, -2.5, 5.0, 5.0,
                                                    "fill_color", "green",
                                                    "stroke_color", "black", NULL);
       g_object_set( trame_pass->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
     }


    Trame_rafraichir_passerelle ( trame_pass );

    trame_pass->type = TYPE_PASSERELLE;
    trame->trame_items = g_list_append( trame->trame_items, trame_pass );

    return(trame_pass);
  }
/**********************************************************************************************************/
/* Trame_ajout_capteur: Ajoute un capteur sur le visuel                                                       */
/* Entrée: une structure capteur, la trame de reference                                                     */
/* Sortie: reussite                                                                                       */
/**********************************************************************************************************/
 struct TRAME_ITEM_CAPTEUR *Trame_ajout_capteur ( gint flag, struct TRAME *trame, struct CAPTEUR *capteur )
  { struct TRAME_ITEM_CAPTEUR *trame_capteur;
       
    if (!(trame && capteur)) return(NULL);
    trame_capteur = g_malloc0( sizeof(struct TRAME_ITEM_CAPTEUR) );
    if (!trame_capteur) return(NULL);

    trame_capteur->capteur = capteur;

    trame_capteur->item_groupe = goo_canvas_group_new ( trame->canvas_root,             /* Groupe capteur */
                                                        NULL);

    trame_capteur->item_carre = goo_canvas_rect_new (trame_capteur->item_groupe,
                                                     -55.0, -15.0, 110.0, 30.0,
                                                     "fill_color", "gray",
                                                     "stroke_color", "green", NULL);

    trame_capteur->item_entry = goo_canvas_text_new ( trame_capteur->item_groupe,
                                                      "- capteur -", 0.0, 0.0,
                                                      -1, GTK_ANCHOR_CENTER,
                                                      "font", "arial italic 12",
                                                      NULL);
    if ( flag )
     { trame_capteur->select_mi = goo_canvas_rect_new (trame_capteur->item_groupe,
                                                       -2.5, -2.5, 7.5, 7.5,
                                                       "fill_color", "green",
                                                       "stroke_color", "black", NULL);
       g_object_set( trame_capteur->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
     }

    Trame_rafraichir_capteur ( trame_capteur );

    trame_capteur->type = TYPE_CAPTEUR;
    trame->trame_items = g_list_append( trame->trame_items, trame_capteur );

    return(trame_capteur);
  }
/**********************************************************************************************************/
/* Trame_creer_trame: Creation d'une nouvelle trame                                                       */
/* Entrée: les tailles x, y et la couleur de fond                                                         */
/* Sortie: un widget GTK                                                                                  */
/**********************************************************************************************************/
 struct TRAME *Trame_creer_trame ( guint taille_x, guint taille_y, char *coul, guint grille )
  { struct TRAME *trame;
    gdouble x, y;

    trame = g_malloc0( sizeof(struct TRAME) );
    if (!trame) return(NULL);

    trame->trame_widget = goo_canvas_new();
    g_object_set( trame->trame_widget, "background-color", coul, "anchor", GTK_ANCHOR_CENTER, NULL );
    goo_canvas_set_bounds (GOO_CANVAS (trame->trame_widget), 0, 0, taille_x, taille_y);
    trame->canvas_root = goo_canvas_get_root_item (GOO_CANVAS (trame->trame_widget));
    trame->fond = goo_canvas_rect_new (trame->canvas_root,
                                       0.0, 0.0, (double) taille_x, (double) taille_y,
                                         "stroke_color", "yellow", NULL);
    goo_canvas_item_lower( GOO_CANVAS_ITEM(trame->fond), NULL );

    if (grille)
     { for ( x=grille; x<taille_x; x+=grille )
        { for ( y=grille; y<taille_y; y+=grille )
           { goo_canvas_polyline_new_line (trame->canvas_root, x, y-1.0, x, y+1.0, 
                                           "stroke_color", "dark_blue", NULL );

             goo_canvas_polyline_new_line (trame->canvas_root, x-1.0, y, x+1.0, y, 
                                           "stroke_color", "darkblue", NULL );
           }
        }
     }

    return(trame);
  }

/**********************************************************************************************************/
/* Trame_effacer_trame: Efface la trame en parametre                                                      */
/* Entrée: la trame voulue                                                                                */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Trame_effacer_trame ( struct TRAME *trame )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct TRAME_ITEM_COMMENT *trame_comm;
    struct TRAME_ITEM_PASS *trame_pass;
    struct TRAME_ITEM_CAPTEUR *trame_capteur;
    struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    GList *objet;

    objet = trame->trame_items;                          /* Destruction des items du synoptique precedent */
    while(objet)
     { printf("Trame_effacer_trame: objet = %p data=%p  type=%d\n", objet, objet->data, *((gint *)objet->data) );
       switch ( *((gint *)objet->data) )
        { case TYPE_PASSERELLE:
                            trame_pass = (struct TRAME_ITEM_PASS *)objet->data;
                            Trame_del_passerelle( trame_pass );
                            g_free(trame_pass);
                            break;
          case TYPE_CAPTEUR:  trame_capteur = (struct TRAME_ITEM_CAPTEUR *)objet->data;
                            Trame_del_capteur( trame_capteur );
                            g_free(trame_capteur);
                            break;
          case TYPE_COMMENTAIRE:
                            trame_comm = (struct TRAME_ITEM_COMMENT *)objet->data;
                            Trame_del_commentaire( trame_comm );
                            g_free(trame_comm);
                            break;
          case TYPE_MOTIF:  trame_motif = (struct TRAME_ITEM_MOTIF *)objet->data;
                            Trame_del_item( trame_motif );
                            g_free(trame_motif);
                            break;
          case TYPE_CAMERA_SUP:
                            trame_camera_sup = (struct TRAME_ITEM_CAMERA_SUP *)objet->data;
                            Trame_del_camera_sup( trame_camera_sup );
                            g_free(trame_camera_sup);
                            break;
          default: printf("Trame_effacer_trame: type inconnu\n");
        }
       objet = objet->next;
     }
    g_list_free( trame->trame_items );                                /* Raz de la g_list correspondantes */
    trame->trame_items = NULL;
  }

/**********************************************************************************************************/
/* Trame_detruire_trame: Destruction d'une trame                                                          */
/* Entrée: la trame voulue                                                                                */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Trame_detruire_trame ( struct TRAME *trame )
  { if (!trame) return;
    Trame_effacer_trame ( trame );
    gtk_widget_destroy( trame->trame_widget );
    g_free(trame);
  }
/*--------------------------------------------------------------------------------------------------------*/
