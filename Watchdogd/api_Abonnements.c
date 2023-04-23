/******************************************************************************************************************************/
/* Watchdogd/api_Abonnements.c        Distribution des abonnements l'API                                                      */
/* Projet WatchDog version 4.0       Gestion d'habitat                                                    17.02.2023 22:05:20 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * api_Abonnements.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien LEFEVRE
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
  { static gint next_try = 0;
    while (Partage->abonnements && next_try <= Partage->top)
     { pthread_mutex_lock( &Partage->abonnements_synchro );                           /* Ajout dans la liste de msg a traiter */
       JsonNode *RootNode   = Partage->abonnements->data;
       Partage->abonnements = g_slist_remove ( Partage->abonnements, RootNode );
       pthread_mutex_unlock( &Partage->abonnements_synchro );

       JsonNode *api_result = Http_Post_to_global_API ( "/run/abonnement", RootNode );
       if (api_result == NULL || Json_get_int ( api_result, "api_status" ) != SOUP_STATUS_OK)
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "API Post '%s:%s' for /run/abonnemnent failed. Retry %04d enregs in 60 seconds.",
                    Json_get_string ( RootNode, "tech_id"), Json_get_string ( RootNode, "acronyme" ), g_slist_length(Partage->abonnements) );
          Json_node_unref ( api_result );
          Json_node_unref ( RootNode );
          next_try = Partage->top + 600;
          break;
        }
       Json_node_unref ( api_result );
       Json_node_unref ( RootNode );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
