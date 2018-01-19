/******************************************************************************************************************************/
/* Watchdogd/Teleinfo/Teleinfo.h        Déclaration structure internes des communication TELEINFO                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        dim. 27 mai 2012 13:02:55 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Teleinfo.h
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
 
#ifndef _TELEINFO_H_
 #define _TELEINFO_H_

 #define NOM_THREAD                       "teleinfo"

 #define DEFAUT_PORT_TELEINFO             "/dev/watchdog_TELEINFO"
 #define TAILLE_BUFFER_TELEINFO           25
 #define TINFO_RETRY_DELAI                600                               /* Une minute avant de se reconnecter si probleme */

 enum
  { TINFO_WAIT_BEFORE_RETRY,
	   TINFO_RETRING,
   	TINFO_CONNECTED
  };

 struct TELEINFO_CONFIG
  { struct LIBRAIRIE *lib;
    gint  mode;                                                                    /* Statut de connexion au port TeleInfoEDF */
    gint  date_next_retry;                                                 /* Date de la prochaine connexion au port teleinfo */
    gchar port[80];
    gint  fd;                                                               /* File Descriptor d'acces au module Teleinfo USB */
    gint  last_view;                                                                            /* Date du dernier echange OK */
    gint  last_view_adco;
    gint  last_view_isous;
    gint  last_view_hchc;
    gint  last_view_hchp;
    gint  last_view_iinst;
    gint  last_view_imax;
    gint  last_view_papp;
    gint  last_view_hhchp;
    gint  last_view_ptec;
    gint  last_view_optarif;
    gboolean reload;
    gboolean enable;                                                                                      /* Enable at boot ? */
    gchar buffer[TAILLE_BUFFER_TELEINFO];
 } Cfg_teleinfo;

/************************************************ Définitions des prototypes **************************************************/
 extern gboolean Teleinfo_Lire_config ( void );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
