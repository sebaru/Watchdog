/**********************************************************************************************************/
/* Watchdogd/Scenario/Scenario.c        Déclaration des fonctions pour la gestion des scenario            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 03 aoû 2008 13:12:35 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Scenario.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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

 #include "watchdogd.h"

/**********************************************************************************************************/
/* Charger_scenario: Applique un scenario si celui-ci est dans la période requise                         */
/* Entrée: le numéro du scenario                                                                          */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Checker_scenario ( guint num )
  { struct tm *temps;
    time_t timet;

    if ( !Partage->scenario[num].actif ) return;

    time(&timet);                                                     /* On récupère la date et heure NOW */
    temps = localtime( (time_t *)&timet );

    if ( ! (temps->tm_min == Partage->scenario[num].minute &&
            temps->tm_hour == Partage->scenario[num].heure ) ) return;

    if ( ! Partage->scenario[num].ts_mois )
     { if ( temps->tm_mon == 0  && ! Partage->scenario[num].janvier   ) return;
       if ( temps->tm_mon == 1  && ! Partage->scenario[num].fevrier   ) return;
       if ( temps->tm_mon == 2  && ! Partage->scenario[num].mars      ) return;
       if ( temps->tm_mon == 3  && ! Partage->scenario[num].avril     ) return;
       if ( temps->tm_mon == 4  && ! Partage->scenario[num].mai       ) return;
       if ( temps->tm_mon == 5  && ! Partage->scenario[num].juin      ) return;
       if ( temps->tm_mon == 6  && ! Partage->scenario[num].juillet   ) return;
       if ( temps->tm_mon == 7  && ! Partage->scenario[num].aout      ) return;
       if ( temps->tm_mon == 8  && ! Partage->scenario[num].septembre ) return;
       if ( temps->tm_mon == 9  && ! Partage->scenario[num].octobre   ) return;
       if ( temps->tm_mon == 10 && ! Partage->scenario[num].novembre  ) return;
       if ( temps->tm_mon == 11 && ! Partage->scenario[num].decembre  ) return;
     }
    if ( ! Partage->scenario[num].ts_jour )
     { if ( temps->tm_wday == 1  && ! Partage->scenario[num].lundi    ) return;
       if ( temps->tm_wday == 2  && ! Partage->scenario[num].mardi    ) return;
       if ( temps->tm_wday == 3  && ! Partage->scenario[num].mercredi ) return;
       if ( temps->tm_wday == 4  && ! Partage->scenario[num].jeudi    ) return;
       if ( temps->tm_wday == 5  && ! Partage->scenario[num].vendredi ) return;
       if ( temps->tm_wday == 6  && ! Partage->scenario[num].samedi   ) return;
       if ( temps->tm_wday == 0  && ! Partage->scenario[num].dimanche ) return;
     }

    Info_n( Config.log, DEBUG_INFO, "MSRV: Scenario Active num", num );
    Info_n( Config.log, DEBUG_INFO, "MSRV: Scenario Active bit", Partage->scenario[num].bit_m );
    Envoyer_commande_dls( Partage->scenario[num].bit_m );
  }
/**********************************************************************************************************/
/* Charger_scenario: Chargement des infos sur les scenario depuis la DB                                   */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_scenario ( void )
  { struct DB *db;
    gint i;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database,        /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info( Config.log, DEBUG_INFO, "Charger_scenario: Connexion DB failed" );
       return;
     }                                                                           /* Si pas de histos (??) */

    for (i = 0; i<NBR_SCENARIO; i++)
     { struct SCENARIO_DB *sc;
       sc = Rechercher_scenarioDB ( Config.log, db, i );
       if (sc)
        { memcpy ( &Partage->scenario[i], sc, sizeof(struct SCENARIO_DB) );
          g_free(sc);
        } else Partage->scenario[i].actif = FALSE;
     }
    Libere_DB_SQL( Config.log, &db );
  }
