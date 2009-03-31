/**********************************************************************************************************/
/* Commun/Utilisateur/actif.c    Gestion de l'activité des comptes Watchdog                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 27 avr 2003 12:16:38 CEST */
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
/* Set_compte_actif: positionne le flag enable du compte à vrai ou faux                                   */
/* Entrées: un log, une db et un id d'utilisateur et un flag                                              */
/* Sortie: boolean false si probleme                                                                      */
/**********************************************************************************************************/
 gboolean Set_compte_actif( struct LOG *log, struct DB *db, guint id, gboolean enable )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Set_compte_actif: recherche failed: query=null", id );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET enable = '%d', login_failed = 0 WHERE id=%d",
                NOM_TABLE_UTIL, enable, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Set_compte_actif: update failed", id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Set_compte_actif: update ok", id );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
