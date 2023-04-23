/******************************************************************************************************************************/
/* Watchdogd/Http/ws_histo_msgs.c        Gestion des echanges des élements d'historiques/messages via webconnect de watchdog  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    01.05.2020 09:53:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ws_histo_msgs.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
 #include <sys/time.h>
/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"
 #include "Http.h"

/******************************************************************************************************************************/
/* Http_traiter_log: Répond aux requetes sur l'URI log                                                                        */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_histo_alive ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data)
  { gchar critere[32];

    Http_print_request ( server, msg, path );
    gpointer syn_id_src = g_hash_table_lookup ( query, "syn_id" );
    if (syn_id_src) g_snprintf( critere, sizeof(critere), "AND syn.syn_id=%d", atoi(syn_id_src) );
    else bzero ( critere, sizeof(critere) );

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "JSon RootNode creation failed" );
       soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "enregs",
                                  "SELECT histo.*, histo.alive, histo.libelle, msg.typologie, dls.syn_id,"
                                  "parent_syn.page as syn_parent_page, syn.page as syn_page,"
                                  "dls.shortname as dls_shortname, msg.tech_id, msg.acronyme"
                                  " FROM histo_msgs as histo"
                                  " INNER JOIN msgs as msg ON msg.msg_id = histo.msg_id"
                                  " INNER JOIN dls as dls ON dls.tech_id = msg.tech_id"
                                  " INNER JOIN syns as syn ON syn.syn_id = dls.syn_id"
                                  " INNER JOIN syns as parent_syn ON parent_syn.syn_id = syn.parent_id"
                                  " WHERE alive = 1 %s ORDER BY histo.date_create DESC", critere ) == FALSE)
     { soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL);
       Json_node_unref ( RootNode );
       return;
     }

    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
