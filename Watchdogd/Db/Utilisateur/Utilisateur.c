/**********************************************************************************************************/
/* Watchdogd/Db/Utilisateur/Utilisateur.c    Interface DB mots de passe pour watchdog2.0                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 20:35:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Utilisateur.c
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
 #include <glib.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>

 #include "Erreur.h"
 #include "Utilisateur_DB.h"

 static gchar *UTILISATEURDB_RESERVE[NBR_UTILISATEUR_RESERVE][2]=
  { { N_("root"), N_("Watchdog administrator") }
  };

/************************************ Prototypes des fonctions ********************************************/
 #include "proto_util.h"

/**********************************************************************************************************/
/* Nom_groupe_reserve: renvoie le nom en clair du groupe reserve d'id id                                  */
/* Entrée: l'id du groupe                                                                                 */
/* Sortie: une chaine de caractere non freable                                                            */
/**********************************************************************************************************/
 gchar *Nom_utilisateur_reserve( gint id )
  { if (id>=NBR_UTILISATEUR_RESERVE) return( _("Unknown") );
    else { return( UTILISATEURDB_RESERVE[id][0] ); }
  }
/**********************************************************************************************************/
/* Commentaire_groupe_reserve: renvoie le commentaire en clair du groupe reserve d'id id                  */
/* Entrée: l'id du groupe                                                                                 */
/* Sortie: une chaine de caractere non freable                                                            */
/**********************************************************************************************************/
 gchar *Commentaire_utilisateur_reserve( gint id )
  { if (id>=NBR_UTILISATEUR_RESERVE) return( _("Unknown") );
    else return( UTILISATEURDB_RESERVE[id][1] );
  }
/**********************************************************************************************************/
/* Retirer_utilisateur: Elimine un utilisateur dans la base de données                                    */
/* Entrées: un log, une db, un nom                                                                        */
/* Sortie: true si pas de pb, false sinon                                                                 */
/**********************************************************************************************************/
 gboolean Retirer_utilisateurDB( struct LOG *log, struct DB *db, struct CMD_ID_UTILISATEUR *util )
  { gchar requete[512];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;

    if (util->id < NBR_UTILISATEUR_RESERVE) 
     { Info_c( log, DEBUG_DB, "Retirer_utilisateurDB: elimination failed: id reserve", util->nom );
       g_snprintf( db->last_err, sizeof(db->last_err), _("Permission denied") );
       return(FALSE);
     }

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Retirer_utilisateurDB: elimination failed: query=null", util->nom );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_UTIL, util->id );
    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Retirer_utilisateurDB: elimination failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Retirer_utilisateurDB: succes elimination user", requete );

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id_util=%d", NOM_TABLE_GIDS, util->id );
    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Retirer_utilisateurDB: elimination gids failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Retirer_utilisateurDB: succes elimination gids user", requete );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Max_id_utilisateurs: Renvoie l'id maximum utilisé + 1 des utilisateurs                                 */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_utilisateursDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_utilisateurs: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_utilisateurs: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_UTIL );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_utilisateurs: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_utilisateurs: recherche ok" );

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
/* Ajouter_utilisateur: Ajout d'un utilisateur dans la base de données                                    */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gint Ajouter_utilisateurDB( struct LOG *log, struct DB *db, gchar *clef,
                             struct CMD_ADD_UTILISATEUR *util )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gchar *crypt;
    gint id;

    id = Max_id_utilisateursDB( log, db );
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Ajouter_utilisateurDB: ajout failed: query=null", util->nom );
       return(-1);
     }

    if(g_utf8_strlen(util->code_en_clair, -1))
         { crypt = Crypter( log, clef, util->code_en_clair ); }
    else { crypt = Crypter( log, clef, "bouh" ); }

    if (crypt)
     { gchar *nom, *comment, *code_crypt;

       nom        = Normaliser_chaine ( log, util->nom );                /* Formatage correct des chaines */
       comment    = Normaliser_chaine ( log, util->commentaire );
       code_crypt = Normaliser_chaine ( log, crypt );
       g_free(crypt);

       if (!(nom && comment && code_crypt))
        { Info( log, DEBUG_DB, "Ajouter_utilisateurDB: Normalisation impossible" );
          return(-1);
        }

       g_snprintf( requete, sizeof(requete),                                              /* Requete SQL */
                   "INSERT INTO %s"             
                   "(id,name,changepass,cansetpass,crypt,comment,login_failed,enable,"
                   "date_create,enable_expire,date_expire,date_modif)"
                   "VALUES (%d, '%s', %s, %s, '%s', '%s', '0', true, %d, %s, '%d', '%d' );",
                   NOM_TABLE_UTIL, id, nom,
                   (util->changepass ? "true" : "false"),
                   (util->cansetpass ? "true" : "false"), code_crypt,
                   comment, (gint)time(NULL),
                   (util->expire ? "true" : "false"), (gint)util->date_expire,
                   (gint)time(NULL) );
       g_free(nom);
       g_free(comment);

       retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );       /* Execution de la requete SQL */
       if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
        { Info_c( log, DEBUG_DB, "Ajouter_utilisateurDB: ajout failed", util->nom );
          PrintErrQueryDB( log, db, hquery );
          EndQueryDB( log, db, hquery );
          return(-1);
        }
       else Info_c( log, DEBUG_DB, "Ajouter_utilisateurDB: succes creation user", util->nom );
       EndQueryDB( log, db, hquery );

       Groupe_set_groupe_utilDB ( log, db, id, (guint *)&util->gids );      /* Positionnement des groupes */ 

     } else { Info_c( log, DEBUG_DB, "Ajouter_utilisateurDB: failed to encrypt password for", util->nom );
              EndQueryDB( log, db, hquery );
              return(-1);
            }
    return(id);
  }
