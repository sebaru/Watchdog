/******************************************************************************************************************************/
/* Client/Include/protocli.h     Définition des prototypes de fonctions                                                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          dim 29 mar 2009 09:57:11 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * protocli.h.c
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

 #ifndef _PROTOCLI_H_
 #define _PROTOCLI_H_

 #include <gtk/gtk.h>
 #include <libsoup/soup.h>
 //#include <gtksourceview/gtksourceprintcompositor.h>
 #include <json-glib/json-glib.h>

 #include "client.h"
 #include "trame.h"

 #define TEMPS_MAX_PULSE   10                                            /* 10 secondes de battements maximum pour le serveur */

 #define PRINT_FOOTER_LEFT          "Sebastien & Bruno LEFEVRE"
 #define PRINT_FOOTER_CENTER        "Watchdog ABLS"
 #define PRINT_FOOTER_RIGHT         "1988-2017"
 #define PRINT_HEADER_RIGHT         "page %N / %Q"
 #define PRINT_FONT_NAME            "Monospace 10"
 #define PRINT_NBR_CHAR_GROUPE_PAGE 30

 enum
  { TYPE_PAGE_SYNOPTIQUE,                                                     /* Edition des noms/mnémoniques des synoptiques */
    TYPE_PAGE_ATELIER,                                                                   /* Il s'agit d'un atelier synoptique */
    TYPE_PAGE_SUPERVISION,                                                                /* Supervision graphique synoptique */
  };

 struct PAGE_NOTEBOOK
  { gint type;
    struct CLIENT *client;
    GtkWidget *child;                                                   /* Le widget toplevel, placé dans la page du notebook */
    void *infos;                                                                  /* Pointeur sur une structure TYPE_INFO_xxx */
  };

 struct TYPE_INFO_SUPERVISION
  { SoupWebsocketConnection *ws_motifs;
    guint timer_id;                                    /* Id du timer pour l'animation des motifs sur la trame de supervision */
    gboolean timer_hidden;                                            /* Pour savoir si les motifs doivent etre allumé ou non */
    JsonNode *syn;                                                                       /* Id du synoptique en cours de visu */
    GtkWidget *Dialog_horloge;                                                  /* Boite de dialogue d'affichage des horloges */
    GtkWidget *Liste_horloge;
    GtkWidget *Option_zoom;                                                               /* Choix du zoom sur la supervision */
    GtkWidget *Box_palette;                                                                   /* Widget de la boite a palette */
    GtkWidget *bouton_acq;                                                                   /* Bouton d'acquit du synoptique */
    struct TRAME *Trame;                                                                   /* La trame de fond de supervision */
  };

 struct TYPE_INFO_ATELIER
  { JsonNode *syn;                                                                       /* Id du synoptique en cours de visu */
                                                                    /* Interface de plus haut niveau: affichage du synoptique */
    struct TRAME *Trame_atelier;                                                             /* La trame de fond de l'atelier */
    gint groupe_max;                                                       /* Numéro max du groupe utilisé dans le synoptique */
    GSList *Selection;                                                             /* Les de TRAME_ITEM qui sont sélectionnés */
    GtkWidget *Option_zoom;                                                                    /* Choix du zoom sur l'atelier */
    GtkWidget *Check_grid;                                                                 /* La grille est-elle magnétique ? */
    GtkWidget *Spin_grid;                                                  /* Quel est l'ecartement de la grille magnetique ? */
    GtkWidget *Entry_posxy;                                                /* Affichage des coordonnées X de l'objet en cours */
    GtkWidget *Entry_libelle;                                                       /* Gestion du libelle de l'objet en cours */
    GtkAdjustment *Adj_angle;                                                        /* Angle de rotation de l'objet en cours */
    GtkAdjustment *Adj_scale;                                                                     /* Zoom de l'objet en cours */
    gdouble Clic_x, Clic_y;
    gint Appui;
  };

