/******************************************************************************************************************************/
/* Watchdogd/Http/install.c       Gestion des request install pour le thread HTTP de watchdog                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    20.09.2020 00:44:17 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * install.c
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

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <pwd.h>
 #include <grp.h>
/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;

/******************************************************************************************************************************/
/* Http_Traiter_install: Traite l'installation du système                                                                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_install ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar fichier[80], home[128], chaine[256], *welcome =
          "#Welcome, your instance is now installed !\n"
          "#Sébastien Lefèvre - Abls-Habitat.fr\n";
    GBytes *request_brute;
    struct stat stat_buf;
    struct passwd *pwd;
    gsize taille;

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    g_snprintf ( fichier, sizeof(fichier), "/etc/watchdogd.abls.conf" );
    if (stat (fichier, &stat_buf)!=-1)                   /* Si pas d'erreur et fichier présent, c'est que c'est deja installé */
     { soup_message_set_status_full ( msg, SOUP_STATUS_FORBIDDEN, "Already Installed" );
       return;
     }

    if (getuid()!=0)
     { soup_message_set_status_full ( msg, SOUP_STATUS_FORBIDDEN, "Not Running As ROOT" );
       return;
     }

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if (!request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request !");
       return;
     }

    if ( ! (    Json_has_member ( request, "description" )
             && Json_has_member ( request, "db_username" )
             && Json_has_member ( request, "db_hostname" )
             && Json_has_member ( request, "db_database" )
             && Json_has_member ( request, "db_password" )
             && Json_has_member ( request, "use_subdir" )
             && Json_has_member ( request, "is_master" )
             && Json_has_member ( request, "master_host" )
             && Json_has_member ( request, "run_as" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    if (!g_str_is_ascii (Json_get_string(request, "run_as")))
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Wrong Run_AS");
       return;
     }

    g_snprintf( chaine, sizeof(chaine), "useradd -m -c 'WatchdogServer' %s", Json_get_string(request, "run_as") );
    system(chaine);
    g_snprintf( chaine, sizeof(chaine), "usermod -a -G audio,dialout %s", Json_get_string(request, "run_as") );
    system(chaine);

    pwd = getpwnam ( Json_get_string(request, "run_as" ) );
    if (!pwd)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Wrong Run_AS");
       return;
     }

    g_snprintf( home, sizeof(home), "%s", pwd->pw_dir );
    if (Json_get_int(request, "use_subdir")) { g_strlcat( home, "/.watchdog", sizeof(home) ); }

    mkdir ( home, S_IRUSR | S_IWUSR | S_IXUSR );
    chown ( home, pwd->pw_uid, pwd->pw_gid );
    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Created Home '%s' directory'", __func__, home );

    g_snprintf( chaine, sizeof(chaine), "%s/Dls", home );
    mkdir ( chaine, S_IRUSR | S_IWUSR | S_IXUSR );
    chown ( chaine, pwd->pw_uid, pwd->pw_gid );
    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Created Dls '%s' directory'", __func__, chaine );

    g_snprintf( chaine, sizeof(chaine), "%s/Upload", home );
    mkdir ( chaine, S_IRUSR | S_IWUSR | S_IXUSR );
    chown ( chaine, pwd->pw_uid, pwd->pw_gid );
    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Created Upload '%s' directory'", __func__, chaine );

/******************************************* Test accès Database **************************************************************/
    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Loading DB Schema'", __func__ );
    gchar *DB_SCHEMA = "/usr/local/share/Watchdog/init_db.sql";
    if (stat ( DB_SCHEMA, &stat_buf)==-1)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Stat DB Schema Error" );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Stat DB Schema Error", __func__ );
       return;
     }

    gchar *db_schema = g_try_malloc0 ( stat_buf.st_size+1 );
    if (!db_schema)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory DB Schema Error" );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Memory DB Schema Error", __func__ );
       return;
     }

    gint fd = open ( DB_SCHEMA, O_RDONLY );
    if (!fd)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Open DB Schema Error" );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Open DB Schema Error", __func__ );
       g_free(db_schema);
       return;
     }
    if (read ( fd, db_schema, stat_buf.st_size ) != stat_buf.st_size)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Read DB Schema Error" );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Read DB Schema Error", __func__ );
       g_free(db_schema);
       return;
     }
    close(fd);

    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: DB Schema Loaded. Connecting to DB.'", __func__ );
    struct DB *db = Init_DB_SQL_with ( Json_get_string(request, "db_hostname"), Json_get_string(request, "db_username"),
                                       Json_get_string(request, "db_password"), Json_get_string(request, "db_database"),
                                       Json_get_int(request, "db_port" ), TRUE );

    if (!db)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "DB Connect Error");
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: DB Connect Error", __func__ );
       g_free(db_schema);
       return;
     }
    Lancer_requete_SQL ( db, db_schema );                                                               /* Création du schéma */
    g_free(db_schema);
    Liberer_resultat_SQL ( db );
    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: DB Schema OK. Starting update.", __func__ );

    g_snprintf( chaine, sizeof(chaine),
               "INSERT INTO config SET instance_id='%s',nom_thread='msrv',"
               "nom='instance_is_master',valeur='%s' ", g_get_host_name(), (Json_get_int(request,"is_master") ? "true" : "false") );
    Lancer_requete_SQL ( db, chaine );

    gchar *master_host = Normaliser_chaine ( Json_get_string(request,"master_host") );
    g_snprintf( chaine, sizeof(chaine),
               "INSERT INTO config SET instance_id='%s',nom_thread='msrv',"
               "nom='master_host',valeur='%s' ", g_get_host_name(), master_host );
    Lancer_requete_SQL ( db, chaine );
    g_free(master_host);

    gchar *description = Normaliser_chaine ( Json_get_string(request,"description") );
    g_snprintf( chaine, sizeof(chaine),
               "INSERT INTO config SET instance_id='%s',nom_thread='msrv',"
               "nom='description',valeur='%s' ", g_get_host_name(), description );
    Lancer_requete_SQL ( db, chaine );
    g_snprintf( chaine, sizeof(chaine), "UPDATE syns SET libelle='%s' WHERE id='1'", description );
    Lancer_requete_SQL ( db, chaine );
    g_free(description);

    g_snprintf( chaine, sizeof(chaine),
               "INSERT INTO config SET instance_id='%s',nom_thread='msrv',"
               "nom='subdir',valeur='%s' ", g_get_host_name(), (Json_get_int(request,"use_subdir") ? "true" : "false") );
    Lancer_requete_SQL ( db, chaine );
    g_snprintf( chaine, sizeof(chaine),
               "INSERT INTO config SET instance_id='%s',nom_thread='msrv',nom='log_level',valeur='6' ", g_get_host_name() );
    Lancer_requete_SQL ( db, chaine );
    g_snprintf( chaine, sizeof(chaine),
               "INSERT INTO config SET instance_id='%s',nom_thread='msrv',nom='log_db',valeur='0' ", g_get_host_name() );
    Lancer_requete_SQL ( db, chaine );
    g_snprintf( chaine, sizeof(chaine),
               "INSERT INTO config SET instance_id='%s',nom_thread='msrv',nom='log_zmq',valeur='0' ", g_get_host_name() );
    Lancer_requete_SQL ( db, chaine );
    Libere_DB_SQL ( &db );