/**********************************************************************************************************/
/* Modifier_utilisateurDB: Modification d'u nutilisateur Watchdog                                         */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_utilisateurDB( struct LOG *log, struct DB *db, gchar *clef,
                                  struct CMD_EDIT_UTILISATEUR *util )
  { gchar requete[1024], chaine[100];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gchar *comment;
    gchar *crypt;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Modifier_utilisateurDB: ajout failed: query=null", util->nom );
       return(FALSE);
     }

    comment = Normaliser_chaine ( log, util->commentaire );
    
    if (!comment)
     { Info( log, DEBUG_DB, "Modifier_utilisateurDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                              /* Requete SQL */
                "UPDATE %s SET "             
                "changepass=%s,comment='%s',enable=%s,enable_expire=%s,"
                "cansetpass=%s,date_expire=%d,date_modif='%d'",
                NOM_TABLE_UTIL, (util->changepass ? "true" : "false"), comment,
                                (util->actif ? "true" : "false"),
                                (util->expire ? "true" : "false"),
                                (util->cansetpass ? "true" : "false"),
                                (gint)util->date_expire,
                                (gint)time(NULL) );
    g_free(comment);

    if (util->setpassnow)
     { gchar *code_crypt;
       if(g_utf8_strlen(util->code_en_clair, -1))
            { crypt = Crypter( log, clef, util->code_en_clair ); }
       else { crypt = Crypter( log, clef, "bouh" ); }
       if (!crypt)
        { Info( log, DEBUG_DB, "Modifier_utilisateurDB: cryptage password impossible" );
          EndQueryDB( log, db, hquery );
          return(FALSE);
        }

       code_crypt = Normaliser_chaine ( log, crypt );
       g_free(crypt);
       if (!code_crypt)
        { Info( log, DEBUG_DB, "Modifier_utilisateurDB: Normalisation code crypte impossible" );
          EndQueryDB( log, db, hquery );
          return(FALSE);
        }
       g_snprintf( chaine, sizeof(chaine), ",crypt='%s'", code_crypt );
       g_strlcat ( requete, chaine, sizeof(requete) );
       g_free(code_crypt);
     }

    g_snprintf( chaine, sizeof(chaine), " WHERE id='%d'", util->id ); 
    g_strlcat ( requete, chaine, sizeof(requete) );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Modifier_utilisateurDB: Modif failed", util->nom );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Modifier_utilisateurDB: succes modif user", util->nom );
    EndQueryDB( log, db, hquery );
    Groupe_set_groupe_utilDB ( log, db, util->id, (guint *)&util->gids );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Creer_db_util: creation de la db utilisateurs watchdog                                                 */
