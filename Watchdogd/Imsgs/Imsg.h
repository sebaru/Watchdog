/******************************************************************************************************************************/
/* Watchdogd/Imsgs/Imsg.h     Header et constantes des modules imsg Purple Watchdgog 2.0                                      */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                25.02.2018 16:27:36 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Imsg.h
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

#ifndef _IMSGS_H_
 #define _IMSGS_H_

 #include <strophe.h>

 enum
  { IMSG_NOTIF_SET_BY_DLS = -1,
    IMSG_NOTIF_NO  = 0,
    IMSG_NOTIF_YES = 1
  };

 #define IMSGS_DEFAUT_USERNAME      "defaultuser@jabber.fr"
 #define IMSGS_DEFAUT_PASSWORD      "defaultpassword"
 #define IMSGS_TIME_RECONNECT      600                                              /* 1 minute avant reconnexion si probleme */

 struct IMSGS_VARS
  { xmpp_ctx_t *ctx;
    xmpp_conn_t *conn;
    gboolean signed_off;
  };
/*********************************************** Déclarations des prototypes **************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
