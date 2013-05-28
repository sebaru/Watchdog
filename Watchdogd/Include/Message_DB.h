/**********************************************************************************************************/
/* Watchdogd/Include/Message.h        Déclaration structure internes des messages watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 10 jun 2003 12:12:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Message.h
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
 
#ifndef _MESSAGE_DB_H_
 #define _MESSAGE_DB_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_MSG       "msgs"

/*************************************** Définitions des prototypes ***************************************/
 extern struct CMD_TYPE_MESSAGE *Rechercher_messageDB ( guint num );
 extern struct CMD_TYPE_MESSAGE *Rechercher_messageDB_par_id ( guint id );
 extern gboolean Recuperer_messageDB ( struct DB **db );
 extern struct CMD_TYPE_MESSAGE *Recuperer_messageDB_suite( struct DB **db );
 extern gint Ajouter_messageDB ( struct CMD_TYPE_MESSAGE *msg );
 extern gboolean Retirer_messageDB ( struct CMD_TYPE_MESSAGE *msg );
 extern gboolean Modifier_messageDB( struct CMD_TYPE_MESSAGE *msg );

#endif
/*--------------------------------------------------------------------------------------------------------*/
