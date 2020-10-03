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
       Json_add_string ( builder, "smsbox_apikey", Cfg_smsg.smsbox_apikey );
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
/* Admin_json_smsg_map_list: Liste tous les mappings SMS/GSM                                                                  */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static void Admin_json_smsg_map_list ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { JsonBuilder *builder;
    gchar *buf, chaine[512];
    gsize taille_buf;

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

    g_snprintf(chaine, sizeof(chaine), "SELECT * FROM mnemos_DI WHERE map_thread='SMSG'" );
    if (SQL_Select_to_JSON ( builder, "mappings", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
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

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "smsbox_apikey" ) &&
            Json_has_member ( request, "description" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id       = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *description   = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *smsbox_apikey = Normaliser_chaine ( Json_get_string( request, "smsbox_apikey" ) );
    json_node_unref(request);

    Modifier_configDB ( NOM_THREAD, "tech_id", tech_id );
    Modifier_configDB ( NOM_THREAD, "description", description );
    Modifier_configDB ( NOM_THREAD, "smsbox_apikey", smsbox_apikey );

    g_free(tech_id);
    g_free(description);
    g_free(smsbox_apikey);

    soup_message_set_status (msg, SOUP_STATUS_OK);
    Lib->Thread_reload = TRUE;
  }
/******************************************************************************************************************************/
/* Admin_json_smsg_list: Liste les GSM disponibles                                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_smsg_list ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { JsonBuilder *builder;
    gchar *buf, chaine[512];
    gsize taille_buf;

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

    g_snprintf(chaine, sizeof(chaine), "SELECT DISTINCT(m.map_tech_id) FROM mnemos_DI AS m WHERE map_thread='SMSG'" );
    if (SQL_Select_to_JSON ( builder, "gsms", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Admin_json_smsg_map_set: Ajoute un mapping SMS sur un bit interne                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static void Admin_json_smsg_map_set ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { GBytes *request_brute;
    gsize taille;
    gchar requete[512];

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No Request");
       return;
     }

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
            Json_has_member ( request, "map_tech_id" ) && Json_has_member ( request, "map_tag" )
           ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *map_tech_id = Normaliser_as_tech_id ( Json_get_string( request, "map_tech_id" ) );
    gchar *map_tag     = Normaliser_as_tech_id ( Json_get_string( request, "map_tag" ) );
    gchar *tech_id     = Normaliser_as_tech_id ( Json_get_string( request, "tech_id" ) );
    gchar *acronyme    = Normaliser_as_tech_id ( Json_get_string( request, "acronyme" ) );


    g_snprintf( requete, sizeof(requete),
                "UPDATE mnemos_DI SET map_thread=NULL, map_tech_id=NULL, map_tag=NULL "
                " WHERE map_tech_id='%s' AND map_tag='%s';", map_tech_id, map_tag );

    SQL_Write (requete);

    g_snprintf( requete, sizeof(requete),
                "UPDATE mnemos_DI SET map_thread='SMSG', map_tech_id='%s', map_tag='%s' "
                " WHERE tech_id='%s' AND acronyme='%s';", map_tech_id, map_tag, tech_id, acronyme );

    if (SQL_Write (requete))
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Lib->Thread_reload = TRUE;
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );

    json_node_unref(request);
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
    else if (!strcasecmp(path, "/map/list")) { Admin_json_smsg_map_list ( lib, msg ); }
    else if (!strcasecmp(path, "/map/set"))  { Admin_json_smsg_map_set ( lib, msg ); }
    else if (!strcasecmp(path, "/list"))     { Admin_json_smsg_list ( lib, msg ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
