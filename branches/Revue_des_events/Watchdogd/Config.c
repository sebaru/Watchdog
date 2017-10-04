/******************************************************************************************************************************/
/* Watchdogd/Config/Config.c        Lecture du fichier de configuration Watchdog                                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         mer. 15 déc. 2010 13:30:12 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */

/******************************************************************************************************************************/
/* Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                        */
/* Entrée: le nom de fichier à lire                                                                                           */
/* Sortie: La structure mémoire est à jour                                                                                    */
/******************************************************************************************************************************/
 void Lire_config ( char *fichier_config )
  { gchar *chaine, *fichier;
	   gint num;
    GError *error = NULL;
    GKeyFile *gkf;

    g_snprintf( Config.home,          sizeof(Config.home), "%s", g_get_home_dir() );
    g_snprintf( Config.instance_id,   sizeof(Config.instance_id), "%s", DEFAUT_GLOBAL_ID  );
    g_snprintf( Config.run_as,        sizeof(Config.run_as), "%s", g_get_user_name() );
    g_snprintf( Config.librairie_dir, sizeof(Config.librairie_dir), "%s", DEFAUT_LIBRAIRIE_DIR   );

    Config.instance_is_master = TRUE;
    Config.log_db             = FALSE;
    Config.db_port            = DEFAUT_DB_PORT;

    g_snprintf( Config.db_host,     sizeof(Config.db_host),     "%s", DEFAUT_DB_HOST  );
    g_snprintf( Config.db_database, sizeof(Config.db_database), "%s", DEFAUT_DB_DATABASE  );
    g_snprintf( Config.db_password, sizeof(Config.db_password), "%s", DEFAUT_DB_PASSWORD  );
    g_snprintf( Config.db_username, sizeof(Config.db_username), "%s", DEFAUT_DB_USERNAME  );

    Config.log_level = LOG_NOTICE;
    Config.log_msrv  = FALSE;
    Config.log_dls   = FALSE;
    Config.log_arch  = FALSE;


    if (!fichier_config) fichier = DEFAUT_FICHIER_CONFIG_SRV;                          /* Lecture du fichier de configuration */
                    else fichier = fichier_config;
    printf("Using config file %s\n", fichier );
    gkf = g_key_file_new();

    if (g_key_file_load_from_file(gkf, fichier, G_KEY_FILE_NONE, &error))
     {
       g_snprintf( Config.config_file, sizeof(Config.config_file), "%s", fichier );
/******************************************************* Partie GLOBAL ********************************************************/
       chaine = g_key_file_get_string ( gkf, "GLOBAL", "home", NULL );
       if (chaine)
        { g_snprintf( Config.home, sizeof(Config.home), "%s", chaine ); g_free(chaine); }

       chaine = g_key_file_get_string ( gkf, "GLOBAL", "instance_id", NULL );
       if (chaine)
        { g_snprintf( Config.instance_id, sizeof(Config.instance_id), "%s", chaine ); g_free(chaine); }

       chaine = g_key_file_get_string ( gkf, "GLOBAL", "run_as", NULL );
       if (chaine)
        { g_snprintf( Config.run_as, sizeof(Config.run_as), "%s", chaine ); g_free(chaine); }

       chaine = g_key_file_get_string ( gkf, "GLOBAL", "library_dir", NULL );
       if (chaine)
        { g_snprintf( Config.librairie_dir, sizeof(Config.librairie_dir), "%s", chaine ); g_free(chaine); }

       Config.instance_is_master = g_key_file_get_boolean ( gkf, "GLOBAL", "instance_is_master", NULL );

/******************************************************** Partie DATABASE *****************************************************/
       Config.log_db = g_key_file_get_boolean ( gkf, "DATABASE", "debug", NULL );
       num           = g_key_file_get_integer ( gkf, "DATABASE", "port", NULL );
       if (num) Config.db_port = num;

       chaine = g_key_file_get_string ( gkf, "DATABASE", "host", NULL );
       if (chaine)
        { g_snprintf( Config.db_host, sizeof(Config.db_host), "%s", chaine ); g_free(chaine); }

       chaine = g_key_file_get_string ( gkf, "DATABASE", "database", NULL );
       if (chaine)
        { g_snprintf( Config.db_database, sizeof(Config.db_database), "%s", chaine ); g_free(chaine); }

       chaine = g_key_file_get_string ( gkf, "DATABASE", "password", NULL );
       if (chaine)
        { g_snprintf( Config.db_password, sizeof(Config.db_password), "%s", chaine ); g_free(chaine); }

       chaine = g_key_file_get_string ( gkf, "DATABASE", "username", NULL );
       if (chaine)
        { g_snprintf( Config.db_username, sizeof(Config.db_username), "%s", chaine ); g_free(chaine); }


/******************************************************** Partie LOG **********************************************************/
       chaine = g_key_file_get_string ( gkf, "LOG", "log_level", NULL );
       if (chaine)
        {      if ( ! strcmp( chaine, "debug"    ) ) Config.log_level = LOG_DEBUG;
          else if ( ! strcmp( chaine, "info"     ) ) Config.log_level = LOG_INFO;
          else if ( ! strcmp( chaine, "notice"   ) ) Config.log_level = LOG_NOTICE;
          else if ( ! strcmp( chaine, "warning"  ) ) Config.log_level = LOG_WARNING;
          else if ( ! strcmp( chaine, "error"    ) ) Config.log_level = LOG_ERR;
          else if ( ! strcmp( chaine, "critical" ) ) Config.log_level = LOG_CRIT;
          g_free(chaine);
        }

       Config.log_msrv = g_key_file_get_boolean ( gkf, "LOG", "debug_msrv", NULL );
       Config.log_dls  = g_key_file_get_boolean ( gkf, "LOG", "debug_dls", NULL );
       Config.log_arch = g_key_file_get_boolean ( gkf, "LOG", "debug_arch", NULL );
     }
    else 
     { printf("Unable to parse config file %s, error %s\n", fichier, error->message );
       g_error_free( error );
     }
    g_key_file_free(gkf);
  }
