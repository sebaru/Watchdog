/**********************************************************************************************************/
/* Watchdogd/Mnemo_CPT_IMP.c      Déclaration des fonctions pour la gestion des cpt_imp                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 07 déc. 2010 17:26:52 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Mnemo_CPT_IMP.c
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
/* Ajouter_cpt_impDB: Ajout ou edition d'un entreeANA                                                     */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure cpt_imp                       */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Updater_cpt_impDB ( void )
  { struct CMD_TYPE_MNEMO_CPT_IMP *cpt_imp;
    gchar requete[200];
    struct DB *db;
    gint cpt;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Updater_cpt_impDB: Connexion DB impossible" );
       return;
     }

    for( cpt=0; cpt<NBR_COMPTEUR_IMP; cpt++)
     { cpt_imp = &Partage->ci[cpt].confDB;
       g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                   "UPDATE %s JOIN %s ON id_mnemo=id SET val='%f' WHERE num='%d';",
                   NOM_TABLE_MNEMO_CPTIMP, NOM_TABLE_MNEMO,
                   cpt_imp->valeur, cpt_imp->num );
       Lancer_requete_SQL ( db, requete );
     }
    Libere_DB_SQL( &db );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_mnemo_cptimpDB ( struct DB **db_retour )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_mnemo_cptimpDB: Connexion DB impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT num,val,type_ci,multi,unite_string"
                " FROM %s"
                " INNER JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.type=%d ORDER BY %s.num",
                NOM_TABLE_MNEMO,                                                                  /* From */
                NOM_TABLE_MNEMO_CPTIMP, NOM_TABLE_MNEMO_CPTIMP, NOM_TABLE_MNEMO,            /* INNER JOIN */
                NOM_TABLE_MNEMO, MNEMO_CPT_IMP,                                                  /* WHERE */
                NOM_TABLE_MNEMO                                                               /* Order by */
              );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct CMD_TYPE_MNEMO_CPT_IMP *Recuperer_mnemo_cptimpDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_MNEMO_CPT_IMP *cpt_imp;

    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    cpt_imp = (struct CMD_TYPE_MNEMO_CPT_IMP *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_CPT_IMP) );
    if (!cpt_imp) Info_new( Config.log, FALSE, LOG_WARNING, "Recuperer_mnemo_cptimpDB_suite: Erreur allocation mémoire" );
    else
     { cpt_imp->num      = atoi(db->row[0]);
       cpt_imp->valeur   = atof(db->row[1]);
       cpt_imp->type     = atoi(db->row[2]);
       cpt_imp->multi    = atof(db->row[3]);
       g_snprintf( cpt_imp->unite, sizeof(cpt_imp->unite), "%s", db->row[4] );
     }
    return(cpt_imp);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_cpt_impDB: Recupération de la liste des ids des entreeANAs                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMO_CPT_IMP *Rechercher_mnemo_cptimpDB ( guint id )
  { struct CMD_TYPE_MNEMO_CPT_IMP *cpt_imp;
    gchar requete[200];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_mnemo_cptimpDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT num,val,type_ci,multi,unite_string"
                " FROM %s"
                " INNER JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.id=%d",
                NOM_TABLE_MNEMO,                                                                  /* From */
                NOM_TABLE_MNEMO_CPTIMP, NOM_TABLE_MNEMO_CPTIMP, NOM_TABLE_MNEMO,            /* INNER JOIN */
                NOM_TABLE_MNEMO, id
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                       /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return(NULL);
     }

    cpt_imp = Recuperer_mnemo_cptimpDB_suite( &db );
    if (cpt_imp) Libere_DB_SQL( &db );
    return(cpt_imp);
  }
/**********************************************************************************************************/
/* Modifier_cpt_impDB: Modification d'un compteur d'impulsion                                             */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_mnemo_cptimpDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024], *unite;
    gboolean retour;
    struct DB *db;

    unite = Normaliser_chaine ( mnemo_full->mnemo_cptimp.unite );        /* Formatage correct des chaines */
    if (!unite)
     { Info_new( Config.log, FALSE, LOG_WARNING, "Modifier_mnemo_cptimpDB: Normalisation unite impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s (id_mnemo,type_ci,multi,unite_string) VALUES "
                "('%d','%d','%f','%s') "
                "ON DUPLICATE KEY UPDATE "
                "type_ci=VALUES(type_ci), multi=VALUES(multi), unite_string=VALUES(unite_string) ",
                NOM_TABLE_MNEMO_CPTIMP,
                mnemo_full->mnemo_base.id, mnemo_full->mnemo_cptimp.type,
                mnemo_full->mnemo_cptimp.multi, unite );
    g_free(unite);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_mnemo_cptimpDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Charger_cpt_imp: Chargement des infos sur les compteurs impulsions depuis la DB                        */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_cpt_imp ( void )
  { struct DB *db;

    if (!Recuperer_mnemo_cptimpDB( &db )) return;

    for( ; ; )
     { struct CMD_TYPE_MNEMO_CPT_IMP *cpt_imp;
       cpt_imp = Recuperer_mnemo_cptimpDB_suite( &db );
       if (!cpt_imp) break;
       if (cpt_imp->num < NBR_COMPTEUR_IMP)
        { memcpy ( &Partage->ci[cpt_imp->num].confDB, cpt_imp, sizeof(struct CMD_TYPE_MNEMO_CPT_IMP) );
          Partage->ci[cpt_imp->num].val_en_cours2 = Partage->ci[cpt_imp->num].confDB.valeur;      /* Init */
        }
       else
        { Info_new( Config.log, FALSE, LOG_WARNING, "Charger_cpt_imp: cpt_imp->num (%d) out of range", cpt_imp->num ); }
       g_free(cpt_imp);
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_cpt_imp: DB reloaded" );
  }
/*--------------------------------------------------------------------------------------------------------*/
