/**********************************************************************************************************/
/* Client/Config_cli/config.h        Définitions de la structure de configuration cliente watchdog        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 07 jun 2003 14:38:47 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _CONFIG_CLI_H_
 #define _CONFIG_CLI_H_

 #include "Erreur.h"

 #define TAILLE_NOM_SERVEUR        40
 
 struct CONFIG_CLI
  { gint  port;                                                    /* Port d'ecoute des requetes clientes */
    gint  taille_bloc_reseau;
    gchar serveur[TAILLE_NOM_SERVEUR+1];                                      /* Serveur Watchdog distant */
    gboolean ssl_crypt;                                                  /* Cryptage des transmissions ?? */
    guint debug_level;                                                    /* Niveau de debug du programme */
    guint max_login_failed;                                            /* Nombre maximum d'echec de login */
    struct LOG *log;                                                         /* Pour l'affichage des logs */
  };

 #define DEFAUT_FICHIER_CONFIG_CLI      "watchdog-client.conf"
 #define DEFAUT_SERVEUR                 "SalleCTI.watchdog.fr"
 #define DEFAUT_PORT                    5558
 #define DEFAUT_SSL_CRYPT               0
 #define DEFAUT_DEBUG_LEVEL             0
 #define DEFAUT_TAILLE_BLOC_RESEAU      8192
 #define DEFAUT_MAX_LOGIN_FAILED        3

/******************************************* Prototypes de fonctions **************************************/
 extern void Lire_config_cli ( struct CONFIG_CLI *config, char *fichier_config );
 extern void Print_config_cli ( struct CONFIG_CLI *config );

#endif
  
/*--------------------------------------------------------------------------------------------------------*/
