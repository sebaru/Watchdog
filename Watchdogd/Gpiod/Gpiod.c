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

/******************************************************************************************************************************/
/* GpiodCreer_DB : Creation de la database du process                                                                         */
/* Entrée: le pointeur sur la structure PROCESS                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Gpiod_Creer_DB ( struct PROCESS *lib )
  {
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'.", __func__, lib->database_version );

    SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                    "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                    "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                    "`uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
                    "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                    "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT' "
                    "FOREIGN KEY (`uuid`) REFERENCES `processes` (`uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                    ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", lib->name );

    SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s_IO` ("
                    "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                    "`date_create` datetime NOT NULL DEFAULT NOW(),"
                    "`gpiod_id` int(11) NOT NULL,"
                    "`num` INT(11) NOT NULL DEFAULT '0',"
                    "`mode_inout` INT(11) NOT NULL DEFAULT '0',"
                    "`mode_activelow` TINYINT(1) NOT NULL DEFAULT '0' "
                    "UNIQUE (gpiod_id, num),"
                    "FOREIGN KEY (`gpiod_id`) REFERENCES `%s` (`id`) ON DELETE CASCADE ON UPDATE CASCADE"
                    ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", lib->name, lib->name );

    Process_set_database_version ( lib, 1 );
  }
/******************************************************************************************************************************/
/* Charger_un_gpio: Charge une configuration de GPIO                                                                          */
/* Entrée: La structure Json representant le gpio                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_gpio (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct SUBPROCESS *module = user_data;
    struct GPIOD_VARS *vars = module->vars;

    gint num = Json_get_int ( element, "num" );
    if (num >= vars->num_lines)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                 "%s: GPIO%02d is out of range (>=%d)", __func__, num, vars->num_lines );
       return;
     }

    vars->lignes[num].mode_inout     = Json_get_int ( element, "mode_inout" );
    vars->lignes[num].mode_activelow = Json_get_int ( element, "mode_activelow" );
    vars->lignes[num].gpio_ligne     = gpiod_chip_get_line( vars->chip, num );
    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
              "%s: Chargement du GPIO%02d en mode_inout %d, mode_activelow=%d", __func__,
              num, vars->lignes[num].mode_inout, vars->lignes[num].mode_activelow );

    if (vars->lignes[num].mode_inout==0)
     { gpiod_line_request_input ( vars->lignes[num].gpio_ligne, "Watchdog GPIO INPUT Thread" );
       vars->lignes[num].etat = gpiod_line_get_value( vars->lignes[num].gpio_ligne );
     }
    else
     { gpiod_line_request_output( vars->lignes[num].gpio_ligne, "Watchdog GPIO OUTPUT Thread", vars->lignes[num].mode_activelow );
       vars->lignes[num].etat = !vars->lignes[num].mode_activelow;
     }

    if (Json_has_member ( element, "tech_id" ) && Json_has_member ( element, "acronyme" ))
     { g_snprintf ( vars->lignes[num].tech_id,  sizeof(vars->lignes[num].tech_id),  Json_get_string ( element, "tech_id" ) );
       g_snprintf ( vars->lignes[num].acronyme, sizeof(vars->lignes[num].acronyme), Json_get_string ( element, "acronyme" ) );
       Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                 "%s: GPIO%02d mappé sur '%s:%s'", __func__, num, vars->lignes[num].tech_id, vars->lignes[num].acronyme );
       vars->lignes[num].mapped = TRUE;
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: GPIO%02d not mapped", __func__, num );
       vars->lignes[num].mapped = FALSE;
     }
  }
/******************************************************************************************************************************/
/* Charger_tous_IO: Charge toutes les I/O                                                                                     */
/* Entrée: rien                                                                                                               */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Charger_tous_gpio ( struct SUBPROCESS *module )
  { JsonNode *RootNode = Json_node_create ();
    if (!RootNode) return(FALSE);

    gint gpiod_id = Json_get_int ( module->config, "id" );
    if (SQL_Select_to_json_node ( RootNode, "gpios",
                                  "SELECT * FROM %d_IO WHERE gpiod_id='%d' ORDER BY num", module->lib->name, gpiod_id ) == FALSE)
     { json_node_unref(RootNode);
       return(FALSE);
     }
    Json_node_foreach_array_element ( RootNode, "gpios", Charger_un_gpio, module );
    json_node_unref(RootNode);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct GPIOD_VARS) );
    struct GPIOD_VARS *vars = module->vars;

    gchar *tech_id  = Json_get_string ( module->config, "tech_id" );
    gint   gpiod_id = Json_get_int ( module->config, "id" );

    vars->chip = gpiod_chip_open_lookup("gpiochip0");
    if (!vars->chip)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: Error while loading chip 'gpiochip0'", __func__, tech_id );
       goto end;
     }
    else Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: %s: chip 'gpiochip0' loaded", __func__, tech_id );

    vars->num_lines = gpiod_chip_num_lines(vars->chip);
    if (vars->num_lines > GPIOD_MAX_LINE) vars->num_lines = GPIOD_MAX_LINE;
    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: %s: found %d lines", __func__, tech_id, vars->num_lines );

    for (gint cpt=0; cpt<vars->num_lines; cpt++)                                                        /* Valeurs par défaut */
     { SQL_Write_new ( "INSERT IGNORE INTO `%s_IO` SET gpiod_id=%d, num='%d', mode_inout='0', mode_activelow='0'",
                       module->lib->name, gpiod_id, cpt );
     }

    vars->lignes = g_try_malloc0 ( sizeof( struct GPIOD_LIGNE ) * vars->num_lines );
    if (!vars->lignes)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: Memory Error while loading lignes", __func__, tech_id );
       goto end;
     }

    if ( Charger_tous_gpio( module ) == FALSE )                                                         /* Chargement des I/O */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: Error while loading GPIO -> stop", __func__, tech_id );
       goto end;                                                                                /* Le thread ne tourne plus ! */
     }
    else Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: %s: %d GPIO Lines loaded", __func__, tech_id, vars->num_lines );

    gint last_top = 0, nbr_tour_par_sec = 0, nbr_tour = 0;                        /* Limitation du nombre de tour par seconde */
    while(module->lib->Thread_run == TRUE && module->lib->Thread_reload == FALSE)            /* On tourne tant que necessaire */
     { usleep(100000);
       usleep(vars->delai);
       sched_yield();

       SubProcess_send_comm_to_master_new ( module, module->comm_status );         /* Périodiquement envoie la comm au master */

       if (Partage->top>=last_top+10)                                                                /* Toutes les 1 secondes */
        { nbr_tour_par_sec = nbr_tour;
          nbr_tour = 0;
          if(nbr_tour_par_sec > 50) vars->delai += 50;
          else if(vars->delai>0) vars->delai -= 50;
          last_top = Partage->top;
        }

       for ( gint cpt = 0; cpt < vars->num_lines; cpt++ )
        { if (vars->lignes[cpt].mode_inout == 0) /* Ligne d'entrée ? */
           { gboolean etat = gpiod_line_get_value( vars->lignes[cpt].gpio_ligne );
             if (etat != vars->lignes[cpt].etat) /* Détection de changement */
              { vars->lignes[cpt].etat = etat;
                /*if (vars->lignes[cpt].mapped) Http_Post_to_local_BUS_DI ( module, vars->lignes[cpt].tech_id, vars->lignes[cpt].acronyme, etat );*/
                Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: %s: INPUT: GPIO%02d = %d", __func__, tech_id, cpt, etat );
                break;
              }
           }
        }

/************************************************** Ecoute du Master  *********************************************************/
       JsonNode *request;
       while ( (request = SubProcess_Listen_to_master_new ( module ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "SET_DO" ) )
           { if (!Json_has_member ( request, "tech_id"))
              { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: requete mal formée manque tech_id", __func__, tech_id ); }
             else if (!Json_has_member ( request, "acronyme" ))
              { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: requete mal formée manque acronyme", __func__, tech_id ); }
             else if (!Json_has_member ( request, "etat" ))
              { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: requete mal formée manque etat", __func__, tech_id ); }
             else
              { gchar *target_tech_id  = Json_get_string ( request, "tech_id" );
                gchar *target_acronyme = Json_get_string ( request, "acronyme" );
                gboolean etat   = Json_get_bool   ( request, "etat" );

                Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: %s: Recu SET_DO from bus: %s:%s",
                          __func__, tech_id, target_tech_id, target_acronyme );

                for ( gint cpt = 0; cpt < vars->num_lines; cpt++ )
                 { if (vars->lignes[cpt].mode_inout == 1 &&  /* Ligne de sortie ? */
                       !strcasecmp ( vars->lignes[cpt].tech_id, target_tech_id ) &&
                       !strcasecmp ( vars->lignes[cpt].acronyme, target_acronyme )
                      )
                    { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: %s: OUTPUT: GPIO%02d = %d", __func__, tech_id, cpt, etat );
                      gpiod_line_set_value ( vars->lignes[cpt].gpio_ligne, (vars->lignes[cpt].mode_activelow ? !etat : etat) );
                      break;
                    }
                 }
              }
           }
          json_node_unref (request);
        }
     }

end:
    for ( gint cpt=0; cpt < vars->num_lines; cpt++ )
     { if (vars->lignes[cpt].gpio_ligne) gpiod_line_release( vars->lignes[cpt].gpio_ligne ); }

    if (vars->lignes) g_free(vars->lignes);

    SubProcess_end(module);
  }
/******************************************************************************************************************************/
/* Run_process: Run du Process                                                                                                */
/* Entrée: la structure PROCESS associée                                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_process ( struct PROCESS *lib )
  {
reload:
    Gpiod_Creer_DB ( lib );                                                                    /* Création de la DB du thread */
    Thread_init ( "gpiod", "I/O", lib, WTD_VERSION, "Manage Gpiod I/O" );

    lib->config = Json_node_create();
    if(lib->config) SQL_Select_to_json_node ( lib->config, "subprocess", "SELECT * FROM %s WHERE uuid='%s'", lib->name, lib->uuid );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: %d subprocess to load", __func__, Json_get_int ( lib->config, "nbr_subprocess" ) );

    Json_node_foreach_array_element ( lib->config, "subprocess", Process_Load_one_subprocess, lib );   /* Chargement des modules */
    while( lib->Thread_run == TRUE && lib->Thread_reload == FALSE) sleep(1);                 /* On tourne tant que necessaire */
    Process_Unload_all_subprocess ( lib );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }

    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
