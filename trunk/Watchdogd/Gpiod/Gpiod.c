/******************************************************************************************************************************/
/* Watchdogd/Gpiod/Gpiod.c  Gestion des I/O Gpiod  Watchdog 3.0                                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    03.09.2021 17:51:06 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Gpiod.c
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

 #include <glib.h>
 #include <sys/prctl.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Gpiod.h"

 static struct GPIOD_CONFIG Cfg;
/******************************************************************************************************************************/
/* Gpiod_Creer_DB: Creer la base de données du thread                                                                         */
/* Entrée: rien                                                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Gpiod_Creer_DB ( struct LIBRAIRIE *lib )
  { gint database_version;

    gchar *database_version_string = Recuperer_configDB_by_nom( Cfg.lib->name, "database_version" );
    if (database_version_string)
     { database_version = atoi( database_version_string );
       g_free(database_version_string);
     } else database_version=0;

    Info_new( Config.log, Cfg.lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'. Thread_Version '%s'.", __func__, database_version, WTD_VERSION );

    if (database_version==0)
     { SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `gpiod` ("
                       "`uuid` VARCHAR(37) COLLATE utf8_unicode_ci PRIMARY KEY,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "`tech_id` VARCHAR(32) NULL"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `gpiod` ("
                       "`id` INT(11) NOT NULL AUTO_INCREMENT,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "`uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
                       "`num` INT(11) NOT NULL DEFAULT '0',"
                       "`mode_inout` INT(11) NOT NULL DEFAULT '0',"
                       "`mode_activelow` TINYINT(1) NOT NULL DEFAULT '0',"
                       "`tech_id` VARCHAR(32) NULL DEFAULT NULL,"
                       "`acronyme` VARCHAR(64) NULL DEFAULT NULL,"
                       "PRIMARY KEY (`id`),"
                       "FOREIGN KEY (`uuid`) REFERENCES `gpiod` (uuid) ON DELETE CASCADE ON UPDATE CASCADE,"
                       "UNIQUE (`uuid`,`num`),"
                       "UNIQUE (`tech_id`,`acronyme`)"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       goto end;
     }

end:
    SQL_Write_new ("INSERT IGNORE INTO `gpiod` SET uuid='%s'", lib->uuid );
    database_version = 1;
    Modifier_configDB_int ( lib->name, "database_version", database_version );
  }
/******************************************************************************************************************************/
/* Charger_un_gpio: Charge une configuration de GPIO                                                                          */
/* Entrée: La structure Json representant le gpio                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_gpio (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct LIBRAIRIE *lib = user_data;
    gint num            = Json_get_int ( element, "num" );
    gint mode_inout     = Json_get_int ( element, "mode_inout" );
    gint mode_activelow = Json_get_int ( element, "mode_activelow" );
    Info_new( Config.log, lib->Thread_debug, LOG_INFO,
              "%s: Chargement du GPIO%02d en mode_inout %d, mode_activelow=%d", __func__, num, mode_inout, mode_activelow );

    Cfg.lignes[num].gpio_ligne     = gpiod_chip_get_line( Cfg.chip, num );
    Cfg.lignes[num].mode_inout     = mode_inout;
    Cfg.lignes[num].mode_activelow = mode_activelow;

    if (mode_inout==0)
     { gpiod_line_request_input ( Cfg.lignes[num].gpio_ligne, "Watchdog RPI Thread" );
       Cfg.lignes[num].etat = gpiod_line_get_value( Cfg.lignes[num].gpio_ligne );
     }
    else
     { gpiod_line_request_output( Cfg.lignes[num].gpio_ligne, "Watchdog RPI Thread", mode_activelow );
       Cfg.lignes[num].etat = mode_activelow;
     }

    if (Json_has_member ( element, "tech_id" ) && Json_has_member ( element, "acronyme" ))
     { g_snprintf ( Cfg.lignes[num].tech_id,  sizeof(Cfg.lignes[num].tech_id),  Json_get_string ( element, "tech_id" ) );
       g_snprintf ( Cfg.lignes[num].acronyme, sizeof(Cfg.lignes[num].acronyme), Json_get_string ( element, "acronyme" ) );
       Info_new( Config.log, lib->Thread_debug, LOG_INFO,
                 "%s: GPIO%02d mappé sur '%s:%s'", __func__, num, Cfg.lignes[num].tech_id, Cfg.lignes[num].acronyme );
       Cfg.lignes[num].mapped = TRUE;
     }
    else
     { Info_new( Config.log, lib->Thread_debug, LOG_DEBUG, "%s: GPIO%02d not mapped", __func__, num );
       Cfg.lignes[num].mapped = FALSE;
     }
  }
/******************************************************************************************************************************/
/* Charger_tous_IO: Charge toutes les I/O                                                                                     */
/* Entrée: rien                                                                                                               */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Charger_tous_gpio ( struct LIBRAIRIE *lib )
  { JsonNode *RootNode = Json_node_create ();
    if (!RootNode) return(FALSE);

    if (SQL_Select_to_json_node ( RootNode, "gpios", "SELECT * FROM %s_io WHERE uuid='%s'", Cfg.lib->name, lib->uuid ) == FALSE)
     { json_node_unref(RootNode);
       return(FALSE);
     }
    Json_node_foreach_array_element ( RootNode, "gpios", Charger_un_gpio, lib );
    json_node_unref(RootNode);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du thread Gpiod                                                                                  */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  {
reload:
    memset( &Cfg, 0, sizeof(Cfg) );                                                 /* Mise a zero de la structure de travail */
    Cfg.lib = lib;                                                 /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "gpiod", "I/O", lib, WTD_VERSION, "Manage Gpiod I/O" );
    Gpiod_Creer_DB ( lib );                                                                 /* Création de la base de données */

    Cfg.chip = gpiod_chip_open_lookup("gpiochip0");
    if (!Cfg.chip)
     { Info_new( Config.log, Cfg.lib->Thread_debug, LOG_ERR, "%s: Error while loading chip 'gpiochip0'", __func__ );
       Cfg.lib->Thread_run = FALSE;                                                             /* Le thread ne tourne plus ! */
       goto end;
     }
    else Info_new( Config.log, Cfg.lib->Thread_debug, LOG_NOTICE, "%s: chip 'gpiochip0' loaded", __func__ );

    Cfg.num_lines = gpiod_chip_num_lines(Cfg.chip);
    if (Cfg.num_lines > GPIOD_MAX_LINE) Cfg.num_lines = GPIOD_MAX_LINE;
    Info_new( Config.log, Cfg.lib->Thread_debug, LOG_INFO, "%s: found %d lines", __func__, Cfg.num_lines );

    for (gint cpt=0; cpt<Cfg.num_lines; cpt++)                                                                    /* Valeurs par défaut */
     { SQL_Write_new ( "INSERT IGNORE INTO `%s_io` SET uuid='%s', num='%d', mode_inout='0', mode_activelow='0'",
                       lib->name, lib->uuid, cpt );
     }

    if ( Charger_tous_gpio( lib ) == FALSE )                                                            /* Chargement des I/O */
     { Info_new( Config.log, lib->Thread_debug, LOG_ERR, "%s: Error while loading GPIO -> stop", __func__ );
       Cfg.lib->Thread_run = FALSE;                                                             /* Le thread ne tourne plus ! */
     }
    else Info_new( Config.log, lib->Thread_debug, LOG_INFO, "%s: %d GPIO Lines loaded", __func__, Cfg.num_lines );

    gint last_top = 0, nbr_tour_par_sec = 0, nbr_tour = 0;                        /* Limitation du nombre de tour par seconde */
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { sched_yield();
       usleep(Cfg.delai);

       if (Partage->top>=last_top+10)                                                                /* Toutes les 1 secondes */
        { nbr_tour_par_sec = nbr_tour;
          nbr_tour = 0;
          if(nbr_tour_par_sec > 50) Cfg.delai += 50;
          else if(Cfg.delai>0) Cfg.delai -= 50;
          last_top = Partage->top;
        }

       for ( gint cpt = 0; cpt < Cfg.num_lines; cpt++ )
        { if (Cfg.lignes[cpt].mode_inout == 0) /* Ligne d'entrée ? */
           { gboolean etat = gpiod_line_get_value( Cfg.lignes[cpt].gpio_ligne );
             if (etat != Cfg.lignes[cpt].etat) /* Détection de changement */
              { Cfg.lignes[cpt].etat = etat;
                if (Cfg.lignes[cpt].mapped) Zmq_Send_DI_to_master ( lib, Cfg.lignes[cpt].tech_id, Cfg.lignes[cpt].acronyme, etat );
                Info_new( Config.log, lib->Thread_debug, LOG_DEBUG, "%s: INPUT: GPIO%02d = %d", __func__, cpt, etat );
                break;
              }
           }
        }

       JsonNode *request;                                                                          /* Ecoute du Master Server */
       while ( (request = Thread_Listen_to_master ( lib ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "SET_DO" ) )
           { if (!Json_has_member ( request, "tech_id"))
              { Info_new( Config.log, lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque tech_id", __func__ ); }
             else if (!Json_has_member ( request, "acronyme" ))
              { Info_new( Config.log, Cfg.lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque acronyme", __func__ ); }
             else if (!Json_has_member ( request, "etat" ))
              { Info_new( Config.log, Cfg.lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque etat", __func__ ); }
             else
              { gchar *tech_id  = Json_get_string ( request, "tech_id" );
                gchar *acronyme = Json_get_string ( request, "acronyme" );
                gboolean etat   = Json_get_bool   ( request, "etat" );

                Info_new( Config.log, Cfg.lib->Thread_debug, LOG_DEBUG, "%s: Recu SET_DO from bus: %s:%s",
                          __func__, tech_id, acronyme );

                for ( gint cpt = 0; cpt < Cfg.num_lines; cpt++ )
                 { if (Cfg.lignes[cpt].mode_inout == 1 &&  /* Ligne de sortie ? */
                       !strcasecmp ( Cfg.lignes[cpt].tech_id, tech_id ) &&
                       !strcasecmp ( Cfg.lignes[cpt].acronyme, acronyme )
                      )
                    { gpiod_line_set_value ( Cfg.lignes[cpt].gpio_ligne, etat );
                      Info_new( Config.log, lib->Thread_debug, LOG_DEBUG, "%s: OUTPUT: GPIO%02d = %d", __func__, cpt, etat );
                      break;
                    }
                 }
              }
           }
          json_node_unref (request);
        }
     }

    for ( gint cpt=0; cpt < sizeof(Cfg.num_lines); cpt++ )
     { if (Cfg.lignes[cpt].gpio_ligne) gpiod_line_release( Cfg.lignes[cpt].gpio_ligne ); }

end:
    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
