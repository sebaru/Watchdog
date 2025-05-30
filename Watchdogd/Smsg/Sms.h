/******************************************************************************************************************************/
/* Watchdogd/Smsg/Sms.h        Déclaration structure internes des SMS avec Gammu                                              */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                18.02.2018 11:59:59 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * sms.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

 enum
  { SMSG_NOTIF_BY_DLS = -1,
    SMSG_NOTIF_NO  = 0,
    SMSG_NOTIF_YES = 1,
    SMSG_NOTIF_OVH_ONLY = 2
  };

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
