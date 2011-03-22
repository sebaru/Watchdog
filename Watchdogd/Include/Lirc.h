/**********************************************************************************************************/
/* Watchdogd/Include/Lirc.h        Déclaration structure internes des structures LIRC                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 01 mars 2011 20:08:31 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Lirc.h
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
 
#ifndef _LIRC_H_
 #define _LIRC_H_

 struct COM_LIRC                                                               /* Communication vers LIRC */
  { pthread_t TID;                                                               /* Identifiant du thread */
    pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    gboolean sigusr1;
    void *handle;                                          /* handle de gestion de la librairie tellstick */
    void (*Run_lirc)(void);                                   /* Fonction principale de gestion du thread */
  };

/*************************************** Définitions des prototypes ***************************************/
#endif
/*--------------------------------------------------------------------------------------------------------*/
