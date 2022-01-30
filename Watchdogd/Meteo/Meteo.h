/******************************************************************************************************************************/
/* Watchdogd/Meteo/Meteo.h        Déclaration structure internes pour les données Meteo de Watchdog                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    12.03.2021 18:37:46 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * sMeteo.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

#ifndef _METEO_H_
 #define _METEO_H_

 #define METEO_POLLING                 36000                                             /* Polling du site toutes les heures */

 struct METEO_VARS
  { gint  last_request;
    JsonNode *Temp_min[13];
    JsonNode *Temp_max[13];
    JsonNode *Proba_pluie[13];
    JsonNode *Proba_gel[13];
    JsonNode *Proba_brouillard[13];
    JsonNode *Proba_vent_70[13];
    JsonNode *Proba_vent_100[13];
    JsonNode *Proba_vent_orage[13];
    JsonNode *Vent_10m[13];
    JsonNode *Direction_vent[13];
    JsonNode *Rafale_vent[13];
  };

/*********************************************** Définitions des prototypes ***************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
