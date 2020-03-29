/******************************************************************************************************************************/
/* Watchdogd/Include/Histo_DB.h        Déclaration structure internes des historiques watchdog                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          mar 10 jun 2003 12:12:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Histo_DB.h
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

#ifndef _HISTO_H_
 #define _HISTO_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Message_DB.h"

 #define NOM_TABLE_HISTO_MSGS  "histo_msgs"

/*********************************************** Définitions des prototypes ***************************************************/
 extern gboolean Ajouter_histo_msgsDB ( struct CMD_TYPE_HISTO *histo );
 extern gboolean Modifier_histo_msgsDB ( struct CMD_TYPE_HISTO *histo );
 extern struct CMD_TYPE_HISTO *Rechercher_histo_msgsDB_by_id ( guint id );
 extern gboolean Recuperer_histo_msgsDB_alive ( struct DB **db );
 extern struct CMD_TYPE_HISTO *Recuperer_histo_msgsDB_suite( struct DB **db );

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
