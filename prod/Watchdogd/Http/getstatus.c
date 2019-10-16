/******************************************************************************************************************************/
/* Watchdogd/Http/getstatus.c       Gestion des request getstatus pour le thread HTTP de watchdog                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        sam. 15 juin 2013 11:44:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getstatus.c
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
/* Http_Traiter_request_getstatus: Traite une requete sur l'URI status                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gboolean Http_Traiter_request_getstatus ( struct lws *wsi )
  { gchar date[64], *buf;
    JsonBuilder *builder;
    gsize taille_buf;
    struct tm *temps;
    gint num;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       return(1);
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    json_builder_begin_object (builder);                                                       /* Création du noeud principal */
    json_builder_set_member_name  ( builder, "Status" );
    json_builder_begin_object (builder);                                                                 /* Contenu du Status */

    Json_add_string ( builder, "version",  VERSION );
    Json_add_string ( builder, "instance", g_get_host_name() );
    Json_add_bool   ( builder, "instance_is_master", Config.instance_is_master );

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
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_msg_repeat );                                           /* liste des repeat */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    Json_add_int  ( builder, "length_msg_repeat", num );

    json_builder_end_object (builder);                                                                  /* Fin dump du status */

    json_builder_end_object (builder);                                                                        /* End Document */

    buf = Json_get_buf ( builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
    return(Http_Send_response_code_with_buffer ( wsi, HTTP_200_OK, HTTP_CONTENT_JSON, buf, taille_buf ));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
