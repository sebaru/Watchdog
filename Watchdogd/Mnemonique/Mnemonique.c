/**********************************************************************************************************/
/* Watchdogd/Db/Mnemonique/Mnemonique.c        Déclaration des fonctions pour la gestion des mnemoniques  */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 05 déc 2004 14:15:35 CET */
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
 #include "Mnemonique_DB.h"

/**********************************************************************************************************/
/* Retirer_mnemoDB: Elimination d'un mnemo                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_mnemoDB ( struct LOG *log, struct DB *db, struct CMD_ID_MNEMONIQUE *mnemo )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_mnemoDB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MNEMO, mnemo->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_mnemoDB: elimination failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_mnemoDB: elimination ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Max_id_mnemoniquesDB: Renvoie l'id maximum utilisé + 1 des mnemoniques                                 */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_mnemoniquesDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_mnemoniquesDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_mnemoniquesDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_MNEMO );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_mnemoniquesDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_mnemoniquesDB: recherche ok" );

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
/* Ajouter_mnemoDB: Ajout ou edition d'un mnemo                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure mnemo                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_mnemoDB ( struct LOG *log, struct DB *db, struct CMD_ADD_MNEMONIQUE *mnemo )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *libelle, *objet, *acro;
    guint id;

    id = Max_id_mnemoniquesDB( log, db );
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_mnemoDB: recherche failed: query=null" );
       return(-1);
     }
 
    libelle = Normaliser_chaine ( log, mnemo->libelle );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_mnemoDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    objet = Normaliser_chaine ( log, mnemo->objet );                     /* Formatage correct des chaines */
    if (!objet)
     { Info( log, DEBUG_DB, "Ajouter_mnemoDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       g_free(libelle);
       return(-1);
     }
    acro = Normaliser_chaine ( log, mnemo->acronyme );                   /* Formatage correct des chaines */
    if (!acro)
     { Info( log, DEBUG_DB, "Ajouter_mnemoDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       g_free(objet);
       g_free(libelle);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,type,num,objet,acronyme,libelle) VALUES "
                "(%d,%d,%d,'%s','%s','%s')", NOM_TABLE_MNEMO, id, mnemo->type,
                mnemo->num, objet, acro, libelle );
    g_free(libelle);
    g_free(acro);
    g_free(objet);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Ajouter_mnemoDB: ajout failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Ajouter_mnemoDB: ajout ok" );

    EndQueryDB( log, db, hquery );
    return(id);
  }
/**********************************************************************************************************/
/* Creer_db_mnemo: création des tables associées aux mnemos au fil de l'eau                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_mnemo ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_mnemo: Creation DB failed: query=null", NOM_TABLE_MNEMO );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete),
                                          "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  type INTEGER NOT NULL,"
                                          "  num INTEGER NOT NULL,"
                                          "  objet VARCHAR(%d) NOT NULL,"
                                          "  acronyme VARCHAR(%d) NOT NULL,"
                                          "  libelle VARCHAR(%d) NOT NULL"
                                          ");",
                                          NOM_TABLE_MNEMO, NBR_CARAC_OBJET_MNEMONIQUE_UTF8+1,
                                                           NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1,
                                                           NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_mnemo: création table failed", NOM_TABLE_MNEMO );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_mnemo: succes création table", NOM_TABLE_MNEMO );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_mnemoDB: Recupération de la liste des ids des mnemos                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_mnemoDB ( struct LOG *log, struct DB *db )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[200];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,type,num,objet,acronyme,libelle"
                " FROM %s ORDER BY objet,type,num", NOM_TABLE_MNEMO );          /* order by test 25/01/06 */

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_mnemoDB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_mnemoDB: Recupération de la liste des ids des mnemos                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MNEMONIQUEDB *Recuperer_mnemoDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id_from_sql[10], libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1];
    gchar objet[NBR_CARAC_OBJET_MNEMONIQUE_UTF8+1], acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar type[10], num[10];
    struct MNEMONIQUEDB *mnemo;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &type, sizeof(type), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind du type" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &num, sizeof(num), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind du num" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &objet, sizeof(objet), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind du objet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &acronyme, sizeof(acronyme), NULL );              /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind du acronyme" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    mnemo = (struct MNEMONIQUEDB *)g_malloc0( sizeof(struct MNEMONIQUEDB) );
    if (!mnemo) Info( log, DEBUG_MEM, "Recuperer_mnemoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( mnemo->libelle, libelle, sizeof(mnemo->libelle) );            /* Recopie dans la structure */
       memcpy( mnemo->objet, objet, sizeof(mnemo->objet) );                  /* Recopie dans la structure */
       memcpy( mnemo->acronyme, acronyme, sizeof(mnemo->acronyme) );         /* Recopie dans la structure */
       mnemo->id          = atoi(id_from_sql);
       mnemo->type        = atoi(type);
       mnemo->num         = atoi(num);
     }
    return(mnemo);
  }
