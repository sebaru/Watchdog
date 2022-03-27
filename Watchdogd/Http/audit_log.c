/******************************************************************************************************************************/
/* Watchdogd/Http/audit_log.c       Gestion des request audit_log pour le thread HTTP de watchdog                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    19.09.2020 11:01:43 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * audit_log.c
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

/******************************************************************************************************************************/
/* Http_traiter_log_get: Répond aux requetes sur l'URI log                                                                    */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_log_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data)
  { gchar chaine[256];

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0)) return;
/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
    g_snprintf( chaine, sizeof(chaine), "SELECT * FROM audit_log WHERE access_level<=%d ORDER BY date DESC LIMIT 2000", session->access_level );
    SQL_Select_to_json_node ( RootNode, "logs", chaine );

    gchar *buf = Json_node_to_string (RootNode);
    Json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Audit_log: Ajoute un log applicatif                                                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Audit_log ( struct HTTP_CLIENT_SESSION *session, gchar *format, ... )
  { gchar message_src[128], requete[512];
    va_list ap;

    va_start( ap, format );
    g_vsnprintf ( message_src, sizeof(message_src), format, ap );
    va_end ( ap );

    gchar *message = Normaliser_chaine ( message_src );
    if (message)
     { g_snprintf( requete, sizeof(requete), "INSERT INTO audit_log SET username='%s', access_level='%d', message='%s'",
                                              session->username, session->access_level, message );
       g_free(message);
       if (!SQL_Write ( requete ))
        { Info_new ( Config.log, FALSE, LOG_ERR, "%s: Impossible d'ajouter l'audit log '%s'", __func__, requete ); }
     }
    else Info_new ( Config.log, FALSE, LOG_ERR, "%s: Impossible de normaliser le message '%s'", __func__, message_src );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
