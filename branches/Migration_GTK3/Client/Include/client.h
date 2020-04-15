/******************************************************************************************************************************/
/* Client/Include/client.h      D�clarations g�n�rale watchdog client                                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          mar 03 jun 2003 10:39:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * client.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - S�bastien Lefevre
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

 #include "Reseaux.h"
 #include "Config_cli.h"

 #define PROGRAMME          "Watchdog-client"

 #define EXIT_ERREUR       -1                                                                   /* Sortie sur erreur inconnue */
 #define EXIT_OK           0                                                                                /* Sortie normale */
 #define EXIT_INACTIF      1                                                                 /* Un fils est mort d'inactivit� */

 #define REPERTOIR_CONF    ".watchdog"                             /* Repertoire ou sont stock� les fichiers de configuration */
 #define FICHIER_VERSION   "icone_version.dat"
 
 enum
  { DISCONNECTED,
    ATTENTE_INTERNAL,
    ATTENTE_AUTORISATION,
    CONNECTE,
    VALIDE,
  };

 struct CLIENT
  { guint32 mode;
    GList *gids;
    struct REZO_CLI_IDENT ident;
    struct CONNEXION *connexion;
    SSL_CTX *ssl_ctx;
    X509 *srv_certif;
    X509 *cli_certif;
    gboolean ssl_needed;
    gboolean ssl_needed_with_cert;
    gchar host[TAILLE_NOM_SERVEUR+1];                                               /* Nom du serveur sur lequel se connecter */
    struct CMD_TYPE_UTILISATEUR util;
  };
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
