/**********************************************************************************************************/
/* Client/Config_cli/Config_cli.c        Lecture du fichier de configuration cliente Watchdog             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 19:02:43 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Config_cli.c
 * This file is part of <Watchdog>
 *
 * Copyright (C) 2010 - sebastien
 *
 * <Watchdog> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <Watchdog> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <Watchdog>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #include <stdio.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <string.h>

 #include "Config_cli.h"
/**********************************************************************************************************/
/* Lire_config_cli_cli: lecture et prise en compte de la configuration cliente Watchdog                   */
/* Entrée: La structure à remplir, le nom du fichier originel                                             */
/**********************************************************************************************************/
 void Lire_config_cli ( struct CONFIG_CLI *config_cli, char *fichier_config_cli )
  { gchar *chaine, *fichier;
    GKeyFile *gkf;
    GError *error=NULL;

    if (!config_cli) return;

    if (!fichier_config_cli) fichier = DEFAUT_FICHIER_CONFIG_CLI;
                        else fichier = fichier_config_cli;
    printf("Using config file %s\n", fichier );
    gkf = g_key_file_new();

    config_cli->log_level = DEFAUT_LOG_LEVEL;                                 /* Niveau de log par défaut */
    config_cli->log_override = TRUE;
    if (g_key_file_load_from_file( gkf, fichier, G_KEY_FILE_NONE, &error ))
     {

/********************************************* Partie SERVEUR *********************************************/
       chaine = g_key_file_get_string ( gkf, "SERVER", "host", NULL );
       if (chaine)
        { g_snprintf( config_cli->host, sizeof(config_cli->host), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( config_cli->host, sizeof(config_cli->host), "%s", DEFAUT_SERVEUR  ); }

       chaine = g_key_file_get_string ( gkf, "SERVER", "user", NULL );
       if (chaine)
        { g_snprintf( config_cli->user, sizeof(config_cli->user), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( config_cli->user, sizeof(config_cli->user), "%s", DEFAUT_USER  ); }

       chaine = g_key_file_get_string ( gkf, "SERVER", "password", NULL );
       if (chaine)
        { g_snprintf( config_cli->passwd, sizeof(config_cli->passwd), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( config_cli->passwd, sizeof(config_cli->passwd), "%s", DEFAUT_PASSWD  ); }

       chaine = g_key_file_get_string ( gkf, "SERVER", "ssl_file_ca", NULL );
       if (chaine)
        { g_snprintf( config_cli->ssl_file_ca, sizeof(config_cli->ssl_file_ca), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( config_cli->ssl_file_ca, sizeof(config_cli->ssl_file_ca), "%s", DEFAUT_SSL_FILE_CA ); }

       config_cli->port_ihm = g_key_file_get_integer ( gkf, "SERVER", "port_ihm", NULL );
       if (!config_cli->port_ihm) config_cli->port_ihm = DEFAUT_PORT_IHM;

       config_cli->port_http = g_key_file_get_integer ( gkf, "SERVER", "port_http", NULL );
       if (!config_cli->port_http) config_cli->port_http = DEFAUT_PORT_HTTP;
/********************************************* Partie LOG *************************************************/
       chaine = g_key_file_get_string ( gkf, "LOG", "log_level", NULL );
       if (chaine)
        {      if ( ! strcmp ( chaine, "debug"     ) ) config_cli->log_level = LOG_DEBUG;
          else if ( ! strcmp ( chaine, "info  "    ) ) config_cli->log_level = LOG_INFO;
          else if ( ! strcmp ( chaine, "warning"   ) ) config_cli->log_level = LOG_WARNING;
          else if ( ! strcmp ( chaine, "error"     ) ) config_cli->log_level = LOG_ERR;
          else if ( ! strcmp ( chaine, "critical"  ) ) config_cli->log_level = LOG_CRIT;
          else if ( ! strcmp ( chaine, "emergency" ) ) config_cli->log_level = LOG_EMERG;
          g_free(chaine);
        }

       config_cli->log_override = g_key_file_get_boolean ( gkf, "DEBUG", "log_all", NULL );

/********************************************* Partie GUI *************************************************/
       config_cli->gui_tech = g_key_file_get_boolean ( gkf, "GUI", "technical", NULL );
       config_cli->gui_fullscreen = g_key_file_get_boolean ( gkf, "GUI", "fullscreen", NULL );
     } else 
        { printf("Unable to parse config file %s, error %s\n", fichier, error->message );
          g_error_free( error );
        }
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Print_config_cli: Affichage (enfin log) la config actuelle en parametre                                */
/* Entrée: une config !! -> le champ log doit etre initialisé via la librairie Erreur                     */
/**********************************************************************************************************/
 void Print_config_cli ( struct CONFIG_CLI *config_cli )
  { if (!config_cli->log) return;
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config GUI Technical ---- %d", config_cli->gui_tech );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config host ------------- %s", config_cli->host );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config port_ihm --------- %d", config_cli->port_ihm );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config port_http -------- %d", config_cli->port_http );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config user ------------- %s", config_cli->user );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config passwd ----------- %s", config_cli->passwd );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config ssl_file_ca ----------- %s", config_cli->ssl_file_ca );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config log_level -------- %d", config_cli->log_level );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config log_override ----- %d", config_cli->log_override );
  }
/*--------------------------------------------------------------------------------------------------------*/
