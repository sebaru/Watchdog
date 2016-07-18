/**********************************************************************************************************/
/* Watchdogd/Mnemo_AI.c        Déclaration des fonctions pour la gestion des Analog Input                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 18 avr 2009 13:30:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Mnemo_AI.c
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
/* Recuperer_analogInputDB: Recupération de la liste des EANA                                             */
/* Entrée: un pointeur sur une nouvelle connexion DB                                                      */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gboolean Recuperer_analogInputDB ( struct DB **db_retour )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_analogInputDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.num,%s.min,%s.max,%s.type,%s.unite"
                " FROM %s"
                " INNER JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.type=%d ORDER BY %s.num",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI,
                NOM_TABLE_MNEMO,                                                                  /* FROM */
                NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO,                    /* INNER JOIN */
                NOM_TABLE_MNEMO, MNEMO_ENTREE_ANA,                                               /* WHERE */
                NOM_TABLE_MNEMO                                                               /* Order by */
              );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_analogInputDB_suite: Recupération de la liste des ids des entreeANAs                         */
/* Entrée: le pointeur sur la connexion DB en cours                                                       */
/* Sortie: une structure hébergeant une analogInput                                                       */
/**********************************************************************************************************/
 static struct CMD_TYPE_MNEMO_AI *Recuperer_analogInputDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_MNEMO_AI *mnemo_ai;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo_ai = (struct CMD_TYPE_MNEMO_AI *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_AI) );
    if (!mnemo_ai) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                             "Recuperer_analogInputDB_suite: Erreur allocation mémoire" );
    else
     { mnemo_ai->num      = atoi(db->row[0]);
       mnemo_ai->min      = atof(db->row[1]);
       mnemo_ai->max      = atof(db->row[2]);
       mnemo_ai->type     = atoi(db->row[3]);
       g_snprintf( mnemo_ai->unite, sizeof(mnemo_ai->unite), "%s", db->row[4] );
     }
    return(mnemo_ai);
  }
/**********************************************************************************************************/
/* Rechercher_mnemo_aiDB: Recupération de la conf de l'entrée analogique en parametre                     */
/* Entrée: l'id a récupérer                                                                               */
/* Sortie: une structure hébergeant l'entrée analogique                                                   */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMO_AI *Rechercher_mnemo_aiDB ( guint id )
  { struct CMD_TYPE_MNEMO_AI *mnemo_ai;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_mnemo_aiDB: DB connexion failed" );
       return(NULL);
     }
 
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.num,%s.min,%s.max,%s.type,%s.unite"
                " FROM %s"
                " INNER JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.id_mnemo=%d",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI,
                NOM_TABLE_MNEMO,                                                                  /* FROM */
                NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO,                    /* INNER JOIN */
                NOM_TABLE_MNEMO_AI, id                                                           /* WHERE */
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                       /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return(NULL);
     }

    mnemo_ai = Recuperer_analogInputDB_suite( &db );
    if (mnemo_ai) Libere_DB_SQL( &db );
    return(mnemo_ai);
  }
/**********************************************************************************************************/
/* Modifier_analogInputDB: Modification d'un entreeANA Watchdog                                           */
/* Entrées: une structure hébergeant l'entrée analogique a modifier                                       */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_mnemo_aiDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;
    gchar *unite;

    unite = Normaliser_chaine ( mnemo_full->mnemo_ai.unite );            /* Formatage correct des chaines */
    if (!unite)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_analogInputDB: Normalisation unite impossible" );
       return(FALSE);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_analogInputDB: DB connexion failed" );
       g_free(unite);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s (id_mnemo,min,max,unite,type) VALUES "
                "('%d','%f','%f','%s','%d') "
                "ON DUPLICATE KEY UPDATE "
                "min=VALUES(min),max=VALUES(max),unite=VALUES(unite),type=VALUES(type)",
                NOM_TABLE_MNEMO_AI, mnemo_full->mnemo_base.id, 
                mnemo_full->mnemo_ai.min, mnemo_full->mnemo_ai.max, unite, mnemo_full->mnemo_ai.type
              );

    g_free(unite);
    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Charger_analogInput: Chargement des infos sur les Entrees ANA                                          */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_analogInput ( void )
  { struct DB *db;

    if (!Recuperer_analogInputDB( &db ))
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Charger_analogInput: DB Connexion Failed" );
       return;
     }                                                                         /* Si pas d'enregistrement */

    for( ; ; )
     { struct CMD_TYPE_MNEMO_AI *entree;
       entree = Recuperer_analogInputDB_suite( &db );
       if (!entree) break;

       if (entree->num < NBR_ENTRE_ANA)
        { memcpy( &Partage->ea[entree->num].confDB, entree, sizeof(struct CMD_TYPE_MNEMO_AI) );
          Partage->ea[entree->num].last_arch = 0; /* Mise à zero du champ de la derniere date d'archivage */
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                   "Charger_analogInput: entree->num (%d) out of range (max=%d)",
                    entree->num, NBR_ENTRE_ANA );
        }
       g_free(entree);
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_analogInput: DB reloaded" );
  }
/*--------------------------------------------------------------------------------------------------------*/