/**********************************************************************************************************/
/* Rechercher_mnemoDB: Recupération du mnemo dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MNEMONIQUEDB *Rechercher_mnemoDB ( struct LOG *log, struct DB *db, guint id )
  { gchar libelle[ NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1 ];
    gchar objet[ NBR_CARAC_OBJET_MNEMONIQUE_UTF8+1 ];
    gchar acronyme[ NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1 ];
    gchar type_from_sql[10];
    gchar num_from_sql[10];
    gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    struct MNEMONIQUEDB *mnemo;
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &acronyme, sizeof(acronyme), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &objet, sizeof(objet), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB: erreur bind du objet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &type_from_sql, sizeof(type_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "RRechercher_mnemoDB: erreur bind du type" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &num_from_sql, sizeof(num_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB: erreur bind du numsyn" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle,acronyme,objet,type,num FROM %s WHERE id=%d", NOM_TABLE_MNEMO, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_mnemoDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_mnemoDB: Mnemo non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_mnemoDB: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    mnemo = (struct MNEMONIQUEDB *)g_malloc0( sizeof(struct MNEMONIQUEDB) );
    if (!mnemo)
     { Info( log, DEBUG_MEM, "Rechercher_mnemoDB: Mem error" );
       return(NULL);
     }

    memcpy( mnemo->libelle, libelle, sizeof(mnemo->libelle) );               /* Recopie dans la structure */
    memcpy( mnemo->objet, objet, sizeof(mnemo->objet) );                     /* Recopie dans la structure */
    memcpy( mnemo->acronyme, acronyme, sizeof(mnemo->acronyme) );            /* Recopie dans la structure */
    mnemo->id      = id;
    mnemo->type    = atoi( type_from_sql );
    mnemo->num     = atoi( num_from_sql );

    return(mnemo);
  }
/**********************************************************************************************************/
/* Rechercher_mnemoDB: Recupération du mnemo dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MNEMONIQUEDB *Rechercher_mnemoDB_type_num ( struct LOG *log, struct DB *db,
                                                    struct CMD_TYPE_NUM_MNEMONIQUE *critere )
  { gchar libelle[ NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1 ];
    gchar acronyme[ NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1 ];
    gchar objet[ NBR_CARAC_OBJET_MNEMONIQUE_UTF8+1 ];
    gchar id_from_sql[10];
    gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    struct MNEMONIQUEDB *mnemo;
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB_type_num: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB_type_num: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &acronyme, sizeof(acronyme), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB_type_num: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &objet, sizeof(objet), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB_type_num: erreur bind du objet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB_type_num: erreur bind du id_from_sql" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle,acronyme,objet,id FROM %s WHERE type=%d AND num=%d",
                NOM_TABLE_MNEMO, critere->type, critere->num );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_mnemoDB_type_num: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_mnemoDB_type_num: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_mnemoDB_type_num: Mnemo non trouvé dans la BDD", atoi(id_from_sql) );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_mnemoDB_type_num: Multiple solution", atoi(id_from_sql) );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    mnemo = (struct MNEMONIQUEDB *)g_malloc0( sizeof(struct MNEMONIQUEDB) );
    if (!mnemo)
     { Info( log, DEBUG_MEM, "Rechercher_mnemoDB_type_num: Mem error" );
       return(NULL);
     }

    mnemo->id      = atoi(id_from_sql);
    memcpy( mnemo->libelle, libelle, sizeof(mnemo->libelle) );               /* Recopie dans la structure */
    memcpy( mnemo->objet, objet, sizeof(mnemo->objet) );                     /* Recopie dans la structure */
    memcpy( mnemo->acronyme, acronyme, sizeof(mnemo->acronyme) );            /* Recopie dans la structure */
    mnemo->type    = critere->type;
    mnemo->num     = critere->num;

    return(mnemo);
  }
