/**********************************************************************************************************/
/* Watchdogd/Db/Synoptiques/Synoptiques.c       Déclaration des fonctions pour la gestion des synoptiques */
/* Projet WatchDog version 2.0       Gestion d'habitat                     sam 04 avr 2009 11:28:56 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Synoptiques.c
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
 #include "Utilisateur_DB.h"
 #include "Icones_DB.h"

/**********************************************************************************************************/
/* Tester_groupe_synoptique: renvoie true si l'utilisateur fait partie du groupe en parametre             */
/* Entrées: un id utilisateur, une liste de groupe, un id de groupe                                       */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Tester_groupe_synoptique( struct LOG *log, struct DB *db, struct UTILISATEURDB *util, guint syn_id )
  { struct SYNOPTIQUEDB *syn;
    gint cpt;

    if (util->id==UID_ROOT) return(TRUE);                            /* Le tech est dans tous les groupes */

    syn = Rechercher_synoptiqueDB ( log, db, syn_id );
    if (!syn) return(FALSE);

    cpt=0;
    while( util->gids[cpt] )
     { if( util->gids[cpt] == syn->groupe )
        { g_free(syn);
          return(TRUE);
        }
       cpt++;
     }
    g_free(syn);
    return(FALSE);
  }
/**********************************************************************************************************/
/* Retirer_msgDB: Elimination d'un synoptique                                                             */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_synoptiqueDB ( struct LOG *log, struct DB *db, struct CMD_ID_SYNOPTIQUE *syn )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    if (syn->id == 0) return(TRUE);                                 /* Le synoptique 0 est indestructible */
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_synoptiqueDB: recherche failed: query=null" );
       return(FALSE);
     }

/****************************************** Retrait de la base SYN ****************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_SYNOPTIQUE, syn->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination ok" );

/****************************************** Retrait des capteurs ******************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_id=%d", NOM_TABLE_CAPTEUR, syn->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination capteur failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination capteur ok" );

/****************************************** Retrait des comment *******************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_id=%d", NOM_TABLE_COMMENT, syn->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination comment failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination comment ok" );

/****************************************** Retrait des motif *********************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn=%d", NOM_TABLE_MOTIF, syn->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination syn failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination syn ok" );

/****************************************** Retrait des palette *******************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_cible_id=%d", NOM_TABLE_PALETTE, syn->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination palette failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination palette ok" );

/****************************************** Retrait des passerelle ****************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_cible_id=%d", NOM_TABLE_PASSERELLE, syn->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination passerelle failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info( log, DEBUG_DB, "Retirer_synoptiqueDB: elimination passerelle ok" );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Max_id_synoptiquesDB: Renvoie l'id maximum utilisé + 1 des synoptiques                                 */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_synoptiquesDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_synoptiquesDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_synoptiquesDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_SYNOPTIQUE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_synoptiquesDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_synoptiquesDB: recherche ok" );

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
 gint Ajouter_synoptiqueDB ( struct LOG *log, struct DB *db, struct CMD_ADD_SYNOPTIQUE *syn )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *libelle, *mnemo;
    gint id;

    id = Max_id_synoptiquesDB ( log, db );
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_synoptiqueDB: recherche failed: query=null" );
       return(-1);
     }

    libelle = Normaliser_chaine ( log, syn->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_synoptiqueDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    mnemo = Normaliser_chaine ( log, syn->mnemo );                       /* Formatage correct des chaines */
    if (!mnemo)
     { Info( log, DEBUG_DB, "Ajouter_synoptiqueDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       g_free(libelle);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                "INSERT INTO %s(id,libelle,mnemo,groupe) VALUES "
                "(%d,'%s','%s',%d)", NOM_TABLE_SYNOPTIQUE, id, libelle, mnemo, syn->groupe );
    g_free(libelle);
    g_free(mnemo);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Ajouter_synoptiqueDB: ajout failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Ajouter_synoptiqueDB: ajout ok" );

    EndQueryDB( log, db, hquery );
    return(id);
  }
/**********************************************************************************************************/
/* Creer_db_synoptique: création des tables associées aux synoptiques                                     */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si pb                                                                                    */
/**********************************************************************************************************/
 gboolean Creer_db_synoptique ( struct LOG *log, struct DB *db )
  { struct CMD_ADD_SYNOPTIQUE add_syn;
    SQLHSTMT hquery;
    gchar requete[4096];
    long retour;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_c( log, DEBUG_DB, "Creer_db_synoptique: Creation DB failed: query=null", NOM_TABLE_SYNOPTIQUE );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete), "CREATE TABLE %s"                                /* Requete SQL */
                                          "( id INTEGER PRIMARY KEY,"
                                          "  libelle VARCHAR(%d) NOT NULL,"
                                          "  mnemo VARCHAR(%d) NOT NULL,"
                                          "  groupe INTEGER NOT NULL"
                                          ");",
                                          NOM_TABLE_SYNOPTIQUE, NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1,
                                          NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Creer_db_synoptique: création table failed", NOM_TABLE_SYNOPTIQUE );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Creer_db_synoptique: succes création table", NOM_TABLE_SYNOPTIQUE );

    EndQueryDB( log, db, hquery );
