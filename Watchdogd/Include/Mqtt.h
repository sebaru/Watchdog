/******************************************************************************************************************************/
/* Watchdogd/Include/Mqtt.h        Déclaration structure internes des archivages                                              */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                      sam 08 jui 2006 12:02:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mqtt.h
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

#ifndef _MQTT_H_
 #define _MQTT_H_

 #define ARCHIVE_NONE           0
 #define ARCHIVE_5_SEC          50
 #define ARCHIVE_1_MIN          600
 #define ARCHIVE_5_MIN          3000
 #define ARCHIVE_1_HEURE        36000
 #define ARCHIVE_1_JOUR         864000

/******************************************* Définitions des prototypes *******************************************************/
 extern gboolean MQTT_Start_MQTT_API ( void );                                                               /* Dans mqtt_x.h */
 extern void MQTT_Stop_MQTT_API ( void );
 extern void MQTT_Send_MSGS_to_API ( void );
 extern void MQTT_Send_visuels_to_API ( void );
 extern void MQTT_Send_archive_to_API( gchar *tech_id, gchar *nom, gdouble valeur );

 extern gboolean MQTT_Start_MQTT_LOCAL ( void );                                                             /* Dans mqtt_x.h */
 extern void MQTT_Stop_MQTT_LOCAL ( void );

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
