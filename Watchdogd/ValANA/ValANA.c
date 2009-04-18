/**********************************************************************************************************/
/* Watchdogd/Db/ValANA/ValANA.c        Déclaration des fonctions pour la gestion des valeurs analogiques  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 21 aoû 2005 17:09:35 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "Erreur.h"
 #include "ValANA_DB.h"

/**********************************************************************************************************/
/* Ajouter_valanaDB: Ajout ou edition d'un entreeANA                                                      */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure valana                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Ajouter_valANADB ( struct LOG *log, struct DB *db, struct VALANADB *valana )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */

    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_valANADB: recherche failed: query=null" );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(date,num,val) VALUES "
                "(%d,%d,%d)", NOM_TABLE_VALANA, valana->date,
                valana->num, valana->valeur );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Ajouter_valANADB: ajout failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return;
     }
    /*else Info( log, DEBUG_DB, "Ajouter_valANADB: ajout ok" );*/

    EndQueryDB( log, db, hquery );
  }
/**********************************************************************************************************/
/* Creer_db_valana: création des tables associées aux entreeANAs au fil de l'eau                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_valANA ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_valANA: Creation DB failed: query=null", NOM_TABLE_VALANA );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( num INTEGER NOT NULL,"
                                          "  date INTEGER NOT NULL,"
                                          "  val INTEGER NOT NULL"
                                          ");",
                                          NOM_TABLE_VALANA );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_valANA: création table failed", NOM_TABLE_VALANA );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_valANA: succes création table", NOM_TABLE_VALANA );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_valanaDB: Recupération de la liste des ids des entreeANAs                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_valANADB ( struct LOG *log, struct DB *db, guint id )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[200];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_valANADB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT num,date,val"
                " FROM %s WHERE num=%d ORDER BY date DESC LIMIT 600", NOM_TABLE_VALANA, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_valANADB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info_c( log, DEBUG_DB, "Recuperer_valANADB: recherche ok", requete );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_valanaDB: Recupération de la liste des ids des entreeANAs                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct VALANADB *Recuperer_valANADB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar date[10], num[10], valeur[10];
    struct VALANADB *valana;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &num, sizeof(num), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_valANADB_suite: erreur bind du num" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &date, sizeof(date), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_valANADB_suite: erreur bind de date" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }


    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &valeur, sizeof(valeur), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_valANADB_suite: erreur bind du valeur" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    valana = (struct VALANADB *)g_malloc0( sizeof(struct VALANADB) );
    if (!valana) Info( log, DEBUG_MEM, "Recuperer_valANADB_suite: Erreur allocation mémoire" );
    else
     { valana->date   = atoi(date);
       valana->num    = atoi(num);
       valana->valeur = atoi(valeur);
     }
    return(valana);
  }
/*--------------------------------------------------------------------------------------------------------*/
