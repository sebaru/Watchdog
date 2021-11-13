/******************************************************************************************************************************/
/* Watchdog/db.c          Gestion des connexions à la base de données                                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          sam 18 avr 2009 00:44:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * db.c
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

 #include <glib.h>
 #include <string.h>

/**************************************************** Chargement des prototypes ***********************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Normaliser_chaine: Normalise les chaines ( remplace ' par \', " par "" )                                                   */
/* Entrées: un commentaire (gchar *)                                                                                          */
/* Sortie: boolean false si probleme                                                                                          */
/******************************************************************************************************************************/
 gchar *Normaliser_chaine( gchar *pre_comment )
  { gchar *comment, *source, *cible;
    gunichar car;

    g_utf8_validate( pre_comment, -1, NULL );                                                           /* Validate la chaine */
    comment = g_try_malloc0( (2*g_utf8_strlen(pre_comment, -1))*6 + 1 );                  /* Au pire, ts les car sont doublés */
                                                                                                      /* *6 pour gerer l'utf8 */
    if (!comment)
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "Normaliser_chaine: memory error %s", pre_comment );
       return(NULL);
     }
    source = pre_comment;
    cible  = comment;
    
    while( (car = g_utf8_get_char( source )) )
     { if ( car == '\'' )                                                                     /* Dédoublage de la simple cote */
        { g_utf8_strncpy( cible, "\'", 1 ); cible = g_utf8_next_char( cible );
          g_utf8_strncpy( cible, "\'", 1 ); cible = g_utf8_next_char( cible );
        }
       else if (car =='\\')                                                                        /* Dédoublage du backspace */
        { g_utf8_strncpy( cible, "\\", 1 ); cible = g_utf8_next_char( cible );
          g_utf8_strncpy( cible, "\\", 1 ); cible = g_utf8_next_char( cible );
        }
       else
        { g_utf8_strncpy( cible, source, 1 ); cible = g_utf8_next_char( cible );
        }
       source = g_utf8_next_char(source);
     }
    return(comment);
  }
/******************************************************************************************************************************/
/* Init_DB_SQL: essai de connexion à la DataBase db                                                                           */
/* Entrée: toutes les infos necessaires a la connexion                                                                        */
/* Sortie: une structure DB de référence                                                                                      */
/******************************************************************************************************************************/
 static struct DB *Init_DB_SQL_with ( gchar *host, gchar *username, gchar *password, gchar *database, guint port )
  { static gint id = 1, taille;
    struct DB *db;
    my_bool reconnect;

    db = (struct DB *)g_try_malloc0( sizeof(struct DB) );
    if (!db)                                                                              /* Si probleme d'allocation mémoire */
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: Memory error", __func__ );
       return(NULL);
     }

    db->mysql = mysql_init(NULL);
    if (!db->mysql)
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: Mysql_init failed (%s)", __func__,
                              (char *) mysql_error(db->mysql)  );
       g_free(db);
       return (NULL);
     }

    reconnect = 1;
    mysql_options( db->mysql, MYSQL_OPT_RECONNECT, &reconnect );
    mysql_options( db->mysql, MYSQL_SET_CHARSET_NAME, (void *)"utf8" );
    if ( ! mysql_real_connect( db->mysql, host, username, password, database, port, NULL, 0 ) )
     { Info_new( Config.log, Config.log_db, LOG_ERR,
                 "%s: mysql_real_connect failed (%s)", __func__,
                 (char *) mysql_error(db->mysql)  );
       mysql_close( db->mysql );
       g_free(db);
       return (NULL);
     }
    db->free = TRUE;
    pthread_mutex_lock ( &Partage->com_db.synchro );
    db->id = id++;
    Partage->com_db.Liste = g_slist_prepend ( Partage->com_db.Liste, db );
    taille = g_slist_length ( Partage->com_db.Liste );
    pthread_mutex_unlock ( &Partage->com_db.synchro );
    Info_new( Config.log, Config.log_db, LOG_DEBUG,
              "%s: Database Connection OK with %s@%s:%d on %s (DB%07d). Nbr_requete_en_cours=%d", __func__,
               username, host, port, database, db->id, taille );
    return(db);
  }
