/******************************************************************************************************************************/
/* Watchdogd/Archive/admin_arch.c  Gestion des responses Admin du thread "Archive" de watchdog                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    17.03.2017 08:37:09 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_arch.c
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

/******************************************************************************************************************************/
/* Admin_arch_testdb: Test la response vers le serveur de base de données                                                    */
/* Entrée: la response pour sortiee client et la ligne de commande                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_testdb ( SoupMessage *msg )
  { JsonBuilder *builder;
    gchar *buf, chaine[512];
    gsize taille_buf;
    struct DB *db;

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    db = Init_ArchDB_SQL();
    g_snprintf( chaine, sizeof(chaine), "Connection '%s' (Host='%s':%d, User='%s' DB='%s')", (db ? "OK" : "Failed"),
                Partage->com_arch.archdb_hostname, Partage->com_arch.archdb_port, Partage->com_arch.archdb_username, Partage->com_arch.archdb_database );
    Json_add_bool   ( builder, "result", (db ? TRUE : FALSE) );
    Json_add_string ( builder, "details", chaine );
    Libere_DB_SQL ( &db );

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Admin_arch_json_clear: Supprime tous les enregistrements dans le tampon d'attente                                          */
/* Entrée: le JSON builder pour préparer la réponse au client                                                                 */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_clear ( SoupMessage *msg )
  { JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf;

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    gint nbr = Arch_Clear_list();
    Json_add_int ( builder, "nbr_archive_deleted", nbr );

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Admin_arch_json_purge: Lance le thread de purge des archives                                                               */
/* Entrée: le JSON builder pour préparer la réponse au client                                                                 */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_purge ( SoupMessage *msg )
  { JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf;
    pthread_t tid;

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (pthread_create( &tid, NULL, (void *)Arch_Update_SQL_Partitions_thread, NULL ))
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s: pthread_create failed for Update SQL Partitions", __func__ );
       Json_add_string ( builder, "exec_purge_thread", "failed" );
     }
    else
     { pthread_detach( tid );                                        /* On le detache pour qu'il puisse se terminer tout seul */
       Json_add_string ( builder, "exec_purge_thread", "success" );
     }

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_instance_list: Fourni une list JSON des instances Watchdog dans le domaine                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_arch_del ( SoupMessage *msg )
  { GBytes *request_brute;
    gsize taille;
    gchar requete[256];
    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No Request");
       return;
     }

    if ( ! (Json_has_member ( request, "table_name" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    if (!g_str_has_prefix ( Json_get_string( request, "table_name" ), "histo_bit_" ))
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvaise cible");
       return;
     }

    gchar *table_name  = Normaliser_chaine ( Json_get_string( request, "table_name" ) );
    json_node_unref(request);

    g_snprintf( requete, sizeof(requete), "DROP TABLE %s", table_name );
    if (SQL_Arch_Write (requete)) { soup_message_set_status (msg, SOUP_STATUS_OK); }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
  }
/******************************************************************************************************************************/
/* Admin_arch_set: Configure le thread d'archivage                                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_arch_set ( SoupMessage *msg )
  { GBytes *request_brute;
    gsize taille;

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

    if ( Json_has_member ( request, "hostname" ) )
     { g_snprintf ( Partage->com_arch.archdb_hostname, sizeof(Partage->com_arch.archdb_hostname), "%s", Json_get_string(request, "hostname"));
       Modifier_configDB ( "archive", "hostname", Partage->com_arch.archdb_hostname );
     }

    if ( Json_has_member ( request, "port" ) )
     { Partage->com_arch.archdb_port = Json_get_int(request,"port");
       Modifier_configDB_int ( "archive", "port", Partage->com_arch.archdb_port );
     }

    if ( Json_has_member ( request, "username" ) )
     { g_snprintf ( Partage->com_arch.archdb_username, sizeof(Partage->com_arch.archdb_username), "%s", Json_get_string(request, "username"));
       Modifier_configDB ( "archive", "username", Partage->com_arch.archdb_username );
     }

    if ( Json_has_member ( request, "password" ) )
     { g_snprintf ( Partage->com_arch.archdb_password, sizeof(Partage->com_arch.archdb_password), "%s", Json_get_string(request, "password"));
       Modifier_configDB ( "archive", "password", Partage->com_arch.archdb_password );
     }

    if ( Json_has_member ( request, "database" ) )
     { g_snprintf ( Partage->com_arch.archdb_database, sizeof(Partage->com_arch.archdb_database), "%s", Json_get_string(request, "database"));
       Modifier_configDB ( "archive", "database", Partage->com_arch.archdb_database );
     }

    if ( Json_has_member ( request, "buffer_size" ) )
     { Partage->com_arch.buffer_size = Json_get_int(request,"buffer_size");
       Modifier_configDB_int ( "archive", "buffer_size", Partage->com_arch.buffer_size );
     }

    if ( Json_has_member ( request, "retention" ) )
     { Partage->com_arch.retention = Json_get_int(request,"retention");
       Modifier_configDB_int ( "archive", "retention", Partage->com_arch.retention );
     }

    json_node_unref(request);
    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Admin_arch_status: Fourni le statut du thread d'archivage                                                                  */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_arch_status ( SoupMessage *msg )
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
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_bool   ( builder, "thread_is_running", Partage->com_arch.Thread_run );
    Json_add_string ( builder, "hostname",    Partage->com_arch.archdb_hostname);
    Json_add_int    ( builder, "port",        Partage->com_arch.archdb_port);
    Json_add_string ( builder, "database",    Partage->com_arch.archdb_database);
    Json_add_string ( builder, "username",    Partage->com_arch.archdb_username);
    Json_add_int    ( builder, "buffer_size", Partage->com_arch.buffer_size);
    Json_add_int    ( builder, "retention",   Partage->com_arch.retention);

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_instance_list: Fourni une list JSON des instances Watchdog dans le domaine                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_arch_table_status ( SoupMessage *msg )
  { JsonBuilder *builder;
    gchar *buf, requete[256];
    gsize taille_buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT table_name, table_rows FROM information_schema.tables WHERE table_schema='%s' "
                "AND table_name like 'histo_bit_%%'", Partage->com_arch.archdb_database );
    SQL_Arch_to_JSON ( builder, "tables", requete );

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 void Admin_arch_json ( SoupMessage *msg, const char *path, GHashTable *query, gint access_level )
  { if (access_level < 6)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }
         if (!strcasecmp(path, "status")) { Admin_arch_status ( msg ); }
    else if (!strcasecmp(path, "set"))    { Admin_arch_set   ( msg ); }
    else if (!strcasecmp(path, "table_status"))  { Admin_arch_table_status ( msg ); }
    else if (!strcasecmp(path, "del"))    { Admin_arch_del    ( msg ); }
    else if (!strcasecmp(path, "clear"))  { Admin_arch_clear  ( msg ); }
    else if (!strcasecmp(path, "purge"))  { Admin_arch_purge  ( msg ); }
    else if (!strcasecmp(path, "testdb")) { Admin_arch_testdb ( msg ); }
    else { soup_message_set_status_full (msg, SOUP_STATUS_NOT_IMPLEMENTED, "Command not found"); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
