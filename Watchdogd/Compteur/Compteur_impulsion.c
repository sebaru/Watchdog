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
     { struct CMD_TYPE_OPTION_COMPTEUR_IMP *cpt_imp;
       cpt_imp = Rechercher_cpt_impDB_par_num( Config.log, db, i );
       if (cpt_imp)
        { memcpy ( &Partage->ci[i].cpt_impdb, cpt_imp, sizeof(struct CMD_TYPE_OPTION_COMPTEUR_IMP) );
          g_free(cpt_imp);
        }
       else
        { Partage->ci[i].cpt_impdb.valeur   = 0;
          Partage->ci[i].cpt_impdb.id_mnemo = -1;
          Partage->ci[i].cpt_impdb.unite    = 0;
          Partage->ci[i].cpt_impdb.num      = i;
        }
     }
    Libere_DB_SQL( Config.log, &db );
  }
/**********************************************************************************************************/
/* Ajouter_cpt_impDB: Ajout ou edition d'un entreeANA                                                     */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure cpt_imp                       */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Updater_cpt_impDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_OPTION_COMPTEUR_IMP *cpt_imp )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET val='%d' WHERE id_mnemo='%d';", NOM_TABLE_CPT_IMP,
                cpt_imp->valeur, cpt_imp->id_mnemo );

    Lancer_requete_SQL ( log, db, requete );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_cpt_impDB: Recupération de la liste des ids des entreeANAs                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_OPTION_COMPTEUR_IMP *Rechercher_cpt_impDB_par_num ( struct LOG *log, struct DB *db, guint num )
  { struct CMD_TYPE_OPTION_COMPTEUR_IMP *cpt_imp;
    gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id_mnemo,val,unite,num"
                " FROM %s,%s WHERE %s.id=%s.id_mnemo AND %s.num=%d",
                NOM_TABLE_CPT_IMP, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_MNEMO, NOM_TABLE_CPT_IMP, NOM_TABLE_MNEMO, num /* WHERE */
              );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_cpt_impDB_par_num: Cpt_imp non trouvé dans la BDD", num );
       return(NULL);
     }

    cpt_imp = (struct CMD_TYPE_OPTION_COMPTEUR_IMP *)g_malloc0( sizeof(struct CMD_TYPE_OPTION_COMPTEUR_IMP) );
    if (!cpt_imp) Info( log, DEBUG_INFO, "Rechercher_cpt_impDB_par_num: Erreur allocation mémoire" );
    else
     { cpt_imp->id_mnemo = atoi(db->row[0]);
       cpt_imp->valeur   = atoi(db->row[1]);
       cpt_imp->unite    = atoi(db->row[2]);
       cpt_imp->num      = atoi(db->row[3]);
     }
    Liberer_resultat_SQL ( log, db );
    return(cpt_imp);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_cpt_impDB: Recupération de la liste des ids des entreeANAs                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_OPTION_COMPTEUR_IMP *Rechercher_cpt_impDB_par_id ( struct LOG *log, struct DB *db, guint id )
  { struct CMD_TYPE_OPTION_COMPTEUR_IMP *cpt_imp;
    gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id_mnemo,val,unite,num"
                " FROM %s,%s WHERE %s.id=%s.id_mnemo AND %s.id=%d",
                NOM_TABLE_CPT_IMP, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_MNEMO, NOM_TABLE_CPT_IMP, NOM_TABLE_CPT_IMP, NOM_TABLE_MNEMO, id /* WHERE */
              );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_cpt_impDB_par_id: Cpt_imp non trouvé dans la BDD", id );
       return(NULL);
     }

    cpt_imp = (struct CMD_TYPE_OPTION_COMPTEUR_IMP *)g_malloc0( sizeof(struct CMD_TYPE_OPTION_COMPTEUR_IMP) );
    if (!cpt_imp) Info( log, DEBUG_INFO, "Rechercher_cpt_impDB_par_id: Erreur allocation mémoire" );
    else
     { cpt_imp->id_mnemo = atoi(db->row[0]);
       cpt_imp->valeur   = atoi(db->row[1]);
       cpt_imp->unite    = atoi(db->row[2]);
       cpt_imp->num      = atoi(db->row[3]);
     }
    Liberer_resultat_SQL ( log, db );
    return(cpt_imp);
  }
/**********************************************************************************************************/
/* Modifier_cpt_impDB: Modification d'un compteur d'impulsion                                             */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_cpt_impDB( struct LOG *log, struct DB *db, struct CMD_TYPE_OPTION_COMPTEUR_IMP *cpt_imp )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "unite=%d WHERE id_mnemo=%d",
                NOM_TABLE_CPT_IMP, cpt_imp->unite, cpt_imp->id_mnemo );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
