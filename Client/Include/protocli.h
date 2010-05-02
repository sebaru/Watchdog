/**********************************************************************************************************/
/* Client/protocli.h     Définition des prototypes de fonctions                                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 29 mar 2009 09:57:11 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocli.h.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2007 - Sébastien Lefevre
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
 #include "protocommunclient.h"

 #define TEMPS_MAX_PULSE   10                        /* 10 secondes de battements maximum pour le serveur */
 #define NBR_BIT_DLS       10000
 #define SIGNATURE_PRINT   "Watchdog 2_1.ABLS 2008"
 #define PRINT_FONT_SIZE   10.0

 enum
  { TYPE_PAGE_PLUGIN_DLS,                                                     /* Listes des plugins D.L.S */
    TYPE_PAGE_HISTO,                                           /* Page de garde: messages au fil de l'eau */
    TYPE_PAGE_GROUPE,                                                  /* Edition des groupes de Watchdog */
    TYPE_PAGE_UTIL,                                                    /* Liste des utilisateurs Watchdog */
    TYPE_PAGE_MESSAGE,                                                   /* Edition des messages Watchdog */
    TYPE_PAGE_ENTREEANA,                                             /* Il s'agit d'un atelier synoptique */
    TYPE_PAGE_SYNOPTIQUE,                                 /* Edition des noms/mnémoniques des synoptiques */
    TYPE_PAGE_MNEMONIQUE,                                        /* Page de visualisation des mnemoniques */
    TYPE_PAGE_ICONE,                                /* Ajout/retrait/modif des icones et classes d'icones */
    TYPE_PAGE_SOURCE_DLS,                                                   /* Edition d'une source D.L.S */
    TYPE_PAGE_HISTO_HARD,                                                     /* Parcours de l'historique */
    TYPE_PAGE_ATELIER,                                               /* Il s'agit d'un atelier synoptique */
    TYPE_PAGE_SUPERVISION,                                            /* Supervision graphique synoptique */
    TYPE_PAGE_COURBE,                                              /* Affichage des courbes en temps reel */
    TYPE_PAGE_HISTO_COURBE,
    TYPE_PAGE_SCENARIO,                                                           /* Gestion des scenario */
    TYPE_PAGE_CAMERA,                                                               /* Gestion des camera */
    TYPE_PAGE_SUPERVISION_CAMERA,                                          /* Page affichant une camera ! */
  };

 struct PAGE_NOTEBOOK
  { gint type;
    GtkWidget *child;                               /* Le widget toplevel, placé dans la page du notebook */
    void *infos;                                              /* Pointeur sur une structure TYPE_INFO_xxx */
  };

 struct TYPE_INFO_SUPERVISION
  { guint timer_id;                /* Id du timer pour l'animation des motifs sur la trame de supervision */
    guint syn_id;                                                    /* Id du synoptique en cours de visu */
    GtkWidget *Option_zoom;                                           /* Choix du zoom sur la supervision */
    GtkWidget *Box_palette;                                               /* Widget de la boite a palette */
    struct TRAME *Trame;                                               /* La trame de fond de supervision */
  };

 struct COURBE
  { guint actif;
    GtkDataboxGraph *index;                                           /* Index retourné par le gtkdatabox */
    GdkColor couleur;                                                             /* Couleur de la courbe */
    guint nbr_points;                          /* Pour l'ajout initial des données dans la zone graphique */
    gfloat Y[TAILLEBUF_HISTO_EANA];                                         /* Coordonnées Y de la courbe */
    time_t X_date[TAILLEBUF_HISTO_EANA];                                  /* Coordonnées date des courbes */
    guint type;
    union
     { struct CMD_TYPE_ENTREEANA eana;                                            /* Libelle de la courbe */
       struct CMD_TYPE_MNEMONIQUE mnemo;
     };
  };

 #define NBR_MAX_COURBES   6                             /* Nombre maximum de courbes affichées a l'écran */
 #define MAX_RESOLUTION    4096
 #define ENTREAXE_Y_TOR    400
 #define HAUTEUR_Y_TOR     350.0

 struct TYPE_INFO_COURBE
  { guint slot_id;                                                   /* Numero du slot en cours d'edition */
    guint posx_select;
    gfloat x_select[2], y_select[2];                                            /* Coord affichage select */
    GtkWidget *Databox;
    GtkWidget *Entry[NBR_MAX_COURBES];                           /* Zone de capteur pour description Eana */
    GtkWidget *Radio[NBR_MAX_COURBES];                           /* Zone de capteur pour description Eana */
    struct COURBE Courbes[NBR_MAX_COURBES];
    GtkDataboxGraph *index_grille;                       /* Index de la grille retourné par le gtkdatabox */
    GtkDataboxGraph *index_select;                       /* Index de la grille retourné par le gtkdatabox */
    GtkWidget *Date_debut;
    GtkWidget *Date_fin;
    GtkWidget *Entry_date_select;                                 /* Affichage de la date actuelle select */
    gfloat X[TAILLEBUF_HISTO_EANA];        /* Coordonnées X des courbes. Toutes les courbes ont le meme X */
    gint echelle_active;                                     /* N° de la courbe dont l'echelle est active */
  };

 struct TYPE_INFO_HISTO_HARD
  { GtkListStore *Liste_histo_hard;                  /* GtkTreeView pour la gestion des messages Watchdog */
    GtkWidget *F_histo_hard;
    GtkWidget *Check_ID;
    GtkWidget *Spin_ID;
    GtkWidget *Check_type;
    GtkWidget *Option_type;
    GtkWidget *Check_nom_ack;
    GtkWidget *Entry_nom_ack;
    GtkWidget *Check_libelle;
    GtkWidget *Entry_libelle; 
    GtkWidget *Check_objet;
    GtkWidget *Entry_objet;
    GtkWidget *Check_debut;
    GtkWidget *Date_debut;
    GtkWidget *Check_fin;
    GtkWidget *Date_fin;
    gint page_id;
  };
  
 #define CAMERA_DELAI_START 4
 struct TYPE_INFO_CAMERA
  { struct CMD_TYPE_CAMERA_SUP camera;                                          /* Structure de la camera */
    GtkLibVLCInstance* instance;
    GtkWidget *vlc;
  };