/******************************************* Création fichier de config *******************************************************/
    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Creating config file '%s'", __func__, fichier );

    fd = creat ( fichier, S_IRUSR | S_IWUSR );
    if (fd==-1)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "File Create Error");
       return;
     }
    write (fd, welcome, strlen(welcome) );

    g_snprintf(chaine, sizeof(chaine), "\n[GLOBAL]\n" );
    write (fd, chaine, strlen(chaine) );
    g_snprintf(chaine, sizeof(chaine), "run_as = %s\n", Json_get_string(request, "run_as") );
    write (fd, chaine, strlen(chaine) );

    g_snprintf(chaine, sizeof(chaine), "\n[DATABASE]\n" );
    write (fd, chaine, strlen(chaine) );
    g_snprintf(chaine, sizeof(chaine), "username = %s\n", Json_get_string(request, "db_username") );
    write (fd, chaine, strlen(chaine) );
    g_snprintf(chaine, sizeof(chaine), "hostname = %s\n", Json_get_string(request, "db_hostname") );
    write (fd, chaine, strlen(chaine) );
    g_snprintf(chaine, sizeof(chaine), "database = %s\n", Json_get_string(request, "db_database") );
    write (fd, chaine, strlen(chaine) );
    g_snprintf(chaine, sizeof(chaine), "password = %s\n", Json_get_string(request, "db_password") );
    write (fd, chaine, strlen(chaine) );
    close(fd);
    json_node_unref(request);

    Partage->com_msrv.Thread_run = FALSE;                                                    /* On reboot toute la baraque !! */
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
