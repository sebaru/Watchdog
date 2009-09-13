/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_atelier.c    Gestion du protocole_atelier pour Watchdog            */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_atelier.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
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
 #include "Reseaux.h"
 #include "watchdogd.h"
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_atelier( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_SYNOPTIQUE) )
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
             { struct CMD_ID_SYNOPTIQUE *syn;
               syn = (struct CMD_ID_SYNOPTIQUE *)connexion->donnees;
               printf("Le client desire le synoptique numéro %d: %s\n", syn->id, syn->libelle );
               memcpy( &client->syn, connexion->donnees, sizeof(struct CMD_ID_SYNOPTIQUE) );
                                                              /* Sauvegarde du syn voulu pour envoi motif */
               Client_mode( client, ENVOI_MOTIF_ATELIER );
             }
            break;
       case SSTAG_CLIENT_ATELIER_ADD_MOTIF:
             { struct CMD_ADD_MOTIF *motif;
               motif = (struct CMD_ADD_MOTIF *)connexion->donnees;
               printf("Le client desire ajouter un motif\n" );
               Proto_ajouter_motif_atelier( client, motif );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_MOTIF:
             { struct CMD_ID_MOTIF *motif;
               motif = (struct CMD_ID_MOTIF *)connexion->donnees;
               printf("Le client desire effacer le motif numéro %d: %s\n", motif->id, motif->libelle );
               Proto_effacer_motif_atelier( client, motif );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_MOTIF:
             { struct CMD_EDIT_MOTIF *motif;
               motif = (struct CMD_EDIT_MOTIF *)connexion->donnees;
               printf("Le client desire modifier le motif numéro %d: %s\n", motif->id, motif->libelle );
               Proto_valider_editer_motif_atelier( client, motif );
             }
            break;
       case SSTAG_CLIENT_WANT_PAGE_CLASSE_FOR_ATELIER:
             { Client_mode( client, ENVOI_CLASSE_FOR_ATELIER );
               printf("Le client desire les classes icones par atelier\n" );
             }
            break;
       case SSTAG_CLIENT_WANT_PAGE_ICONE_FOR_ATELIER:
             { Client_mode( client, ENVOI_ICONE_FOR_ATELIER );
               client->classe_icone = ((struct CMD_ID_CLASSE *)connexion->donnees)->id;
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMO_CLIC:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               printf("Le client desire le mnemonique %d %d\n", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_ATELIER, SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC,
                                                 client, mnemo );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMO_CLIC2:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               printf("Le client desire le mnemonique %d %d\n", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_ATELIER, SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC2,
                                                 client, mnemo );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMO_CTRL:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               printf("Le client desire le mnemonique %d %d\n", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_ATELIER, SSTAG_SERVEUR_TYPE_NUM_MNEMO_CTRL,
                                                 client, mnemo );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_EA:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               printf("Le client desire le mnemonique %d %d\n", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_ATELIER, SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_EA,
                                                 client, mnemo );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_PASS:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               printf("Le client desire le mnemonique %d %d\n", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_ATELIER, SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_PASS,
                                                         client, mnemo );
             }
            break;
/************************************* Gestion des commentaires synoptiques *******************************/
       case SSTAG_CLIENT_ATELIER_ADD_COMMENT: 
             { struct CMD_ADD_COMMENT *comment;
               comment = (struct CMD_ADD_COMMENT *)connexion->donnees;
               Info_c( Config.log, DEBUG_INFO, "Le client desire ajouter un commentaire", comment->libelle );
               Proto_ajouter_comment_atelier( client, comment );
               Info_c( Config.log, DEBUG_INFO, "fin ajout commentaire", comment->libelle );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_COMMENT:
             { struct CMD_ID_COMMENT *comment;
               comment = (struct CMD_ID_COMMENT *)connexion->donnees;
               printf("Le client desire effacer le comment numéro %d: %s\n", comment->id, comment->libelle );
               Proto_effacer_comment_atelier( client, comment );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_COMMENT:
             { struct CMD_EDIT_COMMENT *comment;
               comment = (struct CMD_EDIT_COMMENT *)connexion->donnees;
               Info_n( Config.log, DEBUG_INFO, "Le client desire modifier le comment numéro", comment->id );
               Proto_valider_editer_comment_atelier( client, comment );
               Info_n( Config.log, DEBUG_INFO, "fin edit comment numéro", comment->id );
             }
            break;
/************************************* Gestion des passerelle synoptiques *********************************/
       case SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER:
             { printf("Le client desire les syn pour atelier\n" );
               Client_mode( client, ENVOI_SYNOPTIQUE_FOR_ATELIER );
             }
            break;
       case SSTAG_CLIENT_ATELIER_ADD_PASS:
             { struct CMD_ADD_PASSERELLE *pass;
               pass = (struct CMD_ADD_PASSERELLE *)connexion->donnees;
               printf("Le client desire ajouter une passerelle\n" );
               Proto_ajouter_passerelle_atelier( client, pass );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_PASS:
             { struct CMD_ID_PASSERELLE *pass;
               pass = (struct CMD_ID_PASSERELLE *)connexion->donnees;
               printf("Le client desire virer une passerelle\n" );
               Proto_effacer_passerelle_atelier( client, pass );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_PASS:
             { struct CMD_EDIT_PASSERELLE *pass;
               pass = (struct CMD_EDIT_PASSERELLE *)connexion->donnees;
               Info_n( Config.log, DEBUG_INFO, "Le client desire modifier le pass numéro", pass->id );
               Proto_valider_editer_passerelle_atelier( client, pass );
               Info_n( Config.log, DEBUG_INFO, "fin edit le pass numéro", pass->id );
             }
            break;
/************************************* Gestion des palettes synoptiques ***********************************/
       case SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER_PALETTE: 
             { struct CMD_ID_SYNOPTIQUE *syn;
               printf("Le client desire les syn pour palettes atelier\n" );
               memcpy ( &client->syn, connexion->donnees, sizeof (struct CMD_ID_SYNOPTIQUE) );
               Client_mode( client, ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE );
             }
            break;
       case SSTAG_CLIENT_ATELIER_ADD_PALETTE:
             { struct CMD_ADD_PALETTE *palette;
               palette = (struct CMD_ADD_PALETTE *)connexion->donnees;
               printf("Le client desire ajouter une palette\n" );
               Proto_ajouter_palette_atelier( client, palette );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_PALETTE:
             { struct CMD_EDIT_PALETTE *palette;
               palette = (struct CMD_EDIT_PALETTE *)connexion->donnees;
               Info_n( Config.log, DEBUG_INFO, "Le client desire modifier le palette numéro", palette->id );
               Proto_valider_editer_palette_atelier( client, palette );
               Info_n( Config.log, DEBUG_INFO, "fin edit le palette numéro", palette->id );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_PALETTE:
             { struct CMD_ID_PALETTE *palette;
               palette = (struct CMD_ID_PALETTE *)connexion->donnees;
               Info_n( Config.log, DEBUG_INFO, "Le client desire deleter le palette numéro", palette->id );
               Proto_effacer_palette_atelier( client, palette );
               Info_n( Config.log, DEBUG_INFO, "fin effacer le palette numéro", palette->id );
             }
            break;
/*********************************** Gestion des capteurs synoptiques *************************************/
       case SSTAG_CLIENT_ATELIER_ADD_CAPTEUR:
             { struct CMD_ADD_CAPTEUR *capteur;
               capteur = (struct CMD_ADD_CAPTEUR *)connexion->donnees;
               printf("Le client desire ajouter un capteur\n" );
               Proto_ajouter_capteur_atelier( client, capteur );
             }
            break;
       case SSTAG_CLIENT_ATELIER_DEL_CAPTEUR:
             { struct CMD_ID_CAPTEUR *capteur;
               capteur = (struct CMD_ID_CAPTEUR *)connexion->donnees;
               printf("Le client desire effacer le texte numéro %d: %s\n", capteur->id, capteur->libelle );
               Proto_effacer_capteur_atelier( client, capteur );
             }
            break;
       case SSTAG_CLIENT_ATELIER_EDIT_CAPTEUR:
             { struct CMD_EDIT_CAPTEUR *capteur;
               capteur = (struct CMD_EDIT_CAPTEUR *)connexion->donnees;
               Info_n( Config.log, DEBUG_INFO, "Le client desire modifier le palette numéro", capteur->id );
               Proto_valider_editer_capteur_atelier( client, capteur );
               Info_n( Config.log, DEBUG_INFO, "fin edit le palette numéro", capteur->id );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