/* entrées: un log et une db                                                                              */
/* Sortie: true si aucun problème                                                                         */
/**********************************************************************************************************/
 gboolean Creer_db_util ( struct LOG *log, gchar *clef, struct DB *db )
  { struct CMD_ADD_UTILISATEUR util;
    gchar requete[4096];
    SQLHSTMT hquery;
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_util: Creation DB failed: query=null", NOM_TABLE_UTIL );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  name VARCHAR(%d) UNIQUE NOT NULL,"
                                          "  changepass boolean NOT NULL,"/* l'util. doit changer de pass */
                                          "  cansetpass boolean NOT NULL,"/* l'util. peut changer de pass */
                                          "  crypt CHAR(%d) NOT NULL,"
                                          "  comment VARCHAR(%d),"
                                          "  login_failed INTEGER NOT NULL,"
                                          "  enable BOOLEAN NOT NULL,"
                                          "  date_create INTEGER NOT NULL,"
                                          "  enable_expire BOOLEAN NOT NULL,"
                                          "  date_expire INTEGER," 
                                          "  date_modif INTEGER" 
                                          ");",
                                          NOM_TABLE_UTIL, NBR_CARAC_LOGIN_UTF8,
                                          NBR_CARAC_CODE_CRYPTE, NBR_CARAC_COMMENTAIRE_UTF8 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_util: création table failed", NOM_TABLE_UTIL );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Creer_db_util: succes création table", NOM_TABLE_UTIL );

    EndQueryDB( log, db, hquery );
/*********************************** Insertion du technicien **********************************************/
    g_snprintf( util.nom, sizeof(util.nom), "%s", Nom_utilisateur_reserve(UID_ROOT) );
    g_snprintf( util.commentaire, sizeof(util.commentaire), Commentaire_utilisateur_reserve(UID_ROOT) );
    g_snprintf( util.code_en_clair, sizeof(util.code_en_clair), "bouh" );
    util.date_expire = 0;
    util.expire      = FALSE;
    util.changepass  = FALSE;
    util.cansetpass  = TRUE;
/* Gestion des groupes ?? */
    if (Ajouter_utilisateurDB( log, db, clef, &util ) == -1)
     { return(FALSE); }

    return(TRUE);
  }
/**********************************************************************************************************/
/* Creer_db_gids: creation de la db utilisateurs watchdog                                                 */
/* entrées: un log et une db                                                                              */
/* Sortie: true si aucun problème                                                                         */
/**********************************************************************************************************/
 gboolean Creer_db_gids ( struct LOG *log, struct DB *db )
  { gchar requete[4096];
    SQLHSTMT hquery;
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_gids: Creation DB failed: query=null", NOM_TABLE_GIDS );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id_util INTEGER NOT NULL,"
                                          "  gids INTEGER NOT NULL"
                                          ");",
                                          NOM_TABLE_GIDS );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_gids: création table failed", NOM_TABLE_GIDS );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Creer_db_gids: succes création table", NOM_TABLE_GIDS );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_utilsDB: Recuperation de tous les champs des utilisateurs                                   */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_utilsDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
  
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_utilsDB: recherche failed: query=null" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT name,id,changepass,comment,enable,date_create,"
                "enable_expire,date_expire,cansetpass,date_modif "
                "FROM %s", NOM_TABLE_UTIL );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_utilsDB: recherche ok" );

    return( hquery );
  }
