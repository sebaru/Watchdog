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
 #include <curl/curl.h>
 #include <json-glib/json-glib.h>

 #include "Reseaux.h"
 #include "client.h"
 #include "trame.h"

 #define TEMPS_MAX_PULSE   10                                            /* 10 secondes de battements maximum pour le serveur */

 #define PRINT_FOOTER_LEFT          "Sebastien & Bruno LEFEVRE"
 #define PRINT_FOOTER_CENTER        "Watchdog ABLS"
 #define PRINT_FOOTER_RIGHT         "1988-2017"
 #define PRINT_HEADER_RIGHT         "page %N / %Q"
 #define PRINT_FONT_NAME            "Monospace 10"
 #define PRINT_NBR_CHAR_GROUPE_PAGE 30
 #define WATCHDOG_USER_AGENT        "Watchdog Client - libcurl"

 enum
  { TYPE_PAGE_PLUGIN_DLS,                                                                         /* Listes des plugins D.L.S */
    TYPE_PAGE_SYNOPTIQUE,                                                     /* Edition des noms/mnémoniques des synoptiques */
    TYPE_PAGE_ALL_MNEMONIQUE,                                                /* Page de visualisation de tous les mnemoniques */
    TYPE_PAGE_MNEMONIQUE,                                                            /* Page de visualisation des mnemoniques */
    TYPE_PAGE_ICONE,                                                    /* Ajout/retrait/modif des icones et classes d'icones */
    TYPE_PAGE_SOURCE_DLS,                                                                       /* Edition d'une source D.L.S */
    TYPE_PAGE_HISTO_MSGS,                                                                         /* Parcours de l'historique */
    TYPE_PAGE_ATELIER,                                                                   /* Il s'agit d'un atelier synoptique */
    TYPE_PAGE_SUPERVISION,                                                                /* Supervision graphique synoptique */
    TYPE_PAGE_CAMERA,                                                                                   /* Gestion des camera */
    TYPE_PAGE_ADMIN,                                               /* Page de gestion des commandes/requests d'administration */
    TYPE_PAGE_HORLOGE
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

 struct TYPE_INFO_HORLOGE
  { guint id_mnemo;
    GtkWidget *Liste_horloge;
  };

 struct TYPE_INFO_MNEMONIQUE
  { gint id;                                                 /* ID du module DLS dont les mnemoniques sont en cours d'edition */
    GtkWidget *Liste_mnemonique;                                      /* GtkTreeView pour la gestion des mnemoniques Watchdog */
  };

 struct TYPE_INFO_SOURCE_DLS
  { GtkWidget *text;                                  /* Pour les plugins DLS, ici est placé le widget TextView correspondant */
    struct CMD_TYPE_PLUGIN_DLS rezo_dls;
    GtkWidget *F_mnemo;
    GtkWidget *Option_type;
    GtkWidget *Spin_num;
  };

 struct TYPE_INFO_HISTO_MSGS
  { SoupWebsocketConnection *websocket;
    GtkListStore *Liste_histo_msgs;                                      /* GtkTreeView pour la gestion des messages Watchdog */
    GtkWidget *F_histo_msgs;
    GtkWidget *Check_num;
    GtkWidget *Spin_num;
    GtkWidget *Check_type;
    GtkWidget *Option_type;
    GtkWidget *Check_nom_ack;
    GtkWidget *Entry_nom_ack;
    GtkWidget *Check_libelle;
    GtkWidget *Entry_libelle;
    GtkWidget *Check_groupage;
    GtkWidget *Entry_groupage;
    GtkWidget *Check_debut;
    GtkWidget *Date_debut;
    GtkWidget *Check_fin;
    GtkWidget *Date_fin;
    gint page_id;
  };

 struct TYPE_INFO_ATELIER
  { JsonNode *syn;                                                                       /* Id du synoptique en cours de visu */
                                                                    /* Interface de plus haut niveau: affichage du synoptique */
    struct TRAME *Trame_atelier;                                                             /* La trame de fond de l'atelier */
    gint new_layer;                                               /* Numéro du prochain groupe "layer" pour grouper les items */
    GSList *Selection;                                                             /* Les de TRAME_ITEM qui sont sélectionnés */
    GtkWidget *Option_zoom;                                                                    /* Choix du zoom sur l'atelier */
    GtkWidget *Check_grid;                                                                 /* La grille est-elle magnétique ? */
    GtkWidget *Spin_grid;                                                  /* Quel est l'ecartement de la grille magnetique ? */
    GtkWidget *Entry_posxy;                                                /* Affichage des coordonnées X de l'objet en cours */
    GtkWidget *Entry_libelle;                                                       /* Gestion du libelle de l'objet en cours */
    GtkAdjustment *Adj_angle;                                                        /* Angle de rotation de l'objet en cours */
    gdouble Clic_x, Clic_y;
    gint Appui;
  };

 enum                                       /* Numéro des colonnes dans les listes CAM (liste_camera et atelier_ajout_camera) */
  {  COL_CAM_ID,
     COL_CAM_NUM,
     COL_CAM_LOCATION,
     COL_CAM_LIBELLE,
     NBR_COL_CAM
  };

/*--------------------------------------- Déclarations des prototypes de fonctions -------------------------------------------*/
 extern GtkWidget *Creer_page_histo( struct CLIENT *Client );                                           /* Dans liste_histo.c */
 extern void Traiter_reception_ws_msgs_CB ( SoupWebsocketConnection *self, gint type, GBytes *message_brut, gpointer user_data );
 extern void Reset_page_histo( struct CLIENT *client );
 extern void Afficher_histo_alive_CB (SoupSession *session, SoupMessage *msg, gpointer user_data);
// extern void Acquitter_histo ( struct CLIENT *Client );

 extern gboolean Timer ( gpointer data );                                                                     /* Dans timer.c */

 extern void Log( struct CLIENT *client, gchar *chaine );                                                       /* Dans ihm.c */
 extern GtkWidget *Creer_boite_travail ( struct CLIENT *Client );
 extern GtkWidget *Bouton ( gchar *libelle, gchar *icone, gchar *tooltip );
 extern GtkWidget *Menu ( gchar *libelle, gchar *icone );
 extern void Effacer_pages ( struct CLIENT *client );
 extern void Update_progress_bar( SoupMessage *msg, SoupBuffer *chunk, gpointer data );
 extern void Set_progress_pulse( struct CLIENT *client );
 extern struct PAGE_NOTEBOOK *Chercher_page_notebook ( struct CLIENT *client, guint type, guint id, gboolean affiche );
#ifdef bouh
 extern void Detruire_page ( struct PAGE_NOTEBOOK *page_a_virer );
 extern void Set_progress_plus( gint plus );
 extern void Set_progress_text( gchar *cadran, gint max );
 extern void Set_progress_ratio( gint nbr, gint max );
 extern void Set_progress_pulse( void );
 extern struct PAGE_NOTEBOOK *Chercher_page_notebook ( guint type, guint id, gboolean affiche );
 extern gboolean Tester_page_notebook ( guint type );
 extern gint Nbr_page_type ( gint type );
 extern struct PAGE_NOTEBOOK *Page_actuelle ( void );
#endif

 extern void Connecter ( struct CLIENT *Client );                                                           /* Dans connect.c */
 extern void Deconnecter ( struct CLIENT *Client );
 extern void Envoi_au_serveur ( struct CLIENT *Client, gchar *methode, gchar *payload, gsize taille_buf, gchar *URI, SoupSessionCallback callback );
 extern void Envoi_json_au_serveur ( struct CLIENT *client, gchar *methode, JsonBuilder *builder, gchar *URI, SoupSessionCallback callback );
#ifdef bouh
 extern gboolean Connecter_ssl ( void );
 extern gboolean Connecter_au_serveur ( void );
 extern void Envoyer_authentification ( void );
 extern void Deconnecter_sale ( void );
 extern gboolean Envoi_serveur ( gint tag, gint ss_tag, gchar *buffer, gint taille );
 extern gboolean Changer_password ( void );


 extern gboolean WTD_Curl_post_request ( gchar *uri, gint post, gchar *post_data, gint post_length );
 extern CURL *WTD_Curl_init ( gchar *erreur );
 extern void Firefox_exec ( gchar *uri );

 extern void Ecouter_serveur ( void );                                                                    /* Dans protocole.c */
 extern void Gerer_protocole_gtk_message ( struct CONNEXION *connexion );
 extern void Gerer_protocole_icone ( struct CONNEXION *connexion );
 extern void Gerer_protocole_dls ( struct CONNEXION *connexion );
 extern void Gerer_protocole_synoptique ( struct CONNEXION *connexion );
 extern void Gerer_protocole_mnemonique ( struct CONNEXION *connexion );
 extern void Gerer_protocole_supervision ( struct CONNEXION *connexion );
 extern void Gerer_protocole_histo ( struct CONNEXION *connexion );
 extern void Gerer_protocole_atelier ( struct CONNEXION *connexion );
 extern void Gerer_protocole_fichier_connecte ( struct CONNEXION *connexion );
 extern void Gerer_protocole_connexion ( struct CONNEXION *connexion );
 extern void Gerer_protocole_lowlevel ( struct CONNEXION *connexion );
 extern void Gerer_protocole_admin ( struct CONNEXION *connexion );
 extern gint Get_icone_version( void );

 extern void Proto_afficher_un_admin( struct CMD_TYPE_ADMIN *admin );                                         /* Dans admin.c */
 extern void Creer_page_admin( void );

#endif
 extern void Menu_want_edition_DLS ( struct CLIENT *client );                                      /* Dans liste_plugin_dls.c */
 extern void Detruire_page_plugin_dls( struct PAGE_NOTEBOOK *page );
#ifdef bouh
 extern void Proto_afficher_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *dls );
 extern void Proto_cacher_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *dls );
 extern void Proto_rafraichir_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *dls );
 extern void Creer_page_plugin_dls( void );

 extern void Menu_ajouter_editer_plugin_dls ( struct CMD_TYPE_PLUGIN_DLS *edit_dls );                   /* ajout_plugin_dls.c */
 extern void Proto_afficher_un_syn_for_plugin_dls ( struct CMD_TYPE_SYNOPTIQUE *syn );

 extern void Creer_page_source_dls( struct CMD_TYPE_PLUGIN_DLS *rezo_dls );                         /* Dans edit_source_dls.c */
 extern void Proto_append_source_dls( struct CMD_TYPE_SOURCE_DLS *dls, gchar *buffer );
 extern void Proto_afficher_mnemo_dls ( struct CMD_TYPE_MNEMO_BASE *mnemo );
 extern void Dls_set_compil_status ( gchar *chaine );

 extern void Proto_cacher_un_synoptique( struct CMD_TYPE_SYNOPTIQUE *synoptique );                  /* Dans liste_synoptique.c*/
 extern void Proto_afficher_un_synoptique( struct CMD_TYPE_SYNOPTIQUE *synoptique );
 extern void Proto_rafraichir_un_synoptique( struct CMD_TYPE_SYNOPTIQUE *synoptique );
 extern void Creer_page_synoptique( void );
