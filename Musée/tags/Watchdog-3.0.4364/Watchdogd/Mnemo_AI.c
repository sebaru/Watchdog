/******************************************************************************************************************************/
/* Watchdogd/Mnemo_AI.c        Déclaration des fonctions pour la gestion des Analog Input                                     */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 18 avr 2009 13:30:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_AI.c
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
/* Mnemo_auto_create_AI_by_tech_id: Ajoute un mnemonique dans la base via le tech_id                                          */
/* Entrée: le tech_id, l'acronyme, le libelle et l'unite                                                                      */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_AI ( gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src )
  { gchar *acro, *libelle, *unite;
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

    unite      = Normaliser_chaine ( unite_src );                                            /* Formatage correct des chaines */
    if ( !unite )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation unite impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       g_free(libelle);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO mnemos_AI SET tech_id='%s',acronyme='%s',libelle='%s', unite='%s' "
                " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle),unite=VALUES(unite)",
                tech_id, acro, libelle, unite );
    g_free(unite);
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
/* Rechercher_mnemo_aiDB: Recupération de la conf de l'entrée analogique en parametre                                         */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant l'entrée analogique                                                                       */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_AI *Rechercher_mnemo_aiDB ( guint id )
  { struct CMD_TYPE_MNEMO_AI *mnemo_ai;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_mnemo_aiDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT %s.min,%s.max,%s.type,%s.unite"
                " FROM %s"
                " INNER JOIN %s ON id_mnemo = id"
                " WHERE id_mnemo=%d LIMIT 1",
                NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI,
                NOM_TABLE_MNEMO_AI,                                                                                   /* FROM */
                NOM_TABLE_MNEMO,                                                                                /* INNER JOIN */
                id                                                                                                   /* WHERE */
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

    mnemo_ai = (struct CMD_TYPE_MNEMO_AI *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_AI) );
    if (!mnemo_ai) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                             "Rechercher_mnemo_aiDB: Erreur allocation mémoire" );
    else
     { mnemo_ai->min      = atof(db->row[0]);
       mnemo_ai->max      = atof(db->row[1]);
       mnemo_ai->type     = atoi(db->row[2]);
       g_snprintf( mnemo_ai->unite, sizeof(mnemo_ai->unite), "%s", db->row[3] );
     }
    Libere_DB_SQL( &db );
    return(mnemo_ai);
  }
/******************************************************************************************************************************/
/* Rechercher_AI: Recupération des champs de base de données pour le AI tech_id:acro en parametre                             */
/* Entrée: le tech_id et l'acronyme a récupérer                                                                               */
/* Sortie: la struct DB                                                                                                       */
/******************************************************************************************************************************/
 struct DB *Rechercher_AI ( gchar *tech_id, gchar *acronyme )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT ai.unite"
                " FROM mnemos_AI as ai"
                " WHERE ai.tech_id='%s' AND ai.acronyme='%s' LIMIT 1",
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
/* Rechercher_AI_by_text: Recupération des champs de base de données pour le AI par map                                       */
/* Entrée: le tech_id et l'acronyme a récupérer                                                                               */
/* Sortie: la struct DB                                                                                                       */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemos_AI_by_text ( struct DB **db_retour, gchar *thread, gchar *text )
  { gchar requete[1024];
    gchar *commande;
    gboolean retour;
    struct DB *db;

    commande = Normaliser_chaine ( text );
    if (!commande)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible commande", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),
               "SELECT m.tech_id, m.acronyme, m.map_text, m.libelle "
               "FROM mnemos_AI as m "
               " WHERE (m.map_host='*' OR m.map_host LIKE '%s') AND (m.map_thread='*' OR m.map_thread LIKE '%s')"
               " AND m.map_text LIKE '%s'", g_get_host_name(), thread, commande );

    g_free(commande);

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
/* Rechercher_AI_by_map_snips: Recupere l'AI lié au parametre snips                                                           */
/* Entrée: le map_snips a rechercher                                                                                          */
/* Sortie: la struct DB                                                                                                       */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemos_AI_by_map_question_vocale ( struct DB **db_retour, gchar *map_snips )
  { gchar requete[1024];
    gchar *commande;
    gboolean retour;
    struct DB *db;

    commande = Normaliser_chaine ( map_snips );
    if (!commande)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible commande", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),
               "SELECT m.tech_id, m.acronyme, m.libelle, m.map_question_vocale, m.map_reponse_vocale "
               "FROM mnemos_AI as m"
               " WHERE m.map_question_vocale LIKE '%s'", commande );
    g_free(commande);

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
/* Recuperer_mnemo_base_DB_suite: Fonction itérative de récupération des mnémoniques de base                                  */
/* Entrée: un pointeur sur la connexion de baase de données                                                                   */
/* Sortie: une structure nouvellement allouée                                                                                 */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemos_AI_suite( struct DB **db_orig )
  { struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(FALSE);
     }

    return(TRUE);                                                                                    /* Résultat dans db->row */
  }
