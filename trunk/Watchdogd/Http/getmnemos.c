/******************************************************************************************************************************/
/* Watchdogd/Http/getmnemos.c       Gestion des requests sur l'URI /syn du webservice                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    23.07.2020 17:30:10 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getmnemos.c
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
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_mnemos_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { gchar *buf, chaine[256];
    gsize taille_buf;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gchar *tech_id = g_hash_table_lookup ( query, "tech_id" );
    if (!tech_id)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres tech_id");
       return;
     }
    Normaliser_as_tech_id ( tech_id );

    gchar *classe = g_hash_table_lookup ( query, "classe" );
    if (!classe)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres classe");
       return;
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (!strcasecmp ( classe, "DI" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_DI AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "DO" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_DO AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "AI" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_AI AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "AO" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_AO AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "R" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_R AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "CI" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_CI AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "CH" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_CH AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "HORLOGE" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_HORLOGE AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "TEMPO" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_Tempo AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "BOOL" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_BOOL AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "MSG" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from msgs AS m WHERE m.tech_id='%s'", tech_id ); }
    SQL_Select_to_JSON ( builder, classe, chaine );

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_mnemos_set: Modifie la config d'un mnemonique                                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_mnemos_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille;

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( ! (request && Json_has_member ( request, "classe" ) && Json_has_member ( request, "tech_id" ) &&
                       Json_has_member ( request, "acronyme" ) ) )
     { if (request) json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar   *tech_id  = Normaliser_as_tech_id ( Json_get_string ( request,"tech_id" ) );
    gchar   *acronyme = Normaliser_as_tech_id ( Json_get_string ( request,"acronyme" ) );
    gchar   *classe   = Normaliser_as_tech_id ( Json_get_string ( request,"classe" ) );

    if ( ! strcasecmp ( classe, "CI" ) )
     { struct DLS_CI *ci=NULL;
       gchar chaine[128];
       Dls_data_get_CI ( tech_id, acronyme, (gpointer)&ci );
       if ( Json_has_member ( request, "archivage" ) )
        { gboolean archivage = Json_get_bool ( request, "archivage" );
          if (ci) { ci->archivage =  archivage; }                            /* Si le bit existe, on change sa running config */
          g_snprintf(chaine, sizeof(chaine), "UPDATE mnemos_CI SET archivage=%d WHERE tech_id='%s' AND acronyme='%s'",
                     archivage, tech_id, acronyme );
          SQL_Write ( chaine );                                                   /* Qu'il existe ou non, ou met a jour la DB */
        }
     }
    /*else if ( ! strcasecmp ( thread, "dls"  ) ) { Partage->com_dls.Thread_debug = status; }
    else if ( ! strcasecmp ( thread, "db" ) )   { Config.log_db = status; }
    else if ( ! strcasecmp ( thread, "msrv" ) ) { Config.log_msrv = status; }*/
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_traiter_mnemos_validate: Valide la presence ou non d'un tech_id/acronyme dans le dico                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_mnemos_validate ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille, taille_buf;
    gchar *buf, chaine[256];

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( ! (request && Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) ) )
     { if (request) json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar   *tech_id  = Normaliser_as_tech_id ( Json_get_string ( request,"tech_id" ) );
    gchar   *acronyme = Normaliser_as_tech_id ( Json_get_string ( request,"acronyme" ) );

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    g_snprintf(chaine, sizeof(chaine),
              "SELECT DISTINCT(tech_id) FROM dictionnaire WHERE tech_id LIKE '%%%s%%' ORDER BY tech_id LIMIT 10", tech_id );
    SQL_Select_to_JSON ( builder, "tech_ids_found", chaine );

    g_snprintf(chaine, sizeof(chaine),
              "SELECT acronyme FROM dictionnaire WHERE tech_id='%s' AND acronyme LIKE '%%%s%%' ORDER BY acronyme LIMIT 10",
               tech_id, acronyme );
    SQL_Select_to_JSON ( builder, "acronymes_found", chaine );
    json_node_unref(request);

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
