/******************************************************************************************************************************/
/* Watchdog/db.c          Gestion des connexions à la base de données                                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 18 avr 2009 00:44:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * db.c
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
/* Normaliser_as_ascii: S'assure que la chaine en parametre respecte les caracteres d'un tech_id                              */
/* Entrées: une chaine de caractere                                                                                           */
/* Sortie: la meme chaine, avec les caracteres interdits remplacés ar '_'                                                     */
/******************************************************************************************************************************/
 gchar *Normaliser_as_ascii( gchar *chaine )
  { if (!chaine) return(NULL);
    return ( g_strcanon ( chaine, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' ) );
  }
/******************************************************************************************************************************/
/* Init_DB_SQL: essai de connexion à la DataBase db                                                                           */
/* Entrée: toutes les infos necessaires a la connexion                                                                        */
/* Sortie: une structure DB de référence                                                                                      */
/******************************************************************************************************************************/
 struct DB *Init_DB_SQL_with ( gchar *host, gchar *username, gchar *password, gchar *database, guint port, gboolean multi_statements )
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
    if ( ! mysql_real_connect( db->mysql, host, username, password, database, port, NULL, (multi_statements ? CLIENT_MULTI_STATEMENTS : 0) ) )
     { Info_new( Config.log, Config.log_db, LOG_ERR,
                 "%s: mysql_real_connect failed (%s)", __func__,
                 (char *) mysql_error(db->mysql)  );
       mysql_close( db->mysql );
       g_free(db);
       return (NULL);
     }
    db->free = TRUE;
    db->multi_statement = multi_statements;
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
  { return( Init_DB_SQL_with ( Config.db_hostname, Config.db_username,
                               Config.db_password, Config.db_database, Config.db_port, FALSE ) );
  }
/******************************************************************************************************************************/
/* Init_DB_SQL: essai de connexion à la DataBase db                                                                           */
/* Entrée: toutes les infos necessaires a la connexion                                                                        */
/* Sortie: une structure DB de référence                                                                                      */
/******************************************************************************************************************************/
 struct DB *Init_ArchDB_SQL ( void )
  { return( Init_DB_SQL_with ( Partage->com_arch.archdb_hostname, Partage->com_arch.archdb_username,
                               Partage->com_arch.archdb_password, Partage->com_arch.archdb_database, Partage->com_arch.archdb_port, FALSE ) );
  }
/******************************************************************************************************************************/
/* SQL_Select_to_JSON : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 gboolean SQL_Select_to_JSON ( JsonBuilder *builder, gchar *array_name, gchar *requete )
  { struct DB *db = Init_DB_SQL ();
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "%s: Init DB FAILED for '%s'", __func__, requete );
       return(FALSE);
     }

    if ( mysql_query ( db->mysql, requete ) )
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "%s: FAILED (%s) for '%s'", __func__, (char *)mysql_error(db->mysql), requete );
       Libere_DB_SQL ( &db );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_db, LOG_DEBUG, "%s: DB OK for '%s'", __func__, requete );

    db->result = mysql_store_result ( db->mysql );
    if ( ! db->result )
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "%s: store_result failed (%s)", __func__, (char *) mysql_error(db->mysql) );
       db->nbr_result = 0;
     }
    else
     { if (array_name)
        { gchar chaine[80];
          g_snprintf(chaine, sizeof(chaine), "nbr_%s", array_name );
          Json_add_int ( builder, chaine, mysql_num_rows ( db->result ));
          Json_add_array( builder, array_name );
        }
       while ( (db->row = mysql_fetch_row(db->result)) != NULL )
        { if (array_name) Json_add_object ( builder, NULL );
          for (gint cpt=0; cpt<mysql_num_fields(db->result); cpt++)
           { Json_add_string( builder, mysql_fetch_field_direct(db->result, cpt)->name, db->row[cpt] ); }
          if (array_name) Json_end_object ( builder );
        }
       if (array_name) Json_end_array ( builder );
       mysql_free_result( db->result );
     }
    Libere_DB_SQL( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* SQL_Select_to_JSON : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 gboolean SQL_Arch_to_JSON ( JsonBuilder *builder, gchar *array_name, gchar *requete )
  { struct DB *db = Init_ArchDB_SQL ();
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "%s: Init DB FAILED for '%s'", __func__, requete );
       return(FALSE);
     }

    if ( mysql_query ( db->mysql, requete ) )
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "%s: FAILED (%s) for '%s'", __func__, (char *)mysql_error(db->mysql), requete );
       Libere_DB_SQL ( &db );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_db, LOG_DEBUG, "%s: DB OK for '%s'", __func__, requete );


    db->result = mysql_store_result ( db->mysql );
    if ( ! db->result )
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "%s: store_result failed (%s)", __func__, (char *) mysql_error(db->mysql) );
       db->nbr_result = 0;
     }
    else
     { if (array_name)
        { gchar chaine[80];
          g_snprintf(chaine, sizeof(chaine), "nbr_%s", array_name );
          Json_add_int ( builder, chaine, mysql_num_rows ( db->result ));
          Json_add_array( builder, array_name );
        }
       while ( (db->row = mysql_fetch_row(db->result)) != NULL )
        { if (array_name) Json_add_object ( builder, NULL );
          for (gint cpt=0; cpt<mysql_num_fields(db->result); cpt++)
           { Json_add_string( builder, mysql_fetch_field_direct(db->result, cpt)->name, db->row[cpt] ); }
          if (array_name) Json_end_object ( builder );
        }
       if (array_name) Json_end_array ( builder );
       mysql_free_result( db->result );
     }
    Libere_DB_SQL( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* SQL_Select_to_JSON : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 gboolean SQL_Write ( gchar *requete )
  { struct DB *db = Init_DB_SQL ();
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: Init DB FAILED for '%s'", __func__, requete );
       return(FALSE);
     }

    if ( mysql_query ( db->mysql, requete ) )
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: FAILED (%s) for '%s'", __func__, (char *)mysql_error(db->mysql), requete );
       Libere_DB_SQL ( &db );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_db, LOG_DEBUG, "%s: DB OK for '%s'", __func__, requete );

    Libere_DB_SQL ( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* SQL_Select_to_JSON : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 gboolean SQL_Writes ( gchar *requete )
  { struct DB *db = Init_DB_SQL_with ( Config.db_hostname, Config.db_username,
                                       Config.db_password, Config.db_database, Config.db_port, TRUE );
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: Init DB FAILED for '%s'", __func__, requete );
       return(FALSE);
     }

    if ( mysql_query ( db->mysql, requete ) )
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: FAILED (%s) for '%s'", __func__, (char *)mysql_error(db->mysql), requete );
       Libere_DB_SQL ( &db );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_db, LOG_DEBUG, "%s: DB OK for '%s'", __func__, requete );

    Libere_DB_SQL ( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* SQL_Select_to_JSON : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 gboolean SQL_Arch_Write ( gchar *requete )
  { struct DB *db = Init_ArchDB_SQL ();
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: Init DB FAILED for '%s'", __func__, requete );
       return(FALSE);
     }

    if ( mysql_query ( db->mysql, requete ) )
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: FAILED (%s) for '%s'", __func__, (char *)mysql_error(db->mysql), requete );
       Libere_DB_SQL ( &db );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_db, LOG_DEBUG, "%s: DB OK for '%s'", __func__, requete );

    Libere_DB_SQL ( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Libere_DB_SQL : Se deconnecte d'une base de données en parametre                                                           */
/* Entrée: La DB                                                                                                              */
/******************************************************************************************************************************/
 void Libere_DB_SQL( struct DB **adr_db )
  { static struct DLS_AI *ai_nbr_dbrequest = NULL;
    struct DB *db, *db_en_cours;
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
    Dls_data_set_AI ( "SYS", "DB_NBR_REQUEST", (gpointer)&ai_nbr_dbrequest, taille, TRUE );
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
                "%s: (DB%07d): FAILED (%s) for '%s'", __func__, db->id, (char *)mysql_error(db->mysql), requete );
       return(FALSE);
     }

    if ( ! strncmp ( requete, "SELECT", 6 ) )
     { db->result = mysql_store_result ( db->mysql );
       db->free = FALSE;
       if ( ! db->result )
        { Info_new( Config.log, Config.log_db, LOG_WARNING,
                   "%s: (DB%07d): store_result failed (%s)", __func__, db->id, (char *) mysql_error(db->mysql) );
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
  { if (!db) return;
encore:
    while( db->row ) Recuperer_ligne_SQL ( db );
    mysql_free_result( db->result );
    if (db->multi_statement)
     { if (mysql_next_result(db->mysql)==0)
        { db->result = mysql_store_result(db->mysql);
          Recuperer_ligne_SQL ( db );
          goto encore;
        }
     }
    db->result = NULL;
    db->free = TRUE;
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
  { gint database_version;
    gchar requete[4096];
    struct DB *db;

    if (Config.instance_is_master != TRUE)                                                  /* Do not update DB if not master */
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "%s: Instance is not master. Don't update schema.", __func__ );
       return;
     }

    gchar *database_version_string = Recuperer_configDB_by_nom( "msrv", "database_version" );/* Récupération d'une config dans la DB */
    if (database_version_string)
     { database_version = atoi( database_version_string );
       g_free(database_version_string);
     } else database_version=0;

    Info_new( Config.log, Config.log_db, LOG_NOTICE,
             "%s: Actual Database_Version detected = %05d. Please wait while upgrading.", __func__, database_version );


    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    if (database_version==0) goto fin;

    if (database_version <= 2500)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE users DROP `imsg_bit_presence`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE users ADD `ssrv_bit_presence` INT NOT NULL DEFAULT '0'"
                 );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2510)
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                  "INSERT INTO `mnemos` (`id`, `type`, `num`, `num_plugin`, `acronyme`, `libelle`, `command_text`) VALUES"
                  "(23, 3,9999, 1, 'EVENT_NONE_TOR', 'Used for detected Event with no mapping yet.', ''),"
                  "(24, 5,9999, 1, 'EVENT_NONE_ANA', 'Used for detected Event with no mapping yet.', '')"
                 );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2532)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE eana TO mnemos_AnalogInput" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete),
                  "CREATE TABLE `mnemos_DigitalInput`"
                  "(`id_mnemo` int(11) NOT NULL, `furtif` int(1) NOT NULL, PRIMARY KEY (`id_mnemo`)"
                  ") ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
                 );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2541)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE dls_cpt_imp TO mnemos_CptImp" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2543)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE tempo TO mnemos_Tempo" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2571)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `users` ADD `imsg_available` TINYINT NOT NULL AFTER `imsg_allow_cde`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2573)
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

    if (database_version <= 2581)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` CHANGE `min_e_tor` `map_E` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` CHANGE `min_e_ana` `map_EA` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` CHANGE `min_s_tor` `map_A` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` CHANGE `min_s_ana` `map_AA` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2582)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `rfxcom` CHANGE `e_min` `map_E` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `rfxcom` CHANGE `ea_min` `map_EA` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `rfxcom` CHANGE `a_min` `map_A` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2583)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE onduleurs TO ups" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `ups` CHANGE `e_min` `map_E` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `ups` CHANGE `ea_min` `map_EA` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `ups` CHANGE `a_min` `map_A` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2669)
     { g_snprintf( requete, sizeof(requete), "DROP TABLE rfxcom" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2696)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `dls` ADD `compil_date` int(11) NOT NULL AFTER `actif`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `dls` ADD `compil_status` int(11) NOT NULL AFTER `compil_date`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2728)
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

    if (database_version <= 2743)
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

    if (database_version <= 2748)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_DigitalInput` DROP COLUMN `furtif`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2847)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE dls_cpth TO mnemos_CptHoraire" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2850)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_CptHoraire` CHANGE `val` `valeur` INT(11) NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2857)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos` ADD `acro_syn` text NOT NULL AFTER `tableau`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2871)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_CptImp` CHANGE `val` `valeur`  float NOT NULL" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2909)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls ADD `shortname` text NOT NULL AFTER `name`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "UPDATE dls SET shortname='Systeme' WHERE id=1" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2911)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD `id_plugin_dls` INT(11) NOT NULL AFTER `num`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "UPDATE msgs SET id_plugin_dls=1" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2912)
     { g_snprintf( requete, sizeof(requete), "UPDATE msgs INNER JOIN syns ON msgs.id_syn=syns.id INNER JOIN dls ON dls.num_syn=syns.id SET msgs.id_plugin_dls=dls.id;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs DROP COLUMN `id_syn`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2914)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls CHANGE `num_syn` `syn_id` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2915)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `id_plugin_dls` `dls_id` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2934)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE histo_msgs CHANGE `id_num` `id_msg` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2951)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE histo_bit ADD `date_time` DATETIME(6) AFTER `date_usec`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2974)
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

    if (database_version <= 2991)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` ENGINE = INNODB ROW_FORMAT = DYNAMIC;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE histo_msgs ADD FOREIGN KEY fk_id_msg (`id_msg`) REFERENCES msgs(`id`)"
                                             " ON DELETE CASCADE ON UPDATE RESTRICT;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 2994)
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

    if (database_version <= 3019)
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

    if (database_version <= 3055)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `bit_voc` `bit_audio` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD `audio` TINYINT(1) NOT NULL DEFAULT '0' AFTER `bit_audio`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs DROP type_voc, DROP vitesse_voc" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3083)
     { g_snprintf( requete, sizeof(requete), "RENAME TABLE syns_capteurs TO syns_cadrans" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD `persist` TINYINT(1) NOT NULL DEFAULT '0' AFTER `enable`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3086)
     { g_snprintf( requete, sizeof(requete),
                  "CREATE TABLE IF NOT EXISTS `mnemos_Registre` ("
                  "`id_mnemo` int(11) NOT NULL,"
                  "`unite` text COLLATE utf8_unicode_ci NOT NULL,"
                  "PRIMARY KEY (`id_mnemo`),"
                  "CONSTRAINT `id_mnemo` FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE"
                  ") ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3113)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos CHANGE `num_plugin` `dls_id` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3128)
     { g_snprintf( requete, sizeof(requete), "INSERT INTO mnemos (`id`, `type`, `num`, `dls_id`, `acronyme`, `libelle`, `command_text`) VALUES "
                                             "(110, 1, 07, 1, 'SYS_EVENT_NOT_FOUND', 'Event not found', '')" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3159)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls ADD `nbr_compil` INT(11) NOT NULL DEFAULT '0' AFTER `compil_status`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3178)
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

    if (database_version <= 3209)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs CHANGE `syn` `syn_id` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3215)
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

    if (database_version <= 3247)
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

    if (database_version <= 3252)
     { g_snprintf( requete, sizeof(requete), "INSERT INTO mnemos (`id`, `type`, `num`, `dls_id`, `acronyme`, `libelle`, `command_text`) VALUES "
                                             "(111, 1,  8, 1, 'SYS_NEW_TICK', 'Default Command by Tick', '');" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3310)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD `is_mp3` TINYINT(1) NOT NULL DEFAULT '0' AFTER `time_repeat`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3348)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE users ADD `phphash` VARCHAR(130) NOT NULL AFTER `hash`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3353)
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

    if (database_version <= 3380)
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

    if (database_version <= 3386)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls ADD `sourcecode` MEDIUMTEXT NOT NULL AFTER `nbr_compil`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3444)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE modbus_modules ADD `date_create` DATETIME NOT NULL AFTER `id`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE modbus_modules DROP `instance_id`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3448)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE rs485 CHANGE `instance_id` `host` TEXT NOT NULL" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE rs485 ADD `date_ajout` DATETIME AFTER `host`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3463)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos ADD `ev_host` VARCHAR(40) NOT NULL AFTER `libelle`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos ADD `ev_thread` VARCHAR(20) NOT NULL AFTER `ev_host`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos CHANGE `command_text` `ev_text` VARCHAR(160) NOT NULL" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3470)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE modbus_modules ADD `max_nbr_E` INT(11) NOT NULL AFTER `map_E`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3490)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE rs485 ADD `forced_e_min` INT(11) NOT NULL DEFAULT '0' AFTER `e_min`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3521)
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

    if (database_version <= 3550)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos` ADD `created_by_user` INT(1) NOT NULL DEFAULT '1' AFTER `id`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos` CHANGE `libelle` `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table mnemos change `ev_host` `ev_host` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table mnemos change `ev_thread` `ev_thread` VARCHAR(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table mnemos change `ev_text` `ev_text` VARCHAR(160) COLLATE utf8_unicode_ci NOT NULL DEFAULT ''");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table mnemos change `tableau` `tableau` TEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT ''");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table mnemos change `acro_syn` `acro_syn` TEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT ''");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3555)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `dls` ADD `tech_id` VARCHAR(24) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT id AFTER `id`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3586)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `dls` DROP `type`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `dls` ADD `package` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'custom' AFTER `tech_id`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3596)
     { g_snprintf( requete, sizeof(requete), "CREATE TABLE IF NOT EXISTS `mnemos_Horloge` ("
                                             "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                                             "`id_mnemo` int(11) NOT NULL,"
                                             "`heure` int(11) NOT NULL,"
                                             "`minute` int(112) NOT NULL,"
                                             "FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE"
                                             ") ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;"
                  );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3641)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos` DROP `created_by_user`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3663)
     { g_snprintf( requete, sizeof(requete), "UPDATE mnemos SET num='-1', nom='ARCH_REQUEST_NUMBER' WHERE id=13" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos SET num='-1', nom='DLS_BIT_PER_SEC' WHERE id=12" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos SET num='-1', nom='DLS_TOUR_PER_SEC' WHERE id=11" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos SET num='-1', nom='DLS_WAIT' WHERE id=10" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos SET num='-1', nom='DB_REQUEST_SIMULT' WHERE id=14" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3681)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs ADD `mnemo_id` int(11) UNIQUE NULL DEFAULT NULL" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs ADD CONSTRAINT FOREIGN KEY (`mnemo_id`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3682)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE users CHANGE `sms_phone` `phone` VARCHAR(80) DEFAULT ''" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE users ALTER TABLE users DROP `jabberid`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE users CHANGE `name` `username` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE users CHANGE `login_failed` `login_attempts` int(11) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE users ADD `username` varchar(100) DEFAULT NULL; "
                                             "ALTER TABLE users ADD `email` varchar(254) NOT NULL; "
                                             "ALTER TABLE users ADD `password` varchar(255) NOT NULL; ");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3687)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` CHANGE `ip` `hostname` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT ''" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3693)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls ADD `nbr_ligne` int(11) NOT NULL DEFAULT '0' AFTER `sourcecode`" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3694)
     { g_snprintf( requete, sizeof(requete), "UPDATE dls SET nbr_ligne = LENGTH(`sourcecode`)-LENGTH(REPLACE(`sourcecode`,'\n',''));" );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
     }

    if (database_version <= 3728)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE users ADD `email` varchar(120) COLLATE utf8_unicode_ci DEFAULT NULL AFTER `hash`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3747)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls ADD `compil_date_temp` DATETIME NOT NULL DEFAULT NOW();");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE dls SET compil_date_temp=FROM_UNIXTIME(compil_date);");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE dls DROP `compil_date`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE dls CHANGE `compil_date_temp` `compil_date` DATETIME NOT NULL DEFAULT NOW();");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3751)
     { g_snprintf( requete, sizeof(requete), "CREATE TABLE IF NOT EXISTS `audit_log` ("
                                             "`id` int(11) NOT NULL AUTO_INCREMENT,"
                                             "`username` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                                             "`access_level` int(11) NOT NULL DEFAULT '0',"
                                             "`message` varchar(256) COLLATE utf8_unicode_ci NOT NULL,"
                                             "`date` DATETIME NOT NULL DEFAULT NOW(),"
                                             "PRIMARY KEY (`id`)"
                                             ") ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
                                             " AUTO_INCREMENT=10000;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` DROP FOREIGN KEY `fk_msgs_dls_id`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` ADD CONSTRAINT `fk_msgs_dls_id` FOREIGN KEY (`dls_id`)"
                                             " REFERENCES `dls` (`id`) ON DELETE CASCADE");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `dls` ADD `errorlog` TEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT 'No Error'");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3779)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD `mnemo_id` int(11) NULL DEFAULT NULL AFTER `id`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3781)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `libelle` `libelle` VARCHAR(256)"
                                             " COLLATE utf8_unicode_ci NOT NULL DEFAULT 'No libelle'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `libelle_audio` `libelle_audio` VARCHAR(256)"
                                             " COLLATE utf8_unicode_ci NOT NULL DEFAULT 'No audio'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `libelle_sms` `libelle_sms` VARCHAR(256)"
                                             " COLLATE utf8_unicode_ci NOT NULL DEFAULT 'No sms'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `time_repeat` `time_repeat` int(11) NOT NULL DEFAULT '0'");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3792)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE modbus_modules CHANGE `libelle` `description` VARCHAR(128)"
                                             " COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT'" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE modbus_modules ADD `tech_id` VARCHAR(32)"
                                             " COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT hostname");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3796)
     { g_snprintf( requete, sizeof(requete), "DROP TABLE gids" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE icons ENGINE=INNODB");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE icons ADD FOREIGN KEY (`id_classe`)"
                                             " REFERENCES `class` (`id`) ON DELETE CASCADE;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs ADD FOREIGN KEY (`icone`)"
                                             " REFERENCES `icons` (`id`) ON DELETE CASCADE");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE icons ADD `date_create` datetime NOT NULL DEFAULT NOW() AFTER `id`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE class ADD `date_create` datetime NOT NULL DEFAULT NOW() AFTER `id`");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 3815)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD `acronyme` VARCHAR(64)"
                                             " COLLATE utf8_unicode_ci NULL DEFAULT NULL" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs ADD UNIQUE(`dls_id`,`acronyme`)" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4009)
     { g_snprintf( requete, sizeof(requete),
      "ALTER TABLE `dls` CHANGE `tech_id` `tech_id` VARCHAR(25) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL;" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4024)
     { g_snprintf( requete, sizeof(requete),
                  "CREATE TABLE IF NOT EXISTS `mnemos_DI` ("
                  "`id` int(11) NOT NULL AUTO_INCREMENT,"
                  "`dls_id` int(11) NOT NULL DEFAULT '0',"
                  "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                  "`libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                  "`src_host` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',"
                  "`src_thread` VARCHAR(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',"
                  "`src_text` VARCHAR(160) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                  "PRIMARY KEY (`id`),"
                  "UNIQUE (`dls_id`,`acronyme`),"
                  "FOREIGN KEY (`dls_id`) REFERENCES `dls` (`id`) ON DELETE CASCADE"
                  ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "DROP TABLE mnemos_DigitalInput" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4030)
     { g_snprintf( requete, sizeof(requete),
                   "CREATE TABLE IF NOT EXISTS `mnemos_DO` ("
                   "`id` int(11) NOT NULL AUTO_INCREMENT,"
                   "`dls_id` int(11) NOT NULL DEFAULT '0',"
                   "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                   "`libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                   "`dst_host` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',"
                   "`dst_taghread` VARCHAR(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',"
                   "`dst_tag` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                   "`dst_param1` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                   "PRIMARY KEY (`id`),"
                   "UNIQUE (`dls_id`,`acronyme`),"
                   "FOREIGN KEY (`dls_id`) REFERENCES `dls` (`id`) ON DELETE CASCADE"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       Lancer_requete_SQL ( db, requete );
     }


    if (database_version <= 4079)
     { g_snprintf( requete, sizeof(requete),
                   "CREATE TABLE IF NOT EXISTS `mnemos_CI` ("
                   "`id` INT(11) NOT NULL AUTO_INCREMENT,"
                   "`dls_id` INT(11) NOT NULL DEFAULT '0',"
                   "`etat` BOOLEAN NOT NULL DEFAULT '0',"
                   "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                   "`libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                   "`valeur` INT(11) NOT NULL DEFAULT '0',"
                   "`multi` float NOT NULL DEFAULT '1',"
                   "`unite` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'fois',"
                   "PRIMARY KEY (`id`),"
                   "UNIQUE (`dls_id`,`acronyme`),"
                   "FOREIGN KEY (`dls_id`) REFERENCES `dls` (`id`) ON DELETE CASCADE"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4084)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE syns_cadrans ADD `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT ''");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE syns_cadrans ADD `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT ''");
       Lancer_requete_SQL ( db, requete );
     }


    if (database_version <= 4087)
     { g_snprintf( requete, sizeof(requete),
                  "CREATE TABLE IF NOT EXISTS `mnemos_AI` ("
                  "`id` int(11) NOT NULL AUTO_INCREMENT,"
                  "`dls_id` int(11) NOT NULL DEFAULT '0',"
                  "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                  "`libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                  "`type` int(11) NOT NULL DEFAULT '0',"
                  "`min` float NOT NULL DEFAULT '0',"
                  "`max` float NOT NULL DEFAULT '0',"
                  "`unite` text COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                  "`map_host` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',"
                  "`map_thread` VARCHAR(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',"
                  "`map_text` VARCHAR(160) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                  "`map_question_vocale` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                  "PRIMARY KEY (`id`), "
                  "UNIQUE (`dls_id`,`acronyme`),"
                  "FOREIGN KEY (`dls_id`) REFERENCES `dls` (`id`) ON DELETE CASCADE"
                  ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4094)
     { g_snprintf( requete, sizeof(requete),
                  "CREATE TABLE IF NOT EXISTS `mnemos_CH` ("
                  "`id` INT(11) NOT NULL AUTO_INCREMENT,"
                  "`dls_id` INT(11) NOT NULL DEFAULT '0',"
                  "`etat` BOOLEAN NOT NULL DEFAULT '0',"
                  "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                  "`libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                  "`valeur` int(11) NOT NULL DEFAULT '0',"
                  "PRIMARY KEY (`id`),"
                  "UNIQUE (`dls_id`,`acronyme`),"
                  "FOREIGN KEY (`dls_id`) REFERENCES `dls` (`id`) ON DELETE CASCADE"
                  ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4103)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `icons` ADD `type` VARCHAR(4) NOT NULL DEFAULT 'svg' AFTER `id`");
       g_snprintf( requete, sizeof(requete),
                   "UPDATE `icons` SET `type`='gif'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_motifs` ADD `tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '';" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_motifs` ADD `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '';" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4105)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_motifs` ADD `scale` float NOT NULL DEFAULT '1' AFTER `angle`;"
                   "ALTER TABLE `syns_motifs` ADD `def_color` VARCHAR(12) NOT NULL DEFAULT '#c0c0c0' AFTER `scale`;" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4110)
     { g_snprintf( requete, sizeof(requete),
                   "CREATE TABLE IF NOT EXISTS `mnemos_HORLOGE` ("
                   "`id` INT(11) NOT NULL AUTO_INCREMENT,"
                   "`dls_id` INT(11) NOT NULL DEFAULT '0',"
                   "`etat` BOOLEAN NOT NULL DEFAULT '0',"
                   "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                   "`libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                   "`heure` int(11) NOT NULL DEFAULT '0',"
                   "`minute` int(11) NOT NULL DEFAULT '0',"
                   "PRIMARY KEY (`id`),"
                   "UNIQUE (`dls_id`,`acronyme`),"
                   "FOREIGN KEY (`dls_id`) REFERENCES `dls` (`id`) ON DELETE CASCADE"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "DROP TABLE mnemos_Horloge");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4115)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_motifs` DROP `bitclic2`;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_motifs` ADD `clic_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '';" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_motifs` ADD `clic_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '';" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4117)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `mnemos_AI` ADD `map_reponse_vocale` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'aucun';" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4174)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `ups` ADD `tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT 'NEW';" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `ups` DROP `instance_id`;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `ups` DROP `bit_comm`;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `ups` CHANGE `ups` `name` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `ups` ADD `date_create` DATETIME NOT NULL DEFAULT NOW()");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4187)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `mnemos_Tempo` ADD `dls_id` int(11) NOT NULL DEFAULT '0';" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `mnemos_Tempo` ADD `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `mnemos_Tempo` ADD `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '';" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_Tempo` DROP id_mnemo;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_Tempo` DROP delai_on;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_Tempo` DROP delai_off;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_Tempo` DROP min_on;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_Tempo` DROP max_on;" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4203)
     { g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_AI` ADD `tech_id` VARCHAR(32) NULL DEFAULT NULL AFTER `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "UPDATE mnemos_AI inner join dls on mnemos_AI.dls_id = dls.id SET mnemos_AI.tech_id=dls.tech_id " );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_AI` ADD CONSTRAINT `mnemos_AI_tech_id` FOREIGN KEY (`tech_id`)"
                  " REFERENCES `dls`(`tech_id`) ON DELETE CASCADE ON UPDATE RESTRICT;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `watchdog`.`mnemos_AI` ADD UNIQUE `tech_id` (`tech_id`, `acronyme`);");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_AI` DROP INDEX `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_AI` DROP `dls_id`;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4205)
     { g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_DI` ADD `tech_id` VARCHAR(32) NULL DEFAULT NULL AFTER `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "UPDATE mnemos_DI inner join dls on mnemos_DI.dls_id = dls.id SET mnemos_DI.tech_id=dls.tech_id " );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_DI` ADD CONSTRAINT `mnemos_DI_tech_id` FOREIGN KEY (`tech_id`)"
                  " REFERENCES `dls`(`tech_id`) ON DELETE CASCADE ON UPDATE RESTRICT;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `watchdog`.`mnemos_DI` ADD UNIQUE `tech_id` (`tech_id`, `acronyme`);");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_DI` DROP INDEX `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_DI` DROP `dls_id`;");
       Lancer_requete_SQL ( db, requete );

       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_DO` ADD `tech_id` VARCHAR(32) NULL DEFAULT NULL AFTER `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "UPDATE mnemos_DO inner join dls on mnemos_DO.dls_id = dls.id SET mnemos_DO.tech_id=dls.tech_id " );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_AO` ADD CONSTRAINT `mnemos_DO_tech_id` FOREIGN KEY (`tech_id`)"
                  " REFERENCES `dls`(`tech_id`) ON DELETE CASCADE ON UPDATE RESTRICT;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `watchdog`.`mnemos_DO` ADD UNIQUE `tech_id` (`tech_id`, `acronyme`);");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_DO` DROP INDEX `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_DO` DROP `dls_id`;");
       Lancer_requete_SQL ( db, requete );

       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_CI` ADD `tech_id` VARCHAR(32) NULL DEFAULT NULL AFTER `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "UPDATE mnemos_CI inner join dls on mnemos_CI.dls_id = dls.id SET mnemos_CI.tech_id=dls.tech_id " );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_CI` ADD CONSTRAINT `mnemos_CI_tech_id` FOREIGN KEY (`tech_id`)"
                  " REFERENCES `dls`(`tech_id`) ON DELETE CASCADE ON UPDATE RESTRICT;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `watchdog`.`mnemos_CI` ADD UNIQUE `tech_id` (`tech_id`, `acronyme`);");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_CI` DROP INDEX `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_CI` DROP `dls_id`;");
       Lancer_requete_SQL ( db, requete );

       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_CH` ADD `tech_id` VARCHAR(32) NULL DEFAULT NULL AFTER `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "UPDATE mnemos_CH inner join dls on mnemos_CH.dls_id = dls.id SET mnemos_CH.tech_id=dls.tech_id " );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_CH` ADD CONSTRAINT `mnemos_CH_tech_id` FOREIGN KEY (`tech_id`)"
                  " REFERENCES `dls`(`tech_id`) ON DELETE CASCADE ON UPDATE RESTRICT;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `watchdog`.`mnemos_CH` ADD UNIQUE `tech_id` (`tech_id`, `acronyme`);");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_CH` DROP INDEX `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_CH` DROP `dls_id`;");
       Lancer_requete_SQL ( db, requete );

       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_Tempo` ADD `tech_id` VARCHAR(32) NULL DEFAULT NULL AFTER `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "UPDATE mnemos_Tempo inner join dls on mnemos_Tempo.dls_id = dls.id SET mnemos_Tempo.tech_id=dls.tech_id " );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_Tempo` ADD CONSTRAINT `mnemos_Tempo_tech_id` FOREIGN KEY (`tech_id`)"
                  " REFERENCES `dls`(`tech_id`) ON DELETE CASCADE ON UPDATE RESTRICT;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `watchdog`.`mnemos_Tempo` ADD UNIQUE `tech_id` (`tech_id`, `acronyme`);");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_Tempo` DROP INDEX `dls_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "ALTER TABLE `mnemos_Tempo` DROP `dls_id`;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4206)
     { g_snprintf( requete, sizeof(requete),
                  "CREATE TABLE IF NOT EXISTS `mnemos_BOOL` ("
                  "`id` int(11) NOT NULL AUTO_INCREMENT,"
                  "`tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,"
                  "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                  "`libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                  "`etat` BOOLEAN NOT NULL DEFAULT 0,"
                  "PRIMARY KEY (`id`),"
                  "UNIQUE (`tech_id`,`acronyme`),"
                  "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE"
                  ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4219)
     { g_snprintf( requete, sizeof(requete),
                   "CREATE TABLE IF NOT EXISTS `tableau` ("
                   "`id` INT NOT NULL AUTO_INCREMENT ,"
                   "`type` int(11) NOT NULL DEFAULT 0,"
                   "`titre` VARCHAR(128) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL ,"
                   "`access_level` int(11) NOT NULL ,"
                   "`date_create` DATETIME NOT NULL ,"
                   "PRIMARY KEY (`id`)) ENGINE = InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                  "CREATE TABLE IF NOT EXISTS `courbes` ("
                  "`id` INT NOT NULL AUTO_INCREMENT ,"
                  "`tableau_id` INT NOT NULL ,"
                  "`tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,"
                  "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                  "`color` varchar(32) COLLATE utf8_unicode_ci NOT NULL,"
                  "PRIMARY KEY (`id`),"
                  "INDEX (`tableau_id`),"
                  "FOREIGN KEY (`tableau_id`) REFERENCES `tableau` (`id`) ON DELETE CASCADE"
                  ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4230)
     { g_snprintf( requete, sizeof(requete), "DROP TABLE mnemos_HORLOGE" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "CREATE TABLE IF NOT EXISTS `mnemos_HORLOGE` ("
                   "`id` INT(11) NOT NULL AUTO_INCREMENT,"
                   "`tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,"
                   "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                   "PRIMARY KEY (`id`),"
                   "UNIQUE (`tech_id`,`acronyme`),"
                   "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "CREATE TABLE IF NOT EXISTS `mnemos_HORLOGE_ticks` ("
                   "`id` INT(11) NOT NULL AUTO_INCREMENT,"
                   "`horloge_id` INT(11) NOT NULL,"
                   "`heure` int(11) NOT NULL DEFAULT '0',"
                   "`minute` int(11) NOT NULL DEFAULT '0',"
                   "`lundi` tinyint(1) NOT NULL DEFAULT '0',"
                   "PRIMARY KEY (`id`),"
                   "FOREIGN KEY (`horloge_id`) REFERENCES `mnemos_HORLOGE` (`id`) ON DELETE CASCADE"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4267)
     { g_snprintf( requete, sizeof(requete), "alter table histo_msgs ADD `date_create` DATETIME(2)");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "update histo_msgs set date_create = FROM_UNIXTIME(CONCAT(date_create_sec,'.',date_create_usec));" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table histo_msgs DROP `date_create_sec`;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table histo_msgs DROP `date_create_usec`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table histo_msgs ADD `date_fin_temp` DATETIME(2);" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table histo_msgs ADD `date_fixe_temp` DATETIME(2);" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "update histo_msgs set date_fin_temp = FROM_UNIXTIME(date_fin);" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "update histo_msgs set date_fixe_temp = FROM_UNIXTIME(date_fixe);" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table histo_msgs DROP `date_fin`;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table histo_msgs DROP `date_fixe`;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table histo_msgs CHANGE `date_fin_temp` `date_fin` DATETIME(2);" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "alter table histo_msgs CHANGE `date_fixe_temp` `date_fixe` DATETIME(2);");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4271)
     { g_snprintf( requete, sizeof(requete), "alter table msgs ADD `etat` tinyint(1) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4277)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_cadrans ADD `fleche_left` tinyint(1) NOT NULL DEFAULT '0'" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_cadrans ADD `nb_decimal` int(11) NOT NULL DEFAULT '2'" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_cadrans CHANGE `angle` `angle` int(11) NOT NULL DEFAULT '0'");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4307)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE histo_msgs CHANGE `alive` `alive` tinyint(1) NULL DEFAULT NULL" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE histo_msgs SET `alive`=NULL WHERE `alive`=0" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "DELETE FROM histo_msgs WHERE `alive`=1" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE histo_msgs ADD UNIQUE (`id_msg`,`alive`)" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4323)
     { g_snprintf( requete, sizeof(requete),
                   "CREATE TABLE IF NOT EXISTS `syns_liens` ("
                   "`id` int(11) NOT NULL AUTO_INCREMENT,"
                   "`syn_id` int(11) NOT NULL DEFAULT 0,"
                   "`src_posx` int(11) NOT NULL DEFAULT 0,"
                   "`src_posy` int(11) NOT NULL DEFAULT 0,"
                   "`dst_posx` int(11) NOT NULL DEFAULT 0,"
                   "`dst_posy` int(11) NOT NULL DEFAULT 0,"
                   "`stroke` varchar(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'black',"
                   "`stroke_dasharray` varchar(32) COLLATE utf8_unicode_ci DEFAULT NULL,"
                   "`stroke_width` int(11) NOT NULL DEFAULT 1,"
                   "PRIMARY KEY (`id`),"
                   "FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE"
                   ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "CREATE TABLE IF NOT EXISTS `syns_rectangles` ("
                   "`id` int(11) NOT NULL AUTO_INCREMENT,"
                   "`syn_id` int(11) NOT NULL DEFAULT 0,"
                   "`posx` int(11) NOT NULL DEFAULT 0,"
                   "`posy` int(11) NOT NULL DEFAULT 0,"
                   "`width` int(11) NOT NULL DEFAULT 10,"
                   "`height` int(11) NOT NULL DEFAULT 10,"
                   "`rx` int(11) NOT NULL DEFAULT 0,"
                   "`ry` int(11) NOT NULL DEFAULT 0,"
                   "`stroke` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'black',"
                   "`fill` varchar(32) COLLATE utf8_unicode_ci NOT NULL,"
                   "`stroke_width` int(11) NOT NULL DEFAULT 1,"
                   "`stroke_dasharray` varchar(32) COLLATE utf8_unicode_ci NULL,"
                   "PRIMARY KEY (`id`),"
                   "FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE"
                   ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4327)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_liens` ADD `stroke_linecap` varchar(32) COLLATE utf8_unicode_ci DEFAULT 'butt'" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_liens` ADD `tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '';" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_liens` ADD `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '';" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4328)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_rectangles` ADD `tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '';" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_rectangles` ADD `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '';" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4333)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE users ADD `session_id` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'NONE'" );
       Lancer_requete_SQL ( db, requete );
     }


    if (database_version <= 4334)
     { g_snprintf( requete, sizeof(requete),
                   "ALTER TABLE `syns_rectangles` CHANGE "
                   "`fill` `def_color` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL;" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4377)
     { g_snprintf( requete, sizeof(requete),
                   "CREATE TABLE IF NOT EXISTS `mnemos_AO` ("
                   "`id` int(11) NOT NULL AUTO_INCREMENT,"
                   "`tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,"
                   "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                   "`libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                   "`type` int(11) NOT NULL DEFAULT '0',"
                   "`min` float NOT NULL DEFAULT '0',"
                   "`max` float NOT NULL DEFAULT '0',"
                   "`valeur` float NOT NULL DEFAULT '0',"
                   "`map_host` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',"
                   "`map_thread` VARCHAR(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',"
                   "`map_text` VARCHAR(160) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                   "PRIMARY KEY (`id`),"
                   "UNIQUE (`tech_id`,`acronyme`),"
                   "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4412)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` ADD `mode_old_static` TINYINT(1) NOT NULL DEFAULT '0'");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4422)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_cadrans` CHANGE `fleche_left` `fleche` int(11) NOT NULL DEFAULT '0'");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4430)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` DROP `mode_old_static`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `modbus_modules` DROP `bit`");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4433)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_Tempo` DROP FOREIGN KEY `mnemos_Tempo_tech_id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_Tempo` ADD CONSTRAINT `mnemos_Tempo_tech_id`"
                  " FOREIGN KEY (`tech_id`) REFERENCES `dls`(`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4435)
     { g_snprintf( requete, sizeof(requete), "UPDATE `mnemos_CptHoraire` SET valeur = valeur * 60;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE `mnemos_CH` SET valeur = valeur * 60;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4437)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_comments` ADD `font_size` int(11) NOT NULL DEFAULT '20'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_comments` ADD `def_color` VARCHAR(12) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'white'");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4446)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` ADD `profil_audio` VARCHAR(80) NOT NULL DEFAULT 'P_ALL'");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4480)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_cadrans` DROP `bitctrl`");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4494)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_HORLOGE` ADD `libelle` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default' AFTER `acronyme`; ");
       Lancer_requete_SQL ( db, requete );
     }


    if (database_version <= 4503)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_motifs` ADD `forme` VARCHAR(80) NOT NULL DEFAULT 'unknown' AFTER`id`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_motifs` ADD `auto_create` tinyint(1) NULL DEFAULT NULL AFTER `id`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_motifs` ADD UNIQUE (`tech_id`, `acronyme`, `auto_create`);");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4511)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` ADD `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '' AFTER `id`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE `msgs` INNER JOIN dls ON msgs.dls_id=dls.id SET msgs.tech_id=dls.tech_id;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` ADD UNIQUE(`tech_id`,`acronyme`);" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` ADD FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4511)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` DROP `dls_id`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` DROP `mnemo_id`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` DROP `num`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` DROP `bit_audio`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` DROP `time_repeat`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `msgs` DROP `persist`");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4531)
     { g_snprintf( requete, sizeof(requete), "update mnemos_DI set src_thread='imsgs' where src_thread='imsgp'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "DROP TABLE `mnemos_CptImp`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "DROP TABLE `mnemos_CptHoraire`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "DROP TABLE `mnemos_Registre`");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "CREATE TABLE IF NOT EXISTS `mnemos_R` ("
                                             "`id` int(11) NOT NULL AUTO_INCREMENT,"
                                             "`tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,"
                                             "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                                             "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL,"
                                             "`valeur` float NOT NULL DEFAULT '0',"
                                             "`unite` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                                             "PRIMARY KEY (`id`),"
                                             "UNIQUE (`tech_id`,`acronyme`),"
                                             "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
                                             ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4551)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_R` ADD `archivage` BOOLEAN NOT NULL DEFAULT '0'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_R` ADD `map_question_vocale` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT ''");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_R` ADD `map_reponse_vocale` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'aucun'");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4562)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `syns_cadrans` DROP `fleche`");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4567)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_AI` ADD `valeur` float NOT NULL DEFAULT '0' AFTER `max`");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4722)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE users ENGINE=INNODB");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "DROP TABLE users_sessions;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "CREATE TABLE `users_sessions` ("
                                             "`username` VARCHAR(32) NOT NULL,"
                                             "`wtd_session` VARCHAR(42) NOT NULL,"
                                             "`date_create` datetime NOT NULL,"
                                             "FOREIGN KEY (`username`) REFERENCES `users` (`username`) ON DELETE CASCADE ON UPDATE CASCADE"
                                             ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4851)
     { g_snprintf( requete, sizeof(requete), "DROP TABLE `mnemos_AnalogInput`");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4880)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_DI CHANGE `src_thread` `map_thread` VARCHAR(20) COLLATE utf8_unicode_ci NULL DEFAULT NULL");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_DI CHANGE `src_text` `map_tag` VARCHAR(160) COLLATE utf8_unicode_ci NULL DEFAULT NULL");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_DI CHANGE `src_host` `map_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos_DI SET map_tech_id=LEFT(map_tag,7) WHERE map_thread='MODBUS'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos_DI SET map_tech_id='GSM01' WHERE map_thread='SMSG'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos_DI SET map_tech_id='IMSGS' WHERE map_thread LIKE 'imsg%%'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos_DI SET map_tag=RIGHT(map_tag,4) WHERE map_thread='MODBUS'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE `mnemos_DI` SET map_tech_id=NULL WHERE map_tech_id='*';");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE `mnemos_DI` SET map_tag=NULL WHERE map_tag='';");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_DI` ADD UNIQUE(`map_tech_id`, `map_tag`); ");
       Lancer_requete_SQL ( db, requete );

     }

    if (database_version <= 4889)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_AI CHANGE `map_host` `map_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_AI CHANGE `map_thread` `map_thread` VARCHAR(20) COLLATE utf8_unicode_ci NULL DEFAULT NULL");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_AI CHANGE `map_text` `map_tag` VARCHAR(160) COLLATE utf8_unicode_ci NULL DEFAULT NULL");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos_AI SET map_tech_id=LEFT(map_tag,7) WHERE map_thread='MODBUS'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos_AI SET map_tag=RIGHT(map_tag,4) WHERE map_thread='MODBUS'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE `mnemos_AI` SET map_tech_id=NULL WHERE map_tech_id='*';");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE `mnemos_AI` SET map_thread=NULL WHERE map_thread='*';");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE `mnemos_AI` SET map_tag=NULL WHERE map_tag='';");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_AI` ADD UNIQUE(`map_tech_id`, `map_tag`); ");
       Lancer_requete_SQL ( db, requete );
     }
    if (database_version <= 4891)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_DO CHANGE `dst_host` `map_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_DO CHANGE `dst_thread` `map_thread` VARCHAR(20) COLLATE utf8_unicode_ci NULL DEFAULT NULL");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_DO CHANGE `dst_tag` `map_tag` VARCHAR(160) COLLATE utf8_unicode_ci NULL DEFAULT NULL");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos_DO SET map_tech_id=LEFT(map_tag,7) WHERE map_thread='MODBUS'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE mnemos_DO SET map_tag=RIGHT(map_tag,4) WHERE map_thread='MODBUS'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE `mnemos_DO` SET map_tech_id=NULL WHERE map_tech_id='*';");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE `mnemos_DO` SET map_thread=NULL WHERE map_thread='*';");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "UPDATE `mnemos_DO` SET map_tag=NULL WHERE map_tag='';");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE `mnemos_DO` ADD UNIQUE(`map_tech_id`, `map_tag`); ");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4908)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_CI ADD `archivage` BOOLEAN NOT NULL DEFAULT '1'");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4952)
     { g_snprintf( requete, sizeof(requete), "UPDATE syns_cadrans SET tech_id='SYS' WHERE tech_id=''");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_cadrans ADD "
                                             "FOREIGN KEY (`tech_id`) REFERENCES `dls`(`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4968)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE config DROP CONSTRAINT PRIMARY KEY;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE config ADD `id` int(11) NOT NULL AUTO_INCREMENT");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4984)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_DI ADD `deletable` TINYINT(1) NOT NULL DEFAULT '1' AFTER `id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_DO ADD `deletable` TINYINT(1) NOT NULL DEFAULT '1' AFTER `id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_AI ADD `deletable` TINYINT(1) NOT NULL DEFAULT '1' AFTER `id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_AO ADD `deletable` TINYINT(1) NOT NULL DEFAULT '1' AFTER `id`;");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_BOOL ADD `deletable` TINYINT(1) NOT NULL DEFAULT '1' AFTER `id`;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 4987)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs DROP `bitclic`;");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 5039)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE dls ADD `debug` TINYINT(1) NOT NULL DEFAULT '0';");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 5041)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE mnemos_CI CHANGE `archivage` `archivage` INT(11) NOT NULL DEFAULT '1'" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 5044)
     { g_snprintf( requete, sizeof(requete), "UPDATE msgs SET profil_audio='P_NONE' WHERE audio=0" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs DROP `audio`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 5057)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs DROP `bitctrl`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs DROP `rouge`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs DROP `vert`" );
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE syns_motifs DROP `bleu`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 5070)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs DROP `enable`" );
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 5105)
     { g_snprintf( requete, sizeof(requete), "update syns_motifs set forme='abls' where icone=535");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "update syns_motifs set forme='none' where forme='unknown'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "update syns_motifs set forme='goutte_eau' where icone=443");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "update syns_motifs set forme='klaxon' where icone=44");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "update syns_motifs set forme='lampe_lune' where icone=458");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 5132)
     { g_snprintf( requete, sizeof(requete), "CREATE TABLE IF NOT EXISTS `mnemos_WATCHDOG` ("
                                             "`id` int(11) NOT NULL AUTO_INCREMENT,"
                                             "`deletable` tinyint(1) NOT NULL DEFAULT '1',"
                                             "`tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,"
                                             "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                                             "`libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                                             "PRIMARY KEY (`id`),"
                                             "UNIQUE (`tech_id`,`acronyme`),"
                                             "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
                                             ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000");
       Lancer_requete_SQL ( db, requete );
     }

    if (database_version <= 5153)
     { g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `sms` `sms_notification` int(11) NOT NULL DEFAULT '0'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `libelle_sms` `sms_libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT ''");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `profil_audio` `audio_profil` VARCHAR(80) NOT NULL DEFAULT 'P_NONE'");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `libelle_audio` `audio_libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT ''");
       Lancer_requete_SQL ( db, requete );
       g_snprintf( requete, sizeof(requete), "ALTER TABLE msgs CHANGE `type` `typologie` int(11) NOT NULL DEFAULT '0'");
       Lancer_requete_SQL ( db, requete );
     }

fin:
    g_snprintf( requete, sizeof(requete), "CREATE OR REPLACE VIEW db_status AS SELECT "
                                          "(SELECT COUNT(*) FROM syns) AS nbr_syns, "
                                          "(SELECT COUNT(*) FROM syns_motifs) AS nbr_syns_motifs, "
                                          "(SELECT COUNT(*) FROM syns_liens) AS nbr_syns_liens, "
                                          "(SELECT COUNT(*) FROM dls) AS nbr_dls, "
                                          "(SELECT COUNT(*) FROM mnemos_DI) AS nbr_dls_di, "
                                          "(SELECT COUNT(*) FROM mnemos_DO) AS nbr_dls_do, "
                                          "(SELECT COUNT(*) FROM mnemos_AI) AS nbr_dls_ai, "
                                          "(SELECT COUNT(*) FROM mnemos_AO) AS nbr_dls_ao, "
                                          "(SELECT COUNT(*) FROM mnemos_BOOL) AS nbr_dls_bool, "
                                          "(SELECT SUM(dls.nbr_ligne) FROM dls) AS nbr_dls_lignes, "
                                          "(SELECT COUNT(*) FROM users) AS nbr_users, "
                                          "(SELECT COUNT(*) FROM msgs) AS nbr_msgs, "
                                          "(SELECT COUNT(*) FROM histo_msgs) AS nbr_histo_msgs, "
                                          "(SELECT COUNT(*) FROM audit_log) AS nbr_audit_log" );
    Lancer_requete_SQL ( db, requete );

    g_snprintf( requete, sizeof(requete),
       "CREATE OR REPLACE VIEW dictionnaire AS "
       "SELECT 'AI' AS classe, %d AS classe_int,tech_id,acronyme,libelle from mnemos_AI UNION "
       "SELECT 'DI' AS classe, %d AS classe_int,tech_id,acronyme,libelle from mnemos_DI UNION "
       "SELECT 'DO' AS classe, %d AS classe_int,tech_id,acronyme,libelle from mnemos_DO UNION "
       "SELECT 'AO' AS classe, %d AS classe_int,tech_id,acronyme,libelle from mnemos_AO UNION "
       "SELECT 'BOOL' AS classe, type AS classe_int,tech_id,acronyme,libelle from mnemos_BOOL UNION "
       "SELECT 'CH' AS classe, %d AS classe_int,tech_id,acronyme,libelle from mnemos_CH UNION "
       "SELECT 'CI' AS classe, %d AS classe_int,tech_id,acronyme,libelle from mnemos_CI UNION "
       "SELECT 'HORLOGE' AS classe, %d AS classe_int,tech_id,acronyme,libelle from mnemos_HORLOGE UNION "
       "SELECT 'TEMPO' AS classe, %d AS classe_int,tech_id,acronyme,libelle from mnemos_Tempo UNION "
       "SELECT 'REGISTRE' AS classe, %d AS classe_int,tech_id,acronyme,libelle from mnemos_R UNION "
       "SELECT 'VISUEL' AS classe, -1 AS classe_int,tech_id,acronyme,libelle from syns_motifs UNION "
       "SELECT 'WATCHDOG' AS classe, %d1 AS classe_int,tech_id,acronyme,libelle from mnemos_WATCHDOG UNION "
       "SELECT 'MESSAGE' AS classe, %d AS classe_int,tech_id,acronyme,libelle from msgs",
        MNEMO_ENTREE_ANA, MNEMO_ENTREE, MNEMO_SORTIE, MNEMO_SORTIE_ANA, MNEMO_CPTH, MNEMO_CPT_IMP, MNEMO_HORLOGE,
        MNEMO_TEMPO, MNEMO_REGISTRE, MNEMO_WATCHDOG, MNEMO_MSG
      );
    Lancer_requete_SQL ( db, requete );
    Libere_DB_SQL(&db);

    if (Modifier_configDB ( "msrv", "database_version", WTD_DB_VERSION ))
     { Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: updating Database_version to %s OK", __func__, WTD_DB_VERSION ); }
    else
     { Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: updating Database_version to %s FAILED", __func__, WTD_DB_VERSION ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
