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
    gint debug;
    GKeyFile *gkf;

    if (!config_cli) return;

    if (!fichier_config_cli) fichier = DEFAUT_FICHIER_CONFIG_CLI;
                        else fichier = fichier_config_cli;

    gkf = g_key_file_new();

    if (g_key_file_load_from_file(gkf, fichier, G_KEY_FILE_NONE, NULL))
     {

/********************************************* Partie SERVEUR *********************************************/
       config_cli->taille_bloc_reseau = g_key_file_get_integer ( gkf, "SERVER", "taille_bloc_reseau", NULL );
       if (!config_cli->taille_bloc_reseau) config_cli->taille_bloc_reseau = DEFAUT_TAILLE_BLOC_RESEAU;

       config_cli->ssl_crypt          = g_key_file_get_integer ( gkf, "SERVER", "ssl_crypt", NULL );
       if (!config_cli->ssl_crypt) config_cli->ssl_crypt = DEFAUT_SSL_CRYPT;

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

/********************************************* Partie DEBUG ***********************************************/
       config_cli->debug_level = 0;
       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_SIGNAUX", NULL );
       if (debug) config_cli->debug_level += DEBUG_SIGNAUX;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_DB", NULL );
       if (debug) config_cli->debug_level += DEBUG_DB;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_CONFIG", NULL );
       if (debug) config_cli->debug_level += DEBUG_CONFIG;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_USER", NULL );
       if (debug) config_cli->debug_level += DEBUG_USER;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_CRYPTO", NULL );
       if (debug) config_cli->debug_level += DEBUG_CRYPTO;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_INFO", NULL );
       if (debug) config_cli->debug_level += DEBUG_INFO;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_SERVEUR", NULL );
       if (debug) config_cli->debug_level += DEBUG_SERVEUR;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_CDG", NULL );
       if (debug) config_cli->debug_level += DEBUG_CDG;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_NETWORK", NULL );
       if (debug) config_cli->debug_level += DEBUG_NETWORK;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_ARCHIVE", NULL );
       if (debug) config_cli->debug_level += DEBUG_ARCHIVE;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_CONNEXION", NULL );
       if (debug) config_cli->debug_level += DEBUG_CONNEXION;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_DLS", NULL );
       if (debug) config_cli->debug_level += DEBUG_DLS;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_MODBUS", NULL );
       if (debug) config_cli->debug_level += DEBUG_MODBUS;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_ADMIN", NULL );
       if (debug) config_cli->debug_level += DEBUG_ADMIN;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_RS485", NULL );
       if (debug) config_cli->debug_level += DEBUG_RS485;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_ONDULEUR", NULL );
       if (debug) config_cli->debug_level += DEBUG_ONDULEUR;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_SMS", NULL );
       if (debug) config_cli->debug_level += DEBUG_SMS;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_AUDIO", NULL );
       if (debug) config_cli->debug_level += DEBUG_AUDIO;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_CAMERA", NULL );
       if (debug) config_cli->debug_level += DEBUG_CAMERA;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_COURBE", NULL );
       if (debug) config_cli->debug_level += DEBUG_COURBE;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_ALL", NULL );
       if (debug) config_cli->debug_level = ~0;

       if (!config_cli->debug_level) config_cli->debug_level = DEFAUT_DEBUG_LEVEL;          /* Par défaut */
     }
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Print_config_cli: Affichage (enfin log) la config actuelle en parametre                                */
/* Entrée: une config !! -> le champ log doit etre initialisé via la librairie Erreur                     */
/**********************************************************************************************************/
 void Print_config_cli ( struct CONFIG_CLI *config_cli )
  { if (!config_cli->log) return;
    Info_c( config_cli->log, DEBUG_CONFIG, "Config user                 ", config_cli->user );
    Info_c( config_cli->log, DEBUG_CONFIG, "Config serveur              ", config_cli->serveur );
    Info_n( config_cli->log, DEBUG_CONFIG, "Config port                 ", config_cli->port );
    Info_n( config_cli->log, DEBUG_CONFIG, "Config ssl crypt            ", config_cli->ssl_crypt );
    Info_n( config_cli->log, DEBUG_CONFIG, "Config debug level          ", config_cli->debug_level );
    Info_n( config_cli->log, DEBUG_CONFIG, "Config taille_bloc_reseau   ", config_cli->taille_bloc_reseau );
  }
/*--------------------------------------------------------------------------------------------------------*/
