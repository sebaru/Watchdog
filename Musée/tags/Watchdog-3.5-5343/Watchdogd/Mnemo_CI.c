/******************************************************************************************************************************/
/* Watchdogd/Mnemo_CI.c      Déclaration des fonctions pour la gestion des compteurs d'impulsions                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mar. 07 déc. 2010 17:26:52 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_CI.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
                "SELECT cpt.valeur, cpt.etat, cpt.unite, cpt.multi, cpt.archivage"
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
     { cpt_imp->valeur    = atoi(db->row[0]);
       cpt_imp->etat      = atoi(db->row[1]);
       cpt_imp->archivage = atoi(db->row[4]);
       g_snprintf( cpt_imp->unite, sizeof(cpt_imp->unite), "%s", db->row[2] );
       cpt_imp->multi  = atof(db->row[3]);
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: CI '%s:%s'=%d %s (%d) loaded", __func__,
                 cpt_imp->tech_id, cpt_imp->acronyme, cpt_imp->valeur, cpt_imp->unite, cpt_imp->etat );
       Libere_DB_SQL( &db );
     }
  }
/******************************************************************************************************************************/
/* Updater_confDB_CI: Mise a jour des valeurs de CI en base                                                                   */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_confDB_CI ( void )
  { gchar requete[200];
    GSList *liste;
    struct DB *db;
    gint cpt;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    cpt = 0;
    liste = Partage->Dls_data_CI;
    while ( liste )
     { struct DLS_CI *cpt_imp = (struct DLS_CI *)liste->data;
       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE mnemos_CI as m SET valeur='%d', etat='%d' "
                   "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                   cpt_imp->valeur, cpt_imp->etat, cpt_imp->tech_id, cpt_imp->acronyme );
       Lancer_requete_SQL ( db, requete );
       liste = g_slist_next(liste);
       cpt++;
     }

    Libere_DB_SQL( &db );
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d CptIMP updated", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Dls_CI_to_json : Formate un CI au format JSON                                                                              */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_CI_to_json ( JsonBuilder *builder, struct DLS_CI *bit )
  { Json_add_string ( builder, "tech_id",   bit->tech_id );
    Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_int    ( builder, "valeur",    bit->valeur );
    Json_add_int    ( builder, "imp_par_minute", bit->imp_par_minute );
    Json_add_double ( builder, "multi",     bit->multi );
    Json_add_string ( builder, "unite",     bit->unite );
    Json_add_bool   ( builder, "etat",      bit->etat );
    Json_add_int    ( builder, "archivage", bit->archivage );
    Json_add_int    ( builder, "last_arch", bit->last_arch );
  };
/*----------------------------------------------------------------------------------------------------------------------------*/
