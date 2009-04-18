/**********************************************************************************************************/
/* Watchdogd/Db/Message/Message.c        Déclaration des fonctions pour la gestion des message            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 16 avr 2006 16:35:23 CEST */
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
 #include "Message_DB.h"

/**********************************************************************************************************/
/* Max_id_messageDB: Renvoie l'id maximum utilisé + 1 des messageDB                                       */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_messageDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_messageDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_messageDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_MSG );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_messageDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_messageDB: recherche ok" );

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
/* Retirer_msgDB: Elimination d'un message                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_messageDB ( struct LOG *log, struct DB *db, struct CMD_ID_MESSAGE *msg )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_msgDB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MSG, msg->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_msgDB: elimination failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_msgDB: elimination ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_messageDB ( struct LOG *log, struct DB *db, struct CMD_ADD_MESSAGE *msg )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *libelle, *objet;
    gint id;

    id = Max_id_messageDB( log, db );
    if (id==-1)
     { Info( log, DEBUG_DB, "Ajouter_messageDB: Id max non trouvé" );
       return(-1);
     }

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_messageDB: recherche failed: query=null" );
       return(-1);
     }

    libelle = Normaliser_chaine ( log, msg->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_messageDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    objet = Normaliser_chaine ( log, msg->objet );                       /* Formatage correct des chaines */
    if (!objet)
     { g_free(libelle);
       Info( log, DEBUG_DB, "Ajouter_messageDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,num,libelle,type,num_syn,num_voc,not_inhibe,objet,sms) VALUES "
                "(%d,%d,'%s',%d,%d,%d,%s,'%s',%s)", NOM_TABLE_MSG, id, msg->num, libelle, msg->type,
                msg->num_syn, msg->num_voc, (msg->not_inhibe ? "true" : "false"), objet,
                (msg->sms ? "true" : "false") );
    g_free(libelle);
    g_free(objet);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Ajouter_messageDB: ajout failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Ajouter_messageDB: ajout ok" );

    EndQueryDB( log, db, hquery );
    return(id);
  }
/**********************************************************************************************************/
/* Creer_db_msg: création des tables associées aux messages au fil de l'eau                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_msg ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_msg: Creation DB failed: query=null", NOM_TABLE_MSG );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete),
                                          "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  num INTEGER NOT NULL,"
                                          "  libelle VARCHAR(%d) NOT NULL,"
                                          "  type INTEGER NOT NULL,"
                                          "  num_syn INTEGER NOT NULL,"
                                          "  num_voc INTEGER,"
                                          "  not_inhibe BOOLEAN NOT NULL,"
                                          "  objet VARCHAR(%d) NOT NULL,"
                                          "  sms BOOLEAN NOT NULL"
                                          ");",
                                          NOM_TABLE_MSG, NBR_CARAC_LIBELLE_MSG_UTF8+1,
                                                         NBR_CARAC_OBJET_MSG_UTF8+1 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_msg: création table failed", NOM_TABLE_MSG );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_msg: succes création table", NOM_TABLE_MSG );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_messageDB ( struct LOG *log, struct DB *db )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[200];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_messageDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,num,libelle,type,num_syn,num_voc,not_inhibe,objet,sms"
                " FROM %s ORDER BY objet,num", NOM_TABLE_MSG );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_messageDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_messageDB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MSGDB *Recuperer_messageDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id_from_sql[10], libelle[NBR_CARAC_LIBELLE_MSG_UTF8+1], num[10], sms[10];
    gchar type[10], num_syn[10], num_voc[10], not_inhibe[10], objet[NBR_CARAC_OBJET_MSG_UTF8+1];
    struct MSGDB *msg;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_messageDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &num, sizeof(num), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_messageDB_suite: erreur bind du num" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_messageDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &type, sizeof(type), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_messageDB_suite: erreur bind du type" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &num_syn, sizeof(num_syn), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_messageDB_suite: erreur bind du num_syn" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &num_voc, sizeof(num_voc), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_messageDB_suite: erreur bind du num_voc" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &not_inhibe, sizeof(not_inhibe), NULL );          /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_messageDB_suite: erreur bind du not_inhibe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &objet, sizeof(objet), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_messageDB_suite: erreur bind du objet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &sms, sizeof(sms), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_messageDB_suite: erreur bind du sms" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    msg = (struct MSGDB *)g_malloc0( sizeof(struct MSGDB) );
    if (!msg) Info( log, DEBUG_MEM, "Recuperer_messageDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( msg->libelle, libelle, sizeof(msg->libelle) );                /* Recopie dans la structure */
       memcpy( msg->objet, objet, sizeof(msg->objet) );
       msg->id          = atoi(id_from_sql);
       msg->num         = atoi(num);
       msg->type        = atoi(type);
       msg->num_syn     = atoi(num_syn);
       msg->num_voc     = atoi(num_voc);
       msg->not_inhibe  = atoi(not_inhibe);
       msg->sms         = atoi(sms);
     }
    return(msg);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MSGDB *Rechercher_messageDB ( struct LOG *log, struct DB *db, guint num )
  { gchar libelle[ NBR_CARAC_LIBELLE_MSG_UTF8+1 ];
    gchar objet[NBR_CARAC_OBJET_MSG_UTF8+1];
    gchar type_from_sql[10];
    gchar numsyn_from_sql[10];
    gchar numvoc_from_sql[10];
    gchar not_inhibe_from_sql[10];
    gchar requete[200];
    gchar id[10], sms[10];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    struct MSGDB *msg;
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_msgDB: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id, sizeof(id), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB: erreur bind du num" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &type_from_sql, sizeof(type_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "RRechercher_msgDB: erreur bind du type" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &numsyn_from_sql, sizeof(numsyn_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB: erreur bind du numsyn" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &numvoc_from_sql, sizeof(numvoc_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB: erreur bind du numvocal" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &not_inhibe_from_sql, sizeof(not_inhibe_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB: erreur bind du not_inhibe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &objet, sizeof(objet), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB: erreur bind du objet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &sms, sizeof(sms), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB: erreur bind du sms" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,type,num_syn,num_voc,not_inhibe,objet,sms FROM %s WHERE num=%d",
                NOM_TABLE_MSG, num );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Rechercher_msgDB: recherche failed", num );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_msgDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info( log, DEBUG_DB, "Rechercher_msgDB: recherche unsuccessful" );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_c( log, DEBUG_DB, "Rechercher_msgDB: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    msg = g_malloc0( sizeof(struct MSGDB) );
    if (!msg)
     { Info( log, DEBUG_MEM, "Rechercher_msgDB: Mem error" );
       return(NULL);
     }

    msg->num     = num;
    msg->id      = atoi(id);
    memcpy( msg->libelle, libelle, sizeof(msg->libelle) );
    memcpy( msg->objet, objet, sizeof(msg->objet) );
    msg->type    = atoi( type_from_sql );
    msg->num_syn = atoi( numsyn_from_sql );
    msg->num_voc = atoi( numvoc_from_sql );
    msg->not_inhibe   = atoi( not_inhibe_from_sql );
    msg->sms     = atoi( sms );

    return(msg);
  }
