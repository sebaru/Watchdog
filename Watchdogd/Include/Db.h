/**********************************************************************************************************/
/* Include/Db.h           Gestion des bases de données Watchdog via ODBC                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mer 22 avr 2009 23:18:36 CEST */
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

 struct DB
  { MYSQL *mysql;
    MYSQL_RES *result;
    gint nbr_result;
    gboolean free;                                                    /* Le resultat est-il free ou non ? */
    MYSQL_ROW row;
  };
/************************************* Prototypes de fonctions ********************************************/
 extern gchar *Init_db_watchdog( void );                                                 /* Dans initdb.c */
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
