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
 #include <curl/curl.h>

 #define NOM_THREAD                    "smsg"
 #define SMSG_TEMPS_UPDATE_COMM        300

 struct SMS_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean sending_is_disabled;                                  /* Variable permettant d'interdire l'envoi de sms si panic */
    GSM_Error gammu_send_status;
    GSM_StateMachine *gammu_machine;
    INI_Section *gammu_cfg;
    gchar tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];                                                       /* Tech_id du téléphone */
    gchar description[80];                                         /* Une description du téléphone ou sa position par exemple */
    gchar ovh_service_name[16];                                                                     /* Login de connexion OVH */
    gchar ovh_application_key[33];                                                                            /* Clef API OVH */
    gchar ovh_application_secret[33];                                                                         /* Clef API OVH */
    gchar ovh_consumer_key[33];                                                                               /* Clef API OVH */
    gboolean send_test_GSM;                                              /* TRUE si une demande de test a été faite par l'IHM */
    gboolean send_test_OVH;                                              /* TRUE si une demande de test a été faite par l'IHM */
    void *zmq_to_master;                                             /* Envoi des events au master si l'instance est un slave */
    guint nbr_sms;
  };

 struct SMSDB
  { gint     user_id;                                                                                     /* From users table */
    gchar    user_name[80];
    gboolean user_enable;
    gchar    user_comment[80];
    gboolean user_notification;
    gchar    user_phone[80];
    gboolean user_allow_cde;
  };

/*********************************************** Définitions des prototypes ***************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
