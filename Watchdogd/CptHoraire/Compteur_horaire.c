/**********************************************************************************************************/
/* Watchdogd/Db/CptHoraire/Compteur_horaire.c        Déclaration des fonctions pour la gestion des cpt_h  */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 14 fév 2006 15:03:51 CET */
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
 #include "Cst_dls.h"
 #include "Cpth_DB.h"

/**********************************************************************************************************/
/* Ajouter_cpthDB: Ajout ou edition d'un entreeANA                                                        */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure cpth                          */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Updater_cpthDB ( struct LOG *log, struct DB *db, struct CPTH_DB *cpth )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */

    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Updater_cpthDB: recherche failed: query=null" );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET val=%d WHERE id=%d;", NOM_TABLE_CPTH, cpth->valeur, cpth->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Updater_cpthDB: ajout failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return;
     }
    EndQueryDB( log, db, hquery );
  }
/**********************************************************************************************************/
/* Creer_db_cpth: création des tables associées aux entreeANAs au fil de l'eau                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_cpth ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;
    int cpt;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_cpth: Creation DB failed: query=null", NOM_TABLE_CPTH );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  val INTEGER NOT NULL"
                                          ");",
                                          NOM_TABLE_CPTH );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_cpth: création table failed", NOM_TABLE_CPTH );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_cpth: succes création table", NOM_TABLE_CPTH );

    for (cpt=0; cpt<NBR_COMPTEUR_H; cpt++)          /* Création des bits internes dans la base de données */
     { g_snprintf( requete, sizeof(requete),                                      /* Requete SQL */
                   "INSERT INTO %s(id,val) VALUES "
                   "(%d,0)", NOM_TABLE_CPTH, cpt );

       retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );       /* Execution de la requete SQL */
       if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
        { Info( log, DEBUG_DB, "Creer_db_cpth: ajout failed" );
          PrintErrQueryDB( log, db, hquery );
          EndQueryDB( log, db, hquery );
          return(FALSE);
        }
     }

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_cpthDB: Recupération de la liste des ids des entreeANAs                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CPTH_DB *Rechercher_cpthDB ( struct LOG *log, struct DB *db, guint id )
  { gchar valeur[20];
    struct CPTH_DB *cpth;
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[200];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_cpthDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT val"
                " FROM %s WHERE id=%d", NOM_TABLE_CPTH, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Rechercher_cpthDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    /*else Info_c( log, DEBUG_DB, "Rechercher_cpthDB: recherche ok", requete );*/

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &valeur, sizeof(valeur), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_cpthDB: erreur bind du valeur" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    EndQueryDB( log, db, hquery );
    cpth = (struct CPTH_DB *)g_malloc0( sizeof(struct CPTH_DB) );
    if (!cpth) Info( log, DEBUG_MEM, "Rechercher_cpthDB: Erreur allocation mémoire" );
    else
     { cpth->id     = id;
       cpth->valeur = atoi(valeur);
     }
    return(cpth);
  }
/*--------------------------------------------------------------------------------------------------------*/