/***************************** Ajout du synoptique par défaut *********************************************/
    g_snprintf( add_syn.libelle, sizeof(add_syn.libelle), _("General") );
    g_snprintf( add_syn.mnemo,   sizeof(add_syn.mnemo),   _("General") );
    add_syn.groupe = GID_TOUTLEMONDE;
    Ajouter_synoptiqueDB( log, db, &add_syn );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_synoptiqueDB ( struct LOG *log, struct DB *db )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[200];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_synoptiqueDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,mnemo,groupe"
                " FROM %s ORDER BY id", NOM_TABLE_SYNOPTIQUE );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_synoptiqueDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_synoptiqueDB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct SYNOPTIQUEDB *Recuperer_synoptiqueDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar id_from_sql[10], libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar mnemo[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1], groupe[10];
    struct SYNOPTIQUEDB *syn;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );        /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_synoptiqueDB_suite: erreur bind de l'id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_synoptiqueDB_suite: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &mnemo, sizeof(mnemo), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_synoptiqueDB_suite: erreur bind du mnemo" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &groupe, sizeof(groupe), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Recuperer_synoptiqueDB_suite: erreur bind du groupe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    syn = (struct SYNOPTIQUEDB *)g_malloc0( sizeof(struct SYNOPTIQUEDB) );
    if (!syn) Info( log, DEBUG_MEM, "Recuperer_synoptiqueDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( syn->libelle, libelle, sizeof(syn->libelle) );                /* Recopie dans la structure */
       memcpy( syn->mnemo, mnemo, sizeof(syn->mnemo) );                      /* Recopie dans la structure */
       syn->id          = atoi(id_from_sql);
       syn->groupe      = atoi(groupe);
     }
    return(syn);
  }
/**********************************************************************************************************/
/* Rechercher_synoptiqueDB: Recupération du synoptique dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct SYNOPTIQUEDB *Rechercher_synoptiqueDB ( struct LOG *log, struct DB *db, guint id )
  { gchar libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar mnemo[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1], groupe[10];
    struct SYNOPTIQUEDB *syn;
    gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    
    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_synoptiqueDB: recherche failed: query=null" );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_synoptiqueDB: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &mnemo, sizeof(mnemo), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_synoptiqueDB: erreur bind du mnemo" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &groupe, sizeof(groupe), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_synoptiqueDB: erreur bind du groupe" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle,mnemo,groupe FROM %s WHERE id=%d", NOM_TABLE_SYNOPTIQUE, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_synoptiqueDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Rechercher_synoptiqueDB: recherche ok" );

    SQLRowCount( hquery, &nbr );
    if (nbr==0)
     { Info_n( log, DEBUG_DB, "Rechercher_synoptiqueDB: Synoptique non trouvé dans la BDD", id );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    if (nbr>1) Info_n( log, DEBUG_DB, "Rechercher_synoptiqueDB: Multiple solution", id );
    SQLFetch( hquery );
    EndQueryDB( log, db, hquery );

    syn = (struct SYNOPTIQUEDB *)g_malloc0( sizeof(struct SYNOPTIQUEDB) );
    if (!syn)
     { Info( log, DEBUG_MEM, "Rechercher_synoptiqueDB: Mem error" );
       return(NULL);
     }

    syn->id      = id;
    memcpy( syn->libelle, libelle, sizeof(syn->libelle) );                /* Recopie dans la structure */
    memcpy( syn->mnemo, mnemo, sizeof(syn->mnemo) );                      /* Recopie dans la structure */
    syn->groupe  = atoi(groupe);
    return(syn);
  }
/**********************************************************************************************************/
/* Modifier_synoptiqueDB: Modification d'un synoptique Watchdog                                           */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_synoptiqueDB( struct LOG *log, struct DB *db, struct CMD_EDIT_SYNOPTIQUE *syn )
  { gchar requete[1024];
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLRETURN retour;
    gchar *libelle, *mnemo;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Modifier_synoptiqueDB: ajout failed: query=null", syn->id );
       return(FALSE);
     }

    libelle = Normaliser_chaine ( log, syn->libelle );
    if (!libelle)
     { Info( log, DEBUG_DB, "Modifier_synoptiqueDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    mnemo = Normaliser_chaine ( log, syn->mnemo );
    if (!libelle)
     { Info( log, DEBUG_DB, "Modifier_synoptiqueDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       g_free(libelle);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                              /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s',mnemo='%s',groupe=%d WHERE id=%d",
                NOM_TABLE_SYNOPTIQUE, libelle, mnemo, syn->groupe, syn->id );
    g_free(libelle);
    g_free(mnemo);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_n( log, DEBUG_DB, "Modifier_synoptiqueDB: Modif failed", syn->id );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Modifier_synoptiqueDB: succes modif", syn->id );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
