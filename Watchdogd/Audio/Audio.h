/******************************************************************************************************************************/
/* Watchdogd/Include/Audio.h        Déclaration structure internes pour audio                                                 */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                     mer 15 avr 2009 15:40:43 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Audio.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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

 #include <json-glib/json-glib.h>

 #define AUDIO_JINGLE                3000                                    /* Jingle si pas de message au bout de 5 minutes */
 #define AUDIO_DEFAUT_LANGUAGE       "fr"                                                  /* Language par défaut pour le TTS */

 struct AUDIO_VARS
  { gint last_audio;                                                                   /* Date de la derniere emission sonore */
    gboolean diffusion_enabled;                                                        /* Diffusion autorisée pourle thread ? */
/*********************************************************** Digital Input ****************************************************/
    JsonNode *p_all;
    JsonNode *p_none;
  };

/*********************************************** Définitions des prototypes ***************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
