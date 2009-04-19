/**********************************************************************************************************/
/* Watchdog/db.c          Gestion des connexions à la base de données                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 18 avr 2009 00:44:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Db.c
 * This file is part of <program name>
 *
 * Copyright (C) 2009 - 
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #include <glib.h>

 #include <sql.h>                                             /* Entetes de gestion de la base de données */
 #include <sqlext.h> 
 #include <sqltypes.h>
 #include <string.h>

/******************************************** Chargement des prototypes ***********************************/
 #include "watchdogd.h"
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */

/**********************************************************************************************************/
/* Normaliser_chaine: Normalise les chaines ( remplace ' par \', " par "" )                               */
/* Entrées: un commentaire (gchar *)                                                                      */
/* Sortie: boolean false si probleme                                                                      */
/**********************************************************************************************************/
 gchar *Normaliser_chaine( struct LOG *log, gchar *pre_comment )
  { gchar *comment, *source, *cible;
    gunichar car;

    g_utf8_validate( pre_comment, -1, NULL );                                       /* Validate la chaine */
    comment = g_malloc0( (2*g_utf8_strlen(pre_comment, -1))*6 + 1 );  /* Au pire, ts les car sont doublés */
                                                                                  /* *6 pour gerer l'utf8 */
    if (!comment)
     { Info_c( log, DEBUG_MEM, "Normaliser_chaine: erreur mémoire", pre_comment );
       return(NULL);
     }
    source = pre_comment;
    cible  = comment;
    
    while( (car = g_utf8_get_char( source )) )
     { if ( car == '\'' )                                                 /* Dédoublage de la simple cote */
        { g_utf8_strncpy( cible, "\'", 1 ); cible = g_utf8_next_char( cible );
          g_utf8_strncpy( cible, "\'", 1 ); cible = g_utf8_next_char( cible );
        }
       else if (car =='\\')                                                    /* Dédoublage du backspace */
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
/**********************************************************************************************************/
/* PrintErr: affichage des informations sur la dernière opérations sur la base de données                 */
/* Entrée: un log, un type de handle, un handle                                                           */
/**********************************************************************************************************/
 static void PrintErr ( struct LOG *log, struct DB *db, SQLSMALLINT type, SQLHANDLE handle )
  { SQLSMALLINT len;    SQLINTEGER err;
    gchar status[10];
    SQLGetDiagRec( type, handle, 1, status, &err, db->last_err, TAILLE_MAX_MSG_ERREURDB, &len);
    Info( log, DEBUG_DB, db->last_err );
  }
/**********************************************************************************************************/
/* PrintErrDB: affichage des erreurs de retour de la DB                                                   */
/* Entrée: un log et un handle de base de données                                                         */
/**********************************************************************************************************/
 void PrintErrDB ( struct LOG *log, struct DB *db )
  { PrintErr( log, db, SQL_HANDLE_DBC, db->hdb ); }

/**********************************************************************************************************/
/* PrintErrQueryDB: affichage des erreurs de retour des requetes                                          */
/**********************************************************************************************************/
 void PrintErrQueryDB ( struct LOG *log, struct DB *Db, SQLHSTMT hquery_err )
  { PrintErr( log, Db, SQL_HANDLE_STMT, hquery_err ); }

/**********************************************************************************************************/
/* ConnexionDB: essai de connexion à la DataBase db via le DSN db                                         */
/* Sortie: une structure DB ou NULL si erreur                                                             */
/**********************************************************************************************************/
 struct DB *ConnexionDB ( struct LOG *log, gchar *dsn, gchar *user, gchar *password )
  { struct DB *db;
    SQLHENV Odbc;
    SQLHDBC Db;
    long retour;

    retour = SQLAllocHandle ( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &Odbc );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "ConnexionDB: Erreur d'allocation mémoire HENV" );
       return(NULL);
     }

    retour = SQLSetEnvAttr( Odbc, SQL_ATTR_ODBC_VERSION,              /* On définit nos besoins (version) */
                           (void *)SQL_OV_ODBC3, 0); 
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { SQLFreeHandle( SQL_HANDLE_ENV, Odbc );
       Info( log, DEBUG_DB, "ConnexionDB: Erreur de version ODBC (3 requis)" );
       return(NULL);
     }
                                                                           /* Préparation de la connexion */
    retour = SQLAllocHandle( SQL_HANDLE_DBC, Odbc, &Db ); 
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { SQLFreeHandle(SQL_HANDLE_ENV, Odbc);
       Info( log, DEBUG_DB, "ConnexionDB: Erreur d'allocation mémoire HDBC" );
       return(NULL);
     }
  /*SQLSetConnectAttr( Db, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)100, 0);                    /* Timeout reseau */
  /*  SQLSetConnectAttr( Db, SQL_OPT_TRACE, (SQLPOINTER *)SQL_OPT_TRACE_ON, 0);
    SQLSetConnectAttr( Db, SQL_OPT_TRACEFILE, "traceSQL.seb", 0);*/

                                                            /* Test de connexion à la DSN proprement dite */
    retour = SQLConnect( Db, (SQLCHAR*) dsn, SQL_NTS,                                /* DSN -> DBWatchdog */
                             (SQLCHAR*) user, SQL_NTS,                                        /* Username */
                             (SQLCHAR*) password, SQL_NTS);                                   /* Password */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { PrintErrDB( log, Db );
       Info_n( log, DEBUG_DB, "ConnexionDB: Erreur de connexion", retour );
       SQLFreeHandle( SQL_HANDLE_DBC, Db );
       SQLFreeHandle( SQL_HANDLE_ENV, Odbc);
       return(NULL);
     }

    retour = SQLDisconnect( Db );                                                          /* Deconnexion */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { PrintErrDB( log, db->hdb );
       Info_n( log, DEBUG_DB, "DB: EndQueryDB: Erreur de deconnexion", retour );
       SQLFreeHandle( SQL_HANDLE_DBC, Db );
       SQLFreeHandle( SQL_HANDLE_ENV, Odbc);
       return(NULL);
     }
     
    db = (struct DB *)g_malloc0( sizeof(struct DB) );
    if (!db)                                                          /* Si probleme d'allocation mémoire */
     { SQLFreeHandle( SQL_HANDLE_DBC, Db );
       SQLFreeHandle( SQL_HANDLE_ENV, Odbc );
       Info( log, DEBUG_DB, "ConnexionDB: Erreur allocation mémoire struct DB" );
       return(NULL);
     }

    db->hodbc       = Odbc;                                                    /* Sauvegarde des donnéees */
    db->hdb         = Db;
    db->dsn         = g_strdup( dsn );
    g_snprintf( db->db_username, sizeof(db->db_username), "%s", user );
    g_snprintf( db->db_database, sizeof(db->db_database), "%s", db->dsn );
    g_snprintf( db->db_password, sizeof(db->db_password), "%s", password );
    Info_c( log, DEBUG_DB, "ConnexionDB: Connexion effective DB", db->dsn );
    Info_c( log, DEBUG_DB, "                          avec user", user );
    return(db);
  }
