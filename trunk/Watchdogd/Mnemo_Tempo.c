/******************************************************************************************************************************/
/* Watchdogd/Mnemo_Tempo.c              Déclaration des fonctions pour la gestion des tempo.c                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         sam. 09 mars 2013 11:47:18 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_Tempo.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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
/* Ajouter_Modifier_mnemo_baseDB: Ajout ou modifie le mnemo en parametre                                                      */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_TEMPO ( gint dls_id, gchar *acronyme, gchar *libelle_src )
  { gchar *acro, *libelle;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;

/******************************************** Préparation de la base du mnemo *************************************************/
    acro       = Normaliser_chaine ( acronyme );                                             /* Formatage correct des chaines */
    if ( !acro )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation acro impossible. Mnemo NOT added nor modified.", __func__ );
       return(FALSE);
     }

    libelle    = Normaliser_chaine ( libelle_src );                                          /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation libelle impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO mnemos_Tempo SET dls_id='%d',acronyme='%s',libelle='%s' "
                " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                dls_id, acro, libelle );
    g_free(libelle);
    g_free(acro);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }
    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return (retour);
  }
/******************************************************************************************************************************/
/* Rechercher_AI: Recupération des champs de base de données pour le AI tech_id:acro en parametre                             */
/* Entrée: le tech_id et l'acronyme a récupérer                                                                               */
/* Sortie: la struct DB                                                                                                       */
/******************************************************************************************************************************/
 struct DB *Rechercher_Tempo ( gchar *tech_id, gchar *acronyme )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT ai.libelle"
                " FROM mnemos_Tempo"
                " INNER JOIN dls as d ON ai.dls_id = d.id"
                " WHERE d.tech_id='%s' AND ai.acronyme='%s' LIMIT 1",
                tech_id, acronyme
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return(NULL);
     }
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Libere_DB_SQL( &db );
       return(NULL);
     }
    return(db);
  }
/******************************************************************************************************************************/
/* Rechercher_tempoDB: Recupération du tempo dont l'id est en parametre                                                       */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant la temporisation                                                                          */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_TEMPO *Rechercher_mnemo_tempoDB ( guint id )
  { struct CMD_TYPE_MNEMO_TEMPO *tempo;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_mnemo_tempoDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT %s.delai_on,%s.min_on,%s.max_on,%s.delai_off"
                " FROM %s"
                " INNER JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.id_mnemo=%d",
                NOM_TABLE_MNEMO_TEMPO,
                NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO,
                NOM_TABLE_MNEMO,                                                                                      /* FROM */
                NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO,                                  /* INNER JOIN */
                NOM_TABLE_MNEMO_TEMPO, id                                                                            /* WHERE */
              );

   if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return(NULL);
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    tempo = (struct CMD_TYPE_MNEMO_TEMPO *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_TEMPO) );
    if (!tempo) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                             "Recuperer_tempoDB_suite: Erreur allocation mémoire" );
    else
     { tempo->delai_on  = atoi(db->row[0]);
       tempo->min_on    = atoi(db->row[1]);
       tempo->max_on    = atoi(db->row[2]);
       tempo->delai_off = atoi(db->row[3]);
     }
    Libere_DB_SQL( &db );
    return(tempo);
  }
/******************************************************************************************************************************/
/* Modifier_tempoDB: Modification d'une tempo Watchdog                                                                        */
/* Entrées: une structure hébergeant la temporisation a modifier                                                              */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Modifier_mnemo_tempoDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_tempoDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */

                "INSERT INTO %s (id_mnemo,delai_on,min_on,max_on,delai_off) VALUES "
                "('%d','%d','%d','%d','%d') "
                "ON DUPLICATE KEY UPDATE "
                "delai_on=VALUES(delai_on), min_on=VALUES(min_on), "
                "delai_off=VALUES(delai_off), max_on=VALUES(max_on) ",
                NOM_TABLE_MNEMO_TEMPO, mnemo_full->mnemo_base.id,
                mnemo_full->mnemo_tempo.delai_on,  mnemo_full->mnemo_tempo.min_on,
                mnemo_full->mnemo_tempo.max_on, mnemo_full->mnemo_tempo.delai_off
              );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Charger_tempo: Chargement des infos sur les Temporisations                                                                 */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_tempo ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Charger_tempo: Connexion DB impossible" );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT num, %s.delai_on,%s.min_on,%s.max_on,%s.delai_off"
                " FROM %s"
                " INNER JOIN %s ON %s.id_mnemo = %s.id ORDER BY num",
                NOM_TABLE_MNEMO_TEMPO,
                NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO,
                NOM_TABLE_MNEMO,                                                                                      /* FROM */
                NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO_TEMPO, NOM_TABLE_MNEMO                                   /* INNER JOIN */
              );

   if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while ( Recuperer_ligne_SQL(db) )                                                      /* Chargement d'une ligne resultat */
     { gint num;
       num = atoi( db->row[0] );
       if (num < NBR_TEMPO)
        { Partage->Tempo_R[num].confDB.delai_on  = atoi(db->row[1]);
          Partage->Tempo_R[num].confDB.min_on    = atoi(db->row[2]);
          Partage->Tempo_R[num].confDB.max_on    = atoi(db->row[3]);
          Partage->Tempo_R[num].confDB.delai_off = atoi(db->row[4]);
          Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                   "Charger_tempo: Chargement config T[%04d]", num );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
			       "Charger_tempo: num (%d) out of range (max=%d)", num, NBR_TEMPO ); }
      }
    Libere_DB_SQL (&db);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_tempo: DB reloaded" );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
