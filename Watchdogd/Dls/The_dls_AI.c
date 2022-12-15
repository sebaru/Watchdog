/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_AI.c  Gestion des Analog Input                                                                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    30.01.2022 14:07:24 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_AI.c
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

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_data_AI_create_by_array : Création d'un AI pour le plugin                                                              */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_AI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_AI *bit = g_try_malloc0 ( sizeof(struct DLS_AI) );
    if (!bit)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, tech_id, acronyme );
       return;
     }
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    bit->valeur   = Json_get_double ( element, "valeur"   );
    bit->in_range = Json_get_bool   ( element, "in_range" );
    if (!strcasecmp ( tech_id, "SYS" ) ) bit->archivage = 2;            /* Si AI du plugin SYS, on archive toutes les minutes */
    plugin->Dls_data_AI = g_slist_prepend ( plugin->Dls_data_AI, bit );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
              "%s: Create bit DLS_AI '%s:%s'=%f (%s)", __func__, bit->tech_id, bit->acronyme, bit->valeur, bit->libelle );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_AI : Recherche un CH dans les plugins DLS                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 struct DLS_AI *Dls_data_lookup_AI ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_AI;
          while (liste)
           { struct DLS_AI *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gdouble Dls_data_get_AI ( struct DLS_AI *bit )
  { if (!bit) return(0.0);
    return( bit->valeur );
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_AI_inrange ( struct DLS_AI *bit )
  { if (!bit) return(FALSE);
    return( bit->in_range );
  }
/******************************************************************************************************************************/
/* Met à jour l'entrée analogique num à partir de sa valeur avant mise a l'echelle                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_AI ( struct DLS_TO_PLUGIN *vars, struct DLS_AI *bit, gdouble valeur, gboolean in_range )
  { if (!bit) return;
    bit->valeur   = valeur;
    bit->in_range = in_range;
    Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
              "%s: Changing DLS_AI '%s:%s'=%d", __func__, bit->tech_id, bit->acronyme, valeur );
  }
/******************************************************************************************************************************/
/* Dls_AI_to_json : Formate un bit au format JSON                                                                             */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_AI_to_json ( JsonNode *element, struct DLS_AI *bit )
  { Json_node_add_string ( element, "tech_id",      bit->tech_id );
    Json_node_add_string ( element, "acronyme",     bit->acronyme );
    Json_node_add_double ( element, "valeur",       bit->valeur );
    Json_node_add_string ( element, "unite",        bit->unite );
    Json_node_add_int    ( element, "in_range",     bit->in_range );
    Json_node_add_int    ( element, "last_arch",    bit->last_arch );
    Json_node_add_int    ( element, "archivage",    bit->archivage );
  }
/******************************************************************************************************************************/
/* Dls_all_AI_to_json: Transforme tous les bits en JSON                                                                       */
/* Entrée: target                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_AI_to_json ( gpointer array, struct DLS_PLUGIN *plugin )
  { JsonArray *RootArray = array;
    GSList *liste = plugin->Dls_data_AI;
    while ( liste )
     { struct DLS_AI *bit = liste->data;
       JsonNode *element = Json_node_create();
       Dls_AI_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