/*---------------------------- Déclarations des prototypes de fonctions ----------------------------------*/
 extern void Log( gchar *chaine );                                                          /* Dans ihm.c */
 extern void Effacer_pages ( void );
 extern void Detruire_page ( struct PAGE_NOTEBOOK *page_a_virer );
 extern GtkWidget *Creer_boite_travail ( void );
 extern void Set_progress_plusun( void );
 extern void Set_progress_plus( gint plus );
 extern void Set_progress_text( gchar *capteur, gint max );
 extern void Set_progress_pulse( void );
 extern void Raz_progress( void );
 extern struct PAGE_NOTEBOOK *Chercher_page_notebook ( guint type, guint id, gboolean affiche );
 extern gboolean Tester_page_notebook ( guint type );
 extern GtkWidget *Bobouton ( GdkPixmap *pix, GdkBitmap *bitmap, gchar *capteur );
 extern gint Nbr_page_type ( gint type );
 extern struct PAGE_NOTEBOOK *Page_actuelle ( void );
 
 extern void Connecter ( void );                                                        /* Dans connect.c */
 extern void Deconnecter_sale ( void );
 extern void Deconnecter ( void );
 extern gboolean Envoi_serveur ( gint tag, gint ss_tag, gchar *buffer, gint taille );
 extern gboolean Changer_password ( void );

 extern void Envoyer_identification ( void );                                             /* Dans ident.c */

 extern void Ecouter_serveur ( void );                                                /* Dans protocole.c */
 extern void Gerer_protocole_gtk_message ( struct CONNEXION *connexion );
 extern void Gerer_protocole_utilisateur ( struct CONNEXION *connexion );
 extern void Gerer_protocole_groupe ( struct CONNEXION *connexion );
 extern void Gerer_protocole_supervision ( struct CONNEXION *connexion );
 extern void Gerer_protocole_histo ( struct CONNEXION *connexion );
 extern void Gerer_protocole_courbe ( struct CONNEXION *connexion );
 extern void Gerer_protocole_histo_connecte ( struct CONNEXION *connexion );
 extern void Gerer_protocole_fichier_connecte ( struct CONNEXION *connexion );
 extern void Gerer_protocole_connexion ( struct CONNEXION *connexion );
 extern void Gerer_protocole_histo_courbe ( struct CONNEXION *connexion );


 extern gboolean Connecter_ssl ( void );                                                    /* Dans ssl.c */
 extern SSL_CTX *Init_ssl ( void );

 extern gboolean Timer ( gpointer data );                                                 /* Dans timer.c */

 extern void Proto_afficher_un_groupe( struct CMD_TYPE_GROUPE *groupe );           /* Dans liste_groupe.c */ 
 extern void Proto_cacher_un_groupe( struct CMD_TYPE_GROUPE *groupe );
 extern void Proto_rafraichir_un_groupe( struct CMD_TYPE_GROUPE *groupe );
 extern void Creer_page_groupe( void );

 extern void Menu_ajouter_editer_groupe ( struct CMD_TYPE_GROUPE *edit_groupe );   /* Dans ajout_groupe.c */

 extern void Proto_afficher_un_utilisateur( struct CMD_TYPE_UTILISATEUR *util );     /* Dans liste_util.c */ 
 extern void Proto_cacher_un_utilisateur( struct CMD_TYPE_UTILISATEUR *util );
 extern void Proto_rafraichir_un_utilisateur( struct CMD_TYPE_UTILISATEUR *util );
 extern void Creer_page_utilisateur( void );

 extern void Proto_afficher_un_groupe_existant ( struct CMD_TYPE_GROUPE *groupe );        /* ajout_util.c */
 extern void Proto_fin_affichage_groupes_existants ( void );
 extern void Menu_ajouter_editer_utilisateur ( struct CMD_TYPE_UTILISATEUR *edit_util );

 extern void Proto_afficher_un_histo( struct CMD_TYPE_HISTO *histo );               /* Dans liste_histo.c */
 extern void Proto_cacher_un_histo( struct CMD_TYPE_HISTO *histo );
 extern void Proto_rafraichir_un_histo( struct CMD_TYPE_HISTO *histo );
 extern void Creer_page_histo( void );                               
 extern gchar *Type_vers_string ( guint type );

 extern void Menu_want_util ( void );
 extern void Menu_want_groupe ( void );
 extern void Menu_want_histo_hard ( void );
 extern void Menu_want_supervision( void );
 extern void Menu_want_courbe ( void );
 extern void Menu_want_histo_courbe ( void );

 extern void Creer_page_liste_histo_hard( void );                              /* Dans liste_histo_hard.c */
 extern void Proto_effacer_liste_histo_hard( gint page_id );
 extern void Proto_afficher_un_histo_hard( struct CMD_TYPE_HISTO_HARD *histo );
 
                                                                                    /* Dans supervision.c */
 extern void Creer_page_supervision( gchar *libelle, guint syn_id );
 extern void Detruire_page_supervision( struct PAGE_NOTEBOOK *page );
 extern void Proto_afficher_un_motif_supervision( struct CMD_TYPE_MOTIF *rezo_motif );
 extern void Proto_changer_etat_motif( struct CMD_ETAT_BIT_CTRL *etat_motif );
 extern struct TYPE_INFO_SUPERVISION *Rechercher_infos_supervision_par_id_syn ( gint syn_id );

                                                                               /* Dans supervision_clic.c */
 extern  void Clic_sur_motif_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                           GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif );
 extern void Clic_sur_camera_sup_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                               GdkEvent *event, struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup );

                                                                            /* Dans supervision_comment.c */
 extern void Proto_afficher_un_comment_supervision( struct CMD_TYPE_COMMENT *rezo_comment );
 
                                                                         /* Dans supervision_passerelle.c */
 extern void Proto_afficher_une_passerelle_supervision( struct CMD_TYPE_PASSERELLE *rezo_pass );
 extern void Changer_vue_directe ( guint num_syn );

                                                                            /* Dans supervision_palette.c */
 extern void Proto_afficher_une_palette_supervision( struct CMD_TYPE_PALETTE *rezo_palette );

                                                                            /* Dans supervision_capteur.c */
 extern void Proto_afficher_un_capteur_supervision( struct CMD_TYPE_CAPTEUR *rezo_capteur );
 extern void Proto_changer_etat_capteur( struct CMD_ETAT_BIT_CAPTEUR *etat_capteur );

                                                                             /* Dans supervision_camera.c */
 extern void Proto_afficher_un_camera_sup_supervision( struct CMD_TYPE_CAMERA_SUP *rezo_camera_sup );

 extern GtkPrintOperation *New_print_job ( gchar *nom );                                  /* Dans print.c */
 extern void Begin_print (GtkPrintOperation *operation,
                          GtkPrintContext   *context,
                          gpointer           user_data);
                                                                                         /* Dans courbe.c */
 extern void Proto_afficher_une_source_EA_for_courbe( struct CMD_TYPE_ENTREEANA *entreeANA );
 extern void Proto_afficher_une_source_for_courbe( struct CMD_TYPE_MNEMONIQUE *mnemo );
 extern void Creer_page_courbe ( gchar *libelle );
 extern void Detruire_page_courbe( struct PAGE_NOTEBOOK *page );
 extern gboolean CB_deplacement_databox ( struct TYPE_INFO_COURBE *infos, GdkEvent *event, gpointer data );
 extern gboolean CB_sortir_databox ( struct TYPE_INFO_COURBE *infos, GdkEvent *event, gpointer data );
 extern void Proto_ajouter_courbe( struct CMD_TYPE_COURBE *courbe );
 extern void Proto_append_courbe( struct CMD_APPEND_COURBE *append_courbe );
                                                                                   /* Dans histo_courbe.c */
 extern void Proto_afficher_une_source_EA_for_histo_courbe( struct CMD_TYPE_ENTREEANA *entreeANA );
 extern void Proto_afficher_une_source_for_histo_courbe( struct CMD_TYPE_MNEMONIQUE *mnemo );
 extern void Creer_page_histo_courbe ( gchar *libelle );
 extern void Detruire_page_histo_courbe( struct PAGE_NOTEBOOK *page );
 extern gint Append_courbe ( struct COURBE *courbe, struct CMD_APPEND_COURBE *append_courbe );
 extern void Menu_changer_echelle0 ( struct TYPE_INFO_COURBE *infos );
 extern void Menu_changer_echelle1 ( struct TYPE_INFO_COURBE *infos );
 extern void Menu_changer_echelle2 ( struct TYPE_INFO_COURBE *infos );
 extern void Menu_changer_echelle3 ( struct TYPE_INFO_COURBE *infos );
 extern void Menu_changer_echelle4 ( struct TYPE_INFO_COURBE *infos );
 extern void Menu_changer_echelle5 ( struct TYPE_INFO_COURBE *infos );
 extern void Proto_ajouter_histo_courbe( struct CMD_TYPE_COURBE *courbe );
 extern void Proto_append_histo_courbe( struct CMD_APPEND_COURBE *append_courbe );

                                                                           /* Dans supervision_scenario.c */
 extern void Proto_supervision_afficher_un_scenario( struct CMD_TYPE_SCENARIO *scenario );
 extern void Proto_supervision_cacher_un_scenario( struct CMD_TYPE_SCENARIO *scenario );
 extern void Proto_supervision_rafraichir_un_scenario( struct CMD_TYPE_SCENARIO *scenario );
 extern void Menu_supervision_ajouter_editer_scenario ( struct CMD_TYPE_SCENARIO *edit_sce );
 extern void Creer_fenetre_scenario( struct MOTIF *motif );

                                                                        /* Dans supervision_page_camera.c */
 extern void Creer_page_supervision_camera ( struct CMD_TYPE_CAMERA_SUP *camera );
 extern void Detruire_page_supervision_camera( struct PAGE_NOTEBOOK *page );

 #endif
/*--------------------------------------------------------------------------------------------------------*/

