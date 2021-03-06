/******************************************************************************************************************************/
/* Watchdogd/Mnemo_BOOL.c        Déclaration des fonctions pour la gestion des booleans                                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    24.06.2019 22:07:06 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_BOOL.c
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

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Mnemo_auto_create_BOOL: Ajoute un mnemonique dans la base via le tech_id                                                   */
/* Entrée: le tech_id, l'acronyme, le libelle                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_BOOL ( gboolean deletable, gint type, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
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
                "INSERT INTO mnemos_BOOL SET deletable='%d', type='%d',tech_id='%s',acronyme='%s',libelle='%s' "
                "ON DUPLICATE KEY UPDATE libelle=VALUES(libelle), type=VALUES(type)",
                deletable, type, tech_id, acro, libelle );
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
/* Charger_conf_ai: Recupération de la conf de l'entrée analogique en parametre                                               */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant l'entrée analogique                                                                       */
/******************************************************************************************************************************/
 void Charger_confDB_BOOL ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT m.tech_id, m.acronyme, m.etat, m.type FROM mnemos_BOOL as m"
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while (Recuperer_ligne_SQL(db))                                                        /* Chargement d'une ligne resultat */
     { gint type = atoi(db->row[3]);
            if (type == MNEMO_BISTABLE )   Dls_data_set_BI   ( NULL, db->row[0], db->row[1], NULL, atoi(db->row[2]) );
       else if (type == MNEMO_MONOSTABLE ) Dls_data_set_MONO ( NULL, db->row[0], db->row[1], NULL, atoi(db->row[2]) );
       Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: BOOL '%s:%s'=%d loaded", __func__,
                 db->row[0], db->row[1], atoi(db->row[2]) );
     }
    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Ajouter_cpt_impDB: Ajout ou edition d'un entreeANA                                                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_confDB_BOOL ( void )
  { gchar requete[200];
    GSList *liste;
    struct DB *db;
    gint cpt = 0;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    liste = Partage->Dls_data_BOOL;
    while ( liste )
     { struct DLS_BOOL *bool = (struct DLS_BOOL *)liste->data;
       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE mnemos_BOOL as m SET etat='%d' "
                   "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                   bool->etat, bool->tech_id, bool->acronyme );
       Lancer_requete_SQL ( db, requete );
       liste = g_slist_next(liste);
       cpt++;
     }

    Libere_DB_SQL( &db );
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d BOOL updated", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Dls_BOOL_to_json : Formate un bit au format JSON                                                                           */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_BOOL_to_json ( JsonBuilder *builder, struct DLS_BOOL *bit )
  { Json_add_string ( builder, "tech_id",  bit->tech_id );
    Json_add_string ( builder, "acronyme", bit->acronyme );
    Json_add_bool   ( builder, "etat",     bit->etat );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
