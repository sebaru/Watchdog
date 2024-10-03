/******************************************************************************************************************************/
/* Watchdogd/Include/Config.h        Définitions de la structure de configuration watchdog                                    */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                      lun 02 jun 2003 14:23:17 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Config.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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

 #define NOM_TABLE_CONFIG         "config"                                              /* Nom de la table en base de données */

 #define TAILLE_HOME              80                                                               /* Chemin HOME de watchdog */

 extern struct CONFIG Config;                                /* Parametre de configuration du serveur via /etc/watchdogd.conf */

  struct CONFIG
  { JsonNode *config;                                                           /* imported from /etc/abls-habitat-agent.conf */
    gchar config_file[80];                                        /* Nom du fichier dont est issu les informations ci dessous */
    gboolean headless;                                                                          /* Headless instance or not ? */

    gchar home [ TAILLE_HOME+1 ];                                                              /* Repertoire maison du daemon */
    gchar librairie_dir [ TAILLE_HOME+1 ];                                   /* Repertoire de stockage des libraires watchdog */
    gboolean instance_is_master;                                               /* TRUE si l'instance est l'instance maitresse */
    gchar master_hostname[ 32 ];
    gchar mqtt_hostname[ 32 ];                                                                /* Hostname du serveur MQTT_API */
    guint mqtt_port;                                                                 /* Port de connexion du serveur MQTT_API */
    gboolean mqtt_over_ssl;                                                                          /* TRUE si MQTT over SSL */
    gchar mqtt_password[ 129 ];
    guint    log_level;                                                                       /* Niveau de debug du programme */
    gboolean log_msrv;                                                                                    /* TRUE si log_msrv */
    gboolean log_bus;                                                                                      /* TRUE si log_bus */
    gboolean log_dls;                                                                                      /* TRUE si log_dls */
    gboolean single;                                                                                /* Demarrage des thread ? */
  };

 #define DEFAUT_PROCESS_DIR           "/usr/local/lib"        /* Ne pas depasser TAILLE_HOME caracteres */

/******************************************* Prototypes de fonctions **************************************/
 extern void Lire_config ( int argc, char *argv[] );
 extern void Print_config ( void );
#endif

/*--------------------------------------------------------------------------------------------------------*/
