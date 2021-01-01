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
/* Admin_json_smsg_list: Liste les parametres de bases de données associés au thread SMSG                                     */
/* Entrée : Le message libsoup                                                                                                */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_smsg_list ( struct LIBRAIRIE *Lib, SoupMessage *msg )
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

    SQL_Select_to_JSON_new ( builder, "gsms", "SELECT instance, tech_id, description FROM %s", NOM_THREAD );
    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
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
  { if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "description" ) &&
            Json_has_member ( request, "ovh_service_name" ) && Json_has_member ( request, "ovh_application_key" ) &&
            Json_has_member ( request, "ovh_consumer_key" ) && Json_has_member ( request, "ovh_application_secret" )
           ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id                = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *description            = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *ovh_service_name       = Normaliser_chaine ( Json_get_string( request, "ovh_service_name" ) );
    gchar *ovh_application_key    = Normaliser_chaine ( Json_get_string( request, "ovh_application_key" ) );
    gchar *ovh_application_secret = Normaliser_chaine ( Json_get_string( request, "ovh_application_secret" ) );
    gchar *ovh_consumer_key       = Normaliser_chaine ( Json_get_string( request, "ovh_consumer_key" ) );

    SQL_Write_new ( "UPDATE %s SET tech_id='%s', description='%s', ovh_service_name='%s', ovh_application_key='%s',"
                    "ovh_application_secret='%s', ovh_consumer_key='%s' "
                    "WHERE instance='%s'", NOM_THREAD,
                    tech_id, description, ovh_service_name, ovh_application_key, ovh_application_secret, ovh_consumer_key, g_get_host_name() );
    json_node_unref(request);
    g_free(tech_id);
    g_free(description);
    g_free(ovh_service_name);
    g_free(ovh_application_key);
    g_free(ovh_application_secret);
    g_free(ovh_consumer_key);
    soup_message_set_status (msg, SOUP_STATUS_OK);
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
    else if (!strcasecmp(path, "/list"))     { Admin_json_smsg_list   ( lib, msg ); }
    else if (!strcasecmp(path, "/set"))      { Admin_json_smsg_set ( lib, msg ); }
    else if (!strcasecmp(path, "/send") && lib->Thread_run)
     { if ( msg->method != SOUP_METHOD_PUT )
        {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED); }
       else
        { JsonNode *request = Http_Msg_to_Json ( msg );
          if (!request) return;
          if ( !Json_has_member ( request, "mode" ) )
           { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres"); }
          else if (!strcasecmp ( Json_get_string ( request, "mode" ), "OVH" ))
           { Cfg_smsg.send_test_OVH = TRUE;
             soup_message_set_status (msg, SOUP_STATUS_OK);
           }
          else if (!strcasecmp ( Json_get_string ( request, "mode" ), "GSM" ))
           { Cfg_smsg.send_test_GSM = TRUE;
             soup_message_set_status (msg, SOUP_STATUS_OK);
           }
          json_node_unref(request);
        }
     }
    else soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