/******************************************************************************************************************************/
/* Charger_conf_ai: Recupération de la conf de l'entrée analogique en parametre                                               */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant l'entrée analogique                                                                       */
/******************************************************************************************************************************/
 void Charger_conf_AI ( struct DLS_AI *ai )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT a.min,a.max,a.type,a.unite"
                " FROM mnemos_AI as a"
                " WHERE a.tech_id='%s' AND a.acronyme='%s' LIMIT 1",
                ai->tech_id, ai->acronyme
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Libere_DB_SQL( &db );
       return;
     }

    ai->min      = atof(db->row[0]);
    ai->max      = atof(db->row[1]);
    ai->type     = atoi(db->row[2]);
    g_snprintf( ai->unite, sizeof(ai->unite), "%s", db->row[3] );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: AI '%s:%s' loaded", __func__, ai->tech_id, ai->acronyme );
    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Modifier_analogInputDB: Modification d'un entreeANA Watchdog                                                               */
/* Entrées: une structure hébergeant l'entrée analogique a modifier                                                           */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Modifier_mnemo_aiDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;
    gchar *unite;

    unite = Normaliser_chaine ( mnemo_full->mnemo_ai.unite );                                /* Formatage correct des chaines */
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

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s (id_mnemo,min,max,unite,type) VALUES "
                "('%d','%f','%f','%s','%d') "
                "ON DUPLICATE KEY UPDATE "
                "min=VALUES(min),max=VALUES(max),unite=VALUES(unite),type=VALUES(type)",
                NOM_TABLE_MNEMO_AI, mnemo_full->mnemo_base.id,
                mnemo_full->mnemo_ai.min, mnemo_full->mnemo_ai.max, unite, mnemo_full->mnemo_ai.type
              );

    g_free(unite);
    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Charger_analogInput: Chargement des infos sur les Entrees ANA                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_analogInput ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Charger_analogInput: Connexion DB impossible" );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT num,min,max,%s.type,%s.unite"
                " FROM %s"
                " INNER JOIN %s ON id_mnemo = id ORDER BY num",
                NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI,
                NOM_TABLE_MNEMO,                                                                                      /* FROM */
                NOM_TABLE_MNEMO_AI                                                                              /* INNER JOIN */
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while ( Recuperer_ligne_SQL(db) )                                                      /* Chargement d'une ligne resultat */
     { gint num;
       num = atoi( db->row[0] );
       if (num < NBR_ENTRE_ANA)
        { Partage->ea[num].confDB.min  = atof(db->row[1]);
          Partage->ea[num].confDB.max      = atof(db->row[2]);
          Partage->ea[num].confDB.type     = atoi(db->row[3]);
          g_snprintf( Partage->ea[num].confDB.unite, sizeof(Partage->ea[num].confDB.unite), "%s", db->row[4] );
          Partage->ea[num].last_arch = 0;                             /* Mise à zero du champ de la derniere date d'archivage */
          Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                   "Charger_analogInput: Chargement config EA[%04d]=%d", num );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
			       "Charger_analogInput: num (%d) out of range (max=%d)", num, NBR_ENTRE_ANA ); }
     }
    Libere_DB_SQL (&db);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_analogInput: DB reloaded" );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
