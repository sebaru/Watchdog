/******************************************************************************************************************************/
/* Watchdogd/HttpMobile/admin_http.c  Gestion des responses Admin du thread "Http" de watchdog                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_http.c
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
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;
/******************************************************************************************************************************/
/* Admin_http_status: Print le statut du thread HTTP                                                                          */
/* EntrÃ©e : les adresses d'un buffer json et un entier pour sortir sa taille                                                 */
/* Sortie : les parametres d'entrÃ©e sont mis Ã  jour                                                                         */
/******************************************************************************************************************************/
 static void Admin_Http_status ( struct LIBRAIRIE *Lib, SoupMessage *msg )
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
     { Json_add_int    ( builder, "tcp_port",                 Cfg_http.tcp_port );
       Json_add_int    ( builder, "ssl_enable",               Cfg_http.ssl_enable );
       Json_add_string ( builder, "ssl_cert_filepath",        Cfg_http.ssl_cert_filepath );
       Json_add_string ( builder, "ssl_private_key_filepath", Cfg_http.ssl_private_key_filepath );
       pthread_mutex_lock( &Cfg_http.lib->synchro );
       Json_add_int    ( builder, "Abonnes_motifs",           g_slist_length (Cfg_http.liste_ws_motifs_clients) );
       Json_add_int    ( builder, "Abonnes_msgs",             g_slist_length (Cfg_http.liste_ws_msgs_clients) );
       Json_add_int    ( builder, "nbr_sessions",             g_slist_length (Cfg_http.liste_http_clients ) );
       pthread_mutex_unlock( &Cfg_http.lib->synchro );
     }
    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelÃ© par le thread http lors d'une requete /run/                                                  */
/* EntrÃ©e : les adresses d'un buffer json et un entier pour sortir sa taille                                                 */
/* Sortie : les parametres d'entrée sont mis Ã  jour                                                                          */
/******************************************************************************************************************************/
 void Admin_json ( struct LIBRAIRIE *lib, SoupMessage *msg, const char *path, GHashTable *query, gint access_level )
  { if (access_level < 6)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
    if (!strcasecmp(path, "/status")) { Admin_Http_status ( lib, msg ); }
    else soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
