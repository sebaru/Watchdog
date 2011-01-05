/**********************************************************************************************************/
/* Include/Unite.h:   Déclaration des unite pour watchdog 2.0 par lefevre Sebastien                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 15 déc. 2010 09:47:24 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Unite.h
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

#ifndef _UNITE_H_
 #define _UNITE_H_

 enum
  { UNITE_DEGRE_C,                                                   /* Definitions des types de messages */
    UNITE_DEGRE_K,
    UNITE_BAR,
    UNITE_MILLIBAR,
    UNITE_METRE,
    UNITE_METRE_PAR_SECONDE,
    UNITE_KILOMETRE,
    UNITE_KILOMETRE_PAR_HEURE,
    UNITE_LITRE,
    UNITE_LITRE_PAR_SECONDE,
    UNITE_LITRE_PAR_MINUTE,
    UNITE_DECILITRE,
    UNITE_DECILITRE_PAR_SECONDE,
    UNITE_DECILITRE_PAR_MINUTE,
    UNITE_METRE_CUBE,
    UNITE_METRE_CUBE_PAR_SECONDE,
    UNITE_METRE_CUBE_PAR_MINUTE,
    UNITE_POURCENT,
    UNITE_POURCENT_HR,
    UNITE_SECONDE,
    UNITE_HEURE,
    UNITE_AMPERE,
    UNITE_VOLTS,
    NBR_TYPE_UNITE
  };

 extern gchar *Unite_vers_string ( guint type );

#endif
/*--------------------------------------------------------------------------------------------------------*/

