/******************************************************************************************************************************/
/* Watchdogd/Serveur/protocole_atelier.c    Gestion du protocole_atelier pour Watchdog                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mar. 17 nov. 2009 13:47:17 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * protocole_atelier.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
 #include <sys/prctl.h>
/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/******************************************************************************************************************************/
/* Proto_Envoyer_atelier_thread: Envoi du synoptique demandé par le client en mode atelier                                    */
/* Entrée: Le client destinaire                                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void *Proto_Envoyer_atelier_thread ( struct CLIENT *client )
  { gchar titre[20];

    g_snprintf( titre, sizeof(titre), "W-ATLR-%03d", client->ssrv_id );
    prctl(PR_SET_NAME, titre, 0, 0, 0 );

    Envoyer_motif_tag ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF,
	                                         SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF_FIN );

    Envoyer_palette_tag ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE,
                                               SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE_FIN );

    Envoyer_cadran_tag ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CADRAN,
                                               SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CADRAN_FIN );

    Envoyer_passerelle_tag ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS,
	                                              SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS_FIN );

    Envoyer_comment_tag ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT,
                                               SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT_FIN );

    Envoyer_camera_sup_tag ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP,
                                                  SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP_FIN );

    g_free(client->syn_to_send);
    client->syn_to_send = NULL;
    Unref_client( client );                                                               /* Déréférence la structure cliente */
    pthread_exit ( NULL );
  }
