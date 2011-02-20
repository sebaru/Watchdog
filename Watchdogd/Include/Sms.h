/**********************************************************************************************************/
/* Watchdogd/Include/Sms.h        Déclaration structure internes des SMS                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mer 15 avr 2009 15:46:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * sms.h
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
 
#ifndef _SMS_H_
 #define _SMS_H_

 #define TOP_MIN_ENVOI_SMS     1200                           /* 2 minutes sans envoi de SMS au démarrage */

 struct COM_SMS                                                        /* Communication entre MSRV et SMS */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *liste_sms;                                              /* liste de struct MSGDB msg a envoyer */
    gboolean sigusr1;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Run_sms ( void );                                                              /* Dans Sms.c */
 extern void Envoyer_sms ( struct CMD_TYPE_MESSAGE *msg );
 extern void Envoyer_sms_smsbox_text ( gchar *texte );
#endif
/*--------------------------------------------------------------------------------------------------------*/
