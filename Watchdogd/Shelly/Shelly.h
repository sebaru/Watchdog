/******************************************************************************************************************************/
/* Watchdogd/Include/Shelly.h   Header et constantes des modules SHELLY Watchdgo 4.0                                          */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                08.03.2024 23:33:53 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Shelly.h
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

#ifndef _SHELLY_H_
 #define _SHELLY_H_

/************************************************** Gestion des shelly ********************************************************/
 #define SHELLY_PRO_EM_50   "shellyproem50"
 #define SHELLY_PRO_3_EM    "shellypro3em"

 struct SHELLY_VARS
  { /* Shelly PRO EM 50 Monophasé */
    JsonNode *EM10_ACT_POWER;
    JsonNode *EM10_APRT_POWER;
    JsonNode *EM10_CURRENT;
    JsonNode *EM10_FREQ;
    JsonNode *EM10_PF;
    JsonNode *EM10_VOLTAGE;
    JsonNode *EM10_ENERGY;
    JsonNode *EM10_INJECTION;
    JsonNode *EM11_ACT_POWER;
    JsonNode *EM11_APRT_POWER;
    JsonNode *EM11_CURRENT;
    JsonNode *EM11_FREQ;
    JsonNode *EM11_PF;
    JsonNode *EM11_VOLTAGE;
    JsonNode *EM11_ENERGY;
    JsonNode *EM11_INJECTION;
    /* Shelly PRO_3_EM triphasé */
    JsonNode *U1;
    JsonNode *U2;
    JsonNode *U3;
    JsonNode *I1;
    JsonNode *I2;
    JsonNode *I3;
    JsonNode *I_TOTAL;
    JsonNode *ACT_TOTAL;
    JsonNode *ACT_POWER1;
    JsonNode *ACT_POWER2;
    JsonNode *ACT_POWER3;
    JsonNode *APRT_TOTAL;
    JsonNode *APRT_POWER1;
    JsonNode *APRT_POWER2;
    JsonNode *APRT_POWER3;
    JsonNode *FREQ1;
    JsonNode *FREQ2;
    JsonNode *FREQ3;
    JsonNode *PF1;
    JsonNode *PF2;
    JsonNode *PF3;
    JsonNode *ENERGY1;
    JsonNode *ENERGY2;
    JsonNode *ENERGY3;
    JsonNode *INJECTION1;
    JsonNode *INJECTION2;
    JsonNode *INJECTION3;
  };

/****************************************************** Déclaration des prototypes ********************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
