/******************************************************************************************************************************/
/* Watchdogd/Http/getdlslist.c       Gestion des request getdlslist pour le thread HTTP de watchdog                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    15.02.2019 19:54:39 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getdlslist.c
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

 #include <string.h>
 #include <unistd.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"

/******************************************************************************************************************************/
/* Http_dls_do_plugin: Produit un enregistrement json pour un plugin dls                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static void Http_dls_do_plugin ( void *user_data, struct DLS_PLUGIN *dls )
  { JsonArray *array = user_data;
    struct tm *temps;
    gchar date[80];

    temps = localtime( (time_t *)&dls->start_date );
    if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
          else { g_snprintf(date, sizeof(date), "Erreur"); }

    JsonNode *element = Json_node_create();
    Json_node_add_int    ( element, "dls_id",    dls->dls_id );
    Json_node_add_string ( element, "tech_id",   dls->tech_id );
    Json_node_add_string ( element, "shortname", dls->shortname );
    Json_node_add_string ( element, "name" ,     dls->name );
    if (dls->version) Json_node_add_string ( element, "version", dls->version() );
                 else Json_node_add_string ( element, "version", "Unknown" );
    Json_node_add_bool   ( element, "started",   dls->enable );
    Json_node_add_string ( element, "start_date", date );

    Json_node_add_double ( element, "conso", dls->conso );
    Json_node_add_bool   ( element, "debug",                dls->vars.debug );
    Json_node_add_bool   ( element, "bit_comm",             Dls_data_get_MONO ( dls->vars.dls_comm ) );
    Json_node_add_bool   ( element, "bit_defaut",           Dls_data_get_MONO ( dls->vars.dls_memsa_defaut ) );
    Json_node_add_bool   ( element, "bit_defaut_fixe",      Dls_data_get_MONO ( dls->vars.dls_memsa_defaut_fixe ) );
    Json_node_add_bool   ( element, "bit_alarme",           Dls_data_get_MONO ( dls->vars.dls_memsa_alarme ) );
    Json_node_add_bool   ( element, "bit_alarme_fixe",      Dls_data_get_MONO ( dls->vars.dls_memsa_alarme_fixe ) );
    Json_node_add_bool   ( element, "bit_activite_ok",      Dls_data_get_MONO ( dls->vars.dls_memsa_ok ) );

    Json_node_add_bool   ( element, "bit_alerte",           Dls_data_get_MONO ( dls->vars.dls_memssb_alerte ) );
    Json_node_add_bool   ( element, "bit_alerte_fixe",      Dls_data_get_MONO ( dls->vars.dls_memssb_alerte_fixe ) );
    Json_node_add_bool   ( element, "bit_alerte_fugitive",  Dls_data_get_MONO ( dls->vars.dls_memssb_alerte_fugitive ) );
    Json_node_add_bool   ( element, "bit_veille",           Dls_data_get_MONO ( dls->vars.dls_memssb_veille ) );

    Json_node_add_bool   ( element, "bit_derangement",      Dls_data_get_MONO ( dls->vars.dls_memssp_derangement ) );
    Json_node_add_bool   ( element, "bit_derangement_fixe", Dls_data_get_MONO ( dls->vars.dls_memssp_derangement_fixe ) );
    Json_node_add_bool   ( element, "bit_danger",           Dls_data_get_MONO ( dls->vars.dls_memssp_danger ) );
    Json_node_add_bool   ( element, "bit_danger_fixe",      Dls_data_get_MONO ( dls->vars.dls_memssp_danger_fixe ) );
    Json_node_add_bool   ( element, "bit_secu_pers_ok",     Dls_data_get_MONO ( dls->vars.dls_memssp_ok ) );

    Json_node_add_bool   ( element, "bit_acquit",           Dls_data_get_DI   ( dls->vars.dls_osyn_acquit ) );
    JsonArray *comm_array = Json_node_add_array  ( element, "bit_Comms" );
    GSList *liste = dls->Arbre_Comm;
    while(liste)
     { gpointer comm = liste->data;
       JsonNode *json_comm = Json_node_create ();
       Dls_MONO_to_json ( json_comm, comm );
       Json_array_add_element (comm_array, json_comm );
       liste = g_slist_next( liste );
     }
    Json_array_add_element ( array, element );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getdlslist: Traite une requete sur l'URI dlslist                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_dls_status ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data )
  {
/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *dls_status = Json_node_create ();
    if (dls_status == NULL)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "JSon RootNode creation failed" );
       soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
/*------------------------------------------------------- Dumping dlslist ----------------------------------------------------*/
    JsonArray *plugins = Json_node_add_array ( dls_status, "plugins" );
    Dls_foreach_plugins ( plugins, Http_dls_do_plugin );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, dls_status );
  }