/******************************************************************************************************************************/
/* Init_DB_SQL: essai de connexion à la DataBase db                                                                           */
/* Entrée: toutes les infos necessaires a la connexion                                                                        */
/* Sortie: une structure DB de référence                                                                                      */
/******************************************************************************************************************************/
 struct DB *Init_DB_SQL ( void )
  { return( Init_DB_SQL_with ( Config.db_host, Config.db_username,
                               Config.db_password, Config.db_database, Config.db_port ) );
  }
/******************************************************************************************************************************/
/* Init_DB_SQL: essai de connexion à la DataBase db                                                                           */
/* Entrée: toutes les infos necessaires a la connexion                                                                        */
/* Sortie: une structure DB de référence                                                                                      */
/******************************************************************************************************************************/
 struct DB *Init_ArchDB_SQL ( void )
  { return( Init_DB_SQL_with ( Partage->com_arch.archdb_host, Partage->com_arch.archdb_username,
                               Partage->com_arch.archdb_password, Partage->com_arch.archdb_database, Partage->com_arch.archdb_port ) );
  }
/******************************************************************************************************************************/
/* Libere_DB_SQL : Se deconnecte d'une base de données en parametre                                                           */
/* Entrée: La DB                                                                                                              */
/******************************************************************************************************************************/
 void Libere_DB_SQL( struct DB **adr_db )
  { struct DB *db, *db_en_cours;
    gboolean found;
    GSList *liste;
    gint taille;
    if ( (!adr_db) || !(*adr_db) ) return;

    db = *adr_db;                                                                       /* Récupération de l'adresse de la db */
    found = FALSE;
    pthread_mutex_lock ( &Partage->com_db.synchro );
    liste = Partage->com_db.Liste;
    while ( liste )
     { db_en_cours = (struct DB *)liste->data;
       if (db_en_cours->id == db->id) found = TRUE;
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock ( &Partage->com_db.synchro );

    if (!found)
     { Info_new( Config.log, Config.log_db, LOG_CRIT,
                "Libere_DB_SQL: DB Free Request not in list ! DB%07d, request=%s",
                 db->id, db->requete );
       return;
     }

    if (db->free==FALSE)
     { Liberer_resultat_SQL ( db ); }
    mysql_close( db->mysql );
    pthread_mutex_lock ( &Partage->com_db.synchro );
    Partage->com_db.Liste = g_slist_remove ( Partage->com_db.Liste, db );
    taille = g_slist_length ( Partage->com_db.Liste );
    pthread_mutex_unlock ( &Partage->com_db.synchro );
    Info_new( Config.log, Config.log_db, LOG_DEBUG,
             "Libere_DB_SQL: Deconnexion effective (DB%07d), Nbr_requete_en_cours=%d", db->id, taille );
    g_free( db );
    *adr_db = NULL;
    SEA ( NUM_EA_SYS_DBREQUEST_SIMULT, taille );                                            /* Enregistrement pour historique */
  }
/******************************************************************************************************************************/
/* Lancer_requete_SQL : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 gboolean Lancer_requete_SQL ( struct DB *db, gchar *requete )
  { gint top;
    if (!db) return(FALSE);

    if (db->free==FALSE)
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "Lancer_requete_SQL (DB%07d): Reste un result a FREEer!", db->id );
     }

    g_snprintf( db->requete, sizeof(db->requete), "%s", requete );                                      /* Save for later use */
    Info_new( Config.log, Config.log_db, LOG_DEBUG,
             "Lancer_requete_SQL (DB%07d): NEW    (%s)", db->id, requete );
    top = Partage->top;
    if ( mysql_query ( db->mysql, requete ) )
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "Lancer_requete_SQL (DB%07d): FAILED (%s) for '%s'",
                 db->id, (char *)mysql_error(db->mysql), requete );
       return(FALSE);
     }

    if ( ! strncmp ( requete, "SELECT", 6 ) )
     { db->result = mysql_store_result ( db->mysql );
       db->free = FALSE;
       if ( ! db->result )
        { Info_new( Config.log, Config.log_db, LOG_WARNING,
                   "Lancer_requete_SQL (DB%07d): store_result failed (%s)",
                    db->id, (char *) mysql_error(db->mysql) );
          db->nbr_result = 0;
        }
       else 
        { /*Info( Config.log, DEBUG_DB, "Lancer_requete_SQL: store_result OK" );*/
          db->nbr_result = mysql_num_rows ( db->result );
        }
     }
    Info_new( Config.log, Config.log_db, LOG_DEBUG,
             "Lancer_requete_SQL (DB%07d): OK in %05.1fs", db->id, (Partage->top - top)/10.0 );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Liberer_resultat_SQL: Libere la mémoire affectée au resultat SQL                                                           */
