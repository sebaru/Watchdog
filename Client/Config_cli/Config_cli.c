/**********************************************************************************************************/
/* Watchdogd/Config/Config.c        Lecture du fichier de configuration Watchdog                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 19:02:43 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Config_cli.c
 * This file is part of <Watchdog>
 *
 * Copyright (C) 2009 - sebastien
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
 #include "Config_cli_lignes.h"

 extern void Interpreter ( FILE *rc, struct CONFIG_CLI *config_cli );

/**********************************************************************************************************/
/* Lire_config_cli_cli: lecture et prise en compte de la configuration cliente Watchdog                   */
/* Entrée: La structure à remplir, le nom du fichier originel                                             */
/**********************************************************************************************************/
 void Lire_config_cli ( struct CONFIG_CLI *config_cli, char *fichier_config_cli )
  { char *fichier;
    FILE *rc;

    if (!config_cli) return;

    config_cli->port                  = DEFAUT_PORT;
    config_cli->ssl_crypt             = DEFAUT_SSL_CRYPT;
    config_cli->taille_bloc_reseau    = DEFAUT_TAILLE_BLOC_RESEAU;
    config_cli->debug_level           = DEFAUT_DEBUG_LEVEL;
    g_snprintf( config_cli->serveur, sizeof(config_cli->serveur), "%s", DEFAUT_SERVEUR );
    g_snprintf( config_cli->user,    sizeof(config_cli->user),    "%s", DEFAUT_USER );

    if (!fichier_config_cli) fichier = DEFAUT_FICHIER_CONFIG_CLI;
                        else fichier = fichier_config_cli;
    rc = fopen( fichier, "r" );
    if (rc)
     { Interpreter(rc, config_cli);
       fclose(rc);
     }
    else printf( "file %s unknown\n", fichier );
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
