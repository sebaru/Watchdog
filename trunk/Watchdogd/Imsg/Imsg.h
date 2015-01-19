/**********************************************************************************************************/
/* Watchdogd/Include/Imsg.h   Header et constantes des modules imsg Watchdgog 2.0                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 11 août 2012 14:56:02 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

 #include <loudmouth/loudmouth.h>

 #define NOM_THREAD                "imsg"

 #define DEFAUT_USERNAME_IMSG      "defaultuser"
 #define DEFAUT_SERVER_IMSG        "jabber.fr"
 #define DEFAUT_PASSWORD_IMSG      "defaultpassword"
 #define TIME_RECONNECT_IMSG       600                          /* 1 minute avant reconnexion si probleme */

 struct IMSGDB
  { gint     user_id;
    gchar    user_jabberid[80];
    gchar    user_name[80];
    gchar    user_comment[80];
    gboolean user_enable;
    gboolean user_imsg_enable;
    gboolean user_allow_cde;
    gboolean user_available;
  };

 struct IMSG_CONFIG
  { struct LIBRAIRIE *lib;
    gchar username[80];
    gchar server  [80];
    gchar password[80];
    gboolean enable;
    LmConnection *connection;
    GSList *Contacts;
    GSList *Liste_histos;
    gboolean set_status;
    gboolean reload;
    gchar new_status[80];
    gint date_retente;                                                 /* Date de reconnexion si probleme */
 } Cfg_imsg;
/****************************************** Déclarations des prototypes ***********************************/
 extern gboolean Imsg_Lire_config ( void );
 extern void Imsg_Envoi_message_to ( const gchar *dest, gchar *message );
 extern void Imsg_Mode_presence ( gchar *type, gchar *show, gchar *status );
 extern gboolean Recuperer_imsgDB ( struct DB *db );
 extern struct IMSGDB *Recuperer_imsgDB_suite( struct DB *db );
#endif
/*--------------------------------------------------------------------------------------------------------*/
