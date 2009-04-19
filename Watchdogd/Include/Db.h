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
 #include <mysql.h>

 #include "Erreur.h"

 #define TAILLE_DB_HOST           30
 #define TAILLE_DB_USERNAME       30
 #define TAILLE_DB_PASSWORD       30
 #define TAILLE_DB_DATABASE       20
 #define TAILLE_MAX_MSG_ERREURDB   2048

 struct DB
  { SQLHDBC hdb;/* a virer */
    SQLHENV hodbc;/* a virer */
    guint nbr_query;/* a virer */
    gchar *dsn;/* a virer */
    gchar last_err[ TAILLE_MAX_MSG_ERREURDB + 1 ];/* a virer */
    gchar db_username[ TAILLE_DB_USERNAME + 1 ]; /* a virer */
    gchar db_password[ TAILLE_DB_PASSWORD + 1 ];/* a virer */
    gchar db_database [ TAILLE_DB_DATABASE + 1 ];/* a virer */
    MYSQL *mysql;
    MYSQL_RES *result;
    gint nbr_result;
    gboolean free;                                                    /* Le resultat est-il free ou non ? */
    MYSQL_ROW row;
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

 extern struct DB *Init_DB_SQL ( struct LOG *log, gchar *host, gchar *database,
                                 gchar *user, gchar *password, guint port );
 extern void Libere_DB_SQL( struct LOG *log, struct DB **adr_db );
 extern gboolean Lancer_requete_SQL ( struct LOG *log, struct DB *db, gchar *requete );
 extern MYSQL_ROW Recuperer_ligne_SQL ( struct LOG *log, struct DB *db );
 extern void Liberer_resultat_SQL ( struct LOG *log, struct DB *db );
 extern guint Recuperer_last_ID_SQL ( struct LOG *log, struct DB *db );

 #endif
/*--------------------------------------------------------------------------------------------------------*/