/******************************************************************************************************************************/
/* Http_Traiter_dls_run: Donne l'état des bits d'un module, ou de tous les modules si pas de tech_id fourni                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_run ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data )
  { GSList *liste;
    JsonArray *array;

    gchar *tech_id = g_hash_table_lookup ( query, "tech_id" );
    gchar *classe  = g_hash_table_lookup ( query, "classe" );
    if (! (tech_id && classe))
     { soup_server_message_set_status (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    struct DLS_PLUGIN *plugin = Dls_get_plugin_by_tech_id ( tech_id );
    if (!plugin)
     { soup_server_message_set_status (msg, SOUP_STATUS_NOT_FOUND, "Plugin not found");
       return;
     }

    JsonNode *dls_run = Json_node_create ();
    if (!dls_run)
     { soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_node_add_string ( dls_run, "tech_id", tech_id );
    Json_node_add_int    ( dls_run, "top", Partage->top );
/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    if (!strcasecmp ( classe, "CI" ))
     { array = Json_node_add_array ( dls_run, "CI" );
       liste = plugin->Dls_data_CI;
       while(liste)
        { struct DLS_CI *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_CI_to_json ( element, bit );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*--------------------------------------------------- Monostables ------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "MONO" ))
     { array = Json_node_add_array ( dls_run, "MONO" );
       liste = plugin->Dls_data_MONO;
       while(liste)
        { struct DLS_MONO *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_MONO_to_json ( element, bit );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    else if (!strcasecmp ( classe, "BI" ))
     { array = Json_node_add_array ( dls_run, "BI" );
       liste = plugin->Dls_data_BI;
       while(liste)
        { struct DLS_BI *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_BI_to_json ( element, bit );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*--------------------------------------------------- Compteur horaires ------------------------------------------------------*/
    else if (!strcasecmp ( classe, "CH" ))
     { array = Json_node_add_array ( dls_run, "CH" );
       liste = plugin->Dls_data_CH;
       while(liste)
        { struct DLS_CH *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_CH_to_json ( element, bit );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Entrée Analogique ----------------------------------------------------------*/
    else if (!strcasecmp ( classe, "AI" ))
     { array = Json_node_add_array ( dls_run, "AI" );
       liste = plugin->Dls_data_AI;
       while(liste)
        { struct DLS_AI *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_AI_to_json ( element, bit );
          MSRV_Map_to_thread ( element );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Sortie Analogique ----------------------------------------------------------*/
    else if (!strcasecmp ( classe, "AO" ))
     { array = Json_node_add_array ( dls_run, "AO" );
       liste = plugin->Dls_data_AO;
       while(liste)
        { struct DLS_AO *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_AO_to_json ( element, bit );
          MSRV_Map_to_thread ( element );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Temporisations -------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "TEMPO" ))
     { array = Json_node_add_array ( dls_run, "TEMPO" );
       liste = plugin->Dls_data_TEMPO;
       while(liste)
        { struct DLS_TEMPO *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_TEMPO_to_json ( element, bit );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Entrées TOR ----------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "DI" ))
     { array = Json_node_add_array ( dls_run, "DI" );
       liste = plugin->Dls_data_DI;
       while(liste)
        { struct DLS_DI *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_DI_to_json ( element, bit );
          MSRV_Map_to_thread ( element );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Sortie TOR -----------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "DO" ))
     { array = Json_node_add_array ( dls_run, "DO" );
       liste = plugin->Dls_data_DO;
       while(liste)
        { struct DLS_DO *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_DO_to_json ( element, bit );
          MSRV_Map_to_thread ( element );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Visuels --------------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "VISUEL" ))
      {array = Json_node_add_array ( dls_run, "VISUEL" );
       liste = plugin->Dls_data_VISUEL;
       while(liste)
        { struct DLS_VISUEL *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_VISUEL_to_json ( element, bit );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Messages -------------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "MSG" ))
     { array = Json_node_add_array ( dls_run, "MSG" );
       liste = plugin->Dls_data_MESSAGE;
       while(liste)
        { struct DLS_MESSAGE *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_MESSAGE_to_json ( element, bit );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Registre -------------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "REGISTRE" ))
     { array = Json_node_add_array ( dls_run, "REGISTRE" );
       liste = plugin->Dls_data_REGISTRE;
       while(liste)
        { struct DLS_REGISTRE *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_REGISTRE_to_json ( element, bit );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Watchdog -------------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "WATCHDOG" ))
     { array = Json_node_add_array ( dls_run, "WATCHDOG" );
       liste = plugin->Dls_data_WATCHDOG;
       while(liste)
        { struct DLS_WATCHDOG *bit=liste->data;
          JsonNode *element = Json_node_create();
          Dls_WATCHDOG_to_json ( element, bit );
          Json_array_add_element ( array, element );
          liste = g_slist_next(liste);
        }
     }
/*------------------------------------------------------- fin ----------------------------------------------------------------*/
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, dls_run );
  }
/******************************************************************************************************************************/
/* Http_traiter_dls_run_set: Set un bit interne                                                                               */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_run_set ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request )
  { if (!request) return;
    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
            Json_has_member ( request, "classe" ) && Json_has_member ( request, "valeur" )
           )
       )
     { Http_Send_json_response (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres", NULL );
       return;
     }

    struct DLS_PLUGIN *plugin = Dls_get_plugin_by_tech_id ( Json_get_string ( request, "tech_id" ) );
    if (!plugin)
     { Http_Send_json_response (msg, SOUP_STATUS_NOT_FOUND, "Plugin not found", NULL );
       return;
     }

    gchar *tech_id  = Json_get_string ( request, "tech_id" );
    gchar *acronyme = Json_get_string ( request, "acronyme" );
    gchar *classe   = Json_get_string ( request, "classe" );
    if ( !strcasecmp ( classe, "BI" ) )
     { gboolean valeur = Json_get_bool ( request, "valeur" );
       struct DLS_BI *bit = Dls_data_lookup_BI ( tech_id, acronyme );
       Dls_data_set_BI ( NULL, bit, valeur );
       /*Audit_log ( session, "DLS %s '%s:%s' set to %d", classe, tech_id, acronyme, valeur );*/
       Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, NULL );
     }
    else if ( !strcasecmp ( classe, "MONO" ) )
     { struct DLS_MONO *bit = Dls_data_lookup_MONO ( tech_id, acronyme );
       Dls_data_set_MONO ( NULL, bit, TRUE );
       /*Audit_log ( session, "DLS %s '%s:%s' set to TRUE", classe, tech_id, acronyme );*/
       Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, NULL );
     }
    else if ( !strcasecmp ( classe, "MSG" ) )
     { gboolean valeur = Json_get_bool ( request, "valeur" );
       struct DLS_MESSAGE *bit = Dls_data_lookup_MESSAGE ( tech_id, acronyme );
       Dls_data_set_MESSAGE ( NULL, bit, valeur );
       /*Audit_log ( session, "DLS %s '%s:%s' set to %d", classe, tech_id, acronyme, valeur );*/
       Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, NULL );
     }
    else if ( !strcasecmp ( classe, "REGISTRE" ) )
     { gdouble valeur = Json_get_double ( request, "valeur" );
       struct DLS_REGISTRE *bit = Dls_data_lookup_REGISTRE ( tech_id, acronyme );
       Dls_data_set_REGISTRE ( NULL, bit, valeur );
       /*Audit_log ( session, "DLS %s '%s:%s' set to %f", classe, tech_id, acronyme, valeur );*/
       Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, NULL );
     }
    else Http_Send_json_response (msg, SOUP_STATUS_NOT_IMPLEMENTED, "Wrong Class", NULL );
  }
/******************************************************************************************************************************/
/* Http_traiter_dls_run_acquitter: Acquitte un dls                                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_run_acquitter ( SoupServer *server, SoupServerMessage *msg, const char *path, JsonNode *request )
  { if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { Http_Send_json_response (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres", NULL );
       return;
     }

    Dls_Acquitter_plugin ( Json_get_string ( request, "tech_id" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
