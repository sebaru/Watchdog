/**********************************************************************************************************/
/* Watchdogd/Config/Config.c        Lecture du fichier de configuration Watchdog                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 15 déc. 2010 13:30:12 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Config.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */

/**********************************************************************************************************/
/* Lire_config : Lit la config Watchdog et rempli la structure mémoire                                    */
/* Entrée: le nom de fichier à lire                                                                       */
/* Sortie: La structure mémoire est à jour                                                                */
/**********************************************************************************************************/
 void Lire_config ( char *fichier_config )
  { gchar *chaine, *fichier;
    GKeyFile *gkf;

    if (!fichier_config) fichier = DEFAUT_FICHIER_CONFIG_SRV;
                    else fichier = fichier_config;

    gkf = g_key_file_new();

    if (g_key_file_load_from_file(gkf, fichier, G_KEY_FILE_NONE, NULL))
     {
       g_snprintf( Config.config_file, sizeof(Config.config_file), "%s", fichier );
/********************************************** Partie GLOBAL *********************************************/
       Config.db_port            = g_key_file_get_integer ( gkf, "GLOBAL", "db_port", NULL );
       if (!Config.db_port) Config.db_port = DEFAUT_DB_PORT;

       chaine                    = g_key_file_get_string ( gkf, "GLOBAL", "home", NULL );
       if (chaine)
        { g_snprintf( Config.home, sizeof(Config.home), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.home, sizeof(Config.home), "%s", DEFAUT_HOME  ); }

       chaine                    = g_key_file_get_string ( gkf, "GLOBAL", "run_as", NULL );
       if (chaine)
        { g_snprintf( Config.run_as, sizeof(Config.run_as), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.run_as, sizeof(Config.run_as), "%s", DEFAUT_RUN_AS   ); }

       chaine                    = g_key_file_get_string ( gkf, "GLOBAL", "library_dir", NULL );
       if (chaine)
        { g_snprintf( Config.librairie_dir, sizeof(Config.librairie_dir), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.librairie_dir, sizeof(Config.librairie_dir), "%s", DEFAUT_LIBRAIRIE_DIR   ); }

       chaine                    = g_key_file_get_string ( gkf, "GLOBAL", "db_host", NULL );
       if (chaine)
        { g_snprintf( Config.db_host, sizeof(Config.db_host), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.db_host, sizeof(Config.db_host), "%s", DEFAUT_DB_HOST  ); }

       chaine                    = g_key_file_get_string ( gkf, "GLOBAL", "db_database", NULL );
       if (chaine)
        { g_snprintf( Config.db_database, sizeof(Config.db_database), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.db_database, sizeof(Config.db_database), "%s", DEFAUT_DB_DATABASE  ); }

       chaine                    = g_key_file_get_string ( gkf, "GLOBAL", "db_password", NULL );
       if (chaine)
        { g_snprintf( Config.db_password, sizeof(Config.db_password), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.db_password, sizeof(Config.db_password), "%s", DEFAUT_DB_PASSWORD  ); }

       chaine                    = g_key_file_get_string ( gkf, "GLOBAL", "db_username", NULL );
       if (chaine)
        { g_snprintf( Config.db_username, sizeof(Config.db_username), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.db_username, sizeof(Config.db_username), "%s", DEFAUT_DB_USERNAME  ); }

       chaine                    = g_key_file_get_string ( gkf, "GLOBAL", "crypto_key", NULL );
       if (chaine)
        { g_snprintf( Config.crypto_key, sizeof(Config.crypto_key), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.crypto_key, sizeof(Config.crypto_key), "%s", DEFAUT_CRYPTO_KEY  ); }

/********************************************** Partie TELLSTICK ******************************************/
       Config.tellstick_a_min    = g_key_file_get_integer ( gkf, "TELLSTICK", "min_a", NULL );
       if (!Config.tellstick_a_min) Config.tellstick_a_min = DEFAUT_TELLSTICK_A_MIN;

       Config.tellstick_a_max    = g_key_file_get_integer ( gkf, "TELLSTICK", "max_a", NULL );
       if (!Config.tellstick_a_max) Config.tellstick_a_max = DEFAUT_TELLSTICK_A_MAX;

/********************************************** Partie ASTERISK *******************************************/
       Config.asterisk_m_min    = g_key_file_get_integer ( gkf, "ASTERISK", "min_m", NULL );
       if (!Config.asterisk_m_min) Config.asterisk_m_min = DEFAUT_ASTERISK_M_MIN;

       Config.asterisk_m_max    = g_key_file_get_integer ( gkf, "ASTERISK", "max_m", NULL );
       if (!Config.asterisk_m_max) Config.asterisk_m_max = DEFAUT_ASTERISK_M_MAX;

/********************************************* Partie LOG *************************************************/
       Config.log_level = LOG_NOTICE;
       chaine = g_key_file_get_string ( gkf, "LOG", "log_level", NULL );
       if (chaine)
        {      if ( ! strcmp( chaine, "debug"    ) ) Config.log_level = LOG_DEBUG;
          else if ( ! strcmp( chaine, "info"     ) ) Config.log_level = LOG_INFO;
          else if ( ! strcmp( chaine, "notice"   ) ) Config.log_level = LOG_NOTICE;
          else if ( ! strcmp( chaine, "warning"  ) ) Config.log_level = LOG_WARNING;
          else if ( ! strcmp( chaine, "error"    ) ) Config.log_level = LOG_ERR;
          else if ( ! strcmp( chaine, "critical" ) ) Config.log_level = LOG_CRIT;
          g_free(chaine);
        }

       Config.log_all = g_key_file_get_boolean ( gkf, "LOG", "log_all", NULL );
       Config.log_db  = g_key_file_get_boolean ( gkf, "LOG", "log_db", NULL );
     }
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Print_config: Affichage (enfin log) la config actuelle en parametre                                    */
/* Entrée: une config !! -> le champ log doit etre initialisé via la librairie Erreur                     */
/**********************************************************************************************************/
 void Print_config ( void )
  { 
    if (!Config.log) return;
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config file                 %s", Config.config_file );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config run_as               %s", Config.run_as );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config log_level            %d", Config.log_level );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config home                 %s", Config.home );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config librairie_dir        %s", Config.librairie_dir );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config db host              %s", Config.db_host );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config db database          %s", Config.db_database );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config db username          %s", Config.db_username );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config db password          %s", Config.db_password );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config db port              %d", Config.db_port );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config crypto key           %s", Config.crypto_key );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config compil               %d", Config.compil );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config single               %d", Config.single );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config tellstick A(min)     %d", Config.tellstick_a_min );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config tellstick A(max)     %d", Config.tellstick_a_max );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config asterisk M(min)      %d", Config.asterisk_m_min );
    Info_new( Config.log, Config.log_all, LOG_INFO, "Config asterisk M(max)      %d", Config.asterisk_m_max );
  }
/*--------------------------------------------------------------------------------------------------------*/
