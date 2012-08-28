/**********************************************************************************************************/
/* Watchdogd/Include/Config.h        Définitions de la structure de configuration watchdog                */
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
 #include <openssl/rsa.h>

 #define NBR_ID_RS485             16

 #define TAILLE_PORT_RS485        30
 #define TAILLE_HOME              80                                           /* Chemin HOME de watchdog */
 #define TAILLE_CRYPTO_KEY        16      /* 16 octets (128bits) pour le cryptage BlowFish. Multiple de 8 */
 #define TAILLE_NUM_TELEPHONE     16                /* Nombre de caractere maximum du numero de telephone */
 #define TAILLE_SMSBOX_USERNAME   32                                /* Nombre de caractere du user SMSBOX */
 #define TAILLE_SMSBOX_PASSWORD   32                        /* Nombre de caractere du mot de passe SMSBOX */

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */

  struct CONFIG
  { gchar config_file[80];                    /* Nom du fichier dont est issu les informations ci dessous */
    gint  port;                                                    /* Port d'ecoute des requetes clientes */
    gint  max_client;                  /* Nombre maximum de client qui peuvent se connecter en meme temps */
    gint  min_serveur;                                     /* Nombre de server min à lancer en même temps */
    gint  max_serveur;                                     /* Nombre de server max à lancer en même temps */
    gint  max_inactivite;                                            /* temps max d'inactivite du serveur */
    gint  taille_clef_dh;                                       /* Taille en bits de la clef DH de codage */
    gint  taille_clef_rsa;                                     /* Taille en bits de la clef RSA de codage */
    gint  taille_bloc_reseau;
    gint  db_port;
    gchar db_host    [ TAILLE_DB_HOST+1 ];                            /* Nom du host de la base de donnes */
    gchar db_username[ TAILLE_DB_USERNAME+1 ];            /* Nom de l'administrateur de la base de données*/
    gchar db_database[ TAILLE_DB_DATABASE+1 ];                          /* Chemin d'acces aux DB watchdog */
    gchar db_password[ TAILLE_DB_PASSWORD+1 ];                          /* Mot de passe de connexion ODBC */
    gchar crypto_key[TAILLE_CRYPTO_KEY+1];            /* Clef de cryptage des mots de passes utilisateurs */
    gchar home [ TAILLE_HOME+1 ];                                          /* Repertoire maison du daemon */
    gchar librairie_dir [ TAILLE_HOME+1 ];               /* Repertoire de stockage des libraires watchdog */
    gchar sms_telephone1[TAILLE_NUM_TELEPHONE+1];               /* Numero de telephone pour les envoi SMS */
    gchar sms_telephone2[TAILLE_NUM_TELEPHONE+1];               /* Numero de telephone pour les envoi SMS */
    gchar smsbox_username[TAILLE_SMSBOX_USERNAME+1];                                       /* User SMSBOX */
    gchar smsbox_password[TAILLE_SMSBOX_PASSWORD+1];                         /* Mot de passe envoi SMSBOX */
    gboolean ssl_crypt;                                                  /* Cryptage des transmissions ?? */
    gint  timeout_connexion;                       /* Temps max d'attente de reponse de la part du client */
    guint log_level;                                                      /* Niveau de debug du programme */
    gboolean log_all;                                                             /* TRUE si log_override */
    guint max_login_failed;                                            /* Nombre maximum d'echec de login */
    struct LOG *log;                                                         /* Pour l'affichage des logs */
    RSA *rsa;                                                      /* Clefs publique et privée du serveur */
    gint single;                                                                /* Demarrage des thread ? */
    gint compil;                                            /* Compilation des plugins DLS au demarrage ? */
    gint tellstick_a_min;                           /* Numéro de la sortie minimum gérée par le tellstick */
    gint tellstick_a_max;                           /* Numéro de la sortie maximum gérée par le tellstick */
    gint sms_m_min;                               /* Numéro du monostable minimum gérée par le thread sms */
    gint sms_m_max;                               /* Numéro du monostable maximum gérée par le thread SMS */
    gint asterisk_m_min;                     /* Numéro du monostable minimum gérée par le thread ASTERISK */
    gint asterisk_m_max;                     /* Numéro du monostable maximum gérée par le thread ASTERISK */
  };

 #define DEFAUT_FICHIER_CONFIG_SRV      "/etc/watchdogd.conf"
 #define DEFAUT_PORT                    5558
 #define DEFAUT_MAX_CLIENT              100
 #define DEFAUT_MIN_SERVEUR             1
 #define DEFAUT_MAX_SERVEUR             3
 #define DEFAUT_MAX_INACTIVITE          600
 #define DEFAUT_TAILLE_CLEF_DH          512
 #define DEFAUT_TAILLE_CLEF_RSA         2048
 #define DEFAUT_SSL_CRYPT               0
 #define DEFAUT_DB_HOST                 "localhost"          /* Ne pas depasser TAILLE_DB_HOST caracteres */
 #define DEFAUT_DB_DATABASE             "WatchdogDB"         /* Ne pas depasser TAILLE_DB_NAME caracteres */
 #define DEFAUT_DB_USERNAME             "watchdog"    /* Ne pas depasser TAILLE_ADMIN_USERNAME caracteres */
 #define DEFAUT_DB_PASSWORD             "seb"            /* Ne pas depasser TAILLE_DB_PASSWORD caractères */
 #define DEFAUT_DB_PORT                 3306
 #define DEFAUT_TIMEOUT_CONNEXION       30               /* 30 secondes max pour se loguer sur le serveur */
 #define DEFAUT_TAILLE_BLOC_RESEAU      8192
 #define DEFAUT_HOME                    g_get_home_dir()        /* Ne pas depasser TAILLE_HOME caracteres */
 #define DEFAUT_LIBRAIRIE_DIR           "/usr/local/lib"        /* Ne pas depasser TAILLE_HOME caracteres */
 #define DEFAUT_MAX_LOGIN_FAILED        3
 #define DEFAUT_CRYPTO_KEY              "My/Name/Is/Bond/"
 #define DEFAUT_SMSBOX_USERNAME         "user"
 #define DEFAUT_SMSBOX_PASSWORD         "changeit"
 #define DEFAUT_SMS_TELEPHONE           "01"
 #define DEFAUT_SMS_M_MIN               -1
 #define DEFAUT_SMS_M_MAX               -1
 #define DEFAUT_TELLSTICK_A_MIN         -1
 #define DEFAUT_TELLSTICK_A_MAX         -1
 #define DEFAUT_ASTERISK_M_MIN          -1
 #define DEFAUT_ASTERISK_M_MAX          -1
/******************************************* Prototypes de fonctions **************************************/
 extern void Lire_config ( char *fichier_config );
 extern void Print_config ( void );

#endif
  
/*--------------------------------------------------------------------------------------------------------*/
