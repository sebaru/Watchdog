/******************************************************************************************************************************/
/* Client/Include/protocli.h     Définition des prototypes de fonctions                                                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          dim 29 mar 2009 09:57:11 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * protocli.h.c
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

 #ifndef _PROTOCLI_H_
 #define _PROTOCLI_H_

 #include <gnome.h>
 #include <openssl/ssl.h>
 #include <gtkdatabox.h>
 #include <gtksourceview/gtksourceprintcompositor.h>

 #include "Reseaux.h"
 #include "trame.h"


 #define TEMPS_MAX_PULSE   10                                            /* 10 secondes de battements maximum pour le serveur */

 #define PRINT_FOOTER_LEFT          "Sebastien & Bruno LEFEVRE"
 #define PRINT_FOOTER_CENTER        "Watchdog ABLS"
 #define PRINT_FOOTER_RIGHT         "1988-2017"
 #define PRINT_HEADER_RIGHT         "page %N / %Q"
 #define PRINT_FONT_NAME            "Monospace 10"
 #define PRINT_NBR_CHAR_GROUPE_PAGE 30

 enum
  { TYPE_PAGE_PLUGIN_DLS,                                                                         /* Listes des plugins D.L.S */
    TYPE_PAGE_HISTO,                                                               /* Page de garde: messages au fil de l'eau */
    TYPE_PAGE_GROUPE,                                                                      /* Edition des groupes de Watchdog */
    TYPE_PAGE_UTIL,                                                                        /* Liste des utilisateurs Watchdog */
    TYPE_PAGE_MESSAGE,                                                                       /* Edition des messages Watchdog */
    TYPE_PAGE_SYNOPTIQUE,                                                     /* Edition des noms/mnémoniques des synoptiques */
    TYPE_PAGE_MNEMONIQUE,                                                            /* Page de visualisation des mnemoniques */
    TYPE_PAGE_ICONE,                                                    /* Ajout/retrait/modif des icones et classes d'icones */
    TYPE_PAGE_SOURCE_DLS,                                                                       /* Edition d'une source D.L.S */
    TYPE_PAGE_HISTO_MSGS,                                                                         /* Parcours de l'historique */
    TYPE_PAGE_ATELIER,                                                                   /* Il s'agit d'un atelier synoptique */
    TYPE_PAGE_SUPERVISION,                                                                /* Supervision graphique synoptique */
    TYPE_PAGE_CAMERA,                                                                                   /* Gestion des camera */
#ifdef bouh
    TYPE_PAGE_ENTREEANA,                                                                 /* Il s'agit d'un atelier synoptique */
    TYPE_PAGE_ONDULEUR,                                                            /* Page affichant la liste des onduleurs ! */
    TYPE_PAGE_RS485,                                                                      /* Page affichant les modules RS485 */
    TYPE_PAGE_MODBUS,                                                                    /* Page affichant les modules MODBUS */
    TYPE_PAGE_RFXCOM,                                                                    /* Page affichant les modules RFXCOM */
#endif
    TYPE_PAGE_ADMIN,                                               /* Page de gestion des commandes/requests d'administration */
  };

 struct PAGE_NOTEBOOK
  { gint type;
    GtkWidget *child;                                                   /* Le widget toplevel, placé dans la page du notebook */
    void *infos;                                                                  /* Pointeur sur une structure TYPE_INFO_xxx */
  };

 struct TYPE_INFO_SUPERVISION
  { guint timer_id;                                    /* Id du timer pour l'animation des motifs sur la trame de supervision */
    guint syn_id;                                                                        /* Id du synoptique en cours de visu */
    GtkWidget *Option_zoom;                                                               /* Choix du zoom sur la supervision */
    GtkWidget *Box_palette;                                                                   /* Widget de la boite a palette */
    struct TRAME *Trame;                                                                   /* La trame de fond de supervision */
  };

 enum                                       /* Numéro des colonnes dans les listes CAM (liste_camera et atelier_ajout_camera) */
  {  COL_CAM_ID,
     COL_CAM_NUM,
     COL_CAM_TYPE,
     COL_CAM_BIT,
     COL_CAM_OBJET,
     COL_CAM_LIBELLE,
     COL_CAM_LOCATION,
     NBR_COL_CAM
  };

 struct TYPE_INFO_SOURCE_DLS
  { GtkWidget *text;                                  /* Pour les plugins DLS, ici est placé le widget TextView correspondant */
    guint id;                                       /* Pour les plugins DLS, ici est stocké l'id du plugin en cours d'edition */
    gchar plugin_name[80];
    GtkWidget *F_mnemo;
    GtkWidget *Option_type;
    GtkWidget *Spin_num;
  };

 struct TYPE_INFO_HISTO_MSGS
  { GtkListStore *Liste_histo_msgs;                                      /* GtkTreeView pour la gestion des messages Watchdog */
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
  { struct CMD_TYPE_SYNOPTIQUE syn;                                                    /* Id du synoptique en cours d'edition */
                                                                    /* Interface de plus haut niveau: affichage du synoptique */
    struct TRAME *Trame_atelier;                                                             /* La trame de fond de l'atelier */
    struct
     { gint type;                                                                     /* Type de l'item selectionné principal */
       gint groupe;                                                                     /* Le groupe actuellement selectionné */
       union { struct TRAME_ITEM_MOTIF *trame_motif;                             /* Pointeur sur l'item selectionné principal */
               struct TRAME_ITEM_PASS  *trame_pass;                              /* Pointeur sur l'item selectionné principal */
               struct TRAME_ITEM_COMMENT *trame_comment;
               struct TRAME_ITEM_CADRAN *trame_cadran;
               struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
             };

       GList *items;                                                          /* Tous les items faisant parti de la selection */
     } Selection;
    GtkWidget *Option_zoom;                                                                    /* Choix du zoom sur l'atelier */
    GtkWidget *Check_grid;                                                                 /* La grille est-elle magnétique ? */
    GtkWidget *Spin_grid;                                                  /* Quel est l'ecartement de la grille magnetique ? */
    GtkWidget *Entry_posxy;                                                /* Affichage des coordonnées X de l'objet en cours */
    GtkWidget *Entry_libelle;                                                       /* Gestion du libelle de l'objet en cours */
    GtkAdjustment *Adj_angle;                                                        /* Angle de rotation de l'objet en cours */

    GtkWidget *F_ajout_palette;                                                              /* Le fenetre d'ajout de palette */
    GtkWidget *Liste_syn;                                            /* La liste des synoptiques pour la fenetre des palettes */
    GtkWidget *Liste_palette;                                                /* La liste des palettes associées au synoptique */
  };

/*--------------------------------------- Déclarations des prototypes de fonctions -------------------------------------------*/
 extern void Log( gchar *chaine );                                                                              /* Dans ihm.c */
 extern void Effacer_pages ( void );
 extern void Detruire_page ( struct PAGE_NOTEBOOK *page_a_virer );
 extern GtkWidget *Creer_boite_travail ( void );
 extern void Set_progress_plus( gint plus );
 extern void Set_progress_text( gchar *cadran, gint max );
 extern void Set_progress_pulse( void );
 extern void Raz_progress_pulse( void );
 extern struct PAGE_NOTEBOOK *Chercher_page_notebook ( guint type, guint id, gboolean affiche );
 extern gboolean Tester_page_notebook ( guint type );
 extern GtkWidget *Bobouton ( GdkPixmap *pix, GdkBitmap *bitmap, gchar *cadran );
 extern gint Nbr_page_type ( gint type );
 extern struct PAGE_NOTEBOOK *Page_actuelle ( void );
 
 extern void Connecter ( void );                                                                            /* Dans connect.c */
 extern gboolean Connecter_ssl ( void );
 extern gboolean Connecter_au_serveur ( void );
 extern void Envoyer_authentification ( void );
 extern void Deconnecter_sale ( void );
 extern void Deconnecter ( void );
 extern gboolean Envoi_serveur ( gint tag, gint ss_tag, gchar *buffer, gint taille );
 extern gboolean Changer_password ( void );

 extern void Ecouter_serveur ( void );                                                                    /* Dans protocole.c */
 extern void Gerer_protocole_gtk_message ( struct CONNEXION *connexion );
 extern void Gerer_protocole_icone ( struct CONNEXION *connexion );
 extern void Gerer_protocole_dls ( struct CONNEXION *connexion );
 extern void Gerer_protocole_utilisateur ( struct CONNEXION *connexion );
 extern void Gerer_protocole_groupe ( struct CONNEXION *connexion );
 extern void Gerer_protocole_message ( struct CONNEXION *connexion );
 extern void Gerer_protocole_synoptique ( struct CONNEXION *connexion );
 extern void Gerer_protocole_mnemonique ( struct CONNEXION *connexion );
 extern void Gerer_protocole_supervision ( struct CONNEXION *connexion );
 extern void Gerer_protocole_histo ( struct CONNEXION *connexion );
 extern void Gerer_protocole_atelier ( struct CONNEXION *connexion );
 extern void Gerer_protocole_fichier_connecte ( struct CONNEXION *connexion );
 extern void Gerer_protocole_connexion ( struct CONNEXION *connexion );
 extern void Gerer_protocole_camera ( struct CONNEXION *connexion );
 extern void Gerer_protocole_admin ( struct CONNEXION *connexion );
 extern gint Get_icone_version( void );

 extern gboolean Timer ( gpointer data );                                                                     /* Dans timer.c */

 extern void Proto_afficher_un_admin( struct CMD_TYPE_ADMIN *admin );                                         /* Dans admin.c */ 
 extern void Creer_page_admin( void );

 extern void Proto_afficher_un_groupe( struct CMD_TYPE_GROUPE *groupe );                               /* Dans liste_groupe.c */ 
 extern void Proto_cacher_un_groupe( struct CMD_TYPE_GROUPE *groupe );
 extern void Proto_rafraichir_un_groupe( struct CMD_TYPE_GROUPE *groupe );
 extern void Creer_page_groupe( void );

 extern void Menu_ajouter_editer_groupe ( struct CMD_TYPE_GROUPE *edit_groupe );                       /* Dans ajout_groupe.c */

 extern void Proto_afficher_un_utilisateur( struct CMD_TYPE_UTILISATEUR *util );                         /* Dans liste_util.c */ 
 extern void Proto_cacher_un_utilisateur( struct CMD_TYPE_UTILISATEUR *util );
 extern void Proto_rafraichir_un_utilisateur( struct CMD_TYPE_UTILISATEUR *util );
 extern void Creer_page_utilisateur( void );

 extern void Proto_afficher_un_groupe_existant ( struct CMD_TYPE_GROUPE *groupe );                            /* ajout_util.c */
 extern void Proto_fin_affichage_groupes_existants ( void );
 extern void Menu_ajouter_editer_utilisateur ( struct CMD_TYPE_UTILISATEUR *edit_util );

 extern void Proto_afficher_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *dls );                      /* Dans liste_plugin_dls.c */ 
 extern void Proto_cacher_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *dls );
 extern void Proto_rafraichir_un_plugin_dls( struct CMD_TYPE_PLUGIN_DLS *dls );
 extern void Creer_page_plugin_dls( void );

 extern void Menu_ajouter_editer_plugin_dls ( struct CMD_TYPE_PLUGIN_DLS *edit_dls );                   /* ajout_plugin_dls.c */
 extern void Proto_afficher_un_syn_for_plugin_dls ( struct CMD_TYPE_SYNOPTIQUE *syn );

 extern void Creer_page_source_dls( struct CMD_TYPE_PLUGIN_DLS *rezo_dls );                         /* Dans edit_source_dls.c */
 extern void Proto_append_source_dls( struct CMD_TYPE_SOURCE_DLS *dls, gchar *buffer );
 extern void Proto_afficher_mnemo_dls ( struct CMD_TYPE_MNEMO_BASE *mnemo );
 extern void Dls_set_compil_status ( gchar *chaine );

 extern void Proto_afficher_un_histo( struct CMD_TYPE_HISTO *histo );                                   /* Dans liste_histo.c */
 extern void Proto_cacher_un_histo( struct CMD_TYPE_HISTO *histo );
 extern void Proto_rafraichir_un_histo( struct CMD_TYPE_HISTO *histo );
 extern void Creer_page_histo( void );                               
 extern void Jouer_remote_mp3 ( gchar *file, gint id );
 
 extern void Proto_afficher_un_message( struct CMD_TYPE_MESSAGE *message );                           /* Dans liste_message.c */
 extern void Proto_cacher_un_message( struct CMD_TYPE_MESSAGE *message );
 extern void Proto_rafraichir_un_message( struct CMD_TYPE_MESSAGE *message );
 extern void Creer_page_message( void );
 extern gchar *Type_vers_string ( guint type );
 extern gchar *Type_sms_vers_string ( guint type );

 extern void Menu_ajouter_editer_message ( struct CMD_TYPE_MESSAGE *edit_msg );                       /* Dans ajout_message.c */
 extern void Proto_afficher_mnemo_voc_message ( struct CMD_TYPE_MNEMO_BASE *mnemo );
 extern void Proto_afficher_un_dls_for_message ( struct CMD_TYPE_PLUGIN_DLS *dls );

 extern void Proto_cacher_un_synoptique( struct CMD_TYPE_SYNOPTIQUE *synoptique );                  /* Dans liste_synoptique.c*/
 extern void Proto_afficher_un_synoptique( struct CMD_TYPE_SYNOPTIQUE *synoptique );
 extern void Proto_rafraichir_un_synoptique( struct CMD_TYPE_SYNOPTIQUE *synoptique );
 extern void Creer_page_synoptique( void );

 extern void Menu_ajouter_editer_synoptique ( struct CMD_TYPE_SYNOPTIQUE *edit_syn );                    /* ajout_synoptique.c*/
 extern void Proto_afficher_un_groupe_pour_propriete_synoptique ( struct CMD_TYPE_GROUPE *groupe );
 extern void Proto_fin_affichage_groupes_pour_synoptique ( void );
 extern void Proto_afficher_les_groupes_pour_synoptique ( GList *liste );
 extern void Proto_afficher_un_dls_for_mnemonique ( struct CMD_TYPE_PLUGIN_DLS *dls );

 extern void Proto_cacher_un_mnemonique( struct CMD_TYPE_MNEMO_BASE *mnemonique );                  /* Dans liste_mnemonique.c*/
 extern void Proto_afficher_un_mnemonique( struct CMD_TYPE_MNEMO_BASE *mnemonique );
 extern void Proto_rafraichir_un_mnemonique( struct CMD_TYPE_MNEMO_BASE *smnemonique );
 extern gchar *Type_bit_interne ( gint num );
 extern gchar *Type_bit_interne_court ( gint num );
 extern gint Type_bit_interne_int ( gchar *type );
 extern void Creer_page_mnemonique( void );

 extern void Menu_ajouter_editer_mnemonique ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );                  /* ajout_mnemonique.c*/
 extern void Proto_afficher_un_syn_for_mnemonique ( struct CMD_TYPE_SYNOPTIQUE *syn );

 extern void Proto_cacher_un_icone( struct CMD_TYPE_ICONE *icone );                                     /* Dans liste_icone.c */
 extern void Proto_afficher_un_icone( struct CMD_TYPE_ICONE *icone );
 extern void Proto_rafraichir_un_icone( struct CMD_TYPE_ICONE *icone );
 extern void Proto_cacher_une_classe( struct CMD_TYPE_CLASSE *classe );
 extern void Proto_afficher_une_classe( struct CMD_TYPE_CLASSE *classe );
 extern void Proto_rafraichir_une_classe( struct CMD_TYPE_CLASSE *classe );
 extern void Proto_envoyer_gif ( struct CMD_TYPE_ICONE *icone );
 extern void Creer_page_icone( void );

 extern void Menu_ajouter_editer_icone ( struct CMD_TYPE_ICONE *edit_icone );                                /* ajout_icone.c */
 extern void Menu_ajouter_editer_classe ( struct CMD_TYPE_CLASSE *edit_classe );

 extern void Menu_want_plugin_dls ( void );                                                                    /* Dans menu.c */
 extern void Menu_want_util ( void );
 extern void Menu_want_groupe ( void );
 extern void Menu_want_message ( void );
 extern void Menu_want_icone ( void );
 extern void Menu_want_synoptique ( void );
 extern void Menu_want_mnemonique ( void );
 extern void Menu_want_camera ( void );
 extern void Menu_want_histo_msgs ( void );
 extern void Menu_want_supervision( void );
 extern void Menu_want_page_admin ( void );


 extern void Proto_afficher_un_motif_atelier( struct CMD_TYPE_MOTIF *motif );                               /* Dans atelier.c */
 extern void Proto_cacher_un_motif_atelier( struct CMD_TYPE_MOTIF *motif );
 extern void Reduire_en_vignette ( struct CMD_TYPE_MOTIF *motif );
 extern void Creer_page_atelier( gint syn_id, gchar *libelle_syn );
 extern void Detruire_page_atelier ( struct PAGE_NOTEBOOK *page );
 extern struct TYPE_INFO_ATELIER *Rechercher_infos_atelier_par_id_syn ( gint syn_id );

                                                                                                 /* Dans atelier_clic_trame.c */
 extern void Clic_sur_fond ( struct TYPE_INFO_ATELIER *infos, GdkEvent *event, gpointer data );
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
 extern gint Nouveau_groupe ( void );
 
                                                                                                  /* Dans atelier_selection.c */
 extern void Tout_deselectionner ( struct TYPE_INFO_ATELIER *infos );
 extern gboolean Tester_selection ( struct TYPE_INFO_ATELIER *infos, gint groupe );
 extern void Selectionner ( struct TYPE_INFO_ATELIER *infos, gint groupe, gboolean deselect );
 extern void Deplacer_selection ( struct TYPE_INFO_ATELIER *infos, gint deltax, gint deltay );
 extern void Rotationner_selection ( struct TYPE_INFO_ATELIER *infos );
 extern void Effacer_selection ( void );
 extern void Dupliquer_selection ( void );
 extern void Deselectionner ( struct TYPE_INFO_ATELIER *infos, struct TRAME_ITEM *item );
 extern void Fusionner_selection ( void );
 extern void Detacher_selection ( void );
 extern void Mettre_echelle_selection_1_1 ( void );
 extern void Mettre_echelle_selection_1_Y ( void );
 extern void Mettre_echelle_selection_X_1 ( void );
                                                                                                   /* Dans atelier_agrandir.c */
 extern void Agrandir_bd ( GooCanvasItem *widget, GooCanvasItem *target,
                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Agrandir_bg ( GooCanvasItem *widget, GooCanvasItem *target,
                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Agrandir_hd ( GooCanvasItem *widget, GooCanvasItem *target,
                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Agrandir_hg ( GooCanvasItem *widget, GooCanvasItem *target,
                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );

 extern void Creer_fenetre_propriete_TOR ( struct TYPE_INFO_ATELIER *infos );                     /* Dans atelier_propriete.c */
 extern void Detruire_fenetre_propriete_TOR ();
 extern void Editer_propriete_TOR ( struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Changer_couleur_motif_directe( struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Proto_afficher_mnemo_atelier ( int tag, struct CMD_TYPE_MNEMO_BASE *mnemo );

                                                                                       /* Dans atelier_propriete_passerelle.c */
 extern void Editer_propriete_pass ( struct TRAME_ITEM_PASS *trame_pass );
 extern void Proto_afficher_mnemo_atelier_pass ( struct CMD_TYPE_MNEMO_BASE *mnemo );

 extern void Creer_fenetre_ajout_motif ( void );                                                /* Dans atelier_ajout_motif.c */
 extern void Detruire_fenetre_ajout_motif ( void );
 extern void Choisir_motif_a_ajouter ( void );
 extern void Proto_afficher_une_classe_atelier( struct CMD_TYPE_CLASSE *classe );
 extern void Proto_afficher_un_icone_atelier( struct CMD_TYPE_ICONE *icone );

 extern void Creer_fenetre_ajout_commentaire ( void );                                        /* Dans atelier_ajout_comment.c */
 extern void Proto_afficher_un_comment_atelier( struct CMD_TYPE_COMMENT *rezo_comment );
 extern void Proto_cacher_un_comment_atelier( struct CMD_TYPE_COMMENT *comment );

 extern void Creer_fenetre_ajout_passerelle ( void );                                      /* Dans atelier_ajout_passerelle.c */
 extern void Proto_afficher_un_syn_for_passerelle_atelier( struct CMD_TYPE_SYNOPTIQUE *syn );
 extern void Proto_afficher_une_passerelle_atelier( struct CMD_TYPE_PASSERELLE *rezo_pass );
 extern void Proto_cacher_une_passerelle_atelier( struct CMD_TYPE_PASSERELLE *pass );

                                                                                               /* Dans atelier_ajout_cadran.c */
 extern void Menu_ajouter_editer_cadran ( struct TRAME_ITEM_CADRAN *trame_cadran );
 extern void Proto_afficher_un_cadran_atelier( struct CMD_TYPE_CADRAN *rezo_cadran );
 extern void Proto_cacher_un_cadran_atelier( struct CMD_TYPE_CADRAN *cadran );
 extern void Proto_afficher_mnemo_cadran_atelier ( struct CMD_TYPE_MNEMO_BASE *mnemo );

                                                                                              /* Dans atelier_ajout_palette.c */
 extern void Creer_fenetre_ajout_palette ( struct TYPE_INFO_ATELIER *infos );
 extern void Proto_afficher_une_palette_atelier( struct CMD_TYPE_PALETTE *rezo_palette );
 extern void Proto_cacher_une_palette_atelier( struct CMD_TYPE_PALETTE *palette );
 extern void Proto_afficher_un_syn_for_palette_atelier( struct CMD_TYPE_SYNOPTIQUE *synoptique );

                                                                                           /* Dans atelier_ajout_camera_sup.c */
 extern struct TRAME_ITEM_CAMERA_SUP *Id_vers_trame_camera_sup ( struct TYPE_INFO_ATELIER *infos, gint id );
 extern void Menu_ajouter_camera_sup ( void );
 extern void Proto_afficher_un_camera_for_atelier( struct CMD_TYPE_CAMERA *camera );
 extern void Proto_afficher_un_camera_sup_atelier( struct CMD_TYPE_CAMERA_SUP *rezo_camera_sup );
 extern void Proto_cacher_un_camera_sup_atelier( struct CMD_TYPE_CAMERA_SUP *camera_sup );

 extern void Creer_page_liste_histo_msgs( void );                                                  /* Dans liste_histo_msgs.c */
 extern void Proto_effacer_liste_histo_msgs( gint page_id );
 extern void Proto_afficher_un_histo_msgs( struct CMD_RESPONSE_HISTO_MSGS *response );
 
                                                                                                        /* Dans supervision.c */
 extern void Creer_page_supervision( gchar *libelle, guint syn_id );
 extern void Detruire_page_supervision( struct PAGE_NOTEBOOK *page );
 extern void Proto_afficher_un_motif_supervision( struct CMD_TYPE_MOTIF *rezo_motif );
 extern void Proto_changer_etat_motif( struct CMD_ETAT_BIT_CTRL *etat_motif );
 extern struct TYPE_INFO_SUPERVISION *Rechercher_infos_supervision_par_id_syn ( gint syn_id );

                                                                                                   /* Dans supervision_clic.c */
 extern void Clic_sur_motif_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Clic_sur_camera_sup_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                               GdkEvent *event, struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup );
 extern void Clic_sur_cadran_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                            GdkEvent *event, struct TRAME_ITEM_CADRAN *trame_cadran );

                                                                                                /* Dans supervision_comment.c */
 extern void Proto_afficher_un_comment_supervision( struct CMD_TYPE_COMMENT *rezo_comment );
 
                                                                                             /* Dans supervision_passerelle.c */
 extern void Proto_afficher_une_passerelle_supervision( struct CMD_TYPE_PASSERELLE *rezo_pass );
 extern void Changer_vue_directe ( guint num_syn );

                                                                                                /* Dans supervision_palette.c */
 extern void Proto_afficher_une_palette_supervision( struct CMD_TYPE_PALETTE *rezo_palette );

                                                                                                 /* Dans supervision_cadran.c */
 extern void Proto_afficher_un_cadran_supervision( struct CMD_TYPE_CADRAN *rezo_cadran );
 extern void Proto_changer_etat_cadran( struct CMD_ETAT_BIT_CADRAN *etat_cadran );

                                                                                                 /* Dans supervision_camera.c */
 extern void Proto_afficher_un_camera_sup_supervision( struct CMD_TYPE_CAMERA_SUP *rezo_camera_sup );

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

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/

