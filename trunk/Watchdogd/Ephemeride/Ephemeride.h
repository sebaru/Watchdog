/******************************************************************************************************************************/
/* Watchdogd/Ephemeride/Ephemeride.h        Déclaration structure internes pour l'Ephemeride Watchdog                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    12.03.2021 18:37:46 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * sEphemeride.h
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

#ifndef _EPHEMERIDE_H_
 #define _EPHEMERIDE_H_

 #include <curl/curl.h>

 #define NOM_THREAD                    "ephemeride"
 #define SMSG_TEMPS_UPDATE_COMM        300

 struct EPHEMERIDE_CONFIG
  { struct LIBRAIRIE *lib;
    gchar tech_id[32];                                                                                /* Tech_id du téléphone */
    gchar description[80];                                         /* Une description du téléphone ou sa position par exemple */
    gchar token[65];                                                                                 /* Clef API MeteoConcept */
    gchar code_insee[32];                                                                         /* Code Insee de la commune */
    gint  last_request;
    gboolean test_api;                                                      /* True pour tester l'accès a l'API meteo concept */
    void *zmq_to_master;                                             /* Envoi des events au master si l'instance est un slave */
  };

/*********************************************** Définitions des prototypes ***************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
