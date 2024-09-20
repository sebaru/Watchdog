/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_BI.c        Déclaration des fonctions pour la gestion des booleans                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    24.06.2019 22:07:06 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_BI.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
/* Dls_data_BI_create_by_array : Création d'un BI pour le plugin                                                              */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_BI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_BI *bit = g_try_malloc0 ( sizeof(struct DLS_BI) );
    if (!bit)
     { Info_new( __func__, Config.log_dls, LOG_ERR, "Memory error for '%s:%s'", tech_id, acronyme );
       return;
     }
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    bit->etat = Json_get_bool ( element, "etat" );
    plugin->Dls_data_BI = g_slist_prepend ( plugin->Dls_data_BI, bit );
    Info_new( __func__, Config.log_dls, LOG_INFO,
              "Create bit DLS_BI '%s:%s'=%d (%s)", bit->tech_id, bit->acronyme, bit->etat, bit->libelle );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_BI : Recherche un CH dans les plugins DLS                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 struct DLS_BI *Dls_data_lookup_BI ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_BI;
          while (liste)
           { struct DLS_BI *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_set_BI: Positionne un bistable                                                                                    */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_set_BI ( struct DLS_TO_PLUGIN *vars, struct DLS_BI *bi, gboolean valeur )
  { if (!bi) return;

    if (bi->etat != valeur)
     { Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_BI '%s:%s'=%d up %d down %d",
                 (vars ? vars->num_ligne : -1), bi->tech_id, bi->acronyme, valeur, bi->edge_up, bi->edge_down );
       bi->etat = valeur;
       if (bi->etat == TRUE)
        { Partage->com_dls.Set_Dls_BI_Edge_up   = g_slist_prepend ( Partage->com_dls.Set_Dls_BI_Edge_up, bi ); }
       else
        { Partage->com_dls.Set_Dls_BI_Edge_down = g_slist_prepend ( Partage->com_dls.Set_Dls_BI_Edge_down, bi ); }
       if (vars && vars->debug) Dls_BI_export_to_API ( bi );                                       /* Si debug, envoi a l'API */
       Partage->audit_bit_interne_per_sec++;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_BI: Remonte l'etat d'un bistable                                                                             */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_BI ( struct DLS_BI *bi )
  { if (!bi) return(FALSE);
    return( bi->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bi_up: Remonte le front montant d'un boolean                                                                    */
/* Sortie : TRUE sur le boolean vient de passer à UP                                                                            */
/******************************************************************************************************************************/
 gboolean Dls_data_get_BI_up ( struct DLS_BI *bi )
  { if (!bi) return(FALSE);
    return( bi->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bi_down: Remonte le front descendant d'un boolean                                                               */
/* Sortie : TRUE sur le boolean vient de passer à DOWN                                                                          */
/******************************************************************************************************************************/
 gboolean Dls_data_get_BI_down ( struct DLS_BI *bi )
  { if (!bi) return(FALSE);
    return( bi->edge_down );
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
/******************************************************************************************************************************/
/* Dls_BI_export_to_API : Formate un bit au format JSON                                                                       */
/* Entrées: le bit                                                                                                            */
/* Sortie : le JSON                                                                                                           */
/******************************************************************************************************************************/
 void Dls_BI_export_to_API ( struct DLS_BI *bit )
  { JsonNode *element = Json_node_create ();
    if (element)
     { Json_node_add_bool ( element, "etat", bit->etat );
       MQTT_Send_to_API   ( element, "DLS_REPORT/BI/%s/%s", bit->tech_id, bit->acronyme );
       Json_node_unref    ( element );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