#endif
                                                                                                    /* Dans liste_synoptique.c*/
 extern void Menu_want_liste_synoptique ( struct CLIENT *client );
 extern void Detruire_page_liste_synoptique( struct PAGE_NOTEBOOK *page );
#ifdef bouh

 extern void Proto_cacher_un_mnemonique( struct CMD_TYPE_MNEMO_BASE *mnemonique );                  /* Dans liste_mnemonique.c*/
 extern void Proto_afficher_un_mnemonique( struct CMD_TYPE_MNEMO_BASE *mnemonique );
 extern void Proto_afficher_tous_mnemonique( struct CMD_TYPE_MNEMO_BASE *mnemonique );
 extern void Proto_rafraichir_un_mnemonique( struct CMD_TYPE_MNEMO_BASE *smnemonique );
 extern gchar *Type_bit_interne ( gint num );
 extern gchar *Type_bit_interne_court ( gint num );
 extern gint Type_bit_interne_int ( gchar *type );
 extern void Creer_page_mnemonique( struct CMD_TYPE_PLUGIN_DLS *plugin );
 extern void Creer_page_all_mnemonique( void );

 extern void Menu_ajouter_editer_mnemonique ( struct CMD_TYPE_MNEMO_FULL *mnemo_full, gint dls_id );     /* ajout_mnemonique.c*/
 extern void Proto_afficher_un_syn_for_mnemonique ( struct CMD_TYPE_SYNOPTIQUE *syn );

 extern void Menu_want_plugin_dls ( void );                                                                    /* Dans menu.c */
 extern void Menu_want_client_leger ( void );
 extern void Menu_want_synoptique ( void );
 extern void Menu_want_camera ( void );
 extern void Menu_want_histo_msgs ( void );
 extern void Menu_want_page_admin ( void );
 extern void Menu_want_compilation_forcee ( void );

