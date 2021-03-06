/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_atelier.c    Gestion du protocole_atelier pour Watchdog                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 17 nov. 2009 13:47:17 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_atelier.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_atelier( struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util, GID_SYNOPTIQUE) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { 
/********************************************* atelier ****************************************************/
       case SSTAG_CLIENT_ATELIER_SYNOPTIQUE:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire le synoptique numéro %d: %s", syn->id, syn->libelle );
               memcpy( &client->syn, connexion->donnees, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
                                                              /* Sauvegarde du syn voulu pour envoi motif */
               Client_mode( client, ENVOI_MOTIF_ATELIER );
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
                         "Le client desire effacer le motif numéro %d: %d", motif->id, motif->libelle );
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
       case SSTAG_CLIENT_WANT_PAGE_CLASSE_FOR_ATELIER:
             { Client_mode( client, ENVOI_CLASSE_FOR_ATELIER );
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire les classes icones par atelier\n" );
             }
            break;
       case SSTAG_CLIENT_WANT_PAGE_ICONE_FOR_ATELIER:
             { Client_mode( client, ENVOI_ICONE_FOR_ATELIER );
               client->classe_icone = ((struct CMD_TYPE_CLASSE *)connexion->donnees)->id;
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
       case SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_PASS:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire le mnemonique %d %d", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_ATELIER, SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_PASS,
                                                         client, mnemo );
             }
            break;
/************************************* Gestion des commentaires synoptiques *******************************/
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
/************************************* Gestion des cameras synoptiques ************************************/
       case SSTAG_CLIENT_WANT_PAGE_CAMERA_FOR_ATELIER:
             { Ref_client( client, "Send Camera for atelier" );
               pthread_create( &tid, NULL, (void *)Envoyer_cameras_for_atelier_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_ATELIER_ADD_CAMERA_SUP: 
             { struct CMD_TYPE_CAMERA_SUP *camera_sup;
               camera_sup = (struct CMD_TYPE_CAMERA_SUP *)connexion->donnees;
               Proto_ajouter_camera_sup_atelier( client, camera_sup );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_CAMERA_SUP:
             { struct CMD_TYPE_CAMERA_SUP *camera_sup;
               camera_sup = (struct CMD_TYPE_CAMERA_SUP *)connexion->donnees;
               Proto_effacer_camera_sup_atelier( client, camera_sup );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_CAMERA_SUP:
             { struct CMD_TYPE_CAMERA_SUP *camera_sup;
               camera_sup = (struct CMD_TYPE_CAMERA_SUP *)connexion->donnees;
               Proto_valider_editer_camera_sup_atelier( client, camera_sup );
             }
            break;
/************************************* Gestion des passerelle synoptiques *********************************/
       case SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER:
             { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire les syn pour atelier" );
               Client_mode( client, ENVOI_SYNOPTIQUE_FOR_ATELIER );
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
/************************************* Gestion des palettes synoptiques ***********************************/
       case SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER_PALETTE: 
             { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire les syn pour palettes atelier" );
               memcpy ( &client->syn, connexion->donnees, sizeof (struct CMD_TYPE_SYNOPTIQUE) );
               Client_mode( client, ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE );
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
/*********************************** Gestion des capteurs synoptiques *************************************/
       case SSTAG_CLIENT_ATELIER_ADD_CAPTEUR:
             { struct CMD_TYPE_CAPTEUR *capteur;
               capteur = (struct CMD_TYPE_CAPTEUR *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire ajouter un capteur" );
               Proto_ajouter_capteur_atelier( client, capteur );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_CAPTEUR:
             { struct CMD_TYPE_CAPTEUR *capteur;
               capteur = (struct CMD_TYPE_CAPTEUR *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire effacer le capteur numéro %d", capteur->id );
               Proto_effacer_capteur_atelier( client, capteur );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_CAPTEUR:
             { struct CMD_TYPE_CAPTEUR *capteur;
               capteur = (struct CMD_TYPE_CAPTEUR *)connexion->donnees;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Le client desire modifier le palette numéro %d", capteur->id );
               Proto_valider_editer_capteur_atelier( client, capteur );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
