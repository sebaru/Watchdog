/**********************************************************************************************************/
/* Include/Cst_camera.h        Déclaration des constantes dediées aux cameras                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 11:31:03 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Cst_camera.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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

#ifndef _CST_CAMERA_H_
 #define _CST_CAMERA_H_

 #define NBR_CARAC_LOCATION_CAMERA      100
 #define NBR_CARAC_LOCATION_CAMERA_UTF8 (6*NBR_CARAC_LOCATION_CAMERA)

 enum
  { CAMERA_MODE_INCRUSTATION,                                         /* Definitions des types de cameras */
    CAMERA_MODE_ICONE,
    NBR_TYPE_CAMERA
  };
#endif
/*--------------------------------------------------------------------------------------------------------*/
