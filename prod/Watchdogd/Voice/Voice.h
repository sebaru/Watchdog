/******************************************************************************************************************************/
/* Watchdogd/Voice/Voice.h        Déclaration structure internes du thread VOICE                                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    29.12.2018 23:04:43 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Voice.h
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
 
#ifndef _VOICE_H_
 #define _VOICE_H_

 #define NOM_THREAD                    "voice"
 #define QUELLE_VERSION                "quelle est ta version"

 struct VOICE_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean enable;                                                                    /* Is this tread is enabled at boot ? */
    gchar audio_device[80]; /* Nom du device audio: alsa_input.usb-0b0e_Jabra_SPEAK_510_USB_745C4B657953021800-00.analog-mono */
    gchar key_words[80];                                                         /* Mot magic pour debuter la detection audio */
    gchar vad_threshold[5];                                                                    /* Seuil de detection des mots */
    void *zmq_to_master;                                             /* Envoi des events au master si l'instance est un slave */
  } Cfg_voice;

/*********************************************** Définitions des prototypes ***************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
