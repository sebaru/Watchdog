/**********************************************************************************************************/
/* Include/Reseaux_admin.h:   Sous_tag de admin pour watchdog 2.0 par lefevre Sebastien                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                    lun. 24 déc. 2012 12:23:12 CET  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_admin.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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

#ifndef _RESEAUX_ADMIN_H_
 #define _RESEAUX_ADMIN_H_

 #define NBR_CARAC_BUFFER_ADMIN         2048
 #define NBR_CARAC_BUFFER_ADMIN_UTF8    (2*NBR_CARAC_BUFFER_ADMIN)

 struct CMD_TYPE_ADMIN
  { gchar  buffer[NBR_CARAC_BUFFER_ADMIN_UTF8+1];
  };

 enum 
  { SSTAG_CLIENT_WANT_PAGE_ADMIN,                                      /* Le client demande la page admin */
    SSTAG_SERVEUR_CREATE_PAGE_ADMIN_OK,              /* Les serveur repond OK pour la creation de la page */
    SSTAG_CLIENT_REQUEST,                                 /* Le client envoi une requete d'administration */
    SSTAG_SERVEUR_RESPONSE_START,                                    /* Le serveur envoi la reponse texte */
    SSTAG_SERVEUR_RESPONSE_BUFFER,                                   /* Le serveur envoi la reponse texte */
    SSTAG_SERVEUR_RESPONSE_STOP,                                     /* Le serveur envoi la reponse texte */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

