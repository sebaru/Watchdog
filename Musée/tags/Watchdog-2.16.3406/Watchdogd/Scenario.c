/******************************************************************************************************************************/
/* Watchdogd/Scenario.c        Déclaration des fonctions pour la gestion des Scenario                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    15.08.2017 09:58:27 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

/******************************************************************************************************************************/
/* Charger_messages: Chargement de la configuration des messages depuis la DB vers la running config                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Check_scenario_tick_thread ( void )
  { gchar requete[512];
    struct DB *db;
#ifdef bouh
    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT num,persist"
                " FROM %s",
                NOM_TABLE_MSG );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while ( Recuperer_ligne_SQL(db) )                                                      /* Chargement d'une ligne resultat */
     { gint num;
       num = atoi( db->row[0] );
       if (num < NBR_MESSAGE_ECRITS)
        { Partage->g[num].persist = atoi( db->row[1] );
          Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                    "%s: Chargement config MSG[%04d]", __func__, num );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
			       "%s: num (%d) out of range (max=%d)", __func__, num, NBR_MESSAGE_ECRITS ); }
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: DB reloaded", __func__ );
    Libere_DB_SQL (&db);
#endif
  }
/******************************************************************************************************************************/
/* Ajouter_scenario_detailsDB: Ajout d'un tick dans la base                                                                   */
/* Entrée: un numéro de scenario et un tick                                                                                   */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Set_scenario_detailsDB ( gint num, GSList *Liste )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),  "DELETE FROM %s WHERE num=%d", NOM_TABLE_SCENARIO_TICK, num );    /* Requete SQL */
    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Delete ScenarioDetail num '%d' failed", __func__, num );
       Libere_DB_SQL(&db);
       return(FALSE);
     }

    while (Liste)
     { struct SCENARIO_TICK *detail;
       detail = (struct SCENARIO_TICK *)Liste->data;

       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "INSERT INTO %s(num,minute,heure,jour,date,mois,mnemo_id) VALUES "
                   "('%d','%d','%d','%d','%d','%d')", NOM_TABLE_SCENARIO_TICK, num,
                   detail->minute, detail->heure, detail->jour, detail->date, detail->mois, detail->mnemo_id
                 );
       Lancer_requete_SQL ( db, requete );                                                     /* Execution de la requete SQL */
       Liste = g_slist_remove ( Liste, detail );
       g_free(detail);
     }
    Libere_DB_SQL(&db); 
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Recuperer_scenario_detailsDB: Recupere les details du scenario en parametre                                                */
/* Entrée: une database de retour et l'id du scenario                                                                         */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Recuperer_scenario_detailsDB ( struct DB **db_retour, gint num )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT s.id,s.num,s.minute,s.heure,s.jour,s.date,s.mois, mnemo.num, mnemo.libelle"
                " FROM %s as s"
                " INNER JOIN %s as mnemo ON mnemo.id=s.mnemo_id"
                " WHERE mnemo.type=%d AND s.num=%d",
                NOM_TABLE_SCENARIO_TICK, NOM_TABLE_MNEMO, MNEMO_MONOSTABLE, num
              );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_scenario_detailDB_suite: Recupération de la liste des details du scenario                                        */
/* Entrée: la base de données précédemment ouverte par la fonction ci dessus                                                  */
/* Sortie: une structure SCENARIO_TICK                                                                                      */
/******************************************************************************************************************************/
 struct SCENARIO_TICK *Recuperer_scenario_detailsDB_suite( struct DB **db_orig )
  { struct SCENARIO_TICK *sce;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    sce = (struct SCENARIO_TICK *)g_try_malloc0( sizeof(struct SCENARIO_TICK) );
    if (!sce)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
       return(NULL);
     }
                                                                                                 /* Recopie dans la structure */
    g_snprintf( sce->mnemo_libelle, sizeof(sce->mnemo_libelle), "%s", db->row[8] );
    sce->id        = atoi(db->row[0]);
    sce->num       = atoi(db->row[1]);
    sce->minute    = atoi(db->row[2]);
    sce->heure     = atoi(db->row[3]);
    sce->jour      = atoi(db->row[4]);
    sce->date      = atoi(db->row[5]);
    sce->mois      = atoi(db->row[6]);
    sce->mnemo_num = atoi(db->row[7]);
    return(sce);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
