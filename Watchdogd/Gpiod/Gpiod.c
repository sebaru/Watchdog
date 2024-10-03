/******************************************************************************************************************************/
/* Watchdogd/Gpiod/Gpiod.c  Gestion des I/O Gpiod  Watchdog 3.0                                                               */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                                03.09.2021 17:51:06 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Gpiod.c
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

 #include <glib.h>
 #include <sys/prctl.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Gpiod.h"

/******************************************************************************************************************************/
/* Charger_un_gpio: Charge une configuration de GPIO                                                                          */
/* Entrée: La structure Json representant le gpio                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Charger_un_gpio (JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct THREAD *module = user_data;
    struct GPIOD_VARS *vars = module->vars;

    gint num = Json_get_int ( element, "num" );
    if (num >= vars->num_lines)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "GPIO%02d is out of range (>=%d)", num, vars->num_lines );
       return;
     }

    vars->lignes[num].mode_inout     = Json_get_int ( element, "mode_inout" );
    vars->lignes[num].mode_activelow = Json_get_int ( element, "mode_activelow" );
    Info_new( __func__, module->Thread_debug, LOG_INFO,
              "Chargement du GPIO%02d en mode_inout %d, mode_activelow=%d",
              num, vars->lignes[num].mode_inout, vars->lignes[num].mode_activelow );

    if (vars->lignes[num].mode_inout==0)
     { struct gpiod_line_settings *settings = gpiod_line_settings_new();
       if (!settings) return;

       gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);

       struct gpiod_line_config *line_cfg = gpiod_line_config_new();
       if (!line_cfg) return;

       int ret = gpiod_line_config_add_line_settings(line_cfg, &num, 1, settings);
       if (!ret)
        { struct gpiod_request_config *req_cfg = gpiod_request_config_new();
          if (req_cfg) gpiod_request_config_set_consumer(req_cfg, "WATCHDOG GPIO Thread");
          vars->lignes[num].gpio_ligne = gpiod_chip_request_lines(vars->chip, req_cfg, line_cfg);
          if (req_cfg) gpiod_request_config_free(req_cfg);
          gpiod_line_config_free(line_cfg);
          gpiod_line_settings_free(settings);
          vars->lignes[num].etat = gpiod_line_request_get_value( vars->lignes[num].gpio_ligne, num );
        }
     }
    else
     { struct gpiod_line_settings *settings = gpiod_line_settings_new();
       if (!settings) return;

       gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

       struct gpiod_line_config *line_cfg = gpiod_line_config_new();
       if (!line_cfg) return;

       int ret = gpiod_line_config_add_line_settings(line_cfg, &num, 1, settings);
       if (!ret)
        { struct gpiod_request_config *req_cfg = gpiod_request_config_new();
          if (req_cfg) gpiod_request_config_set_consumer(req_cfg, "WATCHDOG GPIO Thread");
          vars->lignes[num].gpio_ligne = gpiod_chip_request_lines(vars->chip, req_cfg, line_cfg);
          if (req_cfg) gpiod_request_config_free(req_cfg);
          gpiod_line_config_free(line_cfg);
          gpiod_line_settings_free(settings);
          vars->lignes[num].etat = gpiod_line_request_set_value( vars->lignes[num].gpio_ligne, num, vars->lignes[num].mode_activelow );
        }
     }
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct GPIOD_VARS) );
    struct GPIOD_VARS *vars = module->vars;

    gchar *tech_id  = Json_get_string ( module->config, "tech_id" );
    vars->chip = gpiod_chip_open("/dev/gpiochip0");
    if (!vars->chip)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Error while loading chip 'gpiochip0'", tech_id );
       goto end;
     }
    else Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: chip 'gpiochip0' loaded", tech_id );

    struct gpiod_chip_info *info = gpiod_chip_get_info(vars->chip);
    if (!info) goto end;

    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s [%s] %d lines",
              gpiod_chip_info_get_name(info), gpiod_chip_info_get_label(info), gpiod_chip_info_get_num_lines(info) );
    vars->num_lines = gpiod_chip_info_get_num_lines(info);
    gpiod_chip_info_free(info);

    if (vars->num_lines > GPIOD_MAX_LINE) vars->num_lines = GPIOD_MAX_LINE;
    Info_new( __func__, module->Thread_debug, LOG_INFO, "%s: found %d lines", tech_id, vars->num_lines );

    JsonNode *RootNode = Json_node_create ();                                                     /* Envoi de la conf a l'API */
    if (!RootNode) goto end;
    Json_node_add_string ( RootNode, "thread_tech_id", tech_id );
    Json_node_add_int    ( RootNode, "nbr_lignes",     vars->num_lines );
    JsonNode *API_result = Http_Post_to_global_API ( "/run/gpiod/add/io", RootNode );
    Json_node_unref ( API_result );
    Json_node_unref ( RootNode );

    vars->lignes = g_try_malloc0 ( sizeof( struct GPIOD_LIGNE ) * vars->num_lines );
    if (!vars->lignes)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Memory Error while loading lignes", tech_id );
       goto end;
     }

    Json_node_foreach_array_element ( module->config, "gpios", Charger_un_gpio, module );

    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
       for ( gint cpt = 0; cpt < vars->num_lines; cpt++ )
        { if (vars->lignes[cpt].mode_inout == 0) /* Ligne d'entrée ? */
           { gboolean etat = gpiod_line_request_get_value( vars->lignes[cpt].gpio_ligne, cpt );
             if (etat != vars->lignes[cpt].etat) /* Détection de changement */
              { vars->lignes[cpt].etat = etat;
                /*if (vars->lignes[cpt].mapped) MQTT_Send_DI ( module, vars->lignes[cpt].tech_id, vars->lignes[cpt].acronyme, etat );*/
                Info_new( __func__, module->Thread_debug, LOG_DEBUG, "%s: INPUT: GPIO%02d = %d", tech_id, cpt, etat );
                break;
              }
           }
        }

/****************************************************** Ecoute du master ******************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *tag = Json_get_string ( request, "tag" );

          if ( !strcasecmp( tag, "SET_DO" ) )
           { /*gboolean etat = Json_get_bool ( request, "etat" );
             pthread_mutex_lock ( &module->synchro );
             for (gint num=0; num<vars->num_lines; num++)
              { if ( vars->gpio_lignes && vars->gpio_lignes[num] &&
                     !strcasecmp ( Json_get_string(vars->DO[num], "thread_acronyme"), msg_thread_acronyme ) )
                 { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "SET_DO '%s:%s'/'%s:%s'=%d",
                             msg_thread_tech_id, msg_thread_acronyme, msg_tech_id, msg_acronyme, etat );
                   Json_node_add_bool ( vars->DO[num], "etat", etat );
                   break;
                 }
              }
             pthread_mutex_unlock ( &module->synchro );*/
           }
          Json_node_unref (request);
        }
     }

end:
    for ( gint cpt=0; cpt < vars->num_lines; cpt++ )
     { if (vars->lignes[cpt].gpio_ligne) gpiod_line_request_release( vars->lignes[cpt].gpio_ligne ); }

    if (vars->lignes) g_free(vars->lignes);
    if (vars->chip) gpiod_chip_close (vars->chip);

    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
