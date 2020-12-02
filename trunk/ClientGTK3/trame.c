/******************************************************************************************************************************/
/* Client/trame.c         gestion de la trame du synoptique                                                                   */
/* Projet WatchDog version 3.0      Gestion d'habitat                                           dim 28 sep 2003 16:44:11 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * trame.c
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
 * along with Watxhdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #define FACTEUR_PI 3.141592654/180.0

 #include <gtk/gtk.h>                                                                               /* Bibliothèque graphique */
 #include <gdk-pixbuf/gdk-pixbuf.h>                                          /* Gestion des images/motifs */
 #include <gdk-pixbuf/gdk-pixdata.h>                                         /* Gestion des images/motifs */
 #include <goocanvas.h>                                                            /* Interface GooCanvas */
 #include <gif_lib.h>
 #include <string.h>
 #include <stdlib.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>

 #include "trame.h"
/* #define DEBUG_TRAME*/
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"
 #include "client.h"

/**********************************************************************************************************/
/* Reduire_en_vignette: Met un motif aux dimensions de vignette                                           */
/* Entrée: Le trame_motif souhaité                                                                        */
/* Sortie: niet                                                                                           */
/**********************************************************************************************************/
 void Reduire_en_vignette ( struct CMD_TYPE_MOTIF *motif )
  { if ( motif && (motif->largeur>TAILLE_ICONE_X || motif->hauteur>TAILLE_ICONE_Y ) )
     { double facteur;
       facteur = (gdouble)motif->hauteur/motif->largeur;
       if (facteur>=1.0)
        { motif->largeur = (guint)((double)TAILLE_ICONE_Y/facteur);
          motif->hauteur = TAILLE_ICONE_Y;
        }
       else
        { motif->largeur = TAILLE_ICONE_X;
          motif->hauteur = (guint)((double)TAILLE_ICONE_X*facteur);
        }
     }
  }
/**********************************************************************************************************/
/* Trame_new_item: Renvoi un nouveau item, completement vierge                                            */
/* Entrée: kedal                                                                                          */
/* Sortie: une structure TRAME_ITEM_MOTIF                                                                 */
/**********************************************************************************************************/
 struct TRAME_ITEM_MOTIF *Trame_new_item ( void )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    trame_motif = g_try_malloc0( sizeof(struct TRAME_ITEM_MOTIF) );
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
    g_list_foreach( trame_motif->images, (GFunc) g_object_unref /*g_free*/, NULL );
    g_list_free( trame_motif->images );
    if (trame_motif->pixbuf) g_object_unref(trame_motif->pixbuf);
  }
/**********************************************************************************************************/
/* Trame_del_item: Renvoi un nouveau item, completement vierge                                            */
/* Entrée: un item                                                                                        */
/* Sortie: rieng                                                                                          */
/**********************************************************************************************************/
 void Trame_del_camera_sup ( struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup )
  {
    if (trame_camera_sup->item_groupe) goo_canvas_item_remove( trame_camera_sup->item_groupe );
    if (trame_camera_sup->select_mi) goo_canvas_item_remove( trame_camera_sup->select_mi );
  }
/******************************************************************************************************************************/
/* Trame_del_svg: Libere la mémoire liée à un objet de type SVG                                                               */
/* Entrée: le SVG                                                                                                             */
/* Sortie: rieng                                                                                                              */
/******************************************************************************************************************************/
 void Trame_del_SVG ( struct TRAME_ITEM_SVG *trame_svg )
  { if (!trame_svg) return;
    trame_svg->trame->Liste_timer = g_slist_remove ( trame_svg->trame->Liste_timer, trame_svg );/* Désactive la gestion clignotement */
    g_free(trame_svg);
  }
/******************************************************************************************************************************/
/* Trame_del_item: Renvoi un nouveau item, completement vierge                                                                */
/* Entrée: un item                                                                                                            */
/* Sortie: rieng                                                                                                              */
/******************************************************************************************************************************/
 void Trame_del_cadran ( struct TRAME_ITEM_CADRAN *trame_cadran )
  { if (trame_cadran->item_groupe) goo_canvas_item_remove( trame_cadran->item_groupe );
  }
/******************************************************************************************************************************/
/* Trame_del_item: Renvoi un nouveau item, completement vierge                                                                */
/* Entrée: un item                                                                                                            */
/* Sortie: rieng                                                                                                              */
/******************************************************************************************************************************/
 void Trame_del_passerelle ( struct TRAME_ITEM_PASS *trame_pass )
  { goo_canvas_item_remove( trame_pass->item_groupe );
    Trame_del_SVG (trame_pass->item_1);                                                  /* Désactive la gestion clignotement */
    Trame_del_SVG (trame_pass->item_2);                                                  /* Désactive la gestion clignotement */
    Trame_del_SVG (trame_pass->item_3);                                                  /* Désactive la gestion clignotement */
  }
/**********************************************************************************************************/
/* Trame_del_item: Renvoi un nouveau item, completement vierge                                            */
/* Entrée: un item                                                                                        */
/* Sortie: rieng                                                                                          */
/**********************************************************************************************************/
 void Trame_del_commentaire ( struct TRAME_ITEM_COMMENT *trame_comm )
  { if (trame_comm->item_groupe) goo_canvas_item_remove( trame_comm->item_groupe );
  }
