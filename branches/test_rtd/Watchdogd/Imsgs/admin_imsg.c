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
 extern struct IMSGS_CONFIG Cfg_imsgs;
/******************************************************************************************************************************/
/* Admin_json_status : fonction appelée pour vérifier le status de la librairie                                               */
/* Entrée : un JSon Builder                                                                                                   */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_imsg_status ( struct LIBRAIRIE *Lib, SoupMessage *msg )
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

    if (Lib->Thread_run)                                      /* Warning : Cfg_meteo does not exist if thread is not running ! */
     { /*Json_node_add_string ( RootNode, "tech_id", Cfg_meteo.tech_id );*/
     }
    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 void Admin_json ( struct LIBRAIRIE *lib, SoupMessage *msg, const char *path, GHashTable *query, gint access_level )
  { if (access_level < 6)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }
         if (!strcasecmp(path, "/status")) { Admin_json_imsg_status ( lib, msg ); }
    else if (!strcasecmp(path, "/send/" ) )
				 { Imsgs_Envoi_message_to_all_available ( path+6 );
     }
    else soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/