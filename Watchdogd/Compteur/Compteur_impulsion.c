/**********************************************************************************************************/
/* Watchdogd/Compteur/Compteur_impulsion.c      Déclaration des fonctions pour la gestion des cpt_imp     */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 07 déc. 2010 17:26:52 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Compteur_impulsion.c
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
/* Charger_cpt_imp: Chargement des infos sur les compteurs impulsions depuis la DB                        */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_cpt_imp ( void )
  { struct DB *db;
    gint i;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info( Config.log, DEBUG_INFO, "Charger_cpt_imp: Connexion DB failed" );
       return;
     }                                                                           /* Si pas de histos (??) */

    for (i = 0; i<NBR_COMPTEUR_IMP; i++)
     { struct CPT_IMP_DB *cpt_imp;
       cpt_imp = Rechercher_cpt_impDB( Config.log, db, i );
       if (cpt_imp)
        { memcpy ( &Partage->ci[cpt_imp->id].cpt_impdb, cpt_imp, sizeof(struct CPT_IMP_DB) );
          g_free(cpt_imp);
        }
       else
        { Partage->ci[i].cpt_impdb.valeur = 0;
          Partage->ci[i].cpt_impdb.id     = i;
        }
     }
    Libere_DB_SQL( Config.log, &db );
  }
/**********************************************************************************************************/
/* Ajouter_cpt_impDB: Ajout ou edition d'un entreeANA                                                        */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure cpt_imp                          */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Updater_cpt_impDB ( struct LOG *log, struct DB *db, struct CPT_IMP_DB *cpt_imp )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET val='%d',unite='%d' WHERE id='%d';", NOM_TABLE_CPT_IMP,
                cpt_imp->valeur, cpt_imp->unite, cpt_imp->id );

    Lancer_requete_SQL ( log, db, requete );
  }

/**********************************************************************************************************/
/* Recuperer_liste_id_cpt_impDB: Recupération de la liste des ids des entreeANAs                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CPT_IMP_DB *Rechercher_cpt_impDB ( struct LOG *log, struct DB *db, guint id )
  { struct CPT_IMP_DB *cpt_imp;
    gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT val,unite"
                " FROM %s WHERE id='%d'", NOM_TABLE_CPT_IMP, id );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_cpt_impDB: Cpt_imp non trouvé dans la BDD", id );
       return(NULL);
     }

    cpt_imp = (struct CPT_IMP_DB *)g_malloc0( sizeof(struct CPT_IMP_DB) );
    if (!cpt_imp) Info( log, DEBUG_INFO, "Rechercher_cpt_impDB: Erreur allocation mémoire" );
    else
     { cpt_imp->id     = id;
       cpt_imp->valeur = atoi(db->row[0]);
       cpt_imp->unite  = atoi(db->row[1]);
     }
    Liberer_resultat_SQL ( log, db );
    return(cpt_imp);
  }
/*--------------------------------------------------------------------------------------------------------*/
