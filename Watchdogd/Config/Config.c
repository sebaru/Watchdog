/**********************************************************************************************************/
/* Watchdogd/Config/Config.c        Lecture du fichier de configuration Watchdog                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 22 jun 2004 17:44:24 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <stdio.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <string.h>

 #include "Config.h"
 #include "Config_lignes.h"

 extern void Interpreter_config ( FILE *rc, struct CONFIG *config );

/**********************************************************************************************************/
/* Lire_config: lecture et prise en compte de la configuration serveur Watchdog                           */
/* Entrée: La structure à remplir, le nom du fichier originel                                             */
/**********************************************************************************************************/
 void Lire_config ( struct CONFIG *config, char *fichier_config )
  { char *fichier;
    FILE *rc;
    int cpt;

    if (!config) return;

    config->port                  = DEFAUT_PORT;
    config->max_client            = DEFAUT_MAX_CLIENT;
    config->min_serveur           = DEFAUT_MIN_SERVEUR;
    config->max_serveur           = DEFAUT_MAX_SERVEUR;
    config->max_inactivite        = DEFAUT_MAX_INACTIVITE;
    config->taille_clef_dh        = DEFAUT_TAILLE_CLEF_DH;
    config->taille_clef_rsa       = DEFAUT_TAILLE_CLEF_RSA;
    config->ssl_crypt             = DEFAUT_SSL_CRYPT;
    config->max_msg_visu          = DEFAUT_MAX_MSG_VISU;
    config->taille_bloc_reseau    = DEFAUT_TAILLE_BLOC_RESEAU;
    config->debug_level           = DEFAUT_DEBUG_LEVEL;
    config->timeout_connexion     = DEFAUT_TIMEOUT_CONNEXION;
    config->max_login_failed      = DEFAUT_MAX_LOGIN_FAILED;
    snprintf( config->port_RS485,  sizeof(config->port_RS485),  "%s", DEFAUT_PORT_RS485  );
    snprintf( config->crypto_key,  sizeof(config->crypto_key),  "%s", DEFAUT_CRYPTO_KEY  );
    snprintf( config->home,        sizeof(config->home),        "%s", DEFAUT_HOME        );
    snprintf( config->db_database, sizeof(config->db_database), "%s", DEFAUT_DB_DATABASE );
    snprintf( config->db_password, sizeof(config->db_password), "%s", DEFAUT_DB_PASSWORD );
    snprintf( config->db_username, sizeof(config->db_username), "%s", DEFAUT_DB_USERNAME );
    config->db_port               = DEFAUT_DB_PORT;

    for ( cpt=0; cpt < NBR_ID_RS485; cpt++)
     { config->module_rs485[cpt].id = -1; }

    if (!fichier_config) fichier = DEFAUT_FICHIER_CONFIG_SRV;
                    else fichier = fichier_config;
    rc = fopen( fichier, "r");
    if (rc)
     { Interpreter_config(rc, config);
       fclose(rc);
     }
  }
/**********************************************************************************************************/
/* Print_config: Affichage (enfin log) la config actuelle en parametre                                    */
/* Entrée: une config !! -> le champ log doit etre initialisé via la librairie Erreur                     */
/**********************************************************************************************************/
 void Print_config ( struct CONFIG *config )
  { int cpt;

    if (!config->log) return;
    Info_n( config->log, DEBUG_CONFIG, "Config port                 ", config->port );
    Info_c( config->log, DEBUG_CONFIG, "Config port rs485           ", config->port_RS485  );
    Info_n( config->log, DEBUG_CONFIG, "Config max client           ", config->max_client );
    Info_n( config->log, DEBUG_CONFIG, "Config min serveur          ", config->min_serveur );
    Info_n( config->log, DEBUG_CONFIG, "Config max serveur          ", config->max_serveur );
    Info_n( config->log, DEBUG_CONFIG, "Config max inactivite       ", config->max_inactivite );
    Info_n( config->log, DEBUG_CONFIG, "Config taille_clef_dh       ", config->taille_clef_dh );
    Info_n( config->log, DEBUG_CONFIG, "Config taille_clef_rsa      ", config->taille_clef_rsa );
    Info_n( config->log, DEBUG_CONFIG, "Config ssl crypt            ", config->ssl_crypt );
    Info_n( config->log, DEBUG_CONFIG, "Config max msg visu         ", config->max_msg_visu );
    Info_n( config->log, DEBUG_CONFIG, "Config debug level          ", config->debug_level );
    Info_n( config->log, DEBUG_CONFIG, "Config taille_bloc_reseau   ", config->taille_bloc_reseau );
    Info_n( config->log, DEBUG_CONFIG, "Config timeout connexion    ", config->timeout_connexion );
    Info_n( config->log, DEBUG_CONFIG, "Config max login failed     ", config->max_login_failed );
    Info_c( config->log, DEBUG_CONFIG, "Config home                 ", config->home );
    Info_c( config->log, DEBUG_CONFIG, "Config db database          ", config->db_database );
    Info_c( config->log, DEBUG_CONFIG, "Config db username          ", config->db_username );
    Info_c( config->log, DEBUG_CONFIG, "Config db password          ", config->db_password );
    Info_n( config->log, DEBUG_CONFIG, "Config db port              ", config->db_port );
    Info_c( config->log, DEBUG_CONFIG, "Config crypto key           ", config->crypto_key );
    for ( cpt=0; cpt < NBR_ID_RS485; cpt++)
     { if (config->module_rs485[cpt].id != -1)
        { Info_n( config->log, DEBUG_CONFIG, "Config Module RS485    id = ", config->module_rs485[cpt].id );
          if (config->module_rs485[cpt].ea_min != -1)
           { Info_n( config->log, DEBUG_CONFIG, "                   ea_min = ", config->module_rs485[cpt].ea_min );
             Info_n( config->log, DEBUG_CONFIG, "                   ea_max = ", config->module_rs485[cpt].ea_max );
           }
          if (config->module_rs485[cpt].e_min != -1)
           { Info_n( config->log, DEBUG_CONFIG, "                    e_min = ", config->module_rs485[cpt].e_min );
             Info_n( config->log, DEBUG_CONFIG, "                    e_max = ", config->module_rs485[cpt].e_max );
           }
          if (config->module_rs485[cpt].ec_min != -1)
           { Info_n( config->log, DEBUG_CONFIG, "                   ec_min = ", config->module_rs485[cpt].ec_min );
             Info_n( config->log, DEBUG_CONFIG, "                   ec_max = ", config->module_rs485[cpt].ec_max );
           }
          if (config->module_rs485[cpt].s_min != -1)
           { Info_n( config->log, DEBUG_CONFIG, "                    s_min = ", config->module_rs485[cpt].s_min );
             Info_n( config->log, DEBUG_CONFIG, "                    s_max = ", config->module_rs485[cpt].s_max );
           }
          if (config->module_rs485[cpt].sa_min != -1)
           { Info_n( config->log, DEBUG_CONFIG, "                   sa_min = ", config->module_rs485[cpt].sa_min );
             Info_n( config->log, DEBUG_CONFIG, "                   sa_max = ", config->module_rs485[cpt].sa_max );
           }
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
