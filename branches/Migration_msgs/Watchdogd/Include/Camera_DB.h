/******************************************************************************************************************************/
/* Watchdogd/Include/Camera.h        Déclaration structure internes des Camera watchdog                                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       dim. 13 sept. 2009 11:13:20 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Camera.h
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
 
#ifndef _CAMERA_DB_H_
 #define _CAMERA_DB_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_CAMERA              "cameras"

/************************************************** Définitions des prototypes ************************************************/
 extern gboolean Retirer_cameraDB ( gint id );
 extern gint Ajouter_cameraDB ( struct CMD_TYPE_CAMERA *camera );
 extern gint Modifier_cameraDB ( struct CMD_TYPE_CAMERA *camera );
 extern gboolean Recuperer_cameraDB ( struct DB **db_retour );
 extern struct CMD_TYPE_CAMERA *Recuperer_cameraDB_suite( struct DB **db_orig );
 extern struct CMD_TYPE_CAMERA *Rechercher_cameraDB ( guint id );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
