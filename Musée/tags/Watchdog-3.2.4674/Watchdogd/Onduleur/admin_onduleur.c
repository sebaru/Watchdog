/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_ups.c        Gestion des responses Admin ONDULEUR au serveur watchdog                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mer. 11 nov. 2009 11:28:29 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_ups.c
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

 #include "watchdogd.h"
 #include "Onduleur.h"
 extern struct UPS_CONFIG Cfg_ups;
/******************************************************************************************************************************/
/* Admin_json_list : fonction appelée pour lister les modules modbus                                                          */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_ups_print ( JsonBuilder *builder, struct MODULE_UPS *module )
  { json_builder_begin_object (builder);                                                       /* Création du noeud principal */

    Json_add_string ( builder, "tech_id", module->tech_id );
    Json_add_string ( builder, "host", module->host );
    Json_add_string ( builder, "name", module->name );
    Json_add_string ( builder, "libelle", module->libelle );

    Json_add_bool ( builder, "enable", module->enable );
    Json_add_bool ( builder, "started", module->started );
    Json_add_bool ( builder, "comm", Dls_data_get_bool ( module->tech_id, "COMM", &module->bit_comm ) );
    Json_add_int ( builder, "nbr_connexion", module->nbr_connexion );

    Json_add_string ( builder, "admin_username", module->username );
    Json_add_string ( builder, "admin_password", module->password );


    Json_add_double ( builder, "ups_load", Dls_data_get_AI( NULL, NULL, &module->ai_load ) );
    Json_add_double ( builder, "ups_realpower", Dls_data_get_AI( NULL, NULL, &module->ai_realpower ) );
    Json_add_double ( builder, "ups_battery_charge", Dls_data_get_AI( NULL, NULL, &module->ai_battery_charge ) );
    Json_add_double ( builder, "ups_battery_runtime", Dls_data_get_AI( NULL, NULL, &module->ai_battery_runtime ) );
    Json_add_double ( builder, "ups_battery_voltage", Dls_data_get_AI( NULL, NULL, &module->ai_battery_voltage ) );
    Json_add_double ( builder, "ups_input_voltage", Dls_data_get_AI( NULL, NULL, &module->ai_input_voltage ) );
    Json_add_double ( builder, "ups_input_frequency", Dls_data_get_AI( NULL, NULL, &module->ai_input_frequency ) );
    Json_add_double ( builder, "ups_output_current", Dls_data_get_AI( NULL, NULL, &module->ai_output_current ) );
    Json_add_double ( builder, "ups_output_voltage", Dls_data_get_AI( NULL, NULL, &module->ai_output_voltage ) );
    Json_add_double ( builder, "ups_output_frequency", Dls_data_get_AI( NULL, NULL, &module->ai_output_frequency ) );

    Json_add_bool ( builder, "ups_outlet_1_status", Dls_data_get_DI( NULL, NULL, &module->di_outlet_1_status ) );
    Json_add_bool ( builder, "ups_outlet_2_status", Dls_data_get_DI( NULL, NULL, &module->di_outlet_2_status ) );
    Json_add_bool ( builder, "ups_online",          Dls_data_get_DI( NULL, NULL, &module->di_ups_online ) );
    Json_add_bool ( builder, "ups_charging",        Dls_data_get_DI( NULL, NULL, &module->di_ups_charging ) );
    Json_add_bool ( builder, "ups_on_batt",         Dls_data_get_DI( NULL, NULL, &module->di_ups_on_batt ) );
    Json_add_bool ( builder, "ups_replace_batt",    Dls_data_get_DI( NULL, NULL, &module->di_ups_replace_batt ) );
    Json_add_bool ( builder, "ups_alarm",           Dls_data_get_DI( NULL, NULL, &module->di_ups_alarm ) );

    json_builder_end_object (builder);                                                                        /* End Document */
  }
/******************************************************************************************************************************/
/* Admin_ups_list : Affichage de toutes les infos opérationnelles de tous les onduleurs                                       */
/* Entrée: La response response ADMIN                                                                                       */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_ups_list ( JsonBuilder *builder )
  { GSList *liste_modules;

    json_builder_begin_array (builder);
    pthread_mutex_lock ( &Cfg_ups.lib->synchro );
    liste_modules = Cfg_ups.Modules_UPS;
    while ( liste_modules )
     { struct MODULE_UPS *module;
       module = (struct MODULE_UPS *)liste_modules->data;
       Admin_json_ups_print ( builder, module );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_ups.lib->synchro );
    json_builder_end_array (builder);
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 void Admin_json ( gchar *commande, gchar **buffer_p, gint *taille_p )
  { JsonBuilder *builder;
    gsize taille_buf;

    *buffer_p = NULL;
    *taille_p = 0;

    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
                                                                      /* Lancement de la requete de recuperation des messages */
    if (!strcmp(commande, "/list")) { Admin_json_ups_list ( builder ); }

/************************************************ Génération du JSON **********************************************************/
    *buffer_p = Json_get_buf ( builder, &taille_buf );
    *taille_p = taille_buf;
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
