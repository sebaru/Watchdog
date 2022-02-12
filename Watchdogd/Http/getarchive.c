/******************************************************************************************************************************/
/* Watchdogd/Http/getarchive.c       Gestion des requests sur l'URI /archive du webservice                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    06.05.2020 10:57:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getarchive.c
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

 #include <string.h>
 #include <unistd.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"

/******************************************************************************************************************************/
/* Http_traiter_archive_testdb: Test la response vers le serveur de base de données                                           */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_traiter_archive_testdb ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { gchar chaine[512];
    struct DB *db;

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6)) return;

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
/* Http_traiter_archive_clear: Supprime tous les enregistrements dans le tampon d'attente                                     */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_traiter_archive_clear ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6)) return;

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
/* Http_traiter_archive_purge: Lance le thread de purge des archives                                                          */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_traiter_archive_purge ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { pthread_t tid;

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6)) return;

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
/* Http_traiter_archive_del: Delete une table d'archivage                                                                     */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_archive_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { gchar requete[256];
    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6)) return;
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
/* Http_traiter_archive_set: Configure le thread d'archivage                                                                  */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_archive_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6)) return;
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
/* Http_traiter_archive_status: Fourni le statut du thread d'archivage                                                        */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_archive_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6)) return;

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
/* Http_traiter_archive_table_status: Fourni une list JSON du status des tables d'archivages                                  */
/* Entrée: le Message Soup                                                                                                    */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_archive_table_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                          SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6)) return;

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
/* Http_traiter_archive_get: Fourni une list JSON des elements d'archive                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_archive_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { gchar *requete = NULL, chaine[512], *interval, nom_courbe[12];
    gint nbr;

    if (msg->method != SOUP_METHOD_PUT || Config.instance_is_master == FALSE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0)) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "period" ) && Json_has_member ( request, "courbes" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    gchar *period   = Normaliser_chaine ( Json_get_string ( request, "period" ) );
    gint periode = 450;
    interval = " ";
         if (!strcasecmp(period, "HOUR"))  { periode = 150;   interval = " WHERE date_time>=NOW() - INTERVAL 4 HOUR"; }
    else if (!strcasecmp(period, "DAY"))   { periode = 450;   interval = " WHERE date_time>=NOW() - INTERVAL 2 DAY"; }
    else if (!strcasecmp(period, "WEEK"))  { periode = 3600;  interval = " WHERE date_time>=NOW() - INTERVAL 2 WEEK"; }
    else if (!strcasecmp(period, "MONTH")) { periode = 43200; interval = " WHERE date_time>=NOW() - INTERVAL 9 WEEK"; }
    else if (!strcasecmp(period, "YEAR"))  { periode = 86400; interval = " WHERE date_time>=NOW() - INTERVAL 13 MONTH"; }
    g_free(period);

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       json_node_unref(request);
       return;
     }

    gint taille_requete = 32;
    requete = g_try_malloc(taille_requete);
    if (!requete)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       json_node_unref(request);
       return;
     }

    g_snprintf( requete, taille_requete, "SELECT * FROM ");

    int nbr_courbe = json_array_get_length ( Json_get_array ( request, "courbes" ) );
    for (nbr=0; nbr<nbr_courbe; nbr++)
     { g_snprintf( nom_courbe, sizeof(nom_courbe), "courbe%d", nbr+1 );

       JsonNode *courbe = json_array_get_element ( Json_get_array ( request, "courbes" ), nbr );
       gchar *tech_id  = Normaliser_chaine ( Json_get_string ( courbe, "tech_id" ) );
       gchar *acronyme = Normaliser_chaine ( Json_get_string ( courbe, "acronyme" ) );

       g_snprintf( chaine, sizeof(chaine),
                  "%s "
                  "(SELECT FROM_UNIXTIME((UNIX_TIMESTAMP(date_time) DIV %d)*%d) AS date, COALESCE(ROUND(AVG(valeur),3),0) AS moyenne%d "
                  " FROM histo_bit_%s_%s %s GROUP BY date ORDER BY date) AS %s "
                  "%s ",
                  (nbr!=0 ? "INNER JOIN" : ""), periode, periode, nbr+1, tech_id, acronyme, interval, nom_courbe,
                  (nbr!=0 ? "USING(date)" : "") );

       taille_requete += strlen(chaine)+1;
       requete = g_try_realloc ( requete, taille_requete );
       if (requete) g_strlcat ( requete, chaine, taille_requete );

       JsonNode *json_courbe = Json_node_add_objet ( RootNode, nom_courbe );
       g_snprintf(chaine, sizeof(chaine), "SELECT * FROM dictionnaire WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme );
       SQL_Select_to_json_node ( json_courbe, NULL, chaine );

       g_free(tech_id);
       g_free(acronyme);
     }

    if (SQL_Arch_to_json_node ( RootNode, "valeurs", requete ) == FALSE)
     { g_free(requete);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       json_node_unref(request);
       json_node_unref(RootNode);
       return;
     }

    g_free(requete);
    json_node_unref(request);

    gchar *buf = Json_node_to_string (RootNode);
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
