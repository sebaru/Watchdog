/**********************************************************************************************************/
/* Watchdogd/Db/Icones/classe.c        Déclaration des fonctions pour la gestion des classes-classes       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 30 sep 2003 10:38:04 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "Erreur.h"
 #include "Icones_DB.h"

/**********************************************************************************************************/
/* Retirer_msgDB: Elimination d'une classe                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_classeDB ( struct LOG *log, struct DB *db, struct CMD_ID_CLASSE *classe )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    if (classe->id == 0) return(TRUE);                              /* La classe 0 n'est pas destructible */
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_classeDB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET id_classe=0 WHERE id_classe=%d",
                NOM_TABLE_ICONE, classe->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_classeDB: update icone failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_classeDB: update icone ok" );

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_CLASSE, classe->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_classeDB: elimination failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_classeDB: elimination ok" );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Max_id_classesDB: Renvoie l'id maximum utilisé + 1 des classes                                         */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_classesDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_classesDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_classesDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_CLASSE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_classesDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_classesDB: recherche ok" );

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
 gint Ajouter_classeDB ( struct LOG *log, struct DB *db, struct CMD_ADD_CLASSE *classe )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *libelle;
    gint id;

    id = Max_id_classesDB ( log, db );
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_classeDB: recherche failed: query=null" );
       return(-1);
     }

    libelle = Normaliser_chaine ( log, classe->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_classeDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,libelle) VALUES "
                "(%d,'%s')", NOM_TABLE_CLASSE, id, libelle );
    g_free(libelle);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Ajouter_classeDB: ajout failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Ajouter_classeDB: ajout ok" );

    EndQueryDB( log, db, hquery );
    return(id);
  }
/**********************************************************************************************************/
/* Creer_db_classe: création des tables associées aux classes                                             */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_classe ( struct LOG *log, struct DB *db )
  { struct CMD_ADD_CLASSE add_classe;
    SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_classe: Creation DB failed: query=null", NOM_TABLE_CLASSE );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  libelle VARCHAR(%d) NOT NULL"
                                          ");",
                                          NOM_TABLE_CLASSE, NBR_CARAC_CLASSE_ICONE_UTF8+1 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_classe: création table failed", NOM_TABLE_CLASSE );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_classe: succes création table", NOM_TABLE_CLASSE );

    EndQueryDB( log, db, hquery );
/************************************* Ajout de la classe par défaut **************************************/
    g_snprintf( add_classe.libelle, sizeof(add_classe.libelle), _("Default") );
    Ajouter_classeDB( log, db, &add_classe );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_classeDB ( struct LOG *log, struct DB *db )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[200];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_classeDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle"
                " FROM %s ORDER BY id", NOM_TABLE_CLASSE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_classeDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_classeDB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CLASSEDB *Recuperer_classeDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id_from_sql[10], libelle[NBR_CARAC_CLASSE_ICONE_UTF8+1];
    struct CLASSEDB *classe;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_classeDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_classeDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    classe = (struct CLASSEDB *)g_malloc0( sizeof(struct CLASSEDB) );
    if (!classe) Info( log, DEBUG_MEM, "Recuperer_classeDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( classe->libelle, libelle, sizeof(classe->libelle) );          /* Recopie dans la structure */
       classe->id          = atoi(id_from_sql);
     }
    return(classe);
  }
/**********************************************************************************************************/
/* Rechercher_classeDB: Recupération du classe dont l'id est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CLASSEDB *Rechercher_classeDB ( struct LOG *log, struct DB *db, guint id )
  { gchar libelle[NBR_CARAC_CLASSE_ICONE_UTF8+1];
    struct CLASSEDB *classe;
    gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_classeDB: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_classeDB: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle FROM %s WHERE id=%d", NOM_TABLE_CLASSE, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_classeDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_classeDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_classeDB: Classe non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_classeDB: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    classe = (struct CLASSEDB *)g_malloc0( sizeof(struct CLASSEDB) );
    if (!classe)
     { Info( log, DEBUG_MEM, "Rechercher_classeDB: Mem error" );
       return(NULL);
     }

    classe->id          = id;
    memcpy( classe->libelle, libelle, sizeof(classe->libelle) );             /* Recopie dans la structure */
    return(classe);
  }
/**********************************************************************************************************/
/* Modifier_classeDB: Modification d'un classe Watchdog                                                   */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_classeDB( struct LOG *log, struct DB *db, struct CMD_EDIT_CLASSE *classe )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gchar *libelle;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Modifier_classeDB: ajout failed: query=null", classe->id );
       return(FALSE);
     }

    libelle = Normaliser_chaine ( log, classe->libelle );
    if (!libelle)
     { Info( log, DEBUG_DB, "Modifier_classeDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s' WHERE id=%d",
                NOM_TABLE_CLASSE, libelle, classe->id );
    g_free(libelle);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Modifier_classeDB: Modif failed", classe->id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Modifier_classeDB: succes modif", classe->id );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
