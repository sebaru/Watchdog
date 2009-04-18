/**********************************************************************************************************/
/* Watchdogd/Db/Icones/Icones.c        Déclaration des fonctions pour la gestion des icones               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 30 sep 2003 10:38:04 CEST */
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
 #include "Icones_DB.h"
 #include "Synoptiques_DB.h"

/**********************************************************************************************************/
/* Retirer_msgDB: Elimination d'un message                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_iconeDB ( struct LOG *log, struct DB *db, struct CMD_ID_ICONE *icone )
  { gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_iconeDB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_ICONE, icone->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_iconeDB: elimination failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_iconeDB: elimination ok" );

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE icone=%d", NOM_TABLE_MOTIF, icone->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_iconeDB: elimination syns failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_iconeDB: elimination syns ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Max_id_iconesDB: Renvoie l'id maximum utilisé + 1 des icones                                           */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_iconesDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_iconesDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_iconesDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_ICONE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_iconesDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_iconesDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr!=0)
     { SQLFetch( hquery );
       id = atoi(id_from_sql) + 1;
     }
    else id = 0;
    EndQueryDB( log, db, hquery );
    return( id );
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_iconeDB ( struct LOG *log, struct DB *db, struct CMD_ADD_ICONE *icone )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *libelle;
    gint id;

    id = Max_id_iconesDB ( log, db );
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_iconeDB: recherche failed: query=null" );
       return(-1);
     }

    libelle = Normaliser_chaine ( log, icone->libelle );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_iconeDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,libelle,id_classe) VALUES "
                "(%d,'%s',%d)", NOM_TABLE_ICONE, id, libelle, icone->id_classe );
    g_free(libelle);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Ajouter_iconeDB: ajout failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Ajouter_iconeDB: ajout ok" );

    EndQueryDB( log, db, hquery );
    return(id);
  }
/**********************************************************************************************************/
/* Creer_db_icone: création des tables associées aux icones                                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_icone ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_icone: Creation DB failed: query=null", NOM_TABLE_ICONE );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  libelle VARCHAR(%d) NOT NULL,"
                                          "  id_classe INTEGER NOT NULL"
                                          ");",
                                          NOM_TABLE_ICONE, NBR_CARAC_LIBELLE_ICONE_UTF8+1 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_icone: création table failed", NOM_TABLE_ICONE );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_icone: succes création table", NOM_TABLE_ICONE );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_iconeDB ( struct LOG *log, struct DB *db, guint classe )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[200];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_iconeDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,id_classe"
                " FROM %s WHERE id_classe=%d ORDER BY id", NOM_TABLE_ICONE, classe );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_iconeDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_iconeDB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ICONEDB *Recuperer_iconeDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id_from_sql[10], libelle[NBR_CARAC_LIBELLE_ICONE_UTF8+1];
    gchar id_classe[10];
    struct ICONEDB *icone;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_iconeDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_iconeDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &id_classe, sizeof(id_classe), NULL );            /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_iconeDB_suite: erreur bind du id_classe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    icone = (struct ICONEDB *)g_malloc0( sizeof(struct ICONEDB) );
    if (!icone) Info( log, DEBUG_MEM, "Recuperer_iconeDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( icone->libelle, libelle, sizeof(icone->libelle) );            /* Recopie dans la structure */
       icone->id          = atoi(id_from_sql);
       icone->id_classe   = atoi(id_classe);
     }
    return(icone);
  }
/**********************************************************************************************************/
/* Rechercher_iconeDB: Recupération du icone dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ICONEDB *Rechercher_iconeDB ( struct LOG *log, struct DB *db, guint id )
  { gchar libelle[NBR_CARAC_LIBELLE_ICONE_UTF8+1];
    gchar id_classe[10];
    struct ICONEDB *icone;
    gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_iconeDB: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_iconeDB: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &id_classe, sizeof(id_classe), NULL );            /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_iconeDB: erreur bind du id_classe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle,id_classe FROM %s WHERE id=%d", NOM_TABLE_ICONE, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_iconeDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_iconeDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_iconeDB: Icone non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_iconeDB: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    icone = (struct ICONEDB *)g_malloc0( sizeof(struct ICONEDB) );
    if (!icone)
     { Info( log, DEBUG_MEM, "Rechercher_iconeDB: Mem error" );
       return(NULL);
     }

    icone->id          = id;
    memcpy( icone->libelle, libelle, sizeof(icone->libelle) );               /* Recopie dans la structure */
    icone->id_classe   = atoi(id_classe);
    return(icone);
  }
/**********************************************************************************************************/
/* Modifier_iconeDB: Modification d'un icone Watchdog                                                     */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_iconeDB( struct LOG *log, struct DB *db, struct CMD_EDIT_ICONE *icone )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gchar *libelle;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Modifier_iconeDB: ajout failed: query=null", icone->id );
       return(FALSE);
     }

    libelle = Normaliser_chaine ( log, icone->libelle );
    if (!libelle)
     { Info( log, DEBUG_DB, "Modifier_iconeDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s',id_classe=%d WHERE id=%d",
                NOM_TABLE_ICONE, libelle, icone->id_classe, icone->id );
    g_free(libelle);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Modifier_iconeDB: Modif failed", icone->id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Modifier_iconeDB: succes modif", icone->id );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
