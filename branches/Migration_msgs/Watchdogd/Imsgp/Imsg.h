/******************************************************************************************************************************/
/* Watchdogd/Imsgp/Imsg.h     Header et constantes des modules imsg Purple Watchdgog 2.0                                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    25.02.2018 16:27:36 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Imsg.h
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
 
#ifndef _IMSG_H_
 #define _IMSG_H_

 #include <purple.h>

 #define NOM_THREAD                "imsgp"

 #define IMSGP_DEFAUT_USERNAME      "defaultuser@jabber.fr"
 #define IMSGP_DEFAUT_PASSWORD      "defaultpassword"
 #define IMSGP_TIME_RECONNECT      600                                              /* 1 minute avant reconnexion si probleme */

 struct IMSGPDB
  { gint     user_id;
    gchar    user_jabberid[80];
    gchar    user_name[80];
    gchar    user_comment[80];
    gboolean user_enable;
    gboolean user_imsg_enable;
    gboolean user_allow_cde;
    gboolean user_available;
  };

 struct IMSGP_CONFIG
  { struct LIBRAIRIE *lib;
    PurpleAccount *account;
    gchar username[80];
    gchar password[80];
    gboolean enable;
  } Cfg_imsgp;
/****************************************** Déclarations des prototypes ***********************************/
 extern gboolean Imsgp_Lire_config ( void );
 extern void Imsgp_Envoi_message_to ( const gchar *dest, gchar *message );
 extern void Imsgp_Mode_presence ( gchar *type, gchar *show, gchar *status );
 extern gboolean Recuperer_imsgpDB ( struct DB *db );
 extern struct IMSGPDB *Recuperer_imsgpDB_suite( struct DB *db );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
