/**********************************************************************************************************/
/* Watchdogd/Db/EntreeANA/EntreeANA.c        Déclaration des fonctions pour la gestion des entreeANA.c    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 25 jun 2004 11:50:10 CEST */
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
 #include "EntreeANA_DB.h"
 #include "Mnemonique_DB.h"

/**********************************************************************************************************/
/* Max_id_entreeANADB: Renvoie l'id maximum utilisé + 1 des entreeANA                                     */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_entreeANADB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_entreeANADB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_entreeANADB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_ENTREEANA );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_entreeANADB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

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
/* Retirer_entreeanaDB: Elimination d'un entreeANA                                                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_entreeANADB ( struct LOG *log, struct DB *db, struct CMD_ID_ENTREEANA *entreeana )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_entreeANADB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_ENTREEANA, entreeana->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_entreeanaDB: elimination failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_entreeanaDB: elimination ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }

/**********************************************************************************************************/
/* Ajouter_entreeanaDB: Ajout ou edition d'un entreeANA                                                   */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure entreeana                     */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_entreeANADB ( struct LOG *log, struct DB *db, struct CMD_ADD_ENTREEANA *entreeana )
  { gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gint id;

    id = Max_id_entreeANADB( log, db );
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_entreeANADB: recherche failed: query=null" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,num,min,max,unite) VALUES "
                "(%d,%d,%f,%f,%d)", NOM_TABLE_ENTREEANA, id, entreeana->num,
                entreeana->min, entreeana->max, entreeana->unite );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Ajouter_entreeANADB: ajout failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Ajouter_entreeANADB: ajout ok" );

    EndQueryDB( log, db, hquery );
    return(id);
  }
/**********************************************************************************************************/
/* Creer_db_entreeana: création des tables associées aux entreeANAs au fil de l'eau                       */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_entreeANA ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_entreeana: Creation DB failed: query=null", NOM_TABLE_ENTREEANA );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete),
                                          "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  num INTEGER NOT NULL,"
                                          "  min FLOAT NOT NULL,"
                                          "  max FLOAT NOT NULL,"
                                          "  unite INTEGER NOT NULL"
                                          ");",
                                          NOM_TABLE_ENTREEANA );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_entreeana: création table failed", NOM_TABLE_ENTREEANA );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_entreeana: succes création table", NOM_TABLE_ENTREEANA );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_entreeANADB ( struct LOG *log, struct DB *db )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[512];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.num,%s.min,%s.max,%s.unite,%s.libelle"
                " FROM %s,%s WHERE %s.num=%s.num AND %s.type=%d ORDER BY %s.num",
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_MNEMO,
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_MNEMO, MNEMO_ENTREE_ANA, /* And */
                NOM_TABLE_ENTREEANA /* Order by */
              );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_entreeANADB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ENTREEANA_DB *Recuperer_entreeANADB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id_from_sql[10], num[10], min[10], max[10], libelle[NBR_CARAC_LIBELLE_ENTREEANA_UTF8+1], unite[10];
    struct ENTREEANA_DB *entreeana;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &num, sizeof(num), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB_suite: erreur bind du num" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &min, sizeof(min), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB_suite: erreur bind du min" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &max, sizeof(max), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB_suite: erreur bind du max" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &unite, sizeof(unite), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB_suite: erreur bind du unite" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    entreeana = (struct ENTREEANA_DB *)g_malloc0( sizeof(struct ENTREEANA_DB) );
    if (!entreeana) Info( log, DEBUG_MEM, "Recuperer_entreeANADB_suite: Erreur allocation mémoire" );
    else
     { entreeana->id    = atoi(id_from_sql);
       entreeana->num   = atoi(num);
       entreeana->min   = atof(min);
       entreeana->max   = atof(max);
       entreeana->unite = atoi(unite);
       memcpy( entreeana->libelle, libelle, sizeof(entreeana->libelle) );    /* Recopie dans la structure */
     }
    return(entreeana);
  }
