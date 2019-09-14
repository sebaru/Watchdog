/******************************************************************************************************************************/
/* Watchdogd/Snips/Snips.h        Déclaration structure internes pour snips                                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    14.03.2019 19:48:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Snips.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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

#ifndef _SNIPS_H_
 #define _SNIPS_H_

 #include <json-glib/json-glib.h>
 #include <mosquitto.h>

 #define NOM_THREAD                 "snips"

 struct SNIPS_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean enable;                                                                      /* Is this thread enabled at boot ? */
    void *zmq_to_master;                                                             /* Queue de remontée des infos au master */
    guint nbr_msg_recu;
    gchar snips_host[24];
  } Cfg_snips;

/*********************************************** Définitions des prototypes ***************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
