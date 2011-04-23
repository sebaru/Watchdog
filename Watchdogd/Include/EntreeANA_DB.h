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
 
#ifndef _ENTREEANA_H_
 #define _ENTREEANA_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_ENTREEANA    "eana"

 struct ENTREE_ANA                             /* Traitement des entrées analogiques par le process rs485 */
  { struct CMD_TYPE_OPTION_ENTREEANA cmd_type_eana;
    gdouble val_ech;
    gint    val_int;
    guint   last_arch;                                                     /* Date de la derniere archive */
    guint   inrange;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Charger_eana ( void );
 extern struct CMD_TYPE_OPTION_ENTREEANA *Rechercher_entreeANADB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Recuperer_entreeANADB ( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_OPTION_ENTREEANA *Recuperer_entreeANADB_suite( struct LOG *log, struct DB *db );
 extern gboolean Modifier_entreeANADB( struct LOG *log, struct DB *db, struct CMD_TYPE_OPTION_ENTREEANA *entreeana );
#endif
/*--------------------------------------------------------------------------------------------------------*/
