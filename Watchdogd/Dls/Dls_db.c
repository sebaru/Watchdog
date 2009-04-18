/**********************************************************************************************************/
/* Watchdogd/Db/Dls/Dls_db       Database DLS (gestion des noms de prgs, ...)                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 18 avr 2009 12:52:09 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Dls_db.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

 #include <sql.h>                                             /* Entetes de gestion de la base de données */
 #include <sqlext.h> 
 #include <sqltypes.h>
 #include <glib.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>

 #include "Erreur.h"
 #include "Dls_DB.h"

/**********************************************************************************************************/
/* Retirer_dlsDB: Elimine un prg DLS dans la base de données                                              */
/* Entrées: un log, une db, un nom                                                                        */
/* Sortie: true si pas de pb, false sinon                                                                 */
/**********************************************************************************************************/
 gboolean Retirer_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_ID_PLUGIN_DLS *dls )
  { guchar chaine[200];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Retirer_dlsDB: elimination failed: query=null", dls->nom );
       return(FALSE);
     }

    g_snprintf( (gchar *)chaine, sizeof(chaine),                                           /* Requete SQL */
                "DELETE FROM %s WHERE id=%d",
                NOM_TABLE_DLS, dls->id );
    retour = SQLExecDirect( hquery, chaine, SQL_NTS );                     /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Retirer_dlsDB: elimination failed", dls->nom );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Retirer_dlsDB: succes elimination prg", dls->nom );
    EndQueryDB( log, db, hquery );

    g_snprintf( (gchar *)chaine, sizeof(chaine), "%d.dls", dls->id );
    unlink( (gchar *)chaine );
    g_snprintf( (gchar *)chaine, sizeof(chaine), "lib%d.dls.so", dls->id );
    unlink( (gchar *)chaine );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Max_id_dlsDB: Renvoie l'id maximum utilisé + 1 des prg dls                                             */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_dlsDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_dlsDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_dlsDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_DLS );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_dlsDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_dlsDB: recherche ok" );

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
/* Ajouter_dlsDB: Ajout d'un programme DLS dans la base de données                                        */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gint Ajouter_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_ADD_PLUGIN_DLS *dls )
  { gchar requete[200];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gchar *nom;
    gint id;

    id = Max_id_dlsDB( log, db );
    if (id==-1)
     { Info( log, DEBUG_DB, "Ajouter_dlsDB: Id max non trouvé" );
       return(-1);
     }

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Ajouter_dlsDB: ajout failed: query=null", dls->nom );
       return(-1);
     }

    nom = Normaliser_chaine ( log, dls->nom );                           /* Formatage correct des chaines */
    if (!nom)
     { Info( log, DEBUG_DB, "Ajouter_dlsDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                   "INSERT INTO %s"             
                   "(id,name,actif)"
                   "VALUES (%d, '%s',%s);",
                   NOM_TABLE_DLS, id, nom, (dls->on ? "true" : "false") );
    g_free(nom);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Ajouter_dlsDB: ajout failed", dls->nom );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info_c( log, DEBUG_DB, "Ajouter_dlsDB: succes creation user", dls->nom );
    EndQueryDB( log, db, hquery );
    return(id);
  }

/**********************************************************************************************************/
/* Creer_db_dls: creation de la db dls watchdog                                                           */
/* entrées: un log et une db                                                                              */
/* Sortie: true si aucun problème                                                                         */
/**********************************************************************************************************/
 gboolean Creer_db_dls ( struct LOG *log, struct DB *db )
  { gchar requete[4096];
    SQLHSTMT hquery;
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_dls: Creation DB failed: query=null", NOM_TABLE_DLS );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete),
                                          "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  name VARCHAR(%d) UNIQUE NOT NULL,"
                                          "  actif BOOLEAN NOT NULL"
                                          ");",
                                          NOM_TABLE_DLS, NBR_CARAC_PLUGIN_DLS_UTF8 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_dls: création table failed", NOM_TABLE_DLS );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Creer_db_dls: succes création table", NOM_TABLE_DLS );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_plugins_dlsDB: Recuperation de tous les plugins D.L.S                                        */