/******************************************************************************************************************************/
/* Print_config: Affichage (enfin log) la config actuelle en parametre                                                        */
/* Entrée: une config !! -> le champ log doit etre initialisé via la librairie Erreur                                         */
/******************************************************************************************************************************/
 void Print_config ( void )
  { 
    if (!Config.log) return;
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config file                 %s", Config.config_file );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config run_as               %s", Config.run_as );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config log_level            %d", Config.log_level );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config home                 %s", Config.home );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config instance_id          %s", Config.instance_id );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config instance is master   %d", Config.instance_is_master );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config librairie_dir        %s", Config.librairie_dir );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config db host              %s", Config.db_host );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config db database          %s", Config.db_database );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config db username          %s", Config.db_username );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config db password          *******" );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config db port              %d", Config.db_port );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config db debug             %d", Config.log_db );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config compil               %d", Config.compil );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Config single               %d", Config.single );
  }
/******************************************************************************************************************************/
/* Retirer_configDB: Elimination d'un parametre de configuration                                                              */
/* Entrée: un log et une database                                                                                             */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_configDB ( gchar *nom_thread, gchar *nom )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE instance_id = '%s' AND nom_thread='%s' AND nom = '%s'",
                 NOM_TABLE_CONFIG, Config.instance_id, nom_thread, nom );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_configDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_configDB: Ajout ou edition d'un message                                                                            */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Modifier_configDB ( gchar *nom_thread, gchar *nom, gchar *valeur )
  { gchar requete[2048];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_configDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
               "INSERT INTO %s(instance_id,nom_thread,nom,valeur) VALUES "
               "('%s','%s','%s','%s') ON DUPLICATE KEY UPDATE valeur='%s';",
               NOM_TABLE_CONFIG, Config.instance_id,
               nom_thread, nom, valeur, valeur
              );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Recuperer_configDB : Récupration de la configuration en base pour une instance_id donnée                                   */
/* Entrée: une database de retour et le nom de l'instance_id                                                                  */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Recuperer_configDB ( struct DB **db_retour, gchar *nom_thread )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT nom,valeur"
                " FROM %s"
                " WHERE instance_id = '%s' AND nom_thread='%s' ORDER BY nom,valeur",
                NOM_TABLE_CONFIG, Config.instance_id, nom_thread
              );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_configDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_configDB_suite: Continue la récupération des paramètres de configuration dans la base                            */
/* Entrée: une database                                                                                                       */
/* Sortie: FALSE si plus d'enregistrement                                                                                     */
/******************************************************************************************************************************/
 gboolean Recuperer_configDB_suite( struct DB **db_orig, gchar **nom, gchar **valeur )
  { struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(FALSE);
     }

    *nom =  db->row[0];
    *valeur = db->row[1];
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