/* Entrée: la DB                                                                                                              */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Liberer_resultat_SQL ( struct DB *db )
  { if (db)
     { while( db->row ) Recuperer_ligne_SQL ( db );
       mysql_free_result( db->result );
       db->result = NULL;
       db->free = TRUE;
      /*Info( Config.log, DEBUG_DB, "Liberer_resultat_SQL: free OK" );*/
     }
  }
/******************************************************************************************************************************/
/* Recuperer_ligne_SQL: Renvoie les lignes resultat, une par une                                                              */
/* Entrée: la DB                                                                                                              */
/* Sortie: La ligne ou NULL si il n'y en en plus                                                                              */
/******************************************************************************************************************************/
 MYSQL_ROW Recuperer_ligne_SQL ( struct DB *db )
  { if (!db) return(NULL);
    db->row = mysql_fetch_row(db->result);
    return( db->row );
  }
/******************************************************************************************************************************/
/* Recuperer_last_ID_SQL: Renvoie le dernier ID inséré                                                                        */
/* Entrée: la DB                                                                                                              */
/* Sortie: Le dernier ID                                                                                                      */
/******************************************************************************************************************************/
 guint Recuperer_last_ID_SQL ( struct DB *db )
  { if (!db) return(0);
    return ( mysql_insert_id(db->mysql) );
  }
