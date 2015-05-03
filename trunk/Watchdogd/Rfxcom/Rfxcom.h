/******************************************************************************************************************************/
/* Watchdogd/Rfxcom/Rfxcom.h        Déclaration structure internes des communication RFXCOM                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        dim. 27 mai 2012 13:02:55 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

 #define DEFAUT_PORT_RFXCOM             "/dev/watchdog_RFXCOM"

 #define NOM_THREAD                "rfxcom"
 #define NOM_TABLE_MODULE_RFXCOM   "rfxcom"
 #define NBR_CARAC_LIBELLE_RFXCOM  128
 #define TAILLE_ENTETE_RFXCOM      1
 #define RFXCOM_RETRY_DELAI        600                                      /* Une minute avant de se reconnecter si probleme */

 struct TRAME_RFXCOM                                                                         /* Definition d'une trame RFXCOM */
  { unsigned char taille;
    unsigned char type;
    unsigned char sous_type;
    unsigned char seqno;
    unsigned char data[40];
  };

 enum
  { RFXCOM_WAIT_BEFORE_RETRY,
	   RFXCOM_RETRING,
	   RFXCOM_CONNECTED
  };
  
 struct RFXCOM_CONFIG
  { struct LIBRAIRIE *lib;
    gint  mode;                                                                                /* Mode de connexion au RFXCOM */
    gint  date_next_retry;                                                 /* Date de la prochaine connexion au port teleinfo */
    gchar port[80];
    gint fd;                                                                    /* File descripteur de la connexion au RFXCOM */
    gboolean enable;                                                                               /* Thread enable at boot ? */
    gboolean reload;
    GSList *Modules_RFXCOM;                                                                      /* Listes des modules RFXCOM */
    GSList *Liste_events;                                                                  /* Liste des sorties a positionner */
 } Cfg_rfxcom;

/************************************************** Définitions des prototypes ************************************************/
 extern gboolean Rfxcom_Lire_config ( void );

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
