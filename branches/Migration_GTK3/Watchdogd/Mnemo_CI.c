/******************************************************************************************************************************/
/* Watchdogd/Mnemo_CI.c      Déclaration des fonctions pour la gestion des compteurs d'impulsions                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mar. 07 déc. 2010 17:26:52 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_CI.c
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
 #include <locale.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Ajouter_Modifier_mnemo_baseDB: Ajout ou modifie le mnemo en parametre                                                      */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_CI ( gchar *tech_id, gchar *acronyme, gchar *libelle_src )
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
                "INSERT INTO mnemos_CI SET tech_id='%s',acronyme='%s',libelle='%s' "
                " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                tech_id, acro, libelle );
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
/* Rechercher_CI: Recupération des champs de base de données pour le CI tech_id:acro en parametre                             */
/* Entrée: le tech_id et l'acronyme a récupérer                                                                               */
/* Sortie: la struct DB                                                                                                       */
/******************************************************************************************************************************/
 struct DB *Rechercher_CI ( gchar *tech_id, gchar *acronyme )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT cpt.valeur, cpt.unite"
                " FROM mnemos_CI as cpt"
                " WHERE cpt.tech_id='%s' AND cpt.acronyme='%s' LIMIT 1",
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
/* Charger_conf_CI: Recupération de la conf de l'entrée analogique en parametre                                               */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant l'entrée analogique                                                                       */
/******************************************************************************************************************************/
 void Charger_conf_CI ( struct DLS_CI *cpt_imp )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT cpt.valeur, cpt.etat, cpt.unite, cpt.multi"
                " FROM mnemos_CI as cpt"
                " WHERE cpt.tech_id='%s' AND cpt.acronyme='%s' LIMIT 1",
                cpt_imp->tech_id, cpt_imp->acronyme
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( db->row )
     { cpt_imp->valeur = atoi(db->row[0]);
       cpt_imp->etat   = atoi(db->row[1]);
       g_snprintf( cpt_imp->unite, sizeof(cpt_imp->unite), "%s", db->row[2] );
       cpt_imp->multi  = atof(db->row[3]);
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: CI '%s:%s'=%d %s (%d) loaded", __func__,
                 cpt_imp->tech_id, cpt_imp->acronyme, cpt_imp->valeur, cpt_imp->unite, cpt_imp->etat );
       Libere_DB_SQL( &db );
     }
  }
/******************************************************************************************************************************/
/* Rechercher_mnemocpt_impDB: Recherche les valeurs en DB du compteurs d'impulsion dont l'id est en parametre                 */
/* Entrée: un id_mnemo                                                                                                        */
/* Sortie: les valeurs en base du compteur                                                                                    */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_CPT_IMP *Rechercher_mnemo_cptimpDB ( guint id )
  { struct CMD_TYPE_MNEMO_CPT_IMP *cpt_imp;
    gchar requete[200];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_mnemo_cptimpDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT valeur,type_ci,multi,unite_string"
                " FROM %s"
                " INNER JOIN %s ON id_mnemo = id"
                " WHERE id=%d LIMIT 1",
                NOM_TABLE_MNEMO_CPTIMP,                                                                               /* From */
                NOM_TABLE_MNEMO,                                                                                /* INNER JOIN */
                id
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

    cpt_imp = (struct CMD_TYPE_MNEMO_CPT_IMP *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_CPT_IMP) );
    if (!cpt_imp) Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Recuperer_mnemo_cptimpDB_suite: Erreur allocation mémoire" );
    else
     { cpt_imp->valeur   = atof(db->row[0]);
       cpt_imp->type     = atoi(db->row[1]);
       cpt_imp->multi    = atof(db->row[2]);
       g_snprintf( cpt_imp->unite, sizeof(cpt_imp->unite), "%s", db->row[3] );
     }
    return(cpt_imp);
  }
/******************************************************************************************************************************/
/* Modifier_cpt_impDB: Modification d'un compteur d'impulsion                                                                 */
/* Entrées: la structure mnemo_full (base+options)                                                                            */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Modifier_mnemo_cptimpDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024], *unite;
    gboolean retour;
    struct DB *db;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    unite = Normaliser_chaine ( mnemo_full->mnemo_cptimp.unite );                            /* Formatage correct des chaines */
    if (!unite)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_mnemo_cptimpDB: Normalisation unite impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s (id_mnemo,type_ci,multi,unite_string,valeur) VALUES "
                "('%d','%d','%f','%s','0') "
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

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    if (mnemo_full->mnemo_cptimp.reset) Partage->ci[mnemo_full->mnemo_base.num].val_en_cours1 = 0;
    return(retour);
  }
/******************************************************************************************************************************/
/* Charger_cpt_imp: Chargement des infos sur les compteurs impulsions depuis la DB                                            */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_cpt_imp ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Charger_cpt_impDB: Connexion DB impossible" );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT num,valeur,type_ci,multi,unite_string"
                " FROM %s"
                " INNER JOIN %s ON id_mnemo = id ORDER BY num",
                NOM_TABLE_MNEMO_CPTIMP, NOM_TABLE_MNEMO );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while ( Recuperer_ligne_SQL(db) )                                                      /* Chargement d'une ligne resultat */
     { gint num;
       num = atoi( db->row[0] );
       if (num < NBR_COMPTEUR_IMP)
        { Partage->ci[num].confDB.valeur = atof( db->row[1] );
          Partage->ci[num].confDB.type   = atoi( db->row[2] );
          Partage->ci[num].confDB.multi  = atof( db->row[3] );
          g_snprintf( Partage->ci[num].confDB.unite, sizeof(Partage->ci[num].confDB.unite), "%s", db->row[4] );
          Partage->ci[num].val_en_cours2 = Partage->ci[num].confDB.valeur;                                            /* Init */
          Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                    "Charger_cpt_imp: Chargement config CI[%04d]", num );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
			       "Charger_cpt_imp: num (%d) out of range (max=%d)", num, NBR_COMPTEUR_IMP ); }
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_cpt_imp: DB reloaded" );
    Libere_DB_SQL (&db);
  }
/******************************************************************************************************************************/
/* Ajouter_cpt_impDB: Ajout ou edition d'un entreeANA                                                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_cpt_impDB ( void )
  { struct CMD_TYPE_MNEMO_CPT_IMP *cpt_imp;
    gchar requete[200];
    GSList *liste;
    struct DB *db;
    gint cpt;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    for( cpt=0; cpt<NBR_COMPTEUR_IMP; cpt++)
     { cpt_imp = &Partage->ci[cpt].confDB;
       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE %s JOIN %s ON id_mnemo=id SET valeur='%f' WHERE num='%d';",
                   NOM_TABLE_MNEMO_CPTIMP, NOM_TABLE_MNEMO,
                   cpt_imp->valeur, cpt );
       Lancer_requete_SQL ( db, requete );
     }

    liste = Partage->Dls_data_CI;
    while ( liste )
     { struct DLS_CI *cpt_imp = (struct DLS_CI *)liste->data;
       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE mnemos_CI as m SET valeur='%d', etat='%d' "
                   "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                   cpt_imp->valeur, cpt_imp->etat, cpt_imp->tech_id, cpt_imp->acronyme );
       Lancer_requete_SQL ( db, requete );
       liste = g_slist_next(liste);
     }

    Libere_DB_SQL( &db );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: CptIMP updated", __func__ );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