/******************************************************************************************************************************/
/* Trame_rafraichir_motif: remet à jour la position, rotation, echelle du motif en parametre                                  */
/* Entrée: la structure graphique TRAME_MOTIF                                                                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Trame_rafraichir_motif ( struct TRAME_ITEM_MOTIF *trame_motif )
  { if (!(trame_motif && trame_motif->motif)) return;
printf("Trame_rafraichir_motif : posx=%d, posy=%d\n", trame_motif->motif->position_x, trame_motif->motif->position_y );
    cairo_matrix_init_identity ( &trame_motif->transform );
    cairo_matrix_translate ( &trame_motif->transform,
                             (gdouble)trame_motif->motif->position_x,
                             (gdouble)trame_motif->motif->position_y
                           );
    cairo_matrix_rotate ( &trame_motif->transform, (gdouble)trame_motif->motif->angle*FACTEUR_PI );
    cairo_matrix_scale  ( &trame_motif->transform,
                           (gdouble)trame_motif->motif->largeur/trame_motif->gif_largeur,
                           (gdouble)trame_motif->motif->hauteur/trame_motif->gif_hauteur
                        );
    goo_canvas_item_set_transform ( trame_motif->item_groupe, &trame_motif->transform );

    if (trame_motif->select_hd)
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
/******************************************************************************************************************************/
/* Trame_rafraichir_motif: remet à jour la position, rotation, echelle du motif en parametre                                  */
/* Entrée: la structure graphique TRAME_MOTIF                                                                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Trame_rafraichir_camera_sup ( struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup )
  { if (!(trame_camera_sup && trame_camera_sup->camera_sup)) return;
    cairo_matrix_init_identity ( &trame_camera_sup->transform );
    cairo_matrix_translate ( &trame_camera_sup->transform,
                             (gdouble)trame_camera_sup->camera_sup->posx,
                             (gdouble)trame_camera_sup->camera_sup->posy
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
/******************************************************************************************************************************/
/* Trame_rafraichir_cadran: remet à jour la position, rotation, echelle du cadran en parametre                                */
/* Entrée: la structure graphique TRAME_ITEM_CADRAN                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Trame_rafraichir_cadran ( struct TRAME_ITEM_CADRAN *trame_cadran )
  { if (!(trame_cadran && trame_cadran->cadran)) return;

    cairo_matrix_init_identity ( &trame_cadran->transform );
    cairo_matrix_translate ( &trame_cadran->transform,
                             (gdouble)trame_cadran->cadran->position_x,
                             (gdouble)trame_cadran->cadran->position_y
                           );

    cairo_matrix_rotate ( &trame_cadran->transform, (gdouble)trame_cadran->cadran->angle*FACTEUR_PI );
    cairo_matrix_scale  ( &trame_cadran->transform, 1.0, 1.0 );
    goo_canvas_item_set_transform ( trame_cadran->item_groupe, &trame_cadran->transform );
  }
/**********************************************************************************************************/
/* Trame_peindre_motif: Peint un motif de la couleur selectionnée                                         */
/* Entrée: une structure TRAME_ITEM_MOTIF, la couleur de reference                                        */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Trame_peindre_motif ( struct TRAME_ITEM_MOTIF *trame_motif, gchar *color )
  { guint x, max, base;
    guchar *buffer;

    /*printf("%s: %s:%s => %s\n", __func__, trame_motif->motif->tech_id, trame_motif->motif->acronyme, color );*/
    if (!(trame_motif && trame_motif->motif && trame_motif->image)) return;
    max = trame_motif->gif_largeur*trame_motif->gif_hauteur;

    if (trame_motif->pixbuf) g_object_unref(trame_motif->pixbuf);
    trame_motif->pixbuf = gdk_pixbuf_copy( (GdkPixbuf *)(trame_motif->image->data) );
    buffer = gdk_pixbuf_get_pixels( trame_motif->pixbuf );

    gint rouge = 0, vert = 0, bleu = 0;
         if (!strcasecmp(color, "red"))       { rouge = 255; vert =   0; bleu =   0; }
    else if (!strcasecmp(color, "lime"))      { rouge =   0; vert = 255; bleu =   0; }
    else if (!strcasecmp(color, "blue"))      { rouge =   0; vert =   0; bleu = 255; }
    else if (!strcasecmp(color, "yellow"))    { rouge = 255; vert = 255; bleu =   0; }
    else if (!strcasecmp(color, "orange"))    { rouge = 255; vert = 190; bleu =   0; }
    else if (!strcasecmp(color, "white"))     { rouge = 255; vert = 255; bleu = 255; }
    else if (!strcasecmp(color, "lightgray")) { rouge = 127; vert = 127; bleu = 127; }
    else if (!strcasecmp(color, "brown"))     { rouge =   0; vert = 100; bleu =   0; }
    else if (!strcasecmp(color, "kaki"))      { rouge = 100; vert = 100; bleu = 100; }
    else if (!strcasecmp(color, "black"))     { rouge =   0; vert =   0; bleu =   0; }
    else if (!strcasecmp(color, "#000"))      { rouge =   0; vert =   0; bleu =   0; }
    else if (!strcasecmp(color, "#4A4A4A"))   { rouge = 0x4A; vert = 0x4A; bleu = 0x4A; }
    else if (!strcasecmp(color, "#4D4D4D"))   { rouge = 0x4D; vert = 0x4D; bleu = 0x4D; }
    else if (!strcasecmp(color, "#A9A9A9"))   { rouge = 0xA9; vert = 0xA9; bleu = 0xA9; }
    else if (!strcasecmp(color, "#FFFFFF"))   { rouge = 255; vert = 255; bleu = 255; }
    else if (!strcasecmp(color, "#1A1A1A"))   { rouge = 0x1A; vert = 0x1A; bleu = 0x1A; }
    else if (!strcasecmp(color, "#7F7F7F"))   { rouge = 0x7F; vert = 0x7F; bleu = 0x7F; }
    else if (!strcasecmp(color, "#FFFF0"))    { rouge = 0xFF; vert = 0xFF; bleu = 0x00; }
    else if (!strcasecmp(color, "#ADD8E6"))   { rouge = 0xAD; vert = 0xD8; bleu = 0xE6; }
    else if (!strcasecmp(color, "#A52A2A"))   { rouge = 0xA5; vert = 0x2A; bleu = 0x2A; }
    else if (!strcasecmp(color, "#8B6914"))   { rouge = 0x8B; vert = 0x69; bleu = 0x14; }
    else if (!strcasecmp(color, "#0FFFF"))    { rouge = 0x00; vert = 0xFF; bleu = 0xFF; }
    else if (!strcasecmp(color, "#FF00"))     { rouge = 0xFF; vert = 0x00; bleu = 0x00; }
    else if (!strcasecmp(color, "#00FF"))     { rouge = 0x00; vert = 0x00; bleu = 0xFF; }
    else if (!strcasecmp(color, "#BFBFBF"))   { rouge = 0xBF; vert = 0xBF; bleu = 0xBF; }
    else if (!strcasecmp(color, "#0FFFC"))    { rouge = 0x00; vert = 0xFF; bleu = 0xFC; }
    else if (!strcasecmp(color, "#0FEFE"))    { rouge = 0x00; vert = 0xFE; bleu = 0xFE; }
    else printf("%s: color '%s' unknown\n", __func__, color);

    if ( gdk_pixbuf_get_has_alpha( trame_motif->pixbuf ) )                  /* y a-t-il un canal alpha ?? */
     { for (x=0; x<max; x++)
        { base = x<<2;
          if (buffer[ base+0 ] == buffer[ base+1 ] &&
              buffer[ base+1 ] == buffer[ base+2 ] &&
              buffer[ base+2 ] == COULEUR_ACTIVE)
           {  buffer[ base+0 ] = rouge;
              buffer[ base+1 ] = vert;
              buffer[ base+2 ] = bleu;
           }
        }
     }
    else
     { for (x=0; x<max; x++)
        { base = x*3;
          if (buffer[ base+0 ] == buffer[ base+1 ] &&
              buffer[ base+1 ] == buffer[ base+2 ] &&
              buffer[ base+2 ] == COULEUR_ACTIVE)
           {  buffer[ base+0 ] = rouge;
              buffer[ base+1 ] = vert;
              buffer[ base+2 ] = bleu;
           }
        }
     }

    if (trame_motif->item) g_object_set( trame_motif->item, "pixbuf", trame_motif->pixbuf, NULL );
    g_snprintf( trame_motif->en_cours_color, sizeof(trame_motif->en_cours_color), "%s", color );
  }
