/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_data.c  Impot/Export des dls_data vers l'API                                                         */
/* Projet WatchDog version 4.0       Gestion d'habitat                                                    01.11.2022 15:12:19 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_data.c
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
 * GNU General Public License for more detdils.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_Export_Data_to_API : Envoie les infos DLS_DATA à la base de données pour sauvegarde !                                  */
/* Entrée : Néant                                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_Export_Data_to_API ( struct DLS_PLUGIN *plugin )
  { if (Config.instance_is_master == FALSE) return;                                /* Seul le master sauvegarde les compteurs */

    JsonNode *RootNode = Json_node_create();
    if (!RootNode)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Error when saving dls_data to API.", __func__ ); }

    gint top = Partage->top;

    JsonArray *BIArray = Json_node_add_array ( RootNode, "mnemos_BI" );
    if (plugin) Dls_all_BI_to_json ( BIArray, plugin );
    else Dls_foreach_plugins ( BIArray, Dls_all_BI_to_json );
    Json_node_add_int ( RootNode, "nbr_mnemos_BI", json_array_get_length ( BIArray ) );

    JsonArray *MONOArray = Json_node_add_array ( RootNode, "mnemos_MONO" );
    if (plugin) Dls_all_MONO_to_json ( MONOArray, plugin );
    else Dls_foreach_plugins ( MONOArray, Dls_all_MONO_to_json );
    Json_node_add_int ( RootNode, "nbr_mnemos_MONO", json_array_get_length ( MONOArray ) );

    JsonArray *REGISTREArray = Json_node_add_array ( RootNode, "mnemos_REGISTRE" );
    if (plugin) Dls_all_REGISTRE_to_json ( REGISTREArray, plugin );
    else Dls_foreach_plugins ( REGISTREArray, Dls_all_REGISTRE_to_json );
    Json_node_add_int ( RootNode, "nbr_mnemos_REGISTRE", json_array_get_length ( REGISTREArray ) );

    JsonArray *AIArray = Json_node_add_array ( RootNode, "mnemos_AI" );
    if (plugin) Dls_all_AI_to_json ( AIArray, plugin );
    else Dls_foreach_plugins ( AIArray, Dls_all_AI_to_json );
    Json_node_add_int ( RootNode, "nbr_mnemos_AI", json_array_get_length ( AIArray ) );

    JsonArray *AOArray = Json_node_add_array ( RootNode, "mnemos_AO" );
    if (plugin) Dls_all_AO_to_json ( AOArray, plugin );
    else Dls_foreach_plugins ( AOArray, Dls_all_AO_to_json );
    Json_node_add_int ( RootNode, "nbr_mnemos_AO", json_array_get_length ( AOArray ) );

    JsonArray *DIArray = Json_node_add_array ( RootNode, "mnemos_DI" );
    if (plugin) Dls_all_DI_to_json ( DIArray, plugin );
    else Dls_foreach_plugins ( DIArray, Dls_all_DI_to_json );
    Json_node_add_int ( RootNode, "nbr_mnemos_DI", json_array_get_length ( DIArray ) );

    JsonArray *DOArray = Json_node_add_array ( RootNode, "mnemos_DO" );
    if (plugin) Dls_all_DO_to_json ( DOArray, plugin );
    else Dls_foreach_plugins ( DOArray, Dls_all_DO_to_json );
    Json_node_add_int ( RootNode, "nbr_mnemos_DO", json_array_get_length ( DOArray ) );

    JsonArray *CIArray = Json_node_add_array ( RootNode, "mnemos_CI" );
    if (plugin) Dls_all_CI_to_json ( CIArray, plugin );
    else Dls_foreach_plugins ( CIArray, Dls_all_CI_to_json );
    Json_node_add_int ( RootNode, "nbr_mnemos_CI", json_array_get_length ( CIArray ) );

    JsonArray *CHArray = Json_node_add_array ( RootNode, "mnemos_CH" );
    if (plugin) Dls_all_CH_to_json ( CHArray, plugin );
    else Dls_foreach_plugins ( CHArray, Dls_all_CH_to_json );
    Json_node_add_int ( RootNode, "nbr_mnemos_CH", json_array_get_length ( CHArray ) );

    JsonNode *api_result = Http_Post_to_global_API ( "/run/mnemos/save", RootNode );
    if (api_result && Json_get_int ( api_result, "api_status" ) == SOUP_STATUS_OK)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                 "%s: '%s': Save %d BI to API.", __func__, plugin->tech_id, Json_get_int ( RootNode, "nbr_mnemos_BI" ) );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                 "%s: '%s': Save %d MONO to API.", __func__, plugin->tech_id, Json_get_int ( RootNode, "nbr_mnemos_MONO" ) );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                 "%s: '%s': Save %d REGISTRE to API.", __func__, plugin->tech_id, Json_get_int ( RootNode, "nbr_mnemos_REGISTRE" ) );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                 "%s: '%s': Save %d DI to API.", __func__, plugin->tech_id, Json_get_int ( RootNode, "nbr_mnemos_DI" ) );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                 "%s: '%s': Save %d DO to API.", __func__, plugin->tech_id, Json_get_int ( RootNode, "nbr_mnemos_DO" ) );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                 "%s: '%s': Save %d AI to API.", __func__, plugin->tech_id, Json_get_int ( RootNode, "nbr_mnemos_AI" ) );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                 "%s: '%s': Save %d AO to API.", __func__, plugin->tech_id, Json_get_int ( RootNode, "nbr_mnemos_AO" ) );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                 "%s: '%s': Save %d CI to API.", __func__, plugin->tech_id, Json_get_int ( RootNode, "nbr_mnemos_CI" ) );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                 "%s: '%s': Save %d CH to API.", __func__, plugin->tech_id, Json_get_int ( RootNode, "nbr_mnemos_CH" ) );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Saved '%s' DLS_DATA in %04.1fs", __func__, plugin->tech_id, (Partage->top-top)/10.0 );
     }
    else
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Error when saving '%s' dls_data to API.", __func__, plugin->tech_id ); }
    Json_node_unref ( api_result );
    Json_node_unref ( RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
