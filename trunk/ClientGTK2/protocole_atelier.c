/**********************************************************************************************************/
/* Client/protocole_atelier.c    Gestion du protocole_atelier pour la connexion au serveur Watchdog       */
/* Projet WatchDog version 3.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_atelier.c
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
  { static GList *Arrivee_synoptique_for_atelier = NULL;
    static GList *Arrivee_synoptique_for_atelier_palette = NULL;
    static GList *Arrivee_motif = NULL;
    static GList *Arrivee_pass = NULL;
    static GList *Arrivee_cadran = NULL;
    static GList *Arrivee_comment = NULL;
    static GList *Arrivee_palette = NULL;
    static GList *Arrivee_camera_sup = NULL;
    static GList *Arrivee_camera_for_atelier = NULL;
    static int save_id;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC:
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC2:
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_CTRL:
             { struct CMD_TYPE_MNEMO_BASE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_BASE *)connexion->donnees;
               Proto_afficher_mnemo_atelier( Reseau_ss_tag ( connexion ), mnemo );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_MOTIF_OK:
             { struct CMD_TYPE_MOTIF *motif;
               motif = (struct CMD_TYPE_MOTIF *)connexion->donnees;
               Proto_afficher_un_motif_atelier( motif );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_MOTIF_OK:
             { struct CMD_TYPE_MOTIF *motif;
               motif = (struct CMD_TYPE_MOTIF *)connexion->donnees;
               Proto_cacher_un_motif_atelier( motif );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_COMMENT_OK:
             { struct CMD_TYPE_COMMENT *comment;
               comment = (struct CMD_TYPE_COMMENT *)connexion->donnees;
               Proto_afficher_un_comment_atelier( comment );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_COMMENT_OK:
             { struct CMD_TYPE_COMMENT *comment;
               comment = (struct CMD_TYPE_COMMENT *)connexion->donnees;
               Proto_cacher_un_comment_atelier( comment );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_PASS_OK:
             { struct CMD_TYPE_PASSERELLE *pass;
               pass = (struct CMD_TYPE_PASSERELLE *)connexion->donnees;
               Proto_afficher_une_passerelle_atelier( pass );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_PASS_OK:
             { struct CMD_TYPE_PASSERELLE *pass;
               pass = (struct CMD_TYPE_PASSERELLE *)connexion->donnees;
               Proto_cacher_une_passerelle_atelier( pass );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_PALETTE_OK:
             { struct CMD_TYPE_PALETTE *palette;
               palette = (struct CMD_TYPE_PALETTE *)connexion->donnees;
               Proto_afficher_une_palette_atelier( palette );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_PALETTE_OK:
             { struct CMD_TYPE_PALETTE *palette;
               palette = (struct CMD_TYPE_PALETTE *)connexion->donnees;
               Proto_cacher_une_palette_atelier( palette );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_ADD_CADRAN_OK:
             { struct CMD_TYPE_CADRAN *cadran;
               cadran = (struct CMD_TYPE_CADRAN *)connexion->donnees;
               Proto_afficher_un_cadran_atelier( cadran );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_CADRAN_OK:
             { struct CMD_TYPE_CADRAN *cadran;
               cadran = (struct CMD_TYPE_CADRAN *)connexion->donnees;
               Proto_cacher_un_cadran_atelier( cadran );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               Set_progress_plus(1);

               syn = (struct CMD_TYPE_SYNOPTIQUE *)g_try_malloc0( sizeof( struct CMD_TYPE_SYNOPTIQUE ) );
               if (!syn) return;
               memcpy( syn, connexion->donnees, sizeof(struct CMD_TYPE_SYNOPTIQUE ) );
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
               Set_progress_plus(1);

               cam = (struct CMD_TYPE_CAMERA *)g_try_malloc0( sizeof( struct CMD_TYPE_CAMERA ) );
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
/***************************************** Reception des cameras de supervision ***********************************************/
       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP:
             { struct CMD_TYPE_CAMERASUP *camera_sup;
               Set_progress_plus(1);

               camera_sup = (struct CMD_TYPE_CAMERASUP *)g_try_malloc0( sizeof( struct CMD_TYPE_CAMERASUP ) );
               if (!camera_sup) return;
               memcpy( camera_sup, connexion->donnees, sizeof(struct CMD_TYPE_CAMERASUP ) );
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
             { struct CMD_TYPE_CAMERASUP *cam_sup;
               cam_sup = (struct CMD_TYPE_CAMERASUP *)connexion->donnees;
               Proto_afficher_un_camera_sup_atelier( cam_sup );
             }
            break;
       case SSTAG_SERVEUR_ATELIER_DEL_CAMERA_SUP_OK:
             { struct CMD_TYPE_CAMERASUP *cam_sup;
               cam_sup = (struct CMD_TYPE_CAMERASUP *)connexion->donnees;
               Proto_cacher_un_camera_sup_atelier( cam_sup );
             }
            break;
/******************************************** Reception des palettes **************************************/
       case SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_PALETTE:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               Set_progress_plus(1);

               syn = (struct CMD_TYPE_SYNOPTIQUE *)g_try_malloc0( sizeof( struct CMD_TYPE_SYNOPTIQUE ) );
               if (!syn) return;
               memcpy( syn, connexion->donnees, sizeof(struct CMD_TYPE_SYNOPTIQUE ) );
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
/**************************************************** Reception des motifs ****************************************************/
       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF:
             { struct CMD_TYPE_MOTIFS *motifs;
               gint i;
               motifs = (struct CMD_TYPE_MOTIFS *)connexion->donnees;
               Set_progress_plus( motifs->nbr_motifs );
               for (i=0; i<motifs->nbr_motifs; i++)
                { struct CMD_TYPE_MOTIF *motif;
                  motif = (struct CMD_TYPE_MOTIF *)g_try_malloc0( sizeof( struct CMD_TYPE_MOTIF ) );
                  if (!motif) break;
                  memcpy( motif, &motifs->motif[i], sizeof(struct CMD_TYPE_MOTIF ) );
                  Arrivee_motif = g_list_append( Arrivee_motif, motif );
                  save_id = motif->syn_id;
                }
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
             { struct CMD_TYPE_COMMENT *comment;
               Set_progress_plus(1);

               comment = (struct CMD_TYPE_COMMENT *)g_try_malloc0( sizeof( struct CMD_TYPE_COMMENT ) );
               if (!comment) return;
               memcpy( comment, connexion->donnees, sizeof(struct CMD_TYPE_COMMENT ) );
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
             { struct CMD_TYPE_PASSERELLE *pass;
               Set_progress_plus(1);

               pass = (struct CMD_TYPE_PASSERELLE *)g_try_malloc0( sizeof( struct CMD_TYPE_PASSERELLE ) );
               if (!pass) { printf("Pas assez de mémoire\n"); return;  }
               memcpy( pass, connexion->donnees, sizeof(struct CMD_TYPE_PASSERELLE ) );
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

       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CADRAN:
             { struct CMD_TYPE_CADRAN *cadran;
               Set_progress_plus(1);

               cadran = (struct CMD_TYPE_CADRAN *)g_try_malloc0( sizeof( struct CMD_TYPE_CADRAN ) );
               if (!cadran) { printf("Pas assez de mémoire\n"); return;  }
               memcpy( cadran, connexion->donnees, sizeof(struct CMD_TYPE_CADRAN ) );
               Arrivee_cadran = g_list_append( Arrivee_cadran, cadran );
               save_id = cadran->syn_id;
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CADRAN_FIN:
             { g_list_foreach( Arrivee_cadran, (GFunc)Proto_afficher_un_cadran_atelier, NULL );
               g_list_foreach( Arrivee_cadran, (GFunc)g_free, NULL );
               g_list_free( Arrivee_cadran );
               Arrivee_cadran = NULL;
               Chercher_page_notebook( TYPE_PAGE_ATELIER, save_id, TRUE );
             }
            break;

       case SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE:
             { struct CMD_TYPE_PALETTE *palette;
               Set_progress_plus(1);
               palette = (struct CMD_TYPE_PALETTE *)g_try_malloc0( sizeof( struct CMD_TYPE_PALETTE ) );
               if (!palette) return;
               memcpy( palette, connexion->donnees, sizeof(struct CMD_TYPE_PALETTE ) );
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
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
