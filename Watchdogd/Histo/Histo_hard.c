/**********************************************************************************************************/
/* Watchdogd/Db/Message/Message.c        Déclaration des fonctions pour la gestion des message            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 15 aoû 2003 13:02:48 CEST */
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
 #include "Histo_DB.h"

/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Ajouter_histo_hardDB ( struct LOG *log, struct DB *db, struct HISTO_HARDDB *histo )
  { gchar requete[1024];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *libelle, *nom_ack, *objet;

    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_histo_hardDB: recherche failed: query=null" );
       return(FALSE);
     }

    libelle = Normaliser_chaine ( log, histo->histo.msg.libelle );       /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_histo_hardDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    nom_ack = Normaliser_chaine ( log, histo->histo.nom_ack );           /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_histo_hardDB: Normalisation impossible" );
       g_free(libelle);
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    objet = Normaliser_chaine ( log, histo->histo.msg.objet );           /* Formatage correct des chaines */
    if (!objet)
     { Info( log, DEBUG_DB, "Ajouter_histoDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       g_free(libelle);
       g_free(nom_ack);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(num,libelle,objet,type,num_syn,nom_ack,date_create_sec,date_create_usec,"
                "date_fixe,date_fin) VALUES "
                "(%d,'%s','%s',%d,%d,'%s',%d,%d,%d,%d)", NOM_TABLE_HISTO_HARD, histo->histo.msg.num, libelle,
                objet, histo->histo.msg.type, histo->histo.msg.num_syn, 
                nom_ack, histo->histo.date_create_sec, histo->histo.date_create_usec,
                (gint)histo->histo.date_fixe, (gint)histo->date_fin );
    g_free(libelle);
    g_free(nom_ack);
    g_free(objet);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Ajouter_histo_hardDB: ajout failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Ajouter_histo_hardDB: ajout ok", requete );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }

/**********************************************************************************************************/
/* Creer_db_msg: création des tables associées aux messages au fil de l'eau                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_histo_hard ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_histo_hard: Creation DB failed: query=null", NOM_TABLE_MSG );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( num INTEGER NOT NULL,"
                                          "  libelle VARCHAR(%d) NOT NULL,"
                                          "  type INTEGER NOT NULL,"
                                          "  num_syn INTEGER NOT NULL,"
                                          "  nom_ack VARCHAR(%d),"
                                          "  objet VARCHAR(%d) NOT NULL,"
                                          "  date_create_sec INTEGER NOT NULL,"
                                          "  date_create_usec INTEGER NOT NULL,"
                                          "  date_fixe INTEGER NOT NULL,"
                                          "  date_fin INTEGER NOT NULL"
                                          ");",
                                          NOM_TABLE_HISTO_HARD, NBR_CARAC_LIBELLE_MSG_UTF8+1,
                                          NBR_CARAC_LOGIN_UTF8+1, NBR_CARAC_LIBELLE_MSG_UTF8+1 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_histo_hard: création table failed", NOM_TABLE_HISTO_HARD );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_histo_hard: succes création table", NOM_TABLE_HISTO_HARD );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_histo_hardDB: Recupération de l'historique HARD du système, via requete                      */
/* Entrée: un log et une database, et des champs de requete                                               */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Rechercher_histo_hardDB ( struct LOG *log, struct DB *db, struct CMD_REQUETE_HISTO_HARD *critere )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[1024];
    gchar critereSQL[1024];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_histo_hardDB: recherche failed: query=null" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT num,libelle,objet,type,num_syn,nom_ack,date_create_sec,date_create_usec,"
                "date_fixe,date_fin FROM %s WHERE num<>-1",
                NOM_TABLE_HISTO_HARD );

    memset( critereSQL, 0, sizeof(critereSQL) );
    if (critere->id != -1)
     { g_snprintf( critereSQL, sizeof(critereSQL), " AND num=%d", critere->id );
       g_strlcat( requete, critereSQL, sizeof(requete) );
     }
    if (critere->type != -1)
     { g_snprintf( critereSQL, sizeof(critereSQL), " AND type=%d", critere->type );
       g_strlcat( requete, critereSQL, sizeof(requete) );
     }
    if (critere->date_create_min!=-1)
     { g_snprintf( critereSQL, sizeof(critereSQL), " AND date_create_sec>%d", (int)critere->date_create_min );
       g_strlcat( requete, critereSQL, sizeof(requete) );
     }
    if ( *(critere->nom_ack) )
     { gchar *norm;
       critere->nom_ack[sizeof(critere->nom_ack)-1] = 0;                          /* Anti buffer overflow */
       norm = Normaliser_chaine(log, critere->nom_ack);
       if (norm)
        { g_snprintf( critereSQL, sizeof(critereSQL), " AND nom_ack LIKE '%%%s%%'", norm );
          g_strlcat( requete, critereSQL, sizeof(requete) );
          g_free(norm);
        }
     }
    if ( *(critere->libelle) )
     { gchar *norm;
       critere->libelle[sizeof(critere->libelle)-1] = 0;                          /* Anti buffer overflow */
       norm = Normaliser_chaine(log, critere->libelle);
       if (norm)
        { g_snprintf( critereSQL, sizeof(critereSQL), " AND libelle LIKE '%%%s%%'", norm );
          g_strlcat( requete, critereSQL, sizeof(requete) );
          g_free(norm);
        }
     }
    if ( *(critere->objet) )
     { gchar *norm;
       critere->objet[sizeof(critere->objet)-1] = 0;                              /* Anti buffer overflow */
       norm = Normaliser_chaine(log, critere->objet);
       if (norm)
        { g_snprintf( critereSQL, sizeof(critereSQL), " AND objet LIKE '%%%s%%'", norm );
          g_strlcat( requete, critereSQL, sizeof(requete) );
          g_free(norm);
        }
     }
    g_strlcat( requete, " ORDER BY date_create_sec,date_create_usec LIMIT 500;", sizeof(requete) );
 
    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Rechercher_histo_hardDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info_c( log, DEBUG_DB, "Rechercher_histo_hardDB: recherche ok", requete );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct HISTO_HARDDB *Rechercher_histo_hardDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id_from_sql[10], libelle[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    gchar type[10], num_syn[10], objet[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    gchar date_create_sec[15], date_create_usec[15], date_fixe[15], date_fin[15];
    gchar nom_ack[NBR_CARAC_LOGIN_UTF8+1]; 
    struct HISTO_HARDDB *histo_hard;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histo_hardDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histo_hardDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &objet, sizeof(objet), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histo_hardDB_suite: erreur bind du objet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &type, sizeof(type), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histo_hardDB_suite: erreur bind du type" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &num_syn, sizeof(num_syn), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histo_hardDB_suite: erreur bind du num_syn" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &nom_ack, sizeof(nom_ack), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histo_hardDB_suite: erreur bind du nom_ack" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &date_create_sec, sizeof(date_create_sec), NULL ); /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histo_hardDB_suite: erreur bind du date_create_sec" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &date_create_usec, sizeof(date_create_usec), NULL );/*Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histo_hardDB_suite: erreur bind du date_create_usec" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &date_fixe, sizeof(date_fixe), NULL );            /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histo_hardDB_suite: erreur bind du date_fixe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 10, SQL_C_CHAR, &date_fin, sizeof(date_fin), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histo_hardDB_suite: erreur bind du date_fin" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    histo_hard = (struct HISTO_HARDDB *)g_malloc0( sizeof(struct HISTO_HARDDB) );
    if (!histo_hard) Info( log, DEBUG_MEM, "Recuperer_histo_hardDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( histo_hard->histo.msg.libelle, libelle, sizeof(histo_hard->histo.msg.libelle) );
       memcpy( histo_hard->histo.msg.objet, objet, sizeof(histo_hard->histo.msg.objet) );
                                                                             /* Recopie dans la structure */
       memcpy( histo_hard->histo.nom_ack, nom_ack, sizeof(histo_hard->histo.nom_ack) );
       histo_hard->histo.msg.num          = atoi(id_from_sql);
       histo_hard->histo.msg.type         = atoi(type);
       histo_hard->histo.msg.num_syn      = atoi(num_syn);
       histo_hard->histo.date_create_sec  = atoi(date_create_sec);
       histo_hard->histo.date_create_usec = atoi(date_create_usec);
       histo_hard->histo.date_fixe        = atoi(date_fixe);
       histo_hard->date_fin               = atoi(date_fin);
printf("Recup histo: %d %s %s %s\n", histo_hard->histo.msg.num, objet, libelle, histo_hard->histo.nom_ack );
     }
    return(histo_hard);
  }
/*--------------------------------------------------------------------------------------------------------*/
