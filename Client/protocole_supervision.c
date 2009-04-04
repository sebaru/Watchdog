/**********************************************************************************************************/
/* Client/protocole_supervision.c    Gestion du protocole_supervision pour Watchdog                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 12:13:51 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_supervision.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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
 void Gerer_protocole_supervision ( struct CONNEXION *connexion )
  { static GList *Arrivee_motif = NULL;
    static GList *Arrivee_comment = NULL;
    static GList *Arrivee_palette = NULL;
    static GList *Arrivee_capteur = NULL;
    static GList *Arrivee_pass = NULL;
    static GList *Arrivee_scenario = NULL;
    static int save_id;                

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_AFFICHE_PAGE_SUP:
             { struct CMD_SHOW_SYNOPTIQUE *syn;
               syn = (struct CMD_SHOW_SYNOPTIQUE *)connexion->donnees;
               Creer_page_supervision( syn->libelle, syn->id );         /* Creation de la page synoptique */
               Chercher_page_notebook( TYPE_PAGE_SUPERVISION, syn->id, TRUE );    /* Affichage de la page */
             }
       case SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF:
             { struct CMD_ETAT_BIT_CTRL *change_motif;
               change_motif = (struct CMD_ETAT_BIT_CTRL *)connexion->donnees;
               Proto_changer_etat_motif( change_motif );
             }
            break;
       case SSTAG_SERVEUR_SUPERVISION_CHANGE_CAPTEUR:
             { struct CMD_ETAT_BIT_CAPTEUR *change_capteur;
               change_capteur = (struct CMD_ETAT_BIT_CAPTEUR *)connexion->donnees;
               Proto_changer_etat_capteur( change_capteur );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF:
             { struct CMD_SHOW_MOTIF *motif;
               Set_progress_plusun();

               motif = (struct CMD_SHOW_MOTIF *)g_malloc0( sizeof( struct CMD_SHOW_MOTIF ) );
               if (!motif) return; 
               memcpy( motif, connexion->donnees, sizeof(struct CMD_SHOW_MOTIF ) );
               Arrivee_motif = g_list_append( Arrivee_motif, motif );
               save_id = motif->syn_id;               
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF_FIN:
             { g_list_foreach( Arrivee_motif, (GFunc)Proto_afficher_un_motif_supervision, NULL );
               g_list_foreach( Arrivee_motif, (GFunc)g_free, NULL );
               g_list_free( Arrivee_motif );
               Arrivee_motif = NULL;
               Chercher_page_notebook( TYPE_PAGE_SUPERVISION, save_id, TRUE );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT:
             { struct CMD_SHOW_COMMENT *comment;
               Set_progress_plusun();

               comment = (struct CMD_SHOW_COMMENT *)g_malloc0( sizeof( struct CMD_SHOW_COMMENT ) );
               if (!comment) return; 
               memcpy( comment, connexion->donnees, sizeof(struct CMD_SHOW_COMMENT ) );
               Arrivee_comment = g_list_append( Arrivee_comment, comment );
               save_id = comment->syn_id;
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT_FIN:
             { g_list_foreach( Arrivee_comment, (GFunc)Proto_afficher_un_comment_supervision, NULL );
               g_list_foreach( Arrivee_comment, (GFunc)g_free, NULL );
               g_list_free( Arrivee_comment );
               Arrivee_comment = NULL;
               Chercher_page_notebook( TYPE_PAGE_SUPERVISION, save_id, TRUE );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS:
             { struct CMD_SHOW_PASSERELLE *pass;
               Set_progress_plusun();

               pass = (struct CMD_SHOW_PASSERELLE *)g_malloc0( sizeof( struct CMD_SHOW_PASSERELLE ) );
               if (!pass) return; 
               memcpy( pass, connexion->donnees, sizeof(struct CMD_SHOW_PASSERELLE ) );
               Arrivee_pass = g_list_append( Arrivee_pass, pass );
               save_id = pass->syn_id;
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS_FIN:
             { g_list_foreach( Arrivee_pass, (GFunc)Proto_afficher_une_passerelle_supervision, NULL );
               g_list_foreach( Arrivee_pass, (GFunc)g_free, NULL );
               g_list_free( Arrivee_pass );
               Arrivee_pass = NULL;
               Chercher_page_notebook( TYPE_PAGE_SUPERVISION, save_id, TRUE );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PALETTE:
             { struct CMD_SHOW_PALETTE *palette;
               Set_progress_plusun();

               palette = (struct CMD_SHOW_PALETTE *)g_malloc0( sizeof( struct CMD_SHOW_PALETTE ) );
               if (!palette) return; 
               memcpy( palette, connexion->donnees, sizeof(struct CMD_SHOW_PALETTE ) );
               Arrivee_palette = g_list_append( Arrivee_palette, palette );
               save_id = palette->syn_id;
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PALETTE_FIN:
             { g_list_foreach( Arrivee_palette, (GFunc)Proto_afficher_une_palette_supervision, NULL );
               g_list_foreach( Arrivee_palette, (GFunc)g_free, NULL );
               g_list_free( Arrivee_palette );
               Arrivee_palette = NULL;
               Chercher_page_notebook( TYPE_PAGE_SUPERVISION, save_id, TRUE );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAPTEUR:
             { struct CMD_SHOW_CAPTEUR *capteur;
               Set_progress_plusun();

               capteur = (struct CMD_SHOW_CAPTEUR *)g_malloc0( sizeof( struct CMD_SHOW_CAPTEUR ) );
               if (!capteur) return; 
               memcpy( capteur, connexion->donnees, sizeof(struct CMD_SHOW_CAPTEUR ) );
               Arrivee_capteur = g_list_append( Arrivee_capteur, capteur );
               save_id = capteur->syn_id;
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAPTEUR_FIN:
             { g_list_foreach( Arrivee_capteur, (GFunc)Proto_afficher_un_capteur_supervision, NULL );
               g_list_foreach( Arrivee_capteur, (GFunc)g_free, NULL );
               g_list_free( Arrivee_capteur );
               Arrivee_capteur = NULL;
               Chercher_page_notebook( TYPE_PAGE_SUPERVISION, save_id, TRUE );
             }
            break;
       case SSTAG_SERVEUR_SUPERVISION_ADD_SCENARIO_OK:
             { struct CMD_SHOW_SCENARIO *sce;
               sce = (struct CMD_SHOW_SCENARIO *)connexion->donnees;
               Proto_supervision_afficher_un_scenario( sce );
             }
            break;
       case SSTAG_SERVEUR_SUPERVISION_DEL_SCENARIO_OK:
             { struct CMD_ID_SCENARIO *sce;
               sce = (struct CMD_ID_SCENARIO *)connexion->donnees;
               Proto_supervision_cacher_un_scenario( sce );
             }
            break;
       case SSTAG_SERVEUR_SUPERVISION_EDIT_SCENARIO_OK:
             { struct CMD_EDIT_SCENARIO *sce;
               sce = (struct CMD_EDIT_SCENARIO *)connexion->donnees;
               Menu_supervision_ajouter_editer_scenario( sce );
             }
            break;
       case SSTAG_SERVEUR_SUPERVISION_VALIDE_EDIT_SCENARIO_OK:
             { struct CMD_SHOW_SCENARIO *sce;
               sce = (struct CMD_SHOW_SCENARIO *)connexion->donnees;
               Proto_supervision_rafraichir_un_scenario( sce );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_SCENARIO:
             { struct CMD_SHOW_SCENARIO *sce;
               Set_progress_plusun();
printf("Addprogress scenario\n");
               sce = (struct CMD_SHOW_SCENARIO *)g_malloc0( sizeof( struct CMD_SHOW_SCENARIO ) );
               if (!sce) return; 
               memcpy( sce, connexion->donnees, sizeof(struct CMD_SHOW_SCENARIO ) );
               Arrivee_scenario = g_list_append( Arrivee_scenario, sce );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_SCENARIO_FIN:
             { g_list_foreach( Arrivee_scenario, (GFunc)Proto_supervision_afficher_un_scenario, NULL );
               g_list_foreach( Arrivee_scenario, (GFunc)g_free, NULL );
               g_list_free( Arrivee_scenario );
               Arrivee_scenario = NULL;
printf("Addprogress scenario fin\n");
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
