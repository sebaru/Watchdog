/******************************************************************************************************************************/
/* Watchdogd/Include/Audio.h        Déclaration structure internes pour audio                                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         mer 15 avr 2009 15:40:43 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Audio.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

 #define NOM_THREAD                 "audio"

 #define AUDIO_JINGLE                3000                                    /* Jingle si pas de message au bout de 5 minutes */
 #define AUDIO_DEFAUT_LANGUAGE       "fr"                                                  /* Language par défaut pour le TTS */

 struct AUDIO_CONFIG
  { struct LIBRAIRIE *lib;
    gint last_audio;                                                                   /* Date de la derniere emission sonore */
    gboolean enable;                                                                      /* Is this thread enabled at boot ? */
    gchar language[80];                                             /* Language de restitution vocal, au format google_speech */
    gchar device[32];
    guint nbr_diffusion_wav;
    guint nbr_diffusion_google;
    gboolean diffusion_enabled;                                                        /* Diffusion autorisée pourle thread ? */
  };

/*********************************************** Définitions des prototypes ***************************************************/
 extern gboolean Audio_Lire_config ( void );
 extern gchar *Audio_Admin_response( gchar *ligne );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