/******************************************************************************************************************************/
/* Print_SQL_status : permet de logguer le statut SQL                                                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Print_SQL_status ( void )
  { GSList *liste;
    pthread_mutex_lock ( &Partage->com_db.synchro );
    Info_new( Config.log, Config.log_db, LOG_DEBUG,
             "Print_SQL_status: %03d running connexions",
              g_slist_length(Partage->com_db.Liste) );

    liste = Partage->com_db.Liste;
    while ( liste )
     { struct DB *db;
       db = (struct DB *)liste->data;
       Info_new( Config.log, Config.log_db, LOG_DEBUG,
                "Print_SQL_status: Connexion DB%07d (db->free=%d) requete %s",
                 db->id, db->free, db->requete );
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock ( &Partage->com_db.synchro );
  }
/******************************************************************************************************************************/
/* Update_database_schema: Vérifie la connexion et le schéma de la base de données                                            */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Update_database_schema ( void )
  { gchar chaine[32], requete[4096];
    gint database_version;
    gchar *nom, *valeur;
    struct DB *db;

    if (Config.instance_is_master != TRUE)                                                  /* Do not update DB if not master */
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "Update_database_schema: Instance is not master. Don't update schema." );
       return;
     }

    database_version = 0;                                                                                /* valeur par défaut */
    if ( ! Recuperer_configDB( &db, "global" ) )                                            /* Connexion a la base de données */
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "Update_database_schema: Database connexion failed" );
       return;
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Config.log_db, LOG_INFO,                                                         /* Print Config */
                "Update_database_schema: found global param '%s' = %s in DB", nom, valeur );
       if ( ! g_ascii_strcasecmp ( nom, "database_version" ) )
        { database_version = atoi( valeur ); }
     }

    Info_new( Config.log, Config.log_db, LOG_NOTICE,
             "Update_database_schema: Actual Database_Version detected = %05d", database_version );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_ERR, "Update_database_schema: DB connexion failed" );
       return;
     }

    if (database_version < 2500)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE users DROP `imsg_bit_presence`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE users ADD `ssrv_bit_presence` INT NOT NULL DEFAULT '0'"
                 );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2510)
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                  "INSERT INTO `mnemos` (`id`, `type`, `num`, `num_plugin`, `acronyme`, `libelle`, `command_text`) VALUES"
                  "(23, 3,9999, 1, 'EVENT_NONE_TOR', 'Used for detected Event with no mapping yet.', ''),"
                  "(24, 5,9999, 1, 'EVENT_NONE_ANA', 'Used for detected Event with no mapping yet.', '')"
                 );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2532)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE eana TO mnemos_AnalogInput" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), 
                  "CREATE TABLE `mnemos_DigitalInput`"
                  "(`id_mnemo` int(11) NOT NULL, `furtif` int(1) NOT NULL, PRIMARY KEY (`id_mnemo`)"
                  ") ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
                 );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2541)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE dls_cpt_imp TO mnemos_CptImp" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2543)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE tempo TO mnemos_Tempo" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2571)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `users` ADD `imsg_available` TINYINT NOT NULL AFTER `imsg_allow_cde`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2573)
     { g_snprintf( requete, sizeof(requete), "INSERT INTO `mnemos` "
                   "(`id`, `type`, `num`, `num_plugin`, `acronyme`, `libelle`, `command_text`) VALUES "
                   "(25, 5, 122, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(26, 5, 121, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(27, 5, 120, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(28, 5, 119, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(29, 5, 118, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(30, 5, 117, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(31, 5, 116, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(32, 5, 115, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(33, 5, 114, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(34, 5, 113, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(35, 5, 112, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(36, 5, 111, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(37, 5, 110, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(38, 5, 109, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(39, 5, 108, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(40, 5, 107, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(41, 5, 106, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(42, 5, 105, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(43, 5, 104, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(44, 5, 103, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(45, 5, 102, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(46, 5, 101, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(47, 5, 100, 1, 'SYS_RESERVED', 'Reserved for internal use', '');"
                 );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2581)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` CHANGE `min_e_tor` `map_E` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` CHANGE `min_e_ana` `map_EA` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` CHANGE `min_s_tor` `map_A` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` CHANGE `min_s_ana` `map_AA` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2582)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `rfxcom` CHANGE `e_min` `map_E` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `rfxcom` CHANGE `ea_min` `map_EA` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `rfxcom` CHANGE `a_min` `map_A` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2583)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE onduleurs TO ups" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `ups` CHANGE `e_min` `map_E` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `ups` CHANGE `ea_min` `map_EA` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `ups` CHANGE `a_min` `map_A` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2669)
     { g_snprintf( requete, sizeof(requete), "DROP TABLE rfxcom" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2696)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `dls` ADD `compil_date` int(11) NOT NULL AFTER `actif`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `dls` ADD `compil_status` int(11) NOT NULL AFTER `compil_date`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2728)
     { g_snprintf( requete, sizeof(requete),
      "ALTER TABLE `config` CHANGE `instance_id` `instance_id` VARCHAR(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete),
      "ALTER TABLE `config` CHANGE `nom_thread` `nom_thread` VARCHAR(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete),
      "ALTER TABLE `config` CHANGE `nom` `nom` VARCHAR(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete),
      "ALTER TABLE `config` ADD PRIMARY KEY( `instance_id`, `nom_thread`, `nom`);" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2743)
     { g_snprintf( requete, sizeof(requete), "INSERT INTO `mnemos` "
                   "(`id`, `type`, `num`, `num_plugin`, `acronyme`, `libelle`, `command_text`) VALUES "
                   "(79, 0, 39, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(80, 0, 38, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(81, 0, 37, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(82, 0, 36, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(83, 0, 35, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(84, 0, 34, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(85, 0, 33, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(86, 0, 32, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(87, 0, 31, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(88, 0, 30, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(89, 0, 29, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(90, 0, 28, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(91, 0, 27, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(92, 0, 26, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(93, 0, 25, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(94, 0, 24, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(95, 0, 23, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(96, 0, 22, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(97, 0, 21, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(98, 0, 20, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(99, 0, 19, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(100, 0, 18, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(101, 0, 17, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(102, 0, 16, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(103, 0, 15, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(104, 0, 14, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(105, 0, 13, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(106, 0, 12, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(107, 0, 11, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(108, 0, 10, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),"
                   "(109, 0, 09, 1, 'SYS_RESERVED', 'Reserved for internal use', '');"
                 );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2748)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_DigitalInput` DROP COLUMN `furtif`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2847)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE dls_cpth TO mnemos_CptHoraire" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2850)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_CptHoraire` CHANGE `val` `valeur` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2857)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos` ADD `acro_syn` text NOT NULL AFTER `tableau`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2871)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_CptImp` CHANGE `val` `valeur`  float NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2909)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls ADD `shortname` text NOT NULL AFTER `name`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "UPDATE dls SET shortname='Systeme' WHERE id=1" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2911)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD `id_plugin_dls` INT(11) NOT NULL AFTER `num`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "UPDATE msgs SET id_plugin_dls=1" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2912)
     { g_snprintf( requete, sizeof(requete), "UPDATE msgs INNER JOIN syns ON msgs.id_syn=syns.id INNER JOIN dls ON dls.num_syn=syns.id SET msgs.id_plugin_dls=dls.id;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs DROP COLUMN `id_syn`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2914)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls CHANGE `num_syn` `syn_id` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2915)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `id_plugin_dls` `dls_id` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2934)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE histo_msgs CHANGE `id_num` `id_msg` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2951)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE histo_bit ADD `date_time` DATETIME(6) AFTER `date_usec`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2974)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE syns ADD `vignette_activite` INT(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns ADD `vignette_secu_bien` INT(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns ADD `vignette_secu_personne` INT(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "UPDATE syns_pass INNER JOIN syns ON syns_pass.syn_cible_id = syns.id "
                                             "SET vignette_activite=bitctrl1, vignette_secu_bien=bitctrl2, "
                                             "vignette_secu_personne=bitctrl3 "
                                             "WHERE bitctrl1!=0;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_pass DROP bitctrl, DROP bitctrl1, DROP bitctrl2, DROP bitctrl3" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2991)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` ENGINE = INNODB ROW_FORMAT = DYNAMIC;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE histo_msgs ADD FOREIGN KEY fk_id_msg (`id_msg`) REFERENCES msgs(`id`)"
                                             " ON DELETE CASCADE ON UPDATE RESTRICT;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 2994)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos` ENGINE = INNODB ROW_FORMAT = DYNAMIC;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_Tempo` ENGINE = INNODB ROW_FORMAT = DYNAMIC;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_Tempo ADD FOREIGN KEY fk_id_mnemo_tempo (`id_mnemo`) REFERENCES mnemos(`id`)"
                                             " ON DELETE CASCADE ON UPDATE RESTRICT;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_DigitalInput` ENGINE = INNODB ROW_FORMAT = DYNAMIC;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_DigitalInput ADD FOREIGN KEY fk_id_mnemo_di (`id_mnemo`) REFERENCES mnemos(`id`)"
                                             " ON DELETE CASCADE ON UPDATE RESTRICT;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_AnalogInput` ENGINE = INNODB ROW_FORMAT = DYNAMIC;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_AnalogInput ADD FOREIGN KEY fk_id_mnemo_ai (`id_mnemo`) REFERENCES mnemos(`id`)"
                                             " ON DELETE CASCADE ON UPDATE RESTRICT;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_CptImp` ENGINE = INNODB ROW_FORMAT = DYNAMIC;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_CptImp ADD FOREIGN KEY fk_id_mnemo_ci (`id_mnemo`) REFERENCES mnemos(`id`)"
                                             " ON DELETE CASCADE ON UPDATE RESTRICT;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_CptHoraire` ENGINE = INNODB ROW_FORMAT = DYNAMIC;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_CptHoraire ADD FOREIGN KEY fk_id_mnemo_ch (`id_mnemo`) REFERENCES mnemos(`id`)"
                                             " ON DELETE CASCADE ON UPDATE RESTRICT;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3019)
     { g_snprintf( requete, sizeof(requete),
                  "CREATE TABLE IF NOT EXISTS `icons_new` ("
                  "`id` int(11) NOT NULL AUTO_INCREMENT,"
                  "`description` VARCHAR(160) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                  "`classe` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT '0',"
                  "KEY (`classe`),"
                  "PRIMARY KEY (`id`)"
                  ") ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3055)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `bit_voc` `bit_audio` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD `audio` TINYINT(1) NOT NULL DEFAULT '0' AFTER `bit_audio`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs DROP type_voc, DROP vitesse_voc" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3083)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE syns_capteurs TO syns_cadrans" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD `persist` TINYINT(1) NOT NULL DEFAULT '0' AFTER `enable`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }
       
    if (database_version < 3086)
     { g_snprintf( requete, sizeof(requete),
                  "CREATE TABLE IF NOT EXISTS `mnemos_Registre` ("
                  "`id_mnemo` int(11) NOT NULL,"
                  "`unite` text COLLATE utf8_unicode_ci NOT NULL,"
                  "PRIMARY KEY (`id_mnemo`),"
                  "CONSTRAINT `id_mnemo` FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE"
                  ") ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3113)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos CHANGE `num_plugin` `dls_id` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3128)
     { g_snprintf( requete, sizeof(requete), "INSERT INTO mnemos (`id`, `type`, `num`, `dls_id`, `acronyme`, `libelle`, `command_text`) VALUES "
                                             "(110, 1, 07, 1, 'SYS_EVENT_NOT_FOUND', 'Event not found', '')" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3159)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls ADD `nbr_compil` INT(11) NOT NULL DEFAULT '0' AFTER `compil_status`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3178)
     { g_snprintf( requete, sizeof(requete), "CREATE TABLE IF NOT EXISTS `syns_scenario` ("
                                             "`id` int(11) NOT NULL AUTO_INCREMENT,"
                                             "`id_syn` int(11) NOT NULL DEFAULT '0',"
                                             "`libelle` text COLLATE utf8_unicode_ci NOT NULL,"
                                             "`posx` int(11) NOT NULL DEFAULT '0',"
                                             "`posy` int(11) NOT NULL DEFAULT '0',"
                                             "`angle` float NOT NULL DEFAULT '0',"
                                             "PRIMARY KEY (`id`),"
                                             "CONSTRAINT `id_syn` FOREIGN KEY (`id_syn`) REFERENCES `syns` (`id`) ON DELETE CASCADE"
                                             ") ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3209)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs CHANGE `syn` `syn_id` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3215)
     { g_snprintf( requete, sizeof(requete), "CREATE TABLE IF NOT EXISTS `syns_camerasup` ("
                                             "`id` int(11) NOT NULL AUTO_INCREMENT,"
                                             "`syn_id` int(11) NOT NULL,"
                                             "`num` int(11) NOT NULL,"
                                             "`posx` int(11) NOT NULL,"
                                             "`posy` int(11) NOT NULL,"
                                             "PRIMARY KEY (`id`),"
                                             "CONSTRAINT `id_syn`    FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE"
                                             ") ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;"
                                             "ALTER TABLE syns_motifs CHANGE `syn` `syn_id` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3247)
     { g_snprintf( requete, sizeof(requete), "CREATE TABLE IF NOT EXISTS `scenario_ticks` ("
                                             "`id` int(11) NOT NULL,"
                                             "`num` int(11) NOT NULL,"
                                             "`minute` int(11) NOT NULL,"
                                             "`heure` int(11) NOT NULL,"
                                             "`jour` int(11) NOT NULL,"
                                             "`date` int(11) NOT NULL,"
                                             "`mois` int(11) NOT NULL,"
                                             "`mnemo_id` int(11) NOT NULL,"
                                             "PRIMARY KEY (`id`), KEY(`num`),"
                                             "CONSTRAINT `num` FOREIGN KEY (`num`) REFERENCES `syns_scenario` (`id`) ON DELETE CASCADE,"
                                             "CONSTRAINT `mnemo_id` FOREIGN KEY (`mnemo_id`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE"
                                             ") ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3252)
     { g_snprintf( requete, sizeof(requete), "INSERT INTO mnemos (`id`, `type`, `num`, `dls_id`, `acronyme`, `libelle`, `command_text`) VALUES "
                                             "(111, 1,  8, 1, 'SYS_NEW_TICK', 'Default Command by Tick', '');" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3310)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD `is_mp3` TINYINT(1) NOT NULL DEFAULT '0' AFTER `time_repeat`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3348)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE users ADD `phphash` VARCHAR(130) NOT NULL AFTER `hash`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3353)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE users ADD `access_level` INT(11) NOT NULL AFTER `enable`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "UPDATE users SET phphash='$2y$10$9TVOoxmzBJTl6knJ0plKHOCsoSvSSMiPrldhanBKVApFIF3083x6a' WHERE id=0" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "UPDATE users SET access_level=10 WHERE id=0;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns CHANGE `access_groupe` `access_level` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs CHANGE `gid` `access_level` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "DROP TABLE 'groups'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE users CHANGE `date_modif` `date_modif` DATETIME NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE users CHANGE `date_expire` `date_expire` DATETIME NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE users CHANGE `date_create` `date_create` DATETIME NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3380)
     { g_snprintf( requete, sizeof(requete), "CREATE TABLE `users_sessions` ("
                                             "`id` varchar(128) NOT NULL,"
                                             "`login` varchar(32) NOT NULL,"
                                             "`last_date` datetime NOT NULL,"
                                             "`remote_addr` varchar(50) NOT NULL,"
                                             "`x_forwarded_for` varchar(50) NOT NULL,"
                                             "`data` text NOT NULL,"
                                             "PRIMARY KEY (`id`)"
                                             ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
                 );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3386)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls ADD `sourcecode` MEDIUMTEXT NOT NULL AFTER `nbr_compil`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version < 3444)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE modbus_modules ADD `date_create` DATETIME NOT NULL AFTER `id`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE modbus_modules DROP `instance_id`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version < 3448)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE rs485 CHANGE `instance_id` `host` TEXT NOT NULL" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE rs485 ADD `date_ajout` DATETIME AFTER `host`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version < 3463)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos ADD `ev_host` VARCHAR(40) NOT NULL AFTER `libelle`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos ADD `ev_thread` VARCHAR(20) NOT NULL AFTER `ev_host`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos CHANGE `command_text` `ev_text` VARCHAR(160) NOT NULL" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version < 3470)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE modbus_modules ADD `max_nbr_E` INT(11) NOT NULL AFTER `map_E`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version < 3490)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE rs485 ADD `forced_e_min` INT(11) NOT NULL DEFAULT '0' AFTER `e_min`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version < 3521)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns` ENGINE = INNODB" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_camerasup` ENGINE = INNODB" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_scenario` ENGINE = INNODB" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_cadrans` ENGINE = INNODB" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_comments` ENGINE = INNODB" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_motifs` ENGINE = INNODB" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_palettes` ENGINE = INNODB" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns ADD `parent_id` INT(11) NOT NULL AFTER `id`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns` ADD CONSTRAINT `fk_parent_id` FOREIGN KEY (`parent_id`) REFERENCES `syns`(`id`) ON DELETE CASCADE ON UPDATE RESTRICT;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns` DROP `groupe`");
       Lancer_requete_SQL ( db, requete );
     }

    database_version=3521;

    Libere_DB_SQL(&db);

    g_snprintf( chaine, sizeof(chaine), "%d", database_version );
    if (Modifier_configDB ( "global", "database_version", chaine ))
     { Info_new( Config.log, Config.log_db, LOG_NOTICE,
                "Update_database_schema: updating Database_version to %s OK", chaine );
     }
    else
     { Info_new( Config.log, Config.log_db, LOG_NOTICE,
                "Update_database_schema: updating Database_version to %s FAILED", chaine );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
