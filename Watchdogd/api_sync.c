/******************************************************************************************************************************/
/* Watchdogd/api_sync.c        Interconnexion avec l'API                                                                      */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                10.06.2022 10:04:24 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * api_sync.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

 #include <glib.h>
 #include <string.h>
 #include <unistd.h>
 #include <time.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Post_to_global_API ( gchar *URI, JsonNode *RootNode )
  { gboolean unref_RootNode = FALSE;
    gchar query[256];

    g_snprintf( query, sizeof(query), "https://%s%s", Json_get_string ( Config.config, "api_url" ), URI );
/********************************************************* Envoi de la requete ************************************************/
    SoupMessage *soup_msg  = soup_message_new ( "POST", query );
    if (!soup_msg)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Wrong URI Sending to API %s", query );
       return(NULL);
     }

    if (!RootNode) { RootNode = Json_node_create(); unref_RootNode = TRUE; }
    Json_node_add_int ( RootNode, "request_time", time(NULL) );

    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Sending to API %s", query );

    JsonNode *ResponseNode = Http_Request ( query, RootNode );

    if (unref_RootNode) Json_node_unref(RootNode);

    gint http_code = Json_get_int ( ResponseNode, "http_code" );
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "%s Status %d", URI, status_code );

    if (Http_Request!=200)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "%s Error %d for '%s': %s\n", URI, status_code, query, reason_phrase ); }
    return(ResponseNode);
 }
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Get_from_global_API ( gchar *URI, gchar *format, ... )
  { gchar query[512];
    va_list ap;

    if (format)
     { gchar parametres[128];
       va_start( ap, format );
       g_vsnprintf ( parametres, sizeof(parametres), format, ap );
       va_end ( ap );
       g_snprintf( query, sizeof(query), "https://%s/%s?%s", Json_get_string ( Config.config, "api_url"), URI, parametres );
     }
    else g_snprintf( query, sizeof(query), "https://%s/%s", Json_get_string ( Config.config, "api_url"), URI );
/********************************************************* Envoi de la requete ************************************************/
    JsonNode *ResponseNode = Http_Request ( query, NULL );
    if (!ResponseNode) { Info_new( __func__, Config.log_msrv, LOG_ERR, "Error with Http_Get %s", query ); return(NULL); }
    gint http_code = Json_get_int ( ResponseNode, "http_code" );
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "%s Status %d for '%s'", URI, http_code, query );

    gchar nom_fichier[256];
    g_snprintf ( nom_fichier, sizeof(nom_fichier), "cache-%s", query );
    g_strcanon ( nom_fichier+6, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWYYZ", '_' );

    if (http_code!=200)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "%s Error %d for '%s'", URI, http_code, query );
       Json_node_unref ( ResponseNode );
       ResponseNode = Json_read_from_file ( nom_fichier );
       if (ResponseNode) Info_new( __func__, Config.log_msrv, LOG_WARNING, "Using cache for %s", query );
     }
    else
     { if (Json_has_member ( ResponseNode, "api_cache" ) && Json_get_bool ( ResponseNode, "api_cache" ) )
        { Json_write_to_file ( nom_fichier, ResponseNode ); }
     }
    return(ResponseNode);
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
