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
 #include <locale.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

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
    gint timeout = 10;
    mysql_options( db->mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout );                         /* Timeout en cas de non reponse */
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
/* SQL_Field_to_Json : Intéègre un MYSQL_FIELD dans une structure JSON                                                        */
/* Entrée: Le JsonNode et le MYSQL_FIELD                                                                                      */
/* Sortie: le jsonnode est mis a jour                                                                                         */
/******************************************************************************************************************************/
 static void SQL_Field_to_Json ( JsonNode *node, MYSQL_FIELD *field, gchar *chaine )
  { if ( field->type == MYSQL_TYPE_FLOAT || field->type==MYSQL_TYPE_DOUBLE )
     { if (chaine) Json_node_add_double( node, field->name, atof(chaine) );
              else Json_node_add_null  ( node, field->name );
     }
    else if ( field->type == MYSQL_TYPE_TINY )
     { if (chaine) Json_node_add_bool ( node, field->name, atoi(chaine) );
              else Json_node_add_null ( node, field->name );
     }
    else if ( IS_NUM(field->type) )
     { if (chaine) Json_node_add_int  ( node, field->name, atoi(chaine) );
              else Json_node_add_null ( node, field->name );
     }
    else
     { Json_node_add_string( node, field->name, chaine ); }
  }