/* Entrées: un log, une db                                                                                */
/* Sortie: une hquery, null si erreur                                                                     */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_plugins_dlsDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
  
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_plugins_dlsDB: recherche failed: query=null" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                         /* Requete SQL */
                "SELECT name,id,actif "
                "FROM %s ORDER BY name", NOM_TABLE_DLS );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_plugins_dlsDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_plugins_dlsDB: recherche ok" );

    return( hquery );
  }
/**********************************************************************************************************/
/* Recuperer_plugins_dlsDB_suite: poursuite de la recherche des plugins DLS                               */
/* Entrées: une hquery                                                                                    */
/* Sortie: une structure plugindls, ou null si erreur ou fin de requete                                   */
/**********************************************************************************************************/
 struct PLUGIN_DLS *Recuperer_plugins_dlsDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar nom[NBR_CARAC_PLUGIN_DLS_UTF8+1], id_from_sql[10], actif[10];
    struct PLUGIN_DLS *dls;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &nom, sizeof(nom), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_plugins_dlsDB_suite: erreur bind du nom" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_plugins_dlsDB_suite: erreur bind du id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &actif, sizeof(actif), NULL );                   /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_plugins_dlsDB_suite: erreur bind du actif" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    dls = (struct PLUGIN_DLS *)g_malloc0( sizeof(struct PLUGIN_DLS) );
    if (!dls) Info( log, DEBUG_MEM, "Recuperer_plugins_dlsDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( dls->nom, nom, sizeof(dls->nom) );                            /* Recopie dans la structure */
       dls->id = atoi(id_from_sql);
       dls->on = atoi(actif);
     }
    return( dls );
  }
/**********************************************************************************************************/
/* Rechercher_utilDB: Recuperation de tous les champs des utilisateurs                                    */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 struct PLUGIN_DLS *Rechercher_plugin_dlsDB( struct LOG *log, struct DB *db, gint id )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar nom[NBR_CARAC_PLUGIN_DLS_UTF8+1], actif[10];
    struct PLUGIN_DLS *dls;
    SQLINTEGER nbr;
  
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Rechercher_dlsDB: recherche failed: query=null", id );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &nom, sizeof(nom), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Rechercher_dlsDB: erreur bind du nom", id );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &actif, sizeof(actif), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Rechercher_dlsDB: erreur bind du actif", id );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT name,actif "
                "FROM %s WHERE id=%d", NOM_TABLE_DLS, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_dlsDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_dlsDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_dlsDB: DLS non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_dlsDB: Multiple solution", id );

    SQLFetch(hquery);
    EndQueryDB( log, db, hquery );

    dls = (struct PLUGIN_DLS *)g_malloc0( sizeof(struct PLUGIN_DLS) );
    if (!dls) Info( log, DEBUG_MEM, "Rechercher_dlsDB: Erreur allocation mémoire" );
    else
     { g_snprintf( dls->nom, sizeof(dls->nom), "%s", nom );                  /* Recopie dans la structure */
       dls->id = id;
       dls->on = atoi(actif);
     }
    return( dls );
  }
/**********************************************************************************************************/
/* Modifier_plugin_dlsDB: Modification d'un plugin Watchdog                                               */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_EDIT_PLUGIN_DLS *dls )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gchar *nom;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Modifier_plugin_dlsDB: ajout failed: query=null", dls->id );
       return(FALSE);
     }

    nom = Normaliser_chaine ( log, dls->nom );                           /* Formatage correct des chaines */
    if (!nom)
     { Info( log, DEBUG_DB, "Modifier_plugin_dlsDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "name='%s',actif=%d WHERE id=%d",
                NOM_TABLE_DLS, nom, dls->on, dls->id );
    g_free(nom);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Modifier_plugin_dlsDB: Modif failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
