/**********************************************************************************************************/
/* Watchdogd/Include/Config.h        Définitions de la structure de configuration watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:23:17 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Config.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
  { gchar config_file[80];                                        /* Nom du fichier dont est issu les informations ci dessous */
    gchar run_as [ 40 ];                                                            /* Nom du user sur lequel tourne Watchdog */

    gint  db_port;
    gchar db_hostname[ TAILLE_DB_HOST+1 ];                                                /* Nom du host de la base de donnes */
    gchar db_username[ TAILLE_DB_USERNAME+1 ];                                /* Nom de l'administrateur de la base de données*/
    gchar db_database[ TAILLE_DB_DATABASE+1 ];                                              /* Chemin d'acces aux DB watchdog */
    gchar db_password[ TAILLE_DB_PASSWORD+1 ];                                              /* Mot de passe de connexion ODBC */

    gboolean installed;                                                                    /* TRUE si la config a pu etre lue */
    gchar home [ TAILLE_HOME+1 ];                                                              /* Repertoire maison du daemon */
    gchar librairie_dir [ TAILLE_HOME+1 ];                                   /* Repertoire de stockage des libraires watchdog */
    gboolean instance_is_master;                                               /* TRUE si l'instance est l'instance maitresse */
    gchar master_host[ TAILLE_DB_HOST+1 ];
    guint    log_level;                                                                       /* Niveau de debug du programme */
    gboolean log_msrv;                                                                                /* TRUE si log_override */
    gboolean log_arch;                                                                                /* TRUE si log_override */
    gboolean log_db;                                                                              /* TRUE si log des acces DB */
    gboolean log_zmq;                                                                            /* TRUE si log des acces ZMQ */
    gboolean log_trad;                                                                    /* TRUE si log des compilations DLS */
    gboolean single;                                                                                /* Demarrage des thread ? */
    struct LOG *log;                                                                             /* Pour l'affichage des logs */
  };

 #define DEFAUT_DB_HOST                 "localhost"          /* Ne pas depasser TAILLE_DB_HOST caracteres */
 #define DEFAUT_DB_DATABASE             "WatchdogDB"         /* Ne pas depasser TAILLE_DB_NAME caracteres */
 #define DEFAUT_DB_USERNAME             "watchdog"    /* Ne pas depasser TAILLE_ADMIN_USERNAME caracteres */
 #define DEFAUT_DB_PASSWORD             "seb"            /* Ne pas depasser TAILLE_DB_PASSWORD caractères */
 #define DEFAUT_DB_PORT                 3306
 #define DEFAUT_LIBRAIRIE_DIR           "/usr/local/lib"        /* Ne pas depasser TAILLE_HOME caracteres */

/******************************************* Prototypes de fonctions **************************************/
 extern gboolean Lire_config ( void );
 extern void Print_config ( void );
 extern gboolean Modifier_configDB ( gchar *nom_thread, gchar *nom, gchar *valeur );
 extern gboolean Modifier_configDB_int ( gchar *nom_thread, gchar *nom, gint valeur );
 extern gboolean Creer_configDB ( gchar *nom_thread, gchar *nom, gchar *valeur );
 extern gboolean Creer_configDB_int ( gchar *nom_thread, gchar *nom, gint valeur );
 extern gboolean Recuperer_configDB ( struct DB **db_retour, gchar *nom_thread );
 extern gboolean Recuperer_configDB_suite( struct DB **db_orig, gchar **nom, gchar **valeur );
 extern gchar *Recuperer_configDB_by_nom ( gchar *nom_thread, gchar *nom_param );
#endif

/*--------------------------------------------------------------------------------------------------------*/
