/**********************************************************************************************************/
/* Watchdogd/Include/Admin.h        Déclaration structure internes pour admin                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer 15 avr 2009 15:40:43 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Admin.h
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
 
#ifndef _ADMIN_H_
 #define _ADMIN_H_

 #include <pthread.h>

 #define NOM_SOCKET "socket.wdg"

 enum
  { MODE_ADMIN_RUNNING,
    MODE_ADMIN_MODBUS,
    MODE_ADMIN_PROCESS,
    MODE_ADMIN_RS485,
    MODE_ADMIN_DLS,
    MODE_ADMIN_ONDULEUR,
    MODE_ADMIN_TELLSTICK,
    NBR_MODE_ADMIN
  };
 extern gchar *Mode_admin[NBR_MODE_ADMIN];

 struct CLIENT_ADMIN
  { gint connexion;
    gint mode;
    gint last_use;
  };

 struct COM_ADMIN                                                  /* Communication entre DLS et la RS485 */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    gboolean sigusr1; 
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Run_admin ( void );                                                          /* Dans Audio.c */
 extern void Admin_modbus ( struct CLIENT_ADMIN *client, gchar *ligne );
 extern void Admin_modbus_list ( struct CLIENT_ADMIN *client );
 extern void Admin_running ( struct CLIENT_ADMIN *client, gchar *ligne );
 extern void Admin_process ( struct CLIENT_ADMIN *client, gchar *ligne );
 extern void Admin_rs485 ( struct CLIENT_ADMIN *client, gchar *ligne );
 extern void Admin_rs485_list ( struct CLIENT_ADMIN *client );
 extern void Admin_dls ( struct CLIENT_ADMIN *client, gchar *ligne );
 extern void Admin_dls_list ( struct CLIENT_ADMIN *client );
 extern void Admin_onduleur ( struct CLIENT_ADMIN *client, gchar *ligne );
 extern void Admin_onduleur_list ( struct CLIENT_ADMIN *client );
 extern void Write_admin ( gint fd, gchar *chaine );

#endif
/*--------------------------------------------------------------------------------------------------------*/
