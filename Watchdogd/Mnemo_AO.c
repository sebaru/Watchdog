/******************************************************************************************************************************/
/* Watchdogd/Mnemo_AO.c        Déclaration des fonctions pour la gestion des Entrée TOR                                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    25.03.2019 14:16:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_AO.c
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
/* Mnemo_create_thread_AO: Créer un JSON pour une AO                                                                          */
/* Entrée: la structure THREAD, les parametres de l'AO                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_thread_AO ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage )
  { JsonNode *node = Json_node_create();
    if (!node) return(NULL);
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Json_node_add_string ( node, "classe", "AO" );
    Json_node_add_string ( node, "thread_tech_id", thread_tech_id );
    Json_node_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_node_add_string ( node, "libelle", libelle );
    Json_node_add_string ( node, "unite", unite );
    Json_node_add_int    ( node, "archivage", archivage );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/ao", node );
    if (!api_result || Json_get_int ( api_result, "api_status" ) != 200)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR,
                 "%s: %s: Could not add AO %s to API", __func__, thread_tech_id, thread_acronyme );
     }
    Json_node_unref ( api_result );
    return(node);
  }
/******************************************************************************************************************************/
/* Dls_AO_to_json: Convertir un AO en JSON                                                                                    */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_AO_to_json ( JsonNode *element, struct DLS_AO *bit )
  { Json_node_add_string ( element, "tech_id",      bit->tech_id );
    Json_node_add_string ( element, "acronyme",     bit->acronyme );
    Json_node_add_double ( element, "valeur_brute", bit->valeur );
    Json_node_add_double ( element, "valeur_min",   bit->min );
    Json_node_add_double ( element, "valeur_max",   bit->max );
    Json_node_add_double ( element, "valeur",       bit->valeur );
    Json_node_add_int    ( element, "type",         bit->type );
  }
/******************************************************************************************************************************/
/* Dls_all_AO_to_json: Transforme tous les bits en JSON                                                                       */
/* Entrée: target                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_AO_to_json ( JsonNode *target )
  { gint cpt = 0;

    JsonArray *RootArray = Json_node_add_array ( target, "mnemos_AO" );
    GSList *liste = Partage->Dls_data_AO;
    while ( liste )
     { struct DLS_AO *bit = (struct DLS_AO *)liste->data;
       JsonNode *element = Json_node_create();
       Dls_AO_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
       cpt++;
     }
    Json_node_add_int ( target, "nbr_mnemos_AO", cpt );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
