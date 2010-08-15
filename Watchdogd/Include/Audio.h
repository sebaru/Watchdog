/**********************************************************************************************************/
/* Watchdogd/Include/Audio.h        Déclaration structure internes pour audio                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer 15 avr 2009 15:40:43 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Audio.h
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
 
#ifndef _AUDIO_H_
 #define _AUDIO_H_

 #define AUDIO_JINGLE                3000                /* Jingle si pas de message au bout de 5 minutes */
 #define NUM_BIT_M_AUDIO_END         5                       /* M5 positionné quand arret diffusion audio */

 struct COM_AUDIO                                                  /* Communication entre DLS et la RS485 */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *liste_audio;                                                   /* liste de message a prononcer */
    gboolean sigusr1;
    gint last_audio;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Run_audio ( void );                                                          /* Dans Audio.c */
 extern void Ajouter_audio( gint num );
#endif
/*--------------------------------------------------------------------------------------------------------*/
