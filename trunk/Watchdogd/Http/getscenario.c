/******************************************************************************************************************************/
/* Watchdogd/Http/getscenario.c       Gestion des request getscenario pour le thread HTTP de watchdog                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    04.08.2017 20:10:47 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getscenario.c
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

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
/******************************************************************************************************************************/
/* Http_Traiter_request_getscenario: Traite une requete sur l'URI scenario                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gint Http_Traiter_request_getscenario ( struct lws *wsi, struct HTTP_SESSION *session )
  { gchar token_id[12];
    const gchar *id_s;
    gchar requete[1024], critere[512];
    struct CMD_TYPE_MESSAGE *msg;
    gint type, start, length;
    struct DB *db;
    gint retour;
    JsonBuilder *builder;
    JsonGenerator *gen;
    gchar *buf;
    gsize taille_buf;

#idef bouh
    if ( session==NULL || session->util==NULL || Tester_groupe_util(session->util, GID_MESSAGE)==FALSE )
     { Http_Send_response_code ( wsi, HTTP_UNAUTHORIZED );
       return(1);
     }
#endif

    id_s = lws_get_urlarg_by_name	( wsi, "id=", token_id, sizeof(token_id) );
    if (id_s)
     { id = atoi (id_s); }
    else id=0;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = json_builder_new ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getscenario : JSon builder creation failed" );
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       return(1);
     }
                                                                      /* Lancement de la requete de recuperation des scenarios */
    if ( ! Recuperer_scenarioDB_with_conditions( &db, requete, start, length ) )
     { g_object_unref(builder);
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       return(1);
     }
/*------------------------------------------------------- Dumping scenario ----------------------------------------------------*/
    json_builder_begin_object (builder);                                                       /* Création du noeud principal */
    json_builder_set_member_name  ( builder, "Messages" );
    json_builder_begin_array (builder);                                                        /* Création du noeud principal */
    while ( (msg=Recuperer_scenarioDB_suite( &db )) != NULL )                     /* Mise en forme avant envoi au client léger */
     { 
       json_builder_begin_object (builder);                                                             /* Contenu du Message */

       json_builder_set_member_name  ( builder, "id" );            json_builder_add_int_value    ( builder, msg->id );
       json_builder_set_member_name  ( builder, "type" );          json_builder_add_int_value    ( builder, msg->type );
       json_builder_set_member_name  ( builder, "enable" );        json_builder_add_boolean_value( builder, msg->enable );
       json_builder_set_member_name  ( builder, "num" );           json_builder_add_int_value    ( builder, msg->num );
       json_builder_set_member_name  ( builder, "sms" );           json_builder_add_int_value    ( builder, msg->sms );
       json_builder_set_member_name  ( builder, "audio" );         json_builder_add_boolean_value( builder, msg->audio );
       json_builder_set_member_name  ( builder, "bit_audio" );     json_builder_add_int_value    ( builder, msg->bit_audio );
       json_builder_set_member_name  ( builder, "time_repeat" );   json_builder_add_int_value    ( builder, msg->time_repeat );
       json_builder_set_member_name  ( builder, "dls_id" );        json_builder_add_int_value    ( builder, msg->dls_id );
       json_builder_set_member_name  ( builder, "libelle" );       json_builder_add_string_value ( builder, msg->libelle );
       json_builder_set_member_name  ( builder, "libelle_sms" );   json_builder_add_string_value ( builder, msg->libelle_sms );
       json_builder_set_member_name  ( builder, "syn_groupe" );    json_builder_add_string_value ( builder, msg->syn_groupe );
       json_builder_set_member_name  ( builder, "syn_page" );      json_builder_add_string_value ( builder, msg->syn_page );
       json_builder_set_member_name  ( builder, "syn_libelle" );   json_builder_add_string_value ( builder, msg->syn_libelle );
       json_builder_set_member_name  ( builder, "dls_shortname" ); json_builder_add_string_value ( builder, msg->dls_shortname );

       json_builder_end_object (builder);                                                              /* Fin dump du scenario */
       g_free(msg);
     }
    json_builder_end_array (builder);                                                                         /* End Document */
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
