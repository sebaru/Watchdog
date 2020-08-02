/**********************************************************************************************************/
/* Include/Reseaux_gtk_message.h:   Type de message utilisé pour watchdog 2.0   par lefevre Sebastien     */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_gtk_message.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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

#ifndef _RESEAUX_GTK_MESSAGE_H_
 #define _RESEAUX_GTK_MESSAGE_H_

 #define NBR_CARAC_MSGERREUR         512                        /* 80 caractères de messages d'erreur max */
 #define NBR_CARAC_MSGERREUR_UTF8    (2*NBR_CARAC_MSGERREUR)

 struct CMD_GTK_MESSAGE
  { gchar message[NBR_CARAC_MSGERREUR_UTF8+1];
  };

 enum 
  { SSTAG_SERVEUR_ERREUR,
    SSTAG_SERVEUR_WARNING,
    SSTAG_SERVEUR_INFO,
    SSTAG_SERVEUR_NBR_ENREG,                                     /* Envoi du nombre d'objet a telecharger */    
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/
