/******************************************************************************************************************************/
/* Watchdogd/Teleinfo/Teleinfo.h        Déclaration structure internes des communication TELEINFO                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        dim. 27 mai 2012 13:02:55 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Teleinfo.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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

#ifndef _TELEINFO_H_
 #define _TELEINFO_H_

 #define TAILLE_BUFFER_TELEINFO           25
 #define TINFO_RETRY_DELAI                600                               /* Une minute avant de se reconnecter si probleme */

 enum
  { TINFO_WAIT_BEFORE_RETRY,
	   TINFO_RETRING,
   	TINFO_CONNECTED
  };

 struct TELEINFO_VARS
  { struct PROCESS *lib;
    gint  mode;                                                                    /* Statut de connexion au port TeleInfoEDF */
    gint  date_next_retry;                                                 /* Date de la prochaine connexion au port teleinfo */
    gint  fd;                                                               /* File Descriptor d'acces au module Teleinfo USB */
    gchar buffer[TAILLE_BUFFER_TELEINFO];
    gint  last_view;                                                                            /* Date du dernier echange OK */
    JsonNode *Adco;
    JsonNode *Isous;
    JsonNode *Base;
    JsonNode *Hchc;
    JsonNode *Hchp;
    JsonNode *Iinst;
    JsonNode *Imax;
    JsonNode *Papp;
 };

/************************************************ Définitions des prototypes **************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
