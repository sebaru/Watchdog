/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_WATCHDOG.c        Déclaration des fonctions pour la gestion des Watchdogs                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    25.03.2019 14:16:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_WATCHDOG.c
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
/* Dls_data_WATCHDOG_create_by_array : Création d'un WATCHDOG pour le plugin                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_WATCHDOG_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_WATCHDOG *bit = g_try_malloc0 ( sizeof(struct DLS_WATCHDOG) );
    if (!bit)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, tech_id, acronyme );
       return;
     }
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    plugin->Dls_data_WATCHDOG = g_slist_prepend ( plugin->Dls_data_WATCHDOG, bit );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
              "%s: Create bit DLS_WATCHDOG '%s:%s' (%s)", __func__, bit->tech_id, bit->acronyme, bit->libelle );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_WATCHDOG: Recherche un WATCHDOG dans les plugins DLS                                                       */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_WATCHDOG *Dls_data_lookup_WATCHDOG ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       GSList *liste = plugin->Dls_data_WATCHDOG;
       while (liste)
        { struct DLS_WATCHDOG *bit = liste->data;
          if ( !strcasecmp ( bit->acronyme, acronyme ) && !strcasecmp( bit->tech_id, tech_id ) ) return(bit);
          liste = g_slist_next(liste);
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_set_WATCHDOG: Positionne un watchdog en fonction de la valeur en parametre                                        */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_set_WATCHDOG ( struct DLS_TO_PLUGIN *vars, struct DLS_WATCHDOG *bit, gint consigne )
  { if (!bit) return;
    bit->top = Partage->top + consigne;
    Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
              "%s: ligne %04d: Changing DLS_WATCHDOG '%s:%s'=%d", __func__,
              (vars ? vars->num_ligne : -1), bit->tech_id, bit->acronyme, consigne );
    Partage->audit_bit_interne_per_sec++;
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool: Remonte l'etat d'un boolean                                                                             */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_WATCHDOG ( struct DLS_WATCHDOG *bit )
  { if (!bit) return(FALSE);
    bit->etat = (Partage->top < bit->top ? TRUE : FALSE);
    return( bit->etat );
  }
/******************************************************************************************************************************/
/* Dls_DI_to_json : Formate un bit au format JSON                                                                             */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_WATCHDOG_to_json ( JsonNode *element, struct DLS_WATCHDOG *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool   ( element, "etat",     bit->etat );
    gint decompte = bit->top - Partage->top;
    Json_node_add_int    ( element, "decompte", (decompte > 0 ? decompte : 0) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/