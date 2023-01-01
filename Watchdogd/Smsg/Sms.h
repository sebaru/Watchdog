/******************************************************************************************************************************/
/* Watchdogd/Smsg/Sms.h        Déclaration structure internes des SMS avec Gammu                                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    18.02.2018 11:59:59 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * sms.h
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

#ifndef _SMSG_H_
 #define _SMSG_H_

 #include <gammu.h>

 struct SMS_VARS
  { gboolean sending_is_disabled;                                  /* Variable permettant d'interdire l'envoi de sms si panic */
    GSM_Error gammu_send_status;
    GSM_StateMachine *gammu_machine;
    INI_Section *gammu_cfg;
    JsonNode *ai_nbr_sms;
    JsonNode *ai_signal_quality;
    guint nbr_sms;
  };

/*********************************************** Définitions des prototypes ***************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
