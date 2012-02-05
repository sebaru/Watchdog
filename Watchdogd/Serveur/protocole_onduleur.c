/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_onduleur.c    Gestion du protocole_onduleur pour Watchdog                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                   mer. 14 juil. 2010 20:40:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_onduleur.c
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
 #include "Reseaux.h"
 #include "watchdogd.h"
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_onduleur( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_LOWLEVEL_IO) )
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message), "Permission denied" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_ONDULEUR:
             { Envoi_client( client, TAG_ONDULEUR, SSTAG_SERVEUR_CREATE_PAGE_ONDULEUR_OK, NULL, 0 );
               Ref_client( client );                             /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_onduleurs_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_EDIT_ONDULEUR:
             { struct CMD_TYPE_ONDULEUR *onduleur;
               onduleur = (struct CMD_TYPE_ONDULEUR *)connexion->donnees;
               Proto_editer_onduleur( client, onduleur );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_ONDULEUR:
             { struct CMD_TYPE_ONDULEUR *onduleur;
               onduleur = (struct CMD_TYPE_ONDULEUR *)connexion->donnees;
               Proto_valider_editer_onduleur( client, onduleur );
             }
            break;
       case SSTAG_CLIENT_ADD_ONDULEUR:
             { struct CMD_TYPE_ONDULEUR *onduleur;
               onduleur = (struct CMD_TYPE_ONDULEUR *)connexion->donnees;
               Proto_ajouter_onduleur( client, onduleur );
             }
            break;
       case SSTAG_CLIENT_DEL_ONDULEUR:
             { struct CMD_TYPE_ONDULEUR *onduleur;
               onduleur = (struct CMD_TYPE_ONDULEUR *)connexion->donnees;
               Proto_effacer_onduleur( client, onduleur );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMO_BIT_COMM:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               Proto_envoyer_type_num_mnemo_tag( TAG_ONDULEUR, SSTAG_SERVEUR_TYPE_NUM_MNEMO_BIT_COMM,
                                                 client, mnemo );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMO_EA_MIN:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               Proto_envoyer_type_num_mnemo_tag( TAG_ONDULEUR, SSTAG_SERVEUR_TYPE_NUM_MNEMO_EA_MIN,
                                                 client, mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
