/******************************************************************************************************************************/
/* Watchdogd/mqtt_visuels.c        Distribution des Visuels à l'API                                                           */
/* Projet WatchDog version 4.0       Gestion d'habitat                                          lun 10 mai 2004 11:31:17 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * api_Ixxx.c
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

 #include <glib.h>
 #include <string.h>
 #include <unistd.h>
 #include <time.h>
 #include <sys/prctl.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* MQTT_Send_visuels_to_API: Envoi les visuels a l'API                                                                        */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void MQTT_Send_visuels_to_API ( void )
  { gint cpt = 0;
    JsonNode *element = Json_node_create ();
    while (Partage->com_msrv.liste_visuel && Partage->com_msrv.Thread_run == TRUE && cpt<100)
     { pthread_mutex_lock( &Partage->com_msrv.synchro );
       struct DLS_VISUEL *visuel = Partage->com_msrv.liste_visuel->data;                            /* Recuperation du visuel */
       Partage->com_msrv.liste_visuel = g_slist_remove ( Partage->com_msrv.liste_visuel, visuel );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );

       Info_new( __func__, Config.log_msrv, LOG_DEBUG,
                "Send VISUEL %s:%s mode=%s, color=%s, valeur='%f', cligno=%d, libelle='%s', disable=%d",
                 visuel->tech_id, visuel->acronyme, visuel->mode, visuel->color, visuel->valeur, visuel->cligno, visuel->libelle, visuel->disable
               );
       Dls_VISUEL_to_json ( element, visuel );
       MQTT_Send_to_API ( element, "DLS_VISUEL" );
       cpt++;
     }
    Json_node_unref ( element );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
