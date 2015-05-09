/**********************************************************************************************************/
/* Include/Reseaux_satellite.h:   Sous_tag de satellite pour watchdog 2.0 par lefevre Sebastien           */
/* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 04 oct. 2014 16:35:50 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_satellite.h
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

#ifndef _RESEAUX_SATELLITE_H_
 #define _RESEAUX_SATELLITE_H_

 enum MSRV_EVENT_TYPE
  { EVENT_INPUT,
    EVENT_OUTPUT
  };

 struct CMD_TYPE_MSRV_EVENT
  { guint type;
    gchar instance[12];
    gchar thread[12];
    gchar objet[60];
    gfloat val_float;
  };

 enum 
  { SSTAG_CLIENT_SAT_SET_INTERNAL,                                      /* Le client demande le positionnement de bit interne */
    SSTAG_SSRV_SAT_SET_INTERNAL,                                                 /* Le serveur envoi un evenement au client ! */
  };

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/