/*--------------------------------------- Déclarations des prototypes de fonctions -------------------------------------------*/
 extern GtkWidget *Creer_page_histo( struct CLIENT *Client );                                           /* Dans liste_histo.c */
 extern void Reset_page_histo( struct CLIENT *client );
 extern void Afficher_histo_alive_CB (SoupSession *session, SoupMessage *msg, gpointer user_data);
 extern void Updater_histo ( struct CLIENT *client, JsonNode *element );

 extern gboolean Timer ( gpointer data );                                                                     /* Dans timer.c */

 extern void Log( struct CLIENT *client, gchar *chaine );                                                       /* Dans ihm.c */
 extern GtkWidget *Creer_boite_travail ( struct CLIENT *Client );
 extern GtkWidget *Bouton ( gchar *libelle, gchar *icone, gchar *tooltip );
 extern GtkWidget *Menu ( gchar *libelle, gchar *icone );
 extern void Effacer_pages ( struct CLIENT *client );
 extern void Update_progress_bar( SoupMessage *msg, SoupBuffer *chunk, gpointer data );
 extern void Set_progress_pulse( struct CLIENT *client );
 extern struct PAGE_NOTEBOOK *Chercher_page_notebook ( struct CLIENT *client, guint type, guint id, gboolean affiche );

 extern void Connecter ( struct CLIENT *Client );                                                           /* Dans connect.c */
 extern void Deconnecter ( struct CLIENT *Client );
 extern void Envoi_json_au_serveur ( struct CLIENT *client, gchar *methode, JsonBuilder *builder, gchar *URI, SoupSessionCallback callback );
 extern void Envoi_json_au_serveur_new ( struct CLIENT *client, gchar *methode, JsonNode *RootNode, gchar *URI, SoupSessionCallback callback );
 extern void Envoi_ws_au_serveur ( struct CLIENT *client, SoupWebsocketConnection *ws, JsonBuilder *builder );

 extern void Menu_want_liste_synoptique ( struct CLIENT *client );
 extern void Detruire_page_liste_synoptique( struct PAGE_NOTEBOOK *page );
 extern void Detruire_page_atelier ( struct PAGE_NOTEBOOK *page );                                          /* Dans atelier.c */
 extern void Creer_page_atelier_CB (SoupSession *session, SoupMessage *msg, gpointer user_data);

 extern void Clic_sur_motif ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                              struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Clic_sur_comment ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                                struct TRAME_ITEM_COMMENT *trame_comment );
 extern void Clic_sur_pass ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                             struct TRAME_ITEM_PASS *trame_pass );
 extern void Clic_sur_cadran ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                               struct TRAME_ITEM_CADRAN *trame_cadran );
                                                                                                  /* Dans atelier_selection.c */
 extern void Tout_deselectionner ( struct PAGE_NOTEBOOK *page );
 extern void Selectionner ( struct PAGE_NOTEBOOK *page, gpointer trame_item, gint groupe );
 extern void Deplacer_selection (  struct PAGE_NOTEBOOK *page, gint deltax, gint deltay );
 extern void Rotationner_selection ( struct PAGE_NOTEBOOK *page );
 extern void Zoomer_selection ( struct PAGE_NOTEBOOK *page );
 extern void Effacer_selection ( struct PAGE_NOTEBOOK *page );
 extern void Dupliquer_selection ( struct PAGE_NOTEBOOK *page );
 extern void Deselectionner ( struct TYPE_INFO_ATELIER *infos, struct TRAME_ITEM *item );
 extern void Fusionner_selection ( struct PAGE_NOTEBOOK *page );
 extern void Detacher_selection ( struct PAGE_NOTEBOOK *page );
 extern void Mettre_echelle_selection_1_1 ( struct PAGE_NOTEBOOK *page );
 extern void Mettre_echelle_selection_1_Y ( struct PAGE_NOTEBOOK *page );
 extern void Mettre_echelle_selection_X_1 ( struct PAGE_NOTEBOOK *page );

 extern void Afficher_un_motif (JsonArray *array, guint index, JsonNode *element, gpointer user_data);/* Dans atelier_motif.c */
 extern void Updater_un_visuel( struct TRAME_ITEM_MOTIF *trame_motif, JsonNode *motif );
                                                                                                    /* Dans atelier_comment.c */
 extern void Afficher_un_commentaire (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
                                                                                                 /* Dans atelier_passerelle.c */
 extern void Afficher_une_passerelle (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
                                                                                                     /* Dans atelier_cadran.c */
 extern void Afficher_un_cadran (JsonArray *array, guint index, JsonNode *element, gpointer user_data);

                                                                                                        /* Dans supervision.c */
 extern void Menu_want_supervision_accueil( struct CLIENT *client );
 extern void Demander_synoptique_supervision ( struct CLIENT *client, gint id );
 extern void Detruire_page_supervision( struct PAGE_NOTEBOOK *page );
 extern void Clic_sur_motif_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                          GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Clic_sur_bouton_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
                                                                                                /* Dans supervision_comment.c */

                                                                                             /* Dans supervision_passerelle.c */
 extern gboolean Supervision_clic_passerelle (GooCanvasItem *canvasitem, GooCanvasItem *target,
                                              GdkEvent *event, struct TRAME_ITEM_PASS *trame_pass );
                                                                                                /* Dans supervision_cadran.c */
 extern void Updater_les_cadrans( struct PAGE_NOTEBOOK *page, JsonNode *cadran );
 extern void Clic_sur_cadran_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                           GdkEvent *event, struct TRAME_ITEM_CADRAN *trame_cadran );

/************************************************ Définitions des prototypes **************************************************/
 extern JsonNode *Json_node_create ( void );
 extern void Json_node_add_string ( JsonNode *RootNode, gchar *name, gchar *chaine );
 extern void Json_node_add_bool ( JsonNode *RootNode, gchar *name, gboolean valeur );
 extern void Json_node_add_int ( JsonNode *RootNode, gchar *name, gint64 valeur );
 extern void Json_node_add_double ( JsonNode *RootNode, gchar *name, gdouble valeur );
 extern JsonArray *Json_node_add_array ( JsonNode *RootNode, gchar *name );
 extern JsonNode *Json_node_add_objet ( JsonNode *RootNode, gchar *name );
 extern void Json_array_add_element ( JsonArray *array, JsonNode *element );
 extern void Json_node_foreach_array_element ( JsonNode *RootNode, gchar *nom, JsonArrayForeach fonction, gpointer data );
 extern gchar *Json_node_to_string ( JsonNode *RootNode );

 extern JsonBuilder *Json_create ( void );
 extern void Json_add_string ( JsonBuilder *builder, gchar *name, gchar *chaine );
 extern void Json_add_int ( JsonBuilder *builder, gchar *name, gint valeur );
 extern void Json_add_double ( JsonBuilder *builder, gchar *name, gdouble valeur );
 extern void Json_add_bool ( JsonBuilder *builder, gchar *name, gboolean valeur );
 extern void Json_add_object ( JsonBuilder *builder, gchar *name );
 extern void Json_end_object ( JsonBuilder *builder );
 extern void Json_add_array ( JsonBuilder *builder, gchar *name );
 extern void Json_end_array ( JsonBuilder *builder );
 extern gchar *Json_get_buf ( JsonBuilder *builder, gsize *taille_buf_p );
 extern JsonNode *Json_get_from_string ( gchar *chaine );
 extern gchar *Json_get_string ( JsonNode *query, gchar *chaine );
 extern gdouble Json_get_double ( JsonNode *query, gchar *chaine );
 extern gint Json_get_int ( JsonNode *query, gchar *chaine );
 extern gboolean Json_get_bool ( JsonNode *query, gchar *chaine );
 extern JsonArray *Json_get_array ( JsonNode *query, gchar *chaine );
 extern gboolean Json_has_member ( JsonNode *query, gchar *chaine );
 extern JsonObject *Json_get_object_as_object ( JsonNode *query, gchar *chaine );
 extern JsonNode *Json_get_object_as_node ( JsonNode *query, gchar *chaine );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/