/******************************************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                                                 */
/* Entrée: la connexion avec le serveur                                                                                       */
/* Sortie: Kedal                                                                                                              */
/******************************************************************************************************************************/
 void Gerer_protocole_atelier( struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_level_util( client->util, ACCESS_LEVEL_ATELIER ) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     {
/******************************************************** atelier *************************************************************/
       case SSTAG_CLIENT_ATELIER_SYNOPTIQUE:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;         /* Récupération du numéro du synoptique désiré */
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Gerer_protocole_atelier: Le client desire le synoptique numéro %d: %s", syn->id, syn->libelle );

               if ( client->syn_to_send )
                { struct CMD_GTK_MESSAGE gtkmessage;
                  g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Another Syn is sending." );
                  Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
                  return;
                }

               client->syn_to_send = Rechercher_synoptiqueDB ( syn->id );
               if ( ! client->syn_to_send )
                { struct CMD_GTK_MESSAGE gtkmessage;
                  g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Synoptique inconnu..." );
                  Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
                  return;
                }

               Ref_client( client, "Send atelier" );
               pthread_create( &tid, NULL, (void *)Proto_Envoyer_atelier_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_ATELIER_ADD_MOTIF:
             { struct CMD_TYPE_MOTIF *motif;
               motif = (struct CMD_TYPE_MOTIF *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire ajouter un motif" );
               Proto_ajouter_motif_atelier( client, motif );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_MOTIF:
             { struct CMD_TYPE_MOTIF *motif;
               motif = (struct CMD_TYPE_MOTIF *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire effacer le motif numéro %d: %s", motif->id, motif->libelle );
               Proto_effacer_motif_atelier( client, motif );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_MOTIF:
             { struct CMD_TYPE_MOTIF *motif;
               motif = (struct CMD_TYPE_MOTIF *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire modifier le motif numéro %d: %d", motif->id, motif->libelle );
               Proto_valider_editer_motif_atelier( client, motif );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMO_CLIC:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire le mnemonique %d %d", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_ATELIER, SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC,
                                                 client, mnemo );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMO_CLIC2:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire le mnemonique %d %d", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_ATELIER, SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC2,
                                                 client, mnemo );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMO_CTRL:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire le mnemonique %d %d", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_ATELIER, SSTAG_SERVEUR_TYPE_NUM_MNEMO_CTRL,
                                                 client, mnemo );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_EA:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire le mnemonique %d %d", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_ATELIER, SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_EA,
                                                 client, mnemo );
             }
            break;
/************************************************* Gestion des commentaires synoptiques ***************************************/
       case SSTAG_CLIENT_ATELIER_ADD_COMMENT:
             { struct CMD_TYPE_COMMENT *comment;
               comment = (struct CMD_TYPE_COMMENT *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire ajouter le commentaire %s", comment->libelle );
               Proto_ajouter_comment_atelier( client, comment );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_COMMENT:
             { struct CMD_TYPE_COMMENT *comment;
               comment = (struct CMD_TYPE_COMMENT *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire effacer le comment numéro %d: %s", comment->id, comment->libelle );
               Proto_effacer_comment_atelier( client, comment );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_COMMENT:
             { struct CMD_TYPE_COMMENT *comment;
               comment = (struct CMD_TYPE_COMMENT *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire modifier le comment numéro", comment->id );
               Proto_valider_editer_comment_atelier( client, comment );
             }
            break;
/************************************************ Gestion des cameras synoptiques *********************************************/
       case SSTAG_CLIENT_WANT_PAGE_CAMERA_FOR_ATELIER:
             { Ref_client( client, "Send Camera for atelier" );
               pthread_create( &tid, NULL, (void *)Envoyer_cameras_for_atelier_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_ATELIER_ADD_CAMERA_SUP:
             { struct CMD_TYPE_CAMERASUP *camera_sup;
               camera_sup = (struct CMD_TYPE_CAMERASUP *)connexion->donnees;
               Proto_ajouter_camera_sup_atelier( client, camera_sup );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_CAMERA_SUP:
             { struct CMD_TYPE_CAMERASUP *camera_sup;
               camera_sup = (struct CMD_TYPE_CAMERASUP *)connexion->donnees;
               Proto_effacer_camera_sup_atelier( client, camera_sup );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_CAMERA_SUP:
             { struct CMD_TYPE_CAMERASUP *camera_sup;
               camera_sup = (struct CMD_TYPE_CAMERASUP *)connexion->donnees;
               Proto_valider_editer_camera_sup_atelier( client, camera_sup );
             }
            break;
/******************************************* Gestion des passerelle synoptiques ***********************************************/
       case SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER:
             { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire les syn pour atelier" );
               Ref_client( client, "Send synoptique atelier" );
               pthread_create( &tid, NULL, (void *)Envoyer_synoptiques_pour_atelier_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_ATELIER_ADD_PASS:
             { struct CMD_TYPE_PASSERELLE *pass;
               pass = (struct CMD_TYPE_PASSERELLE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire ajouter une passerelle" );
               Proto_ajouter_passerelle_atelier( client, pass );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_PASS:
             { struct CMD_TYPE_PASSERELLE *pass;
               pass = (struct CMD_TYPE_PASSERELLE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire virer une passerelle" );
               Proto_effacer_passerelle_atelier( client, pass );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_PASS:
             { struct CMD_TYPE_PASSERELLE *pass;
               pass = (struct CMD_TYPE_PASSERELLE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire modifier le pass numéro", pass->id );
               Proto_valider_editer_passerelle_atelier( client, pass );
             }
            break;
/******************************************** Gestion des palettes synoptiques ************************************************/
       case SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER_PALETTE:
             { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire les syn pour palettes atelier" );
               Ref_client( client, "Send synoptique atelier palette" );
               pthread_create( &tid, NULL, (void *)Envoyer_synoptiques_pour_atelier_palette_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_ATELIER_ADD_PALETTE:
             { struct CMD_TYPE_PALETTE *palette;
               palette = (struct CMD_TYPE_PALETTE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire ajouter une palette" );
               Proto_ajouter_palette_atelier( client, palette );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_PALETTE:
             { struct CMD_TYPE_PALETTE *palette;
               palette = (struct CMD_TYPE_PALETTE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire modifier le palette numéro %d", palette->id );
               Proto_valider_editer_palette_atelier( client, palette );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_PALETTE:
             { struct CMD_TYPE_PALETTE *palette;
               palette = (struct CMD_TYPE_PALETTE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire deleter le palette numéro %d", palette->id );
               Proto_effacer_palette_atelier( client, palette );
             }
            break;
/************************************************* Gestion des cadrans synoptiques *******************************************/
       case SSTAG_CLIENT_ATELIER_ADD_CADRAN:
             { struct CMD_TYPE_CADRAN *cadran;
               cadran = (struct CMD_TYPE_CADRAN *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire ajouter un cadran" );
               Proto_ajouter_cadran_atelier( client, cadran );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_CADRAN:
             { struct CMD_TYPE_CADRAN *cadran;
               cadran = (struct CMD_TYPE_CADRAN *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire effacer le cadran numéro %d", cadran->id );
               Proto_effacer_cadran_atelier( client, cadran );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_CADRAN:
             { struct CMD_TYPE_CADRAN *cadran;
               cadran = (struct CMD_TYPE_CADRAN *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire modifier le cadran numéro %d", cadran->id );
               Proto_valider_editer_cadran_atelier( client, cadran );
             }
            break;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
