/**********************************************************************************************************/
/* Watchdogd/Db/Scenario/Scenario.c        Déclaration des fonctions pour la gestion des scenario         */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 03 aoû 2008 13:12:35 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Scenario.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "Erreur.h"
 #include "Cst_scenario.h"
 #include "Scenario_DB.h"

/**********************************************************************************************************/
/* Max_id_scenarioDB: Renvoie l'id maximum utilisé + 1 des scenarioDB                                       */
/* Entrées: un log, une db                                                                                */
/* Sortie: un entier                                                                                      */
/**********************************************************************************************************/
 static gint Max_id_scenarioDB( struct LOG *log, struct DB *db )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    SQLINTEGER nbr;
    gchar id_from_sql[10];
    guint id;

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Max_id_scenarioDB: recherche failed: query=null" );
       return(-1);
     }

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &id_from_sql, sizeof(id_from_sql), NULL );     /* Bind id */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_scenarioDB: erreur bind id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT MAX(id) FROM %s", NOM_TABLE_SCENARIO );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Max_id_scenarioDB: recherche failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info( log, DEBUG_DB, "Max_id_scenarioDB: recherche ok" );

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
/* Ajouter_scenarioDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure sc                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_scenarioDB ( struct LOG *log, struct DB *db, struct CMD_ADD_SCENARIO *scenario )
  { gchar requete[4096];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar *libelle;
    gint id;

    id = Max_id_scenarioDB( log, db );
    if (id==-1)
     { Info( log, DEBUG_DB, "Ajouter_scenarioDB: Id max non trouvé" );
       return(-1);
     }

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Ajouter_scenarioDB: recherche failed: query=null" );
       return(-1);
     }

    libelle = Normaliser_chaine ( log, scenario->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_scenarioDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(id,bitm,ts_jour,ts_mois,heure,minute,"
                "lundi,mardi,mercredi,jeudi,vendredi,samedi,dimanche,"
                "janvier,fevrier,mars,avril,mai,juin,juillet,aout,"
                "septembre,octobre,novembre,decembre,actif,libelle)"
                " VALUES "
                "(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
                "%d,%d,%d,%d,%d,'%s')",
                NOM_TABLE_SCENARIO, id, scenario->bit_m, scenario->ts_jour, scenario->ts_mois,
                scenario->heure, scenario->minute,
                scenario->lundi, scenario->mardi, scenario->mercredi, scenario->jeudi, scenario->vendredi,
                scenario->samedi, scenario->dimanche,
                scenario->janvier, scenario->fevrier, scenario->mars, scenario->avril, scenario->mai,
                scenario->juin, scenario->juillet, scenario->aout, scenario->septembre, scenario->octobre,
                scenario->novembre, scenario->decembre,
                scenario->actif, libelle );

    g_free(libelle);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Ajouter_scenarioDB: ajout failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(-1);
     }
    else Info_c( log, DEBUG_DB, "Ajouter_scenarioDB: ajout ok", requete );

    EndQueryDB( log, db, hquery );
    return(id);
  }
/**********************************************************************************************************/
/* Ajouter_scenarioDB: Ajout ou edition d'un entreeANA                                                    */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure scenario                      */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_scenarioDB ( struct LOG *log, struct DB *db, struct CMD_EDIT_SCENARIO *scenario )
  { gchar requete[512], *libelle;
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */

    hquery = NewQueryDB( log, db );            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Updater_scenarioDB: recherche failed: query=null" );
       return(FALSE);
     }

    libelle = Normaliser_chaine ( log, scenario->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Udpate_scenarioDB: Normalisation impossible" );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET bitm=%d,ts_jour=%d,ts_mois=%d,heure=%d,minute=%d,"
                "lundi=%d,mardi=%d,mercredi=%d,jeudi=%d,vendredi=%d,samedi=%d,dimanche=%d,"
                "janvier=%d,fevrier=%d,mars=%d,avril=%d,mai=%d,juin=%d,juillet=%d,aout=%d,"
                "septembre=%d,octobre=%d,novembre=%d,decembre=%d,"
                "actif=%d,libelle='%s'"
                " WHERE id=%d;",
                NOM_TABLE_SCENARIO, scenario->bit_m, scenario->ts_jour, scenario->ts_mois,
                scenario->heure, scenario->minute,
                scenario->lundi, scenario->mardi, scenario->mercredi, scenario->jeudi, scenario->vendredi,
                scenario->samedi, scenario->dimanche,
                scenario->janvier, scenario->fevrier, scenario->mars, scenario->avril, scenario->mai,
                scenario->juin, scenario->juillet, scenario->aout, scenario->septembre, scenario->octobre,
                scenario->novembre, scenario->decembre,
                scenario->actif, libelle,
                scenario->id );
    g_free(libelle);

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Updater_scenarioDB: ajout failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     } else Info_c( log, DEBUG_DB, "Updater_scenarioDB: ajout success", requete );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Retirer_scDB: Elimination d'un message                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_scenarioDB ( struct LOG *log, struct DB *db, struct CMD_ID_SCENARIO *sc )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    
    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Retirer_scenarioDB: recherche failed: query=null" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_SCENARIO, sc->id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Retirer_scenarioDB: elimination failed" );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_c( log, DEBUG_DB, "Retirer_scenarioDB: elimination ok", requete );

    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_scDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_scenarioDB ( struct LOG *log, struct DB *db )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[1024];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_scenarioDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT bitm,ts_jour,ts_mois,heure,minute,libelle,actif,"
                "lundi,mardi,mercredi,jeudi,vendredi,samedi,dimanche,"
                "janvier,fevrier,mars,avril,mai,juin,juillet,aout,"
                "septembre,octobre,novembre,decembre,id"
                " FROM %s ORDER BY libelle", NOM_TABLE_SCENARIO );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_scenarioDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_scenarioDB: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_scDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 SQLHSTMT Recuperer_scenarioDB_par_bitm ( struct LOG *log, struct DB *db, struct CMD_WANT_SCENARIO_MOTIF *sce )
  { SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[1024];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Recuperer_scenarioDB_par_bitm: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT bitm,ts_jour,ts_mois,heure,minute,libelle,actif,"
                "lundi,mardi,mercredi,jeudi,vendredi,samedi,dimanche,"
                "janvier,fevrier,mars,avril,mai,juin,juillet,aout,"
                "septembre,octobre,novembre,decembre,id"
                " FROM %s WHERE bitm=%d OR bitm=%d ORDER BY libelle",
                NOM_TABLE_SCENARIO, sce->bit_clic, sce->bit_clic2 );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Recuperer_scenarioDB_par_bitm: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    else Info( log, DEBUG_DB, "Recuperer_scenarioDB_par_bitm: recherche ok" );

    return(hquery);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_scDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct SCENARIO_DB *Recuperer_scenarioDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery )
  { gchar actif[20], bitm[20], ts_jour[20], ts_mois[20], heure[20], minute[20];
    gchar lundi[20], mardi[20], mercredi[20], jeudi[20], vendredi[20], samedi[20], dimanche[20];
    gchar janvier[20], fevrier[20], mars[20], avril[20], mai[20], juin[20], juillet[20];
    gchar aout[20], septembre[20], octobre[20], novembre[20], decembre[20], id[20];
    gchar libelle[NBR_CARAC_LIBELLE_SCENARIO_UTF8+1];
    struct SCENARIO_DB *sc;
    SQLRETURN retour;

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &bitm, sizeof(bitm), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du bitm" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &ts_jour, sizeof(ts_jour), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du ts_jour" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &ts_mois, sizeof(ts_mois), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du ts_mois" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &heure, sizeof(heure), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du heure" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &minute, sizeof(minute), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du minute" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &actif, sizeof(actif), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du actif" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &lundi, sizeof(lundi), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du lundi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &mardi, sizeof(mardi), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du mardi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 10, SQL_C_CHAR, &mercredi, sizeof(mercredi), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du mercredi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 11, SQL_C_CHAR, &jeudi, sizeof(jeudi), NULL );                   /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du jeudi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 12, SQL_C_CHAR, &vendredi, sizeof(vendredi), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du vendredi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 13, SQL_C_CHAR, &samedi, sizeof(samedi), NULL );                 /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du samedi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 14, SQL_C_CHAR, &dimanche, sizeof(dimanche), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du dimanche" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 15, SQL_C_CHAR, &janvier, sizeof(janvier), NULL );               /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du janvier" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 16, SQL_C_CHAR, &fevrier, sizeof(fevrier), NULL );               /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du fevrier" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 17, SQL_C_CHAR, &mars, sizeof(mars), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du mars" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 18, SQL_C_CHAR, &avril, sizeof(avril), NULL );                   /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du avril" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 19, SQL_C_CHAR, &mai, sizeof(mai), NULL );                       /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du mai" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 20, SQL_C_CHAR, &juin, sizeof(juin), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du juin" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 21, SQL_C_CHAR, &juillet, sizeof(juillet), NULL );               /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du juillet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 22, SQL_C_CHAR, &aout, sizeof(aout), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du aout" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 23, SQL_C_CHAR, &septembre, sizeof(septembre), NULL );           /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du septembre" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 24, SQL_C_CHAR, &octobre, sizeof(octobre), NULL );               /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du octobre" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 25, SQL_C_CHAR, &novembre, sizeof(novembre), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du novembre" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 26, SQL_C_CHAR, &decembre, sizeof(decembre), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du decembre" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 27, SQL_C_CHAR, &id, sizeof(id), NULL );                         /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du id" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    sc = (struct SCENARIO_DB *)g_malloc0( sizeof(struct SCENARIO_DB) );
    if (!sc) Info( log, DEBUG_MEM, "Recuperer_scenarioDB_suite: Erreur allocation mémoire" );
    else
     { sc->id        = atoi(id);
       sc->bit_m     = atoi(bitm);
       sc->ts_mois   = atoi(ts_mois);
       sc->ts_jour   = atoi(ts_jour);
       sc->heure     = atoi(heure);
       sc->minute    = atoi(minute);
       sc->actif     = atoi(actif);
       sc->lundi     = atoi(lundi);
       sc->mardi     = atoi(mardi);
       sc->mercredi  = atoi(mercredi);
       sc->jeudi     = atoi(jeudi);
       sc->vendredi  = atoi(vendredi);
       sc->samedi    = atoi(samedi);
       sc->dimanche  = atoi(dimanche);
       sc->janvier   = atoi(janvier);
       sc->fevrier   = atoi(fevrier);
       sc->mars      = atoi(mars);
       sc->avril     = atoi(avril);
       sc->mai       = atoi(mai);
       sc->juin      = atoi(juin);
       sc->juillet   = atoi(juillet);
       sc->aout      = atoi(aout);
       sc->septembre = atoi(septembre);
       sc->octobre   = atoi(octobre);
       sc->novembre  = atoi(novembre);
       sc->decembre  = atoi(decembre);
       g_snprintf( sc->libelle, sizeof(sc->libelle), "%s", libelle );
     }
    return(sc);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_scenarioDB: Recupération de la liste des ids des entreeANAs                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct SCENARIO_DB *Rechercher_scenarioDB ( struct LOG *log, struct DB *db, guint id )
  { gchar actif[20], bitm[20], ts_jour[20], ts_mois[20], heure[20], minute[20];
    gchar lundi[20], mardi[20], mercredi[20], jeudi[20], vendredi[20], samedi[20], dimanche[20];
    gchar janvier[20], fevrier[20], mars[20], avril[20], mai[20], juin[20], juillet[20];
    gchar aout[20], septembre[20], octobre[20], novembre[20], decembre[20];
    gchar libelle[NBR_CARAC_LIBELLE_SCENARIO_UTF8+1];
    struct SCENARIO_DB *scenario;
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */
    gchar requete[1024];

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: recherche failed: query=null" );
       return(NULL);
     }
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT bitm,ts_jour,ts_mois,heure,minute,libelle,actif,"
                "lundi,mardi,mercredi,jeudi,vendredi,samedi,dimanche,"
                "janvier,fevrier,mars,avril,mai,juin,juillet,aout,"
                "septembre,octobre,novembre,decembre"
                " FROM %s WHERE id=%d", NOM_TABLE_SCENARIO, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Rechercher_scenarioDB: recherche failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    /*else Info_c( log, DEBUG_DB, "Rechercher_scenarioDB: recherche ok", requete );*/

    retour = SQLBindCol( hquery, 1, SQL_C_CHAR, &bitm, sizeof(bitm), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du bitm" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 2, SQL_C_CHAR, &ts_jour, sizeof(ts_jour), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du ts_jour" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 3, SQL_C_CHAR, &ts_mois, sizeof(ts_mois), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du ts_mois" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 4, SQL_C_CHAR, &heure, sizeof(heure), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du heure" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 5, SQL_C_CHAR, &minute, sizeof(minute), NULL );                  /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du minute" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 6, SQL_C_CHAR, &libelle, sizeof(libelle), NULL );                /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du libelle" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 7, SQL_C_CHAR, &actif, sizeof(actif), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du actif" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 8, SQL_C_CHAR, &lundi, sizeof(lundi), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du lundi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 9, SQL_C_CHAR, &mardi, sizeof(mardi), NULL );                    /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du mardi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 10, SQL_C_CHAR, &mercredi, sizeof(mercredi), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du mercredi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 11, SQL_C_CHAR, &jeudi, sizeof(jeudi), NULL );                   /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du jeudi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 12, SQL_C_CHAR, &vendredi, sizeof(vendredi), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du vendredi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 13, SQL_C_CHAR, &samedi, sizeof(samedi), NULL );                 /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du samedi" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 14, SQL_C_CHAR, &dimanche, sizeof(dimanche), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du dimanche" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 15, SQL_C_CHAR, &janvier, sizeof(janvier), NULL );               /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du janvier" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 16, SQL_C_CHAR, &fevrier, sizeof(fevrier), NULL );               /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du fevrier" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 17, SQL_C_CHAR, &mars, sizeof(mars), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du mars" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 18, SQL_C_CHAR, &avril, sizeof(avril), NULL );                   /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du avril" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 19, SQL_C_CHAR, &mai, sizeof(mai), NULL );                       /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du mai" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 20, SQL_C_CHAR, &juin, sizeof(juin), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du juin" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 21, SQL_C_CHAR, &juillet, sizeof(juillet), NULL );               /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du juillet" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 22, SQL_C_CHAR, &aout, sizeof(aout), NULL );                     /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du aout" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 23, SQL_C_CHAR, &septembre, sizeof(septembre), NULL );           /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du septembre" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 24, SQL_C_CHAR, &octobre, sizeof(octobre), NULL );               /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du octobre" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 25, SQL_C_CHAR, &novembre, sizeof(novembre), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du novembre" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }
    retour = SQLBindCol( hquery, 26, SQL_C_CHAR, &decembre, sizeof(decembre), NULL );             /* Bind */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info( log, DEBUG_DB, "Rechercher_scenarioDB: erreur bind du decembre" );
       PrintErrDB( log, db );
       EndQueryDB( log, db, hquery );
       return(NULL);
     }

    if ( SQLFetch( hquery ) == SQL_NO_DATA )
     { EndQueryDB( log, db, hquery );
       return(NULL);
     }

    EndQueryDB( log, db, hquery );
    scenario = (struct SCENARIO_DB *)g_malloc0( sizeof(struct SCENARIO_DB) );
    if (!scenario) Info( log, DEBUG_MEM, "Rechercher_scenarioDB: Erreur allocation mémoire" );
    else
     { scenario->id        = id;
       scenario->bit_m     = atoi(bitm);
       scenario->ts_mois   = atoi(ts_mois);
       scenario->ts_jour   = atoi(ts_jour);
       scenario->heure     = atoi(heure);
       scenario->minute    = atoi(minute);
       scenario->actif     = atoi(actif);
       scenario->lundi     = atoi(lundi);
       scenario->mardi     = atoi(mardi);
       scenario->mercredi  = atoi(mercredi);
       scenario->jeudi     = atoi(jeudi);
       scenario->vendredi  = atoi(vendredi);
       scenario->samedi    = atoi(samedi);
       scenario->dimanche  = atoi(dimanche);
       scenario->janvier   = atoi(janvier);
       scenario->fevrier   = atoi(fevrier);
       scenario->mars      = atoi(mars);
       scenario->avril     = atoi(avril);
       scenario->mai       = atoi(mai);
       scenario->juin      = atoi(juin);
       scenario->juillet   = atoi(juillet);
       scenario->aout      = atoi(aout);
       scenario->septembre = atoi(septembre);
       scenario->octobre   = atoi(octobre);
       scenario->novembre  = atoi(novembre);
       scenario->decembre  = atoi(decembre);
       g_snprintf( scenario->libelle, sizeof(scenario->libelle), "%s", libelle );
     }
    return(scenario);
  }
/*--------------------------------------------------------------------------------------------------------*/
