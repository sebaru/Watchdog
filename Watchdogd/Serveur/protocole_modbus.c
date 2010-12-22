/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_modbus.c    Gestion du protocole_modbus pour Watchdog                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 05 sept. 2010 15:45:27 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_modbus.c
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
 void Gerer_protocole_modbus( gint Id_serveur, struct CLIENT *client )
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
     { case SSTAG_CLIENT_WANT_PAGE_MODBUS:
             { Envoi_client( client, TAG_MODBUS, SSTAG_SERVEUR_CREATE_PAGE_MODBUS_OK, NULL, 0 );
               Ref_client( client );                             /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_modbus_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_EDIT_MODBUS:
             { struct CMD_TYPE_MODBUS *modbus;
               modbus = (struct CMD_TYPE_MODBUS *)connexion->donnees;
               Proto_editer_modbus( client, modbus );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_MODBUS:
             { struct CMD_TYPE_MODBUS *modbus;
               modbus = (struct CMD_TYPE_MODBUS *)connexion->donnees;
               Proto_valider_editer_modbus( client, modbus );
             }
            break;
       case SSTAG_CLIENT_ADD_MODBUS:
             { struct CMD_TYPE_MODBUS *modbus;
               modbus = (struct CMD_TYPE_MODBUS *)connexion->donnees;
               Proto_ajouter_modbus( client, modbus );
             }
            break;
       case SSTAG_CLIENT_DEL_MODBUS:
             { struct CMD_TYPE_MODBUS *modbus;
               modbus = (struct CMD_TYPE_MODBUS *)connexion->donnees;
               Proto_effacer_modbus( client, modbus );
             }
            break;
       case SSTAG_CLIENT_WANT_BORNE_MODBUS:
             { struct CMD_TYPE_MODBUS *modbus;
               modbus = (struct CMD_TYPE_MODBUS *)connexion->donnees;
               client->id_modbus_bornes_a_editer = modbus->id;
               Ref_client( client );                             /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_borne_modbus_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_DEL_BORNE_MODBUS:
             { struct CMD_TYPE_BORNE_MODBUS *borne;
               borne = (struct CMD_TYPE_BORNE_MODBUS *)connexion->donnees;
               Proto_effacer_borne_modbus( client, borne );
             }
            break;
       case SSTAG_CLIENT_TYPE_NUM_MNEMO_MODBUS:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               Proto_envoyer_type_num_mnemo_tag( TAG_MODBUS, SSTAG_SERVEUR_TYPE_NUM_MNEMO_MODBUS,
                                                 client, mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
