/******************************************************************************************************************************/
/* Watchdogd/Include/Message.h        Déclaration structure internes des messages watchdog                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          mar 10 jun 2003 12:12:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

 #include "Db.h"

/******************************************************** Les messages ********************************************************/
 #define NBR_CARAC_LIBELLE_MSG       100                                                                  /* Attention au SMS */
 #define NBR_CARAC_LIBELLE_MSG_UTF8  (2*NBR_CARAC_LIBELLE_MSG)

 enum
  { MSG_ETAT,                                                        /* Definitions des types de messages */
    MSG_ALERTE,
    MSG_DEFAUT,
    MSG_ALARME,
    MSG_VEILLE,
    MSG_ATTENTE,
    MSG_DANGER,
    MSG_DERANGEMENT,
    NBR_TYPE_MSG
  };

 enum
  { MSG_SMS_NONE,
    MSG_SMS_YES,
    MSG_SMS_GSM_ONLY,
    MSG_SMS_SMSBOX_ONLY,
    NBR_TYPE_MSG_SMS
  };

 struct DB_MESSAGE
  { gint   mnemo_id;
    gboolean enable;                                                  /* Flag pour la gestion par exemple de l'inhibition ... */
    guchar type;                                                                           /* Etat, prealarme, defaut, alarme */
    gchar  libelle[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    guint  sms;                                                                                             /* Envoi de sms ? */
    gchar  libelle_sms[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    gboolean audio;                                                                             /* Activation message audio ? */
    gchar  libelle_audio[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    guint  bit_audio;                                                         /* Numéro du Monostable associé au profil vocal */
    guint  time_repeat;                                               /* Temps entre deux répétitions (si non nul) en minutes */
    gboolean persist;                                                                               /* Persistence du message */
    gboolean is_mp3;                                                            /* Un mp3 a-t'il été chargé pour ce message ? */
  };

/******************************************** Définitions des prototypes ******************************************************/
 extern struct DB_MESSAGE *Rechercher_messageDB ( gchar *tech_id, gchar *acro );

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
