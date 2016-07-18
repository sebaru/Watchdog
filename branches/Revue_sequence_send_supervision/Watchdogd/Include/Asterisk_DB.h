/**********************************************************************************************************/
/* Watchdogd/Include/Asterisk.h        Déclaration structure internes des Asterisk watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 11:13:20 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Asterisk.h
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
 
#ifndef _ASTERISK_DB_H_
 #define _ASTERISK_DB_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_ASTERISK              "from_asterisk"

/******************************************** Gestion des camera ******************************************/
 struct CMD_TYPE_ASTERISK
  { guint   id;                                                                 /* ID unique de la camera */
    gchar   calleridnum[128];
    gchar   calleridname[128];
    gint    bit;                      /* Numéro du bistable a positioner en cas de detection de mouvement */
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Asterisk_check_call ( struct LOG *log, struct DB *db );

#endif
/*--------------------------------------------------------------------------------------------------------*/
