/**********************************************************************************************************/
/* Watchdogd/Db/Histo/Histo.c        Déclaration des fonctions pour la gestion de l'historique            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 21 aoû 2003 15:29:16 CEST */
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
/* Clear_histoDB: Elimination des messages histo au boot systeme                                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Clear_histoDB ( struct LOG *log, struct DB *db )
  { gchar requete[1024];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Clear_histoDB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s", NOM_TABLE_HISTO );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Clear_histoDB: elimination failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Clear_histoDB: elimination ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Retirer_msgDB: Elimination d'un message                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_histoDB ( struct LOG *log, struct DB *db, struct CMD_ID_HISTO *histo )
  { gchar requete[1024];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_histoDB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_HISTO, histo->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Retirer_histoDB: elimination failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Retirer_histoDB: elimination ok", requete );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }

/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Ajouter_histoDB ( struct LOG *log, struct DB *db, struct HISTODB *histo )
  { gchar requete[1024];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *libelle, *nom_ack, *objet;

    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_histoDB: recherche failed: query=null" );
       return(FALSE);
     }

    libelle = Normaliser_chaine ( log, histo->msg.libelle );             /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_histoDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    objet = Normaliser_chaine ( log, histo->msg.objet );                 /* Formatage correct des chaines */
    if (!objet)
     { Info( log, DEBUG_DB, "Ajouter_histoDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       g_free(libelle);
       return(FALSE);
     }

    nom_ack = Normaliser_chaine ( log, histo->nom_ack );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_histoDB: Normalisation impossible" );
       g_free(libelle);
       g_free(objet);
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,libelle,objet,type,num_syn,nom_ack,"
                "date_create_sec,date_create_usec,"
                "date_fixe) VALUES "
                "(%d,'%s','%s',%d,%d,'%s',%d,%d,0)", NOM_TABLE_HISTO, histo->msg.num, libelle, 
                objet, histo->msg.type,
                histo->msg.num_syn, nom_ack,
                histo->date_create_sec, histo->date_create_usec );
    g_free(libelle);
    g_free(nom_ack);
    g_free(objet);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Ajouter_histoDB: ajout failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Ajouter_histoDB: ajout ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Modifier_histoDB: Modification des champs editables d'un histo                                         */
