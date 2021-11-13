/******************************************************************************************************************************/
/* Watchdogd/Http/getstatus.c       Gestion des request getstatus pour le thread HTTP de watchdog                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        sam. 15 juin 2013 11:44:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getstatus.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
  { unsigned char header[256], *header_cur, *header_end;
	   gchar host[128], date[64], *buf;
    JsonBuilder *builder;
    JsonGenerator *gen;
    gsize taille_buf;
    struct tm *temps;
    gint retour, num;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = json_builder_new ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "%s : JSon builder creation failed", __func__ );
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       return(1);
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    json_builder_begin_object (builder);                                                       /* Création du noeud principal */
    json_builder_set_member_name  ( builder, "Status" );
    json_builder_begin_object (builder);                                                                 /* Contenu du Status */

    gethostname( host, sizeof(host) );
    json_builder_set_member_name  ( builder, "host" );         json_builder_add_string_value ( builder, host );
    json_builder_set_member_name  ( builder, "version" );      json_builder_add_string_value ( builder, VERSION );
    json_builder_set_member_name  ( builder, "instance" );     json_builder_add_string_value ( builder, Config.instance_id );
    localtime( (time_t *)&Partage->start_time );
    if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
    else       { g_snprintf( date, sizeof(date), "Erreur" ); }
    json_builder_set_member_name  ( builder, "started" );      json_builder_add_string_value ( builder, date );
    json_builder_set_member_name  ( builder, "license" );      json_builder_add_string_value ( builder, "GPLv2 or newer" );
    json_builder_set_member_name  ( builder, "author_name" );  json_builder_add_string_value ( builder, "Sébastien LEFEVRE" );
    json_builder_set_member_name  ( builder, "author_email" ); json_builder_add_string_value ( builder, "sebastien.lefevre@abls-habitat.fr" );

/*------------------------------------------------------- Dumping Running config ---------------------------------------------*/
    json_builder_set_member_name  ( builder, "top" );          json_builder_add_int_value  ( builder, Partage->top );
    json_builder_set_member_name  ( builder, "bit_par_sec" );  json_builder_add_int_value  ( builder, Partage->audit_bit_interne_per_sec_hold );
    json_builder_set_member_name  ( builder, "tour_par_sec" ); json_builder_add_int_value  ( builder, Partage->audit_tour_dls_per_sec_hold );
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_i );                                            /* Recuperation du nbr de i */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    json_builder_set_member_name  ( builder, "length_i" );     json_builder_add_int_value  ( builder, num );
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_msg );                                       /* Recuperation du numero de i */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    json_builder_set_member_name  ( builder, "length_msg" );   json_builder_add_int_value  ( builder, num );
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_msg_repeat );                                           /* liste des repeat */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    json_builder_set_member_name  ( builder, "length_msg_repeat" ); json_builder_add_int_value  ( builder, num );

    json_builder_end_object (builder);                                                                  /* Fin dump du status */

    json_builder_end_object (builder);                                                                        /* End Document */

    gen = json_generator_new ();
    json_generator_set_root ( gen, json_builder_get_root(builder) );
    json_generator_set_pretty ( gen, TRUE );
    buf = json_generator_to_data (gen, &taille_buf);
    g_object_unref(builder);
    g_object_unref(gen);
          
/*************************************************** Envoi au client **********************************************************/
    Http_Send_response_code_with_buffer ( wsi, HTTP_200_OK, HTTP_CONTENT_JSON, buf, taille_buf );
    g_free(buf);                                                      /* Libération du buffer dont nous n'avons plus besoin ! */
    return(lws_http_transaction_completed(wsi));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
