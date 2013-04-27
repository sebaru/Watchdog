/**********************************************************************************************************/
/* Watchdogd/HttpMobile/HttpMobile.h        Déclaration structure internes des HttpMobile                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                   mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * httpmobile.h
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
 
#ifndef _HTTP_H_
 #define _HTTP_H_
 #include <microhttpd.h>

 struct HTTP_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean enable;                              /* True si la config indique que le thread doit tourner */
    gint port;
    struct MHD_Daemon *server;
 } Cfg_httpmobile;

/*************************************** Définitions des prototypes ***************************************/
#endif
/*--------------------------------------------------------------------------------------------------------*/
