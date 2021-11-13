/******************************************************************************************************************************/
/* Client/Include/client.h      Déclarations générale watchdog client                                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          mar 03 jun 2003 10:39:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * client.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sébastien Lefevre
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

 #ifndef _CLIENT_H_
 #define _CLIENT_H_
 #include <glib.h>
 #include <gtk/gtk.h>
 #include <libsoup/soup.h>

 #include "Reseaux.h"

 #define REPERTOIR_CONF    ".watchdog"                             /* Repertoire ou sont stocké les fichiers de configuration */

/* enum
  { DISCONNECTED,
    ATTENTE_INTERNAL,
    ATTENTE_AUTORISATION,
    CONNECTE,
    VALIDE,
  };*/

 struct CLIENT
  { GSettings *settings;
    GtkWidget *window;
    GtkWidget *Notebook;
    GSList *Liste_pages;
    GtkWidget *Liste_histo;
    GtkWidget *Liste_plugin_dls;
    GtkWidget *Liste_synoptique;
    GtkWidget *Barre_pulse;
    GtkWidget *Barre_progress;
    GtkWidget *Entry_status;
    gchar wtd_session[42];
    gint network_size_sent;
    gint network_size_to_send;
    gint access_level;
    SoupSession *connexion;
    SoupWebsocketConnection *ws_msgs;
    gchar hostname[32];                                                             /* Nom du serveur sur lequel se connecter */
    gchar username[32];
    gchar password[32];
  };
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
