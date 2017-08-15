/******************************************************************************************************************************/
/* Watchdogd/Archive/archiveDB.c       Déclaration des fonctions pour la gestion des valeurs analogiques                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         mer. 09 mai 2012 12:45:53 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * archiveDB.c
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
 
 #include <glib.h>
 #include <sys/time.h>
 #include <sys/prctl.h>
 
 #include "watchdogd.h"
/******************************************************************************************************************************/
/* Ajouter_archDB: Ajout d'une entree archive dans la Base de Données                                                         */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure arch                                              */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 void Ajouter_archDB ( struct DB *db, struct ARCHDB *arch )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "CREATE TABLE IF NOT EXISTS `%s_%03d_%06d`("
                "`date_time` datetime(6) DEFAULT NULL,"
                "`valeur` float NOT NULL DEFAULT '0',"
                "KEY `index_date` (`date_time`)"
                ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
                "  PARTITION BY KEY (date_time) PARTITIONS 52;",
                NOM_TABLE_ARCH, arch->type, arch->num );
    Lancer_requete_SQL ( db, requete );                                                        /* Execution de la requete SQL */

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s_%03d_%06d(date_time,valeur) VALUES "
                "(FROM_UNIXTIME(%d.%d),'%f')",
                NOM_TABLE_ARCH, arch->type, arch->num, arch->date_sec, arch->date_usec, arch->valeur );

    Lancer_requete_SQL ( db, requete );                                                        /* Execution de la requete SQL */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
