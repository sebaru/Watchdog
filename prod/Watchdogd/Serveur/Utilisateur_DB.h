/******************************************************************************************************************************/
/* Watchdogd/Include/Utilisateur.h     definition de la structure  MSG et des constantes prg des password                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          lun 02 jun 2003 14:50:09 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Utilisateur_DB.h
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

#ifndef _UTILISATEUR_H_
 #define _UTILISATEUR_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Config.h"

 #define NOM_TABLE_UTIL      "users"

 enum                                                                    /* Enumeration des utilisateurs spéciaux de watchdog */
  { UID_ROOT,
    UID_GUEST,
    NBR_UTILISATEUR_RESERVE
  };
                                                                                  /* Enumeration des groupes de base Watchdog */
 #define ACCESS_LEVEL_DLS       6
 #define ACCESS_LEVEL_CLI       6
 #define ACCESS_LEVEL_MSG       6
 #define ACCESS_LEVEL_USER      6
 #define ACCESS_LEVEL_SATELLITE 6
 #define ACCESS_LEVEL_ATELIER   6
 #define ACCESS_LEVEL_HISTO     6
 #define ACCESS_LEVEL_ICONE     6
 #define ACCESS_LEVEL_ALL       0


/************************************************ Prototypes de fonctions *****************************************************/
 extern gboolean Tester_level_util( struct CMD_TYPE_UTILISATEUR *util, guint id_groupe );               /* Dans Utilisateur.c */
 extern struct CMD_TYPE_UTILISATEUR *Recuperer_utilisateurDB_suite( struct DB **db );
 extern struct CMD_TYPE_UTILISATEUR *Rechercher_utilisateurDB_by_name( gchar *nom );
 extern gboolean Check_utilisateur_password( struct CMD_TYPE_UTILISATEUR *util, gchar *pwd );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
