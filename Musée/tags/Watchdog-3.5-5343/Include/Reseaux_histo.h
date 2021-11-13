/**********************************************************************************************************/
/* Include/Reseaux_histo.h:   Sous_tag de histo pour watchdog 2.0 par lefevre Sebastien                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_histo.h
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

#ifndef _RESEAUX_HISTO_H_
 #define _RESEAUX_HISTO_H_

 struct CMD_TYPE_HISTO
  { gint id;                                                                             /* id unique dans la base de données */
    gboolean alive;                                                                 /* Le message est-il encore d'actualité ? */
    struct CMD_TYPE_MESSAGE msg;
    gchar nom_ack [ NBR_CARAC_LOGIN_UTF8 + 1 ];
    gchar date_create[32];
    gchar date_fixe[32];
    gchar date_fin[32];
  };

 enum
  { SSTAG_SERVEUR_ADDPROGRESS_HISTO,
    SSTAG_SERVEUR_ADDPROGRESS_HISTO_FIN,
    SSTAG_SERVEUR_SHOW_HISTO,
    SSTAG_SERVEUR_DEL_HISTO,

    SSTAG_CLIENT_ACK_HISTO,
    SSTAG_SERVEUR_ACK_HISTO,

  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

