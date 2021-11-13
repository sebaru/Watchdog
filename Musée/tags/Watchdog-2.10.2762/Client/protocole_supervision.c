/**********************************************************************************************************/
/* Client/protocole_supervision.c    Gestion du protocole_supervision pour Watchdog                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 12:13:51 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_supervision.c
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
  { static GList *Arrivee_comment = NULL;
    static GList *Arrivee_palette = NULL;
    static GList *Arrivee_camera_sup = NULL;
    static int save_id = 0;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_AFFICHE_PAGE_SUP:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               Creer_page_supervision( syn->libelle, syn->id );         /* Creation de la page synoptique */
               Chercher_page_notebook( TYPE_PAGE_SUPERVISION, syn->id, TRUE );    /* Affichage de la page */
             }
            break;
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
             { struct CMD_TYPE_MOTIFS *motifs;
               gint i;
               motifs = (struct CMD_TYPE_MOTIFS *)connexion->donnees;
               for (i=0; i<motifs->nbr_motifs; i++)
                { Proto_afficher_un_motif_supervision ( &motifs->motif[i] );
                  save_id = motifs->motif[i].syn_id;
                }
               Set_progress_plus( motifs->nbr_motifs );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF_FIN:
             { Chercher_page_notebook( TYPE_PAGE_SUPERVISION, save_id, TRUE ); }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT:
             { struct CMD_TYPE_COMMENT *comment;
               Set_progress_plus(1);

               comment = (struct CMD_TYPE_COMMENT *)g_try_malloc0( sizeof( struct CMD_TYPE_COMMENT ) );
               if (!comment) return; 
               memcpy( comment, connexion->donnees, sizeof(struct CMD_TYPE_COMMENT ) );
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
             { struct CMD_TYPE_PASSERELLE *pass;
               pass = (struct CMD_TYPE_PASSERELLE *)connexion->donnees;
               Proto_afficher_une_passerelle_supervision ( pass );
               save_id = pass->syn_id;
               Set_progress_plus(1);
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS_FIN:
             { Chercher_page_notebook( TYPE_PAGE_SUPERVISION, save_id, TRUE ); }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PALETTE:
             { struct CMD_TYPE_PALETTE *palette;
               Set_progress_plus(1);

               palette = (struct CMD_TYPE_PALETTE *)g_try_malloc0( sizeof( struct CMD_TYPE_PALETTE ) );
               if (!palette) return; 
               memcpy( palette, connexion->donnees, sizeof(struct CMD_TYPE_PALETTE ) );
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
             { struct CMD_TYPE_CAPTEUR *capteur;
               capteur = (struct CMD_TYPE_CAPTEUR *)connexion->donnees;
               Proto_afficher_un_capteur_supervision ( capteur );
               Set_progress_plus(1);
               save_id = capteur->syn_id;
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAPTEUR_FIN:
             { Chercher_page_notebook( TYPE_PAGE_SUPERVISION, save_id, TRUE ); }
            break;
/******************************************** Reception des cameras de supervision ********************************************/
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAMERA_SUP:
             { struct CMD_TYPE_CAMERA_SUP *camera_sup;
               Set_progress_plus(1);

               camera_sup = (struct CMD_TYPE_CAMERA_SUP *)g_try_malloc0( sizeof( struct CMD_TYPE_CAMERA_SUP ) );
               if (!camera_sup) return; 
               memcpy( camera_sup, connexion->donnees, sizeof(struct CMD_TYPE_CAMERA_SUP ) );
               Arrivee_camera_sup = g_list_append( Arrivee_camera_sup, camera_sup );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAMERA_SUP_FIN:
             { g_list_foreach( Arrivee_camera_sup, (GFunc)Proto_afficher_un_camera_sup_supervision, NULL );
               g_list_foreach( Arrivee_camera_sup, (GFunc)g_free, NULL );
               g_list_free( Arrivee_camera_sup );
               Arrivee_camera_sup = NULL;
             }
            break;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
