/**********************************************************************************************************/
/* Watchdogd/Include/Camera.h        D�claration structure internes des Camera watchdog                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 11:13:20 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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
 #define NOM_TABLE_CAMERA_MOTION       "cameras_motion"

/*************************************** D�finitions des prototypes ***************************************/
 extern gboolean Retirer_cameraDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *camera );
 extern gint Ajouter_cameraDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *camera );
 extern gboolean Recuperer_cameraDB ( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_CAMERA *Recuperer_cameraDB_suite( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_CAMERA *Rechercher_cameraDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_cameraDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *camera );
 extern void Camera_check_motion ( struct LOG *log, struct DB *db );

#endif
/*--------------------------------------------------------------------------------------------------------*/
