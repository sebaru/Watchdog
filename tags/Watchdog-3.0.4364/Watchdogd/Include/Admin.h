/******************************************************************************************************************************/
/* Watchdogd/Include/Admin.h        Déclaration structure internes pour admin                                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         mer 15 avr 2009 15:40:43 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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


/*********************************************** Définitions des prototypes ***************************************************/
 extern gchar *Admin_set ( gchar *response, gchar *ligne );                                                   /* Dans Admin.c */
 extern gchar *Admin_get ( gchar *response, gchar *ligne );
 extern gchar *Admin_write ( gchar *response, gchar *new_ligne );
 extern gchar *Processer_commande_admin ( gchar *user, gchar *host, gchar *ligne );
 extern void New_Processer_commande_admin ( struct ZMQ_TARGET *event, gchar *ligne );

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
