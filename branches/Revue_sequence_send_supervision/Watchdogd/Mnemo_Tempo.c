/**********************************************************************************************************/
/* Watchdogd/Tempo.c              Déclaration des fonctions pour la gestion des tempo.c                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                     sam. 09 mars 2013 11:47:18 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Tempo.c
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
/* Recuperer_digitalInputDB: Recupération de la liste des ETOR                                            */
/* Entrée: un pointeur vers une nouvelle DB                                                               */
/* Sortie: TRUE si OK                                                                                     */
/**********************************************************************************************************/
 static gboolean Recuperer_tempoDB ( struct DB **db_retour )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.num,%s.delai_on,%s.min_on,%s.max_on,%s.delai_off"
                " FROM %s"
                " INNER JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.type=%d ORDER BY %s.num",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO_TEMPO,
                NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO,
                NOM_TABLE_MNEMO,                                                                  /* FROM */
                NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO,              /* INNER JOIN */
                NOM_TABLE_MNEMO, MNEMO_TEMPO,                                                    /* WHERE */
                NOM_TABLE_MNEMO                                                               /* Order by */
              );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_tempoDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_tempoDB_suite: Recupération de la liste des informations sur les temposids des               */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct CMD_TYPE_MNEMO_TEMPO *Recuperer_tempoDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_MNEMO_TEMPO *tempo;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    tempo = (struct CMD_TYPE_MNEMO_TEMPO *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_TEMPO) );
    if (!tempo) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                             "Recuperer_tempoDB_suite: Erreur allocation mémoire" );
    else
     { tempo->num       = atoi(db->row[0]);
       tempo->delai_on  = atoi(db->row[1]);
       tempo->min_on    = atoi(db->row[2]);
       tempo->max_on    = atoi(db->row[3]);
       tempo->delai_off = atoi(db->row[4]);
     }
    return(tempo);
  }
/**********************************************************************************************************/
/* Rechercher_tempoDB: Recupération du tempo dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMO_TEMPO *Rechercher_mnemo_tempoDB ( guint id )
  { struct CMD_TYPE_MNEMO_TEMPO *tempo;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_tempoDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.num,%s.delai_on,%s.min_on,%s.max_on,%s.delai_off"
                " FROM %s"
                " INNER JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.id_mnemo=%d",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO_TEMPO,
                NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO,
                NOM_TABLE_MNEMO,                                                                  /* FROM */
                NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO,              /* INNER JOIN */
                NOM_TABLE_MNEMO_TEMPO, id                                                        /* WHERE */
              );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    tempo = Recuperer_tempoDB_suite( &db );
    if (tempo) Libere_DB_SQL ( &db );
    return(tempo);
  }
/**********************************************************************************************************/
/* Modifier_tempoDB: Modification d'une tempo Watchdog                                                    */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_mnemo_tempoDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_tempoDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */

                "INSERT INTO %s (id_mnemo,delai_on,min_on,max_on,delai_off) VALUES "
                "('%d','%d','%d','%d','%d') "
                "ON DUPLICATE KEY UPDATE "
                "delai_on=VALUES(delai_on), min_on=VALUES(min_on), "
                "delai_off=VALUES(delai_off), max_on=VALUES(max_on) ",
                NOM_TABLE_MNEMO_TEMPO, mnemo_full->mnemo_base.id,
                mnemo_full->mnemo_tempo.delai_on,  mnemo_full->mnemo_tempo.min_on, 
                mnemo_full->mnemo_tempo.delai_off, mnemo_full->mnemo_tempo.max_on
              );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Charger_tempo: Chargement des infos sur les Temporisations                                             */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_tempo ( void )
  { struct DB *db;

    if (!Recuperer_tempoDB( &db )) return;

    for( ; ; )
     { struct CMD_TYPE_MNEMO_TEMPO *tempo;
       tempo = Recuperer_tempoDB_suite( &db );
       if (!tempo) break;

       if (tempo->num < NBR_TEMPO)
        { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                   "Charger_tempo: Setting T(%03d) -> delai_on=%d, min_on=%d, max_on=%d, delai_off=%d",
                    tempo->num, tempo->delai_on, tempo->min_on, tempo->max_on, tempo->delai_off );
          memcpy( &Partage->Tempo_R[tempo->num].confDB, tempo, sizeof(struct CMD_TYPE_MNEMO_TEMPO) );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                   "Charger_tempo: tempo->num (%d) out of range (max=%d)", tempo->num, NBR_TEMPO );
        }
       g_free(tempo);
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_tempo: DB reloaded" );
  }
/*--------------------------------------------------------------------------------------------------------*/
