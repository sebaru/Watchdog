/******************************************************************************************************************************/
/* Watchdogd/mqtt.c        Fonctions communes de gestion des requetes HTTP                                                    */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                30.12.2020 22:03:58 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mqtt.c
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

 #include <glib.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* MQTT_on_log_CB: Affiche un log de la librairie MQTT                                                                        */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_on_log_CB( struct mosquitto *mosq, void *obj, int level, const char *message )
  { gint info_level;
    switch(level)
     { case MOSQ_LOG_DEBUG:   return;                                         /* On ne log pas les message de DEBUG mosquitto */
       default:
       case MOSQ_LOG_INFO:    info_level = LOG_INFO;    break;
       case MOSQ_LOG_NOTICE:  info_level = LOG_NOTICE;  break;
       case MOSQ_LOG_WARNING: info_level = LOG_WARNING; break;
       case MOSQ_LOG_ERR:     info_level = LOG_ERR;     break;
     }
    Info_new( __func__, Config.log_msrv, info_level, "%s", message );
  }
/******************************************************************************************************************************/
/* MQTT_on_connect_CB: appelé par la librairie quand le broker est connecté                                                   */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_on_connect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Connected with return code %d: %s",
              return_code, mosquitto_connack_string( return_code ) );
    if (return_code == 0) Partage->com_msrv.MQTT_connected = TRUE ;
  }
/******************************************************************************************************************************/
/* MQTT_on_disconnect_CB: appelé par la librairie quand le broker est déconnecté                                              */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_on_disconnect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Disconnected with return code %d: %s",
              return_code, mosquitto_connack_string( return_code ) );
    Partage->com_msrv.MQTT_connected = FALSE;
  }
/******************************************************************************************************************************/
/* MQTT_Subscribe: souscrit à un topic                                                                                        */
/* Entrée: la structure MQTT, le topic                                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Subscribe ( struct mosquitto *mqtt_session, gchar *format, ... )
  { va_list ap;

    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound ( format, ap );
    va_end ( ap );
    gchar *topic = g_try_malloc(taille+1);
    if (!topic)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Memory Error for '%s'", format );
       return;
     }

    va_start( ap, format );
    g_vsnprintf ( topic, taille, format, ap );
    va_end ( ap );

    if ( mosquitto_subscribe( mqtt_session, NULL, topic, 2 ) != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, Config.log_bus, LOG_ERR, "Subscribe to topic '%s' FAILED", topic ); }
    else
     { Info_new( __func__, Config.log_bus, LOG_INFO, "Subscribe to topic '%s' OK", topic ); }

    g_free(topic);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
