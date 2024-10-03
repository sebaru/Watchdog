/******************************************************************************************************************************/
/* Watchdogd/Include/Dmx.h   Header et constantes des modules DMX Watchdog 2.0                                                */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                22.10.2019 23:14:43 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dmx.h
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

#ifndef _DMX_H_
 #define _DMX_H_

 #include <json-glib/json-glib.h>

 enum
  { DMX_Reprogram_Firmware_Request = 1,
    DMX_Program_Flash_Page_Request = 2,
    DMX_Program_Flash_Page_Reply   = 2,
    DMX_Get_Widget_Parameters_Request = 3,
    DMX_Get_Widget_Parameters_Reply   = 3,
    DMX_Set_Widget_Parameters_Request = 4,
    DMX_Received_DMX_Packet = 5,
    DMX_Output_Only_Send_DMX_Packet_Request = 6,
    DMX_Send_RDM_Packet_Request = 7,
    DMX_Receive_DMX_on_Change = 8,
    DMX_Received_DMX_Change_Of_State_Packet = 9,
    DMX_Get_Widget_Serial_Number_Request = 10,
    DMX_Get_Widget_Serial_Number_Reply   = 10,
    DMX_Send_RDM_Discovery_Request = 11,
  };

 #define DMX_RETRY_DELAI 100                                             /* 10 secondes entre chaque retry si pb de connexion */
 #define DMX_CHANNEL     512

 struct TRAME_DMX                                                                               /* Definition d'une trame DMX */
  { guchar start_delimiter;                                                            /* Start of message delimiter, hex 7E. */
    guchar label;                                                                       /* Label to identify type of message. */
    guchar length_lsb;                                            /* Data length LSB. Valid range for data length is 0 to 600 */
    guchar length_msb;                                            /* Data length MSB. Valid range for data length is 0 to 600 */
    guchar start_code;                                                                 /* 1 Start Code + 24 canaux minimums ! */
    guchar channel[DMX_CHANNEL];                                                       /* 1 Start Code + 24 canaux minimums ! */
    guchar end_delimiter;                                        /* (0xE7). Ne pas oublier le end_delimiter en fin de trame ! */
  };

 struct DMX_VARS                                                                         /* Communication entre DLS et la DMX */
  { gint nbr_request;                                                                        /* Nombre de requete par seconde */
    gint fd;                                                                       /* File Descriptor d'accès au port USB DMX */
    gint date_next_retry;                                                                              /* Date de reconnexion */
    struct DLS_AO Canal[DMX_CHANNEL];                                          /* Tableau dynamique d'accès aux bits internes */
    gint taille_trame_dmx;
    struct TRAME_DMX Trame_dmx;
  };

/****************************************************** Déclaration des prototypes ********************************************/
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
