/**********************************************************************************************************/
/* Watchdogd/Config/Config.c        Lecture du fichier de configuration Watchdog                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 22 jun 2004 17:44:24 CEST */
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

 #include "Config.h"
 #include "Config_lignes.h"


/**********************************************************************************************************/
/* Lire_config: lecture et prise en compte de la configuration serveur Watchdog                           */
/* Entrée: La structure à remplir, le nom du fichier originel                                             */
/**********************************************************************************************************/
 void Lire_config ( char *fichier_config )
  { char *fichier;
    FILE *rc;

    Config.port                  = DEFAUT_PORT;
    Config.max_client            = DEFAUT_MAX_CLIENT;
    Config.min_serveur           = DEFAUT_MIN_SERVEUR;
    Config.max_serveur           = DEFAUT_MAX_SERVEUR;
    Config.max_inactivite        = DEFAUT_MAX_INACTIVITE;
    Config.taille_clef_dh        = DEFAUT_TAILLE_CLEF_DH;
    Config.taille_clef_rsa       = DEFAUT_TAILLE_CLEF_RSA;
    Config.ssl_crypt             = DEFAUT_SSL_CRYPT;
    Config.max_msg_visu          = DEFAUT_MAX_MSG_VISU;
    Config.taille_bloc_reseau    = DEFAUT_TAILLE_BLOC_RESEAU;
    Config.debug_level           = DEFAUT_DEBUG_LEVEL;
    Config.timeout_connexion     = DEFAUT_TIMEOUT_CONNEXION;
    Config.max_login_failed      = DEFAUT_MAX_LOGIN_FAILED;
    snprintf( Config.port_RS485,  sizeof(Config.port_RS485),  "%s", DEFAUT_PORT_RS485  );
    snprintf( Config.crypto_key,  sizeof(Config.crypto_key),  "%s", DEFAUT_CRYPTO_KEY  );
    snprintf( Config.home,        sizeof(Config.home),        "%s", DEFAUT_HOME        );
    snprintf( Config.db_database, sizeof(Config.db_database), "%s", DEFAUT_DB_DATABASE );
    snprintf( Config.db_password, sizeof(Config.db_password), "%s", DEFAUT_DB_PASSWORD );
    snprintf( Config.db_username, sizeof(Config.db_username), "%s", DEFAUT_DB_USERNAME );
    Config.db_port               = DEFAUT_DB_PORT;

    if (!fichier_config) fichier = DEFAUT_FICHIER_CONFIG_SRV;
                    else fichier = fichier_config;
    rc = fopen( fichier, "r");
    if (rc)
     { Interpreter_config(rc);
       fclose(rc);
     }
  }
/**********************************************************************************************************/
/* Print_config: Affichage (enfin log) la config actuelle en parametre                                    */
/* Entrée: une config !! -> le champ log doit etre initialisé via la librairie Erreur                     */
/**********************************************************************************************************/
 void Print_config ( void )
  { 
    if (!Config.log) return;
    Info_n( Config.log, DEBUG_CONFIG, "Config port                 ", Config.port );
    Info_c( Config.log, DEBUG_CONFIG, "Config port rs485           ", Config.port_RS485  );
    Info_n( Config.log, DEBUG_CONFIG, "Config max client           ", Config.max_client );
    Info_n( Config.log, DEBUG_CONFIG, "Config min serveur          ", Config.min_serveur );
    Info_n( Config.log, DEBUG_CONFIG, "Config max serveur          ", Config.max_serveur );
    Info_n( Config.log, DEBUG_CONFIG, "Config max inactivite       ", Config.max_inactivite );
    Info_n( Config.log, DEBUG_CONFIG, "Config taille_clef_dh       ", Config.taille_clef_dh );
    Info_n( Config.log, DEBUG_CONFIG, "Config taille_clef_rsa      ", Config.taille_clef_rsa );
    Info_n( Config.log, DEBUG_CONFIG, "Config ssl crypt            ", Config.ssl_crypt );
    Info_n( Config.log, DEBUG_CONFIG, "Config max msg visu         ", Config.max_msg_visu );
    Info_n( Config.log, DEBUG_CONFIG, "Config debug level          ", Config.debug_level );
    Info_n( Config.log, DEBUG_CONFIG, "Config taille_bloc_reseau   ", Config.taille_bloc_reseau );
    Info_n( Config.log, DEBUG_CONFIG, "Config timeout connexion    ", Config.timeout_connexion );
    Info_n( Config.log, DEBUG_CONFIG, "Config max login failed     ", Config.max_login_failed );
    Info_c( Config.log, DEBUG_CONFIG, "Config home                 ", Config.home );
    Info_c( Config.log, DEBUG_CONFIG, "Config db database          ", Config.db_database );
    Info_c( Config.log, DEBUG_CONFIG, "Config db username          ", Config.db_username );
    Info_c( Config.log, DEBUG_CONFIG, "Config db password          ", Config.db_password );
    Info_n( Config.log, DEBUG_CONFIG, "Config db port              ", Config.db_port );
    Info_c( Config.log, DEBUG_CONFIG, "Config crypto key           ", Config.crypto_key );
    Info_n( Config.log, DEBUG_CONFIG, "Config single               ", Config.single );
  }
/*--------------------------------------------------------------------------------------------------------*/