/**********************************************************************************************************/
/* Rechercher_utilsDB: Recuperation de tous les champs des utilisateurs                                   */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 struct UTILISATEURDB *Recuperer_utilsDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { SQLRETURN retour;
    gchar nom[NBR_CARAC_LOGIN_UTF8+1], change_from_sql[10], comment_from_sql[NBR_CARAC_COMMENTAIRE_UTF8+1];
    gchar actif_from_sql[10], date_create[30], id_from_sql[10], cansetpass[10];
    gchar expire[10], date_expire[30], date_modif[30]; /*, groupes[TAILLE_CHAINE_GROUPES];*/
    struct UTILISATEURDB *util;
  
    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &nom, sizeof(nom), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB_suite: erreur bind du nom" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB_suite: erreur bind du id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &change_from_sql, sizeof(change_from_sql), NULL );/* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB_suite: erreur bind du changepass" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &comment_from_sql, sizeof(comment_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB_suite: erreur bind du comment" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &actif_from_sql, sizeof(actif_from_sql), NULL );  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB_suite: erreur bind de l'actif" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &date_create, sizeof(date_create), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB_suite: erreur bind du date_create" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &expire, sizeof(expire), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB_suite: erreur bind du expire" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &date_expire, sizeof(date_expire), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB_suite: erreur bind du date_expire" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &cansetpass, sizeof(cansetpass), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB_suite: erreur bind du cansetpass" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 10, SQL_C_CHAR, &date_modif, sizeof(date_modif), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_utilsDB_suite: erreur bind du date_modif" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    util = (struct UTILISATEURDB *)g_malloc0( sizeof(struct UTILISATEURDB) );
    if (!util) Info( log, DEBUG_MEM, "Recuperer_utilsDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( util->nom, nom, sizeof(util->nom) );                          /* Recopie dans la structure */
       memcpy( util->commentaire, comment_from_sql, sizeof(util->commentaire) );
       util->id            = atoi(id_from_sql);
       util->date_creation = atoi(date_create);
       util->actif         = atoi(actif_from_sql);
       util->changepass    = atoi(change_from_sql);
       util->expire        = atoi(expire);
       util->date_expire   = atoi(date_expire);
       util->cansetpass    = atoi(cansetpass);
       util->date_modif    = atoi(date_modif);
     }
    return( util );
  }
/**********************************************************************************************************/
/* Rechercher_utilDB: Recuperation de tous les champs des utilisateurs                                    */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 struct UTILISATEURDB *Rechercher_utilisateurDB( struct LOG *log, struct DB *db, gint id )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar nom[NBR_CARAC_LOGIN_UTF8+1], change_from_sql[10], comment_from_sql[NBR_CARAC_COMMENTAIRE_UTF8];
    gchar actif_from_sql[10], date_create[30], date_modif[30], cansetpass[10];;
    gchar expire[10], date_expire[30];/*, groupes[TAILLE_CHAINE_GROUPES];*/
    struct UTILISATEURDB *util;
    SQLINTEGER nbr;
  
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Rechercher_utilisateurDB: recherche failed: query=null", id );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &nom, sizeof(nom), NULL );                        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Rechercher_utilisateurDB: erreur bind du nom", id );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &change_from_sql, sizeof(change_from_sql), NULL );/* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_utilisateurDB: erreur bind du changepass" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &comment_from_sql, sizeof(comment_from_sql), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_utilisateurDB: erreur bind du comment" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &actif_from_sql, sizeof(actif_from_sql), NULL );  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_utilisateurDB: erreur bind de l'actif" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &date_create, sizeof(date_create), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_utilisateurDB: erreur bind du date_create" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &expire, sizeof(expire), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_utilisateurDB: erreur bind du expire" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &date_expire, sizeof(date_expire), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_utilisateurDB: erreur bind du date_expire" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &cansetpass, sizeof(cansetpass), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_utilisateurDB: erreur bind du cansetpass" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &date_modif, sizeof(date_modif), NULL );
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_utilisateurDB: erreur bind du date_modif" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT name,changepass,comment,enable,"
                "date_create,enable_expire,date_expire,cansetpass,date_modif "
                "FROM %s WHERE id=%d", NOM_TABLE_UTIL, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_utilisateurDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_utilisateurDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_utilisateurDB: Utilisateur non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_utilisateurDB: Multiple solution", id );

    SQLFetch(hquery);
    EndQueryDB( log, db, hquery );

    util = (struct UTILISATEURDB *)g_malloc0( sizeof(struct UTILISATEURDB) );
    if (!util) Info( log, DEBUG_MEM, "Rechercher_utilisateurDB: Erreur allocation mémoire" );
    else
     { g_snprintf( util->nom, sizeof(util->nom), "%s", nom );             /* Recopie dans la structure */
       g_snprintf( util->commentaire, sizeof(util->commentaire), "%s", comment_from_sql );
       util->id            = id;
       util->date_creation = atoi(date_create);
       util->actif         = atoi(actif_from_sql);
       util->changepass    = atoi(change_from_sql);
       util->expire        = atoi(expire);
       util->date_expire   = atoi(date_expire);
       util->cansetpass    = atoi(cansetpass);
       util->date_modif    = atoi(date_modif);
     }
    return( util );
  }
/*--------------------------------------------------------------------------------------------------------*/
