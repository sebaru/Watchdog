/**********************************************************************************************************/
/* Client/Include/Config_cli.h      Définitions de la structure de configuration cliente watchdog         */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 07 jun 2003 14:38:47 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

 #include "Cst_utilisateur.h"
 #include "Erreur.h"

 #define TAILLE_NOM_SERVEUR        40
 
 struct CONFIG_CLI
  { gint  port;                                                    /* Port d'ecoute des requetes clientes */
    gint  taille_bloc_reseau;
    gchar user[NBR_CARAC_LOGIN_UTF8+1]; 
    gchar serveur[TAILLE_NOM_SERVEUR+1];                                      /* Serveur Watchdog distant */
    gboolean ssl_crypt;                                                  /* Cryptage des transmissions ?? */
    guint debug_level;                                                    /* Niveau de debug du programme */
    guint max_login_failed;                                            /* Nombre maximum d'echec de login */
    struct LOG *log;                                                         /* Pour l'affichage des logs */
  };

 #define DEFAUT_FICHIER_CONFIG_CLI      "watchdog-client.conf"
 #define DEFAUT_USER                    "supervision"
 #define DEFAUT_SERVEUR                 "localhost"
 #define DEFAUT_PORT                    5558
 #define DEFAUT_SSL_CRYPT               0
 #define DEFAUT_DEBUG_LEVEL             (DEBUG_CONFIG+DEBUG_CRYPTO+DEBUG_INFO)
 #define DEFAUT_TAILLE_BLOC_RESEAU      8192
 #define DEFAUT_MAX_LOGIN_FAILED        3

/******************************************* Prototypes de fonctions **************************************/
 extern void Lire_config_cli ( struct CONFIG_CLI *config, char *fichier_config );
 extern void Print_config_cli ( struct CONFIG_CLI *config );

#endif
  
/*--------------------------------------------------------------------------------------------------------*/
