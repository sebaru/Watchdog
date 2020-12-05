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
 #include "Sms.h"
 extern struct SMS_CONFIG Cfg_smsg;
/******************************************************************************************************************************/
/* Admin_json_status : fonction appelée pour vérifier le status de la librairie                                               */
/* Entrée : un JSon Builder                                                                                                   */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_smsg_status ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_bool ( builder, "thread_is_running", Lib->Thread_run );

    if (Lib->Thread_run)                                      /* Warning : Cfg_smsg does not exist if thread is not running ! */
     { Json_add_string ( builder, "tech_id", Cfg_smsg.tech_id );
       Json_add_string ( builder, "ovh_service_name", Cfg_smsg.ovh_service_name );
       Json_add_string ( builder, "ovh_application_key", Cfg_smsg.ovh_application_key );
       Json_add_string ( builder, "ovh_application_secret", Cfg_smsg.ovh_application_secret );
       Json_add_string ( builder, "ovh_consumer_key", Cfg_smsg.ovh_consumer_key );
       Json_add_string ( builder, "description", Cfg_smsg.description );
       Json_add_bool   ( builder, "comm_status", Cfg_smsg.comm_status );
       Json_add_int    ( builder, "nbr_sms", Cfg_smsg.nbr_sms );
     }
    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Admin_json_smsg_set: Configure le thread SMSG                                                                              */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_smsg_set ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { GBytes *request_brute;
    gsize taille;

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No Request");
       return;
     }

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "description" ) &&
            Json_has_member ( request, "ovh_service_name" ) && Json_has_member ( request, "ovh_application_key" ) &&
            Json_has_member ( request, "ovh_consumer_key" ) && Json_has_member ( request, "ovh_application_secret" )
           ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    Modifier_configDB ( NOM_THREAD, "tech_id", Json_get_string( request, "tech_id" ) );
    Modifier_configDB ( NOM_THREAD, "description", Json_get_string( request, "description" ) );
    Modifier_configDB ( NOM_THREAD, "ovh_service_name", Json_get_string( request, "ovh_service_name" ) );
    Modifier_configDB ( NOM_THREAD, "ovh_application_key", Json_get_string( request, "ovh_application_key" ) );
    Modifier_configDB ( NOM_THREAD, "ovh_application_secret", Json_get_string( request, "ovh_application_secret" ) );
    Modifier_configDB ( NOM_THREAD, "ovh_consumer_key", Json_get_string( request, "ovh_consumer_key" ) );
    json_node_unref(request);

    soup_message_set_status (msg, SOUP_STATUS_OK);
    Lib->Thread_reload = TRUE;
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
    else if (!strcasecmp(path, "/send") && lib->Thread_run)
     { Cfg_smsg.send_test = TRUE;
       soup_message_set_status (msg, SOUP_STATUS_OK);
     }
    else soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
