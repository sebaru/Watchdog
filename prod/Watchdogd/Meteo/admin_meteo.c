/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_sms.c        Gestion des responses Admin SET au serveur watchdog                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        sam. 19 mai 2012 11:03:52 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_sms.c
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

 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"
 #include "Meteo.h"
 extern struct METEO_CONFIG Cfg_meteo;

/******************************************************************************************************************************/
/* Admin_json_status : fonction appelée pour vérifier le status de la librairie                                               */
/* Entrée : un JSon Builder                                                                                                   */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_smsg_status ( struct LIBRAIRIE *Lib, SoupMessage *msg )
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
     { Json_node_add_string ( RootNode, "tech_id", Cfg_meteo.tech_id );
       Json_node_add_string ( RootNode, "description", Cfg_meteo.description );
       Json_node_add_string ( RootNode, "token", Cfg_meteo.token );
       Json_node_add_string ( RootNode, "code_insee", Cfg_meteo.code_insee );
     }
    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Admin_json_smsg_set: Configure le thread SMSG                                                                              */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_smsg_set ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "description" ) &&
            Json_has_member ( request, "token" ) && Json_has_member ( request, "code_insee" )
           ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id     = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    Modifier_configDB ( NOM_THREAD, "tech_id", tech_id );
    g_free(tech_id);

    gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );
    Modifier_configDB ( NOM_THREAD, "description", description );
    g_free(description);

    gchar *token       = Normaliser_chaine ( Json_get_string( request, "token" ) );
    Modifier_configDB ( NOM_THREAD, "token", token );
    g_free(token);

    gchar *code_insee  = Normaliser_chaine ( Json_get_string( request, "code_insee" ) );
    Modifier_configDB ( NOM_THREAD, "code_insee", code_insee );
    g_free(code_insee);

    json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
    Lib->Thread_reload = TRUE;
    while ( Lib->Thread_run == TRUE && Lib->Thread_reload == TRUE);                              /* Attente reboot du process */
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
         if (!strcasecmp(path, "/status"))   { Admin_json_smsg_status ( lib, msg ); }
    else if (!strcasecmp(path, "/set"))      { Admin_json_smsg_set ( lib, msg ); }
    else if (!strcasecmp(path, "/test") && lib->Thread_run)
     { if ( msg->method != SOUP_METHOD_PUT )
        {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED); }
       else
        { soup_message_set_status (msg, SOUP_STATUS_OK);
          Cfg_meteo.test_api = TRUE;
        }
     }
    else soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
