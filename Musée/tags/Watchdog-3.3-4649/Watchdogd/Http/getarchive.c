/******************************************************************************************************************************/
/* Watchdogd/Http/getarchive.c       Gestion des requests sur l'URI /archive du webservice                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    06.05.2020 10:57:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getarchive.c
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
 void Http_traiter_archive_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { gchar *buf, requete[256];
    gsize taille_buf;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );

    if ( !session )
     { soup_message_set_status (msg, SOUP_STATUS_FORBIDDEN);
       return;
     }

    gchar *prefix = "/archive/get/";
    if ( ! g_str_has_prefix ( path, prefix ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Prefix");
       return;
     }

    if (!strlen (path+strlen(prefix)))
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
    gchar *temp = g_utf8_strup( path+strlen(prefix), -1 );
    gchar **params = g_strsplit ( temp, "/", 3 );
    g_free(temp);
    if( ! (params && params[0] && params[1]) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       g_strfreev( params );
       return;
     }
    if (params[2]==NULL) params[2]=g_strdup("DAY");
    gchar *tech_id  = Normaliser_chaine ( params[0] );
    gchar *acronyme = Normaliser_chaine ( params[1] );
    gchar *period   = Normaliser_chaine ( params[2] );
    g_strfreev( params );

    g_snprintf(requete, sizeof(requete), "SELECT * FROM dictionnaire WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme );
    if (SQL_Select_to_JSON ( builder, NULL, requete ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       g_free(tech_id);
       g_free(acronyme);
       g_free(period);
       g_object_unref(builder);
       return;
     }
    gint periode = 450;
         if (!strcasecmp(period, "DAY"))   periode = 450;
    else if (!strcasecmp(period, "WEEK"))  periode = 3600;
    else if (!strcasecmp(period, "MONTH")) periode = 43200;
    else if (!strcasecmp(period, "YEAR"))  periode = 86400;
    g_snprintf( requete, sizeof(requete),
               "SELECT FROM_UNIXTIME((UNIX_TIMESTAMP(date_time) DIV %d)*%d) AS date, COALESCE(ROUND(AVG(valeur),3),0) AS moyenne "
               "FROM histo_bit_%s_%s", periode, periode, tech_id, acronyme );
    if (!strcasecmp(period, "HOUR"))  g_strlcat ( requete, " WHERE date_time>=NOW() - INTERVAL 4 HOUR", sizeof(requete) );
    if (!strcasecmp(period, "DAY"))   g_strlcat ( requete, " WHERE date_time>=NOW() - INTERVAL 2 DAY", sizeof(requete) );
    if (!strcasecmp(period, "WEEK"))  g_strlcat ( requete, " WHERE date_time>=NOW() - INTERVAL 2 WEEK", sizeof(requete) );
    if (!strcasecmp(period, "MONTH")) g_strlcat ( requete, " WHERE date_time>=NOW() - INTERVAL 9 WEEK", sizeof(requete) );
    if (!strcasecmp(period, "YEAR"))  g_strlcat ( requete, " WHERE date_time>=NOW() - INTERVAL 13 MONTH", sizeof(requete) );

    g_strlcat ( requete, " GROUP BY date ORDER BY date", sizeof(requete) );

    if (SQL_Arch_to_JSON ( builder, "enregs", requete ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       g_free(tech_id);
       g_free(acronyme);
       g_free(period);
       g_object_unref(builder);
       return;
     }

    g_free(tech_id);
    g_free(acronyme);
    g_free(period);

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
