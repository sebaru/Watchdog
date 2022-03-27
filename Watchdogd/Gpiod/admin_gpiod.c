/******************************************************************************************************************************/
/* Watchdogd/Gpiod/admin_raspberryPI.c        Gestion des connexions Gpiod pour watchdog                                      */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    01.10.2021 21:52:58 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_raspberryPI.c
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

 #include <glib.h>
 #include "watchdogd.h"
 #include "Gpiod.h"

/******************************************************************************************************************************/
/* Admin_json_gpiod_status : fonction appelée pour vérifier le status de la librairie                                         */
/* Entrée : un JSon Builder                                                                                                   */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_gpiod_status ( struct PROCESS *Lib, SoupMessage *msg )
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
    SQL_Select_to_json_node ( RootNode, "gpios", "SELECT * FROM gpiod_io WHERE uuid='%s' ORDER BY num", Lib->uuid );

    if (Lib->Thread_run)                                      /* Warning : Cfg_meteo does not exist if thread is not running ! */
     { /*Json_node_add_string ( RootNode, "tech_id", Cfg.tech_id );*/
     }
    gchar *buf = Json_node_to_string ( RootNode );
    Json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Admin_json_gpiod_set: Configure le thread GPIOD                                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_gpiod_set ( struct PROCESS *Lib, SoupMessage *msg )
  { if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "id" ) &&
            Json_has_member ( request, "mode_inout" ) && Json_has_member ( request, "mode_activelow" ) &&
            Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" )
           ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       Json_node_unref(request);
       return;
     }

    gint id             = Json_get_int( request, "id" );
    gint mode_inout     = Json_get_int( request, "mode_inout" );
    gint mode_activelow = Json_get_int( request, "mode_activelow" );
    gchar *tech_id      = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    gchar *acronyme     = Normaliser_chaine ( Json_get_string ( request, "acronyme" ) );
    Json_node_unref(request);

    SQL_Write_new ( "UPDATE gpiod_io SET tech_id=NULL, acronyme=NULL "
                    "WHERE tech_id='%s', acronyme='%s'", tech_id, acronyme );

    SQL_Write_new ( "UPDATE gpiod_io SET mode_inout='%d', mode_activelow='%d', tech_id='%s', acronyme='%s' "
                    "WHERE id=%d", mode_inout, mode_activelow, tech_id, acronyme, id );
    g_free(tech_id);
    g_free(acronyme);

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
         if (!strcasecmp(path, "/status")) { Admin_json_gpiod_status ( lib, msg ); }
    else if (!strcasecmp(path, "/set"))    { Admin_json_gpiod_set ( lib, msg ); }
    else soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