/**********************************************************************************************************/
/* Init_DB_SQL: essai de connexion à la DataBase db                                                       */
/* Entrée: toutes les infos necessaires a la connexion                                                    */
/* Sortie: une structure DB de référence                                                                  */
/**********************************************************************************************************/
 struct DB *Init_DB_SQL ( struct LOG *log, gchar *host, gchar *database,
                          gchar *user, gchar *password, guint port )
  { struct DB *db;
    db = (struct DB *)g_malloc0( sizeof(struct DB) );
    if (!db)                                                          /* Si probleme d'allocation mémoire */
     { Info( log, DEBUG_DB, "Init_DB_SQL: Erreur allocation mémoire struct DB" );
       return(NULL);
     }

    db->mysql = mysql_init(NULL);
    if (!db->mysql)
     { Info_c( log, DEBUG_DB, "Init_DB_SQL: Probleme d'initialisation mysql_init",
                              (char *) mysql_error(db->mysql)  );
       g_free(db);
       return (NULL);
     }
    if ( ! mysql_real_connect( db->mysql, host, user, password, database, port, NULL, 0 ) )
     { Info_c( log, DEBUG_DB, "Init_DB_SQL: Probleme de connexion à la base",
                              (char *) mysql_error(db->mysql)  );
       mysql_close( db->mysql );
       g_free(db);
       return (NULL);
     }
    Info_c( log, DEBUG_DB, "Init_DB_SQL: Connexion effective DB", database );
    Info_c( log, DEBUG_DB, "                          avec user", user );
    return(db);
  }
/**********************************************************************************************************/
/* Libere_DB_SQL : Se deconnecte d'une base de données en parametre                                       */
/* Entrée: La DB                                                                                          */
/**********************************************************************************************************/
 void Libere_DB_SQL( struct LOG *log, struct DB **adr_db )
  { struct DB *db;
    if (!(adr_db && *adr_db)) return;

    db = *adr_db;
    mysql_close( db->mysql );
    Info( log, DEBUG_DB, "Libere_DB_SQL: Deconnexion effective" );
    g_free( db );
    *adr_db = NULL;
  }
