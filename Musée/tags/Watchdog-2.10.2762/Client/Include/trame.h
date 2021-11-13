/**********************************************************************************************************/
/* Client/Include/trame.h:   Gestion des objets sur une trame synoptique                                  */
/* Projet WatchDog version 2.0     Gestion d'habitat                        sam 27 sep 2003 15:05:09 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * trame.h
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

 #ifndef _TRAME_H_
 #define _TRAME_H_

 #include <gnome.h>
 #include <gdk-pixbuf/gdk-pixbuf.h>                                          /* Gestion des images/motifs */
 #include <goocanvas.h>                                                            /* Interface GooCanvas */
 #include "Reseaux.h"

 #define TAILLE_SYNOPTIQUE_X        1024               /* Généralités sur la taille de la Trame synoptique */
 #define TAILLE_SYNOPTIQUE_Y        768
 #define TAILLE_FONT                60                  /* Nombre de caractere pour la police d'affichage */
 #define COULEUR_FOND_SYN           "MidnightBlue"
 #define COULEUR_ACTIVE             200
 
 #define TAILLE_ICONE_X             100
 #define TAILLE_ICONE_Y             100

 enum                                                            /* Différents types de gestion de motifs */
  { TYPE_INERTE,
    TYPE_FOND,
    TYPE_STATIQUE,
    TYPE_DYNAMIQUE,
    TYPE_CYCLIQUE_0N,
    TYPE_CYCLIQUE_1N,
    TYPE_CYCLIQUE_2N,
    TYPE_PROGRESSIF,
    TYPE_BOUTON,
    NBR_TYPE_GESTION_MOTIF
  };

 enum                                                             /* Différente action associée à un item */
  { ACTION_SANS,
    ACTION_IMMEDIATE,
    ACTION_PROGRAMME,
    ACTION_NBR_ACTION                                                         /* nombre d'action possible */
  };

 enum { TYPE_RIEN,
        TYPE_PASSERELLE,
        TYPE_COMMENTAIRE,
        TYPE_MOTIF,
        TYPE_CAPTEUR,
        TYPE_CAMERA_SUP
      };

 struct TRAME_ITEM_MOTIF
  { gint type;                                                                          /* Type de l'item */
    GooCanvasItem *item;
    cairo_matrix_t transform;
    cairo_matrix_t transform_hg;
    cairo_matrix_t transform_hd;
    cairo_matrix_t transform_bg;
    cairo_matrix_t transform_bd;
    GooCanvasItem *item_groupe;
    GooCanvasItem *select_hg;
    GooCanvasItem *select_hd;
    GooCanvasItem *select_bg;
    GooCanvasItem *select_bd;
    GList *images;                                     /* Toutes les images présentes dans le fichier GIF */
    GList *image;                                                 /* Image en cours d'affichage à l'écran */
    GdkPixbuf *pixbuf;                                           /* Pixbuf colorié et visualisé à l'écran */
    guchar num_image;                                         /* Numero de l'image actuellement présentée */
    guchar nbr_images;                                               /* Nombre total d'image dans le .gif */
    
    guchar rouge;                                                            /* Couleur attendue du motif */
    guchar vert;
    guchar bleu;

    guchar en_cours_rouge;                                                   /* Couleur actuelle du motif */
    guchar en_cours_vert;
    guchar en_cours_bleu;

    guint gif_largeur;
    guint gif_hauteur;

    guchar etat;                                                                 /* Etat attendu du motif */
    gint   cligno;                                                                /* Etat cligno du motif */

    struct CMD_TYPE_MOTIF *motif;
    gint   groupe_dpl;                                                  /* Groupe de deplacement du motif */
    gint selection;                                                                  /* Encore utilisé ?? */
  };

 struct TRAME_ITEM_PASS
  { gint type;                                                                          /* Type de l'item */
    GooCanvasItem *item_groupe;
    GooCanvasItem *item_texte;
    GooCanvasItem *item_1;
    GooCanvasItem *item_2;
    GooCanvasItem *item_3;
    GooCanvasItem *item_fond;
    GooCanvasItem *select_mi;
    cairo_matrix_t transform;
    struct CMD_TYPE_PASSERELLE *pass;
    gint   cligno1;                                                               /* Etat cligno du motif */
    guchar rouge1;                                                            /* Couleur attendue du motif */
    guchar vert1;
    guchar bleu1;
    guchar en_cours_rouge1;                                                  /* Couleur actuelle du motif */
    guchar en_cours_vert1;
    guchar en_cours_bleu1;
    gint   cligno2;                                                                /* Etat cligno du motif */
    guchar rouge2;                                                            /* Couleur attendue du motif */
    guchar vert2;
    guchar bleu2;
    guchar en_cours_rouge2;                                                  /* Couleur actuelle du motif */
    guchar en_cours_vert2;
    guchar en_cours_bleu2;
    gint   cligno3;                                                                /* Etat cligno du motif */
    guchar rouge3;                                                            /* Couleur attendue du motif */
    guchar vert3;
    guchar bleu3;
    guchar en_cours_rouge3;                                                  /* Couleur actuelle du motif */
    guchar en_cours_vert3;
    guchar en_cours_bleu3;
    gint   groupe_dpl;                                                  /* Groupe de deplacement du motif */
    gint selection;
  };

 struct TRAME_ITEM_COMMENT
  { gint type;                                                                          /* Type de l'item */
    GooCanvasItem *item_groupe;
    GooCanvasItem *item;
    GooCanvasItem *select_mi;
    cairo_matrix_t transform;
    struct CMD_TYPE_COMMENT *comment;
    gint   groupe_dpl;                                                  /* Groupe de deplacement du motif */
    gint selection;
  };

 struct TRAME_ITEM_CAPTEUR
  { gint type;                                                                          /* Type de l'item */
    GooCanvasItem *item_groupe;
    GooCanvasItem *item_carre;
    GooCanvasItem *item_entry;
    GooCanvasItem *select_mi;
    cairo_matrix_t transform;
    struct CMD_TYPE_CAPTEUR *capteur;
    gint   groupe_dpl;                                                  /* Groupe de deplacement du motif */
    gint selection;
  };

 struct TRAME_ITEM_CAMERA_SUP
  { gint type;                                                                          /* Type de l'item */
    GooCanvasItem *item;
    cairo_matrix_t transform;
    GooCanvasItem *item_groupe;
    GooCanvasItem *select_mi;

    struct CMD_TYPE_CAMERA_SUP *camera_sup;
    gint   groupe_dpl;                                                  /* Groupe de deplacement du motif */
    gint selection;
  };


 struct TRAME_ITEM
  { union { struct TRAME_ITEM_MOTIF motif;
            struct TRAME_ITEM_PASS pass;
            struct TRAME_ITEM_COMMENT comment;
            struct TRAME_ITEM_CAPTEUR capteur;
            struct TRAME_ITEM_CAMERA_SUP camera_sup;
          };
  };

 struct TRAME
  { GooCanvasItem *canvas_root;
    GtkWidget *trame_widget;
    GooCanvasItem *fond;
    GList *trame_items;
  };

