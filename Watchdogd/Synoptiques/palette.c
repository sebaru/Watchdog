/**********************************************************************************************************/
/* Watchdogd/Db/Synoptiques/palette.c       Ajout/retrait de palette dans les synoptiques                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                       jeu 03 fév 2005 14:23:39 CET */
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
/* Retirer_paletteDB: Elimination d'une palette                                                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_paletteDB ( struct LOG *log, struct DB *db, struct CMD_ID_PALETTE *palette )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_paletteDB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_PALETTE, palette->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_paletteDB: elimination failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_paletteDB: elimination ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Max_id_palettesDB: Renvoie l'id maximum utilisé + 1 des palettes                                 */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_paletteDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_paletteDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_paletteDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_PALETTE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_paletteDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_paletteDB: recherche ok" );

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
/* Max_id_palettesDB: Renvoie l'id maximum utilisé + 1 des palettes                                 */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_pos_paletteDB( struct LOG *log, struct DB *db, gint syn_id )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar pos_from_sql[10];
    guint pos;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_pos_paletteDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &pos_from_sql, sizeof(pos_from_sql), NULL );   /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_pos_paletteDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT COUNT(*) FROM %s WHERE syn_id=%d", NOM_TABLE_PALETTE, syn_id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Max_pos_paletteDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info_c( log, DEBUG_DB, "Max_pos_paletteDB: recherche ok", requete );

    pos = atoi(pos_from_sql) + 1;

    EndQueryDB( log, db, hquery );
    return( pos );
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_paletteDB ( struct LOG *log, struct DB *db, struct CMD_ADD_PALETTE *palette )
  { gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gint id, max_pos;

    id = Max_id_paletteDB( log, db );
    max_pos = Max_pos_paletteDB(log, db, palette->syn_id);
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_paletteDB: recherche failed: query=null" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,syn_id,syn_cible_id,pos)"
                " VALUES (%d,%d,%d,%d)", NOM_TABLE_PALETTE,
                id, palette->syn_id, palette->syn_cible_id, max_pos );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Ajouter_paletteDB: ajout failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info_c( log, DEBUG_DB, "Ajouter_paletteDB: ajout ok", requete );

    EndQueryDB( log, db, hquery );
    return(id);
  }
/**********************************************************************************************************/
/* Creer_db_palette: création des tables associées aux palettes                                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_palette ( struct LOG *log, struct DB *db )
  { SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_palette: Creation DB failed: query=null", NOM_TABLE_PALETTE );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id           INTEGER       PRIMARY KEY,"
                                          "  syn_id       INTEGER       NOT NULL,"
                                          "  syn_cible_id INTEGER       NOT NULL,"
                                          "  pos          INTEGER       NOT NULL"
                                          ");",
                                          NOM_TABLE_PALETTE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_palette: création table failed", NOM_TABLE_PALETTE );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_palette: succes création table", NOM_TABLE_PALETTE );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_paletteDB ( struct LOG *log, struct DB *db, gint id_syn )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[2048];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_paletteDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.syn_id,%s.syn_cible_id,%s.mnemo,%s.pos"
                " FROM %s,%s WHERE %s.syn_id=%d AND %s.id=%s.syn_cible_id ORDER BY %s.pos",
                NOM_TABLE_PALETTE, NOM_TABLE_PALETTE, NOM_TABLE_PALETTE,
                NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_PALETTE,
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PALETTE,                                          /* From */
                NOM_TABLE_PALETTE, id_syn, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PALETTE,           /* Jointure */
                NOM_TABLE_PALETTE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_paletteDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_paletteDB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct PALETTEDB *Recuperer_paletteDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id[10], syn_cible_id[10], libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    gchar syn_id[10], pos[20];
    struct PALETTEDB *palette;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id, sizeof(id), NULL );                          /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_paletteDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &syn_id, sizeof(syn_id), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_paletteDB_suite: erreur bind du syn_id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &syn_cible_id, sizeof(syn_cible_id), NULL );      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_paletteDB_suite: erreur bind du syn_cible_id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_paletteDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &pos, sizeof(pos), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_paletteDB_suite: erreur bind du pos" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    palette = (struct PALETTEDB *)g_malloc0( sizeof(struct PALETTEDB) );
    if (!palette) Info( log, DEBUG_MEM, "Recuperer_paletteDB_suite: Erreur allocation mémoire" );
    else
     { palette->id = atoi(id);
       palette->syn_id = atoi(syn_id);                             /* Synoptique ou est placée la palette */
       palette->syn_cible_id = atoi(syn_cible_id);                      /* Synoptique cible de la palette */
       palette->position = atoi(pos);                                        /* en abscisses et ordonnées */
       memcpy ( palette->libelle, libelle, sizeof(palette->libelle) );
     }
    return(palette);
  }
/**********************************************************************************************************/
/* Rechercher_paletteDB: Recupération du palette dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct PALETTEDB *Rechercher_paletteDB ( struct LOG *log, struct DB *db, guint id )
  { gchar libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    gchar syn_id[10], syn_cible_id[10];
    gchar pos[20];
    struct PALETTEDB *palette;
    gchar requete[512];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_paletteDB: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &syn_id, sizeof(syn_id), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_paletteDB_suite: erreur bind du syn_id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &syn_cible_id, sizeof(syn_cible_id), NULL );      /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_paletteDB_suite: erreur bind du syn_cible_id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_paletteDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &pos, sizeof(pos), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_paletteDB_suite: erreur bind du posx" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.syn_id,%s.syn_cible_id,%s.mnemo,%s.pos "
                "FROM %s,%s WHERE %s.id=%d AND %s.id=%s.syn_cible_id", 
                NOM_TABLE_PALETTE, NOM_TABLE_PALETTE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_PALETTE,
                NOM_TABLE_PALETTE, NOM_TABLE_SYNOPTIQUE,                                          /* From */
                NOM_TABLE_PALETTE, id, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PALETTE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Rechercher_paletteDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_paletteDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_paletteDB: Palette non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_paletteDB: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    palette = (struct PALETTEDB *)g_malloc0( sizeof(struct PALETTEDB) );
    if (!palette) Info( log, DEBUG_MEM, "Recuperer_paletteDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( palette->libelle, libelle, sizeof(palette->libelle) );        /* Recopie dans la structure */
       palette->id = id;                                                          /* Id unique du palette */
       palette->syn_cible_id = atoi(syn_cible_id);
       palette->syn_id = atoi(syn_id);
       palette->position = atoi(pos);                                        /* en abscisses et ordonnées */
     }
    return(palette);
  }
/**********************************************************************************************************/
/* Modifier_paletteDB: Modification d'un palette Watchdog                                                 */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_paletteDB( struct LOG *log, struct DB *db, struct CMD_EDIT_PALETTE *palette )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Modifier_paletteDB: ajout failed: query=null", palette->id );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "pos=(SELECT pos FROM %s WHERE id=%d)"
                " WHERE syn_id=%d AND pos=%d;"
                "UPDATE %s SET "             
                "pos=%d"
                " WHERE id=%d;",
                NOM_TABLE_PALETTE, NOM_TABLE_PALETTE, palette->id,
                palette->syn_id,
                palette->position,
                NOM_TABLE_PALETTE,
                palette->position,
                palette->id
              );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Modifier_paletteDB: Modif failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Modifier_paletteDB: succes modif", palette->id );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
