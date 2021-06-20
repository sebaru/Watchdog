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
 #include <libsoup/soup.h>
 #include <json-glib/json-glib.h>
 #include <phidget22.h>

 #define NOM_THREAD                "phidget"

 struct PHIDGET_CONFIG                                                                 /* Communication entre DLS et la Phidget */
  { struct LIBRAIRIE *lib;
    GSList *Liste_sensors;
  };

 struct PHIDGET_ELEMENT
  { PhidgetHandle handle;
    gchar hub_tech_id[32];
    gpointer bit_comm;
    gchar capteur[32];
    gchar classe[32];
    gint intervalle;
    union { struct DLS_AI *dls_ai;
            struct DLS_DI *dls_di;
            struct DLS_DO *dls_do;
          };
  };

/****************************************************** Déclaration des prototypes ********************************************/

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
