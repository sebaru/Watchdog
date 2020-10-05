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
 extern struct HTTP_CONFIG Cfg_http;
/******************************************************************************************************************************/
/* Http_Traiter_request_getstatus: Traite une requete sur l'URI status                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  { gchar date[64], *buf;
    JsonBuilder *builder;
    gsize taille_buf;
    struct tm *temps;
    gint num;

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
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_add_string ( builder, "version",            WTD_VERSION );
    Json_add_string ( builder, "instance",           g_get_host_name() );
    Json_add_bool   ( builder, "instance_is_master", Config.instance_is_master );
    Json_add_string ( builder, "master_host",        (Config.instance_is_master ? "-" : Config.master_host) );
    Json_add_string ( builder, "run_as",             Config.run_as );

    temps = localtime( (time_t *)&Partage->start_time );
    if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
    else       { g_snprintf( date, sizeof(date), "Erreur" ); }
    Json_add_string ( builder, "started",      date );
    Json_add_string ( builder, "license",      "GPLv2 or newer" );
    Json_add_string ( builder, "author_name",  "Sébastien LEFEVRE" );
    Json_add_string ( builder, "author_email", "sebastien.lefevre@abls-habitat.fr" );

/*------------------------------------------------------- Dumping Running config ---------------------------------------------*/
    Json_add_int  ( builder, "top",          Partage->top );
    Json_add_int  ( builder, "bit_par_sec",  Partage->audit_bit_interne_per_sec_hold );
    Json_add_int  ( builder, "tour_par_sec", Partage->audit_tour_dls_per_sec_hold );
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_i );                                            /* Recuperation du nbr de i */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    Json_add_int  ( builder, "length_i", num );
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_msg );                                       /* Recuperation du numero de i */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    Json_add_int  ( builder, "length_msg", num );

    SQL_Select_to_JSON ( builder, NULL, "SELECT * FROM db_status");
    Json_add_int    ( builder, "nbr_sessions", g_slist_length(Cfg_http.liste_http_clients) );

    Json_add_string ( builder, "db_username", Config.db_username );
    Json_add_string ( builder, "db_hostname", Config.db_hostname );
    Json_add_int    ( builder, "db_port",     Config.db_port );
    Json_add_string ( builder, "db_database", Config.db_database );

    Json_add_string ( builder, "archdb_username", Partage->com_arch.archdb_username );
    Json_add_string ( builder, "archdb_hostname", Partage->com_arch.archdb_hostname );
    Json_add_int    ( builder, "archdb_port", Partage->com_arch.archdb_port );
    Json_add_string ( builder, "archdb_database", Partage->com_arch.archdb_database );

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
