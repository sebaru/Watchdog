/******************************************************************************************************************************/
/* Watchdogd/Mnemo_CH.c      Déclaration des fonctions pour la gestion des cpt_h                                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           mar 14 fév 2006 15:03:51 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_CH.c
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
 #include "Erreur.h"
/******************************************************************************************************************************/
/* Ajouter_Modifier_mnemo_baseDB: Ajout ou modifie le mnemo en parametre                                                      */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_CH ( gchar *tech_id, gchar *acronyme, gchar *libelle_src )
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
                "INSERT INTO mnemos_CH SET tech_id='%s',acronyme='%s',libelle='%s' "
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
/* Charger_conf_ai: Recupération de la conf de l'entrée analogique en parametre                                               */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant l'entrée analogique                                                                       */
/******************************************************************************************************************************/
 void Charger_confDB_CH ( struct DLS_CH *cpt_h )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT cpt.valeur, cpt.etat"
                " FROM mnemos_CH as cpt"
                " WHERE cpt.tech_id='%s' AND cpt.acronyme='%s' LIMIT 1",
                cpt_h->tech_id, cpt_h->acronyme
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( db->row )
     { cpt_h->valeur = atoi(db->row[0]);
       cpt_h->etat   = FALSE;
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: CH '%s:%s'=%d (%d) loaded", __func__,
                 cpt_h->tech_id, cpt_h->acronyme, cpt_h->valeur, cpt_h->etat );
       Libere_DB_SQL( &db );
     }
  }
/******************************************************************************************************************************/
/* Updater_confDB_CH : Met à jour l'ensemble des CompteurHoraire dans la base de données                                      */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_confDB_CH ( void )
  { gchar requete[200];
    GSList *liste;
    struct DB *db;
    gint cpt;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }
    JsonNode  *RootNode  = Json_node_create();
    JsonArray *RootArray = Json_node_add_array ( RootNode, "mnemos_CH" );
    cpt=0;
    liste = Partage->Dls_data_CH;
    while ( liste )
     { struct DLS_CH *cpt_h = (struct DLS_CH *)liste->data;
       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE mnemos_CH as m SET valeur='%d', etat='%d' "
                   "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                   cpt_h->valeur, cpt_h->etat, cpt_h->tech_id, cpt_h->acronyme );
       Lancer_requete_SQL ( db, requete );
       JsonNode *element = Json_node_create();
       Json_node_add_string ( RootNode, "tech_id", cpt_h->tech_id );
       Json_node_add_string ( RootNode, "acronyme", cpt_h->acronyme );
       Json_node_add_int    ( RootNode, "valeur", cpt_h->valeur );
       Json_node_add_bool   ( RootNode, "etat", cpt_h->etat );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
       cpt++;
     }
    JsonNode *api_result = Http_Post_to_global_API ( "/run/mnemos", RootNode );
    if (api_result && Json_get_int ( api_result, "api_status" ) == SOUP_STATUS_OK)
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Save %d MONO to API.", __func__, cpt ); }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Error when saving %d MONO to API.", __func__, cpt ); }
    Json_node_unref ( api_result );
    Json_node_unref ( RootNode );
    Libere_DB_SQL( &db );
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d CptH updated", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Dls_CH_to_json : Formate un CH au format JSON                                                                              */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_CH_to_json ( JsonNode *element, struct DLS_CH *bit )
  { Json_node_add_string ( element, "tech_id",   bit->tech_id );
    Json_node_add_string ( element, "acronyme",  bit->acronyme );
    Json_node_add_int    ( element, "valeur",    bit->valeur );
    Json_node_add_bool   ( element, "etat",      bit->etat );
    Json_node_add_int    ( element, "last_arch", bit->last_arch );
  };
/*----------------------------------------------------------------------------------------------------------------------------*/
