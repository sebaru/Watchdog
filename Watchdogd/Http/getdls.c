/******************************************************************************************************************************/
/* Watchdogd/Http/getdlslist.c       Gestion des request getdlslist pour le thread HTTP de watchdog                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    15.02.2019 19:54:39 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getdlslist.c
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

 #include <string.h>
 #include <unistd.h>
 #include <libxml/xmlwriter.h>

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
    Json_node_add_int    ( element, "dls_id",    dls->id );
    Json_node_add_string ( element, "tech_id",   dls->tech_id );
    Json_node_add_string ( element, "shortname", dls->shortname );
    Json_node_add_string ( element, "name" ,     dls->nom );
    if (dls->version) Json_node_add_string ( element, "version", dls->version() );
                 else Json_node_add_string ( element, "version", "Unknown" );
    Json_node_add_bool   ( element, "started",   dls->on );
    Json_node_add_string ( element, "start_date", date );

    Json_node_add_double ( element, "conso", dls->conso );
    Json_node_add_bool   ( element, "debug",                dls->vars.debug );
    Json_node_add_bool   ( element, "bit_comm",             Dls_data_get_MONO ( dls->tech_id, "COMM", &dls->vars.bit_comm ) );
    Json_node_add_bool   ( element, "bit_defaut",           Dls_data_get_MONO ( dls->tech_id, "MEMSA_DEFAUT", &dls->vars.bit_defaut ) );
    Json_node_add_bool   ( element, "bit_defaut_fixe",      Dls_data_get_MONO ( dls->tech_id, "MEMSA_DEFAUT_FIXE", &dls->vars.bit_defaut_fixe ) );
    Json_node_add_bool   ( element, "bit_alarme",           Dls_data_get_MONO ( dls->tech_id, "MEMSA_ALARME", &dls->vars.bit_alarme ) );
    Json_node_add_bool   ( element, "bit_alarme_fixe",      Dls_data_get_MONO ( dls->tech_id, "MEMSA_ALARME_FIXE", &dls->vars.bit_alarme_fixe ) );
    Json_node_add_bool   ( element, "bit_activite_ok",      Dls_data_get_MONO ( dls->tech_id, "MEMSA_OK", &dls->vars.bit_activite_ok ) );

    Json_node_add_bool   ( element, "bit_alerte",           Dls_data_get_MONO ( dls->tech_id, "MEMSSB_ALERTE", &dls->vars.bit_alerte ) );
    Json_node_add_bool   ( element, "bit_alerte_fixe",      Dls_data_get_MONO ( dls->tech_id, "MEMSSB_ALERTE_FIXE", &dls->vars.bit_alerte_fixe ) );
    Json_node_add_bool   ( element, "bit_alerte_fugitive",  Dls_data_get_MONO ( dls->tech_id, "MEMSSB_ALERTE_FUGITIVE", &dls->vars.bit_alerte_fugitive ) );
    Json_node_add_bool   ( element, "bit_veille",           Dls_data_get_MONO ( dls->tech_id, "MEMSSB_VEILLE", &dls->vars.bit_veille ) );

    Json_node_add_bool   ( element, "bit_derangement",      Dls_data_get_MONO ( dls->tech_id, "MEMSSP_DERANGEMENT", &dls->vars.bit_derangement ) );
    Json_node_add_bool   ( element, "bit_derangement_fixe", Dls_data_get_MONO ( dls->tech_id, "MEMSSP_DERANGEMENT_FIXE", &dls->vars.bit_derangement_fixe ) );
    Json_node_add_bool   ( element, "bit_danger",           Dls_data_get_MONO ( dls->tech_id, "MEMSSP_DANGER", &dls->vars.bit_danger ) );
    Json_node_add_bool   ( element, "bit_danger_fixe",      Dls_data_get_MONO ( dls->tech_id, "MEMSSP_DANGER_FIXE", &dls->vars.bit_danger_fixe ) );
    Json_node_add_bool   ( element, "bit_secu_pers_ok",     Dls_data_get_MONO ( dls->tech_id, "MEMSSP_OK", &dls->vars.bit_secupers_ok ) );

    Json_node_add_bool   ( element, "bit_acquit",           Dls_data_get_DI   ( dls->tech_id, "OSYN_ACQUIT", &dls->vars.bit_acquit ) );
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
 void Http_traiter_dls_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  {
    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *dls_status = Json_node_create ();
    if (dls_status == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping dlslist ----------------------------------------------------*/
    JsonArray *plugins = Json_node_add_array ( dls_status, "plugins" );
    Dls_foreach_plugins ( plugins, Http_dls_do_plugin );

    gchar *buf = Json_node_to_string ( dls_status );
    json_node_unref ( dls_status );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_dls_run: Donne l'état des bits d'un module, ou de tous les modules si pas de tech_id fourni                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_run ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { GSList *liste;
    JsonArray *array;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gchar *tech_id_src = g_hash_table_lookup ( query, "tech_id" );
    gchar *classe_src  = g_hash_table_lookup ( query, "classe" );
    if (! (tech_id_src && classe_src))
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    JsonNode *dls_run = Json_node_create ();
    if (!dls_run)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    gchar *tech_id = Normaliser_chaine ( tech_id_src );
    gchar *classe  = Normaliser_chaine ( classe_src );

/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_node_add_string ( dls_run, "tech_id", tech_id );
    Json_node_add_int    ( dls_run, "top", Partage->top );
/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    if (!strcasecmp ( classe, "CI" ))
     { array = Json_node_add_array ( dls_run, "CI" );
       liste = Partage->Dls_data_CI;
       while(liste)
        { struct DLS_CI *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_CI_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*--------------------------------------------------- Monostables ------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "MONO" ))
     { array = Json_node_add_array ( dls_run, "MONO" );
       liste = Partage->Dls_data_MONO;
       while(liste)
        { struct DLS_MONO *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_MONO_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    else if (!strcasecmp ( classe, "BI" ))
     { array = Json_node_add_array ( dls_run, "BI" );
       liste = Partage->Dls_data_BI;
       while(liste)
        { struct DLS_BI *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_BI_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*--------------------------------------------------- Compteur horaires ------------------------------------------------------*/
    else if (!strcasecmp ( classe, "CH" ))
     { array = Json_node_add_array ( dls_run, "CH" );
       liste = Partage->Dls_data_CH;
       while(liste)
        { struct DLS_CH *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_CH_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Entrée Analogique ----------------------------------------------------------*/
    else if (!strcasecmp ( classe, "AI" ))
     { array = Json_node_add_array ( dls_run, "AI" );
       liste = Partage->Dls_data_AI;
       while(liste)
        { struct DLS_AI *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_AI_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Sortie Analogique ----------------------------------------------------------*/
    else if (!strcasecmp ( classe, "AO" ))
     { array = Json_node_add_array ( dls_run, "AO" );
       liste = Partage->Dls_data_AO;
       while(liste)
        { struct DLS_AO *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_AO_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Temporisations -------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "TEMPO" ))
     { array = Json_node_add_array ( dls_run, "TEMPO" );
       liste = Partage->Dls_data_TEMPO;
       while(liste)
        { struct DLS_TEMPO *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_TEMPO_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Entrées TOR ----------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "DI" ))
     { array = Json_node_add_array ( dls_run, "DI" );
       liste = Partage->Dls_data_DI;
       while(liste)
        { struct DLS_DI *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_DI_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Sortie TOR -----------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "DO" ))
     { array = Json_node_add_array ( dls_run, "DO" );
       liste = Partage->Dls_data_DO;
       while(liste)
        { struct DLS_DO *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_DO_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Visuels --------------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "VISUEL" ))
      {array = Json_node_add_array ( dls_run, "VISUEL" );
       liste = Partage->Dls_data_VISUEL;
       while(liste)
        { struct DLS_VISUEL *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_VISUEL_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Messages -------------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "MSG" ))
     { array = Json_node_add_array ( dls_run, "MSG" );
       liste = Partage->Dls_data_MSG;
       while(liste)
        { struct DLS_MESSAGES *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_MESSAGE_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Registre -------------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "REGISTRE" ))
     { array = Json_node_add_array ( dls_run, "REGISTRE" );
       liste = Partage->Dls_data_REGISTRE;
       while(liste)
        { struct DLS_REGISTRE *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_REGISTRE_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*----------------------------------------------- Watchdog -------------------------------------------------------------------*/
    else if (!strcasecmp ( classe, "WATCHDOG" ))
     { array = Json_node_add_array ( dls_run, "WATCHDOG" );
       liste = Partage->Dls_data_WATCHDOG;
       while(liste)
        { struct DLS_WATCHDOG *bit=liste->data;
          if (!strcasecmp(bit->tech_id, tech_id))
           { JsonNode *element = Json_node_create();
             Dls_WATCHDOG_to_json ( element, bit );
             Json_array_add_element ( array, element );
           }
          liste = g_slist_next(liste);
        }
     }
/*------------------------------------------------------- fin ----------------------------------------------------------------*/
    g_free(tech_id);
    g_free(classe);
    gchar *buf = Json_node_to_string ( dls_run );
    json_node_unref( dls_run );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_dls_source: Fourni une list JSON de la source DLS                                                             */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_source ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gchar *tech_id_src = g_hash_table_lookup ( query, "tech_id" );
    if (!tech_id_src)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *tech_id = Normaliser_chaine ( tech_id_src );
    if (!tech_id)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       g_free(tech_id);
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, NULL,
                                 "SELECT d.*, s.page FROM dls AS d INNER JOIN syns AS s ON d.syn_id=syn_id "
                                 "WHERE tech_id='%s' AND s.access_level<='%d'", tech_id, session->access_level )==FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref ( RootNode );
       g_free(tech_id);
       return;
     }

    g_free(tech_id);
    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_dls_source: Fourni une list JSON de la source DLS                                                             */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_start ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *target = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    json_node_unref(request);
    if (!target)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Normalize Error");
       return;
     }

    Dls_Activer_plugin ( target, TRUE );
    Audit_log ( session, "DLS '%s' started", target );
    g_free(target);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    /*soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );*/
  }
/******************************************************************************************************************************/
/* Http_Traiter_dls_source: Fourni une list JSON de la source DLS                                                             */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_stop ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                              SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *target = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    json_node_unref(request);
    if (!target)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Normalize Error");
       return;
     }

    Dls_Activer_plugin ( target, FALSE );
    Audit_log ( session, "DLS '%s' stopped", target );
    g_free(target);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    /*soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );*/
  }

/******************************************************************************************************************************/
/* Http_Traiter_dls_source: Fourni une list JSON de la source DLS                                                             */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_debug ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data )
   { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *target = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    json_node_unref(request);
    if (!target)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Normalize Error");
       return;
     }

    Dls_Debug_plugin ( target, TRUE );
    Audit_log ( session, "DLS '%s' debug enabled", target );
    g_free(target);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    /*soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );*/
  }
/******************************************************************************************************************************/
/* Http_Traiter_dls_source: Fourni une list JSON de la source DLS                                                             */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_undebug ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *target = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    json_node_unref(request);
    if (!target)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Normalize Error");
       return;
     }

    Dls_Debug_plugin ( target, FALSE );
    Audit_log ( session, "DLS '%s' debug disabled", target );
    g_free(target);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getdlslist: Traite une requete sur l'URI dlslist                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_dls_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                              SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "plugins",
                                 "SELECT d.dls_id, d.tech_id, d.package, d.syn_id, d.name, d.shortname, d.actif, d.compil_status, "
                                 "d.nbr_compil, d.nbr_ligne, d.compil_date, d.debug, ps.page as ppage, s.page as page "
                                 "FROM dls AS d "
                                 "INNER JOIN syns as s ON d.syn_id=s.syn_id "
                                 "INNER JOIN syns as ps ON s.parent_id = ps.syn_id "
                                 "WHERE s.access_level<'%d' ORDER BY d.tech_id", session->access_level )==FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref ( RootNode );
       return;
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *target = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    json_node_unref(request);

    Audit_log ( session, "DLS '%s' supprimé", target );
    if (SQL_Write_new ( "DELETE FROM dls WHERE tech_id='%s'", target )==FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Delete Error"); }
    else
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Partage->com_dls.Thread_reload = TRUE;                                                                  /* Relance DLS */
     }
    g_free(target);
  }