/******************************************************************************************************************************/
/* Trame_choisir_frame: Choisit une frame parmi celles du motif                                                               */
/* Entrée: une structure TRAME_ITEM_MOTIF, le numero de frame voulue                                                          */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Trame_choisir_frame ( struct TRAME_ITEM_MOTIF *trame_motif, gint num, gchar *color )
  { GList *frame;

    if (!(trame_motif && trame_motif->motif)) { printf ("Niet\n"); return; }

    frame = g_list_nth( trame_motif->images, num );
    if (!frame) { frame = trame_motif->images;                                                          /* Bouclage si erreur */
                  num = 0;
                }

    trame_motif->image = frame;
    trame_motif->num_image = num;
    Trame_peindre_motif( trame_motif, color );
  }

/******************************************************************************************************************************/
/* Charger_pixbuf: Tente de charger un ensemble de pixbuf representant un icone                                               */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference                             */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 void Charger_pixbuf_file ( struct TRAME_ITEM_MOTIF *trame_item, gchar *fichier )
  { gint cpt_frame, source;
    gchar from_fichier[256];
    GdkPixbuf *pixbuf;

    trame_item->images = NULL;

    pixbuf = gdk_pixbuf_new_from_file ( fichier, NULL );                            /* Creation du pixbuf */
    if (!pixbuf)                                                   /* Si chargement impossible, on arrete */
     { pixbuf = gdk_pixbuf_new_from_file ( "default.gif", NULL ); }                 /* Creation du pixbuf */

printf("Charger_pixbuf_file: %s\n", fichier );

    trame_item->gif_largeur = gdk_pixbuf_get_width ( pixbuf );
    trame_item->gif_hauteur = gdk_pixbuf_get_height (pixbuf );

    trame_item->images = g_list_append( trame_item->images, pixbuf );        /* Et ajout dans la liste */
    trame_item->image  = trame_item->images;                             /* Synchro sur image numero 1 */
    trame_item->nbr_images = 1;

    for (cpt_frame=1; ; cpt_frame++)
     {
       g_snprintf( from_fichier, sizeof(from_fichier), "%s.%02d", fichier, cpt_frame );
       source = open( from_fichier, O_RDONLY );
       if (source<0) return;
       else { close(source);
              trame_item->nbr_images++;
            }
     }
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Satellite_Receive_response : Recupere la reponse du serveur (master)                                                       */
/* Entrée : Les informations à sauvegarder                                                                                    */
/******************************************************************************************************************************/
 static size_t CB_Receive_gif_data( char *ptr, size_t size, size_t nmemb, void *userdata )
  { gchar *new_buffer;
    printf("%s: %d*%d octets received", __func__, size, nmemb );
    new_buffer = g_try_realloc ( Gif_received_buffer, Gif_received_size +  size*nmemb );
    if (!new_buffer)                                                 /* Si erreur, on arrete le transfert */
     { printf( "%s: Memory Error realloc", __func__ );
       g_free(Gif_received_buffer);
       Gif_received_buffer = NULL;
       return(-1);
     } else Gif_received_buffer = new_buffer;
    memcpy( Gif_received_buffer + Gif_received_size, ptr, size*nmemb );
    Gif_received_size += size*nmemb;
    return(size*nmemb);
  }
/**********************************************************************************************************/
/* Download_gif: Tente de récupérer un .gif depuis le serveur                                             */
/* Entrée: l'id et le mode attendu                                                                        */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 static gboolean Download_gif ( gint id, gint mode )
  { gchar erreur[CURL_ERROR_SIZE+1];
    struct curl_slist *slist = NULL;
    long http_response;
    gchar url[128];
    CURLcode res;
    CURL *curl;

    Gif_received_buffer = NULL;                                     /* Init du tampon de reception à NULL */
    Gif_received_size = 0;                                          /* Init du tampon de reception à NULL */
    http_response = 0;

    curl = curl_easy_init();                                            /* Preparation de la requete CURL */
    if (!curl)
     { printf( "Download_gif: cURL init failed" );
       return(FALSE);
     }

    if (mode) g_snprintf( url, sizeof(url), "https://icons.abls-habitat.fr/assets/gif/%d.gif.%02d", id, mode );
         else g_snprintf( url, sizeof(url), "https://icons.abls-habitat.fr/assets/gif/%d.gif", id );
    printf( "%s: Trying to get %s", __func__, url );
    curl_easy_setopt(curl, CURLOPT_URL, url );
       /*curl_easy_setopt(curl, CURLOPT_POST, 1 );
       curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)buf->content);
       curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, buf->use);*/
       /*slist = curl_slist_append(slist, "Content-Type: application/xml");*/
/*       curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
       curl_easy_setopt(curl, CURLOPT_HEADER, 1);*/
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, erreur );
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CB_Receive_gif_data );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE );
    curl_easy_setopt(curl, CURLOPT_USERAGENT, WATCHDOG_USER_AGENT);
