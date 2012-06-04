/**********************************************************************************************************/
/* Watchdogd/Include/Rfxcom.h        Déclaration structure internes des communication RFXCOM              */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 27 mai 2012 13:02:55 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Rfxcom.h
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
 
#ifndef _RFXCOM_H_
 #define _RFXCOM_H_

 #define TAILLE_ENTETE_RFXCOM    1

 struct TRAME_RFXCOM                                                     /* Definition d'une trame RFXCOM */
  { unsigned char taille;
    unsigned char type;
    unsigned char sous_type;
    unsigned char seqno;
    unsigned char data[40];
  };

 struct COM_RFXCOM                                                           /* Communication vers RFXCOM */
  { pthread_t TID;                                                               /* Identifiant du thread */
    void *dl_handle;                                          /* handle de gestion de la librairie rfxcom */
    void (*Run_rfxcom)(void);                                 /* Fonction principale de gestion du thread */
    void (*Ajouter_rfxcom)( gint, gint );    /* Fonction d'ajout d'une sortie rfxcom dans le tampon */
    pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    gboolean Thread_run;                /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean Thread_sigusr1;                                      /* TRUE si le thread doit gerer le USR1 */
  };

/*************************************** Définitions des prototypes ***************************************/
#endif
/*--------------------------------------------------------------------------------------------------------*/