/**********************************************************************************************************/
/* Modifier_mnemoDB: Modification d'un mnemo Watchdog                                                     */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_mnemoDB( struct LOG *log, struct DB *db, struct CMD_EDIT_MNEMONIQUE *mnemo )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gchar *libelle, *objet, *acronyme;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Modifier_mnemoDB: ajout failed: query=null", mnemo->id );
       return(FALSE);
     }

    libelle = Normaliser_chaine ( log, mnemo->libelle );
    if (!libelle)
     { Info( log, DEBUG_DB, "Modifier_mnemoDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    objet = Normaliser_chaine ( log, mnemo->objet );
    if (!objet)
     { Info( log, DEBUG_DB, "Modifier_mnemoDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       g_free(libelle);
       return(FALSE);
     }

    acronyme = Normaliser_chaine ( log, mnemo->acronyme );
    if (!acronyme)
     { Info( log, DEBUG_DB, "Modifier_mnemoDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       g_free(objet);
       g_free(libelle);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s',acronyme='%s',objet='%s',type=%d,num=%d WHERE id=%d",
                NOM_TABLE_MNEMO, libelle, acronyme, objet,mnemo->type, mnemo->num, mnemo->id );
    g_free(libelle);
    g_free(acronyme);
    g_free(objet);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Modifier_mnemoDB: Modif failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_mnemoDB: Recupération de la liste des ids des mnemos                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_mnemoDB_for_courbe ( struct LOG *log, struct DB *db )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[200];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,type,num,objet,acronyme,libelle"
                " FROM %s WHERE type=%d OR type=%d"
                " ORDER BY objet,type,num",
                NOM_TABLE_MNEMO, MNEMO_ENTREE, MNEMO_SORTIE
              );                                                                /* order by test 25/01/06 */

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_mnemoDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_mnemoDB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_mnemoDB: Recupération de la liste des ids des mnemos                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MNEMONIQUEDB *Recuperer_mnemoDB_for_courbe_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id_from_sql[10], libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1];
    gchar objet[NBR_CARAC_OBJET_MNEMONIQUE_UTF8+1], acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar type[10], num[10];
    struct MNEMONIQUEDB *mnemo;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &type, sizeof(type), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind du type" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &num, sizeof(num), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind du num" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &objet, sizeof(objet), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind du objet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &acronyme, sizeof(acronyme), NULL );              /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind du acronyme" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_mnemoDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    mnemo = (struct MNEMONIQUEDB *)g_malloc0( sizeof(struct MNEMONIQUEDB) );
    if (!mnemo) Info( log, DEBUG_MEM, "Recuperer_mnemoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( mnemo->libelle, libelle, sizeof(mnemo->libelle) );            /* Recopie dans la structure */
       memcpy( mnemo->objet, objet, sizeof(mnemo->objet) );                  /* Recopie dans la structure */
       memcpy( mnemo->acronyme, acronyme, sizeof(mnemo->acronyme) );         /* Recopie dans la structure */
       mnemo->id          = atoi(id_from_sql);
       mnemo->type        = atoi(type);
       mnemo->num         = atoi(num);
     }
    return(mnemo);
  }
/*--------------------------------------------------------------------------------------------------------*/