/*       curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0 );*/
/*     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );                                    Warning ! */
/*       curl_easy_setopt(curl, CURLOPT_CAINFO, Cfg_satellite.https_file_ca );
       curl_easy_setopt(curl, CURLOPT_SSLKEY, Cfg_satellite.https_file_key );
       g_snprintf( chaine, sizeof(chaine), "./%s", Cfg_satellite.https_file_cert );
       curl_easy_setopt(curl, CURLOPT_SSLCERT, chaine );*/

    res = curl_easy_perform(curl);
    if (res)
     { printf( "%s: Error : Could not connect", __func__ );
       curl_easy_cleanup(curl);
       if (Gif_received_buffer) { g_free(Gif_received_buffer); }
       return(FALSE);
     }
    if (curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &http_response ) != CURLE_OK) http_response = 401;
    curl_easy_cleanup(curl);
    curl_slist_free_all(slist);

    if (http_response != 200)                                                                /* HTTP 200 OK ? */
     { printf(
                "%s: Gif %s not received (HTTP_CODE = %d)!", __func__, url, http_response );
       if (Gif_received_buffer) { g_free(Gif_received_buffer); }
       return(FALSE);
     }
    else
     { gchar nom_fichier[80];
       gint fd;
       if (mode) g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.gif.%02d", id, mode );
            else g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.gif", id );
       printf(
                "%s: Saving GIF id %d, mode %d, size %d -> %s", __func__, id, mode, Gif_received_size, nom_fichier );
       unlink(nom_fichier);
       fd = open( nom_fichier, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR );
       if (fd>0)
        { write( fd, Gif_received_buffer, Gif_received_size );
          close (fd);
        }
       else
        { printf(
                   "Download_gif : Unable to save file %s", nom_fichier );
        }
       g_free(Gif_received_buffer);
       Gif_received_buffer = NULL;
       if (fd<=0) return(FALSE);
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Download_icon : Tente de récupérer un fichier depuis le serveur commun abls-habitat.fr                                     */
/* Entrée: le nom de fichier a télécharger                                                                                    */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Download_icon ( gchar *file )
  { gchar erreur[CURL_ERROR_SIZE+1];
    struct curl_slist *slist = NULL;
    long http_response;
    gchar url[128];
    CURLcode res;
    CURL *curl;

    Gif_received_buffer = NULL;                                                         /* Init du tampon de reception à NULL */
    Gif_received_size = 0;                                                              /* Init du tampon de reception à NULL */
    http_response = 0;

    curl = curl_easy_init();                                                                /* Preparation de la requete CURL */
    if (!curl)
     { printf( "%s: cURL init failed for %s", __func__, file );
       return(FALSE);
     }
    printf( "%s: Trying to download %s", __func__, file );

    g_snprintf( url, sizeof(url), "https://icons.abls-habitat.fr/assets/gif/%s", file );
    printf( "%s: Trying to get %s", __func__, url );
    curl_easy_setopt(curl, CURLOPT_URL, url );
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, erreur );
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CB_Receive_gif_data );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE );
    curl_easy_setopt(curl, CURLOPT_USERAGENT, WATCHDOG_USER_AGENT);

    res = curl_easy_perform(curl);
    if (res)
     { printf( "%s: Error : Could not connect", __func__ );
       curl_easy_cleanup(curl);
       if (Gif_received_buffer) { g_free(Gif_received_buffer); }
       return(FALSE);
     }
    if (curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &http_response ) != CURLE_OK) http_response = 401;
    curl_easy_cleanup(curl);
    curl_slist_free_all(slist);

    if (http_response != 200)                                                                                /* HTTP 200 OK ? */
     { printf(
                "%s: URL %s not received (HTTP_CODE = %d)!", __func__, url, http_response );
       if (Gif_received_buffer) { g_free(Gif_received_buffer); }
       return(FALSE);
     }
    else
     { gint fd;
       printf( "%s: Saving FILE %s", __func__, file );
       unlink(file);
       fd = open( file, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR );
       if (fd>0)
        { write( fd, Gif_received_buffer, Gif_received_size );
          close (fd);
        }
       else
        { printf( "%s: Unable to save file %s", __func__, file ); }
       g_free(Gif_received_buffer);
       Gif_received_buffer = NULL;
       if (fd<=0) return(FALSE);
     }
    printf( "%s: %s downloaded", __func__, file );
    return(TRUE);
  }
