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

 #include "watchdogd.h"
/******************************************************************************************************************************/
/* Ajouter_archDB: Ajout d'une entree archive dans la Base de Données                                                         */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure arch                                              */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 void Ajouter_archDB ( struct DB *db, struct ARCHDB *arch )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s(date_time,type,num,valeur) VALUES "
                "(FROM_UNIXTIME(%d.%d),%d,%d,'%f')", NOM_TABLE_ARCH, arch->date_sec, arch->date_usec,
                arch->type, arch->num, arch->valeur );

    Lancer_requete_SQL ( db, requete );                                                        /* Execution de la requete SQL */
  }
/******************************************************************************************************************************/
/* Arch_Update_SQL_Partitions: Ajout d'une partition dans la table histo_bit                                                  */
/* Entrée: la date de la nouvelle partition                                                                                   */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 void Arch_Update_SQL_Partitions ( guint annee, guint mois )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, 
                "Arch_Update_SQL_Partitions: Unable to open database %s", Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "ALTER TABLE %s REORGANIZE PARTITION p_MAX INTO "
                "(PARTITION p%04d%02d VALUES LESS THAN ('%04d-%02d-01'),"
                " PARTITION p_MAX VALUES LESS THAN MAXVALUE);",
                NOM_TABLE_ARCH, annee, mois, annee, mois );

    Lancer_requete_SQL ( db, requete );                                                        /* Execution de la requete SQL */
    Libere_DB_SQL( &db );
    Info_new( Config.log, Config.log_arch, LOG_INFO, 
             "Arch_Update_SQL_Partitions: Create NEw partition p%04d%02d", annee, mois );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