/******************************************************************************************************************************/
/* Http_traiter_dls_set: Positionne les parametres du module DLS                                                              */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     { soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
       return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "shortname" ) &&
            Json_has_member ( request, "name" ) && Json_has_member ( request, "syn_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *tech_id   = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    gchar *shortname = Normaliser_chaine ( Json_get_string ( request, "shortname" ) );
    gchar *name      = Normaliser_chaine ( Json_get_string ( request, "name" ) );

    if (Json_has_member ( request, "dls_id" ) )
     { JsonNode *old = Json_node_create();
       if (old)
        { SQL_Select_to_json_node ( old, NULL, "SELECT tech_id FROM dls WHERE dls_id='%d'", Json_get_int ( request, "dls_id" ) );

          if ( !Json_has_member ( old, "tech_id" ) )
           { soup_message_set_status_full (msg, SOUP_STATUS_NOT_FOUND, "Plugin not found" ); }
          else if (SQL_Write_new ( "UPDATE dls SET syn_id='%d', tech_id='%s', shortname='%s', name='%s' WHERE dls_id='%d'",
                              Json_get_int ( request, "syn_id" ), tech_id, shortname, name, Json_get_int ( request, "dls_id" ) ))
           { soup_message_set_status (msg, SOUP_STATUS_OK);
             if ( strcmp ( Json_get_string ( old, "tech_id" ), tech_id ) )          /* Si modification de tech_id -> recompil */
              { SQL_Write_new ( "UPDATE dls SET `sourcecode` = REPLACE(`sourcecode`, '%s:', '%s:')",
                                Json_get_string ( old, "tech_id" ), tech_id );
                Partage->com_dls.Thread_reload_with_recompil = TRUE;                             /* Relance DLS avec recompil */
              }
             else Partage->com_dls.Thread_reload = TRUE;          /* Relance DLS sans recompil si les tech_id sont identiques */
             Audit_log ( session, "DLS '%s' changed to %d, %s, %s", tech_id, Json_get_int ( request, "syn_id" ), shortname, name );
           }
          else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Update Error" );
          json_node_unref( old );
        }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
     }
    else
     { if (SQL_Write_new ( "INSERT INTO dls SET syn_id='%d', tech_id='%s', shortname='%s', name='%s'",
                           Json_get_int ( request, "syn_id" ), tech_id, shortname, name ) )
        { soup_message_set_status (msg, SOUP_STATUS_OK);
          Partage->com_dls.Thread_reload = TRUE;                                                              /* Relance DLS */
          Audit_log ( session, "DLS '%s' created to %d, %s, %s", tech_id, Json_get_int ( request, "syn_id" ), shortname, name );
        }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Add Error" );
     }

    json_node_unref(request);
    g_free(name);
    g_free(shortname);
    g_free(tech_id);
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_compil ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { gchar log_buffer[1024];
    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    if (Json_has_member ( request, "sourcecode" ))
     { gchar *sourcecode = Json_get_string( request, "sourcecode" );
       Save_source_dls_to_DB ( Json_get_string( request, "tech_id" ), sourcecode, strlen(sourcecode) );
     }

    Dls_Reseter_un_plugin ( Json_get_string( request, "tech_id" ) );

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Json Memory Error");
       return;
     }

    SQL_Select_to_json_node ( RootNode, NULL,
                             "SELECT errorlog, compil_status FROM dls WHERE tech_id='%s'",
                              Json_get_string( request, "tech_id" ) );

    switch(Json_get_int( RootNode, "compil_status" ))
     { case DLS_COMPIL_ERROR_LOAD_SOURCE:
            soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Source File Error" );
       break;
       case DLS_COMPIL_ERROR_LOAD_LOG:
            soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Log File Error" );
            break;
       case DLS_COMPIL_OK_WITH_WARNINGS:
            soup_message_set_status (msg, SOUP_STATUS_OK );
            break;
       case DLS_COMPIL_SYNTAX_ERROR:
            soup_message_set_status (msg, SOUP_STATUS_OK );
            break;
       case DLS_COMPIL_ERROR_FORK_GCC:
            g_snprintf( log_buffer, sizeof(log_buffer), "Gcc fork failed !" );
            soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Gcc Error" );
            break;
       case DLS_COMPIL_OK:
            g_snprintf( log_buffer, sizeof(log_buffer), "-- No error --\n-- Reset plugin OK --" );
            soup_message_set_status (msg, SOUP_STATUS_OK);
            break;
       default : g_snprintf( log_buffer, sizeof(log_buffer), "Unknown Error !");
            soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Unknown Error" );
     }
    Audit_log ( session, "DLS '%s' compilé", Json_get_string ( request, "tech_id" ) );
    json_node_unref(request);
    gchar *buf = Json_node_to_string (RootNode);
    json_node_unref(RootNode);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_run_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
            Json_has_member ( request, "classe" ) && Json_has_member ( request, "valeur" )
           )
       )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *tech_id  = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    gchar *acronyme = Normaliser_chaine ( Json_get_string ( request, "acronyme" ) );
    gchar *classe   = Normaliser_chaine ( Json_get_string ( request, "classe" ) );
    if ( !strcasecmp ( classe, "DI" ) )
     { gboolean valeur = Json_get_bool ( request, "valeur" );
       Dls_data_set_DI ( NULL, tech_id, acronyme, NULL, valeur );
       Audit_log ( session, "DLS %s '%s:%s' set to %d", classe, tech_id, acronyme, valeur );
       soup_message_set_status (msg, SOUP_STATUS_OK );
     }
    else if ( !strcasecmp ( classe, "DO" ) )
     { gboolean valeur = Json_get_bool ( request, "valeur" );
       Dls_data_set_DO ( NULL, tech_id, acronyme, NULL, valeur );
       Audit_log ( session, "DLS %s '%s:%s' set to %d", classe, tech_id, acronyme, valeur );
       soup_message_set_status (msg, SOUP_STATUS_OK );
     }
    else if ( !strcasecmp ( classe, "BI" ) )
     { gboolean valeur = Json_get_bool ( request, "valeur" );
       Dls_data_set_BI ( NULL, tech_id, acronyme, NULL, valeur );
       Audit_log ( session, "DLS %s '%s:%s' set to %d", classe, tech_id, acronyme, valeur );
       soup_message_set_status (msg, SOUP_STATUS_OK );
     }
    else if ( !strcasecmp ( classe, "MONO" ) )
     { Dls_data_set_MONO ( NULL, tech_id, acronyme, NULL, TRUE );
       Audit_log ( session, "DLS %s '%s:%s' set to TRUE", classe, tech_id, acronyme );
       soup_message_set_status (msg, SOUP_STATUS_OK );
     }
    else if ( !strcasecmp ( classe, "MSG" ) )
     { gboolean valeur = Json_get_bool ( request, "valeur" );
       Dls_data_set_MSG ( NULL, tech_id, acronyme, NULL, FALSE, valeur );
       Audit_log ( session, "DLS %s '%s:%s' set to %d", classe, tech_id, acronyme, valeur );
       soup_message_set_status (msg, SOUP_STATUS_OK );
     }
    else if ( !strcasecmp ( classe, "REGISTRE" ) )
     { gdouble valeur = Json_get_double ( request, "valeur" );
       Dls_data_set_REGISTRE ( NULL, tech_id, acronyme, NULL, valeur );
       Audit_log ( session, "DLS %s '%s:%s' set to %f", classe, tech_id, acronyme, valeur );
       soup_message_set_status (msg, SOUP_STATUS_OK );
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_NOT_IMPLEMENTED, "Wrong Class" );
    json_node_unref(request);
    g_free(tech_id);
    g_free(acronyme);
    g_free(classe);
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_acquitter ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    Dls_acquitter_plugin ( Json_get_string ( request, "tech_id" ) );
    Audit_log ( session, "DLS '%s' acquitté", Json_get_string ( request, "tech_id" ) );
    json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
