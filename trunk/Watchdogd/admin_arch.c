/******************************************************************************************************************************/
/* Watchdogd/Archive/admin_arch.c  Gestion des responses Admin du thread "Archive" de watchdog                                */
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
/* Admin_arch_testdb: Test la response vers le serveur de base de données                                                     */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_testdb ( SoupMessage *msg )
  { gchar chaine[512];
    struct DB *db;

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    db = Init_ArchDB_SQL();
    g_snprintf( chaine, sizeof(chaine), "Connection '%s' (Host='%s':%d, User='%s' DB='%s')", (db ? "OK" : "Failed"),
                Partage->com_arch.archdb_hostname, Partage->com_arch.archdb_port,
                Partage->com_arch.archdb_username, Partage->com_arch.archdb_database );
    Json_node_add_bool   ( RootNode, "result", (db ? TRUE : FALSE) );
    Json_node_add_string ( RootNode, "details", chaine );
    Libere_DB_SQL ( &db );

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Admin_arch_json_clear: Supprime tous les enregistrements dans le tampon d'attente                                          */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_clear ( SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    gint nbr = Arch_Clear_list();
    Json_node_add_int ( RootNode, "nbr_archive_deleted", nbr );

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Admin_arch_json_purge: Lance le thread de purge des archives                                                               */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_purge ( SoupMessage *msg )
  { pthread_t tid;

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (pthread_create( &tid, NULL, (void *)Arch_Update_SQL_Partitions_thread, NULL ))
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s: pthread_create failed for Update SQL Partitions", __func__ );
       Json_node_add_string ( RootNode, "exec_purge_thread", "failed" );
     }
    else
     { pthread_detach( tid );                                        /* On le detache pour qu'il puisse se terminer tout seul */
       Json_node_add_string ( RootNode, "exec_purge_thread", "success" );
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_instance_list: Fourni une list JSON des instances Watchdog dans le domaine                                    */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_arch_del ( SoupMessage *msg )
  { gchar requete[256];
    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

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
/* Entrée: le Message Soup                                                                                                    */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_arch_set ( SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( Json_has_member ( request, "hostname" ) )
     { g_snprintf ( Partage->com_arch.archdb_hostname, sizeof(Partage->com_arch.archdb_hostname), "%s",
                    Json_get_string(request, "hostname"));
       Modifier_configDB ( "archive", "hostname", Partage->com_arch.archdb_hostname );
     }

    if ( Json_has_member ( request, "port" ) )
     { Partage->com_arch.archdb_port = Json_get_int(request,"port");
       Modifier_configDB_int ( "archive", "port", Partage->com_arch.archdb_port );
     }

    if ( Json_has_member ( request, "username" ) )
     { g_snprintf ( Partage->com_arch.archdb_username, sizeof(Partage->com_arch.archdb_username), "%s",
                    Json_get_string(request, "username"));
       Modifier_configDB ( "archive", "username", Partage->com_arch.archdb_username );
     }

    if ( Json_has_member ( request, "password" ) )
     { g_snprintf ( Partage->com_arch.archdb_password, sizeof(Partage->com_arch.archdb_password), "%s",
                    Json_get_string(request, "password"));
       Modifier_configDB ( "archive", "password", Partage->com_arch.archdb_password );
     }

    if ( Json_has_member ( request, "database" ) )
     { g_snprintf ( Partage->com_arch.archdb_database, sizeof(Partage->com_arch.archdb_database), "%s",
                    Json_get_string(request, "database"));
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
/* Entrée: le Message Soup                                                                                                    */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_arch_status ( SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_node_add_bool   ( RootNode, "thread_is_running", Partage->com_arch.Thread_run );
    Json_node_add_string ( RootNode, "hostname",    Partage->com_arch.archdb_hostname);
    Json_node_add_int    ( RootNode, "port",        Partage->com_arch.archdb_port);
    Json_node_add_string ( RootNode, "database",    Partage->com_arch.archdb_database);
    Json_node_add_string ( RootNode, "username",    Partage->com_arch.archdb_username);
    Json_node_add_int    ( RootNode, "buffer_size", Partage->com_arch.buffer_size);
    Json_node_add_int    ( RootNode, "retention",   Partage->com_arch.retention);

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_instance_list: Fourni une list JSON des instances Watchdog dans le domaine                                    */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_arch_table_status ( SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Arch_to_json_node ( RootNode, "tables",
                            "SELECT table_name, table_rows, update_time FROM information_schema.tables WHERE table_schema='%s' "
                            "AND table_name like 'histo_bit_%%'", Partage->com_arch.archdb_database );

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée: le Message Soup                                                                                                    */
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
