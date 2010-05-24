/**********************************************************************************************************/
/* Watchdogd/Include/EntreeANA.h        Déclaration structure internes des entree analogiques             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 25 jun 2004 11:17:05 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * EntreeANA.h
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
 
#ifndef _CMD_TYPE_ENTREEANA_H_
 #define _CMD_TYPE_ENTREEANA_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Cst_entreeana.h"

 #define NOM_TABLE_ENTREEANA    "eana"

/*************************************** Définitions des prototypes ***************************************/
 extern void Charger_eana ( void );
 extern struct CMD_TYPE_ENTREEANA *Rechercher_entreeANADB ( struct LOG *log, struct DB *db, guint num );
 extern gboolean Recuperer_entreeANADB ( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_ENTREEANA *Recuperer_entreeANADB_suite( struct LOG *log, struct DB *db );
 extern gboolean Modifier_entreeANADB( struct LOG *log, struct DB *db, struct CMD_TYPE_ENTREEANA *entreeana );
#endif
/*--------------------------------------------------------------------------------------------------------*/
