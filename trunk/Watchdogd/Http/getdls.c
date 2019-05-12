/******************************************************************************************************************************/
/* Watchdogd/Http/getdlslist.c       Gestion des request getdlslist pour le thread HTTP de watchdog                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    15.02.2019 19:54:39 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getdlslist.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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
 static void Http_dls_do_plugin ( void *user_data, struct PLUGIN_DLS *dls )
  { JsonBuilder *builder = user_data;
    struct tm *temps;
    gchar date[80];

    temps = localtime( (time_t *)&dls->start_date );
    if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
          else { g_snprintf(date, sizeof(date), "Erreur"); }

    json_builder_begin_object (builder);
    json_builder_set_member_name  ( builder, "id" );        json_builder_add_int_value ( builder, dls->plugindb.id );
    json_builder_set_member_name  ( builder, "tech_id" );   json_builder_add_string_value ( builder, dls->plugindb.tech_id );
    json_builder_set_member_name  ( builder, "shortname" ); json_builder_add_string_value ( builder, dls->plugindb.shortname );
    json_builder_set_member_name  ( builder, "name" );      json_builder_add_string_value ( builder, dls->plugindb.nom );
    json_builder_set_member_name  ( builder, "started" );   json_builder_add_boolean_value ( builder, dls->plugindb.on );
    json_builder_set_member_name  ( builder, "start_date" );json_builder_add_string_value ( builder, date );

    json_builder_set_member_name  ( builder, "conso" );     json_builder_add_double_value ( builder, dls->conso );
    json_builder_set_member_name  ( builder, "starting" );  json_builder_add_boolean_value ( builder, dls->vars.starting );
    json_builder_set_member_name  ( builder, "debug" );     json_builder_add_boolean_value ( builder, dls->vars.debug );
    json_builder_set_member_name  ( builder, "bit_comm_out" ); json_builder_add_boolean_value ( builder, dls->vars.bit_comm_out );
    json_builder_set_member_name  ( builder, "bit_defaut" ); json_builder_add_boolean_value ( builder, dls->vars.bit_defaut );
    json_builder_set_member_name  ( builder, "bit_defaut_fixe" ); json_builder_add_boolean_value ( builder, dls->vars.bit_defaut_fixe );
    json_builder_set_member_name  ( builder, "bit_alarme" ); json_builder_add_boolean_value ( builder, dls->vars.bit_alarme );
    json_builder_set_member_name  ( builder, "bit_alarme_fixe" ); json_builder_add_boolean_value ( builder, dls->vars.bit_alarme_fixe );
    json_builder_set_member_name  ( builder, "bit_alerte" ); json_builder_add_boolean_value ( builder, dls->vars.bit_alerte );
    json_builder_set_member_name  ( builder, "bit_alerte_fixe" ); json_builder_add_boolean_value ( builder, dls->vars.bit_alerte_fixe );
    json_builder_set_member_name  ( builder, "bit_derangement" ); json_builder_add_boolean_value ( builder, dls->vars.bit_derangement );
    json_builder_set_member_name  ( builder, "bit_derangement_fixe" ); json_builder_add_boolean_value ( builder, dls->vars.bit_derangement_fixe );
    json_builder_set_member_name  ( builder, "bit_danger" ); json_builder_add_boolean_value ( builder, dls->vars.bit_danger );
    json_builder_set_member_name  ( builder, "bit_danger_fixe" ); json_builder_add_boolean_value ( builder, dls->vars.bit_danger_fixe );
    json_builder_set_member_name  ( builder, "bit_acquit" ); json_builder_add_boolean_value ( builder, dls->vars.bit_acquit );
    json_builder_set_member_name  ( builder, "bit_activite_up" ); json_builder_add_boolean_value ( builder, dls->vars.bit_activite_up );
    json_builder_set_member_name  ( builder, "bit_veille" ); json_builder_add_boolean_value ( builder, dls->vars.bit_veille );
    json_builder_end_object (builder);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getdlslist: Traite une requete sur l'URI dlslist                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gint Http_dls_getlist ( struct lws *wsi )
  { gchar *buf;
    JsonBuilder *builder;
    JsonGenerator *gen;
    gsize taille_buf;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = json_builder_new ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_SERVER_ERROR ));
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping dlslist ----------------------------------------------------*/
    json_builder_begin_array (builder);                                                        /* Création du noeud principal */

    Dls_foreach ( builder, Http_dls_do_plugin, NULL );

    json_builder_end_array (builder);                                                                         /* End Document */

    gen = json_generator_new ();
    json_generator_set_root ( gen, json_builder_get_root(builder) );
    json_generator_set_pretty ( gen, TRUE );
    buf = json_generator_to_data (gen, &taille_buf);
    g_object_unref(builder);
    g_object_unref(gen);

/*************************************************** Envoi au client **********************************************************/
    return(Http_Send_response_code_with_buffer ( wsi, HTTP_200_OK, HTTP_CONTENT_JSON, buf, taille_buf ));
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
/* Http_Traiter_request_getprocess: Traite une requete sur l'URI process                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gint Http_Traiter_request_getdls ( struct lws *wsi, gchar *url )
  {
    if ( ! strcasecmp( url, "debug_trad_on" ) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Setting Dls Trad Debug ON", __func__ );
       Trad_dls_set_debug ( TRUE );
     }
    else if ( ! strcasecmp( url, "debug_trad_off" ) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Setting Dls Trad Debug OFF", __func__ );
       Trad_dls_set_debug ( FALSE );
     }
    else if ( ! strcasecmp( url, "list" ) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: /dls/list received", __func__ );
       return(Http_dls_getlist ( wsi ));
     }
    else if ( ! strcasecmp( url, "compil" ) )
     { gint id = Http_get_arg_int ( wsi, "id" );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Compiling DLS %d", __func__, id );
       Compiler_source_dls( TRUE, id, NULL, 0 );
     }
    else if ( ! strcasecmp( url, "acquit" ) )
     { gint id = Http_get_arg_int ( wsi, "id" );
       Dls_foreach ( &id, Http_dls_acquitter_plugin, NULL );
     }
    else if ( ! strcasecmp( url, "debug" ) )
     { gint id = Http_get_arg_int ( wsi, "id" );
       Dls_foreach ( &id, Http_dls_debug_plugin, NULL );
     }
    else if ( ! strcasecmp( url, "undebug" ) )
     { gint id = Http_get_arg_int ( wsi, "id" );
       Dls_foreach ( &id, Http_dls_undebug_plugin, NULL );
     }
    else if ( ! strcasecmp( url, "delete" ) )
     { gint id = Http_get_arg_int ( wsi, "id" );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Delete DLS %d", __func__, id );
       Decharger_plugin_by_id( id );
     }
    else if ( ! strcasecmp( url, "start" ) )
     { gint id = Http_get_arg_int ( wsi, "id" );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Activating DLS %d", __func__, id );
       while (Partage->com_dls.admin_start) sched_yield();
       Partage->com_dls.admin_start = id;
     }
    else if ( ! strcasecmp( url, "stop" ) )
     { gint id = Http_get_arg_int ( wsi, "id" );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: DesActivating DLS %d", __func__, id );
       while (Partage->com_dls.admin_stop) sched_yield();
       Partage->com_dls.admin_stop = id;
     }
    else return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));
    return(Http_Send_response_code ( wsi, HTTP_200_OK ));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
