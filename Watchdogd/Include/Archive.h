/******************************************************************************************************************************/
/* Watchdogd/Include/Archive.h        Déclaration structure internes des archivages                                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          sam 08 jui 2006 12:02:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.h
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

#ifndef _ARCHIVAGE_DB_H_
 #define _ARCHIVAGE_DB_H_

 #define ARCHIVE_MAX_ENREG_TO_API   500

 enum
  { ARCHIVE_NONE,
    ARCHIVE_5_SEC,
    ARCHIVE_1_MIN,
    ARCHIVE_1_HEURE,
    ARCHIVE_1_JOUR
  };

/******************************************* Définitions des prototypes *******************************************************/
 extern void Ajouter_arch( gchar *tech_id, gchar *nom, gdouble valeur );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
