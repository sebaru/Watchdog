/******************************************************************************************************************************/
/* Watchdogd/RaspberryPI/RaspberryPI.h        Déclaration structure internes des communication RASPBERRYPI                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    03.09.2021 18:52:35 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * RaspberryPI.h
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

#ifndef _RASPBERRYPI_H_
 #define _RASPBERRYPI_H_

 #include <gpiod.h>

 struct RASPBERRYPI_CONFIG
  { struct LIBRAIRIE *lib;
    gint   delai;                                                     /* Temps d'attente pour avoir les 50 tours par secondes */
    struct gpiod_chip *chip;
    gint   num_lines;
    struct gpiod_line *lines[64];
 };

/************************************************ Définitions des prototypes **************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/