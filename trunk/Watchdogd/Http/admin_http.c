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
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_Http_status ( JsonBuilder *builder )
  { Json_add_int    ( builder, "tcp_port",                 Cfg_http.tcp_port );
    Json_add_int    ( builder, "ssl_enable",               Cfg_http.ssl_enable );
    Json_add_string ( builder, "ssl_cert_filepath",        Cfg_http.ssl_cert_filepath );
    Json_add_string ( builder, "ssl_private_key_filepath", Cfg_http.ssl_private_key_filepath );
    Json_add_int    ( builder, "Abonnes_motifs",           g_slist_length (Cfg_http.liste_ws_motifs_clients) );
    Json_add_int    ( builder, "Abonnes_msgs",             g_slist_length (Cfg_http.liste_ws_msgs_clients) );
    Json_add_int    ( builder, "nbr_sessions",             g_slist_length (Cfg_http.liste_http_clients ) );
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 void Admin_json ( gchar *commande, gchar **buffer_p, gsize *taille_p )
{ JsonBuilder *builder;
    *buffer_p = NULL;
    *taille_p = 0;

    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
                                                                      /* Lancement de la requete de recuperation des messages */
    if (!strcmp(commande, "/status")) { Admin_Http_status ( builder ); }

/************************************************ Génération du JSON **********************************************************/
    *buffer_p = Json_get_buf (builder, taille_p);
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
