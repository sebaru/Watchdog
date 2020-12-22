/******************************************************************************************************************************/
/* Watchdogd/Include/Message.h        Déclaration structure internes des messages watchdog                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          mar 10 jun 2003 12:12:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Message.h
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

#ifndef _MESSAGE_DB_H_
 #define _MESSAGE_DB_H_

 #include "Reseaux.h"

 #define NOM_TABLE_MSG       "msgs"

 enum
  { MESSAGE_SMS_NONE,
    MESSAGE_SMS_YES,
    MESSAGE_SMS_GSM_ONLY,
    MESSAGE_SMS_OVH_ONLY,
    NBR_TYPE_MESSAGE_SMS
  };

/******************************************** Définitions des prototypes ******************************************************/
 extern struct CMD_TYPE_MESSAGE *Rechercher_messageDB_par_acronyme ( gchar *tech_id, gchar *acronyme );
 extern struct CMD_TYPE_MESSAGE *Recuperer_messageDB_suite( struct DB **db );
 extern gint Mnemo_auto_create_MSG ( gchar *tech_id, gchar *acronyme, gchar *libelle, gint typologie );
 extern void Charger_confDB_MSG ( void );
 extern void Updater_confDB_MSG ( void );
 extern void Dls_MESSAGE_to_json ( JsonBuilder *builder, struct DLS_MESSAGES *bit );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
