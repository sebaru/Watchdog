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
    if (!session)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }

    gchar *prefix = "/mnemos/list/";
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

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_DI AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "DI", chaine );

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_DO AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "DO", chaine );

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_AI AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "AI", chaine );

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_AO AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "AO", chaine );

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_R AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "REGISTRE", chaine );

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_CI AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "CI", chaine );

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_CH AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "CH", chaine );

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_HORLOGE AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "HORLOGE", chaine );

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_TEMPO AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "TEMPO", chaine );

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_BOOL AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "BOOL", chaine );

    g_snprintf(chaine, sizeof(chaine), "SELECT m.* from msgs AS m WHERE m.tech_id='%s'", tech_id );
    SQL_Select_to_JSON ( builder, "MSG", chaine );

    g_free(tech_id);
    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
