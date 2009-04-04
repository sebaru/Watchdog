/**********************************************************************************************************/
/* Watchdogd/Db/Utilisateur/groupe.c    Gestion de l'interface SQL pour les groupes                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 20:32:27 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * groupe.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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
 #include <bonobo/bonobo-i18n.h>                                           /* Pour l'internationalisation */
 #include <unistd.h>
 #include <stdio.h>
 #include <string.h>
 #include <time.h>
 #include <stdlib.h>

 #include "Erreur.h"
 #include "Utilisateur_DB.h"

 static gchar *GROUPE_RESERVE[NBR_GROUPE_RESERVE][2]=
  { { N_("Everybody"),        N_("The default group") },
    { N_("Admin-UserDB"),     N_("Members can add/remove/edit users/groups") },
    { N_("Admin-MsgDB"),      N_("Members can add/remove/edit Msgs") },
    { N_("Admin-iconDB"),     N_("Members can add/remove/edit icons") },
    { N_("Admin-synopDB"),    N_("Members can add/remove/edit syn") },
    { N_("Log"),              N_("Members can see the log") },
    { N_("Admin-dlsDB"),      N_("Members can add/remove/edit DLS plugins") },
    { N_("Admin-histoDB"),    N_("Members can ack/query histo") },
    { N_("Admin-scenarioDB"), N_("Members can add/remove Scenario") }
  };
/************************************ Prototypes des fonctions ********************************************/
 #include "proto_util.h"

/**********************************************************************************************************/
/* Nom_groupe_reserve: renvoie le nom en clair du groupe reserve d'id id                                  */
/* Entrée: l'id du groupe                                                                                 */
/* Sortie: une chaine de caractere non freable                                                            */
/**********************************************************************************************************/
 gchar *Nom_groupe_reserve( gint id )
  { if (id>=NBR_GROUPE_RESERVE) return( _("Unknown") );
    else { return( GROUPE_RESERVE[id][0] ); }
  }
/**********************************************************************************************************/
/* Commentaire_groupe_reserve: renvoie le commentaire en clair du groupe reserve d'id id                  */
/* Entrée: l'id du groupe                                                                                 */
/* Sortie: une chaine de caractere non freable                                                            */
/**********************************************************************************************************/
 gchar *Commentaire_groupe_reserve( gint id )
  { if (id>=NBR_GROUPE_RESERVE) return( _("Unknown") );
    else return( GROUPE_RESERVE[id][1] );
  }
/**********************************************************************************************************/
/* Groupe_set_groupe_utilDB: Positionne les groupes associes a un utilisateur                             */
/* Entrée: une chaine de caractere du type {a,b,c}, a b c entiers                                         */
/* Sortie: une gliste d'entiers                                                                           */
/**********************************************************************************************************/
 gboolean Groupe_set_groupe_utilDB( struct LOG *log, struct DB *db, guint id_util, guint *gids )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gint cpt;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Groupe_set_groupe_utilDB: ajout failed: query=null", id_util );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id_util=%d;",
		NOM_TABLE_GIDS, id_util );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Groupe_set_groupe_utilDB: Delete failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
				   
    for ( cpt=0; cpt<NBR_MAX_GROUPE_PAR_UTIL; cpt++ )
     { g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                   "INSERT INTO %s"
                   "(id_util,gids)"
	           "VALUES (%d, %d);",
                   NOM_TABLE_GIDS, id_util, *(gids + cpt) );

       retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );       /* Execution de la requete SQL */
       if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
        { Info_c( log, DEBUG_DB, "Groupe_set_groupe_utilDB: set failed", requete );
          PrintErrQueryDB( log, db, hquery );
          EndQueryDB( log, db, hquery );
          return(FALSE);
        }
       if ( *(gids + cpt) == 0 ) break;                             /* Le groupe "0" est le groupe de fin */
     }

    Info_c( log, DEBUG_DB, "Groupe_set_groupe_utilDB: set groupe succes", requete );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Groupe_get_groupe_utilDB: Recuperation des groupes d'un utilisateur                                    */
/* Entrées: un log, une db, un id, un tableau d'entiers                                                   */
/* Sortie: une structure GROUPE                                                                           */
/**********************************************************************************************************/
 gboolean Groupe_get_groupe_utilDB( struct LOG *log, struct DB *db, guint id, guint *gids )
  { gchar gid[20];
    gchar requete[200];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    guint cpt;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Groupe_get_groupe_utilDB: ajout failed: query=null" );
       return(FALSE);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &gid, sizeof(gid), NULL );                    /* Bind nom */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Groupe_get_groupe_utilDB: erreur bind du gid" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete), "SELECT gids FROM %s WHERE id_util=%d ORDER BY gids DESC",
                NOM_TABLE_GIDS, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Groupe_get_groupe_utilDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    
    memset ( gids, 0, sizeof(guint)*NBR_MAX_GROUPE_PAR_UTIL );
    cpt=0;
    
    while ( SQLFetch( hquery ) != SQL_NO_DATA && cpt<NBR_MAX_GROUPE_PAR_UTIL)
     { if (atoi(gid) != 0)
        { *(gids + cpt) = atoi(gid);
          cpt++;
        }
     }
    *(gids + cpt) = 0;                                         /* Fin de tableau = groupe "tout le monde" */

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }	    
    
