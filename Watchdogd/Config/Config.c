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
    gint debug;
    GKeyFile *gkf;

    if (!fichier_config) fichier = DEFAUT_FICHIER_CONFIG_SRV;
                    else fichier = fichier_config;

    gkf = g_key_file_new();

    if (g_key_file_load_from_file(gkf, fichier, G_KEY_FILE_NONE, NULL))
     {
/********************************************** Partie GLOBAL *********************************************/
       Config.db_port            = g_key_file_get_integer ( gkf, "GLOBAL", "db_port", NULL );
       if (!Config.db_port) Config.db_port = DEFAUT_DB_PORT;

       chaine                    = g_key_file_get_string ( gkf, "GLOBAL", "home", NULL );
       if (chaine)
        { g_snprintf( Config.home, sizeof(Config.home), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.home, sizeof(Config.home), "%s", DEFAUT_HOME  ); }

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
/********************************************* Partie SMS *************************************************/
       chaine                    = g_key_file_get_string ( gkf, "SMS", "smsbox_username", NULL );
       if (chaine)
        { g_snprintf( Config.smsbox_username, sizeof(Config.smsbox_username), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.smsbox_username, sizeof(Config.smsbox_username), "%s", DEFAUT_SMSBOX_USERNAME  ); }

       chaine                    = g_key_file_get_string ( gkf, "SMS", "smsbox_password", NULL );
       if (chaine)
        { g_snprintf( Config.smsbox_password, sizeof(Config.smsbox_password), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.smsbox_password, sizeof(Config.smsbox_password), "%s", DEFAUT_SMSBOX_PASSWORD  ); }

       chaine                    = g_key_file_get_string ( gkf, "SMS", "sms_telephone", NULL );
       if (chaine)
        { g_snprintf( Config.sms_telephone, sizeof(Config.sms_telephone), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.sms_telephone, sizeof(Config.sms_telephone), "%s", DEFAUT_SMS_TELEPHONE  ); }

/********************************************* Partie SERVER **********************************************/
       Config.port               = g_key_file_get_integer ( gkf, "SERVER", "port", NULL );
       if (!Config.port) Config.port = DEFAUT_PORT;

       Config.max_client         = g_key_file_get_integer ( gkf, "SERVER", "max_client", NULL );
       if (!Config.max_client) Config.max_client = DEFAUT_MAX_CLIENT;

       Config.min_serveur        = g_key_file_get_integer ( gkf, "SERVER", "min_serveur", NULL );
       if (!Config.min_serveur) Config.min_serveur = DEFAUT_MIN_SERVEUR;

       Config.max_serveur        = g_key_file_get_integer ( gkf, "SERVER", "max_serveur", NULL );
       if (!Config.max_serveur) Config.max_serveur = DEFAUT_MAX_SERVEUR;

       Config.max_inactivite     = g_key_file_get_integer ( gkf, "SERVER", "max_inactivite", NULL );
       if (!Config.max_inactivite) Config.max_inactivite = DEFAUT_MAX_INACTIVITE;

       Config.max_login_failed   = g_key_file_get_integer ( gkf, "SERVER", "max_login_failed", NULL );
       if (!Config.max_login_failed) Config.max_login_failed = DEFAUT_MAX_LOGIN_FAILED;

       Config.timeout_connexion  = g_key_file_get_integer ( gkf, "SERVER", "timeout_connexion", NULL );
       if (!Config.timeout_connexion) Config.timeout_connexion = DEFAUT_TIMEOUT_CONNEXION;

       Config.taille_clef_dh     = g_key_file_get_integer ( gkf, "SERVER", "taille_clef_dh", NULL );
       if (!Config.taille_clef_dh) Config.taille_clef_dh = DEFAUT_TAILLE_CLEF_DH;

       Config.taille_clef_rsa    = g_key_file_get_integer ( gkf, "SERVER", "taille_clef_rsa", NULL );
       if (!Config.taille_clef_rsa) Config.taille_clef_rsa = DEFAUT_TAILLE_CLEF_RSA;

       Config.taille_bloc_reseau = g_key_file_get_integer ( gkf, "SERVER", "taille_bloc_reseau", NULL );
       if (!Config.taille_bloc_reseau) Config.taille_bloc_reseau = DEFAUT_TAILLE_BLOC_RESEAU;

       chaine = g_key_file_get_string ( gkf, "RS485", "port_rs485", NULL );
       if (chaine)
        { g_snprintf( Config.port_RS485, sizeof(Config.port_RS485), "%s", chaine ); g_free(chaine); }
       else
        { g_snprintf( Config.port_RS485, sizeof(Config.port_RS485), "%s", DEFAUT_PORT_RS485  ); }

/********************************************** Partie TELLSTICK ******************************************/
       Config.tellstick_a_min    = g_key_file_get_integer ( gkf, "TELLSTICK", "min_a", NULL );
       if (!Config.tellstick_a_min) Config.tellstick_a_min = DEFAUT_TELLSTICK_A_MIN;

       Config.tellstick_a_max    = g_key_file_get_integer ( gkf, "TELLSTICK", "max_a", NULL );
       if (!Config.tellstick_a_max) Config.tellstick_a_max = DEFAUT_TELLSTICK_A_MAX;

/********************************************* Partie DEBUG ***********************************************/
       Config.debug_level = 0;
       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_SIGNAUX", NULL );
       if (debug) Config.debug_level += DEBUG_SIGNAUX;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_DB", NULL );
       if (debug) Config.debug_level += DEBUG_DB;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_CONFIG", NULL );
       if (debug) Config.debug_level += DEBUG_CONFIG;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_USER", NULL );
       if (debug) Config.debug_level += DEBUG_USER;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_CRYPTO", NULL );
       if (debug) Config.debug_level += DEBUG_CRYPTO;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_INFO", NULL );
       if (debug) Config.debug_level += DEBUG_INFO;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_SERVEUR", NULL );
       if (debug) Config.debug_level += DEBUG_SERVEUR;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_CDG", NULL );
       if (debug) Config.debug_level += DEBUG_CDG;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_NETWORK", NULL );
       if (debug) Config.debug_level += DEBUG_NETWORK;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_ARCHIVE", NULL );
       if (debug) Config.debug_level += DEBUG_ARCHIVE;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_CONNEXION", NULL );
       if (debug) Config.debug_level += DEBUG_CONNEXION;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_DLS", NULL );
       if (debug) Config.debug_level += DEBUG_DLS;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_MODBUS", NULL );
       if (debug) Config.debug_level += DEBUG_MODBUS;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_ADMIN", NULL );
       if (debug) Config.debug_level += DEBUG_ADMIN;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_RS485", NULL );
       if (debug) Config.debug_level += DEBUG_RS485;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_ONDULEUR", NULL );
       if (debug) Config.debug_level += DEBUG_ONDULEUR;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_SMS", NULL );
       if (debug) Config.debug_level += DEBUG_SMS;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_AUDIO", NULL );
       if (debug) Config.debug_level += DEBUG_AUDIO;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_CAMERA", NULL );
       if (debug) Config.debug_level += DEBUG_CAMERA;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_COURBE", NULL );
       if (debug) Config.debug_level += DEBUG_COURBE;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_TELLSTICK", NULL );
       if (debug) Config.debug_level += DEBUG_TELLSTICK;

       debug = g_key_file_get_boolean ( gkf, "DEBUG", "debug_ALL", NULL );
       if (debug) Config.debug_level = ~0;

       if (!Config.debug_level) Config.debug_level = DEFAUT_DEBUG_LEVEL;                    /* Par défaut */

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
    Info_n( Config.log, DEBUG_CONFIG, "Config port                 ", Config.port );
    Info_c( Config.log, DEBUG_CONFIG, "Config port rs485           ", Config.port_RS485  );
    Info_n( Config.log, DEBUG_CONFIG, "Config max client           ", Config.max_client );
    Info_n( Config.log, DEBUG_CONFIG, "Config min serveur          ", Config.min_serveur );
    Info_n( Config.log, DEBUG_CONFIG, "Config max serveur          ", Config.max_serveur );
    Info_n( Config.log, DEBUG_CONFIG, "Config max inactivite       ", Config.max_inactivite );
    Info_n( Config.log, DEBUG_CONFIG, "Config taille_clef_dh       ", Config.taille_clef_dh );
    Info_n( Config.log, DEBUG_CONFIG, "Config taille_clef_rsa      ", Config.taille_clef_rsa );
    Info_n( Config.log, DEBUG_CONFIG, "Config ssl crypt            ", Config.ssl_crypt );
    Info_n( Config.log, DEBUG_CONFIG, "Config debug level          ", Config.debug_level );
    Info_n( Config.log, DEBUG_CONFIG, "Config taille_bloc_reseau   ", Config.taille_bloc_reseau );
    Info_n( Config.log, DEBUG_CONFIG, "Config timeout connexion    ", Config.timeout_connexion );
    Info_n( Config.log, DEBUG_CONFIG, "Config max login failed     ", Config.max_login_failed );
    Info_c( Config.log, DEBUG_CONFIG, "Config home                 ", Config.home );
    Info_c( Config.log, DEBUG_CONFIG, "Config db host              ", Config.db_host );
    Info_c( Config.log, DEBUG_CONFIG, "Config db database          ", Config.db_database );
    Info_c( Config.log, DEBUG_CONFIG, "Config db username          ", Config.db_username );
    Info_c( Config.log, DEBUG_CONFIG, "Config db password          ", Config.db_password );
    Info_n( Config.log, DEBUG_CONFIG, "Config db port              ", Config.db_port );
    Info_c( Config.log, DEBUG_CONFIG, "Config crypto key           ", Config.crypto_key );
    Info_n( Config.log, DEBUG_CONFIG, "Config compil               ", Config.compil );
    Info_n( Config.log, DEBUG_CONFIG, "Config single               ", Config.single );
    Info_c( Config.log, DEBUG_CONFIG, "Config smsbox username      ", Config.smsbox_username );
    Info_c( Config.log, DEBUG_CONFIG, "Config smsbox password      ", Config.smsbox_password );
    Info_c( Config.log, DEBUG_CONFIG, "Config sms_telephone        ", Config.sms_telephone );
    Info_n( Config.log, DEBUG_CONFIG, "Config tellstick A(min)     ", Config.tellstick_a_min );
    Info_n( Config.log, DEBUG_CONFIG, "Config tellstick A(max)     ", Config.tellstick_a_max );
  }
/*--------------------------------------------------------------------------------------------------------*/