/************************************* Déclaration des prototypes******************************************/
 extern void Trame_rafraichir_motif ( struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Trame_rafraichir_comment ( struct TRAME_ITEM_COMMENT *trame_comment );
 extern void Trame_rafraichir_passerelle ( struct TRAME_ITEM_PASS *trame_pass );
 extern void Trame_rafraichir_capteur ( struct TRAME_ITEM_CAPTEUR *trame_capteur );
 extern void Trame_rafraichir_camera_sup ( struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup );
 extern void Trame_choisir_frame ( struct TRAME_ITEM_MOTIF *trame_motif, gint num,
                                   guchar r, guchar v, guchar b );
 extern void Trame_peindre_motif ( struct TRAME_ITEM_MOTIF *trame_motif, guchar r, guchar v, guchar b );
 extern void Trame_peindre_pass_1 ( struct TRAME_ITEM_PASS *trame_pass, guchar r, guchar v, guchar b );
 extern void Trame_peindre_pass_2 ( struct TRAME_ITEM_PASS *trame_pass, guchar r, guchar v, guchar b );
 extern void Trame_peindre_pass_3 ( struct TRAME_ITEM_PASS *trame_pass, guchar r, guchar v, guchar b );
 extern void Charger_gif ( struct TRAME_ITEM_MOTIF *trame_item, gchar *nom_fichier );
 extern void Charger_pixbuf_file ( struct TRAME_ITEM_MOTIF *trame_item, gchar *fichier );
 extern struct TRAME_ITEM_MOTIF *Trame_ajout_motif ( gint flag, struct TRAME *trame,
                                                     struct CMD_TYPE_MOTIF *motif );
 extern struct TRAME_ITEM_COMMENT *Trame_ajout_commentaire( gint flag, struct TRAME *trame,
                                                            struct CMD_TYPE_COMMENT *comm );
 extern struct TRAME_ITEM_PASS *Trame_ajout_passerelle ( gint flag, struct TRAME *trame,
                                                         struct CMD_TYPE_PASSERELLE *pass );
 extern struct TRAME_ITEM_CAPTEUR *Trame_ajout_capteur ( gint flag, struct TRAME *trame,
                                                     struct CMD_TYPE_CAPTEUR *capteur );
 extern void Trame_ajout_motif_par_item ( struct TRAME *trame,
                                          struct TRAME_ITEM_MOTIF *trame_motif );
 extern struct TRAME_ITEM_MOTIF *Trame_new_item ( void );
 extern struct TRAME_ITEM_CAMERA_SUP *Trame_ajout_camera_sup ( gint flag, struct TRAME *trame,
                                                               struct CMD_TYPE_CAMERA_SUP *camera_sup );
 extern void Trame_del_capteur ( struct TRAME_ITEM_CAPTEUR *trame_capteur );
 extern void Trame_del_passerelle ( struct TRAME_ITEM_PASS *trame_pass );
 extern void Trame_del_commentaire ( struct TRAME_ITEM_COMMENT *trame_comm );
 extern void Trame_del_item ( struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Trame_del_camera_sup ( struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup );
 extern struct TRAME *Trame_creer_trame ( guint taille_x, guint taille_y, char *coul, guint grille );
 extern void Trame_effacer_trame ( struct TRAME *trame );
 extern void Trame_detruire_trame ( struct TRAME *trame );

 #endif
/*--------------------------------------------------------------------------------------------------------*/
