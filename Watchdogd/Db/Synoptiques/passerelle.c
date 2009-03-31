/**********************************************************************************************************/
/* Watchdogd/Db/Synoptiques/passerelle.c       Ajout/retrait de passerelle dans les synoptiques           */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 13 mai 2007 13:41:33 CEST */
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
 #include "Synoptiques_DB.h"

/**********************************************************************************************************/
/* Retirer_passerelleDB: Elimination d'une passerelle                                                     */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_passerelleDB ( struct LOG *log, struct DB *db, struct CMD_ID_PASSERELLE *passerelle )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_passerelleDB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_PASSERELLE, passerelle->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_passerelleDB: elimination failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_passerelleDB: elimination ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Max_id_passerellesDB: Renvoie l'id maximum utilisé + 1 des passerelles                                 */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_passerelleDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_passerelleDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_passerelleDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_PASSERELLE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_passerelleDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_passerelleDB: recherche ok" );

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
 gint Ajouter_passerelleDB ( struct LOG *log, struct DB *db, struct CMD_ADD_PASSERELLE *passerelle )
  { gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gint id;

    id = Max_id_passerelleDB( log, db );
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_passerelleDB: recherche failed: query=null" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,syn_id,syn_cible_id,bitctrl,bitctrl1,bitctrl2,posx,posy,angle)"
                " VALUES (%d,%d,%d,%d,%d,%d,%d,%d,%f)", NOM_TABLE_PASSERELLE,
                id, passerelle->syn_id, passerelle->syn_cible_id, passerelle->bit_controle,
                passerelle->bit_controle_1, passerelle->bit_controle_2,
                passerelle->position_x, passerelle->position_y, passerelle->angle );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Ajouter_passerelleDB: ajout failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Ajouter_passerelleDB: ajout ok" );

    EndQueryDB( log, db, hquery );
    return(id);
  }
/**********************************************************************************************************/
/* Creer_db_passerelle: création des tables associées aux passerelles                                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_passerelle ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_passerelle: Creation DB failed: query=null", NOM_TABLE_PASSERELLE );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id           INTEGER       PRIMARY KEY,"
                                          "  syn_id       INTEGER       NOT NULL,"
                                          "  syn_cible_id INTEGER       NOT NULL,"
                                          "  bitctrl      INTEGER       NOT NULL,"
                                          "  bitctrl1     INTEGER       NOT NULL,"
                                          "  bitctrl2     INTEGER       NOT NULL,"
                                          "  posx         INTEGER       NOT NULL,"
                                          "  posy         INTEGER       NOT NULL,"
                                          "  angle        FLOAT         NOT NULL"
                                          ");",
                                          NOM_TABLE_PASSERELLE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_passerelle: création table failed", NOM_TABLE_PASSERELLE );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_passerelle: succes création table", NOM_TABLE_PASSERELLE );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_passerelleDB ( struct LOG *log, struct DB *db, gint id_syn )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[2048];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.syn_id,%s.syn_cible_id,%s.mnemo,%s.bitctrl,%s.bitctrl1,%s.bitctrl2,"
                "%s.posx,%s.posy,%s.angle"
                " FROM %s,%s WHERE %s.syn_id=%d AND %s.id=%s.syn_cible_id",
                NOM_TABLE_PASSERELLE, NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,
                NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,
                NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE,                                       /* From */
                NOM_TABLE_PASSERELLE, id_syn, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE );   /* Jointure */

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_passerelleDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_passerelleDB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct PASSERELLEDB *Recuperer_passerelleDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id[10], syn_cible_id[10], libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    gchar syn_id[10], bitctrl[10], posx[20], posy[20], bitctrl1[10], bitctrl2[10],angle[20];
    struct PASSERELLEDB *passerelle;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id, sizeof(id), NULL );                          /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &syn_id, sizeof(syn_id), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du syn_id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &syn_cible_id, sizeof(syn_cible_id), NULL );      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du syn_cible_id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &bitctrl, sizeof(bitctrl), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du bitctrl" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &bitctrl1, sizeof(bitctrl1), NULL );              /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du bitctrl" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &bitctrl2, sizeof(bitctrl2), NULL );              /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du bitctrl" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &posx, sizeof(posx), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du posx" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &posy, sizeof(posy), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du posy" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery,10, SQL_C_CHAR, &angle, sizeof(angle), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du angle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    passerelle = (struct PASSERELLEDB *)g_malloc0( sizeof(struct PASSERELLEDB) );
    if (!passerelle) Info( log, DEBUG_MEM, "Recuperer_passerelleDB_suite: Erreur allocation mémoire" );
    else
     { passerelle->id = atoi(id);
       passerelle->syn_id         = atoi(syn_id);               /* Synoptique ou est placée la passerelle */
       passerelle->syn_cible_id   = atoi(syn_cible_id);              /* Synoptique cible de la passerelle */
       passerelle->bit_controle   = atoi(bitctrl);                                          /* Ixxx, Cxxx */
       passerelle->bit_controle_1 = atoi(bitctrl1);                                         /* Ixxx, Cxxx */
       passerelle->bit_controle_2 = atoi(bitctrl2);                                         /* Ixxx, Cxxx */
       passerelle->position_x     = atoi(posx);                              /* en abscisses et ordonnées */
       passerelle->position_y     = atoi(posy);
       passerelle->angle          = atof(angle);
       memcpy ( passerelle->libelle, libelle, sizeof(passerelle->libelle) );
     }
    return(passerelle);
  }
