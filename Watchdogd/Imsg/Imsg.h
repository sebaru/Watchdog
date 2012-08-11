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

 struct CONFIG_IMSG
  { gchar username[80];
    gchar server  [80];
    gchar password[80];
    gchar **recipients;
    LmConnection *connection;
    struct LIBRAIRIE *lib;
    
 } Cfg_imsg;

#endif
/*--------------------------------------------------------------------------------------------------------*/
