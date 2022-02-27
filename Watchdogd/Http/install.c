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
    struct stat stat_buf;
    struct passwd *pwd;
    gchar *db_schema;

    if (msg->method == SOUP_METHOD_GET)
     { SoupMessageHeaders *headers;
       g_object_get ( G_OBJECT(msg), "response_headers", &headers, NULL );

       gchar fichier[128];
       g_snprintf ( fichier, sizeof(fichier), "%s/install.html", WTD_PKGDATADIR );

       if (stat (fichier, &stat_buf)==-1)
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : File '%s' not found", __func__, fichier );
          soup_message_set_status_full ( msg, SOUP_STATUS_NOT_FOUND, "File not found" );
          return;
        }

       gint   taille_result = stat_buf.st_size;
       gchar *result        = g_try_malloc ( taille_result );
       if (!result)
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : File '%s' malloc error", __func__, fichier );
          soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
          return;
        }

       gint fd = open ( fichier, O_RDONLY );
       if (fd==-1)
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : File '%s' open error '%s'", __func__, fichier, strerror(errno) );
          g_free(result);
          soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "File Open Error" );
          return;
        }
       read ( fd, result, taille_result );
       close(fd);

       soup_message_headers_append ( headers, "cache-control", "private, max-age=86400" );
       soup_message_set_response ( msg, "text/html; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_result );
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }

    if (msg->method != SOUP_METHOD_POST)
     { soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
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

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

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

    gboolean is_master = Json_get_int(request,"is_master");
/******************************************* Creation du user *****************************************************************/
    g_snprintf( chaine, sizeof(chaine), "useradd -m -c 'WatchdogServer' %s", Json_get_string(request, "run_as") );
    system(chaine);
    g_snprintf( chaine, sizeof(chaine), "usermod -a -G audio,dialout,gpio %s", Json_get_string(request, "run_as") );
    system(chaine);

/******************************************* Creation du home *****************************************************************/
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

    if (is_master)
     { g_snprintf( chaine, sizeof(chaine), "%s/Dls", home );
       mkdir ( chaine, S_IRUSR | S_IWUSR | S_IXUSR );
       chown ( chaine, pwd->pw_uid, pwd->pw_gid );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Created Dls '%s' directory'", __func__, chaine );

       g_snprintf( chaine, sizeof(chaine), "%s/Upload", home );
       mkdir ( chaine, S_IRUSR | S_IWUSR | S_IXUSR );
       chown ( chaine, pwd->pw_uid, pwd->pw_gid );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Created Upload '%s' directory'", __func__, chaine );
     }
/******************************************* Test accès Database **************************************************************/
    if (is_master)
     { Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Loading DB Schema", __func__ );
       db_schema = SQL_Read_from_file ( "init_db.sql" );
       if (!db_schema)
        { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Read DB Schema Error" );
          Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Read DB Schema Error", __func__ );
          return;
        }
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: DB Schema Loaded. Connecting to DB.", __func__ );
     }

    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Connecting to DB.", __func__ );
    struct DB *db = Init_DB_SQL_with ( Json_get_string(request, "db_hostname"), Json_get_string(request, "db_username"),
                                       Json_get_string(request, "db_password"), Json_get_string(request, "db_database"),
                                       Json_get_int(request, "db_port" ), TRUE );

    if (!db)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "DB Connect Error");
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: DB Connect Error", __func__ );
       g_free(db_schema);
       return;
     }

    if (is_master)
     { Lancer_requete_SQL ( db, db_schema );                                                            /* Création du schéma */
       Liberer_resultat_SQL ( db );
       Info_new( Config.log, TRUE, LOG_NOTICE, "%s: DB Schema Init OK (Master).", __func__ );
     }
    else Info_new( Config.log, TRUE, LOG_NOTICE, "%s: DB Schema Not Initialize (instance Slave).", __func__ );

    gchar *master_host = Normaliser_chaine ( Json_get_string(request,"master_host") );
    gchar *description = Normaliser_chaine ( Json_get_string(request,"description") );
    SQL_Write_new ( "INSERT INTO instances SET instance='%s', is_master='%d', version='%s', start_time=NOW(), "
                    "debug=0, log_db=0, log_trad=0, log_zmq=0, log_level=6,"
                    "master_host='%s', description='%s', use_subdir='%d'",
                    g_get_host_name(), is_master, WTD_VERSION, master_host, description, (Json_get_int(request,"use_subdir") ? "true" : "false") );
    g_free(master_host);
    g_free(description);

    if (is_master)
     { g_snprintf( chaine, sizeof(chaine), "UPDATE syns SET libelle='%s' WHERE id='1'", description );
       Lancer_requete_SQL ( db, chaine );
     }
    g_free(description);

    Libere_DB_SQL ( &db );
/******************************************* Création fichier de config *******************************************************/
    Info_new( Config.log, TRUE, LOG_NOTICE, "%s: Creating config file '%s'", __func__, fichier );

    gint fd = creat ( fichier, S_IRUSR | S_IWUSR );
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
