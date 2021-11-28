/******************************************************************************************************************************/
/* Watchdogd/Imsgs/admin_imsg.c        Gestion des responses Admin IMSG au serveur watchdog                                   */
/* Projet WatchDog version 3.0        Gestion d'habitat                                                   25.02.2018 17:36:21 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_imsg.c
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

 #include "watchdogd.h"
 #include "Imsg.h"

/******************************************************************************************************************************/
/* Admin_json_imsgs_status : fonction appelée pour vérifier le status de la librairie                                         */
/* Entrée : un JSon Builder                                                                                                   */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_imsgs_status ( struct PROCESS *Lib, SoupMessage *msg )
  {
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_node_add_bool ( RootNode, "thread_is_running", Lib->Thread_run );
    SQL_Select_to_json_node ( RootNode, NULL, "SELECT * FROM %s WHERE instance='%s'", Lib->name, g_get_host_name() );

    if (Lib->Thread_run)                                      /* Warning : Cfg_meteo does not exist if thread is not running ! */
     { /*Json_node_add_string ( RootNode, "tech_id", Cfg.tech_id );*/
     }
    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Admin_json_imsgs_set: Configure le thread IMSGS                                                                              */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_imsgs_set ( struct PROCESS *Lib, SoupMessage *msg )
  { if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) &&
            Json_has_member ( request, "jabberid" ) && Json_has_member ( request, "password" )
           ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id  = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *jabberid = Normaliser_chaine ( Json_get_string( request, "jabberid" ) );
    gchar *password = Normaliser_chaine ( Json_get_string( request, "password" ) );
    json_node_unref(request);

    SQL_Write_new ( "INSERT INTO %s SET instance='%s', tech_id='%s', jabberid='%s', password='%s' "
                    "ON DUPLICATE KEY UPDATE tech_id=VALUES(tech_id), jabberid=VALUES(jabberid), password=VALUES(password)",
                    Lib->name, g_get_host_name(), tech_id, jabberid, password );
    g_free(tech_id);
    g_free(jabberid);
    g_free(password);

    soup_message_set_status (msg, SOUP_STATUS_OK);
    Lib->Thread_reload = TRUE;
    while ( Lib->Thread_run == TRUE && Lib->Thread_reload == TRUE);                              /* Attente reboot du process */
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 void Admin_json ( struct PROCESS *lib, SoupMessage *msg, const char *path, GHashTable *query, gint access_level )
  { if (access_level < 6)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }
         if (!strcasecmp(path, "/status")) { Admin_json_imsgs_status ( lib, msg ); }
    else if (!strcasecmp(path, "/set"))    { Admin_json_imsgs_set ( lib, msg ); }
    else if (!strcasecmp(path, "/send") && lib->Thread_run)
     { if ( msg->method != SOUP_METHOD_PUT )
        {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED); }
       else
				    { Imsgs_Envoi_message_to_all_available ( "Ceci est un test IMSG" );
          soup_message_set_status (msg, SOUP_STATUS_OK);
        }
     }
    else soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