/**********************************************************************************************************/
/* Max_id_groupeDB: Renvoie l'id maximum utilisé + 1 des groupes                                          */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_groupesDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_groupes: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_groupes: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s;", NOM_TABLE_GROUPE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_groupes: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_groupes: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr!=0)
     { SQLFetch( hquery );
       id = atoi(id_from_sql) + 1;
       if (id<100) id = 100;                                        /* On se reserve 100 groupes spéciaux */
     }
    else id = 0;
    EndQueryDB( log, db, hquery );
    return( id );
  }
/**********************************************************************************************************/
/* Rechercher_groupeDB: Recupere un groupe par son id                                                     */
/* Entrées: un log, une db , un id                                                                        */
/* Sortie: une structure GROUPE                                                                           */
/**********************************************************************************************************/
 struct GROUPEDB *Rechercher_groupeDB( struct LOG *log, struct DB *db, gint id )
  { gchar nom[NBR_CARAC_LOGIN_UTF8+1], comment[NBR_CARAC_COMMENTAIRE_UTF8+1];
    struct GROUPEDB *groupe;
    gchar requete[200];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    SQLINTEGER nbr;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_groupeDB: ajout failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &nom, sizeof(nom), NULL );                    /* Bind nom */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_groupesDB: erreur bind du nom" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &comment, sizeof(comment), NULL );        /* Bind comment */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_groupesDB: erreur bind du commentaire" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete), "SELECT name, comment FROM %s WHERE id=%d",
                NOM_TABLE_GROUPE, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Rechercher_groupeDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     } else Info( log, DEBUG_DB, "Rechercher_groupeDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_groupeDB: Groupe non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_groupeDB: Multiple solution", id );

    SQLFetch(hquery);
    EndQueryDB( log, db, hquery );

    groupe = (struct GROUPEDB *)g_malloc0( sizeof(struct GROUPEDB) );
    if (!groupe) Info( log, DEBUG_MEM, "Rechercher_groupeDB: Erreur allocation mémoire" );
    else
     { groupe->id = id;
       g_snprintf( groupe->nom, sizeof(groupe->nom), "%s", nom );            /* Recopie dans la structure */
       g_snprintf( groupe->commentaire, sizeof(groupe->commentaire), "%s", comment );
     }
    return( groupe );
  }
/**********************************************************************************************************/
/* Recuperer_groupeDB: Recupere les groupes                                                               */
/* Entrées: un log, une db                                                                                */
/* Sortie: une structure GROUPE                                                                           */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_groupesDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_groupesDB: ajout failed: query=null" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete), "SELECT id, name, comment FROM %s ORDER BY name", NOM_TABLE_GROUPE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_groupesDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     } else Info( log, DEBUG_DB, "Recuperer_groupesDB: recherche ok" );

   return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_groupesDB_suite: Recupération de l'enregistrement suivant                                    */
/* Entrée: un log et une database                                                                         */
/* Sortie: une structure GROUPE                                                                           */
/**********************************************************************************************************/
 struct GROUPEDB *Recuperer_groupesDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id_from_sql[10], nom[NBR_CARAC_LOGIN_UTF8+1], comment[NBR_CARAC_COMMENTAIRE_UTF8+1];
    struct GROUPEDB *groupe;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );/* Bind groupes */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_groupesDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &nom, sizeof(nom), NULL );                    /* Bind nom */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_groupesDB_suite: erreur bind du nom" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &comment, sizeof(comment), NULL );        /* Bind comment */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_groupesDB_suite: erreur bind du commentaire" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    groupe = (struct GROUPEDB *)g_malloc0( sizeof(struct GROUPEDB) );
    if (!groupe) Info( log, DEBUG_MEM, "Recuperer_groupeDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( groupe->nom, nom, sizeof(groupe->nom) );
       memcpy( groupe->commentaire, comment, sizeof(groupe->commentaire) );
       groupe->id = atoi(id_from_sql);
     }
    return(groupe);
  } 
