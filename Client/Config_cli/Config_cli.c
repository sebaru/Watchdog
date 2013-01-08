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

    if (!config_cli) return;

    if (!fichier_config_cli) fichier = DEFAUT_FICHIER_CONFIG_CLI;
                        else fichier = fichier_config_cli;

    gkf = g_key_file_new();

    config_cli->log_level = DEFAUT_LOG_LEVEL;                                 /* Niveau de log par défaut */
    config_cli->log_override = TRUE;
    if (g_key_file_load_from_file(gkf, fichier, G_KEY_FILE_NONE, NULL))
     {

/********************************************* Partie SERVEUR *********************************************/
       config_cli->ssl_crypt          = g_key_file_get_boolean ( gkf, "SERVER", "ssl_crypt", NULL );

       chaine = g_key_file_get_string ( gkf, "SERVER", "host", NULL );
       if (chaine)
        { g_snprintf( config_cli->serveur, sizeof(config_cli->serveur), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( config_cli->serveur, sizeof(config_cli->serveur), "%s", DEFAUT_SERVEUR  ); }

       chaine = g_key_file_get_string ( gkf, "SERVER", "user", NULL );
       if (chaine)
        { g_snprintf( config_cli->user, sizeof(config_cli->user), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( config_cli->user, sizeof(config_cli->user), "%s", DEFAUT_USER  ); }

       config_cli->port              = g_key_file_get_integer ( gkf, "SERVER", "port", NULL );
       if (!config_cli->port) config_cli->port = DEFAUT_PORT;

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
              "Config user ------------- %s", config_cli->user );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config serveur ---------- %s", config_cli->serveur );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config port ------------- %d", config_cli->port );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config ssl crypt -------- %d", config_cli->ssl_crypt );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config log_level -------- %d", config_cli->log_level );
    Info_new( config_cli->log, config_cli->log_override, LOG_INFO,
              "Config log_override ----- %d", config_cli->log_override );
  }
/*--------------------------------------------------------------------------------------------------------*/
