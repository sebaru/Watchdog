/**********************************************************************************************************/
/* Watchdogd/Include/Config.h        D�finitions de la structure de configuration watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:23:17 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Config.h
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
 
#ifndef _CONFIG_H_
 #define _CONFIG_H_

 #define TAILLE_HOME              80                                           /* Chemin HOME de watchdog */
 #define TAILLE_CRYPTO_KEY        16      /* 16 octets (128bits) pour le cryptage BlowFish. Multiple de 8 */

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */

  struct CONFIG
  { gchar config_file[80];                    /* Nom du fichier dont est issu les informations ci dessous */
    gchar run_as [ 40 ];                                        /* Nom du user sur lequel tourne Watchdog */
    gint  db_port;
    gchar db_host    [ TAILLE_DB_HOST+1 ];                            /* Nom du host de la base de donnes */
    gchar db_username[ TAILLE_DB_USERNAME+1 ];            /* Nom de l'administrateur de la base de donn�es*/
    gchar db_database[ TAILLE_DB_DATABASE+1 ];                          /* Chemin d'acces aux DB watchdog */
    gchar db_password[ TAILLE_DB_PASSWORD+1 ];                          /* Mot de passe de connexion ODBC */
    guchar crypto_key[TAILLE_CRYPTO_KEY+1];           /* Clef de cryptage des mots de passes utilisateurs */
    gchar home [ TAILLE_HOME+1 ];                                          /* Repertoire maison du daemon */
    gchar librairie_dir [ TAILLE_HOME+1 ];               /* Repertoire de stockage des libraires watchdog */
    guint log_level;                                                      /* Niveau de debug du programme */
    gboolean log_msrv;                                                            /* TRUE si log_override */
    gboolean log_dls;                                                             /* TRUE si log_override */
    gboolean log_arch;                                                            /* TRUE si log_override */
    gboolean log_db;                                                          /* TRUE si log des acces DB */
    guint max_login_failed;                                            /* Nombre maximum d'echec de login */
    struct LOG *log;                                                         /* Pour l'affichage des logs */
    gint single;                                                                /* Demarrage des thread ? */
    gint compil;                                            /* Compilation des plugins DLS au demarrage ? */
    gint tellstick_a_min;                           /* Num�ro de la sortie minimum g�r�e par le tellstick */
    gint tellstick_a_max;                           /* Num�ro de la sortie maximum g�r�e par le tellstick */
    gint asterisk_m_min;                     /* Num�ro du monostable minimum g�r�e par le thread ASTERISK */
    gint asterisk_m_max;                     /* Num�ro du monostable maximum g�r�e par le thread ASTERISK */
  };

 #define DEFAUT_FICHIER_CONFIG_SRV      "/etc/watchdogd.conf"
 #define DEFAUT_RUN_AS                  "watchdog"
 #define DEFAUT_DB_HOST                 "localhost"          /* Ne pas depasser TAILLE_DB_HOST caracteres */
 #define DEFAUT_DB_DATABASE             "WatchdogDB"         /* Ne pas depasser TAILLE_DB_NAME caracteres */
 #define DEFAUT_DB_USERNAME             "watchdog"    /* Ne pas depasser TAILLE_ADMIN_USERNAME caracteres */
 #define DEFAUT_DB_PASSWORD             "seb"            /* Ne pas depasser TAILLE_DB_PASSWORD caract�res */
 #define DEFAUT_DB_PORT                 3306
 #define DEFAUT_HOME                    g_get_home_dir()        /* Ne pas depasser TAILLE_HOME caracteres */
 #define DEFAUT_LIBRAIRIE_DIR           "/usr/local/lib"        /* Ne pas depasser TAILLE_HOME caracteres */
 #define DEFAUT_CRYPTO_KEY              "My/Name/Is/Bond/"
 #define DEFAUT_TELLSTICK_A_MIN         -1
 #define DEFAUT_TELLSTICK_A_MAX         -1
 #define DEFAUT_ASTERISK_M_MIN          -1
 #define DEFAUT_ASTERISK_M_MAX          -1
/******************************************* Prototypes de fonctions **************************************/
 extern void Lire_config ( char *fichier_config );
 extern void Print_config ( void );

#endif
  
/*--------------------------------------------------------------------------------------------------------*/
