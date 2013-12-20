/**********************************************************************************************************/
/* Watchdogd/Include/Tempo_DB.h      Déclaration structure internes des temporisations                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                     sam. 09 mars 2013 11:42:30 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Tempo_DB.h
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
 
#ifndef _TEMPO_DB_H_
 #define _TEMPO_DB_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_TEMPO    "tempo"

/*************************************** Définitions des prototypes ***************************************/
 extern void Charger_tempo ( void );
 extern struct CMD_TYPE_OPTION_TEMPO *Rechercher_tempoDB ( guint id );
/* extern gboolean Recuperer_tempoDB ( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_OPTION_TEMPO *Recuperer_tempoDB_suite( struct LOG *log, struct DB *db );*/
 extern gboolean Modifier_tempoDB( struct CMD_TYPE_OPTION_TEMPO *tempo );
#endif
/*--------------------------------------------------------------------------------------------------------*/
