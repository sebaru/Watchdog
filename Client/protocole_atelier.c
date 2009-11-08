/**********************************************************************************************************/
/* Client/protocole_atelier.c    Gestion du protocole_atelier pour la connexion au serveur Watchdog       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_atelier.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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
 
 
 #include <glib.h>
 #include "Erreur.h"
 #include "Reseaux.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_atelier ( struct CONNEXION *connexion )
  { static GList *Arrivee_groupe = NULL;
    static GList *Arrivee_synoptique_for_atelier = NULL;
    static GList *Arrivee_classe = NULL;
    static GList *Arrivee_synoptique_for_atelier_palette = NULL;
    static GList *Arrivee_motif = NULL;
    static GList *Arrivee_icone = NULL;
    static GList *Arrivee_pass = NULL;
    static GList *Arrivee_capteur = NULL;
    static GList *Arrivee_comment = NULL;
    static GList *Arrivee_palette = NULL;
    static GList *Arrivee_camera_sup = NULL;
    static GList *Arrivee_groupe_propriete_syn = NULL;
    static GList *Arrivee_camera_for_atelier = NULL;
    static int save_id;
           
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_PROPRIETE_SYNOPTIQUE:
             { struct CMD_SHOW_GROUPE *groupe;
               Set_progress_plusun();

               groupe = (struct CMD_SHOW_GROUPE *)g_malloc0( sizeof( struct CMD_SHOW_GROUPE ) );
               if (!groupe) return; 
               memcpy( groupe, connexion->donnees, sizeof(struct CMD_SHOW_GROUPE ) );
               Arrivee_groupe_propriete_syn = g_list_append( Arrivee_groupe_propriete_syn, groupe );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_PROPRIETE_SYNOPTIQUE_FIN:
             { g_list_foreach( Arrivee_groupe, (GFunc)Proto_afficher_un_groupe_pour_propriete_synoptique, NULL );
               g_list_foreach( Arrivee_groupe, (GFunc)g_free, NULL );
               g_list_free( Arrivee_groupe_propriete_syn );
               Arrivee_groupe_propriete_syn = NULL;
             }
/**********************************************************************************************************/
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC:
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC2:
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_CTRL:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_afficher_mnemo_atelier( Reseau_ss_tag ( connexion ), mnemo );
             }
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_EA:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_afficher_mnemo_capteur_atelier( mnemo );
             }
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_PASS:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_afficher_mnemo_atelier_pass( mnemo );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_MOTIF_OK:
             { struct CMD_SHOW_MOTIF *motif;
               motif = (struct CMD_SHOW_MOTIF *)connexion->donnees;
               Proto_afficher_un_motif_atelier( motif );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_MOTIF_OK:
             { struct CMD_ID_MOTIF *motif;
               motif = (struct CMD_ID_MOTIF *)connexion->donnees;
               Proto_cacher_un_motif_atelier( motif );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_COMMENT_OK:
             { struct CMD_SHOW_COMMENT *comment;
               comment = (struct CMD_SHOW_COMMENT *)connexion->donnees;
               Proto_afficher_un_comment_atelier( comment );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_COMMENT_OK:
             { struct CMD_ID_COMMENT *comment;
               comment = (struct CMD_ID_COMMENT *)connexion->donnees;
               Proto_cacher_un_comment_atelier( comment );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_PASS_OK:
             { struct CMD_SHOW_PASSERELLE *pass;
               pass = (struct CMD_SHOW_PASSERELLE *)connexion->donnees;
               Proto_afficher_une_passerelle_atelier( pass );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_PASS_OK:
             { struct CMD_ID_PASSERELLE *pass;
               pass = (struct CMD_ID_PASSERELLE *)connexion->donnees;
               Proto_cacher_une_passerelle_atelier( pass );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_PALETTE_OK:
             { struct CMD_SHOW_PALETTE *palette;
               palette = (struct CMD_SHOW_PALETTE *)connexion->donnees;
               Proto_afficher_une_palette_atelier( palette );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_PALETTE_OK:
             { struct CMD_ID_PALETTE *palette;
               palette = (struct CMD_ID_PALETTE *)connexion->donnees;
               Proto_cacher_une_palette_atelier( palette );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_CAPTEUR_OK:
             { struct CMD_SHOW_CAPTEUR *capteur;
               capteur = (struct CMD_SHOW_CAPTEUR *)connexion->donnees;
               Proto_afficher_un_capteur_atelier( capteur );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_CAPTEUR_OK:
             { struct CMD_ID_CAPTEUR *capteur;
               capteur = (struct CMD_ID_CAPTEUR *)connexion->donnees;
               Proto_cacher_un_capteur_atelier( capteur );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER:
             { struct CMD_SHOW_SYNOPTIQUE *syn;
               Set_progress_plusun();

               syn = (struct CMD_SHOW_SYNOPTIQUE *)g_malloc0( sizeof( struct CMD_SHOW_SYNOPTIQUE ) );
               if (!syn) return; 
               memcpy( syn, connexion->donnees, sizeof(struct CMD_SHOW_SYNOPTIQUE ) );
               Arrivee_synoptique_for_atelier = g_list_append( Arrivee_synoptique_for_atelier, syn );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_FIN:
             { g_list_foreach( Arrivee_synoptique_for_atelier,
                               (GFunc)Proto_afficher_un_syn_for_passerelle_atelier, NULL );
               g_list_foreach( Arrivee_synoptique_for_atelier, (GFunc)g_free, NULL );
               g_list_free( Arrivee_synoptique_for_atelier );
               Arrivee_synoptique_for_atelier = NULL;
             }
            break;
/******************************************** Reception des cameras ***************************************/
       case SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FOR_ATELIER:
             { struct CMD_TYPE_CAMERA *cam;
               Set_progress_plusun();

               cam = (struct CMD_TYPE_CAMERA *)g_malloc0( sizeof( struct CMD_TYPE_CAMERA ) );
               if (!cam) return; 
               memcpy( cam, connexion->donnees, sizeof(struct CMD_TYPE_CAMERA ) );
               Arrivee_camera_for_atelier = g_list_append( Arrivee_camera_for_atelier, cam );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FOR_ATELIER_FIN:
             { g_list_foreach( Arrivee_camera_for_atelier,
                               (GFunc)Proto_afficher_un_camera_for_atelier, NULL );
               g_list_foreach( Arrivee_camera_for_atelier, (GFunc)g_free, NULL );
               g_list_free( Arrivee_camera_for_atelier );
               Arrivee_camera_for_atelier = NULL;
             }
            break;
/*********************************** Reception des cameras de supervision *********************************/
       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP:
             { struct CMD_TYPE_CAMERA_SUP *camera_sup;
               Set_progress_plusun();

               camera_sup = (struct CMD_TYPE_CAMERA_SUP *)g_malloc0( sizeof( struct CMD_TYPE_CAMERA_SUP ) );
               if (!camera_sup) return; 
               memcpy( camera_sup, connexion->donnees, sizeof(struct CMD_TYPE_CAMERA_SUP ) );
               Arrivee_camera_sup = g_list_append( Arrivee_camera_sup, camera_sup );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP_FIN:
             { g_list_foreach( Arrivee_camera_sup, (GFunc)Proto_afficher_un_camera_sup_atelier, NULL );
               g_list_foreach( Arrivee_camera_sup, (GFunc)g_free, NULL );
               g_list_free( Arrivee_camera_sup );
               Arrivee_camera_sup = NULL;
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_CAMERA_SUP_OK:
             { struct CMD_TYPE_CAMERA_SUP *cam_sup;
               cam_sup = (struct CMD_TYPE_CAMERA_SUP *)connexion->donnees;
               Proto_afficher_un_camera_sup_atelier( cam_sup );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_CAMERA_SUP_OK:
             { struct CMD_TYPE_CAMERA_SUP *cam_sup;
               cam_sup = (struct CMD_TYPE_CAMERA_SUP *)connexion->donnees;
               Proto_cacher_un_camera_sup_atelier( cam_sup );
             }
            break;
/******************************************** Reception des palettes **************************************/
       case SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_PALETTE:
             { struct CMD_SHOW_SYNOPTIQUE *syn;
               Set_progress_plusun();

               syn = (struct CMD_SHOW_SYNOPTIQUE *)g_malloc0( sizeof( struct CMD_SHOW_SYNOPTIQUE ) );
               if (!syn) return; 
               memcpy( syn, connexion->donnees, sizeof(struct CMD_SHOW_SYNOPTIQUE ) );
               Arrivee_synoptique_for_atelier_palette = g_list_append( Arrivee_synoptique_for_atelier_palette, syn );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_PALETTE_FIN:
             { g_list_foreach( Arrivee_synoptique_for_atelier_palette,
                               (GFunc)Proto_afficher_un_syn_for_palette_atelier, NULL );
               g_list_foreach( Arrivee_synoptique_for_atelier_palette, (GFunc)g_free, NULL );
               g_list_free( Arrivee_synoptique_for_atelier_palette );
               Arrivee_synoptique_for_atelier_palette = NULL;
             }
            break;

       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF:
             { struct CMD_SHOW_MOTIF *motif;
               Set_progress_plusun();

               motif = (struct CMD_SHOW_MOTIF *)g_malloc0( sizeof( struct CMD_SHOW_MOTIF ) );
               if (!motif) return; 
               memcpy( motif, connexion->donnees, sizeof(struct CMD_SHOW_MOTIF ) );
               Arrivee_motif = g_list_append( Arrivee_motif, motif );
               save_id = motif->syn_id;
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF_FIN:
             { g_list_foreach( Arrivee_motif, (GFunc)Proto_afficher_un_motif_atelier, NULL );
               g_list_foreach( Arrivee_motif, (GFunc)g_free, NULL );
               g_list_free( Arrivee_motif );
               Arrivee_motif = NULL;
               Chercher_page_notebook( TYPE_PAGE_ATELIER, save_id, TRUE );
             }
            break;

       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT:
             { struct CMD_SHOW_COMMENT *comment;
               Set_progress_plusun();

               comment = (struct CMD_SHOW_COMMENT *)g_malloc0( sizeof( struct CMD_SHOW_COMMENT ) );
               if (!comment) return; 
               memcpy( comment, connexion->donnees, sizeof(struct CMD_SHOW_COMMENT ) );
               Arrivee_comment = g_list_append( Arrivee_comment, comment );
               save_id = comment->syn_id;
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT_FIN:
             { g_list_foreach( Arrivee_comment, (GFunc)Proto_afficher_un_comment_atelier, NULL );
               g_list_foreach( Arrivee_comment, (GFunc)g_free, NULL );
               g_list_free( Arrivee_comment );
               Arrivee_comment = NULL;
               Chercher_page_notebook( TYPE_PAGE_ATELIER, save_id, TRUE );
             }
            break;

       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS:
             { struct CMD_SHOW_PASSERELLE *pass;
               Set_progress_plusun();

               pass = (struct CMD_SHOW_PASSERELLE *)g_malloc0( sizeof( struct CMD_SHOW_PASSERELLE ) );
               if (!pass) { printf("Pas assez de mémoire\n"); return;  }
               memcpy( pass, connexion->donnees, sizeof(struct CMD_SHOW_PASSERELLE ) );
               Arrivee_pass = g_list_append( Arrivee_pass, pass );
               save_id = pass->syn_id;
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS_FIN:
             { g_list_foreach( Arrivee_pass, (GFunc)Proto_afficher_une_passerelle_atelier, NULL );
               g_list_foreach( Arrivee_pass, (GFunc)g_free, NULL );
               g_list_free( Arrivee_pass );
               Arrivee_pass = NULL;
               Chercher_page_notebook( TYPE_PAGE_ATELIER, save_id, TRUE );
             }
            break;

       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR:
             { struct CMD_SHOW_CAPTEUR *capteur;
               Set_progress_plusun();

               capteur = (struct CMD_SHOW_CAPTEUR *)g_malloc0( sizeof( struct CMD_SHOW_CAPTEUR ) );
               if (!capteur) { printf("Pas assez de mémoire\n"); return;  }
               memcpy( capteur, connexion->donnees, sizeof(struct CMD_SHOW_CAPTEUR ) );
               Arrivee_capteur = g_list_append( Arrivee_capteur, capteur );
               save_id = capteur->syn_id;
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR_FIN:
             { g_list_foreach( Arrivee_capteur, (GFunc)Proto_afficher_un_capteur_atelier, NULL );
               g_list_foreach( Arrivee_capteur, (GFunc)g_free, NULL );
               g_list_free( Arrivee_capteur );
               Arrivee_capteur = NULL;
               Chercher_page_notebook( TYPE_PAGE_ATELIER, save_id, TRUE );
             }
            break;

       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE:
             { struct CMD_SHOW_PALETTE *palette;
               Set_progress_plusun();
               palette = (struct CMD_SHOW_PALETTE *)g_malloc0( sizeof( struct CMD_SHOW_PALETTE ) );
               if (!palette) return; 
               memcpy( palette, connexion->donnees, sizeof(struct CMD_SHOW_PALETTE ) );
               Arrivee_palette = g_list_append( Arrivee_palette, palette );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE_FIN:
             { g_list_foreach( Arrivee_palette, (GFunc)Proto_afficher_une_palette_atelier, NULL );
               g_list_foreach( Arrivee_palette, (GFunc)g_free, NULL );
               g_list_free( Arrivee_palette );
               Arrivee_palette = NULL;
             }
            break;

       case SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FOR_ATELIER:
             { struct CMD_TYPE_CLASSE *classe;
               Set_progress_plusun();
               classe = (struct CMD_TYPE_CLASSE *)g_malloc0( sizeof( struct CMD_TYPE_CLASSE ) );
               if (!classe) return; 
               memcpy( classe, connexion->donnees, sizeof(struct CMD_TYPE_CLASSE ) );
               Arrivee_classe = g_list_append( Arrivee_classe, classe );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FOR_ATELIER_FIN:
             { g_list_foreach( Arrivee_classe, (GFunc)Proto_afficher_une_classe_atelier, NULL );
               g_list_foreach( Arrivee_classe, (GFunc)g_free, NULL );
               g_list_free( Arrivee_classe );
               Arrivee_classe = NULL;
             }
            break;

       case SSTAG_SERVEUR_ADDPROGRESS_ICONE_FOR_ATELIER:
             { struct CMD_TYPE_ICONE *ico;

               ico = (struct CMD_TYPE_ICONE *)g_malloc0( sizeof( struct CMD_TYPE_ICONE ) );
               if (!ico) return; 
               memcpy( ico, connexion->donnees, sizeof(struct CMD_TYPE_ICONE ) );
               Arrivee_icone = g_list_append( Arrivee_icone, ico );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ICONE_FOR_ATELIER_FIN:
             { g_list_foreach( Arrivee_icone, (GFunc)Proto_afficher_un_icone_atelier, NULL );
               g_list_foreach( Arrivee_icone, (GFunc)g_free, NULL );
               g_list_free( Arrivee_icone );
               Arrivee_icone = NULL;
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
