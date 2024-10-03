/******************************************************************************************************************************/
/* Watchdogd/Teleinfo/Teleinfo.h        Déclaration structure internes des communication TELEINFO                             */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                    dim. 27 mai 2012 13:02:55 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Teleinfo.h
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

#ifndef _TELEINFO_H_
 #define _TELEINFO_H_

 #define TAILLE_BUFFER_TELEINFO           128
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
    JsonNode *IRMS1;
    JsonNode *IRMS2;
    JsonNode *IRMS3;
    JsonNode *URMS1;
    JsonNode *URMS2;
    JsonNode *URMS3;
    JsonNode *PREF;
    JsonNode *PCOUP;
    JsonNode *SINSTS;
    JsonNode *SINSTS1;
    JsonNode *SINSTS2;
    JsonNode *SINSTS3;
    JsonNode *SMAXSN;
    JsonNode *SMAXSN1;
    JsonNode *SMAXSN2;
    JsonNode *SMAXSN3;
    JsonNode *UMOY1;
    JsonNode *UMOY2;
    JsonNode *UMOY3;
    JsonNode *NTARF;
    JsonNode *ADSC;
    JsonNode *EAST;
    JsonNode *EASF01;
    JsonNode *EASF02;
    JsonNode *EASF03;
    JsonNode *EASF04;
    JsonNode *EASF05;
    JsonNode *EASF06;
    JsonNode *EASF07;
    JsonNode *EASF08;
    JsonNode *EASF09;
    JsonNode *EASF10;
    JsonNode *EASD01;
    JsonNode *EASD02;
    JsonNode *EASD03;
    JsonNode *EASD04;
    JsonNode *PRM;
 };

/************************************************ Définitions des prototypes **************************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
