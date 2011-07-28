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
 #include <locale.h>

 #include "watchdogd.h"
/**********************************************************************************************************/
/* Charger_cpt_imp: Chargement des infos sur les compteurs impulsions depuis la DB                        */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_cpt_imp ( void )
  { struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info( Config.log, DEBUG_INFO, "Charger_cpt_imp: Connexion DB failed" );
       return;
     }                                                                           /* Si pas de histos (??) */

    if (!Recuperer_cpt_impDB( Config.log, db ))
     { Libere_DB_SQL( Config.log, &db );
       return;
     }                                                                         /* Si pas d'enregistrement */

    for( ; ; )
     { struct CMD_TYPE_OPTION_COMPTEUR_IMP *cpt_imp;
       cpt_imp = Recuperer_cpt_impDB_suite( Config.log, db );
       if (!cpt_imp)
        { Libere_DB_SQL( Config.log, &db );
          return;
        }
       memcpy ( &Partage->ci[cpt_imp->num].cpt_impdb, cpt_imp, sizeof(struct CMD_TYPE_OPTION_COMPTEUR_IMP) );
       Partage->ci[cpt_imp->num].val_en_cours2 = Partage->ci[cpt_imp->num].cpt_impdb.valeur;      /* Init */
       g_free(cpt_imp);
     }
  }
/**********************************************************************************************************/
/* Ajouter_cpt_impDB: Ajout ou edition d'un entreeANA                                                     */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure cpt_imp                       */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Updater_cpt_impDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_OPTION_COMPTEUR_IMP *cpt_imp )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET val='%f' WHERE id_mnemo='%d';", NOM_TABLE_CPT_IMP,
                cpt_imp->valeur, cpt_imp->id_mnemo );

    Lancer_requete_SQL ( log, db, requete );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_cpt_impDB ( struct LOG *log, struct DB *db )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id_mnemo,val,num,type_ci,multi,unite"
                " FROM %s,%s WHERE %s.id=%s.id_mnemo ORDER BY %s.num",
                NOM_TABLE_CPT_IMP, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_MNEMO, NOM_TABLE_CPT_IMP, /* WHERE */
                NOM_TABLE_MNEMO /* Order by */
              );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_OPTION_COMPTEUR_IMP *Recuperer_cpt_impDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_OPTION_COMPTEUR_IMP *cpt_imp;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    cpt_imp = (struct CMD_TYPE_OPTION_COMPTEUR_IMP *)g_malloc0( sizeof(struct CMD_TYPE_OPTION_COMPTEUR_IMP) );
    if (!cpt_imp) Info( log, DEBUG_INFO, "Rechercher_cpt_impDB: Erreur allocation mémoire" );
    else
     { cpt_imp->id_mnemo = atoi(db->row[0]);
       cpt_imp->valeur   = atof(db->row[1]);
       cpt_imp->num      = atoi(db->row[2]);
       cpt_imp->type     = atoi(db->row[3]);
       cpt_imp->multi    = atof(db->row[4]);
       memcpy( &cpt_imp->unite, db->row[5], sizeof(cpt_imp->unite) );
     }
    return(cpt_imp);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_cpt_impDB: Recupération de la liste des ids des entreeANAs                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_OPTION_COMPTEUR_IMP *Rechercher_cpt_impDB ( struct LOG *log, struct DB *db, guint id )
  { struct CMD_TYPE_OPTION_COMPTEUR_IMP *cpt_imp;
    gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id_mnemo,val,num,type_ci,multi,unite"
                " FROM %s,%s WHERE %s.id=%s.id_mnemo AND %s.id=%d",
                NOM_TABLE_CPT_IMP, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_MNEMO, NOM_TABLE_CPT_IMP, NOM_TABLE_MNEMO, id /* WHERE */
              );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_cpt_impDB: Cpt_imp non trouvé dans la BDD", id );
       return(NULL);
     }

    cpt_imp = (struct CMD_TYPE_OPTION_COMPTEUR_IMP *)g_malloc0( sizeof(struct CMD_TYPE_OPTION_COMPTEUR_IMP) );
    if (!cpt_imp) Info( log, DEBUG_INFO, "Rechercher_cpt_impDB: Erreur allocation mémoire" );
    else
     { setlocale( LC_NUMERIC, "C" );                 /* Pour le formattage correct des , . dans les float */
       cpt_imp->id_mnemo = atoi(db->row[0]);
       cpt_imp->valeur   = atof(db->row[1]);
       cpt_imp->num      = atoi(db->row[2]);
       cpt_imp->type     = atoi(db->row[3]);
       cpt_imp->multi    = atof(db->row[4]);
       setlocale( LC_NUMERIC, "" );                                   /* Revient sur la locale habituelle */
printf("Option_cmp_impt db->row = %s, multi = %f\n", db->row[4], cpt_imp->multi );
       memcpy( &cpt_imp->unite, db->row[5], sizeof(cpt_imp->unite) );
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
  { gchar requete[1024], *unite;

    unite = Normaliser_chaine ( log, cpt_imp->unite );                   /* Formatage correct des chaines */
    if (!unite)
     { Info( log, DEBUG_SERVEUR, "Modifier_cpt_impDB: Normalisation unite impossible" );
       return(FALSE);
     }
    setlocale( LC_NUMERIC, "C" );                                      /* Pour le formatage du %f correct */
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "unite='%s',multi='%f',type_ci='%d' WHERE id_mnemo=%d",
                NOM_TABLE_CPT_IMP, unite, cpt_imp->multi, cpt_imp->type, cpt_imp->id_mnemo );
    setlocale( LC_NUMERIC, "" );
    g_free(unite);
    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
