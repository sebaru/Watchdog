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
 extern struct HTTP_CONFIG Cfg_http;
/******************************************************************************************************************************/
/* Http_dls_do_plugin: Produit un enregistrement json pour un plugin dls                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static void Http_dls_do_plugin ( void *user_data, struct DLS_PLUGIN *dls )
  { JsonBuilder *builder = user_data;
    struct tm *temps;
    gchar date[80];

    temps = localtime( (time_t *)&dls->start_date );
    if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
          else { g_snprintf(date, sizeof(date), "Erreur"); }

    Json_add_object ( builder, NULL );
    Json_add_int    ( builder, "id",        dls->id );
    Json_add_string ( builder, "tech_id",   dls->tech_id );
    Json_add_string ( builder, "shortname", dls->shortname );
    Json_add_string ( builder, "name" ,     dls->nom );
    if (dls->version) Json_add_string ( builder, "version", dls->version() );
                 else Json_add_string ( builder, "version", "Unknown" );
    Json_add_bool   ( builder, "started",   dls->on );
    Json_add_string ( builder, "start_date", date );

    Json_add_double ( builder, "conso", dls->conso );
    Json_add_bool   ( builder, "debug",                dls->vars.debug );
    Json_add_bool   ( builder, "bit_comm",             Dls_data_get_MONO ( dls->tech_id, "COMM", &dls->vars.bit_comm ) );
    Json_add_bool   ( builder, "bit_defaut",           dls->vars.bit_defaut );
    Json_add_bool   ( builder, "bit_defaut_fixe",      dls->vars.bit_defaut_fixe );
    Json_add_bool   ( builder, "bit_alarme",           dls->vars.bit_alarme );
    Json_add_bool   ( builder, "bit_alarme_fixe",      dls->vars.bit_alarme_fixe );
    Json_add_bool   ( builder, "bit_activite_ok",      dls->vars.bit_activite_ok );

    Json_add_bool   ( builder, "bit_alerte",           dls->vars.bit_alerte );
    Json_add_bool   ( builder, "bit_alerte_fixe",      dls->vars.bit_alerte_fixe );
    Json_add_bool   ( builder, "bit_veille",           dls->vars.bit_veille );

    Json_add_bool   ( builder, "bit_derangement",      dls->vars.bit_derangement );
    Json_add_bool   ( builder, "bit_derangement_fixe", dls->vars.bit_derangement_fixe );
    Json_add_bool   ( builder, "bit_danger",           dls->vars.bit_danger );
    Json_add_bool   ( builder, "bit_danger_fixe",      dls->vars.bit_danger_fixe );
    Json_add_bool   ( builder, "bit_secu_pers_ok",     dls->vars.bit_secupers_ok );

    Json_add_bool   ( builder, "bit_acquit",           dls->vars.bit_acquit );
    Json_add_array  ( builder, "bit_IO_Comms" );
    GSList *liste = dls->Arbre_IO_Comm;
    while(liste)
     { gpointer wtd = liste->data;
       Json_add_object ( builder, NULL );
       Dls_WATCHDOG_to_json ( builder, wtd );
       Json_end_object ( builder );
       liste = g_slist_next( liste );
     }
    Json_end_array  ( builder );
    Json_end_object ( builder );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getdlslist: Traite une requete sur l'URI dlslist                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_dls_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { JsonBuilder *builder;
    gchar *buf;
    gsize taille_buf;

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping dlslist ----------------------------------------------------*/
    Json_add_array ( builder, "plugins" );
    Dls_foreach_plugins ( builder, Http_dls_do_plugin );
    Json_end_array ( builder );

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_dls_run: Donne l'état des bits d'un module, ou de tous les modules si pas de tech_id fourni                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_run ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { GSList *liste;
    gsize taille_buf;
    gchar *buf;

    if (msg->method != SOUP_METHOD_PUT)
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

    gchar *tech_id = Normaliser_as_ascii ( Json_get_string ( request, "tech_id" ) );
    if (!tech_id)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_add_string ( builder, "tech_id", tech_id );
    Json_add_int    ( builder, "top", Partage->top );
/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    Json_add_array ( builder, "CI" );
    liste = Partage->Dls_data_CI;
    while(liste)
     { struct DLS_CI *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_CI_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    Json_add_array ( builder, "BOOL" );
    liste = Partage->Dls_data_BOOL;
    while(liste)
     { struct DLS_BOOL *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_BOOL_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*--------------------------------------------------- Compteur horaires ------------------------------------------------------*/
    Json_add_array ( builder, "CH" );
    liste = Partage->Dls_data_CH;
    while(liste)
     { struct DLS_CH *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_CH_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Entrée Analogique ----------------------------------------------------------*/
    Json_add_array ( builder, "AI" );
    liste = Partage->Dls_data_AI;
    while(liste)
     { struct DLS_AI *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_AI_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Sortie Analogique ----------------------------------------------------------*/
    Json_add_array ( builder, "AO" );
    liste = Partage->Dls_data_AO;
    while(liste)
     { struct DLS_AO *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_AO_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Temporisations -------------------------------------------------------------*/
    Json_add_array ( builder, "T" );
    liste = Partage->Dls_data_TEMPO;
    while(liste)
     { struct DLS_TEMPO *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_TEMPO_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Entrées TOR ----------------------------------------------------------------*/
    Json_add_array ( builder, "DI" );
    liste = Partage->Dls_data_DI;
    while(liste)
     { struct DLS_DI *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_DI_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Sortie TOR -----------------------------------------------------------------*/
    Json_add_array ( builder, "DO" );
    liste = Partage->Dls_data_DO;
    while(liste)
     { struct DLS_DO *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_DO_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Visuels --------------------------------------------------------------------*/
    Json_add_array ( builder, "VISUEL" );
    liste = Partage->Dls_data_VISUEL;
    while(liste)
     { struct DLS_VISUEL *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_VISUEL_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );

/*----------------------------------------------- Messages -------------------------------------------------------------------*/
    Json_add_array ( builder, "MSG" );
    liste = Partage->Dls_data_MSG;
    while(liste)
     { struct DLS_MESSAGES *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_MESSAGE_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Registre -------------------------------------------------------------------*/
    Json_add_array ( builder, "REGISTRE" );
    liste = Partage->Dls_data_REGISTRE;
    while(liste)
     { struct DLS_REGISTRE *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_REGISTRE_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Watchdog -------------------------------------------------------------------*/
    Json_add_array ( builder, "WATCHDOG" );
    liste = Partage->Dls_data_WATCHDOG;
    while(liste)
     { struct DLS_WATCHDOG *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, NULL );
          Dls_WATCHDOG_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*------------------------------------------------------- fin ----------------------------------------------------------------*/
    json_node_unref(request);
    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_dls_source: Fourni une list JSON de la source DLS                                                             */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_source ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { gchar *buf, chaine[256];
    gsize taille_buf;

    if (msg->method != SOUP_METHOD_PUT)
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

    gchar *tech_id = Normaliser_as_ascii ( Json_get_string ( request, "tech_id" ) );
    if (!tech_id)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
    g_snprintf( chaine, sizeof(chaine),
               "SELECT d.* FROM dls as d INNER JOIN syns as s ON d.syn_id=s.id "
               "WHERE tech_id='%s' AND s.access_level<'%d'", tech_id, session->access_level );
    json_node_unref(request);
    SQL_Select_to_JSON ( builder, NULL, chaine );

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
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

    Activer_plugin ( target, TRUE );
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

    Activer_plugin ( target, FALSE );
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

    Debug_plugin ( target, TRUE );
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

    Debug_plugin ( target, FALSE );
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
  { JsonBuilder *builder;
    gchar *buf, chaine[512];
    gsize taille_buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
    g_snprintf(chaine, sizeof(chaine), "SELECT d.id, d.tech_id, d.package, d.syn_id, d.name, d.shortname, d.actif, d.compil_status, "
                                       "d.nbr_compil, d.nbr_ligne, d.compil_date, d.debug, ps.page as ppage, s.page as page "
                                       "FROM dls AS d "
                                       "INNER JOIN syns as s ON d.syn_id=s.id "
                                       "INNER JOIN syns as ps ON s.parent_id = ps.id "
                                       "WHERE s.access_level<'%d' ORDER BY d.tech_id", session->access_level );
    if (SQL_Select_to_JSON ( builder, "plugins", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }
    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar chaine[256];
    if (msg->method != SOUP_METHOD_DELETE)
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
    g_snprintf(chaine, sizeof(chaine),  "DELETE FROM dls WHERE tech_id='%s'", target );
    g_free(target);
    if (SQL_Write (chaine)==FALSE)
       { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Delete Error"); }
	   else soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_Traiter_dls_source: Fourni une list JSON de la source DLS                                                             */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar requete[256];

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
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

    if (Json_has_member ( request, "id" ) )
     { g_snprintf( requete, sizeof(requete),
                  "UPDATE dls SET syn_id='%d', tech_id='%s', shortname='%s', name='%s' WHERE id='%d'",
                   Json_get_int ( request, "syn_id" ), tech_id, shortname, name, Json_get_int ( request, "id" ) );
     }
    else
     { g_snprintf( requete, sizeof(requete),
                  "INSERT INTO dls SET syn_id='%d', tech_id='%s', shortname='%s', name='%s'",
                   Json_get_int ( request, "syn_id" ), tech_id, shortname, name );
     }

    if (SQL_Write (requete))
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Partage->com_dls.Thread_reload = TRUE;                                                                  /* Relance DLS */
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );

    if (Json_has_member ( request, "id" ) )
     { Audit_log ( session, "DLS '%s' changed to %d, %s, %s", tech_id, Json_get_int ( request, "syn_id" ), shortname, name ); }
    else
     { Audit_log ( session, "DLS '%s' created to %d, %s, %s", tech_id, Json_get_int ( request, "syn_id" ), shortname, name ); }

    json_node_unref(request);
    g_free(name);
    g_free(shortname);
    g_free(tech_id);
  }
/******************************************************************************************************************************/
/* Http_Dls_compil: Compilation du plugin DLS fourni. Appellé récursivement pour compiler tous les plugins                    */
/* Entrées: la session utilisateur et le plugin                                                                               */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Dls_compil (void *user_data, struct DLS_PLUGIN *plugin)
  { struct HTTP_CLIENT_SESSION *session = user_data;
    Compiler_source_dls( FALSE, plugin->tech_id, NULL, 0 );                 /* Reset interdit sinon conflit de mutex */
    Audit_log ( session, "DLS '%s' compilé", plugin->tech_id );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_compil ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille, taille_buf;
    gchar log_buffer[1024];
    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( Json_has_member ( request, "compil_all" ) && Json_get_bool ( request, "compil_all" ) == TRUE )
     { Dls_foreach_plugins ( session, Http_Dls_compil );
       json_node_unref(request);
       soup_message_set_status(msg, SOUP_STATUS_OK);
       return;
     }

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    if (Json_has_member ( request, "sourcecode" ))
     { gchar *sourcecode = Json_get_string( request, "sourcecode" );
       Save_source_dls_to_DB ( Json_get_string( request, "tech_id" ), sourcecode, strlen(sourcecode) );
     }

    gint retour = Compiler_source_dls ( TRUE, Json_get_string( request, "tech_id" ), log_buffer, sizeof(log_buffer) );

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Json Memory Error");
       return;
     }

    switch(retour)
     { case DLS_COMPIL_ERROR_LOAD_SOURCE:
            g_snprintf( log_buffer, sizeof(log_buffer), "Unable to open file for '%s' compilation",
                        Json_get_string ( request, "tech_id" ) );
            Json_add_string ( builder, "errorlog", log_buffer );
            Json_add_string ( builder, "result", "error" );
            soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Source File Error" );
       break;
       case DLS_COMPIL_ERROR_LOAD_LOG:
            g_snprintf( log_buffer, sizeof(log_buffer), "Unable to open log file for '%s'",
                        Json_get_string ( request, "tech_id" ) );
            Json_add_string ( builder, "errorlog", log_buffer );
            Json_add_string ( builder, "result", "error" );
            soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Log File Error" );
            break;
       case DLS_COMPIL_OK_WITH_WARNINGS:
            Json_add_string ( builder, "errorlog", log_buffer );
            Json_add_string ( builder, "result", "warning" );
            soup_message_set_status (msg, SOUP_STATUS_OK );
            break;
       case DLS_COMPIL_SYNTAX_ERROR:
            Json_add_string ( builder, "errorlog", log_buffer );
            Json_add_string ( builder, "result", "error" );
            soup_message_set_status (msg, SOUP_STATUS_OK );
            break;
       case DLS_COMPIL_ERROR_FORK_GCC:
            g_snprintf( log_buffer, sizeof(log_buffer), "Gcc fork failed !" );
            Json_add_string ( builder, "errorlog", log_buffer );
            Json_add_string ( builder, "result", "error" );
            soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Gcc Error" );
            break;
       case DLS_COMPIL_OK:
            g_snprintf( log_buffer, sizeof(log_buffer), "-- No error --\n-- Reset plugin OK --" );
            Json_add_string ( builder, "errorlog", log_buffer );
            Json_add_string ( builder, "result", "success" );
            soup_message_set_status (msg, SOUP_STATUS_OK);
            break;
       default : g_snprintf( log_buffer, sizeof(log_buffer), "Unknown Error !");
            Json_add_string ( builder, "errorlog", log_buffer );
            Json_add_string ( builder, "result", "error" );
            soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Unknown Error" );
     }
    Audit_log ( session, "DLS '%s' compilé", Json_get_string ( request, "tech_id" ) );
    json_node_unref(request);
    gchar *buf = Json_get_buf (builder, &taille_buf);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
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
