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
 static void Gpiod_Creer_DB ( void )
  { gint database_version;

    gchar *database_version_string = Recuperer_configDB_by_nom( Cfg.lib->name, "database_version" );
    if (database_version_string)
     { database_version = atoi( database_version_string );
       g_free(database_version_string);
     } else database_version=0;

    Info_new( Config.log, Cfg.lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'. Thread_Version '%s'.", __func__, database_version, WTD_VERSION );

    if (database_version==0)
     { SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                       "`id` int(11) NOT NULL AUTO_INCREMENT,"
                       "`date_create` datetime NOT NULL DEFAULT NOW(),"
                       "`instance` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'localhost',"
                       "`gpio` INT(11) NOT NULL DEFAULT '0',"
                       "`mode_inout` INT(11) NOT NULL DEFAULT '0',"
                       "`mode_activelow` TINYINT(1) NOT NULL DEFAULT '0',"
                       "PRIMARY KEY (`id`),"
                       "UNIQUE (`instance`,`gpio`)"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", Cfg.lib->name );
       goto end;
     }

end:
    for (gint cpt=0; cpt<27; cpt++)                                                                     /* Valeurs par défaut */
     { SQL_Write_new ( "INSERT IGNORE INTO %s SET instance='%s', gpio='%d', mode_inout='0', mode_activelow='0'",
                       Cfg.lib->name, g_get_host_name(), cpt );
     }
    database_version = 1;
    Modifier_configDB_int ( Cfg.lib->name, "database_version", database_version );
  }
/******************************************************************************************************************************/
/* Charger_un_gpio: Charge une configuration de GPIO                                                                          */
/* Entrée: La structure Json representant le gpio                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_gpio (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { gint gpio           = Json_get_int ( element, "gpio" );
    gint mode_inout     = Json_get_int ( element, "mode_inout" );
    gint mode_activelow = Json_get_int ( element, "mode_activelow" );
    Info_new( Config.log, Cfg.lib->Thread_debug, LOG_INFO,
              "%s: Chargement du GPIO%d en mode_intout %d, mode_activelow=%d", gpio, mode_inout, mode_activelow );

    Cfg.lines[gpio] = gpiod_chip_get_line( Cfg.chip, gpio );
    if (mode_inout)
     { gpiod_line_request_output( Cfg.lines [gpio], "Watchdog RPI Thread", 0 ); }
    else
     { gpiod_line_request_input( Cfg.lines [gpio], "Watchdog RPI Thread" ); }


/*
 * int gpiod_line_get_value(struct gpiod_line *line);
int gpiod_line_set_value(struct gpiod_line *line,
                         int value);
*/
  }
/******************************************************************************************************************************/
/* Charger_tous_IO: Charge toutes les I/O                                                                                     */
/* Entrée: rien                                                                                                               */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Charger_tous_IO ( void  )
  { JsonNode *RootNode = Json_node_create ();
    if (!RootNode) return(FALSE);

    if (SQL_Select_to_json_node ( RootNode, "gpios", "SELECT * FROM '%s' WHERE instance='%s'",
                                  Cfg.lib->name, g_get_host_name() ) == FALSE)
     { json_node_unref(RootNode);
       return(FALSE);
     }
    Json_node_foreach_array_element ( RootNode, "hubs", Charger_un_gpio, NULL );
    json_node_unref(RootNode);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du thread Gpiod                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  {
reload:
    memset( &Cfg, 0, sizeof(Cfg) );                                                 /* Mise a zero de la structure de travail */
    Cfg.lib = lib;                                                 /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "gpiod", "I/O", lib, WTD_VERSION, "Manage Gpiod I/O" );
    Gpiod_Creer_DB ();                                                                /* Création de la base de données */

    Cfg.chip = gpiod_chip_open_lookup("gpiochip0");
    if (!Cfg.chip)
     { Info_new( Config.log, Cfg.lib->Thread_debug, LOG_ERR, "%s: Error while loading chip 'gpiochip0'", __func__ );
       Cfg.lib->Thread_run = FALSE;                                                             /* Le thread ne tourne plus ! */
       goto end;
     }

    Cfg.num_lines = gpiod_chip_num_lines(Cfg.chip);

    if ( Charger_tous_IO() == FALSE )                                                                   /* Chargement des I/O */
     { Info_new( Config.log, Cfg.lib->Thread_debug, LOG_ERR, "%s: Error while loading IO PHIDGET -> stop", __func__ );
       Cfg.lib->Thread_run = FALSE;                                                             /* Le thread ne tourne plus ! */
     }

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

       JsonNode *request;                                                                          /* Ecoute du Master Server */
       while ( (request = Thread_Listen_to_master ( lib ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "SET_DO" ) )
           { if (!Json_has_member ( request, "tech_id"))
              { Info_new( Config.log, Cfg.lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque tech_id", __func__ ); }
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

                /*GSList *liste = Cfg_phidget.Liste_sensors;
                while (liste)
                 { struct PHIDGET_ELEMENT *canal = liste->data;
                   if ( !strcasecmp ( canal->classe, "DigitalOutput" ) &&
                        !strcasecmp ( canal->dls_do->tech_id, tech_id ) &&
                        !strcasecmp ( canal->dls_do->acronyme, acronyme ) )
                    { Info_new( Config.log, Cfg_phidget.lib->Thread_debug, LOG_NOTICE, "%s: SET_DO %s:%s=%d", __func__,
                                canal->dls_do->tech_id, canal->dls_do->acronyme, etat );
                      if ( PhidgetDigitalOutput_setState( (PhidgetDigitalOutputHandle)canal->handle, etat ) != EPHIDGET_OK )
                       { Phidget_print_error ( canal ); }
                      break;
                    }
                   liste = g_slist_next(liste);
                 }*/
              }
           }
          json_node_unref (request);
        }
     }
    for ( gint cpt=0; cpt < sizeof(Cfg.lines); cpt ++ )
     { if (Cfg.lines[cpt]) gpiod_line_release( Cfg.lines[cpt] ); }

end:
    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
