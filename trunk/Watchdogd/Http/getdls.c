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
 static void Http_dls_do_plugin ( void *user_data, struct PLUGIN_DLS *dls )
  { JsonBuilder *builder = user_data;
    struct tm *temps;
    gchar date[80];

    temps = localtime( (time_t *)&dls->start_date );
    if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
          else { g_snprintf(date, sizeof(date), "Erreur"); }

    Json_add_object ( builder, NULL );
    Json_add_int    ( builder, "id",        dls->plugindb.id );
    Json_add_string ( builder, "tech_id",   dls->plugindb.tech_id );
    Json_add_string ( builder, "shortname", dls->plugindb.shortname );
    Json_add_string ( builder, "name" ,     dls->plugindb.nom );
    if (dls->version) Json_add_string ( builder, "version", dls->version() );
                 else Json_add_string ( builder, "version", "Unknown" );
    Json_add_bool   ( builder, "started",   dls->plugindb.on );
    Json_add_string ( builder, "start_date", date );

    Json_add_double ( builder, "conso", dls->conso );
    Json_add_bool   ( builder, "starting",             dls->vars.starting );
    Json_add_bool   ( builder, "debug",                dls->vars.debug );
    Json_add_bool   ( builder, "bit_comm_out",         dls->vars.bit_comm_out );
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
    Json_end_object ( builder );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getdlslist: Traite une requete sur l'URI dlslist                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_dls_run_all ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { JsonBuilder *builder;
    gchar *buf;
    gsize taille_buf;

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!session)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }

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
    Dls_foreach ( builder, Http_dls_do_plugin, NULL );
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
  { gsize taille_buf;
    GSList *liste;
    gchar *buf;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!session)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }

    gchar *prefix = "/dls/run/";
    if ( ! g_str_has_prefix ( path, prefix ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Prefix");
       return;
     }
    if (!strlen (path+strlen(prefix)))
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }
    gchar *tech_id = Normaliser_chaine ( path+strlen(prefix) );
    if (!tech_id)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { g_free(tech_id);
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
        { Json_add_object ( builder, bit->acronyme );
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
        { Json_add_object ( builder, bit->acronyme );
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
        { Json_add_object ( builder, bit->acronyme );
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
        { Json_add_object ( builder, bit->acronyme );
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
        { Json_add_object ( builder, bit->acronyme );
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
        { Json_add_object ( builder, bit->acronyme );
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
        { Json_add_object ( builder, bit->acronyme );
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
        { Json_add_object ( builder, bit->acronyme );
          Dls_DO_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Visuels --------------------------------------------------------------------*/
    Json_add_array ( builder, "I" );
    liste = Partage->Dls_data_VISUEL;
    while(liste)
     { struct DLS_VISUEL *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
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
        { Json_add_object ( builder, bit->acronyme );
          Dls_MESSAGE_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Registre -------------------------------------------------------------------*/
    Json_add_array ( builder, "R" );
    liste = Partage->Dls_data_REGISTRE;
    while(liste)
     { struct DLS_REGISTRE *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Dls_REGISTRE_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*------------------------------------------------------- fin ----------------------------------------------------------------*/
    g_free(tech_id);
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
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!session)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }

    gchar *prefix = "/dls/source/";
    if ( ! g_str_has_prefix ( path, prefix ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Prefix");
       return;
     }
    if (!strlen (path+strlen(prefix)))
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }
    gchar *tech_id = Normaliser_chaine ( path+strlen(prefix) );
    if (!tech_id)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { g_free(tech_id);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
    g_snprintf( chaine, sizeof(chaine),
               "SELECT d.* FROM dls as d INNER JOIN syns as s ON d.syn_id=s.id "
               "WHERE tech_id='%s' AND s.access_level<'%d'", tech_id, session->access_level );
    g_free(tech_id);
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
  { if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!session)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }

    gchar *prefix = "/dls/start/";
    if ( ! g_str_has_prefix ( path, prefix ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Prefix");
       return;
     }
    if (!strlen (path+strlen(prefix)))
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }
    gchar *tech_id = Normaliser_chaine ( path+strlen(prefix) );
    if (!tech_id)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }
    Activer_plugin ( tech_id, TRUE );
    g_free(tech_id);
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
  { if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!session)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }

    gchar *prefix = "/dls/stop/";
    if ( ! g_str_has_prefix ( path, prefix ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Prefix");
       return;
     }
    if (!strlen (path+strlen(prefix)))
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }
    gchar *tech_id = Normaliser_chaine ( path+strlen(prefix) );
    if (!tech_id)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }
    Activer_plugin ( tech_id, FALSE );
    g_free(tech_id);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    /*soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );*/
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
    if (!session)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
    g_snprintf(chaine, sizeof(chaine), "SELECT d.id, d.tech_id, d.package, d.syn_id, d.name, d.shortname, d.actif, d.compil_status, "
                                       "d.nbr_compil, d.nbr_ligne, d.compil_date, ps.page as ppage, s.page as page "
                                       "FROM dls AS d "
                                       "INNER JOIN syns as s ON d.syn_id=s.id "
                                       "INNER JOIN syns as ps ON s.parent_id = ps.id "
                                       "WHERE s.access_level<'%d'", session->access_level );
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
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_dls_acquitter_plugin ( void *user_data, struct PLUGIN_DLS *plugin )
  { gint dls_id = *(gint *)user_data;
    if (plugin->plugindb.id == dls_id)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: Synoptique %d -> plugin %s acquitté", __func__,
                 plugin->plugindb.id, plugin->plugindb.nom );
       plugin->vars.bit_acquit = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_dls_debug_plugin ( void *user_data, struct PLUGIN_DLS *plugin )
  { gint dls_id = *(gint *)user_data;
    if (plugin->plugindb.id == dls_id)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: Synoptique %d -> plugin %s en mode debug", __func__,
                 plugin->plugindb.id, plugin->plugindb.nom );
       plugin->vars.debug = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_dls_undebug_plugin ( void *user_data, struct PLUGIN_DLS *plugin )
  { gint dls_id = *(gint *)user_data;
    if (plugin->plugindb.id == dls_id)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: Synoptique %d -> plugin %s debug désactivé", __func__,
                 plugin->plugindb.id, plugin->plugindb.nom );
       plugin->vars.debug = FALSE;
     }
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_dls_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar *buf, chaine[256];
    gsize taille_buf;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );

    if ( ! (session && session->access_level >= 6) )
     { soup_message_set_status (msg, SOUP_STATUS_FORBIDDEN);
       return;
     }

    if ( ! g_str_has_prefix ( path, "/dls/del/" ) )
     { soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }


    gchar *target = Normaliser_chaine ( path+9 );
    if (!target)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Normalize Error");
       return;
     }

    g_snprintf(chaine, sizeof(chaine),  "DELETE FROM dls WHERE tech_id='%s'", target );
    g_free(target);
    if (SQL_Write (chaine)==FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Delete Error");
       return;
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_string ( builder, "msg_type", "delete_dls_ok" );
    Json_add_string ( builder, "tech_id", path+9 );
    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess: Traite une requete sur l'URI process                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_dls ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                         SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    Http_print_request ( server, msg, path, client );

         if ( ! strcasecmp( path, "/dls/debug_trad_on" ) )  { Trad_dls_set_debug ( TRUE ); }
    else if ( ! strcasecmp( path, "/dls/debug_trad_off" ) ) { Trad_dls_set_debug ( FALSE ); }
    else if ( ! strcasecmp( path, "/dls/compil" ) )
     { gpointer id_string = g_hash_table_lookup ( query, "id" );
       if (id_string) { Compiler_source_dls( TRUE, atoi(id_string), NULL, 0 ); }
     }
    else if ( ! strcasecmp( path, "/dls/acquit" ) )
     { gpointer id_string = g_hash_table_lookup ( query, "id" );
       if (id_string) { gint id = atoi(id_string); Dls_foreach ( &id, Http_dls_acquitter_plugin, NULL ); }
     }
    else if ( ! strcasecmp( path, "/dls/debug" ) )
     { gpointer id_string = g_hash_table_lookup ( query, "id" );
       if (id_string) { gint id = atoi(id_string); Dls_foreach ( &id, Http_dls_debug_plugin, NULL ); }
     }
    else if ( ! strcasecmp( path, "/dls/undebug" ) )
     { gpointer id_string = g_hash_table_lookup ( query, "id" );
       if (id_string) { gint id = atoi(id_string); Dls_foreach ( &id, Http_dls_undebug_plugin, NULL ); }
     }
    else if ( ! strcasecmp( path, "/dls/delete" ) )
     { gpointer id_string = g_hash_table_lookup ( query, "id" );
       if (id_string) { gint id = atoi(id_string); Decharger_plugin_by_id( id ); }
     }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
