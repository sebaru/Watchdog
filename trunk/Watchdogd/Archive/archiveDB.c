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
/******************************************************************************************************************************/
/* Arch_Update_SQL_Partitions: Appelé une fois par jour pour faire des opérations de menage dans les tables d'archivages      */
/* Entrée: néant                                                                                                              */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 void Arch_Update_SQL_Partitions_thread ( void )
  { gchar requete[512];
	GSList *Liste_tables;
    struct DB *db;
    prctl(PR_SET_NAME, "W-ArchSQL", 0, 0, 0 );
    Liste_tables = NULL;

    db = Init_ArchDB_SQL();      
    if (!db)
     { Info_new( Config.log, Config.log_arch, LOG_ERR,
                "%s: Unable to open database %s", __func__, Config.archdb_database );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT table_name FROM information_schema.tables WHERE table_schema='%s' "
                "AND table_name like 'histo_bit_%", Config.archdb_database );
    if (Lancer_requete_SQL ( db, requete )==FALSE)                                             /* Execution de la requete SQL */
     { Libere_DB_SQL(&db);
	   Info_new( Config.log, Config.log_arch, LOG_ERR,
                "%s: Searching table names failed", __func__ );
       return;
     }

    while ( Recuperer_ligne_SQL(db) )                                                      /* Chargement d'une ligne resultat */
     { Liste_tables = g_slist_prepend ( Liste_tables, strdup(db->row[0]) ); }
    Libere_DB_SQL(&db);

    db = Init_ArchDB_SQL();      
    if (!db)
     { Info_new( Config.log, Config.log_arch, LOG_ERR,
                "%s: Unable to open database %s for deleting", __func__, Config.archdb_database );
       while (Liste_tables)
        { gchar *table;
	      table = Liste_tables->data;
	      Liste_tables = g_slist_remove ( Liste_tables, table );
	      g_free(table);
        }
       return;
     }

    while (Liste_tables)
     { gchar *table;
	   table = Liste_tables->data;
	   Liste_tables = g_slist_remove ( Liste_tables, table );
	   g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                  "DELETE FROM %s WHERE date_sec < NOW() - INTERVAL 400 DAY", table );
       if (Lancer_requete_SQL ( db, requete )==FALSE)                                          /* Execution de la requete SQL */
        { Info_new( Config.log, Config.log_arch, LOG_ERR,
                   "%s: Unable to delete from table '%s'", __func__, table );
        }
       g_free(table);
     }
    Libere_DB_SQL(&db);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
