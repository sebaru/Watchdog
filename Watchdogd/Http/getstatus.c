/******************************************************************************************************************************/
/* Watchdogd/Http/getstatus.c       Gestion des request getstatus pour le thread HTTP de watchdog                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        sam. 15 juin 2013 11:44:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getstatus.c
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

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"

/******************************************************************************************************************************/
/* Http_Traiter_request_getstatus: Traite une requete sur l'URI status                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  { struct tm *temps;
    gchar date[64];
    gint num;

    if (msg->method != SOUP_METHOD_GET)
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
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_node_add_string ( RootNode, "version",         WTD_VERSION );
    Json_node_add_string ( RootNode, "instance_name",   g_get_host_name() );
    Json_node_add_bool   ( RootNode, "is_master",       Json_get_bool   ( Config.config, "is_master" ) );
    Json_node_add_string ( RootNode, "domain_uuid",     Json_get_string ( Config.config, "domain_uuid" ) );
    Json_node_add_string ( RootNode, "master_hostname", Json_get_string ( Config.config, "master_hostname" ) );
    Json_node_add_string ( RootNode, "run_as",          Json_get_string ( Config.config, "run_as" ) );

    temps = localtime( (time_t *)&Partage->start_time );
    if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
    else       { g_snprintf( date, sizeof(date), "Erreur" ); }
    Json_node_add_string ( RootNode, "started",      date );
    Json_node_add_string ( RootNode, "license",      "GPLv2 or newer" );
    Json_node_add_string ( RootNode, "author_name",  "Sébastien LEFEVRE" );
    Json_node_add_string ( RootNode, "author_email", "sebastien.lefevre@abls-habitat.fr" );

/*------------------------------------------------------- Dumping Running config ---------------------------------------------*/
    Json_node_add_int  ( RootNode, "top",          Partage->top );
    Json_node_add_int  ( RootNode, "bit_par_sec",  Partage->audit_bit_interne_per_sec_hold );
    Json_node_add_int  ( RootNode, "tour_par_sec", Partage->audit_tour_dls_per_sec_hold );
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_visuel );                                       /* Recuperation du nbr de i */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    Json_node_add_int  ( RootNode, "length_visuel", num );
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_msg );                                       /* Recuperation du numero de i */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    Json_node_add_int  ( RootNode, "length_msg", num );

    SQL_Select_to_json_node ( RootNode, NULL, "SELECT * FROM db_status");
    Json_node_add_int    ( RootNode, "nbr_sessions", g_slist_length(Partage->com_http.liste_http_clients) );

    Json_node_add_string ( RootNode, "db_username", Config.db_username );
    Json_node_add_string ( RootNode, "db_hostname", Config.db_hostname );
    Json_node_add_int    ( RootNode, "db_port",     Config.db_port );
    Json_node_add_string ( RootNode, "db_database", Config.db_database );

    Json_node_add_string ( RootNode, "archdb_username", Partage->com_arch.archdb_username );
    Json_node_add_string ( RootNode, "archdb_hostname", Partage->com_arch.archdb_hostname );
    Json_node_add_int    ( RootNode, "archdb_port", Partage->com_arch.archdb_port );
    Json_node_add_string ( RootNode, "archdb_database", Partage->com_arch.archdb_database );
    Json_node_add_int    ( RootNode, "archdb_nbr_enreg", Partage->com_arch.taille_arch );

    Http_Send_json_response ( msg, RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