#endif
 extern void Detruire_page_atelier ( struct PAGE_NOTEBOOK *page );                                          /* Dans atelier.c */
 extern void Creer_page_atelier_CB (SoupSession *session, SoupMessage *msg, gpointer user_data);

#ifdef bouh

 extern void Proto_afficher_un_motif_atelier( struct CMD_TYPE_MOTIF *motif );                               /* Dans atelier.c */
 extern void Proto_cacher_un_motif_atelier( struct CMD_TYPE_MOTIF *motif );
 extern void Reduire_en_vignette ( struct CMD_TYPE_MOTIF *motif );
 extern void Creer_page_atelier( gint syn_id, gchar *libelle_syn );
 extern void Detruire_page_liste_synoptique ( struct PAGE_NOTEBOOK *page );
 extern struct TYPE_INFO_ATELIER *Rechercher_infos_atelier_par_id_syn ( gint syn_id );

#endif
                                                                                                 /* Dans atelier_clic_trame.c */
 extern void Clic_sur_motif ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                              struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Clic_sur_comment ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                                struct TRAME_ITEM_COMMENT *trame_comment );
 extern void Clic_sur_pass ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                             struct TRAME_ITEM_PASS *trame_pass );
 extern void Clic_sur_cadran ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                               struct TRAME_ITEM_CADRAN *trame_cadran );
 extern void Clic_sur_camera_sup ( GooCanvasItem *widget, GooCanvasItem *target, GdkEvent *event,
                                   struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup );
