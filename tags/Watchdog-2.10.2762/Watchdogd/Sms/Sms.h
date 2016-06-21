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

 #define NOM_THREAD                     "sms"

 #define TOP_MIN_ENVOI_SMS     1200                           /* 2 minutes sans envoi de SMS au démarrage */
 #define TAILLE_SMSBOX_USERNAME   32                                /* Nombre de caractere du user SMSBOX */
 #define TAILLE_SMSBOX_PASSWORD   32                        /* Nombre de caractere du mot de passe SMSBOX */
 #define DEFAUT_SMSBOX_USERNAME         "user"
 #define DEFAUT_SMSBOX_PASSWORD         "changeit"

 struct SMS_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean enable;                                                /* Is this tread is enabled at boot ? */
    gboolean reload;                                  /* Demande rechargement des SMS en bases de données */
    GSList *Liste_histos;                                          /* liste de struct MSGDB msg a envoyer */
    gchar smsbox_username[TAILLE_SMSBOX_USERNAME+1];                                       /* User SMSBOX */
    gchar smsbox_password[TAILLE_SMSBOX_PASSWORD+1];                         /* Mot de passe envoi SMSBOX */
    guint bit_comm;                                       /* Bit B d'état de la communication avec le GSM */
  } Cfg_sms;

 struct SMSDB
  { gint     user_id;                                                                 /* From users table */
    gchar    user_name[80];
    gboolean user_enable;
    gchar    user_comment[80];
    gboolean user_sms_enable;
    gchar    user_sms_phone[80];
    gboolean user_sms_allow_cde;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern gboolean Sms_Lire_config ( void );
 extern void Envoyer_sms_smsbox_text ( gchar *texte );                                      /* Dans Sms.c */
 extern void Envoyer_sms_gsm_text ( gchar *texte );

 extern gboolean Sms_Recuperer_smsDB ( struct DB *db );
 extern struct SMSDB *Sms_Recuperer_smsDB_suite( struct DB *db );

#endif
/*--------------------------------------------------------------------------------------------------------*/