/**********************************************************************************************************/
/* Tester_groupe_util: renvoie true si l'utilisateur fait partie du groupe en parametre                   */
/* Entrées: un id utilisateur, une liste de groupe, un id de groupe                                       */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Tester_groupe_util( guint id_util, guint *groupes, guint id_groupe )
  { gint cpt;
    if (id_util==UID_ROOT) return(TRUE);                             /* Le tech est dans tous les groupes */
    if (id_groupe==GID_TOUTLEMONDE) return(TRUE);

    cpt=0;
    while( groupes[cpt] )
     { if( groupes[cpt] == id_groupe ) return(TRUE);
       cpt++;
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Retirer_groupe: Elimine un groupe dans la base de données                                              */
/* Entrées: un log, une db, un nom                                                                        */
/* Sortie: true si pas de pb, false sinon                                                                 */
/**********************************************************************************************************/
 gboolean Retirer_groupeDB( struct LOG *log, struct DB *db, struct CMD_ID_GROUPE *groupe )
  { gchar requete[200];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;

    if (groupe->id < NBR_GROUPE_RESERVE)
     { Info_c( log, DEBUG_DB, "Retirer_groupe: elimination failed: id reserve", groupe->nom );
       g_snprintf( db->last_err, sizeof(db->last_err), _("Permission denied") );
       return(FALSE);
     }
       
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Retirer_groupe: elimination failed: query=null", groupe->id );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d",
                NOM_TABLE_GROUPE, groupe->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Retirer_groupe: elimination failed", groupe->id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Retirer_groupe: group destroyed", groupe->id );
/* retirer ce groupe de tous les utilisateurs de la base...
*/
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_groupeDB: ajoute un groupe à la database                                                       */
/* Entrée: log, db, id, nom, comment                                                                      */
/* Sortie: -1 si probleme, id sinon                                                                       */
/**********************************************************************************************************/
 gint Ajouter_groupeDB ( struct LOG *log, struct DB *db, struct CMD_ADD_GROUPE *groupe )
  { gchar *nom, *comment;
    gchar requete[4096];
    SQLHSTMT hquery;
    long retour;
    gint id;
  
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Ajouter_groupeDB: query=null", NOM_TABLE_GROUPE );
       return(-1);
     }

    id      = Max_id_groupesDB  ( log, db );
    nom     = Normaliser_chaine ( log, groupe->nom );
    comment = Normaliser_chaine ( log, groupe->commentaire );
    
    if (!(nom && comment))
     { Info( log, DEBUG_DB, "Ajouter_groupeDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,name,comment) VALUES (%d, '%s', '%s');",
                 NOM_TABLE_GROUPE, id, nom, comment );
    g_free(nom); g_free(comment);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Ajouter_groupeDB: insert failed", groupe->nom );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info_c( log, DEBUG_DB, "Ajouter_groupeDB: insert succes", groupe->nom );
    return(id);
  }
/**********************************************************************************************************/
/* Modifier_groupe: positionne le commentaire du groupe en parametre                                      */
/* Entrées: un log, une db et un id de groupe et un commentaire                                           */
/* Sortie: boolean false si probleme                                                                      */
/**********************************************************************************************************/
 gboolean Modifier_groupeDB( struct LOG *log, struct DB *db, struct CMD_EDIT_GROUPE *groupe )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *comment;

    comment = Normaliser_chaine( log, groupe->commentaire );
    if (!comment) return(FALSE);
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Modifier_groupe: recherche failed: query=null", groupe->nom );
       g_free(comment);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET comment = '%s' WHERE id=%d",
                NOM_TABLE_GROUPE, comment, groupe->id );
    g_free(comment);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Modifier_groupe: update failed", groupe->nom );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       g_free(comment);
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Modifier_groupe: update ok", groupe->nom );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Creer_db_groupe: creation de la db groupes watchdog                                                    */
/* entrées: un log et une db                                                                              */
/* Sortie: true si aucun problème                                                                         */
/**********************************************************************************************************/
 gboolean Creer_db_groupe ( struct LOG *log, struct DB *db )
  { struct CMD_ADD_GROUPE groupe;
    SQLHSTMT hquery;
    gchar requete[4096];
    long retour;
    gint cpt;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_groupe: Creation DB failed: query=null", NOM_TABLE_GROUPE );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  name VARCHAR(%d) UNIQUE NOT NULL,"
                                          "  comment VARCHAR(%d) NOT NULL"
                                          ");",
                                          NOM_TABLE_GROUPE,
                                          NBR_CARAC_LOGIN_UTF8+1, NBR_CARAC_COMMENTAIRE_UTF8+1 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_groupe: création table failed", NOM_TABLE_GROUPE );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Creer_db_groupe: succes création table", NOM_TABLE_GROUPE );

/************************************* Insertion des groupes **********************************************/
    for (cpt=0; cpt<NBR_GROUPE_RESERVE; cpt++)
     { g_snprintf( groupe.nom, sizeof(groupe.nom), "%s", Nom_groupe_reserve(cpt) );
       g_snprintf( groupe.commentaire, sizeof(groupe.commentaire), "%s", Commentaire_groupe_reserve(cpt) );
       if (Ajouter_groupeDB( log, db, &groupe ) == -1) return(FALSE);
     }

    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
