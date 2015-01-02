/**********************************************************************************************************/
/* Watchdogd/Include/EntreeTOR.h        Déclaration structure internes des entree analogiques             */
/* Projet WatchDog version 2.0       Gestion d'habitat                    ven. 02 janv. 2015 18:54:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * EntreeTOR.h
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
 
#ifndef _ENTREETOR_H_
 #define _ENTREETOR_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_DIGITAL_INPUT   "mnemos_DigitalInput"

 struct DIGITAL_INPUT                          /* Traitement des entrées analogiques par le process rs485 */
  { struct CMD_TYPE_MNEMO_DI confDB;
    gboolean etat;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Charger_digitalInput ( void );
 extern struct CMD_TYPE_OPTION_MNEMO *Rechercher_digitalInputDB ( guint id );
 extern gboolean Modifier_digitalInputDB( struct CMD_TYPE_OPTION_MNEMO *option_mnemo );
#endif
/*--------------------------------------------------------------------------------------------------------*/
