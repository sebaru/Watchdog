/**********************************************************************************************************/
/* Watchdogd/Config.h        Définitions de la structure de configuration watchdog                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:23:17 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _CONFIG_H_
 #define _CONFIG_H_
 #include <openssl/rsa.h>

 #define NBR_ID_RS485             16

 #define TAILLE_PORT_RS485        30
 #define TAILLE_DB_NAME           30
 #define TAILLE_DB_PASSWORD       30
 #define TAILLE_DB_ADMIN_USERNAME 12
 #define TAILLE_HOME              80                                           /* Chemin HOME de watchdog */
 #define TAILLE_CRYPTO_KEY        16      /* 16 octets (128bits) pour le cryptage BlowFish. Multiple de 8 */

 #include "Erreur.h"
 #include "Modbus.h"

 struct MODULE_RS485
  { gint id;
    gint ea_min, ea_max;
    gint e_min, e_max;
    gint ec_min, ec_max;
    gint s_min, s_max;
    gint sa_min, sa_max;
  };

  struct CONFIG
  { gint  port;                                                    /* Port d'ecoute des requetes clientes */
    gchar port_RS485[ TAILLE_PORT_RS485+1 ];                                         /* Nom du port RS485 */
    struct MODULE_RS485 module_rs485[NBR_ID_RS485];
    struct MODULE_MODBUS module_modbus[NBR_ID_MODBUS];
    gint  max_client;                  /* Nombre maximum de client qui peuvent se connecter en meme temps */
    gint  min_serveur;                                     /* Nombre de server min à lancer en même temps */
    gint  max_serveur;                                     /* Nombre de server max à lancer en même temps */
    gint  max_inactivite;                                            /* temps max d'inactivite du serveur */
    gint  max_msg_visu;                             /* Nombre maximum de message dans une fenetre cliente */
    gint  taille_clef_dh;                                       /* Taille en bits de la clef DH de codage */
    gint  taille_clef_rsa;                                     /* Taille en bits de la clef RSA de codage */
    gint  taille_bloc_reseau;
    gchar db_admin_username[ TAILLE_DB_ADMIN_USERNAME+1 ];/* Nom de l'administrateur de la base de données*/
    gchar db_name[ TAILLE_DB_NAME+1 ];                                  /* Chemin d'acces aux DB watchdog */
    gchar db_password[ TAILLE_DB_PASSWORD+1 ];                          /* Mot de passe de connexion ODBC */
    gchar crypto_key[TAILLE_CRYPTO_KEY+1];            /* Clef de cryptage des mots de passes utilisateurs */
    gchar home [ TAILLE_HOME+1 ];                                          /* Repertoire maison du daemon */
    gboolean ssl_crypt;                                                  /* Cryptage des transmissions ?? */
    gint  timeout_connexion;                       /* Temps max d'attente de reponse de la part du client */
    guint debug_level;                                                    /* Niveau de debug du programme */
    guint max_login_failed;                                            /* Nombre maximum d'echec de login */
    struct LOG *log;                                                         /* Pour l'affichage des logs */
    RSA *rsa;                                                      /* Clefs publique et privée du serveur */
  };

 #define DEFAUT_FICHIER_CONFIG_SRV      "/etc/watchdogd.conf"
 #define DEFAUT_PORT                    5558
 #define DEFAUT_PORT_RS485              "/dev/tts/1"
 #define DEFAUT_MAX_CLIENT              100
 #define DEFAUT_MIN_SERVEUR             1
 #define DEFAUT_MAX_SERVEUR             3
 #define DEFAUT_MAX_INACTIVITE          600
 #define DEFAUT_MAX_MSG_VISU            150
 #define DEFAUT_TAILLE_CLEF_DH          512
 #define DEFAUT_TAILLE_CLEF_RSA         2048
 #define DEFAUT_SSL_CRYPT               0
 #define DEFAUT_DB_NAME                 "WatchdogDB"         /* Ne pas depasser TAILLE_DB_NAME caracteres */
 #define DEFAUT_DB_ADMIN_USERNAME       "Watchdog"    /* Ne pas depasser TAILLE_ADMIN_USERNAME caracteres */
 #define DEFAUT_DB_PASSWORD             "seb"            /* Ne pas depasser TAILLE_DB_PASSWORD caractères */
 #define DEFAUT_DEBUG_LEVEL             0
 #define DEFAUT_TIMEOUT_CONNEXION       30               /* 30 secondes max pour se loguer sur le serveur */
 #define DEFAUT_TAILLE_BLOC_RESEAU      8192
 #define DEFAUT_HOME                    "/home/WatchdogHome"    /* Ne pas depasser TAILLE_HOME caracteres */
 #define DEFAUT_MAX_LOGIN_FAILED        3
 #define DEFAUT_CRYPTO_KEY              "My/Name/Is/Bond/"

/******************************************* Prototypes de fonctions **************************************/
 extern void Lire_config ( struct CONFIG *config, char *fichier_config );
 extern void Print_config ( struct CONFIG *config );

#endif
  
/*--------------------------------------------------------------------------------------------------------*/
