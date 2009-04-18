/**********************************************************************************************************/
/* Watchdogd/Db/CptHoraire/Compteur_horaire.c        Déclaration des fonctions pour la gestion des cpt_h  */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 14 fév 2006 15:03:51 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Compteur_horaire.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
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
 #include "Cst_dls.h"
 #include "Cpth_DB.h"
/**********************************************************************************************************/
/* Charger_cpth: Chargement des infos sur les compteurs horaires depuis la DB                             */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_cpth ( void )
  { struct DB *db;
    gint i;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database,        /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info( Config.log, DEBUG_INFO, "Charger_cpth: Connexion DB failed" );
       return;
     }                                                                           /* Si pas de histos (??) */

    for (i = 0; i<NBR_COMPTEUR_H; i++)
     { struct CPTH_DB *cpth;
       cpth = Rechercher_cpthDB( Config.log, db, i );
       if (cpth)
        { memcpy ( &Partage->ch[cpth->id].cpthdb, cpth, sizeof(struct CPTH_DB) );
          g_free(cpth);
        }
       else
        { Partage->ch[cpth->id].cpthdb.valeur = 0;
        }
     }
    Libere_DB_SQL( Config.log, &db );
  }
/**********************************************************************************************************/
/* Ajouter_cpthDB: Ajout ou edition d'un entreeANA                                                        */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure cpth                          */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Updater_cpthDB ( struct LOG *log, struct DB *db, struct CPTH_DB *cpth )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET val=%d WHERE id=%d;", NOM_TABLE_CPTH, cpth->valeur, cpth->id );

    Lancer_requete_SQL ( log, db, requete );
  }

/**********************************************************************************************************/
/* Recuperer_liste_id_cpthDB: Recupération de la liste des ids des entreeANAs                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CPTH_DB *Rechercher_cpthDB ( struct LOG *log, struct DB *db, guint id )
  { gchar valeur[20];
    struct CPTH_DB *cpth;
    gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT val"
                " FROM %s WHERE id=%d", NOM_TABLE_CPTH, id );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_cpthDB: Cpth non trouvé dans la BDD", id );
       return(NULL);
     }

    cpth = (struct CPTH_DB *)g_malloc0( sizeof(struct CPTH_DB) );
    if (!cpth) Info( log, DEBUG_MEM, "Rechercher_cpthDB: Erreur allocation mémoire" );
    else
     { cpth->id     = id;
       cpth->valeur = atoi(db->row[0]);
     }
    Liberer_resultat_SQL ( log, db );
    return(cpth);
  }
/*--------------------------------------------------------------------------------------------------------*/