/**********************************************************************************************************/
/* Rechercher_passerelleDB: Recupération du passerelle dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct PASSERELLEDB *Rechercher_passerelleDB ( struct LOG *log, struct DB *db, guint id )
  { gchar libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    gchar bitctrl[10], syn_id[10], syn_cible_id[10],angle[20];
    gchar posx[20], posy[20], bitctrl1[10], bitctrl2[10];
    struct PASSERELLEDB *passerelle;
    gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_passerelleDB: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &syn_id, sizeof(syn_id), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du syn_id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &syn_cible_id, sizeof(syn_cible_id), NULL );      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du syn_cible_id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &bitctrl, sizeof(bitctrl), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du bitctrl" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &bitctrl1, sizeof(bitctrl1), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du bitctrl" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &bitctrl2, sizeof(bitctrl2), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du bitctrl" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &posx, sizeof(posx), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du posx" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &posy, sizeof(posy), NULL );                      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du posy" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &angle, sizeof(angle), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_passerelleDB_suite: erreur bind du angle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.syn_id,%s.syn_cible_id,%s.mnemo,%s.bitctrl,%s.bitctrl1,%s.bitctrl2,"
                "%s.posx,%s.posy,%s.angle "
                "FROM %s,%s WHERE %s.id=%d AND %s.id=%s.syn_cible_id", 
                NOM_TABLE_PASSERELLE, NOM_TABLE_PASSERELLE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_PASSERELLE, NOM_TABLE_PASSERELLE, NOM_TABLE_PASSERELLE, 
                NOM_TABLE_PASSERELLE, NOM_TABLE_PASSERELLE, NOM_TABLE_PASSERELLE, 
                NOM_TABLE_PASSERELLE, NOM_TABLE_SYNOPTIQUE,                                       /* From */
                NOM_TABLE_PASSERELLE, id, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Rechercher_passerelleDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_passerelleDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_paserelleDB: Passerelle non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_passerelleDB: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    passerelle = (struct PASSERELLEDB *)g_malloc0( sizeof(struct PASSERELLEDB) );
    if (!passerelle) Info( log, DEBUG_MEM, "Recuperer_passerelleDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( passerelle->libelle, libelle, sizeof(passerelle->libelle) );  /* Recopie dans la structure */
       passerelle->id             = id;                                        /* Id unique du passerelle */
       passerelle->syn_cible_id   = atoi(syn_cible_id);
       passerelle->syn_id         = atoi(syn_id);
       passerelle->bit_controle   = atoi(bitctrl);                                          /* Ixxx, Cxxx */
       passerelle->bit_controle_1 = atoi(bitctrl1);                                         /* Ixxx, Cxxx */
       passerelle->bit_controle_2 = atoi(bitctrl2);                                         /* Ixxx, Cxxx */
       passerelle->position_x     = atoi(posx);                              /* en abscisses et ordonnées */
       passerelle->position_y     = atoi(posy);
       passerelle->angle          = atof(angle);
     }
    return(passerelle);
  }
/**********************************************************************************************************/
/* Modifier_passerelleDB: Modification d'un passerelle Watchdog                                           */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_passerelleDB( struct LOG *log, struct DB *db, struct CMD_EDIT_PASSERELLE *passerelle )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Modifier_passerelleDB: ajout failed: query=null", passerelle->id );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "bitctrl=%d,bitctrl1=%d,bitctrl2=%d,posx=%d,posy=%d,angle=%f"
                " WHERE id=%d;", NOM_TABLE_PASSERELLE,
                passerelle->bit_controle, passerelle->bit_controle_1, passerelle->bit_controle_2,
                passerelle->position_x, passerelle->position_y, passerelle->angle,
                passerelle->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Modifier_passerelleDB: Modif failed", passerelle->id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Modifier_passerelleDB: succes modif", passerelle->id );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