/******************************************************************************************************************************/
/* SQL_Select_to_JSON : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 static gboolean SQL_Select_to_json_node_reel ( gboolean db_arch, JsonNode *RootNode, gchar *array_name, gchar *requete )
  { struct DB *db;
    if (db_arch) db = Init_ArchDB_SQL ();
            else db = Init_DB_SQL ();
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "%s: Init DB FAILED for '%s'", __func__, requete );
       return(FALSE);
     }

    if ( mysql_query ( db->mysql, requete ) )
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: FAILED (%s) for '%s'", __func__, (char *)mysql_error(db->mysql), requete );
       Libere_DB_SQL ( &db );
       if (array_name)
        { gchar chaine[80];
          g_snprintf(chaine, sizeof(chaine), "nbr_%s", array_name );
          Json_node_add_int ( RootNode, chaine, 0 );
          Json_node_add_array( RootNode, array_name );                            /* Ajoute un array vide en cas d'erreur SQL */
        }
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_db, LOG_DEBUG, "%s: DB OK for '%s'", __func__, requete );

    db->result = mysql_store_result ( db->mysql );
    if ( ! db->result )
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "%s: store_result failed (%s)", __func__, (char *) mysql_error(db->mysql) );
       db->nbr_result = 0;
     }
    if (array_name)
     { gchar chaine[80];
       g_snprintf(chaine, sizeof(chaine), "nbr_%s", array_name );
       Json_node_add_int ( RootNode, chaine, mysql_num_rows ( db->result ));
       JsonArray *array = Json_node_add_array( RootNode, array_name );
       while ( (db->row = mysql_fetch_row(db->result)) != NULL )
        { JsonNode *element = Json_node_create();
          for (gint cpt=0; cpt<mysql_num_fields(db->result); cpt++)
           { MYSQL_FIELD *field = mysql_fetch_field_direct(db->result, cpt);
             SQL_Field_to_Json ( element, field, db->row[cpt] );
           }
          Json_array_add_element ( array, element );
        }
     }
    else
     { while ( (db->row = mysql_fetch_row(db->result)) != NULL )
        { for (gint cpt=0; cpt<mysql_num_fields(db->result); cpt++)
           { MYSQL_FIELD *field = mysql_fetch_field_direct(db->result, cpt);
             SQL_Field_to_Json ( RootNode, field, db->row[cpt] );
           }
        }
     }
    mysql_free_result( db->result );
    Libere_DB_SQL( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* SQL_Write_new: Envoie une requete en parametre au serveur de base de données                                               */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 gboolean SQL_Select_to_json_node ( JsonNode *RootNode, gchar *array_name, gchar *format, ... )
  { va_list ap;

    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *chaine = g_try_malloc(taille+1);
    if (chaine)
     { va_start( ap, format );
       g_vsnprintf ( chaine, taille, format, ap );
       va_end ( ap );

       gboolean retour = SQL_Select_to_json_node_reel ( FALSE, RootNode, array_name, chaine );
       g_free(chaine);
       return(retour);
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* SQL_Select_to_JSON : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 gboolean SQL_Arch_to_json_node ( JsonNode *RootNode, gchar *array_name, gchar *format, ... )
  { va_list ap;

    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *chaine = g_try_malloc(taille+1);
    if (chaine)
     { va_start( ap, format );
       g_vsnprintf ( chaine, taille, format, ap );
       va_end ( ap );

       gboolean retour = SQL_Select_to_json_node_reel ( TRUE, RootNode, array_name, chaine );
       g_free(chaine);
       return(retour);
     }
    return(FALSE);
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
/* SQL_Write_new: Envoie une requete en parametre au serveur de base de données                                               */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 gboolean SQL_Write_new( gchar *format, ... )
  { gboolean retour = FALSE;
    va_list ap;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *chaine = g_try_malloc(taille+1);
    if (chaine)
     { va_start( ap, format );
       g_vsnprintf ( chaine, taille, format, ap );
       va_end ( ap );
       retour = SQL_Write ( chaine );
       g_free(chaine);
     }
    return(retour);
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
    Dls_data_set_AI ( "SYS", "DB_NBR_REQUEST", (gpointer)&ai_nbr_dbrequest, (gdouble)taille, TRUE );
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
/* SQL_read_from_file : Lance une requete SQL a partir d'un fichier                                                           */
/* Entrée: le nom de fichier sans le directory                                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *SQL_Read_from_file ( gchar *file )
  { struct stat stat_buf;
    Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: Loading DB %s", __func__, file );
    gchar filename[256];
    g_snprintf ( filename, sizeof(filename), "%s/%s", WTD_PKGDATADIR, file );

    if (stat ( filename, &stat_buf)==-1)
     { Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: Stat DB Error for %s", __func__, filename );
       return(FALSE);
     }

    gchar *db_content = g_try_malloc0 ( stat_buf.st_size+1 );
    if (!db_content)
     { Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: Memory DB Error for %s", __func__, filename );
       return(FALSE);
     }

    gint fd = open ( filename, O_RDONLY );
    if (!fd)
     { Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: Open DB Error for %s", __func__, filename );
       g_free(db_content);
       return(FALSE);
     }
    if (read ( fd, db_content, stat_buf.st_size ) != stat_buf.st_size)
     { Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: Read DB Error for %s", __func__, filename );
       g_free(db_content);
       return(FALSE);
     }
    close(fd);
    return(db_content);
  }
/******************************************************************************************************************************/
/* Update_database_schema: Vérifie la connexion et le schéma de la base de données                                            */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Update_database_schema ( void )
  { gchar requete[4096];
    struct DB *db;

    if (Config.instance_is_master != TRUE)                                                  /* Do not update DB if not master */
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "%s: Instance is not master. Don't update schema.", __func__ );
       return;
     }

    SQL_Write_new ("CREATE TABLE IF NOT EXISTS `mnemos_DI` ("
                   "`mnemos_DI_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                   "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
                   "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
                   "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                   "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                   "UNIQUE (`tech_id`,`acronyme`),"
                   "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    SQL_Write_new ("CREATE TABLE IF NOT EXISTS `mnemos_DO` ("
                   "`mnemos_DO_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                   "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
                   "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
                   "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                   "`etat` BOOLEAN NOT NULL DEFAULT '0',"
                   "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                   "UNIQUE (`tech_id`,`acronyme`),"
                   "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    SQL_Write_new ("CREATE TABLE IF NOT EXISTS `mnemos_AI` ("
                   "`mnemos_AI_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                   "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
                   "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
                   "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                   "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                   "`valeur` FLOAT NOT NULL DEFAULT '0',"
                   "`archivage` INT(11) NOT NULL DEFAULT '2',"
                   "UNIQUE (`tech_id`,`acronyme`),"
                   "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    SQL_Write_new ("CREATE TABLE IF NOT EXISTS `mappings` ("
                   "`mappings_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                   "`classe` VARCHAR(32) NULL DEFAULT NULL,"
                   "`thread_tech_id` VARCHAR(32) NOT NULL,"
                   "`thread_acronyme` VARCHAR(64) NOT NULL,"
                   "`tech_id` VARCHAR(32) NULL DEFAULT NULL,"
                   "`acronyme` VARCHAR(64) NULL DEFAULT NULL,"
                   "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
                   "UNIQUE (`thread_tech_id`,`thread_acronyme`),"
                   "UNIQUE (`tech_id`,`acronyme`),"
                   "UNIQUE (`thread_tech_id`,`thread_acronyme`,`tech_id`,`acronyme`),"
                   "FOREIGN KEY (`tech_id`,`acronyme`) REFERENCES `mnemos_DI` (`tech_id`,`acronyme`) ON DELETE SET NULL ON UPDATE CASCADE"
                   ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE utf8_unicode_ci AUTO_INCREMENT=1;");

    SQL_Write_new ("CREATE TABLE IF NOT EXISTS `syns` ("
                   "`syn_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                   "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                   "`parent_id` INT(11) NOT NULL,"
                   "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,"
                   "`image` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'syn_maison.png',"
                   "`page` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                   "`access_level` INT(11) NOT NULL DEFAULT '0',"
                   "`mode_affichage` TINYINT(1) NOT NULL DEFAULT '0',"
                   "FOREIGN KEY (`parent_id`) REFERENCES `syns` (`syn_id`) ON DELETE CASCADE ON UPDATE CASCADE"

                   ") ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");
    SQL_Write_new ("INSERT IGNORE INTO `syns` (`syn_id`, `parent_id`, `libelle`, `page`, `access_level` ) VALUES"
                   "(1, 1, 'Accueil', 'Defaut Page', 0);");

    SQL_Write_new ("CREATE TABLE IF NOT EXISTS `dls` ("
                   "`dls_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                   "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                   "`is_thread` tinyint(1) NOT NULL DEFAULT '0',"
                   "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                   "`package` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'custom',"
                   "`syn_id` INT(11) NOT NULL DEFAULT '0',"
                   "`name` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,"
                   "`shortname` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                   "`actif` tinyint(1) NOT NULL DEFAULT '0',"
                   "`compil_date` DATETIME NOT NULL DEFAULT NOW(),"
                   "`compil_status` INT(11) NOT NULL DEFAULT '0',"
                   "`nbr_compil` INT(11) NOT NULL DEFAULT '0',"
                   "`sourcecode` MEDIUMTEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */',"
                   "`errorlog` TEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT 'No Error',"
                   "`nbr_ligne` INT(11) NOT NULL DEFAULT '0',"
                   "`debug` TINYINT(1) NOT NULL DEFAULT '0',"
                   "FOREIGN KEY (`syn_id`) REFERENCES `syns` (`syn_id`) ON DELETE CASCADE ON UPDATE CASCADE"
                   ") ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    SQL_Write_new ("INSERT IGNORE INTO `dls` (`dls_id`, `syn_id`, `name`, `shortname`, `tech_id`, `actif`, `compil_date`, `compil_status` ) VALUES "
                   "(1, 1, 'Système', 'Système', 'SYS', FALSE, 0, 0);");

    JsonNode *RootNode = Json_node_create();
    if (!RootNode)
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "%s: Memory error. Don't update schema.", __func__ );
       return;
     }
    SQL_Select_to_json_node ( RootNode, NULL, "SELECT database_version FROM instances WHERE instance='%s'", g_get_host_name() );
    gint database_version;
    if (Json_has_member ( RootNode, "database_version" ) )
         { database_version = Json_get_int ( RootNode, "database_version" ); }
    else { database_version = 0; }
    json_node_unref(RootNode);

    Info_new( Config.log, Config.log_db, LOG_NOTICE,
             "%s: Actual Database_Version detected = %05d. Please wait while upgrading.", __func__, database_version );

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    if (database_version==0) goto fin;

    if (database_version < 6092)
     { SQL_Write_new ("ALTER TABLE mnemos_DO       CHANGE `id`     `mnemos_DO_id` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mnemos_DI       CHANGE `id`     `mnemos_DI_id` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mnemos_AI       CHANGE `id`     `mnemos_AI_id` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mnemos_AO       CHANGE `id`     `mnemos_AO_id` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mappings        CHANGE `id_map` `mappings_id` INT(11) NOT NULL AUTO_INCREMENT" );
     }

    if (database_version < 6093)
     { SQL_Write_new ("ALTER TABLE dls             CHANGE `id`     `dls_id` INT(11) NOT NULL AUTO_INCREMENT" ); }

    if (database_version < 6094)
     { SQL_Write_new ("ALTER TABLE syns            CHANGE `id`     `syn_id` INT(11) NOT NULL AUTO_INCREMENT" ); }
/* a prévoir:
       SQL_Write_new ("ALTER TABLE mnemos_BI       CHANGE `id`     `id_mnemos_BI` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mnemos_MONO     CHANGE `id`     `id_mnemos_MONO` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mnemos_CH       CHANGE `id`     `id_mnemos_CH` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mnemos_CI       CHANGE `id`     `id_mnemos_CI` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mnemos_HORLOGE  CHANGE `id`     `id_mnemos_HORLOGE` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mnemos_Tempos   CHANGE `id`     `id_mnemos_Tempo` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mnemos_R        CHANGE `id`     `id_mnemos_R` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE mnemos_WATCHDOG CHANGE `id`     `id_mnemos_WATCHDOG` INT(11) NOT NULL AUTO_INCREMENT" );
       SQL_Write_new ("ALTER TABLE tableau         CHANGE `id`     `id_tableau` INT(11) NOT NULL AUTO_INCREMENT" );
       "SELECT id,'VISUEL' AS classe, -1 AS classe_int,tech_id,acronyme,libelle, 'none' as unite FROM mnemos_VISUEL UNION "
       "SELECT id,'MESSAGE' AS classe, %d AS classe_int,tech_id,acronyme,libelle, 'none' as unite FROM msgs",
*/

fin:
    database_version = 6094;

    g_snprintf( requete, sizeof(requete), "CREATE OR REPLACE VIEW db_status AS SELECT "
                                          "(SELECT COUNT(*) FROM syns) AS nbr_syns, "
                                          "(SELECT COUNT(*) FROM syns_visuels) AS nbr_syns_visuels, "
                                          "(SELECT COUNT(*) FROM syns_liens) AS nbr_syns_liens, "
                                          "(SELECT COUNT(*) FROM dls) AS nbr_dls, "
                                          "(SELECT COUNT(*) FROM mnemos_DI) AS nbr_dls_di, "
                                          "(SELECT COUNT(*) FROM mnemos_DO) AS nbr_dls_do, "
                                          "(SELECT COUNT(*) FROM mnemos_AI) AS nbr_dls_ai, "
                                          "(SELECT COUNT(*) FROM mnemos_AO) AS nbr_dls_ao, "
                                          "(SELECT COUNT(*) FROM mnemos_BI) AS nbr_dls_bi, "
                                          "(SELECT COUNT(*) FROM mnemos_MONO) AS nbr_dls_mono, "
                                          "(SELECT SUM(dls.nbr_ligne) FROM dls) AS nbr_dls_lignes, "
                                          "(SELECT COUNT(*) FROM users) AS nbr_users, "
                                          "(SELECT COUNT(*) FROM msgs) AS nbr_msgs, "
                                          "(SELECT COUNT(*) FROM histo_msgs) AS nbr_histo_msgs, "
                                          "(SELECT COUNT(*) FROM audit_log) AS nbr_audit_log" );
    Lancer_requete_SQL ( db, requete );

    g_snprintf( requete, sizeof(requete),
       "CREATE OR REPLACE VIEW dictionnaire AS "
       "SELECT dls_id,'DLS' AS classe, -1 AS classe_int,tech_id,shortname as acronyme,name as libelle, 'none' as unite FROM dls UNION "
       "SELECT syn_id,'SYNOPTIQUE' AS classe, -1 AS classe_int,page as tech_id, NULL as acronyme,libelle, 'none' as unite FROM syns UNION "
       "SELECT mnemos_AI_id,'AI' AS classe, %d AS classe_int,tech_id,acronyme,libelle,unite FROM mnemos_AI UNION "
       "SELECT mnemos_DI_id,'DI' AS classe, %d AS classe_int,tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_DI UNION "
       "SELECT mnemos_DO_id,'DO' AS classe, %d AS classe_int,tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_DO UNION "
       "SELECT mnemos_AO_id,'AO' AS classe, %d AS classe_int,tech_id,acronyme,libelle, 'none' as unite FROM mnemos_AO UNION "
       "SELECT id,'BI' AS classe, 0 AS classe_int,tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_BI UNION "
       "SELECT id,'MONO' AS classe, 1 AS classe_int,tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_MONO UNION "
       "SELECT id,'CH' AS classe, %d AS classe_int,tech_id,acronyme,libelle, '1/10 secondes' as unite FROM mnemos_CH UNION "
       "SELECT id,'CI' AS classe, %d AS classe_int,tech_id,acronyme,libelle,unite FROM mnemos_CI UNION "
       "SELECT id,'HORLOGE' AS classe, %d AS classe_int,tech_id,acronyme,libelle, 'none' as unite FROM mnemos_HORLOGE UNION "
       "SELECT id,'TEMPO' AS classe, %d AS classe_int,tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_Tempo UNION "
       "SELECT id,'REGISTRE' AS classe, %d AS classe_int,tech_id,acronyme,libelle,unite FROM mnemos_R UNION "
       "SELECT id,'VISUEL' AS classe, -1 AS classe_int,tech_id,acronyme,libelle, 'none' as unite FROM mnemos_VISUEL UNION "
       "SELECT id,'WATCHDOG' AS classe, %d AS classe_int,tech_id,acronyme,libelle, '1/10 secondes' as unite FROM mnemos_WATCHDOG UNION "
       "SELECT id,'TABLEAU' AS classe, -1 AS classe_int, NULL AS tech_id, NULL AS acronyme, titre AS libelle, 'none' as unite FROM tableau UNION "
       "SELECT id,'MESSAGE' AS classe, %d AS classe_int,tech_id,acronyme,libelle, 'none' as unite FROM msgs",
        MNEMO_ENTREE_ANA, MNEMO_ENTREE, MNEMO_SORTIE, MNEMO_SORTIE_ANA, MNEMO_CPTH, MNEMO_CPT_IMP, MNEMO_HORLOGE,
        MNEMO_TEMPO, MNEMO_REGISTRE, MNEMO_WATCHDOG, MNEMO_MSG
      );
    Lancer_requete_SQL ( db, requete );
    Libere_DB_SQL(&db);

    if (SQL_Write_new ( "UPDATE instances SET database_version='%d' WHERE instance='%s'", database_version, g_get_host_name() ))
     { Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: updating Database_version to %d OK", __func__, database_version ); }
    else
     { Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: updating Database_version to %d FAILED", __func__, database_version ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