/**********************************************************************************************************/
/* Rechercher_entreeanaDB: Recupération du entreeANA dont l'id est en parametre                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ENTREEANA_DB *Rechercher_entreeANADB ( struct LOG *log, struct DB *db, guint id )
  { gchar min[10], max[10], libelle[NBR_CARAC_LIBELLE_ENTREEANA_UTF8+1], num[10], unite[10];
    gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    struct ENTREEANA_DB *entreeana;
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_entreeanaDB: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &num, sizeof(num), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB: erreur bind du num" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &min, sizeof(min), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB_suite: erreur bind du min" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &max, sizeof(max), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB_suite: erreur bind du max" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &unite, sizeof(unite), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB_suite: erreur bind du unite" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_entreeANADB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.num,%s.min,%s.max,%s.unite,%s.libelle"
                " FROM %s,%s WHERE %s.num=%s.num AND %s.type=%d AND %s.id=%d",
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_MNEMO,
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_MNEMO, MNEMO_ENTREE_ANA, /* And */
                NOM_TABLE_ENTREEANA, id /* AND */
              );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_entreeanaDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_entreeanaDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_entreeanaDB: EntreANA non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_entreeanaDB: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    entreeana = (struct ENTREEANA_DB *)g_malloc0( sizeof(struct ENTREEANA_DB) );
    if (!entreeana)
     { Info( log, DEBUG_MEM, "Rechercher_entreeanaDB: Mem error" );
       return(NULL);
     }

    entreeana->id    = id;
    entreeana->num   = atoi(num);
    entreeana->min   = atof(min);
    entreeana->max   = atof(max);
    entreeana->unite = atoi(unite);
    memcpy( entreeana->libelle, libelle, sizeof(entreeana->libelle) );       /* Recopie dans la structure */

    return(entreeana);
  }
/**********************************************************************************************************/
/* Rechercher_entreeanaDB: Recupération du entreeANA dont l'id est en parametre                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ENTREEANA_DB *Rechercher_entreeANADB_par_num ( struct LOG *log, struct DB *db, guint num )
  { gchar min[10], max[10], libelle[NBR_CARAC_LIBELLE_ENTREEANA_UTF8+1], id[10], unite[10];
    gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    struct ENTREEANA_DB *entreeana;
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_entreeanaDB_par_num: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id, sizeof(num), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_entreeanaDB_par_num: erreur bind du num" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &min, sizeof(min), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_entreeanaDB_par_num: erreur bind du min" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &max, sizeof(max), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_entreeanaDB_par_num: erreur bind du max" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &unite, sizeof(unite), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_entreeanaDB_par_num: erreur bind du unite" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_entreeanaDB_par_num: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.min,%s.max,%s.unite,%s.libelle"
                " FROM %s,%s WHERE %s.num=%s.num AND %s.type=%d AND %s.num=%d",
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_MNEMO,
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_MNEMO, MNEMO_ENTREE_ANA, /* And */
                NOM_TABLE_ENTREEANA, num /* AND */
              );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_entreeanaDB_par_num: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_entreeanaDB_par_num: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_entreeanaDB: EntreANA non trouvé dans la BDD", num );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_entreeanaDB_par_num: Multiple solution", num );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    entreeana = (struct ENTREEANA_DB *)g_malloc0( sizeof(struct ENTREEANA_DB) );
    if (!entreeana)
     { Info( log, DEBUG_MEM, "Rechercher_entreeanaDB_par_num: Mem error" );
       return(NULL);
     }

    entreeana->id    = atoi(id);
    entreeana->num   = num;
    entreeana->min   = atof(min);
    entreeana->max   = atof(max);
    entreeana->unite = atoi(unite);
    memcpy( entreeana->libelle, libelle, sizeof(entreeana->libelle) );       /* Recopie dans la structure */

    return(entreeana);
  }
/**********************************************************************************************************/
/* Modifier_entreeANADB: Modification d'un entreeANA Watchdog                                             */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_entreeANADB( struct LOG *log, struct DB *db, struct CMD_EDIT_ENTREEANA *entreeana )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Modifier_entreeANADB: ajout failed: query=null", entreeana->id );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "num=%d,min=%f,max=%f,unite=%d WHERE id=%d",
                NOM_TABLE_ENTREEANA, entreeana->num,entreeana->min, entreeana->max,
                entreeana->unite, entreeana->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Modifier_entreeANADB: Modif failed", entreeana->id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Modifier_entreeANADB: succes modif ea", entreeana->id );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
