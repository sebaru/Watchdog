/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_scenario.c    Gestion du protocole_scenario pour Watchdog                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 03 aoû 2008 16:29:42 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_scenario.c
 * This file is part of Watchodg
 *
 * Copyright (C) 2008 - sebastien
 *
 * Watchodg is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchodg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchodg; if not, write to the Free Software
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
 void Gerer_protocole_scenario( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_SCENARIO) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_SCENARIO:
             { Client_mode( client, ENVOI_SCENARIO );
             }
            break;
       case SSTAG_CLIENT_EDIT_SCENARIO:
             { struct CMD_ID_SCENARIO *msg;
               msg = (struct CMD_ID_SCENARIO *)connexion->donnees;
               Proto_editer_scenario( client, msg );
             }
            break;
       case SSTAG_CLIENT_ADD_SCENARIO:
             { struct CMD_ADD_SCENARIO *msg;
               msg = (struct CMD_ADD_SCENARIO *)connexion->donnees;
               Proto_ajouter_scenario( client, msg );
             }
            break;
       case SSTAG_CLIENT_DEL_SCENARIO:
             { struct CMD_ID_SCENARIO *msg;
               msg = (struct CMD_ID_SCENARIO *)connexion->donnees;
               Proto_effacer_scenario( client, msg );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_SCENARIO:
             { struct CMD_EDIT_SCENARIO *msg;
               msg = (struct CMD_EDIT_SCENARIO *)connexion->donnees;
               Proto_valider_editer_scenario( client, msg );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               printf("Le client desire le mnemonique %d %d\n", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_SCENARIO, SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE,
                                                 client, mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
