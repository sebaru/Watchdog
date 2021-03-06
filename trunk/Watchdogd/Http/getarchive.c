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
  { gchar *requete = NULL, chaine[256], *interval, nom_courbe[12];
    gint nbr;

    if (msg->method != SOUP_METHOD_PUT || Config.instance_is_master == FALSE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0)) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "period" ) && Json_has_member ( request, "courbes" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    gchar *period   = Normaliser_chaine ( Json_get_string ( request, "period" ) );
    gint periode = 450;
    interval = " ";
         if (!strcasecmp(period, "HOUR"))  { periode = 450;   interval = " WHERE date_time>=NOW() - INTERVAL 4 HOUR"; }
    else if (!strcasecmp(period, "DAY"))   { periode = 450;   interval = " WHERE date_time>=NOW() - INTERVAL 2 DAY"; }
    else if (!strcasecmp(period, "WEEK"))  { periode = 3600;  interval = " WHERE date_time>=NOW() - INTERVAL 2 WEEK"; }
    else if (!strcasecmp(period, "MONTH")) { periode = 43200; interval = " WHERE date_time>=NOW() - INTERVAL 9 WEEK"; }
    else if (!strcasecmp(period, "YEAR"))  { periode = 86400; interval = " WHERE date_time>=NOW() - INTERVAL 13 MONTH"; }
    g_free(period);

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       json_node_unref(request);
       return;
     }

    gint taille_requete = 32;
    requete = g_try_malloc(taille_requete);
    if (!requete)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       json_node_unref(request);
       return;
     }

    g_snprintf( requete, taille_requete, "SELECT * FROM ");

    int nbr_courbe = json_array_get_length ( Json_get_array ( request, "courbes" ) );
    for (nbr=0; nbr<nbr_courbe; nbr++)
     { g_snprintf( nom_courbe, sizeof(nom_courbe), "courbe%d", nbr+1 );

       JsonNode *courbe = json_array_get_element ( Json_get_array ( request, "courbes" ), nbr );
       gchar *tech_id  = Normaliser_chaine ( Json_get_string ( courbe, "tech_id" ) );
       gchar *acronyme = Normaliser_chaine ( Json_get_string ( courbe, "acronyme" ) );

       g_snprintf( chaine, sizeof(chaine),
                  "%s "
                  "(SELECT FROM_UNIXTIME((UNIX_TIMESTAMP(date_time) DIV %d)*%d) AS date, COALESCE(ROUND(AVG(valeur),3),0) AS moyenne%d "
                  " FROM histo_bit_%s_%s %s GROUP BY date ORDER BY date) AS %s "
                  "%s ",
                  (nbr!=0 ? "INNER JOIN" : ""), periode, periode, nbr+1, tech_id, acronyme, interval, nom_courbe,
                  (nbr!=0 ? "USING(date)" : "") );

       taille_requete += strlen(chaine)+1;
       requete = g_try_realloc ( requete, taille_requete );
       if (requete) g_strlcat ( requete, chaine, taille_requete );

       JsonNode *json_courbe = Json_node_add_objet ( RootNode, nom_courbe );
       g_snprintf(chaine, sizeof(chaine), "SELECT * FROM dictionnaire WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme );
       SQL_Select_to_json_node ( json_courbe, NULL, chaine );

       g_free(tech_id);
       g_free(acronyme);
     }

    if (SQL_Arch_to_json_node ( RootNode, "valeurs", requete ) == FALSE)
     { g_free(requete);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       json_node_unref(request);
       json_node_unref(RootNode);
       return;
     }

    g_free(requete);
    json_node_unref(request);

    gchar *buf = Json_node_to_string (RootNode);
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
