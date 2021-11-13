/******************************************************************************************************************************/
/* Watchdogd/Http/getmessage.c       Gestion des request getmessage pour le thread HTTP de watchdog                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    22.09.2016 14:18:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getmessage.c
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
/* Http_Traiter_request_getmessage: Traite une requete sur l'URI message                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gboolean Http_Traiter_request_getmessage ( struct lws *wsi, struct HTTP_SESSION *session )
  { gchar token_length[12], token_start[12], token_type[12], token_num[12];
    gchar token_dls[200], token_libelle[200],token_groupe[200];
    const gchar *length_s, *start_s, *type_s, *num_s, *libelle, *groupe, *dls;
    unsigned char header[256], *header_cur, *header_end;
   	const char *content_type = "application/json";
    gchar requete[1024], critere[512];
    struct CMD_TYPE_MESSAGE *msg;
    gint type, start, length;
    struct DB *db;
    gint retour;
    JsonBuilder *builder;
    JsonGenerator *gen;
    gchar *buf;
    gsize taille_buf;

    if ( session==NULL || session->util==NULL || Tester_groupe_util(session->util, GID_MESSAGE)==FALSE)
     { Http_Send_error_code ( wsi, 401 );
       return(TRUE);
     }

    type_s   = lws_get_urlarg_by_name	( wsi, "type=",    token_type,    sizeof(token_type) );
    num_s    = lws_get_urlarg_by_name	( wsi, "num=",     token_num,     sizeof(token_num) );
    start_s  = lws_get_urlarg_by_name	( wsi, "start=",   token_start,   sizeof(token_start) );
    length_s = lws_get_urlarg_by_name	( wsi, "length=",  token_length,  sizeof(token_length) );
    libelle  = lws_get_urlarg_by_name	( wsi, "libelle=", token_libelle, sizeof(token_libelle) );
    groupe   = lws_get_urlarg_by_name	( wsi, "groupe=",  token_groupe,  sizeof(token_groupe) );
    dls      = lws_get_urlarg_by_name	( wsi, "dls=",     token_dls,     sizeof(token_dls) );

    g_snprintf( requete, sizeof(requete), "1=1" );
    if (type_s)
     { g_snprintf( critere, sizeof(critere), " AND msg.type=%d", atoi(type_s) );
       g_strlcat( requete, critere, sizeof(requete) );
     }

    if (start_s)
     { start = atoi (start_s); }
    else start=0;

    if (length_s)
     { length = atoi (length_s); }
    else length=500;

    if (num_s)
     { g_snprintf( critere, sizeof(critere), " AND msg.num=%d", atoi(num_s) );
       g_strlcat( requete, critere, sizeof(requete) );
     }

    if (libelle)
     { g_snprintf( critere, sizeof(critere), " AND msg.libelle LIKE '%%%s%%'", libelle );
       g_strlcat( requete, critere, sizeof(requete) );
     }

    if (dls)
     { g_snprintf( critere, sizeof(critere), " AND dls.shortname LIKE '%%%s%%'", dls );
       g_strlcat( requete, critere, sizeof(requete) );
     }

    if (groupe)
     { g_snprintf( critere, sizeof(critere),
                  " AND (syn.groupe LIKE '%%%s%%' OR syn.page LIKE '%%%s%%' OR syn.libelle LIKE '%%%s%%'"
                       " OR dls.shortname LIKE '%%%s%%')",
                   groupe, groupe, groupe, groupe );
       g_strlcat( requete, critere, sizeof(requete) );
     }

/************************************************ Préparation du buffer XML ***************************************************/
    builder = json_builder_new ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getmessage : JSon builder creation failed" );
       return(FALSE);
     }
                                                                      /* Lancement de la requete de recuperation des messages */
    if ( ! Recuperer_messageDB_with_conditions( &db, requete, start, length ) )
     { g_object_unref(builder);
       return(FALSE);
     }
/*------------------------------------------------------- Dumping message ----------------------------------------------------*/
    json_builder_begin_object (builder);                                                        /* Création du noeud principal */
    json_builder_set_member_name  ( builder, "Messages" );
    json_builder_begin_array (builder);                                                        /* Création du noeud principal */
    while ( (msg=Recuperer_messageDB_suite( &db )) != NULL )                     /* Mise en forme avant envoi au client léger */
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

       json_builder_end_object (builder);                                                              /* Fin dump du message */
       g_free(msg);
     }
    json_builder_end_array (builder);                                                                         /* End Document */
    json_builder_end_object (builder);                                                                         /* End Document */

    gen = json_generator_new ();
    json_generator_set_root ( gen, json_builder_get_root(builder) );
    json_generator_set_pretty ( gen, TRUE );
    buf = json_generator_to_data (gen, &taille_buf);
    g_object_unref(builder);
    g_object_unref(gen);
          
/*************************************************** Envoi au client **********************************************************/
    header_cur = header;
    header_end = header + sizeof(header);
    retour = lws_add_http_header_status( wsi, 200, &header_cur, header_end );
    retour = lws_add_http_header_by_token ( wsi, WSI_TOKEN_HTTP_CONTENT_TYPE, (const unsigned char *)content_type, strlen(content_type),
                                           &header_cur, header_end );
    retour = lws_add_http_header_content_length ( wsi, taille_buf, &header_cur, header_end );
    retour = lws_finalize_http_header ( wsi, &header_cur, header_end );
    *header_cur='\0';                                                                               /* Caractere null d'arret */
    lws_write( wsi, header, header_cur - header, LWS_WRITE_HTTP_HEADERS );
    lws_write ( wsi, buf, taille_buf, LWS_WRITE_HTTP);                                                      /* Send to client */
    g_free(buf);                                                      /* Libération du buffer dont nous n'avons plus besoin ! */
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