/**********************************************************************************************************/
/* Rechercher_messageDB_par_id: Recupération du message dont l'id est en parametre                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MSGDB *Rechercher_messageDB_par_id ( struct LOG *log, struct DB *db, guint id )
  { gchar libelle[ NBR_CARAC_LIBELLE_MSG_UTF8+1 ];
    gchar objet[NBR_CARAC_OBJET_MSG_UTF8+1];
    gchar type_from_sql[10];
    gchar numsyn_from_sql[10];
    gchar numvoc_from_sql[10];
    gchar not_inhibe_from_sql[10];
    gchar requete[200];
    gchar num[10], sms[10];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    struct MSGDB *msg;
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_msgDB_par_id: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &num, sizeof(num), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB_par_id: erreur bind du num" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB_par_id: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &type_from_sql, sizeof(type_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "RRechercher_msgDB_par_id: erreur bind du type" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &numsyn_from_sql, sizeof(numsyn_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB_par_id: erreur bind du numsyn" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &numvoc_from_sql, sizeof(numvoc_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB_par_id: erreur bind du numvocal" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &not_inhibe_from_sql, sizeof(not_inhibe_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB_par_id: erreur bind du not_inhibe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &objet, sizeof(objet), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB_par_id: erreur bind du objet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &sms, sizeof(sms), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_msgDB_par_id: erreur bind du sms" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT num,libelle,type,num_syn,num_voc,not_inhibe,objet,sms FROM %s WHERE id=%d",
                NOM_TABLE_MSG, id );
printf("Requete Message.c: %s\n", requete );
    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Rechercher_msgDB_par_id: recherche failed", id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info_n( log, DEBUG_DB, "Rechercher_msgDB_par_id: recherche ok", id );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info( log, DEBUG_DB, "Rechercher_msgDB_par_id: recherche unsuccessful" );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_msgDB_par_id: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    msg = g_malloc0( sizeof(struct MSGDB) );
    if (!msg)
     { Info( log, DEBUG_MEM, "Rechercher_msgDB_par_id: Mem error" );
       return(NULL);
     }

    msg->id      = id;
    msg->num     = atoi(num);
    memcpy( msg->libelle, libelle, sizeof(msg->libelle) );
    memcpy( msg->objet, objet, sizeof(msg->objet) );
    msg->type    = atoi( type_from_sql );
    msg->num_syn = atoi( numsyn_from_sql );
    msg->num_voc = atoi( numvoc_from_sql );
    msg->not_inhibe   = atoi( not_inhibe_from_sql );
    msg->sms     = atoi( sms );

    return(msg);
  }
/**********************************************************************************************************/
/* Modifier_messageDB: Modification d'un message Watchdog                                                 */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_messageDB( struct LOG *log, struct DB *db, struct CMD_EDIT_MESSAGE *msg )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gchar *libelle, *objet;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Modifier_messageDB: ajout failed: query=null", msg->id );
       return(FALSE);
     }

    libelle = Normaliser_chaine ( log, msg->libelle );
    if (!libelle)
     { Info( log, DEBUG_DB, "Modifier_messageDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    objet = Normaliser_chaine ( log, msg->objet );
    if (!objet)
     { g_free(libelle);
       Info( log, DEBUG_DB, "Modifier_messageDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "num=%d,libelle='%s',type=%d,num_syn=%d,num_voc=%d,not_inhibe=%s,objet='%s',sms=%s "
                "WHERE id=%d",
                NOM_TABLE_MSG, msg->num, libelle, msg->type, 0, 0,
                               (msg->not_inhibe ? "true" : "false"),
                               objet, (msg->sms ? "true" : "false"), msg->id );
    g_free(libelle);
    g_free(objet);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Modifier_messageDB: Modif failed", msg->id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Modifier_messageDB: succes modif user", msg->id );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