#endif
/******************************************************************************************************************************/
/* Add_single_icone_to_item : Chargement d'un icone (ID+Mode) dans l'item en parametre                                        */
/* Entrée: L'item, l'icone_id et le mode attendu                                                                              */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 static gboolean Add_single_icone_to_item ( struct TRAME_ITEM_MOTIF *trame_item, guint icone_id, guint mode )
  { gchar nom_fichier[80];
    GdkPixbuf *pixbuf;

    if (mode==0) g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.gif", icone_id );
    else         g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.gif.%02d", icone_id, mode );
    pixbuf = gdk_pixbuf_new_from_file ( nom_fichier, NULL );                            /* 2nde tentative */
    if (!pixbuf)
    { printf("Add_single_icone_to_item: Erreur chargement %s\n", nom_fichier); return(FALSE); }/* Chargement en erreur ou terminé */
    trame_item->gif_largeur = gdk_pixbuf_get_width ( pixbuf );
    trame_item->gif_hauteur = gdk_pixbuf_get_height( pixbuf );
    trame_item->images = g_list_append( trame_item->images, pixbuf );     /* Et ajout dans la liste */
    trame_item->image  = trame_item->images;                          /* Synchro sur image numero 1 */
    trame_item->nbr_images++;
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Charger_pixbuf: Tente de charger un ensemble de pixbuf representant un icone                                               */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference                             */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 static void Charger_pixbuf_id ( struct TRAME_ITEM_MOTIF *trame_item, guint icone_id )
  { gboolean local_found;

    trame_item->image  = NULL;
    trame_item->images = NULL;
    trame_item->nbr_images  = 0;
    trame_item->gif_largeur = 0;
    trame_item->gif_hauteur = 0;

    local_found = Add_single_icone_to_item(trame_item, icone_id, 0);                       /* Tentatives de chargement locale */
    printf("%s: local_found=%d\n", __func__, local_found );
#ifdef bouh
    if ( local_found == FALSE )                                        /* Si non, tentative de récupération auprès du serveur */
     { while ( Download_gif ( icone_id, trame_item->nbr_images ) == TRUE )                              /* Trying to download */
        { Add_single_icone_to_item(trame_item, icone_id, trame_item->nbr_images); }
     }
#endif
                                                                  /* Chargement des frames restantes (downloadées ou locales) */
    while ( Add_single_icone_to_item(trame_item, icone_id, trame_item->nbr_images) == TRUE );
  }
/******************************************************************************************************************************/
/* Charger_pixbuf: Tente de charger un ensemble de pixbuf representant un icone                                               */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference                             */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 static GdkPixbuf *Charger_svg_pixbuf ( gchar *nom, gchar *couleur, gint mode, gint taille_x, gint taille_y )
  { GdkPixbuf *pixbuf;
    gchar file[80];
    if (!couleur) { g_snprintf(file, sizeof(file), "%s.svg", nom ); }
    else if (mode==0) { g_snprintf(file, sizeof(file), "%s_%s.svg", nom, couleur ); }
                 else { g_snprintf(file, sizeof(file), "%s_%d_%s.svg", nom, mode, couleur ); }
    if (taille_x>0 && taille_y>0) pixbuf = gdk_pixbuf_new_from_file_at_size( file, taille_x, taille_y, NULL );
                             else pixbuf = gdk_pixbuf_new_from_file( file, NULL );

    if (pixbuf) return(pixbuf);                                                       /* Si trouvé en local, on charge direct */
                                                                          /* Sinon on telecharge depuis le repository d'icone */
#ifdef bouh
    if (Download_icon ( file ))
     { if (taille_x>0 && taille_y>0) pixbuf = gdk_pixbuf_new_from_file_at_size( file, taille_x, taille_y, NULL );
                                else pixbuf = gdk_pixbuf_new_from_file( file, NULL );
     }
#endif
    return(pixbuf);
  }
/******************************************************************************************************************************/
/* Trame_ajout_motif: Ajoute un motif sur le visuel                                                                           */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference                             */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 struct TRAME_ITEM_MOTIF *Trame_ajout_motif ( gint flag, struct TRAME *trame, struct CMD_TYPE_MOTIF *motif )
  { struct TRAME_ITEM_MOTIF *trame_motif;

    if (!(trame && motif)) return(NULL);

    trame_motif = Trame_new_item();
    if (!trame_motif) { printf("Trame_ajout_motif: Erreur mémoire\n"); return(NULL); }

    trame_motif->motif = motif;
    trame_motif->page = trame->page;
    trame_motif->type = TYPE_MOTIF;

    Charger_pixbuf_id( trame_motif, motif->icone_id );
    if (!trame_motif->images)                                                                  /* En cas de probleme, on sort */
     { Trame_del_item(trame_motif);
       g_free(trame_motif);
       return(NULL);
     }

    Trame_peindre_motif( trame_motif, motif->def_color );
    trame_motif->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );                             /* Groupe MOTIF */
    trame_motif->item = goo_canvas_image_new ( trame_motif->item_groupe,
                                               trame_motif->pixbuf,
                                               (-(gdouble)(trame_motif->gif_largeur/2)),
                                               (-(gdouble)(trame_motif->gif_hauteur/2)),
                                               NULL );

    if (!motif->largeur) motif->largeur = trame_motif->gif_largeur;
    if (!motif->hauteur) motif->hauteur = trame_motif->gif_hauteur;

    if ( flag )
     { GdkPixbuf *pixbuf;

       pixbuf = gdk_pixbuf_new_from_file( "fleche_hg.gif", NULL );
       trame_motif->select_hg = goo_canvas_image_new ( trame->canvas_root, pixbuf, 0.0, 0.0, NULL );
       g_object_unref(pixbuf);

       pixbuf = gdk_pixbuf_new_from_file( "fleche_hd.gif", NULL );
       trame_motif->select_hd = goo_canvas_image_new ( trame->canvas_root, pixbuf, 0.0, 0.0, NULL );
       g_object_unref(pixbuf);

       pixbuf = gdk_pixbuf_new_from_file( "fleche_bg.gif", NULL );
       trame_motif->select_bg = goo_canvas_image_new ( trame->canvas_root, pixbuf, 0.0, 0.0, NULL );
       g_object_unref(pixbuf);

       pixbuf = gdk_pixbuf_new_from_file( "fleche_bd.gif", NULL );
       trame_motif->select_bd = goo_canvas_image_new ( trame->canvas_root, pixbuf, 0.0, 0.0, NULL );
       g_object_unref(pixbuf);

       g_object_set( trame_motif->select_hg, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
       g_object_set( trame_motif->select_hd, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
       g_object_set( trame_motif->select_bg, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
       g_object_set( trame_motif->select_bd, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
     }

    Trame_rafraichir_motif ( trame_motif );

    trame->trame_items = g_list_append( trame->trame_items, trame_motif );
    if (trame_motif->motif->type_gestion == TYPE_FOND)
     { goo_canvas_item_lower( trame_motif->item_groupe, NULL );
       goo_canvas_item_lower( trame->fond, NULL );
     }
    else if (!flag) g_object_set ( G_OBJECT(trame_motif->item_groupe), "tooltip", motif->libelle, NULL );
    return(trame_motif);
  }
/******************************************************************************************************************************/
/* Trame_ajout_camera_sup: Ajoute un camera_sup sur le visuel                                                                 */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference                             */
/* Sortie: la structure referencant la camera de supervision, ou NULL si erreur                                               */
/******************************************************************************************************************************/
 struct TRAME_ITEM_CAMERA_SUP *Trame_ajout_camera_sup ( gint flag, struct TRAME *trame,
                                                        struct CMD_TYPE_CAMERASUP *camera_sup )
  { struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    GdkPixbuf *pixbuf;

    if (!(trame && camera_sup)) return(NULL);

    trame_camera_sup = g_try_malloc0( sizeof(struct TRAME_ITEM_CAMERA_SUP) );
    if (!trame_camera_sup) return(NULL);
    trame_camera_sup->camera_sup = camera_sup;
    trame_camera_sup->page = trame->page;
    pixbuf = gdk_pixbuf_new_from_file ( "1.gif", NULL );                                      /* Chargement du fichier Camera */
    if (!pixbuf)
     { //Download_gif ( 1, 0 );
       pixbuf = gdk_pixbuf_new_from_file ( "1.gif", NULL );                                   /* Chargement du fichier Camera */
       if (!pixbuf) { g_free(trame_camera_sup); return(NULL); }
     }

    trame_camera_sup->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );    /* Groupe MOTIF */
    if (!flag) g_object_set ( G_OBJECT(trame_camera_sup->item_groupe), "tooltip", camera_sup->libelle, NULL );

    trame_camera_sup->item = goo_canvas_image_new ( trame_camera_sup->item_groupe,
                                                    pixbuf,
                                                    -gdk_pixbuf_get_width(pixbuf)/2.0, -gdk_pixbuf_get_height(pixbuf)/2.0,
                                                    NULL );

/*       g_snprintf( chaine, sizeof(chaine), "CAM%03d", trame_camera_sup->camera_sup->num );
       goo_canvas_text_new ( trame_camera_sup->item_groupe, chaine, 0.0, 0.0,
                                                         -1, GOO_CANVAS_ANCHOR_CENTER,
                                                         "fill-color", "yellow",
                                                         "font", "arial bold 14",
                                                         NULL);*/
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

    Trame_choisir_frame( trame_motif, 0, trame_motif->motif->def_color );
#ifdef DEBUG_TRAME
printf("New motif par item: %f %f\n", trame_motif->motif->largeur, trame_motif->motif->hauteur );
#endif
    trame_motif->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );         /* Groupe MOTIF */

    trame_motif->item = goo_canvas_image_new ( trame_motif->item_groupe,
                                               trame_motif->pixbuf,
                                               (-(gdouble)(trame_motif->gif_largeur/2)),
                                               (-(gdouble)(trame_motif->gif_hauteur/2)),
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
 struct TRAME_ITEM_COMMENT *Trame_ajout_commentaire ( gint flag, struct TRAME *trame,
                                                      struct CMD_TYPE_COMMENT *comm )
  { struct TRAME_ITEM_COMMENT *trame_comm;
    guint couleur;

    if (!(trame && comm)) return(NULL);
    trame_comm = g_try_malloc0( sizeof(struct TRAME_ITEM_COMMENT) );
    if (!trame_comm) return(NULL);
    couleur = ((guint)comm->rouge<<24) + ((guint)comm->vert<<16) + ((guint)comm->bleu<<8) + 0xFF;

#ifdef DEBUG_TRAME
printf("New comment %s %s \n", comm->libelle, comm->font );
#endif
    trame_comm->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );        /* Groupe COMMENT */

    trame_comm->item = goo_canvas_text_new ( trame_comm->item_groupe,
                                             comm->libelle, 0.0, 0.0, -1, GOO_CANVAS_ANCHOR_CENTER,
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
/******************************************************************************************************************************/
/* Trame_peindre_motif: Peint un motif de la couleur selectionnée                                                             */
/* Entrée: une structure TRAME_ITEM_MOTIF, la couleur de reference                                                            */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Trame_set_svg ( struct TRAME_ITEM_SVG *trame_svg, gchar *couleur, gint mode, gboolean cligno )
  { GdkPixbuf *pixbuf;
    if (trame_svg == NULL) return;
    pixbuf = Charger_svg_pixbuf ( trame_svg->svg_name, couleur, mode, trame_svg->taillex, trame_svg->tailley );
    if (! (trame_svg->taillex>0 && trame_svg->tailley>0) )  /* Si chargement sans imposer la taille, reprend la taille du svg */
     { trame_svg->taillex = gdk_pixbuf_get_width(pixbuf);
       trame_svg->tailley = gdk_pixbuf_get_height(pixbuf);
     }
    g_object_set( G_OBJECT(trame_svg->item), "pixbuf", pixbuf, NULL );
    if (cligno && !trame_svg->cligno)                                                    /* Active la gestion du clignotement */
     { trame_svg->trame->Liste_timer = g_slist_prepend ( trame_svg->trame->Liste_timer, trame_svg );
       trame_svg->cligno = TRUE;
     }
    else if (!cligno)                                                                            /* Desactive le clignotement */
     { trame_svg->cligno = FALSE; }                       /* Arret cligno. Il sera sorti de la liste par l'interruption timer */
  }
/******************************************************************************************************************************/
/* Trame_peindre_motif: Peint un motif de la couleur selectionnée                                                             */
/* Entrée: une structure TRAME_ITEM_MOTIF, la couleur de reference                                                            */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static struct TRAME_ITEM_SVG *Trame_new_SVG ( struct TRAME *trame, GooCanvasItem *item_groupe, gchar *nom, gchar *couleur,
                                               gint mode, gint taillex, gint tailley, gint posx, gint posy )
  { struct TRAME_ITEM_SVG *trame_svg;
    trame_svg = (struct TRAME_ITEM_SVG *)g_try_malloc0( sizeof(struct TRAME_ITEM_SVG) );
    if (!trame_svg) return(NULL);
    g_snprintf( trame_svg->svg_name, sizeof(trame_svg->svg_name), "%s", nom );
    trame_svg->trame   = trame;
    trame_svg->taillex = taillex;
    trame_svg->tailley = tailley;
    trame_svg->item    = goo_canvas_image_new ( item_groupe, NULL, 1.0*posx, 1.0*posy, NULL );
    Trame_set_svg ( trame_svg, couleur, mode, FALSE );
    return(trame_svg);
  }
/******************************************************************************************************************************/
/* Trame_ajout_passerelle: Ajoute une passerelle sur le visuel                                                                */
/* Entrée: une structure passerelle, la trame de reference                                                                    */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 struct TRAME_ITEM_PASS *Trame_ajout_passerelle ( gint flag, struct TRAME *trame, struct CMD_TYPE_PASSERELLE *pass )
  { struct TRAME_ITEM_PASS *trame_pass;
    gint taillex, tailley;

    if (!(trame && pass)) return(NULL);
    trame_pass = g_try_malloc0( sizeof(struct TRAME_ITEM_PASS) );
    if (!trame_pass) return(NULL);

    trame_pass->page   = trame->page;
    trame_pass->pass   = pass;
    trame_pass->type   = TYPE_PASSERELLE;
    trame_pass->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );     /* Groupe PASSERELLE */
    tailley = 15;
    taillex = strlen(pass->libelle) * 11;
    trame_pass->item_fond = goo_canvas_rect_new( trame_pass->item_groupe,
                                                 (double)-60.0,
                                                 (double)-(tailley/2+10.0),
                                                 (double)+taillex+78.0,
                                                 (double)(tailley+16),
                                                 "stroke-color", "yellow",
                                                 "fill-color", "blue",
                                                 NULL);

    trame_pass->item_texte = goo_canvas_text_new( trame_pass->item_groupe,
                                                  pass->libelle, 10.0, 0.0,
                                                  -1, GOO_CANVAS_ANCHOR_WEST,
                                                  "font", "courier bold 14",
                                                  "fill-color", "white",
                                                  NULL);

    trame_pass->item_1 = Trame_new_SVG ( trame, trame_pass->item_groupe, "Pignon", "vert", 0, 25, 25, -58, -15 );
    trame_pass->item_2 = Trame_new_SVG ( trame, trame_pass->item_groupe, "Bouclier2", "vert", 0, 20, 20, -32, -13 );
    trame_pass->item_3 = Trame_new_SVG ( trame, trame_pass->item_groupe, "Croix_rouge", "vert", 0, 15, 15, -10, -10 );
    printf("Nouvelle passerelle: %d, %p, %p, %p\n", pass->syn_id, trame_pass->item_1, trame_pass->item_2, trame_pass->item_3 );

    if ( flag )
     { trame_pass->select_mi = goo_canvas_rect_new (trame_pass->item_groupe,
                                                    +10.0, -2.5, 5.0, 5.0,
                                                    "fill_color", "green",
                                                    "stroke_color", "black", NULL);
       g_object_set( trame_pass->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
     }
    else g_object_set ( G_OBJECT(trame_pass->item_groupe), "tooltip", pass->libelle, NULL );


    Trame_rafraichir_passerelle ( trame_pass );

    trame->trame_items = g_list_append( trame->trame_items, trame_pass );

    return(trame_pass);
  }
/******************************************************************************************************************************/
/* Trame_ajout_cadran: Ajoute un cadran sur le visuel                                                                         */
/* Entrée: une structure cadran, la trame de reference                                                                        */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 struct TRAME_ITEM_CADRAN *Trame_ajout_cadran ( gint flag, struct TRAME *trame,
                                                struct CMD_TYPE_CADRAN *cadran )
  { struct TRAME_ITEM_CADRAN *trame_cadran;
    gchar *couleur_bordure;

    if (!(trame && cadran)) return(NULL);
    trame_cadran = g_try_malloc0( sizeof(struct TRAME_ITEM_CADRAN) );
    if (!trame_cadran) return(NULL);

    trame_cadran->cadran = cadran;
    trame_cadran->page   = trame->page;
    trame_cadran->item_groupe = goo_canvas_group_new ( trame->canvas_root,                                   /* Groupe cadran */
                                                        NULL);

    if (cadran->type == MNEMO_REGISTRE) { couleur_bordure = "red"; }
                                   else { couleur_bordure = "green"; }
    trame_cadran->item_carre = goo_canvas_rect_new (trame_cadran->item_groupe,
                                                    -55.0, -15.0, 110.0, 30.0,
                                                    "fill_color", "gray",
                                                    "stroke_color", couleur_bordure, NULL);

    trame_cadran->item_entry = goo_canvas_text_new ( trame_cadran->item_groupe,
                                                     "- cadran -", 0.0, 0.0,
                                                     -1, GOO_CANVAS_ANCHOR_CENTER,
                                                     "font", "arial italic 12",
                                                     NULL);

    if ( flag )                                                                                    /* flag == TRUE si ATELIER */
     { trame_cadran->select_mi = goo_canvas_rect_new (trame_cadran->item_groupe,
                                                       -2.5, -2.5, 7.5, 7.5,
                                                       "fill_color", "green",
                                                       "stroke_color", "black", NULL);
       g_object_set( trame_cadran->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
     }

    Trame_rafraichir_cadran ( trame_cadran );

    trame_cadran->type = TYPE_CADRAN;
    trame->trame_items = g_list_append( trame->trame_items, trame_cadran );

    return(trame_cadran);
  }
/******************************************************************************************************************************/
/* Trame_creer_trame: Creation d'une nouvelle trame                                                                           */
/* Entrée: les tailles x, y et la couleur de fond                                                                             */
/* Sortie: un widget GTK                                                                                                      */
/******************************************************************************************************************************/
 struct TRAME *Trame_creer_trame ( struct PAGE_NOTEBOOK *page, guint taille_x, guint taille_y, char *coul, guint grille )
  { struct TRAME *trame;
    gdouble x, y;

    trame = g_try_malloc0( sizeof(struct TRAME) );
    if (!trame) return(NULL);

    trame->page = page;
    trame->trame_widget = goo_canvas_new();
    g_object_set( trame->trame_widget, "background-color", coul, "anchor", GOO_CANVAS_ANCHOR_CENTER, NULL );
    g_object_set( trame->trame_widget, "has-tooltip", TRUE, NULL );
    goo_canvas_set_bounds (GOO_CANVAS (trame->trame_widget), 0, 0, taille_x+80, taille_y);
    trame->canvas_root = goo_canvas_get_root_item (GOO_CANVAS (trame->trame_widget));
    trame->fond = goo_canvas_rect_new ( trame->canvas_root, 0.0, 0.0, (double) taille_x, (double) taille_y,
                                       "stroke_color", "yellow", NULL);
    goo_canvas_item_lower( GOO_CANVAS_ITEM(trame->fond), NULL );
                                                                                         /* Creation de la vignette activités */
    trame->Logo = Trame_new_SVG ( trame, trame->canvas_root, "logo", "neutre", 0,
                                  60, 60, TAILLE_SYNOPTIQUE_X, 10 );
                                                                                         /* Creation de la vignette activités */
    trame->Vignette_activite = Trame_new_SVG ( trame, trame->canvas_root, "Pignon", "vert", 0,
                                               40, 40, TAILLE_SYNOPTIQUE_X+10,  60 );
                                                                                         /* Creation de la vignette secu bien */
    trame->Vignette_secu_bien = Trame_new_SVG ( trame, trame->canvas_root, "Bouclier2", "vert", 0,
                                               40, 40, TAILLE_SYNOPTIQUE_X+10, 110 );
                                                                                     /* Creation de la vignette secu personne */
    trame->Vignette_secu_personne = Trame_new_SVG ( trame, trame->canvas_root, "Croix_rouge", "vert", 0,
                                                    40, 40, TAILLE_SYNOPTIQUE_X+10, 160 );

    if (grille)
     { for ( x=grille; x<taille_x; x+=grille )
        { for ( y=grille; y<taille_y; y+=grille )
           { goo_canvas_polyline_new_line (trame->canvas_root, x, y-1.0, x, y+1.0, "stroke_color", "blue", NULL );
             goo_canvas_polyline_new_line (trame->canvas_root, x-1.0, y, x+1.0, y, "stroke_color", "blue", NULL );
           }
        }
     }

    return(trame);
  }
/******************************************************************************************************************************/
/* Trame_effacer_trame: Efface la trame en parametre                                                                          */
/* Entrée: la trame voulue                                                                                                    */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Trame_effacer_trame ( struct TRAME *trame )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct TRAME_ITEM_COMMENT *trame_comm;
    struct TRAME_ITEM_PASS *trame_pass;
    struct TRAME_ITEM_CADRAN *trame_cadran;
    struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    GList *objet;

    objet = trame->trame_items;                                              /* Destruction des items du synoptique precedent */
    while(objet)
     { switch ( *((gint *)objet->data) )
        { case TYPE_CADRAN:  trame_cadran = (struct TRAME_ITEM_CADRAN *)objet->data;
                            Trame_del_cadran( trame_cadran );
                            g_free(trame_cadran);
                            break;
          case TYPE_PASSERELLE:
                            trame_pass = (struct TRAME_ITEM_PASS *)objet->data;
                            Trame_del_passerelle( trame_pass );
                            g_free(trame_pass);
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
    g_list_free( trame->trame_items );                                                    /* Raz de la g_list correspondantes */
    trame->trame_items = NULL;

    Trame_del_SVG ( trame->Logo );
    Trame_del_SVG ( trame->Vignette_activite );
    Trame_del_SVG ( trame->Vignette_secu_bien );
    Trame_del_SVG ( trame->Vignette_secu_personne );
  }

/******************************************************************************************************************************/
/* Trame_detruire_trame: Destruction d'une trame                                                                              */
/* Entrée: la trame voulue                                                                                                    */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Trame_detruire_trame ( struct TRAME *trame )
  { if (!trame) return;
    Trame_effacer_trame ( trame );
    gtk_widget_destroy( trame->trame_widget );
    g_free(trame);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
