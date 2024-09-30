/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_DI.c  Gestion des Analog Input                                                                       */
/* Projet WatchDog version 4.0       Gestion d'habitat                                                    30.01.2022 14:07:24 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_DI.c
 * This file is part of Watchdog
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
 * GNU General Public License for more detdils.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_data_DI_create_by_array : Création d'un DI pour le plugin                                                              */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_DI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_DI *bit = g_try_malloc0 ( sizeof(struct DLS_DI) );
    if (!bit)
     { Info_new( __func__, Config.log_dls, LOG_ERR, "Memory error for '%s:%s'", tech_id, acronyme );
       return;
     }
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    bit->archivage = Json_get_int ( element, "archivage" );
    bit->etat      = Json_get_bool ( element, "etat" );
    plugin->Dls_data_DI = g_slist_prepend ( plugin->Dls_data_DI, bit );
    Info_new( __func__, Config.log_dls, LOG_INFO,
              "Create bit DLS_DI '%s:%s'=%d (%s) archivage=%d",
               bit->tech_id, bit->acronyme, bit->etat, bit->libelle, bit->archivage );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_DI: Recherche un DI dans les plugins DLS                                                                   */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_DI *Dls_data_lookup_DI ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_DI;
          while (liste)
           { struct DLS_DI *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_get_DI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DI ( struct DLS_DI *bit )
  { if (!bit) return(FALSE);
    return( bit->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_get_DI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DI_up ( struct DLS_DI *bit )
  { if (!bit) return(FALSE);
    return( bit->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_DI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DI_down ( struct DLS_DI *bit )
  { if (!bit) return(FALSE);
    return( bit->edge_down );
  }
/******************************************************************************************************************************/
/* Met à jour l'entrée analogique num à partir de sa valeur avant mise a l'echelle                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_DI ( struct DLS_DI *bit, gboolean valeur )
  { if (!bit) return;

    if (bit->etat != valeur)
     { bit->etat = valeur;
       Info_new( __func__, Config.log_dls, LOG_NOTICE, "Changing DLS_DI '%s:%s'=%d up %d down %d",
                 bit->tech_id, bit->acronyme, valeur, bit->edge_up, bit->edge_down );
       if (valeur) Partage->com_dls.Set_Dls_DI_Edge_up   = g_slist_prepend ( Partage->com_dls.Set_Dls_DI_Edge_up,   bit );
              else Partage->com_dls.Set_Dls_DI_Edge_down = g_slist_prepend ( Partage->com_dls.Set_Dls_DI_Edge_down, bit );
       Partage->audit_bit_interne_per_sec++;
       MQTT_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->etat*1.0 );                         /* Archivage si besoin */
       bit->last_arch = Partage->top;
       Dls_DI_export_to_API ( bit );                                                                         /* envoi a l'API */
     }
  }
/******************************************************************************************************************************/
/* Dls_data_set_DI_pulse: Envoi une impulsion sur une DI                                                                      */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_DI_pulse ( struct DLS_TO_PLUGIN *vars, struct DLS_DI *bit )
  { if (!bit) return;
    Partage->com_dls.Set_Dls_Data = g_slist_append ( Partage->com_dls.Set_Dls_Data, bit );
    Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_NOTICE,
              "Mise a un du bit DI '%s:%s' demandée", bit->tech_id, bit->acronyme );
  }
/******************************************************************************************************************************/
/* Dls_data_set_DI_from_thread_di: Positionne une DI dans DLS depuis une DI 'thread'                                          */
/* Entrées: la structure JSON                                                                                                 */
/* Sortie : TRUE si OK, sinon FALSE                                                                                           */
/******************************************************************************************************************************/
 gboolean Dls_data_set_DI_from_thread_di ( JsonNode *request )
  { if (! (Json_has_member ( request, "thread_tech_id" ) && Json_has_member ( request, "thread_acronyme" ) &&
           Json_has_member ( request, "etat" )
          )
       ) return(FALSE);

    gchar *thread_tech_id  = Json_get_string ( request, "thread_tech_id" );
    gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
    gchar *tech_id         = thread_tech_id;
    gchar *acronyme        = thread_acronyme;

    if (MSRV_Map_from_thread ( request ) && Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) )
     { tech_id  = Json_get_string ( request, "tech_id" );
       acronyme = Json_get_string ( request, "acronyme" );
     }

    struct DLS_DI *bit = Dls_data_lookup_DI ( tech_id, acronyme );
    if (!bit)
     { Info_new( __func__, Config.log_bus, LOG_WARNING, "SET_DI from '%s': '%s:%s'/'%s:%s' not found",
                 thread_tech_id, thread_tech_id, thread_acronyme, tech_id, acronyme );
       return(FALSE);
     }

    Info_new( __func__, Config.log_bus, LOG_INFO, "SET_DI from '%s': '%s:%s'/'%s:%s'=%d (%s)",
              thread_tech_id, thread_tech_id, thread_acronyme, tech_id, acronyme,
              Json_get_bool ( request, "etat" ), bit->libelle );
    Dls_data_set_DI ( bit, Json_get_bool ( request, "etat" ) );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Dls_DI_to_json : Formate un bit au format JSON                                                                             */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_DI_to_json ( JsonNode *element, struct DLS_DI *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool   ( element, "etat", bit->etat );
  }
/******************************************************************************************************************************/
/* Dls_all_DI_to_json: Transforme tous les bits en JSON                                                                       */
/* Entrée: target                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_DI_to_json ( gpointer array, struct DLS_PLUGIN *plugin )
  { JsonArray *RootArray = array;
    GSList *liste = plugin->Dls_data_DI;
    while ( liste )
     { struct DLS_DI *bit = liste->data;
       JsonNode *element = Json_node_create();
       Dls_DI_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
     }
  }
/******************************************************************************************************************************/
/* Dls_DI_export_to_API : Formate un bit au format JSON                                                                       */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_DI_export_to_API ( struct DLS_DI *bit )
  { JsonNode *element = Json_node_create ();
    if (element)
     { Json_node_add_bool ( element, "etat", bit->etat );
       MQTT_Send_to_API   ( element, "DLS_REPORT/DI/%s/%s", bit->tech_id, bit->acronyme );
       Json_node_unref    ( element );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