/**********************************************************************************************************/
/* Ajouter_scenarioDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure sc                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_scenarioDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_SCENARIO *scenario )
  { gchar requete[4096];
    gchar *libelle;

    libelle = Normaliser_chaine ( log, scenario->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_scenarioDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(bitm,ts_jour,ts_mois,heure,minute,"
                "lundi,mardi,mercredi,jeudi,vendredi,samedi,dimanche,"
                "janvier,fevrier,mars,avril,mai,juin,juillet,aout,"
                "septembre,octobre,novembre,decembre,actif,libelle)"
                " VALUES "
                "(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
                "%d,%d,%d,%d,%d,'%s')",
                NOM_TABLE_SCENARIO, scenario->bit_m, scenario->ts_jour, scenario->ts_mois,
                scenario->heure, scenario->minute,
                scenario->lundi, scenario->mardi, scenario->mercredi, scenario->jeudi, scenario->vendredi,
                scenario->samedi, scenario->dimanche,
                scenario->janvier, scenario->fevrier, scenario->mars, scenario->avril, scenario->mai,
                scenario->juin, scenario->juillet, scenario->aout, scenario->septembre, scenario->octobre,
                scenario->novembre, scenario->decembre,
                scenario->actif, libelle );

    g_free(libelle);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Ajouter_scenarioDB: Ajout ou edition d'un entreeANA                                                    */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure scenario                      */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_scenarioDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_SCENARIO *scenario )
  { gchar requete[512], *libelle;

    libelle = Normaliser_chaine ( log, scenario->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Udpate_scenarioDB: Normalisation impossible" );
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

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Retirer_scDB: Elimination d'un message                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_scenarioDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_SCENARIO *sc )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_SCENARIO, sc->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_scDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_scenarioDB ( struct LOG *log, struct DB *db )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT bitm,ts_jour,ts_mois,heure,minute,libelle,actif,"
                "lundi,mardi,mercredi,jeudi,vendredi,samedi,dimanche,"
                "janvier,fevrier,mars,avril,mai,juin,juillet,aout,"
                "septembre,octobre,novembre,decembre,id"
                " FROM %s ORDER BY libelle", NOM_TABLE_SCENARIO );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_scDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_scenarioDB_par_bitm ( struct LOG *log, struct DB *db, struct CMD_WANT_SCENARIO_MOTIF *sce )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT bitm,ts_jour,ts_mois,heure,minute,libelle,actif,"
                "lundi,mardi,mercredi,jeudi,vendredi,samedi,dimanche,"
                "janvier,fevrier,mars,avril,mai,juin,juillet,aout,"
                "septembre,octobre,novembre,decembre,id"
                " FROM %s WHERE bitm=%d OR bitm=%d ORDER BY libelle",
                NOM_TABLE_SCENARIO, sce->bit_clic, sce->bit_clic2 );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_scDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct SCENARIO_DB *Recuperer_scenarioDB_suite( struct LOG *log, struct DB *db )
  { struct SCENARIO_DB *sc;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    sc = (struct SCENARIO_DB *)g_malloc0( sizeof(struct SCENARIO_DB) );
    if (!sc) Info( log, DEBUG_MEM, "Recuperer_scenarioDB_suite: Erreur allocation mémoire" );
    else
     { sc->id        = atoi(db->row[26]);
       sc->bit_m     = atoi(db->row[0]);
       sc->ts_jour   = atoi(db->row[1]);
       sc->ts_mois   = atoi(db->row[2]);
       sc->heure     = atoi(db->row[3]);
       sc->minute    = atoi(db->row[4]);
       sc->actif     = atoi(db->row[6]);
       sc->lundi     = atoi(db->row[7]);
       sc->mardi     = atoi(db->row[8]);
       sc->mercredi  = atoi(db->row[9]);
       sc->jeudi     = atoi(db->row[10]);
       sc->vendredi  = atoi(db->row[11]);
       sc->samedi    = atoi(db->row[12]);
       sc->dimanche  = atoi(db->row[13]);
       sc->janvier   = atoi(db->row[14]);
       sc->fevrier   = atoi(db->row[15]);
       sc->mars      = atoi(db->row[16]);
       sc->avril     = atoi(db->row[17]);
       sc->mai       = atoi(db->row[18]);
       sc->juin      = atoi(db->row[19]);
       sc->juillet   = atoi(db->row[20]);
       sc->aout      = atoi(db->row[21]);
       sc->septembre = atoi(db->row[22]);
       sc->octobre   = atoi(db->row[23]);
       sc->novembre  = atoi(db->row[24]);
       sc->decembre  = atoi(db->row[25]);
       g_snprintf( sc->libelle, sizeof(sc->libelle), "%s", db->row[5] );
     }
    return(sc);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_scenarioDB: Recupération de la liste des ids des entreeANAs                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct SCENARIO_DB *Rechercher_scenarioDB ( struct LOG *log, struct DB *db, guint id )
  { struct SCENARIO_DB *sc;
    gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT bitm,ts_jour,ts_mois,heure,minute,libelle,actif,"
                "lundi,mardi,mercredi,jeudi,vendredi,samedi,dimanche,"
                "janvier,fevrier,mars,avril,mai,juin,juillet,aout,"
                "septembre,octobre,novembre,decembre"
                " FROM %s WHERE id=%d", NOM_TABLE_SCENARIO, id );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    sc = (struct SCENARIO_DB *)g_malloc0( sizeof(struct SCENARIO_DB) );
    if (!sc) Info( log, DEBUG_MEM, "Rechercher_scenarioDB: Erreur allocation mémoire" );
    else
     { sc->id        = id;
       sc->bit_m     = atoi(db->row[0]);
       sc->ts_jour   = atoi(db->row[1]);
       sc->ts_mois   = atoi(db->row[2]);
       sc->heure     = atoi(db->row[3]);
       sc->minute    = atoi(db->row[4]);
       sc->actif     = atoi(db->row[6]);
       sc->lundi     = atoi(db->row[7]);
       sc->mardi     = atoi(db->row[8]);
       sc->mercredi  = atoi(db->row[9]);
       sc->jeudi     = atoi(db->row[10]);
       sc->vendredi  = atoi(db->row[11]);
       sc->samedi    = atoi(db->row[12]);
       sc->dimanche  = atoi(db->row[13]);
       sc->janvier   = atoi(db->row[14]);
       sc->fevrier   = atoi(db->row[15]);
       sc->mars      = atoi(db->row[16]);
       sc->avril     = atoi(db->row[17]);
       sc->mai       = atoi(db->row[18]);
       sc->juin      = atoi(db->row[19]);
       sc->juillet   = atoi(db->row[20]);
       sc->aout      = atoi(db->row[21]);
       sc->septembre = atoi(db->row[22]);
       sc->octobre   = atoi(db->row[23]);
       sc->novembre  = atoi(db->row[24]);
       sc->decembre  = atoi(db->row[25]);
       g_snprintf( sc->libelle, sizeof(sc->libelle), "%s", db->row[5] );
     }
    Liberer_resultat_SQL ( log, db );
    return(sc);
  }
/*--------------------------------------------------------------------------------------------------------*/
