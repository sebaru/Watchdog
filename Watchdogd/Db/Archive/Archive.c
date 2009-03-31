/**********************************************************************************************************/
/* Watchdogd/Db/ValANA/ValANA.c        Déclaration des fonctions pour la gestion des valeurs analogiques  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 21 aoû 2005 17:09:35 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>

 #include "Erreur.h"
 #include "Archive_DB.h"

/**********************************************************************************************************/
/* Ajouter_archDB: Ajout ou edition d'un entreeANA                                                      */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure arch                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Ajouter_archDB ( struct LOG *log, struct DB *db, struct ARCHDB *arch )
  { gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */

    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_archDB: recherche failed: query=null" );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(date_sec,date_usec,type,num,valeur) VALUES "
                "(%d,%d,%d,%d,%d)", NOM_TABLE_ARCH, arch->date_sec, arch->date_usec,
                arch->type, arch->num, arch->valeur );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Ajouter_archDB: ajout failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return;
     }
    /*else Info( log, DEBUG_DB, "Ajouter_archDB: ajout ok" );*/

    EndQueryDB( log, db, hquery );
  }
/**********************************************************************************************************/
/* Creer_db_arch: création des tables associées aux entreeANAs au fil de l'eau                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_arch ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_arch: Creation DB failed: query=null", NOM_TABLE_ARCH );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "`type` INT NOT NULL ,"
                                          "`num` INT NOT NULL ,"
                                          "`date_sec` INT NOT NULL ,"
                                          "`date_usec` INT NOT NULL ,"
                                          "`valeur` INT NOT NULL"
                                          ")",
                                          NOM_TABLE_ARCH );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_arch: création table failed", NOM_TABLE_ARCH );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_arch: succes création table", NOM_TABLE_ARCH );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_archDB: Recupération de la liste des ids des entreeANAs                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_archDB ( struct LOG *log, struct DB *db, guint type, guint num,
                             time_t date_deb, time_t date_fin )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[1024];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_archDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT `type`,`num`,`date_sec`,`date_usec`,`valeur`"
                " FROM %s WHERE `type`=%d AND `num`=%d AND `date_sec`>=%d AND `date_sec`<=%d"
                " ORDER BY `date_sec`,`date_usec` ASC",
                NOM_TABLE_ARCH, type, num, (gint)date_deb, (gint)date_fin );

/*    g_snprintf( requete, sizeof(requete),
                "SELECT `type`,`num`,`date_sec`,`date_usec`,`valeur` FROM"
                " (SELECT `type`,`num`,`date_sec`,`date_usec`,`valeur`"
                "  FROM %s WHERE `type`=%d AND `num`=%d ORDER BY `date_sec`,`date_usec` DESC"
                "  LIMIT %d) AS `result`"
                " ORDER BY `date_sec`,`date_usec`",
                NOM_TABLE_ARCH, type, num, TAILLEBUF_HISTO_EANA );
*/

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_archDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info_c( log, DEBUG_DB, "Recuperer_archDB: recherche ok", requete );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_archDB: Recupération de la liste des ids des entreeANAs                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ARCHDB *Recuperer_archDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar date_sec[15], date_usec[15], num[15], type[15], valeur[15];
    struct ARCHDB *arch;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &type, sizeof(type), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_archDB_suite: erreur bind du type" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &num, sizeof(num), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_archDB_suite: erreur bind du num" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &date_sec, sizeof(date_sec), NULL );              /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_archDB_suite: erreur bind de date_sec" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &date_usec, sizeof(date_usec), NULL );            /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_archDB_suite: erreur bind de date_usec" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &valeur, sizeof(valeur), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_archDB_suite: erreur bind du valeur" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { Info( log, DEBUG_DB, "Recuperer_archDB_suite: plus d'enregistrement" );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    arch = (struct ARCHDB *)g_malloc0( sizeof(struct ARCHDB) );
    if (!arch) Info( log, DEBUG_MEM, "Recuperer_archDB_suite: Erreur allocation mémoire" );
    else
     { arch->date_sec  = atoi(date_sec);
       arch->date_usec = atoi(date_usec);
       arch->type      = atoi(type);
       arch->num       = atoi(num);
       arch->valeur    = atoi(valeur);
     }
    return(arch);
  }
/*--------------------------------------------------------------------------------------------------------*/
