/******************************************************************************************************************************/
/* Watchdogd/Phidget/Phidget.h   Header et constantes des modules Phidget Watchdgo 3.0                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    18.03.2021 21:59:33 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Phidget.h
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

#ifndef _PHIDGET_H_
 #define _PHIDGET_H_
 #include <phidget22.h>
 #include "watchdogd.h"

 struct PHIDGET_VARS                                                                 /* Communication entre DLS et la Phidget */
  { GSList *Liste_sensors; /* List of Phidget Elements */
  };

 struct PHIDGET_ELEMENT
  { struct SUBPROCESS *module; /* Module père de l'élément */
    PhidgetHandle handle;
    gchar tech_id[32];
    gchar capteur[32];
    gchar classe[32];
    gint intervalle;
    gchar map_tech_id[32];
    gchar map_acronyme[64];
    union { struct DLS_DO *dls_do;
          };
  };

/****************************************************** Déclaration des prototypes ********************************************/

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
