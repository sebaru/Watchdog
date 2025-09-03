/******************************************************************************************************************************/
/* Watchdogd/audio.c        Distribution des messages audio                                                                   */
/* Projet Abls-Habitat version 4.5       Gestion d'habitat                                                24.08.2025 18:12:29 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * audio.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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
/* AUDIO_Send_to_zone: Envoi un message vocal à une zone de diffusion                                                         */
/* Entrée: La zone et l'audio a diffuser                                                                                      */
/******************************************************************************************************************************/
 void AUDIO_Send_to_zone ( gchar *audio_zone_name, gchar *audio_libelle )
  { if (!audio_zone_name) return;
    if (!audio_libelle) return;
    JsonNode *AudioNode = Json_node_create();
    if (!AudioNode)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Cannot send '%s' to zone '%s': memory error", audio_libelle, audio_zone_name );
       return;
     }
    Json_node_add_string ( AudioNode, "audio_libelle", audio_libelle );
    struct DLS_DI *bit = Dls_data_lookup_DI ( Json_get_string ( Config.config, "audio_tech_id" ), audio_zone_name );
    Dls_data_set_DI_pulse ( NULL, bit );
    MQTT_Send_to_topic ( Partage->MQTT_local_session, AudioNode, FALSE, "AUDIO_ZONE/%s", audio_zone_name );
    Json_node_unref ( AudioNode );
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Saying '%s' on zone '%s", audio_libelle, audio_zone_name );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
