/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_MONO.c        Déclaration des fonctions pour la gestion des booleans                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    24.06.2019 22:07:06 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_MONO.c
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
/* Dls_data_MONO_create_by_array : Création d'un MONO pour le plugin                                                              */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_MONO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_MONO *bit = g_try_malloc0 ( sizeof(struct DLS_MONO) );
    if (!bit)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, tech_id, acronyme );
       return;
     }
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    bit->etat = Json_get_bool ( element, "etat" );
    plugin->Dls_data_MONO = g_slist_prepend ( plugin->Dls_data_MONO, bit );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_MONO: Recherche un MONO dans les plugins DLS                                                                   */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_MONO *Dls_data_lookup_MONO ( gchar *tech_id, gchar *acronyme )
  { GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       GSList *liste = plugin->Dls_data_MONO;
       while (liste)
        { struct DLS_MONO *bit = liste->data;
          if ( !strcasecmp ( bit->acronyme, acronyme ) && !strcasecmp( bit->tech_id, tech_id ) ) return(bit);
          liste = g_slist_next(liste);
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_set_MONO: Positionne un monostable                                                                                */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_set_MONO ( struct DLS_TO_PLUGIN *vars, struct DLS_MONO *mono, gboolean valeur )
  { if(!mono) return;
    if (valeur == FALSE) { mono->etat = FALSE; }
    else
     { if (mono->etat == FALSE)
        { Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                    "%s: ligne %04d: Changing DLS_MONO '%s:%s'=1", __func__,
                    (vars ? vars->num_ligne : -1), mono->tech_id, mono->acronyme );
          Partage->audit_bit_interne_per_sec++;
        }
       mono->next_etat = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_MONO: Remonte l'etat d'un monostable                                                                          */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_MONO ( struct DLS_MONO *mono )
  { if (!mono) return(FALSE);
    return( mono->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_get_mono_up: Remonte le front montant d'un boolean                                                                */
/* Sortie : TRUE sur le boolean vient de passer à UP                                                                          */
/******************************************************************************************************************************/
 gboolean Dls_data_get_MONO_up ( struct DLS_MONO *mono )
  { if (!mono) return(FALSE);
    return( mono->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_mono_down: Remonte le front descendant d'un boolean                                                           */
/* Sortie : TRUE sur le boolean vient de passer à DOWN                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_MONO_down ( struct DLS_MONO *mono )
  { if (!mono) return(FALSE);
    return( mono->edge_down );
  }
/******************************************************************************************************************************/
/* Dls_MONO_to_json : Formate un bit au format JSON                                                                           */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_MONO_to_json ( JsonNode *element, struct DLS_MONO *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool   ( element, "etat",     bit->etat );
  }
/******************************************************************************************************************************/
/* Dls_all_MONO_to_json: Transforme tous les bits en JSON                                                                     */
/* Entrée: target                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_MONO_to_json ( gpointer array, struct DLS_PLUGIN *plugin )
  { JsonArray *RootArray = array;
    GSList *liste = plugin->Dls_data_MONO;
    while ( liste )
     { struct DLS_MONO *bit = liste->data;
       JsonNode *element = Json_node_create();
       Dls_MONO_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
