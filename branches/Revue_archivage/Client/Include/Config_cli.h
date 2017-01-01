/******************************************************************************************************************************/
/* Client/Include/Config_cli.h      Définitions de la structure de configuration cliente watchdog                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          sam 07 jun 2003 14:38:47 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Config_cli.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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

 #ifndef _CONFIG_CLI_H_
 #define _CONFIG_CLI_H_

 #include "Reseaux.h"
 #include "Erreur.h"

 #define TAILLE_NOM_SERVEUR        40
 
 struct CONFIG_CLI
  { gchar host[TAILLE_NOM_SERVEUR+1];                                                             /* Serveur Watchdog distant */
    gchar user[NBR_CARAC_LOGIN_UTF8+1]; 
    gchar passwd[TAILLE_NOM_SERVEUR+1];                                                           /* Serveur Watchdog distant */
    guint port_ihm;                                                              /* Port TCP du service IHM (5558 par défaut) */
    gchar ssl_file_ca[80];                                                                                  /* Paramètres SSL */
    guint log_override;                                                         /* Pour afficher tous les informations de log */
    guint log_level;                                                                          /* Niveau de debug du programme */
    gboolean gui_tech;                                                   /* True si la GUI doit présenter l'aspect Technicien */
    gboolean gui_fullscreen;                                                     /* True si la GUI doit etre sur tout l'ecran */
    struct LOG *log;                                                                             /* Pour l'affichage des logs */
  };

 #define DEFAUT_FICHIER_CONFIG_CLI      "watchdog-client.conf"
 #define DEFAUT_SERVEUR                 "localhost"
 #define DEFAUT_USER                    "supervision"
 #define DEFAUT_PASSWD                  "supervision"
 #define DEFAUT_PORT_IHM                5558
 #define DEFAUT_LOG_LEVEL               LOG_INFO
 #define DEFAUT_TAILLE_BLOC_RESEAU      8192
 #define DEFAUT_SSL_FILE_CA            "cacert.pem"         /* Certificat de l'autorite de certification */

/*********************************************** Prototypes de fonctions ******************************************************/
 extern void Lire_config_cli ( struct CONFIG_CLI *config, char *fichier_config );
 extern void Print_config_cli ( struct CONFIG_CLI *config );

#endif
  
/*----------------------------------------------------------------------------------------------------------------------------*/