/**********************************************************************************************************/
/* Lancer_requete_SQL : lance une requete en parametre, sur la structure de reférence                     */
/* Entrée: La DB, la requete                                                                              */
/* Sortie: TRUE si pas de souci                                                                           */
/**********************************************************************************************************/
 gboolean Lancer_requete_SQL ( struct LOG *log, struct DB *db, gchar *requete )
  { if (!db) return(FALSE);
    if ( mysql_query ( db->mysql, requete ) )
     { Info_c( Config.log, DEBUG_DB, "Lancer_requete_SQL: requete failed",
               (char *)mysql_error(db->mysql) );
       return(FALSE);
     }
    else 
     { Info_c( Config.log, DEBUG_DB, "Lancer_requete_SQL: requete OK", requete ); }

    if ( ! strncmp ( requete, "SELECT", 6 ) )
     { db->result = mysql_store_result ( db->mysql );
       if ( ! db->result )
        { Info_c( Config.log, DEBUG_DB, "Lancer_requete_SQL: store_result failed",
                  (char *) mysql_error(db->mysql) );
          db->nbr_result = 0;
        }
       else 
        { Info( Config.log, DEBUG_DB, "Lancer_requete_SQL: store_result OK" );
          db->nbr_result = mysql_num_rows ( db->result );
        }
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_ligne_SQL: Renvoie les lignes resultat, une par une                                          */
/* Entrée: la DB                                                                                          */
/* Sortie: La ligne ou NULL si il n'y en en plus                                                          */
/**********************************************************************************************************/
 MYSQL_ROW Recuperer_ligne_SQL ( struct LOG *log, struct DB *db )
  { if (!db) return(NULL);
    db->row = mysql_fetch_row(db->result);
    return( db->row );
  }
/**********************************************************************************************************/
/* Recuperer_last_ID_SQL: Renvoie le dernier ID inséré                                                    */
/* Entrée: la DB                                                                                          */
/* Sortie: Le dernier ID                                                                                  */
/**********************************************************************************************************/
 guint Recuperer_last_ID_SQL ( struct LOG *log, struct DB *db )
  { if (!db) return(0);
    return ( mysql_insert_id(db->mysql) );
  }
/**********************************************************************************************************/
/* Liberer_resultat_SQL: Libere la mémoire affectée au resultat SQL                                       */
/* Entrée: la DB                                                                                          */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Liberer_resultat_SQL ( struct LOG *log, struct DB *db )
  { if (db) mysql_free_result( db->result );
  }
/**********************************************************************************************************/
/* DeconnexionDB: Deconnexion et libération mémoire de la structure DB en paramètres                      */
/**********************************************************************************************************/
 void DeconnexionDB( struct LOG *log, struct DB **adr_db )
  { struct DB *db;
    if (!(adr_db && *adr_db)) return;

    db = *adr_db;
    Info_c( log, DEBUG_DB, "DeconnexionDB: Deconnexion effective", db->dsn );
    SQLFreeHandle( SQL_HANDLE_DBC, db->hdb );
    SQLFreeHandle( SQL_HANDLE_ENV, db->hodbc );
    g_free( db->dsn );
    g_free( db );
    *adr_db = NULL;
  }
/**********************************************************************************************************/
/* NewQueryDB: renvoie un pointeur sur la nouvelle requete db                                             */
/* entrée: un log et une db                                                                               */
/* sortie: un pointeur query, ou null si pb                                                               */
/**********************************************************************************************************/
 SQLHSTMT NewQueryDB ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    long retour;

    /*Info_n( log, DEBUG_DB, "DB: NewqueryDB: db = ", db );*/
    if ( db->nbr_query != 0 )
     { Info_n( log, DEBUG_DB, "DB: NewqueryDB: nbr_query!=0 !!", db->nbr_query );
     }

    retour = SQLConnect( db->hdb, (SQLCHAR*) db->dsn, SQL_NTS,                       /* DSN -> DBWatchdog */
                             (SQLCHAR*) db->db_username, SQL_NTS,                             /* Username */
                             (SQLCHAR*) db->db_password, SQL_NTS);                            /* Password */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { PrintErrDB( log, db->hdb );
       Info_n( log, DEBUG_DB, "DB: NewqueryDB: Erreur de connexion", retour );
       return(NULL);
     }

    retour = SQLAllocHandle( SQL_HANDLE_STMT, db->hdb, &hquery );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "DB: NewQueryDB: pb allocation pointeur" );
       PrintErrQueryDB( log, db, hquery );
       return(NULL);
     }
    db->nbr_query++;
    return(hquery);
  }
/**********************************************************************************************************/
/* NewQueryDB: renvoie un pointeur sur la nouvelle requete db                                             */
/* entrée: un log et une db                                                                               */
/* sortie: un pointeur query, ou null si pb                                                               */
/**********************************************************************************************************/
 void EndQueryDB ( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { long retour;

    /*Info_n( log, DEBUG_DB, "DB: EndqueryDB: db = ", db );*/
    if ( db->nbr_query != 1 )
     { Info_n( log, DEBUG_DB, "DB: EndqueryDB: nbr_query!=1 !!", db->nbr_query );
     }
    db->nbr_query--;

    retour = SQLFreeHandle( SQL_HANDLE_STMT, hquery );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { PrintErrDB( log, db->hdb );
       Info_n( log, DEBUG_DB, "DB: EndQueryDB: Erreur de FreeHandle", retour );
       return;
     }

    retour = SQLDisconnect( db->hdb );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { PrintErrDB( log, db->hdb );
       Info_n( log, DEBUG_DB, "DB: EndQueryDB: Erreur de deconnexion", retour );
       return;
     }
  }
/**********************************************************************************************************/
/* initDb_watchdog: initialisation des tables de la Db_watchdog watchdog                                  */
/* entrées: un log et une database                                                                        */
/* sortie: Un pointeur sur une chaine de caractere si probleme, null sinon                                */
/**********************************************************************************************************/
 gchar *Init_db_watchdog( void )
  { 
return("test");
  } 
/*--------------------------------------------------------------------------------------------------------*/
