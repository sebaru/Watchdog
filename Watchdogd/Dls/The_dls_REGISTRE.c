/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_REGITRE.c              Déclaration des fonctions pour la gestion des registre.c                      */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                                22.03.2017 10:29:53 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_REGISTRE.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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
/* Dls_data_REGISTRE_create_by_array : Création d'un REGISTRE pour le plugin                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_REGISTRE_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_REGISTRE *bit = g_try_malloc0 ( sizeof(struct DLS_REGISTRE) );
    if (!bit)
     { Info_new( __func__, Config.log_dls, LOG_ERR, "Memory error for '%s:%s'", tech_id, acronyme );
       return;
     }
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    g_snprintf( bit->unite,    sizeof(bit->unite),    "%s", Json_get_string ( element, "unite" ) );
    bit->valeur    = Json_get_double ( element, "valeur" );
    bit->archivage = Json_get_int    ( element, "archivage" );
    plugin->Dls_data_REGISTRE = g_slist_prepend ( plugin->Dls_data_REGISTRE, bit );
    Info_new( __func__, Config.log_dls, LOG_INFO,
              "Create bit DLS_REGISTRE '%s:%s'=%f (%s)", bit->tech_id, bit->acronyme, bit->valeur, bit->libelle );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_REGISTRE: Recherche un REGISTRE dans les plugins DLS                                                       */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_REGISTRE *Dls_data_lookup_REGISTRE ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_REGISTRE;
          while (liste)
           { struct DLS_REGISTRE *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_set_REGISTRE: Positionne un registre                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_REGISTRE ( struct DLS_TO_PLUGIN *vars, struct DLS_REGISTRE *reg, gdouble valeur )
  { if (!reg) return;
    if (valeur != reg->valeur)
     { reg->valeur = valeur;
#warning migrate to DLS_REPORT
/*       if (reg->abonnement) Dls_cadran_send_REGISTRE_to_API ( reg );*/
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_REGISTRE '%s:%s'=%f",
                 (vars ? vars->num_ligne : -1), reg->tech_id, reg->acronyme, reg->valeur );
       Partage->audit_bit_interne_per_sec++;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_REGISTRE: Remonte la valeur d'un registre                                                                     */
/* Sortie : la valeur double du registre                                                                                      */
/******************************************************************************************************************************/
 gdouble Dls_data_get_REGISTRE ( struct DLS_REGISTRE *reg )
  { if (!reg) return(0.0);
    return( reg->valeur );
  }
/******************************************************************************************************************************/
/* Dls_REGISTRE_to_json : Formate un bit au format JSON                                                                       */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_REGISTRE_to_json ( JsonNode *element, struct DLS_REGISTRE *bit )
  { Json_node_add_string ( element, "classe",   "REGISTRE" );
    Json_node_add_string ( element, "tech_id",   bit->tech_id );
    Json_node_add_string ( element, "acronyme",  bit->acronyme );
    Json_node_add_double ( element, "valeur",    bit->valeur );
    Json_node_add_string ( element, "unite",     bit->unite );
    Json_node_add_int    ( element, "archivage", bit->archivage );
    Json_node_add_string ( element, "libelle",   bit->libelle );
  }
/******************************************************************************************************************************/
/* Dls_all_REGISTRE_to_json: Transforme tous les bits en JSON                                                                 */
/* Entrée: target                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_REGISTRE_to_json ( gpointer array, struct DLS_PLUGIN *plugin )
  { JsonArray *RootArray = array;
    GSList *liste = plugin->Dls_data_REGISTRE;
    while ( liste )
     { struct DLS_REGISTRE *bit = liste->data;
       JsonNode *element = Json_node_create();
       Dls_REGISTRE_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
