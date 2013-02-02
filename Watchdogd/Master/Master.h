/**********************************************************************************************************/
/* Watchdogd/Include/Sms.h        Déclaration structure internes des MASTER                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mer 15 avr 2009 15:46:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * master.h
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
 
#ifndef _MASTER_H_
 #define _MASTER_H_

 #define TOP_MIN_ENVOI_MASTER     1200                           /* 2 minutes sans envoi de MASTER au démarrage */
 #define TAILLE_MASTERBOX_USERNAME   32                                /* Nombre de caractere du user MASTERBOX */
 #define TAILLE_MASTERBOX_PASSWORD   32                        /* Nombre de caractere du mot de passe MASTERBOX */
 #define DEFAUT_MASTERBOX_USERNAME         "user"
 #define DEFAUT_MASTERBOX_PASSWORD         "changeit"

 struct MASTER_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    GSList *Liste_master;                                             /* liste de struct MSGDB msg a envoyer */
    gchar **recipients;
    gchar masterbox_username[TAILLE_MASTERBOX_USERNAME+1];                                       /* User MASTERBOX */
    gchar masterbox_password[TAILLE_MASTERBOX_PASSWORD+1];                         /* Mot de passe envoi MASTERBOX */
 } Cfg_master;

/*************************************** Définitions des prototypes ***************************************/
 extern void Envoyer_master_masterbox_text ( gchar *texte );                                      /* Dans Sms.c */
 extern void Envoyer_master_gsm_text ( gchar *texte );
#endif
/*--------------------------------------------------------------------------------------------------------*/
