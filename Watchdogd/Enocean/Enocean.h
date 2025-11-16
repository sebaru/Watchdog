/**********************************************************************************************************/
/* Watchdogd/Enocean/Enocean.h        Déclaration structure internes des communication ENOCEAN             */
/* Projet Abls-Habitat version 4.6       Gestion d'habitat                 dim. 28 déc. 2014 15:43:41 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Enocean.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

#ifndef _ENOCEAN_H_
 #define _ENOCEAN_H_

 #define DEFAUT_PORT_ENOCEAN             "/dev/watchdog_ENOCEAN"

 #define NOM_THREAD                 "enocean"
 #define ENOCEAN_HEADER_LENGTH      5
 #define ENOCEAN_RECONNECT_DELAY    300                     /* 30 secondes avant tentative de reconnexion */
 #define ENOCEAN_TRAME_TIMEOUT      2    /* 200ms avant de dropper la trame si pas de nouveau bytes recus */

 struct TRAME_ENOCEAN                                                   /* Definition d'une trame ENOCEAN */
  { guchar data_length_msb;
    guchar data_length_lsb;
    guchar optional_data_length;
    guchar packet_type;
    guchar crc_header;
    guchar data[80];
  };

 struct ENOCEAN_CONFIG
  { struct PROCESS *lib;
    gchar port[80];
    gint fd;                                               /* File descripteur de la connexion au ENOCEAN */
    gboolean enable;                                                           /* Thread enable at boot ? */
    gboolean reload;
    GSList *Modules_ENOCEAN;                                                /* Listes des modules ENOCEAN */
    GSList *Liste_sortie;                                              /* Liste des sorties a positionner */
    gint  comm_status;
    gint  date_last_view;
    gint  date_retry_connect;
    gint  nbr_oct_lu;
    gint  index_bute;
  };

 enum
  { ENOCEAN_CONNECT,
    ENOCEAN_WAIT_FOR_SYNC,
    ENOCEAN_WAIT_FOR_HEADER,
    ENOCEAN_WAIT_FOR_DATA,
    ENOCEAN_DISCONNECT,
    ENOCEAN_WAIT_BEFORE_RECONNECT,
  };
/*************************************** Définitions des prototypes ***************************************/
 extern gboolean Enocean_Lire_config ( void );
#endif
/*--------------------------------------------------------------------------------------------------------*/