#ifdef bouh
 extern void Clic_sur_fond ( struct TYPE_INFO_ATELIER *infos, GdkEvent *event, gpointer data );
#endif
                                                                                                  /* Dans atelier_selection.c */
 extern void Tout_deselectionner ( struct PAGE_NOTEBOOK *page );
 extern void Selectionner ( struct PAGE_NOTEBOOK *page, gint layer );
 extern void Deplacer_selection (  struct PAGE_NOTEBOOK *page, gint deltax, gint deltay );
 extern void Rotationner_selection ( struct PAGE_NOTEBOOK *page );
 extern void Effacer_selection ( struct PAGE_NOTEBOOK *page );
 extern void Dupliquer_selection ( struct PAGE_NOTEBOOK *page );
 extern void Deselectionner ( struct TYPE_INFO_ATELIER *infos, struct TRAME_ITEM *item );
 extern void Fusionner_selection ( struct PAGE_NOTEBOOK *page );
 extern void Detacher_selection ( struct PAGE_NOTEBOOK *page );
 extern void Mettre_echelle_selection_1_1 ( struct PAGE_NOTEBOOK *page );
 extern void Mettre_echelle_selection_1_Y ( struct PAGE_NOTEBOOK *page );
 extern void Mettre_echelle_selection_X_1 ( struct PAGE_NOTEBOOK *page );
                                                                                                   /* Dans atelier_agrandir.c */
 extern void Agrandir_bd ( GooCanvasItem *widget, GooCanvasItem *target,
                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Agrandir_bg ( GooCanvasItem *widget, GooCanvasItem *target,
                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Agrandir_hd ( GooCanvasItem *widget, GooCanvasItem *target,
                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Agrandir_hg ( GooCanvasItem *widget, GooCanvasItem *target,
                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
#ifdef bouh

 extern void Creer_fenetre_propriete_TOR ( struct TYPE_INFO_ATELIER *infos );                     /* Dans atelier_propriete.c */
 extern void Detruire_fenetre_propriete_TOR ();
 extern void Editer_propriete_TOR ( struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Changer_couleur_motif_directe( struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Proto_afficher_mnemo_atelier ( int tag, struct CMD_TYPE_MNEMO_BASE *mnemo );


#endif
 extern void Afficher_un_motif (JsonArray *array, guint index, JsonNode *element, gpointer user_data);/* Dans atelier_motif.c */
#ifdef bouh

 extern void Creer_fenetre_ajout_motif ( void );
 extern void Detruire_fenetre_ajout_motif ( void );
 extern void Choisir_motif_a_ajouter ( void );
#endif

                                                                                                    /* Dans atelier_comment.c */
 extern void Afficher_un_commentaire (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
#ifdef bouh
 extern void Creer_fenetre_ajout_commentaire ( void );
 extern void Proto_afficher_un_comment_atelier( struct CMD_TYPE_COMMENT *rezo_comment );
 extern void Proto_cacher_un_comment_atelier( struct CMD_TYPE_COMMENT *comment );

#endif
                                                                                                 /* Dans atelier_passerelle.c */
 extern void Afficher_une_passerelle (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
#ifdef bouh
 extern void Creer_fenetre_ajout_passerelle ( void );
 extern void Proto_afficher_un_syn_for_passerelle_atelier( struct CMD_TYPE_SYNOPTIQUE *syn );
 extern void Proto_afficher_une_passerelle_atelier( struct CMD_TYPE_PASSERELLE *rezo_pass );
 extern void Proto_cacher_une_passerelle_atelier( struct CMD_TYPE_PASSERELLE *pass );

#endif
                                                                                                     /* Dans atelier_cadran.c */
 extern void Afficher_un_cadran (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
#ifdef bouh
 extern void Menu_ajouter_editer_cadran ( struct TRAME_ITEM_CADRAN *trame_cadran );
 extern void Proto_afficher_un_cadran_atelier( struct CMD_TYPE_CADRAN *rezo_cadran );
 extern void Proto_cacher_un_cadran_atelier( struct CMD_TYPE_CADRAN *cadran );
 extern void Proto_afficher_mnemo_cadran_atelier ( struct CMD_TYPE_MNEMO_BASE *mnemo );

                                                                                              /* Dans atelier_ajout_palette.c */
 extern void Creer_fenetre_ajout_palette ( struct TYPE_INFO_ATELIER *infos );
 extern void Proto_afficher_une_palette_atelier( struct CMD_TYPE_PALETTE *rezo_palette );
 extern void Proto_cacher_une_palette_atelier( struct CMD_TYPE_PALETTE *palette );
 extern void Proto_afficher_un_syn_for_palette_atelier( struct CMD_TYPE_SYNOPTIQUE *synoptique );

#endif
                                                                                                     /* Dans atelier_camera.c */
 extern void Afficher_une_camera (JsonArray *array, guint index, JsonNode *element, gpointer user_data);

#ifdef bouh
 extern struct TRAME_ITEM_CAMERA_SUP *Id_vers_trame_camera_sup ( struct TYPE_INFO_ATELIER *infos, gint id );
 extern void Menu_ajouter_camera_sup ( void );
 extern void Proto_afficher_un_camera_for_atelier( struct CMD_TYPE_CAMERA *camera );
 extern void Proto_afficher_un_camera_sup_atelier( struct CMD_TYPE_CAMERASUP *rezo_camera_sup );
 extern void Proto_cacher_un_camera_sup_atelier( struct CMD_TYPE_CAMERASUP *camera_sup );

#endif
                                                                                                        /* Dans supervision.c */
 extern void Menu_want_supervision_accueil( struct CLIENT *client );
 extern void Demander_synoptique_supervision ( struct CLIENT *client, gint id );
 extern void Creer_page_supervision_CB (SoupSession *session, SoupMessage *msg, gpointer user_data);
 extern void Detruire_page_supervision( struct PAGE_NOTEBOOK *page );
 extern void Clic_sur_motif_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                          GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );

                                                                                                /* Dans supervision_comment.c */

                                                                                                 /* Dans supervision_camera.c */
 extern void Clic_sur_camera_sup_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                               GdkEvent *event, struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup );

#ifdef bouh
 extern void Detruire_page_supervision( struct PAGE_NOTEBOOK *page );
 extern void Proto_afficher_un_motif_supervision( struct CMD_TYPE_MOTIF *rezo_motif );
 extern void Proto_changer_etat_motif( struct CMD_ETAT_BIT_CTRL *etat_motif );
 extern void Proto_set_syn_vars( struct CMD_TYPE_SYN_VARS *syn_vars );
 extern struct TYPE_INFO_SUPERVISION *Rechercher_infos_supervision_par_id_syn ( gint syn_id );
 extern void Proto_afficher_une_horloge( struct TYPE_INFO_SUPERVISION *infos, struct CMD_TYPE_MNEMO_BASE *mnemo );
                                                                                                   /* Dans supervision_clic.c */
 extern void Clic_sur_motif_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Clic_sur_camera_sup_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                               GdkEvent *event, struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup );
 extern void Clic_sur_cadran_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                            GdkEvent *event, struct TRAME_ITEM_CADRAN *trame_cadran );


#endif
                                                                                             /* Dans supervision_passerelle.c */
 extern void Changer_vue_directe ( struct CLIENT *client, guint num_syn );
 extern gboolean Supervision_clic_passerelle (GooCanvasItem *canvasitem, GooCanvasItem *target,
                                              GdkEvent *event, struct TRAME_ITEM_PASS *trame_pass );
#ifdef bouh
                                                                                                /* Dans supervision_palette.c */
 extern void Proto_afficher_une_palette_supervision( struct CMD_TYPE_PALETTE *rezo_palette );

#endif
                                                                                                 /* Dans supervision_cadran.c */
 extern void Updater_les_cadrans( struct TYPE_INFO_SUPERVISION *infos, JsonNode *cadran );
 extern void Clic_sur_cadran_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                           GdkEvent *event, struct TRAME_ITEM_CADRAN *trame_cadran );
#ifdef bouh
 extern void Proto_changer_etat_cadran( struct CMD_ETAT_BIT_CADRAN *etat_cadran );


                                                                                               /* Dans supervision_horloges.c */
 extern void Proto_afficher_une_horloge_supervision( struct CMD_TYPE_MNEMO_BASE *mnemo );

                                                                                                   /* Dans option_entreetor.c */
 extern void Get_options_DI ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );
 extern GtkWidget *Get_options_DI_gtktable ( void );
 extern void Set_options_DI ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );

                                                                                                   /* Dans option_entreeana.c */
 extern void Get_options_AI ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );
 extern GtkWidget *Get_options_AI_gtktable ( void );
 extern void Set_options_AI ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );

                                                                                                    /* Dans option_registre.c */
 extern void Get_options_Registre ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );
 extern GtkWidget *Get_options_Registre_gtktable ( void );
 extern void Set_options_Registre ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );

                                                                                                       /* Dans option_tempo.c */
 extern void Get_options_Tempo ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );
 extern GtkWidget *Get_options_Tempo_gtktable ( void );
 extern void Set_options_Tempo ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );

                                                                                                     /* Dans option_cpt_imp.c */
 extern void Get_options_CPTIMP ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );
 extern GtkWidget *Get_options_CPTIMP_gtktable ( void );
 extern void Set_options_CPTIMP ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );

 extern GtkPrintOperation *New_print_job ( gchar *nom );                                                      /* Dans print.c */
 extern void Print_draw_page ( GtkPrintOperation *operation,
                               GtkPrintContext   *context,
                               gint               page_nr,
                               gpointer           user_data );
 extern gboolean Print_paginate ( GtkPrintOperation *operation,
                                  GtkPrintContext   *context,
                                  gpointer           user_data );

 extern void Proto_afficher_un_camera( struct CMD_TYPE_CAMERA *camera );                               /* Dans liste_camera.c */
 extern void Proto_cacher_un_camera( struct CMD_TYPE_CAMERA *camera );
 extern void Proto_rafraichir_un_camera( struct CMD_TYPE_CAMERA *camera );
 extern void Creer_page_camera( void );
 extern gchar *Type_camera_vers_string ( guint type );
 extern void Creer_liste_camera ( GtkWidget **Liste_camera, GtkWidget **Scroll );
 extern  void Rafraichir_visu_camera( GtkListStore *store, GtkTreeIter *iter, struct CMD_TYPE_CAMERA *camera );

                                                                                                       /* Dans ajout_camera.c */
 extern void Menu_ajouter_editer_camera ( struct CMD_TYPE_CAMERA *edit_camera );
 extern void Proto_afficher_mnemo_camera ( int tag, struct CMD_TYPE_MNEMO_BASE *mnemo );

 extern void Creer_page_horloge ( gchar *libelle, guint id_mnemo );                                         /* Dans Horloge.c */
 extern void Proto_afficher_un_tick( struct CMD_TYPE_MNEMO_FULL *mnemo );
 extern void Proto_cacher_un_tick( struct CMD_TYPE_MNEMO_FULL *mnemo );
 extern void Proto_rafraichir_un_tick( struct CMD_TYPE_MNEMO_FULL *mnemo );

 extern void Menu_ajouter_editer_horloge ( struct CMD_TYPE_MNEMO_FULL *edit_horloge, gint id_mnemo);  /* Dans ajout_horloge.c */
#endif

/************************************************ Définitions des prototypes **************************************************/
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
 extern gfloat Json_get_float ( JsonNode *query, gchar *chaine );
 extern gint Json_get_int ( JsonNode *query, gchar *chaine );
 extern gboolean Json_get_bool ( JsonNode *query, gchar *chaine );
 extern JsonArray *Json_get_array ( JsonNode *query, gchar *chaine );
 extern gboolean Json_has_member ( JsonNode *query, gchar *chaine );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/

