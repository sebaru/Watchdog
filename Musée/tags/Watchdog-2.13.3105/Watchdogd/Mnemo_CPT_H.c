/******************************************************************************************************************************/
/* Watchdogd/Mnemo_CPT_H.c      Déclaration des fonctions pour la gestion des cpt_h                                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           mar 14 fév 2006 15:03:51 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_CPT_H.c
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
 #include "Erreur.h"
/******************************************************************************************************************************/
/* Rechercher_cpthDB: Recupération du compteur horaire dont l'id est en parametre                                             */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant le compteur                                                                               */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_CPT_H *Rechercher_MNEMO_CPTHDB ( guint id )
  { struct CMD_TYPE_MNEMO_CPT_H *cpth;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_MNEMO_CPTHDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT %s.valeur"
                " FROM %s"
                " INNER JOIN %s ON id_mnemo = id"
                " WHERE id_mnemo=%d LIMIT 1",
                NOM_TABLE_MNEMO_CPTH,
                NOM_TABLE_MNEMO,                                                                                      /* FROM */
                NOM_TABLE_MNEMO_CPTH,                                                                           /* INNER JOIN */
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

    cpth = (struct CMD_TYPE_MNEMO_CPT_H *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_CPT_H) );
    if (!cpth) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                        "Recuperer_cpthDB_suite: Erreur allocation mémoire" );
    else
     { cpth->valeur = atoi(db->row[0]);
     }
    Libere_DB_SQL( &db );
    return(cpth);
  }
/******************************************************************************************************************************/
/* Modifier_cpthDB: Modification d'un compteur horaire Watchdog                                                               */
/* Entrées: une structure hébergeant le compteur horaire a modifier                                                           */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Modifier_mnemo_cpthDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_cpthDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */

                "INSERT INTO %s (id_mnemo) VALUES "
                "('%d') "
                "ON DUPLICATE KEY UPDATE "
                "id_mnemo=id_mnemo",
                NOM_TABLE_MNEMO_CPTH, mnemo_full->mnemo_base.id
              );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Charger_cpth: Chargement des infos sur les compteurs horaires depuis la DB                                                 */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_cpth ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, FALSE, LOG_ERR, "Charger_cpth: Connexion DB failed" );
       return;
     }                                                                                               /* Si pas de histos (??) */

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT num, %s.valeur"
                " FROM %s"
                " INNER JOIN %s ON id_mnemo = id ORDER BY num",
                NOM_TABLE_MNEMO_CPTH,
                NOM_TABLE_MNEMO,                                                                                      /* FROM */
                NOM_TABLE_MNEMO_CPTH                                                                            /* INNER JOIN */
              );


   if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while ( Recuperer_ligne_SQL(db) )                                                      /* Chargement d'une ligne resultat */
     { gint num;
       num = atoi( db->row[0] );
       if (num < NBR_COMPTEUR_H)
        { Partage->ch[num].confDB.valeur = atoi(db->row[1]);
          Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                   "Charger_cpth: Chargement config CH[%04d]", num );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
			       "Charger_cpth: num (%d) out of range (max=%d)", num, NBR_COMPTEUR_H ); }
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_cpth: DB reloaded" );
    Libere_DB_SQL (&db);
  }
/******************************************************************************************************************************/
/* Updater_cpthDB : Met à jour l'ensemble des CompteurHoraire dans la base de données                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_cpthDB ( void )
  { struct CMD_TYPE_MNEMO_CPT_H *cpth;
    gchar requete[200];
    struct DB *db;
    gint cpt;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    for( cpt=0; cpt<NBR_COMPTEUR_H; cpt++)
     { cpth = &Partage->ch[cpt].confDB;
       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE %s JOIN %s ON id_mnemo=id SET valeur='%d' WHERE num='%d';",
                   NOM_TABLE_MNEMO_CPTH, NOM_TABLE_MNEMO,
                   cpth->valeur, cpt );
       Lancer_requete_SQL ( db, requete );
     }
    Libere_DB_SQL( &db );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: CptH updated", __func__ );
  }
/*--------------------------------------------------------------------------------------------------------*/
