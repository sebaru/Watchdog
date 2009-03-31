/**********************************************************************************************************/
/* Commun/Utilisateur/login_failed.c    Interface DB mots de passe pour watchdog2.0                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mer 30 avr 2003 14:19:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include <stdlib.h>

 #include <sql.h>                                             /* Entetes de gestion de la base de données */
 #include <sqlext.h> 
 #include <sqltypes.h>

 #include "Erreur.h"
 #include "Utilisateur_DB.h"

/************************************ Prototypes des fonctions ********************************************/
 #include "proto_util.h"

/**********************************************************************************************************/
/* Get_login_failed: Recupere la valeur du login failed                                             */
/* Entrées: un log, une db et nom d'utilisateur                                                           */
/* Sortie: le nombre de login failed, -1 si erreur                                                        */
/**********************************************************************************************************/
 gint Get_login_failed( struct LOG *log, struct DB *db, guint id )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar login_from_sql[10];
    SQLINTEGER nbr;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Get_login_failed: update failed: query=null", id );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &login_from_sql, sizeof(login_from_sql), NULL );  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Get_login_failed: erreur bind du login_failed" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT login_failed FROM %s WHERE id=%d",
                NOM_TABLE_UTIL, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Get_login_failed: recherche failed", id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info_n( log, DEBUG_DB, "Get_login_failed: recherche ok", id );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Get_login_failed: Login_failed non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Get_login_failed: Multiple solution", id );

    SQLFetch(hquery);
    EndQueryDB( log, db, hquery );
    return( atoi( login_from_sql ) );
  }
/**********************************************************************************************************/
/* Ajouter_one_login_failed: Ajoute 1 au login failed de l'utilisateur                                    */
/* Entrées: un log, une db et nom d'utilisateur, un max_login_failed                                      */
/* Sortie: le nombre de login failed                                                                      */
/**********************************************************************************************************/
 gboolean Ajouter_one_login_failed( struct LOG *log, struct DB *db, guint id, gint max_login_failed )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Ajouter_one_login_failed: update failed: query=null", id );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET login_failed = login_failed+1 WHERE id=%d",
                NOM_TABLE_UTIL, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Ajouter_one_login_failed: update failed", id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Ajouter_one_login_failed: update ok", id );

    EndQueryDB( log, db, hquery );
    if (Get_login_failed( log, db, id )>=max_login_failed)                     /* Desactivation du compte */
     { Info_n( log, DEBUG_DB, "Desactivation compte sur trop login failed", id );
       Set_compte_actif( log, db, id, FALSE );
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Raz_login_failed: Reset le login failed d'un utilisateur                                               */
/* Entrées: un log, une db et nom d'utilisateur                                                           */
/* Sortie: le nombre de login failed                                                                      */
/**********************************************************************************************************/
 gboolean Raz_login_failed( struct LOG *log, struct DB *db, guint id )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Raz_login_failed: recherche failed: query=null", id );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET login_failed = 0 WHERE id=%d",
                NOM_TABLE_UTIL, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Raz_login_failed: update failed", id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Raz_login_failed: update ok", id );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
