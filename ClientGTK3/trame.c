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

/******************************************************************************************************************************/
/* Trame_new_item: Renvoi un nouveau item, completement vierge                                                                */
/* Entrée: kedal                                                                                                              */
/* Sortie: une structure TRAME_ITEM_MOTIF                                                                                     */
/******************************************************************************************************************************/
 struct TRAME_ITEM_MOTIF *Trame_new_item ( void )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    trame_motif = g_try_malloc0( sizeof(struct TRAME_ITEM_MOTIF) );
    if (!trame_motif)
     { printf("%s: Reservation mémoire failed\n", __func__ );
       return(NULL);
     }
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
#warning A revoir quand tout sera migre en full dynamic
    /*g_list_foreach( trame_motif->images, (GFunc) g_object_unref, NULL );
    g_list_free( trame_motif->images );*/
    if (trame_motif->pixbuf) g_object_unref(trame_motif->pixbuf);
  }
/******************************************************************************************************************************/
/* Trame_del_svg: Libere la mémoire liée à un objet de type SVG                                                               */
/* Entrée: le SVG                                                                                                             */
/* Sortie: rieng                                                                                                              */
/******************************************************************************************************************************/
 void Trame_del_SVG ( struct TRAME_ITEM_SVG *trame_svg )
  { if (!trame_svg) return;
    if (trame_svg->trame->Liste_timer)                                                   /* Désactive la gestion clignotement */
     { trame_svg->trame->Liste_timer = g_slist_remove ( trame_svg->trame->Liste_timer, trame_svg ); }
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
/******************************************************************************************************************************/
/* Trame_del_item: Efface un item de la trame                                                                                 */
/* Entrée: un item                                                                                                            */
/* Sortie: rieng                                                                                                              */
/******************************************************************************************************************************/
 void Trame_del_commentaire ( struct TRAME_ITEM_COMMENT *trame_comm )
  { if (trame_comm->item_groupe) goo_canvas_item_remove( trame_comm->item_groupe );
  }
/******************************************************************************************************************************/
/* Trame_rafraichir_motif: remet à jour la position, rotation, echelle du motif en parametre                                  */
/* Entrée: la structure graphique TRAME_MOTIF                                                                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Trame_rafraichir_motif ( struct TRAME_ITEM_MOTIF *trame_motif )
  { if (!(trame_motif && trame_motif->visuel)) return;

    gchar *forme    = Json_get_string ( trame_motif->visuel, "forme" );
    gint position_x = Json_get_int ( trame_motif->visuel, "posx" );
    gint position_y = Json_get_int ( trame_motif->visuel, "posy" );
    gint largeur    = Json_get_int ( trame_motif->visuel, "larg" );
    gint hauteur    = Json_get_int ( trame_motif->visuel, "haut" );
    gint angle      = Json_get_int ( trame_motif->visuel, "angle" );
    gdouble scale   = Json_get_double ( trame_motif->visuel, "scale" );
    if (scale<=0.0) scale=1.0;

    cairo_matrix_init_identity ( &trame_motif->transform );
    cairo_matrix_translate ( &trame_motif->transform, (gdouble)position_x, (gdouble)position_y );

    cairo_matrix_rotate ( &trame_motif->transform, (gdouble)angle*FACTEUR_PI );

    if (forme)
     { cairo_matrix_scale  ( &trame_motif->transform, scale, scale ); }
    else
     { cairo_matrix_scale  ( &trame_motif->transform,
                              (gdouble)largeur/trame_motif->gif_largeur,
                              (gdouble)hauteur/trame_motif->gif_hauteur
                           );
     }
    cairo_matrix_translate ( &trame_motif->transform,
                             -(gdouble)trame_motif->gif_largeur/2.0,
                             -(gdouble)trame_motif->gif_hauteur/2.0);
    goo_canvas_item_set_transform ( trame_motif->item_groupe, &trame_motif->transform );

    if (trame_motif->select_hd)
     { cairo_matrix_init_identity ( &trame_motif->transform_hd );
       cairo_matrix_translate ( &trame_motif->transform_hd, (gdouble)position_x, (gdouble)position_y );
       cairo_matrix_rotate ( &trame_motif->transform_hd, (gdouble)angle*FACTEUR_PI );
       cairo_matrix_translate ( &trame_motif->transform_hd, (gdouble)trame_motif->gif_largeur/2.0*scale, -(gdouble)trame_motif->gif_hauteur/2.0*scale );
       goo_canvas_item_set_transform ( trame_motif->select_hd, &trame_motif->transform_hd );

       cairo_matrix_init_identity ( &trame_motif->transform_bd );
       cairo_matrix_translate ( &trame_motif->transform_bd, (gdouble)position_x, (gdouble)position_y );
       cairo_matrix_rotate ( &trame_motif->transform_bd, (gdouble)angle*FACTEUR_PI );
       cairo_matrix_translate ( &trame_motif->transform_bd, (gdouble)trame_motif->gif_largeur/2.0*scale, (gdouble)trame_motif->gif_hauteur/2.0*scale );
       goo_canvas_item_set_transform ( trame_motif->select_bd, &trame_motif->transform_bd );

       cairo_matrix_init_identity ( &trame_motif->transform_hg );
       cairo_matrix_translate ( &trame_motif->transform_hg, (gdouble)position_x, (gdouble)position_y );
       cairo_matrix_rotate ( &trame_motif->transform_hg, (gdouble)angle*FACTEUR_PI );
       cairo_matrix_translate ( &trame_motif->transform_hg, -(gdouble)trame_motif->gif_largeur/2.0*scale, -(gdouble)trame_motif->gif_hauteur/2.0*scale );
       goo_canvas_item_set_transform ( trame_motif->select_hg, &trame_motif->transform_hg );

       cairo_matrix_init_identity ( &trame_motif->transform_bg );
       cairo_matrix_translate ( &trame_motif->transform_bg, (gdouble)position_x, (gdouble)position_y );
       cairo_matrix_rotate ( &trame_motif->transform_bg, (gdouble)angle*FACTEUR_PI );
       cairo_matrix_translate ( &trame_motif->transform_bg, -(gdouble)trame_motif->gif_largeur/2.0*scale, (gdouble)trame_motif->gif_hauteur/2.0*scale );
       goo_canvas_item_set_transform ( trame_motif->select_bg, &trame_motif->transform_bg );
     }
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
                             (gdouble)Json_get_int ( trame_comment->comment, "posx" ),
                             (gdouble)Json_get_int ( trame_comment->comment, "posy" )
                           );

    cairo_matrix_rotate ( &trame_comment->transform, (gdouble)Json_get_int ( trame_comment->comment, "angle" )*FACTEUR_PI );
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
                             (gdouble)Json_get_int ( trame_pass->pass, "posx" ),
                             (gdouble)Json_get_int ( trame_pass->pass, "posy" )
                           );

    cairo_matrix_rotate ( &trame_pass->transform, (gdouble)Json_get_int ( trame_pass->pass, "angle" )*FACTEUR_PI );
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

    gdouble scale   = Json_get_double ( trame_cadran->cadran, "scale" );
    if (scale<=0.0) scale=1.0;

    cairo_matrix_init_identity ( &trame_cadran->transform );
    cairo_matrix_translate ( &trame_cadran->transform,
                             (gdouble)Json_get_int ( trame_cadran->cadran, "posx" ),
                             (gdouble)Json_get_int ( trame_cadran->cadran, "posy" )
                           );

    cairo_matrix_rotate ( &trame_cadran->transform, Json_get_int ( trame_cadran->cadran, "angle" )*FACTEUR_PI );
    cairo_matrix_scale  ( &trame_cadran->transform, scale, scale );
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
    if (!(trame_motif && trame_motif->visuel && trame_motif->image)) return;
    max = trame_motif->gif_largeur*trame_motif->gif_hauteur;

    if (trame_motif->pixbuf) g_object_unref(trame_motif->pixbuf);
    trame_motif->pixbuf = gdk_pixbuf_copy( (GdkPixbuf *)(trame_motif->image->data) );
    buffer = gdk_pixbuf_get_pixels( trame_motif->pixbuf );

    gint rouge = 0, vert = 0, bleu = 0;
    if (!color) color="gray";
         if (!strcasecmp(color, "red"))       { rouge = 255; vert =   0; bleu =   0; }
    else if (!strcasecmp(color, "green"))     { rouge =   0; vert = 255; bleu =   0; }
    else if (!strcasecmp(color, "blue"))      { rouge =   0; vert =   0; bleu = 255; }
    else if (!strcasecmp(color, "yellow"))    { rouge = 255; vert = 255; bleu =   0; }
    else if (!strcasecmp(color, "orange"))    { rouge = 255; vert = 190; bleu =   0; }
    else if (!strcasecmp(color, "white"))     { rouge = 255; vert = 255; bleu = 255; }
    else if (!strcasecmp(color, "kaki"))      { rouge =   0; vert = 100; bleu =   0; }
    else if (!strcasecmp(color, "darkgreen")) { rouge =   0; vert = 100; bleu =   0; }
    else if (!strcasecmp(color, "gray"))      { rouge = 127; vert = 127; bleu = 127; }
    else if (!strcasecmp(color, "black"))     { rouge =   0; vert =   0; bleu =   0; }
    else if (!strcasecmp(color, "(null)"))    { rouge =   0; vert =   0; bleu =   0; }
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

    if (!(trame_motif && trame_motif->visuel)) { printf ("Niet\n"); return; }

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
/* Trame_create_poignees: Création des poignées autour de l'item groupe + raccordement event handler pour l'atelier           */
/* Entrée : le visuel trame_motif                                                                                             */
/* sortie : Les handlers sont définis ainsi que les poignees                                                                  */
/******************************************************************************************************************************/
 static void Trame_create_poignees ( struct TRAME_ITEM_MOTIF *trame_motif )
  { struct TRAME *trame;
    if (trame_motif->page->type != TYPE_PAGE_ATELIER) return;

    trame = ((struct TYPE_INFO_ATELIER *)trame_motif->page->infos)->Trame_atelier;

    trame_motif->select_hg = goo_canvas_rect_new (trame->canvas_root,
                                                 -2.5, -2.5, 5.0, 5.0,
                                                 "fill_color", "lightblue",
                                                 "stroke_color", "black", NULL);

    trame_motif->select_hd = goo_canvas_rect_new (trame->canvas_root,
                                                 -2.5, -2.5, 5.0, 5.0,
                                                 "fill_color", "red",
                                                 "stroke_color", "black", NULL);

    trame_motif->select_bg = goo_canvas_rect_new (trame->canvas_root,
                                                 -2.5, -2.5, 5.0, 5.0,
                                                 "fill_color", "green",
                                                 "stroke_color", "black", NULL);

    trame_motif->select_bd = goo_canvas_rect_new (trame->canvas_root,
                                                 -2.5, -2.5, 5.0, 5.0,
                                                 "fill_color", "darkblue",
                                                 "stroke_color", "black", NULL);

    g_object_set( trame_motif->select_hg, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
    g_object_set( trame_motif->select_hd, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
    g_object_set( trame_motif->select_bg, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
    g_object_set( trame_motif->select_bd, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );

    g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-press-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-release-event", G_CALLBACK(Clic_sur_motif), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->item_groupe), "enter-notify-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->item_groupe), "leave-notify-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
    g_signal_connect( G_OBJECT(trame_motif->item_groupe), "motion-notify-event",  G_CALLBACK(Clic_sur_motif), trame_motif );
  }
/******************************************************************************************************************************/
/* Trame_render_svg_to_pixbuf : rendre un svg en un pixbuf                                                                    */
/* Entrée: la chaine de caractere SVG                                                                                         */
/* Sortie: le pixbuf                                                                                                          */
/******************************************************************************************************************************/
 static GdkPixbuf *Trame_render_svg_to_pixbuf ( gchar *svg )
  { GError *error = NULL;
    RsvgHandle *handle = rsvg_handle_new_from_data ( svg, strlen(svg), &error );
    if (handle)
     { GdkPixbuf *pixbuf = rsvg_handle_get_pixbuf ( handle );
       g_object_unref ( handle );
       return(pixbuf);
     }
    printf("%s: Load SVG failed: %s\n", __func__, error->message );
    g_error_free(error);
    return(NULL);
  }
/******************************************************************************************************************************/
/* Make_svg_bouton: Renvoie la chaine SVG pour faire un bouton                                                                */
/* Entrée: les parametres du bouton                                                                                           */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 static GdkPixbuf *Trame_Make_svg_bouton ( gchar *couleur, gchar *libelle )
  { gchar bouton[512];
    gint largeur=5*strlen(libelle)+12;
    gint hauteur=18;

    g_snprintf( bouton, sizeof(bouton),
                "<svg>"
                "<rect x='0' y='0' rx='10' width='%d' height='%d' "
                "      fill='%s' stroke='none' />"
                "<text text-anchor='middle' x='%d' y='%d' "
                "      font-size='9' font-family='Bitstream' font-style='' fill='white' stroke='white'>%s</text> "
                "</svg>",
                largeur, hauteur, couleur, largeur/2, hauteur/2+4, libelle );
printf("%s: New bouton %s\n", __func__, bouton );
    return(Trame_render_svg_to_pixbuf (bouton));
  }
/******************************************************************************************************************************/
/* Trame_Make_svg_encadre: Prépare un pixbuf pour l'encadre en parametre                                                      */
/* Entrée: la taille de lencadre, la couleur, son libellé                                                                     */
/* Sortie: la chaine SVG                                                                                                      */
/******************************************************************************************************************************/
 static GdkPixbuf *Trame_Make_svg_encadre ( gint ligne, gint colonne, gchar *couleur, gchar *libelle )
  { gchar encadre[512];
    gint hauteur=64*ligne;
    gint largeur=64*colonne;
    g_snprintf( encadre, sizeof(encadre),
                "<svg viewBox='0 0 %d %d' >"
                "<text text-anchor='middle' x='%d' y='12' "
                "      font-size='14px' font-family='Sans' font-style='italic' fill='black' stroke='black'>%s</text> "
                "<rect x='5' y='20' rx='15' width='%d' height='%d' "
                "      fill='none' stroke='%s' stroke-width='2'  />"
                "</svg>",
                largeur+10, hauteur+25, (largeur+10)/2, libelle, largeur, hauteur, couleur
              );
printf("%s: New encadre %dx%d : %s\n", __func__, ligne, colonne, encadre );
    return (Trame_render_svg_to_pixbuf (encadre));
  }
/******************************************************************************************************************************/
/* Trame_Make_svg_comment: Prépare un pixbuf pour l'encadre en parametre                                                      */
/* Entrée: la taille de lencadre, la couleur, son libellé                                                                     */
/* Sortie: la chaine SVG                                                                                                      */
/******************************************************************************************************************************/
 static GdkPixbuf *Trame_Make_svg_comment ( gchar *libelle, gchar *mode, gchar *color )
  { gchar comment[512], *family, *style, *weight;
    gint size, width;
    if (!strcasecmp ( mode, "titre" ))
     { size = 32;
       family = "'Sans'";
       style  = "italic";
       weight = "normal";
       width  = (gint)strlen(libelle)*20;
     }
    else if (!strcasecmp ( mode, "soustitre" ))
     { size = 24;
       family = "'Sans'";
       style  = "italic";
       weight = "normal";
       width  = (gint)strlen(libelle)*14;
     }
    else
     { size   = 14;
       family = "'Sans'";
       style  = "italic";
       weight = "normal";
       width  = (gint)strlen(libelle)*9;
     }
    g_snprintf( comment, sizeof(comment),
                "<svg viewBox='0 0 %d %d' >"
                "<text x='0' y='%d' text-anchor='start'"
                "      font-size='%dpx' font-family=\"%s, serif\" font-style=\"%s\" font-weight='%s' fill='%s' stroke='%s'>%s</text> "
                "</svg>",
                width, size+15, size+5, size, family, style, weight, color, color, libelle
              );
printf("%s: New comment : %s\n", __func__, comment );
    return (Trame_render_svg_to_pixbuf (comment));
  }
/******************************************************************************************************************************/
/* Trame_calculer_bounds: Calcule les gif_hauteur et gif_largeur de l'item_groupe                                             */
/* Entrée: le trame_motif                                                                                                     */
/* Sortie: met a jour les champs                                                                                              */
/******************************************************************************************************************************/
 static void Trame_calculer_bounds ( struct TRAME_ITEM_MOTIF *trame_motif )
  { GooCanvasBounds bounds;
    goo_canvas_item_get_bounds ( trame_motif->item_groupe, &bounds );
    trame_motif->gif_largeur = (gint)bounds.x2-bounds.x1;
    trame_motif->gif_hauteur = (gint)bounds.y2-bounds.y1;
  }
/******************************************************************************************************************************/
/* Trame_ajout_visuel_encadre: Ajoute un visuel encadré sur le canvas                                                         */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference                             */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 static gboolean Trame_ajout_visuel_encadre ( struct PAGE_NOTEBOOK *page, JsonNode *visuel )
  { struct TRAME *trame;
         if (page->type == TYPE_PAGE_SUPERVISION) trame = ((struct TYPE_INFO_SUPERVISION *)page->infos)->Trame;
    else if (page->type == TYPE_PAGE_ATELIER)     trame = ((struct TYPE_INFO_ATELIER *)page->infos)->Trame_atelier;
    else return(FALSE);

    struct TRAME_ITEM_MOTIF *trame_motif = Trame_new_item();
    if (!trame_motif) { printf("%s: Erreur mémoire\n", __func__); return(FALSE); }

    trame_motif->visuel = visuel;
    trame_motif->page   = page;
    trame_motif->type   = TYPE_MOTIF;
    trame_motif->mode   = 0;                                                                         /* Sauvegarde etat motif */
    trame_motif->cligno = Json_get_bool ( visuel, "cligno" );                                     /* Sauvegarde etat motif */
    g_snprintf( trame_motif->color, sizeof(trame_motif->color), "%s", Json_get_string ( visuel, "color" ) );

    trame_motif->image  = NULL;
    trame_motif->images = NULL;
    trame_motif->nbr_images  = 0;
    trame_motif->gif_largeur = 0;
    trame_motif->gif_hauteur = 0;

    Trame_redessiner_visuel_complexe ( trame_motif, visuel );

    if (!trame_motif->pixbuf)
     { printf("%s: Chargement visuel encadre\n", __func__ );
       g_free(trame_motif);
       return(FALSE);
     }

    trame_motif->images = g_list_append( trame_motif->images, trame_motif->pixbuf );     /* Et ajout dans la liste */
    trame_motif->image  = trame_motif->images;                          /* Synchro sur image numero 1 */
    trame_motif->nbr_images++;

    trame_motif->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );                             /* Groupe MOTIF */
    trame_motif->item = goo_canvas_image_new ( trame_motif->item_groupe,
                                               trame_motif->pixbuf,
                                               0.0, 0.0, NULL );

    Trame_calculer_bounds ( trame_motif );
    Trame_create_poignees ( trame_motif );
    Trame_rafraichir_motif ( trame_motif );
    pthread_mutex_lock ( &trame->lock );
    trame->trame_items = g_list_append( trame->trame_items, trame_motif );
    pthread_mutex_unlock ( &trame->lock );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Trame_ajout_visuel_bouton: Ajoute un visuel 'bouton' sur la canvas                                                         */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference                             */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 static gboolean Trame_ajout_visuel_bouton ( struct PAGE_NOTEBOOK *page, JsonNode *visuel )
  { struct TRAME *trame;
         if (page->type == TYPE_PAGE_SUPERVISION) trame = ((struct TYPE_INFO_SUPERVISION *)page->infos)->Trame;
    else if (page->type == TYPE_PAGE_ATELIER)     trame = ((struct TYPE_INFO_ATELIER *)page->infos)->Trame_atelier;
    else return(FALSE);

    struct TRAME_ITEM_MOTIF *trame_motif = Trame_new_item();
    if (!trame_motif) { printf("%s: Erreur mémoire\n", __func__); return(FALSE); }

    trame_motif->visuel = visuel;
    trame_motif->page   = page;
    trame_motif->type   = TYPE_MOTIF;
    trame_motif->mode   = 0;                                                                         /* Sauvegarde etat motif */
    trame_motif->cligno = Json_get_bool ( visuel, "cligno" );                                     /* Sauvegarde etat motif */
    g_snprintf( trame_motif->color, sizeof(trame_motif->color), "%s", Json_get_string ( visuel, "color" ) );

    trame_motif->image  = NULL;
    trame_motif->images = NULL;
    trame_motif->nbr_images  = 0;
    trame_motif->gif_largeur = 0;
    trame_motif->gif_hauteur = 0;

    Trame_redessiner_visuel_complexe ( trame_motif, visuel );

    if (!trame_motif->pixbuf)
     { printf("%s: Chargement visuel bouton failed\n", __func__ );
       g_free(trame_motif);
       return(FALSE);
     }

    trame_motif->images = g_list_append( trame_motif->images, trame_motif->pixbuf );     /* Et ajout dans la liste */
    trame_motif->image  = trame_motif->images;                          /* Synchro sur image numero 1 */
    trame_motif->nbr_images++;

    trame_motif->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );                             /* Groupe MOTIF */
    trame_motif->item = goo_canvas_image_new ( trame_motif->item_groupe,
                                               trame_motif->pixbuf,
                                               0.0, 0.0, NULL );
    if (page->type == TYPE_PAGE_SUPERVISION)
     { g_signal_connect( G_OBJECT(trame_motif->item), "button-press-event",   G_CALLBACK(Clic_sur_bouton_supervision), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "button-release-event", G_CALLBACK(Clic_sur_bouton_supervision), trame_motif );
     }

    Trame_calculer_bounds ( trame_motif );
    Trame_create_poignees ( trame_motif );
    Trame_rafraichir_motif ( trame_motif );
    pthread_mutex_lock ( &trame->lock );
    trame->trame_items = g_list_append( trame->trame_items, trame_motif );
    pthread_mutex_unlock ( &trame->lock );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Trame_ajout_visuel_comment: Ajoute un commentaire sur la trame                                                             */
/* Entrée: la page et le visuel a ajouter                                                                                     */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Trame_ajout_visuel_comment ( struct PAGE_NOTEBOOK *page, JsonNode *visuel )
  { struct TRAME *trame;
         if (page->type == TYPE_PAGE_SUPERVISION) trame = ((struct TYPE_INFO_SUPERVISION *)page->infos)->Trame;
    else if (page->type == TYPE_PAGE_ATELIER)     trame = ((struct TYPE_INFO_ATELIER *)page->infos)->Trame_atelier;
    else return(FALSE);

    struct TRAME_ITEM_MOTIF *trame_motif = Trame_new_item();
    if (!trame_motif) { printf("%s: Erreur mémoire\n", __func__); return(FALSE); }

    trame_motif->visuel = visuel;
    trame_motif->page   = page;
    trame_motif->type   = TYPE_MOTIF;
    trame_motif->mode   = 0;                                                                         /* Sauvegarde etat motif */
    trame_motif->cligno = Json_get_bool ( visuel, "cligno" );                                     /* Sauvegarde etat motif */
    g_snprintf( trame_motif->color, sizeof(trame_motif->color), "%s", Json_get_string ( visuel, "color" ) );

    trame_motif->image  = NULL;
    trame_motif->images = NULL;
    trame_motif->nbr_images  = 0;
    trame_motif->gif_largeur = 0;
    trame_motif->gif_hauteur = 0;

    trame_motif->pixbuf = Trame_Make_svg_comment ( Json_get_string ( visuel, "libelle" ),
                                                   Json_get_string ( visuel, "mode" ),
                                                   Json_get_string ( visuel, "color" ) );

    if (!trame_motif->pixbuf)
     { printf("%s: Chargement visuel comment failed\n", __func__ );
       g_free(trame_motif);
       return(FALSE);
     }

    trame_motif->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );                             /* Groupe MOTIF */
    trame_motif->item = goo_canvas_image_new ( trame_motif->item_groupe,
                                               trame_motif->pixbuf,
                                               0.0, 0.0, NULL );
    Trame_calculer_bounds ( trame_motif );
    Trame_create_poignees ( trame_motif );
    Trame_rafraichir_motif ( trame_motif );
    pthread_mutex_lock ( &trame->lock );
    trame->trame_items = g_list_append( trame->trame_items, trame_motif );
    pthread_mutex_unlock ( &trame->lock );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Trame_ajout_motif: Ajoute un motif sur le visuel                                                                           */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference                             */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 gboolean Trame_ajout_visuel_complexe ( struct PAGE_NOTEBOOK *page, JsonNode *visuel )
  { gchar *forme = Json_get_string ( visuel, "forme" );
         if ( !strcasecmp ( forme, "encadre" ) )           { return ( Trame_ajout_visuel_encadre          ( page, visuel ) ); }
    else if ( !strcasecmp ( forme, "bouton" ) )            { return ( Trame_ajout_visuel_bouton           ( page, visuel ) ); }
    else if ( !strcasecmp ( forme, "comment" ) )           { return ( Trame_ajout_visuel_comment          ( page, visuel ) ); }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Trame_load_pixbuf: Charge un pixbuf distant en le mettant en cache local                                                   */
/* Entrée: le fichier a telecharger                                                                                           */
/* Sortie: Le pixbuf, ou NULL si erreur                                                                                       */
/******************************************************************************************************************************/
 static GdkPixbuf *Trame_load_pixbuf ( struct CLIENT*client, gchar *fichier )
  { gchar commande[256];
    struct stat result;
    if (stat ( fichier, &result ) == -1)
     { g_snprintf ( commande, sizeof(commande),
                    "wget --no-check-certificate https://static.abls-habitat.fr/img/%s -O %s", fichier, fichier );
       printf("%s: download %s\n", __func__, fichier );
       system(commande); /* Download de l'icone */
     }

    return (gdk_pixbuf_new_from_file ( fichier, NULL ));
  }
/******************************************************************************************************************************/
/* Trame_redessiner_visuel_complexe: Met a jour un visuel complexe                                                            */
/* Entrée: le motif du synoptique et son nouveau statut                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Trame_redessiner_visuel_complexe ( struct TRAME_ITEM_MOTIF *trame_motif, JsonNode *visuel )
  { gchar *forme = Json_get_string ( trame_motif->visuel, "forme" );
    gchar *mode  = Json_get_string ( visuel, "mode" );
    gchar *color = Json_get_string ( visuel, "color" );

    Json_node_add_string ( trame_motif->visuel, "mode", mode );                   /* sauvegarde du mode en cours */
    Json_node_add_string ( trame_motif->visuel, "color", color );                 /* sauvegarde du mode en cours */

    if (trame_motif->pixbuf)
     { g_object_unref(trame_motif->pixbuf);
       trame_motif->pixbuf = NULL;
     }

    if ( !strcasecmp ( forme, "encadre" ) )
     { gint ligne, colonne;
       if ( !Json_has_member ( visuel, "mode" ) ) Json_node_add_string ( visuel, "mode", "hors_comm" );
       gchar *mode = Json_get_string ( visuel, "mode" );
       if ( sscanf ( mode, "%dx%d", &ligne, &colonne ) != 2 ) { ligne = colonne = 1; }

       if ( !strcasecmp ( mode, "hors_comm" ) )
        { trame_motif->pixbuf = Trame_load_pixbuf ( trame_motif->page->client, "question.png" ); }
       else
        { trame_motif->pixbuf = Trame_Make_svg_encadre ( ligne, colonne,  Json_get_string ( visuel, "color" ),
                                                         Json_get_string ( visuel, "libelle" ) );
        }
     }
    else if ( !strcasecmp ( forme, "bouton" ) )
     { gchar *mode = Json_get_string ( visuel, "mode" );
       trame_motif->pixbuf = Trame_Make_svg_bouton ( Json_get_string ( visuel, "color" ), Json_get_string ( visuel, "libelle" ) );
     }
    else if ( !strcasecmp ( forme, "comment" ) )
     { gchar *mode, *color;
       if ( Json_has_member ( visuel, "mode" ) ) mode = Json_get_string ( visuel, "mode" );
       else mode = "annotation";

       if ( Json_has_member ( visuel, "color" ) ) color = Json_get_string ( visuel, "color" );
       else color = "white";

       trame_motif->pixbuf = Trame_Make_svg_comment ( Json_get_string ( visuel, "libelle" ), mode, color );
     }

    if (trame_motif->pixbuf) g_object_set( trame_motif->item, "pixbuf", trame_motif->pixbuf, NULL );
    return;
  }
/******************************************************************************************************************************/
/* Trame_redessiner_visuel_simple: Met a jour un visuel simple                                                                */
/* Entrée: le motif du synoptique et son nouveau statut                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Trame_redessiner_visuel_simple ( struct TRAME_ITEM_MOTIF *trame_motif, JsonNode *visuel )
  { if (trame_motif->pixbuf) g_object_unref(trame_motif->pixbuf);
    gchar *forme         = Json_get_string ( trame_motif->visuel, "forme" );
    gchar *extension     = Json_get_string ( trame_motif->visuel, "extension" );
    gchar *ihm_affichage = Json_get_string ( trame_motif->visuel, "ihm_affichage" );
    gchar *mode          = Json_get_string ( visuel, "mode" );
    gchar *color         = Json_get_string ( visuel, "color" );

    gchar fichier[128];
    if ( !strcmp ( ihm_affichage, "by_color" ) )
     { g_snprintf ( fichier, sizeof(fichier), "%s_%s.%s", forme, color, extension ); }
    else if ( !strcmp ( ihm_affichage, "by_mode" ) )
     { g_snprintf ( fichier, sizeof(fichier), "%s_%s.%s",forme, mode, extension ); }
    else if ( !strcmp ( ihm_affichage, "by_mode_color" ) )
     { g_snprintf ( fichier, sizeof(fichier), "%s_%s_%s.%s", forme, mode, color, extension ); }
    else if ( !strcmp ( ihm_affichage, "static" ) )
     { g_snprintf ( fichier, sizeof(fichier), "%s.%s", forme, extension ); }
    else return;

    trame_motif->pixbuf = Trame_load_pixbuf ( trame_motif->page->client, fichier );
    if (!trame_motif->pixbuf)
     { printf("%s: Chargement visuel simple '%s' pixbuf failed\n", __func__, forme );
       return;
     }

    g_object_set( trame_motif->item, "pixbuf", trame_motif->pixbuf, NULL );
  }
/******************************************************************************************************************************/
/* Trame_ajout_visuel_simple: Ajoute un visuel simple sur le canvas                                                           */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference                             */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 gboolean Trame_ajout_visuel_simple ( struct PAGE_NOTEBOOK *page, JsonNode *visuel )
  { struct TRAME *trame;

         if (page->type == TYPE_PAGE_SUPERVISION) trame = ((struct TYPE_INFO_SUPERVISION *)page->infos)->Trame;
    else if (page->type == TYPE_PAGE_ATELIER)     trame = ((struct TYPE_INFO_ATELIER *)page->infos)->Trame_atelier;
    else return(FALSE);

    struct TRAME_ITEM_MOTIF *trame_motif = Trame_new_item();
    if (!trame_motif) { printf("%s: Erreur mémoire\n", __func__); return(FALSE); }

    trame_motif->visuel = visuel;
    trame_motif->page   = page;
    trame_motif->type   = TYPE_MOTIF;
    g_snprintf( trame_motif->color, sizeof(trame_motif->color), "%s", Json_get_string ( visuel, "color" ) );
    trame_motif->cligno = Json_get_bool ( visuel, "cligno" );                                     /* Sauvegarde etat motif */

    Trame_create_poignees ( trame_motif );

    Trame_redessiner_visuel_simple ( trame_motif, visuel );

    trame_motif->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );                /* Groupe MOTIF */
    trame_motif->item = goo_canvas_image_new ( trame_motif->item_groupe,
                                               trame_motif->pixbuf,
                                               0.0, 0.0, NULL );

    if (page->type==TYPE_PAGE_SUPERVISION)
     { g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-press-event",
                         G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-release-event",
                         G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
     }

    trame_motif->images = g_list_append( trame_motif->images, trame_motif->pixbuf );   /* Et ajout dans la liste */
    trame_motif->image  = trame_motif->images;                                     /* Synchro sur image numero 1 */
    trame_motif->nbr_images++;
    printf("%s : width = %d, height=%d\n", __func__, trame_motif->gif_largeur, trame_motif->gif_hauteur );

    Trame_calculer_bounds ( trame_motif );
    Trame_create_poignees ( trame_motif );
    Trame_rafraichir_motif ( trame_motif );
    pthread_mutex_lock ( &trame->lock );
    trame->trame_items = g_list_append( trame->trame_items, trame_motif );
    pthread_mutex_unlock ( &trame->lock );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Trame_ajout_motif: Ajoute un motif sur le visuel                                                                           */
/* Entrée: flag=1 si on doit creer les boutons resize, une structure MOTIF, la trame de reference                             */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 void Trame_ajout_visuel_old ( gint flag, struct TRAME *trame, JsonNode *visuel )
  { struct TRAME_ITEM_MOTIF *trame_motif;

    if (!(trame && visuel)) return;

    trame_motif = Trame_new_item();
    if (!trame_motif) { printf("Trame_ajout_motif: Erreur mémoire\n"); return; }

    trame_motif->visuel = visuel;
    trame_motif->page   = trame->page;
    trame_motif->type   = TYPE_MOTIF;
    trame_motif->mode   = atoi(Json_get_string(visuel, "mode"));                                     /* Sauvegarde etat motif */
    trame_motif->cligno = 0;                                                                         /* Sauvegarde etat motif */
    g_snprintf( trame_motif->color, sizeof(trame_motif->color), "%s", Json_get_string ( visuel, "color" ) );

    if (flag)
     { struct TYPE_INFO_ATELIER *infos = trame->page->infos;                     /* Pointeur sur les infos de la page atelier */
       gint groupe = Json_get_int ( visuel, "groupe" );
       if (infos->groupe_max < groupe) infos->groupe_max = groupe;
     }

    gint icone = Json_get_int ( visuel, "icone" );
    if (icone == -1 || icone == 0) return;
    Charger_pixbuf_id( trame_motif, icone );
    if (!trame_motif->images)                                                                  /* En cas de probleme, on sort */
     { Trame_del_item(trame_motif);
       g_free(trame_motif);
       return;
     }

    trame_motif->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );                             /* Groupe MOTIF */
    trame_motif->item = goo_canvas_image_new ( trame_motif->item_groupe,
                                               trame_motif->pixbuf,
                                               0.0, 0.0, NULL );

    if (!Json_has_member ( visuel, "larg" ) || Json_get_int ( visuel, "larg" )<=0 )
     { Json_node_add_int ( visuel, "larg", trame_motif->gif_largeur ); }

    if (!Json_has_member ( visuel, "haut" ) || Json_get_int ( visuel, "haut" )<=0 )
     { Json_node_add_int ( visuel, "haut", trame_motif->gif_hauteur ); }

    if ( flag )
     { GdkPixbuf *pixbuf;

       pixbuf = gdk_pixbuf_new_from_file( "fleche_hg.gif", NULL );
       trame_motif->select_hg = goo_canvas_image_new ( trame->canvas_root, pixbuf, -2.5, -2.5, NULL );
       g_object_unref(pixbuf);

       pixbuf = gdk_pixbuf_new_from_file( "fleche_hd.gif", NULL );
       trame_motif->select_hd = goo_canvas_image_new ( trame->canvas_root, pixbuf, -2.5, -2.5, NULL );
       g_object_unref(pixbuf);

       pixbuf = gdk_pixbuf_new_from_file( "fleche_bg.gif", NULL );
       trame_motif->select_bg = goo_canvas_image_new ( trame->canvas_root, pixbuf, -2.5, -2.5, NULL );
       g_object_unref(pixbuf);

       pixbuf = gdk_pixbuf_new_from_file( "fleche_bd.gif", NULL );
       trame_motif->select_bd = goo_canvas_image_new ( trame->canvas_root, pixbuf, -2.5, -2.5, NULL );
       g_object_unref(pixbuf);

       g_object_set( trame_motif->select_hg, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
       g_object_set( trame_motif->select_hd, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
       g_object_set( trame_motif->select_bg, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
       g_object_set( trame_motif->select_bd, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );

       g_signal_connect( G_OBJECT(trame_motif->item), "button-press-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "button-release-event", G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "enter-notify-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "leave-notify-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "motion-notify-event",  G_CALLBACK(Clic_sur_motif), trame_motif );
     }
    else
     { g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-press-event",
                         G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-release-event",
                         G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
     }

    Trame_choisir_frame ( trame_motif, atoi(Json_get_string ( visuel, "mode" )), Json_get_string ( visuel, "color" ) );
    Trame_rafraichir_motif ( trame_motif );

    pthread_mutex_lock ( &trame->lock );
    trame->trame_items = g_list_append( trame->trame_items, trame_motif );
    pthread_mutex_unlock ( &trame->lock );
    if ( Json_get_int ( visuel, "gestion" ) == TYPE_FOND )
     { goo_canvas_item_lower( trame_motif->item_groupe, NULL );
       goo_canvas_item_lower( trame->fond, NULL );
     }
    else if (!flag) g_object_set ( G_OBJECT(trame_motif->item_groupe), "tooltip", Json_get_string ( visuel, "libelle" ), NULL );
  }
/******************************************************************************************************************************/
/* Trame_ajout_commentaire: Ajoute un commentaire sur le visuel                                                               */
/* Entrée: une structure commentaire, la trame de reference                                                                   */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 struct TRAME_ITEM_COMMENT *Trame_ajout_commentaire ( gint flag, struct TRAME *trame, JsonNode *comment )
  { struct TRAME_ITEM_COMMENT *trame_comm;

    if (!(trame && comment)) return(NULL);
    trame_comm = g_try_malloc0( sizeof(struct TRAME_ITEM_COMMENT) );
    if (!trame_comm) return(NULL);

    trame_comm->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );        /* Groupe COMMENT */

    trame_comm->item = goo_canvas_text_new ( trame_comm->item_groupe,
                                             Json_get_string ( comment, "libelle" ), 0.0, 0.0, -1, GOO_CANVAS_ANCHOR_CENTER,
                                            "font", Json_get_string( comment, "font"),
                                            "fill_color", Json_get_string ( comment, "color" ),
                                            NULL );
    trame_comm->comment = comment;
    trame_comm->type = TYPE_COMMENTAIRE;
    trame_comm->page   = trame->page;
    if (flag)
     { struct TYPE_INFO_ATELIER *infos = trame->page->infos;                     /* Pointeur sur les infos de la page atelier */
       gint groupe = Json_get_int ( comment, "groupe" );
       if (infos->groupe_max < groupe) infos->groupe_max = groupe;
     }

    if ( flag )
     { trame_comm->select_mi = goo_canvas_rect_new (trame_comm->item_groupe,
                                                    -5.0, -5.0, 10.0, 10.0,
                                                    "fill_color", "green",
                                                    "stroke_color", "black", NULL);
       g_object_set( trame_comm->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
     }

    Trame_rafraichir_comment ( trame_comm );
    pthread_mutex_lock ( &trame->lock );
    trame->trame_items = g_list_append( trame->trame_items, trame_comm );
    pthread_mutex_unlock ( &trame->lock );
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
 struct TRAME_ITEM_PASS *Trame_ajout_passerelle ( gint flag, struct TRAME *trame, JsonNode *pass )
  { struct TRAME_ITEM_PASS *trame_pass;
    gint taillex, tailley;

    if (!(trame && pass)) return(NULL);
    trame_pass = g_try_malloc0( sizeof(struct TRAME_ITEM_PASS) );
    if (!trame_pass) return(NULL);

    gchar *nom_page = Json_get_string ( pass, "page" );
    trame_pass->page   = trame->page;
    trame_pass->pass   = pass;
    trame_pass->type   = TYPE_PASSERELLE;
    trame_pass->item_groupe = goo_canvas_group_new ( trame->canvas_root, NULL );     /* Groupe PASSERELLE */
    tailley = 15;
    taillex = strlen(nom_page) * 11;
    trame_pass->item_fond = goo_canvas_rect_new( trame_pass->item_groupe,
                                                 (double)-60.0,
                                                 (double)-(tailley/2+10.0),
                                                 (double)+taillex+78.0,
                                                 (double)(tailley+16),
                                                 "stroke-color", "yellow",
                                                 "fill-color", "blue",
                                                 NULL);

    trame_pass->item_texte = goo_canvas_text_new( trame_pass->item_groupe,
                                                  nom_page, 10.0, 0.0,
                                                  -1, GOO_CANVAS_ANCHOR_WEST,
                                                  "font", "courier bold 14",
                                                  "fill-color", "white",
                                                  NULL);

    trame_pass->item_1 = Trame_new_SVG ( trame, trame_pass->item_groupe, "Pignon", "vert", 0, 25, 25, -58, -15 );
    trame_pass->item_2 = Trame_new_SVG ( trame, trame_pass->item_groupe, "Bouclier2", "vert", 0, 20, 20, -32, -13 );
    trame_pass->item_3 = Trame_new_SVG ( trame, trame_pass->item_groupe, "Croix_rouge", "vert", 0, 15, 15, -10, -10 );

    if ( flag )
     { trame_pass->select_mi = goo_canvas_rect_new (trame_pass->item_groupe,
                                                    +10.0, -2.5, 5.0, 5.0,
                                                    "fill_color", "green",
                                                    "stroke_color", "black", NULL);
       g_object_set( trame_pass->select_mi, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
     }
    else g_object_set ( G_OBJECT(trame_pass->item_groupe), "tooltip", nom_page, NULL );


    Trame_rafraichir_passerelle ( trame_pass );

    pthread_mutex_lock ( &trame->lock );
    trame->trame_items = g_list_append( trame->trame_items, trame_pass );
    pthread_mutex_unlock ( &trame->lock );

    return(trame_pass);
  }
/******************************************************************************************************************************/
/* Trame_ajout_cadran: Ajoute un cadran sur le visuel                                                                         */
/* Entrée: une structure cadran, la trame de reference                                                                        */
/* Sortie: reussite                                                                                                           */
/******************************************************************************************************************************/
 struct TRAME_ITEM_CADRAN *Trame_ajout_cadran ( gint flag, struct TRAME *trame, JsonNode *cadran )
  { struct TRAME_ITEM_CADRAN *trame_cadran;
    gchar *couleur_bordure;

    if (!(trame && cadran)) return(NULL);
    trame_cadran = g_try_malloc0( sizeof(struct TRAME_ITEM_CADRAN) );
    if (!trame_cadran) return(NULL);

    trame_cadran->cadran = cadran;
    trame_cadran->page   = trame->page;
    gint groupe = Json_get_int ( cadran, "groupe" );
    if (flag)
     { struct TYPE_INFO_ATELIER *infos = trame->page->infos;                     /* Pointeur sur les infos de la page atelier */
       if (infos->groupe_max < groupe) infos->groupe_max = groupe;
     }
    trame_cadran->item_groupe = goo_canvas_group_new ( trame->canvas_root,                                   /* Groupe cadran */
                                                       NULL);

    if (!g_ascii_strcasecmp(Json_get_string(cadran,"classe"), "REGISTRE") )
         { couleur_bordure = "blue"; }
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
    pthread_mutex_lock ( &trame->lock );
    trame->trame_items = g_list_append( trame->trame_items, trame_cadran );
    pthread_mutex_unlock ( &trame->lock );

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
    /* Désactivation ToolTIP Trame g_object_set( trame->trame_widget, "has-tooltip", TRUE, NULL );*/
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

    pthread_mutexattr_t attr;                                                          /* Initialisation des mutex de synchro */
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_SHARED );
    pthread_mutex_init( &trame->lock, &attr );

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
    GList *objet;

    pthread_mutex_lock ( &trame->lock );
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
          default: printf("Trame_effacer_trame: type inconnu\n");
        }
       objet = objet->next;
     }
    g_list_free( trame->trame_items );                                                    /* Raz de la g_list correspondantes */
    trame->trame_items = NULL;
    pthread_mutex_unlock ( &trame->lock );

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