/* Entrée: un log et une database, une structure de controle de la modification                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_histoDB ( struct LOG *log, struct DB *db, struct CMD_EDIT_HISTO *histo )
  { gchar requete[1024];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *nom_ack;

    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Modifier_histoDB: recherche failed: query=null" );
       return(FALSE);
     }

    nom_ack = Normaliser_chaine ( log, histo->nom_ack );                 /* Formatage correct des chaines */
    if (!nom_ack)
     { Info( log, DEBUG_DB, "Modifier_histoDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET nom_ack='%s',date_fixe=%d WHERE id=%d",
                NOM_TABLE_HISTO, nom_ack, (gint)histo->date_fixe, histo->id );
    g_free(nom_ack);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Modifier_histo->DB: modif failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Modifier_histo->DB: modif ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Creer_db_msg: création des tables associées aux messages au fil de l'eau                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_histo ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_histo: Creation DB failed: query=null", NOM_TABLE_MSG );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  libelle VARCHAR(%d) NOT NULL,"
                                          "  objet VARCHAR(%d) NOT NULL,"
                                          "  type INTEGER NOT NULL,"
                                          "  num_syn INTEGER NOT NULL,"
                                          "  nom_ack VARCHAR(%d),"
                                          "  date_create_sec INTEGER NOT NULL,"
                                          "  date_create_usec INTEGER NOT NULL,"
                                          "  date_fixe INTEGER NOT NULL"
                                          ");",
                                          NOM_TABLE_HISTO, NBR_CARAC_LIBELLE_MSG_UTF8+1,
                                          NBR_CARAC_OBJET_MSG_UTF8+1, NBR_CARAC_LOGIN_UTF8+1 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_histo: création table failed", NOM_TABLE_HISTO );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_histo: succes création table", NOM_TABLE_HISTO );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_histoDB ( struct LOG *log, struct DB *db )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[1024];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_histoDB: recherche failed: query=null" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,objet,type,num_syn,nom_ack,date_create_sec,date_create_usec,"
                "date_fixe FROM %s ORDER by date_create_sec,date_create_usec", NOM_TABLE_HISTO );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histoDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_histoDB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct HISTODB *Recuperer_histoDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id_from_sql[10], libelle[NBR_CARAC_LIBELLE_MSG_UTF8+1], objet[NBR_CARAC_OBJET_MSG_UTF8+1];
    gchar type[10], num_syn[10];
    gchar date_create_sec[15], date_create_usec[15], date_fixe[15], nom_ack[NBR_CARAC_LOGIN_UTF8+1]; 
    struct HISTODB *histo;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histoDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histoDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &objet, sizeof(objet), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histoDB_suite: erreur bind du objet");
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &type, sizeof(type), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histoDB_suite: erreur bind du type" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &num_syn, sizeof(num_syn), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histoDB_suite: erreur bind du num_syn" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &nom_ack, sizeof(nom_ack), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histoDB_suite: erreur bind du nom_ack" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &date_create_sec, sizeof(date_create_sec), NULL );/* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histoDB_suite: erreur bind du date_create_sec" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &date_create_usec, sizeof(date_create_usec), NULL );/*Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histoDB_suite: erreur bind du date_create_usec" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &date_fixe, sizeof(date_fixe), NULL );            /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_histoDB_suite: erreur bind du date_fixe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    histo = (struct HISTODB *)g_malloc0( sizeof(struct HISTODB) );
    if (!histo) Info( log, DEBUG_MEM, "Recuperer_histoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( histo->msg.libelle, libelle, sizeof(histo->msg.libelle) );    /* Recopie dans la structure */
       memcpy( histo->msg.objet, objet, sizeof(histo->msg.objet) );          /* Recopie dans la structure */
       memcpy( histo->nom_ack, nom_ack, sizeof(histo->nom_ack) );            /* Recopie dans la structure */
       histo->msg.id           = 0;                                /* l'id n'est pas dans la base histo ! */
       histo->msg.num          = atoi(id_from_sql);
       histo->msg.type         = atoi(type);
       histo->msg.num_syn      = atoi(num_syn);
       histo->date_create_sec  = atoi(date_create_sec);
       histo->date_create_usec = atoi(date_create_usec);
       histo->date_fixe        = atoi(date_fixe);
     }
    return(histo);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct HISTODB *Rechercher_histoDB( struct LOG *log, struct DB *db, gint id )
  { gchar libelle[NBR_CARAC_LIBELLE_MSG_UTF8+1], objet[NBR_CARAC_OBJET_MSG_UTF8+1];
    gchar type[10], num_syn[10];
    gchar date_create_sec[15], date_create_usec[15], date_fixe[15], nom_ack[NBR_CARAC_LOGIN_UTF8+1]; 
    struct HISTODB *histo;
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[1024];
    SQLINTEGER nbr;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_histoDB: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_histoDB: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &objet, sizeof(objet), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_histoDB: erreur bind du objet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &type, sizeof(type), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_histoDB: erreur bind du type" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &num_syn, sizeof(num_syn), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_histoDB: erreur bind du num_syn" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &nom_ack, sizeof(nom_ack), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_histoDB: erreur bind du nom_ack" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &date_create_sec, sizeof(date_create_sec), NULL );/* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_histoDB: erreur bind du date_create_sec" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &date_create_usec, sizeof(date_create_usec), NULL );/*Bind*/
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_histoDB: erreur bind du date_create_usec" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &date_fixe, sizeof(date_fixe), NULL );            /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_histoDB: erreur bind du date_fixe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle,objet,type,num_syn,nom_ack,date_create_sec,date_create_usec,"
                "date_fixe FROM %s WHERE id=%d", NOM_TABLE_HISTO, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_histoDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_histoDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_histoDB: Message non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_histoDB: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    histo = (struct HISTODB *)g_malloc0( sizeof(struct HISTODB) );
    if (!histo) Info( log, DEBUG_MEM, "Recuperer_histoDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( histo->msg.libelle, libelle, sizeof(histo->msg.libelle) );    /* Recopie dans la structure */
       memcpy( histo->msg.objet, objet, sizeof(histo->msg.objet) );          /* Recopie dans la structure */
       memcpy( histo->nom_ack, nom_ack, sizeof(histo->nom_ack) );            /* Recopie dans la structure */
       histo->msg.num          = id;
       histo->msg.id           = 0;                  /* L'ID msg en histo n'est pas porteur d'information */
       histo->msg.type         = atoi(type);
       histo->msg.num_syn      = atoi(num_syn);
       histo->date_create_sec  = atoi(date_create_sec);
       histo->date_create_usec = atoi(date_create_usec);
       histo->date_fixe        = atoi(date_fixe);
     }
    return(histo);
  }
/*--------------------------------------------------------------------------------------------------------*/
