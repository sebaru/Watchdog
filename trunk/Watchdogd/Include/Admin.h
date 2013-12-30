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

 struct COM_ADMIN                                                  /* Communication entre DLS et la RS485 */
  { pthread_t TID;                                                               /* Identifiant du thread */
    pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    gboolean Thread_run;                /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_sigusr1;                                      /* TRUE si le thread doit gerer le USR1 */
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Run_admin ( void );                                                          /* Dans Admin.c */
 extern void Admin_running ( struct CONNEXION *connexion, gchar *ligne );
 extern void Admin_process ( struct CONNEXION *connexion, gchar *ligne );
 extern void Admin_dls ( struct CONNEXION *connexion, gchar *ligne );
 extern void Admin_set ( struct CONNEXION *connexion, gchar *ligne );
 extern void Admin_get ( struct CONNEXION *connexion, gchar *ligne );
 extern void Admin_user ( struct CONNEXION *connexion, gchar *ligne );
 extern gboolean Admin_dbcfg_thread ( struct CONNEXION *connexion, gchar *thread, gchar *ligne );
 extern void Admin_write ( struct CONNEXION *connexion, gchar *chaine );
 extern void Processer_commande_admin ( struct CONNEXION *connexion, gchar *user, gchar *host, gchar *ligne );

#endif
/*--------------------------------------------------------------------------------------------------------*/
