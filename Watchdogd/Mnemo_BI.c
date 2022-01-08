/******************************************************************************************************************************/
/* Watchdogd/Mnemo_BI.c        Déclaration des fonctions pour la gestion des booleans                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    24.06.2019 22:07:06 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_BI.c
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
/* Mnemo_auto_create_BI: Ajoute un mnemonique dans la base via le tech_id                                                   */
/* Entrée: le tech_id, l'acronyme, le libelle                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_BI ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint groupe )
  { gchar *acro, *libelle;
    gboolean retour;

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

    retour = SQL_Write_new ( "INSERT INTO mnemos_BI SET deletable='%d',tech_id='%s',acronyme='%s',libelle='%s', groupe='%d' "
                             "ON DUPLICATE KEY UPDATE libelle=VALUES(libelle), groupe=VALUES(groupe)",
                             deletable, tech_id, acro, libelle, groupe );
    g_free(libelle);
    g_free(acro);

    struct DLS_BI *bi = Dls_data_BI_lookup ( tech_id, acronyme );                  /* Recherche ou Création du message en RAM */
    if (bi) { bi->groupe = groupe; }                             /* Pas de modification de l'etat, on vient de la compilation */

    return (retour);
  }
/******************************************************************************************************************************/
/* Charger_confDB_un_BI: Recupération de la conf d'un bistable                                                                */
/* Entrée: néant                                                                                                              */
/* Sortie: le message est chargé en mémoire                                                                                   */
/******************************************************************************************************************************/
 static void Charger_confDB_un_BI (JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { gint  *cpt_p    = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    gint   groupe   = Json_get_int    ( element, "groupe" );
    gboolean etat   = Json_get_bool   ( element, "etat" );
    (*cpt_p)++;
    struct DLS_BI *bi = Dls_data_BI_lookup ( tech_id, acronyme );                  /* Recherche ou Création du message en RAM */
    if (bi) /* A l'init, on recopie tous les champs */
     { bi->groupe = groupe;
       bi->etat   = bi->next_etat = etat;
     }
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: BI '%s:%s'=%d loaded", __func__, tech_id, acronyme, etat );
  }
/******************************************************************************************************************************/
/* Charger_confDB_BI: Recupération de la conf des entrées TOR                                                                 */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Charger_confDB_BI ( void )
  { gint cpt = 0;

    JsonNode *RootNode = Json_node_create ();
    if (RootNode)
     { SQL_Select_to_json_node ( RootNode, "bis", "SELECT m.tech_id, m.acronyme, m.etat, m.groupe FROM mnemos_BI as m" );
       Json_node_foreach_array_element ( RootNode, "bis", Charger_confDB_un_BI, &cpt );
       json_node_unref ( RootNode );
     } else Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Memory Error", __func__ );

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d BI loaded", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Updater_confDB_BI: Update les parametres d'un bit interne en database                                                      */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_confDB_BI ( void )
  { gint cpt = 0;

    GSList *liste = Partage->Dls_data_BI;
    while ( liste )
     { struct DLS_BI *bi = (struct DLS_BI *)liste->data;
       SQL_Write_new ( "UPDATE mnemos_BI as m SET etat='%d' "
                       "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                       bi->etat, bi->tech_id, bi->acronyme );
       liste = g_slist_next(liste);
       cpt++;
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d BI updated", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Dls_BI_to_json : Formate un bit au format JSON                                                                           */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_BI_to_json ( JsonNode *element, struct DLS_BI *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool   ( element, "etat",     bit->etat );
    Json_node_add_int    ( element, "groupe",   bit->groupe );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
