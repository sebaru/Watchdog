/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_dmx.c        Gestion des responses Admin MODBUS au serveur watchdog                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       dim. 05 sept. 2010 12:01:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_dmx.c
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
 #include "Dmx.h"
 extern struct DMX_CONFIG Cfg_dmx;
/******************************************************************************************************************************/
/* Admin_json_list : fonction appelée pour lister les modules dmx                                                             */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_dmx_status ( struct PROCESS *Lib, SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_GET)
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

    if (Lib->Thread_run)                                      /* Warning : Cfg_smsg does not exist if thread is not running ! */
     { Json_node_add_string ( RootNode, "tech_id", Cfg_dmx.tech_id );
       Json_node_add_string ( RootNode, "device", Cfg_dmx.device );
       Json_node_add_int    ( RootNode, "nbr_request", Cfg_dmx.nbr_request );
       Json_node_add_int    ( RootNode, "taille_trame_dmx", Cfg_dmx.taille_trame_dmx );
       Json_node_add_bool   ( RootNode, "comm", Cfg_dmx.comm_status );

       if (Cfg_dmx.Canal)
        { for (gint cpt=0; cpt<64; cpt++)
           { gchar canal[12];
             g_snprintf( canal, sizeof(canal), "canal_%d", cpt+1 );
             Json_node_add_int ( RootNode, canal, Cfg_dmx.Trame_dmx.channel[cpt] ); /*Canal[cpt].val_avant_ech );*/
           }
        }
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
 void Admin_json ( struct PROCESS *lib, SoupMessage *msg, const char *path, GHashTable *query, gint access_level )
  { if (access_level < 6)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }
         if (!strcasecmp(path, "/status"))   { Admin_json_dmx_status ( lib, msg ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
