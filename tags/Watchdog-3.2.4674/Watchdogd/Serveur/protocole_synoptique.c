/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_synoptique.c    Gestion du protocole_synoptique pour Watchdog              */
/* Projet WatchDog version 3.0       Gestion d'habitat                   dim. 13 sept. 2009 11:59:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_synoptique.c
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
/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_synoptique( struct CLIENT *client )
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
     { case SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE:
             { Envoi_client( client, TAG_SYNOPTIQUE, SSTAG_SERVEUR_CREATE_PAGE_SYNOPTIQUE_OK, NULL, 0 );
               Ref_client( client, "Send Synoptique" );
               pthread_create( &tid, NULL, (void *)Envoyer_synoptiques_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_EDIT_SYNOPTIQUE:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               Proto_editer_synoptique( client, syn );
             }
            break;
       case SSTAG_CLIENT_ADD_SYNOPTIQUE:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               Proto_ajouter_synoptique( client, syn );
             }
            break;
       case SSTAG_CLIENT_DEL_SYNOPTIQUE:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               Proto_effacer_synoptique( client, syn );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_SYNOPTIQUE:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               Proto_valider_editer_synoptique( client, syn );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
