/******************************************************************************************************************************/
/* Watchdogd/Gpiod/Gpiod.h        Déclaration structure internes des communication RASPBERRYPI                                */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                03.09.2021 18:52:35 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Gpiod.h
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

#ifndef _GPIOD_H_
 #define _GPIOD_H_

 #include <gpiod.h>

 #define GPIOD_MAX_LINE    28                                                         /* 28 Lignes maximums d'entrées/sorties */

 struct GPIOD_LIGNE
  { gboolean etat;
    gboolean mode_inout;
    gboolean mode_activelow;
    struct gpiod_line_request *gpio_ligne;
  };

 struct GPIOD_VARS
  { struct gpiod_chip *chip;
    gint   num_lines;
    struct GPIOD_LIGNE *lignes;
 };

/************************************************ Définitions des prototypes **************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
