/******************************************************************************************************************************/
/* Watchdogd/Config/Config.c        Lecture du fichier de configuration Watchdog                                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mer. 15 déc. 2010 13:30:12 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Config.c
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

 #include <stdio.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <string.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */

/******************************************************************************************************************************/
/* Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                        */
/* Entrée: le nom de fichier à lire                                                                                           */
/* Sortie: La structure mémoire est à jour                                                                                    */
/******************************************************************************************************************************/
 void Lire_config ( void )
  { GError *error = NULL;
    gchar *chaine;
    GKeyFile *gkf;
	   gint num;

    memset ( &Config, 0, sizeof(struct CONFIG) );

    Config.config = Json_read_from_file ( "/etc/abls-habitat-agent.conf" );
    if (!Config.config) Config.config = Json_node_create();

    chaine = getenv ( "ABLS_DOMAIN_UUID" );
    if (chaine) Json_node_add_string ( Config.config, "domain_uuid", chaine );
    if (!Json_has_member ( Config.config, "domain_uuid"   )) Json_node_add_string ( Config.config, "domain_uuid",  "default" );

    chaine = getenv ( "ABLS_DOMAIN_SECRET" );
    if (chaine) Json_node_add_string ( Config.config, "domain_secret", chaine );
    if (!Json_has_member ( Config.config, "domain_secret" )) Json_node_add_string ( Config.config, "domain_secret", "default" );

    chaine = getenv ( "ABLS_API_URL" );
    if (chaine) Json_node_add_string ( Config.config, "api_url", chaine );
    if (!Json_has_member ( Config.config, "api_url" ))       Json_node_add_string ( Config.config, "api_url", "api.abls-habitat.fr" );

    if (!Json_has_member ( Config.config, "install_time"  )) Json_node_add_string ( Config.config, "install_time", "1980-10-22 02:50:00" );

    g_snprintf( Config.master_hostname, sizeof(Config.master_hostname), "localhost" );
    g_snprintf( Config.librairie_dir,   sizeof(Config.librairie_dir),   "%s", DEFAUT_PROCESS_DIR   );

    Config.instance_is_master = TRUE;
    Config.db_port            = DEFAUT_DB_PORT;

    g_snprintf( Config.db_hostname, sizeof(Config.db_hostname), "%s", DEFAUT_DB_HOST  );
    g_snprintf( Config.db_database, sizeof(Config.db_database), "%s", DEFAUT_DB_DATABASE  );
    g_snprintf( Config.db_password, sizeof(Config.db_password), "%s", DEFAUT_DB_PASSWORD  );
    g_snprintf( Config.db_username, sizeof(Config.db_username), "%s", DEFAUT_DB_USERNAME  );

    Config.log_level = LOG_INFO;

    gkf = g_key_file_new();

    g_snprintf( Config.config_file, sizeof(Config.config_file), "%s", "/etc/watchdogd.abls.conf" );
    if (!g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, &error))
     { printf("Unable to parse config file %s, error %s\n", Config.config_file, error->message );
       g_error_free( error );
       return;
     }
/******************************************************* Partie GLOBAL ********************************************************/
    printf("Using config file %s.\n", Config.config_file );

    chaine = g_key_file_get_string ( gkf, "GLOBAL", "library_dir", NULL );
    if (chaine)
     { g_snprintf( Config.librairie_dir, sizeof(Config.librairie_dir), "%s", chaine ); g_free(chaine); }

/******************************************************** Partie DATABASE *****************************************************/
    num           = g_key_file_get_integer ( gkf, "DATABASE", "port", NULL );
    if (num) Config.db_port = num;

    chaine = g_key_file_get_string ( gkf, "DATABASE", "hostname", NULL );
    if (chaine)
     { g_snprintf( Config.db_hostname, sizeof(Config.db_hostname), "%s", chaine ); g_free(chaine); }

    chaine = g_key_file_get_string ( gkf, "DATABASE", "database", NULL );
    if (chaine)
     { g_snprintf( Config.db_database, sizeof(Config.db_database), "%s", chaine ); g_free(chaine); }

    chaine = g_key_file_get_string ( gkf, "DATABASE", "password", NULL );
    if (chaine)
     { g_snprintf( Config.db_password, sizeof(Config.db_password), "%s", chaine ); g_free(chaine); }

    chaine = g_key_file_get_string ( gkf, "DATABASE", "username", NULL );
    if (chaine)
     { g_snprintf( Config.db_username, sizeof(Config.db_username), "%s", chaine ); g_free(chaine); }

    g_key_file_free(gkf);
  }
/******************************************************************************************************************************/
/* Print_config: Affichage (enfin log) la config actuelle en parametre                                                        */
/* Entrée: une config !! -> le champ log doit etre initialisé via la librairie Erreur                                         */
/******************************************************************************************************************************/
 void Print_config ( void )
  {
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config file                 %s", Config.config_file );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config db hostname          %s", Config.db_hostname );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config db database          %s", Config.db_database );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config db username          %s", Config.db_username );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config db password          *******" );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config db port              %d", Config.db_port );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config single               %d", Config.single );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config domain_uuid          %s", Json_get_string ( Config.config, "domain_uuid" ) );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config domain_secret        *******" );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config agent_uuid           %s", Json_get_string ( Config.config, "agent_uuid" ) );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config api_url              %s", Json_get_string ( Config.config, "api_url" ) );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config install_time         %s", Json_get_string ( Config.config, "install_time" ) );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config headless             %d", Config.headless );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config log_level            %d", Config.log_level );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config log_db               %d", Config.log_db );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config log_bus              %d", Config.log_bus );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config log_trad             %d", Config.log_trad );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config log_msrv             %d", Config.log_msrv );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config home                 %s", Config.home );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config instance             %s", g_get_host_name() );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config instance is master   %d", Config.instance_is_master );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config master_hostname      %s", Config.master_hostname );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config librairie_dir        %s", Config.librairie_dir );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
