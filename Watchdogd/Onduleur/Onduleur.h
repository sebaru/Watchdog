/**********************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.h   Header et constantes des modules UPS Watchdgo 2.0                      */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                 mar. 10 nov. 2009 16:35:20 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Onduleur.h
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

#ifndef _UPS_H_
 #define _UPS_H_
 #include <json-glib/json-glib.h>
 #include <upsclient.h>

 #define UPS_PORT_TCP    3493                                                 /* Port de connexion TCP pour accès aux modules */
 #define UPS_RETRY       1800                                              /* 3 minutes entre chaque retry si pb de connexion */
 #define UPS_POLLING      100                                      /* Si tout va bien, on s'y connecte toutes les 10 secondes */

 struct UPS_VARS
  { UPSCONN_t upsconn;                                                                               /* Connexion UPS à l'ups */
    gboolean started;                                                                                      /* Est-il actif ?? */
    time_t date_next_connexion;
/************************************************************ Analog Input ****************************************************/
    JsonNode *Load;
    JsonNode *Realpower;
    JsonNode *Battery_charge;
    JsonNode *Input_voltage;
    JsonNode *Battery_runtime;
    JsonNode *Battery_voltage;
    JsonNode *Input_hz;
    JsonNode *Output_current;
    JsonNode *Output_hz;
    JsonNode *Output_voltage;
/*********************************************************** Digital Input ****************************************************/
    JsonNode *Outlet_1_status;
    JsonNode *Outlet_2_status;
    JsonNode *Ups_online;
    JsonNode *Ups_charging;
    JsonNode *Ups_on_batt;
    JsonNode *Ups_replace_batt;
    JsonNode *Ups_alarm;
  };

/************************************************* Déclaration des prototypes *************************************************/

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/

