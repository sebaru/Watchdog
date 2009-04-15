/**********************************************************************************************************/
/* Include/Db.h           Gestion des bases de données Watchdog via ODBC                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 01 jun 2003 18:08:23 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #ifndef _DB_H_
  #define _DB_H_

 #include <sql.h>                                             /* Entetes de gestion de la base de données */
 #include <sqlext.h> 
 #include <sqltypes.h>

 #include "Erreur.h"
 #include "Config.h"

 #define TAILLE_MAX_MSG_ERREURDB   2048

 struct DB
  { SQLHDBC hdb;
    SQLHENV hodbc;
    guint nbr_query;
    gchar *dsn;
    gchar last_err[ TAILLE_MAX_MSG_ERREURDB + 1 ];
    gchar db_username[ TAILLE_DB_NAME + 1 ];
    gchar db_password[ TAILLE_DB_PASSWORD + 1 ];
  };
/************************************* Prototypes de fonctions ********************************************/
 extern gchar *Init_db_watchdog( void );                                                 /* Dans initdb.c */
 extern SQLHSTMT NewQueryDB ( struct LOG *log, struct DB *db );
 extern void EndQueryDB ( struct LOG *log, struct DB *db, SQLHSTMT hquery );
 extern void PrintErrDB ( struct LOG *log, struct DB *db );
 extern void PrintErrQueryDB ( struct LOG *log, struct DB *Db, SQLHSTMT hquery_err );
 extern struct DB *ConnexionDB ( struct LOG *log, gchar *dsn, gchar *user, gchar *password );
 extern void DeconnexionDB( struct LOG *log, struct DB **adr_db );

 extern gchar *Normaliser_chaine( struct LOG *log, gchar *pre_comment );

 #endif
/*--------------------------------------------------------------------------------------------------------*/
