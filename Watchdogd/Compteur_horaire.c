/**********************************************************************************************************/
/* Watchdogd/CptHoraire/Compteur_horaire.c      Déclaration des fonctions pour la gestion des cpt_h       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 14 fév 2006 15:03:51 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Compteur_horaire.c
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
 #include "Cpth_DB.h"
/**********************************************************************************************************/
/* Recuperer_cpthDB: Recupération de la liste des compteurs horaires                                      */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_cpthDB ( struct LOG *log, struct DB *db )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id_mnemo,val,num"
                " FROM %s,%s WHERE %s.id=%s.id_mnemo AND %s.type=%d ORDER BY %s.num",
                NOM_TABLE_CPTH, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_MNEMO, NOM_TABLE_CPTH, /* WHERE */
                NOM_TABLE_MNEMO, MNEMO_CPTH,
                NOM_TABLE_MNEMO /* Order by */
              );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_cpthDB_suite: Envoi du prochain enregistrement des cpth dans la liste                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CPTH_DB *Recuperer_cpthDB_suite( struct LOG *log, struct DB *db )
  { struct CPTH_DB *cpth;

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       return(NULL);
     }

    cpth = (struct CPTH_DB *)g_try_malloc0( sizeof(struct CPTH_DB) );
    if (!cpth) Info_new( Config.log, FALSE, LOG_WARNING, "Rechercher_cpthDB_suite: Erreur allocation mémoire" );
    else
     { cpth->id_mnemo = atoi(db->row[0]);
       cpth->valeur   = atoi(db->row[1]);
       cpth->num      = atoi(db->row[2]);
     }
    return(cpth);
  }
/**********************************************************************************************************/
/* Charger_cpth: Chargement des infos sur les compteurs horaires depuis la DB                             */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_cpth ( void )
  { struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, FALSE, LOG_ERR, "Charger_cpth: Connexion DB failed" );
       return;
     }                                                                           /* Si pas de histos (??) */

    if (!Recuperer_cpthDB( Config.log, db ))
     { Libere_DB_SQL( &db );
       return;
     }                                                                         /* Si pas d'enregistrement */

    for( ; ; )
     { struct CPTH_DB *cpth;
       cpth = Recuperer_cpthDB_suite( Config.log, db );
       if (!cpth)
        { Libere_DB_SQL( &db );
          return;
        }
       if (cpth->num < NBR_COMPTEUR_H)
        { memcpy ( &Partage->ch[cpth->num].cpthdb, cpth, sizeof(struct CPTH_DB) ); }
       else
        { Info_new( Config.log, FALSE, LOG_WARNING, "Charger_cpth: cpth->num (%d) out of range", cpth->num ); }
       g_free(cpth);
     }
    Libere_DB_SQL( &db );
  }
/**********************************************************************************************************/
/* Updater_cpthDB : Met à jour l'ensemble des CompteurHoraire dans la base de données                     */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure cpth                          */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Updater_cpthDB ( void )
  { struct CPTH_DB *cpth;
    gchar requete[200];
    struct DB *db;
    gint cpt;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Updater_cpthDB: Connexion DB impossible" );
       return;
     }

    for( cpt=0; cpt<NBR_COMPTEUR_H; cpt++)
     { cpth = &Partage->ch[cpt].cpthdb;
       g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                   "UPDATE %s SET val=%d WHERE id_mnemo=%d;", NOM_TABLE_CPTH, cpth->valeur, cpth->id_mnemo );
       Lancer_requete_SQL ( db, requete );
     }
    Libere_DB_SQL( &db );
  }
/*--------------------------------------------------------------------------------------------------------*/
