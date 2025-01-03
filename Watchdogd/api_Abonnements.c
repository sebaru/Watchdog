/******************************************************************************************************************************/
/* Watchdogd/api_Abonnements.c        Distribution des abonnements l'API                                                      */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                17.02.2023 22:05:20 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * api_Abonnements.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* API_Send_Abonnements: Envoi les abonnements à l'API                                                                        */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void API_Send_Abonnements ( void )
  { gint cpt = 0;
    while (Partage->abonnements && Partage->com_msrv.Thread_run == TRUE && cpt<100)
     { pthread_mutex_lock( &Partage->abonnements_synchro );                           /* Ajout dans la liste de msg a traiter */
       JsonNode *element = Partage->abonnements->data;
       Partage->abonnements = g_slist_remove ( Partage->abonnements, element );
       pthread_mutex_unlock( &Partage->abonnements_synchro );
       MQTT_Send_to_API ( "DLS_ABONNEMENT", element );
       Json_node_unref ( element );
       cpt++;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
