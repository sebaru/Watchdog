/**********************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.h   Header et constantes des modules UPS Watchdgo 2.0                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 16:35:20 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Onduleur.h
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

#ifndef _UPS_H_
 #define _UPS_H_
 #include <json-glib/json-glib.h>
 #include <upsclient.h>

 #define NOM_THREAD      "ups"
 #define NOM_TABLE_UPS   "ups"

 #define UPS_PORT_TCP    3493                             /* Port de connexion TCP pour accès aux modules */
 #define UPS_RETRY       1800                          /* 3 minutes entre chaque retry si pb de connexion */
 #define UPS_POLLING      100                  /* Si tout va bien, on s'y connecte toutes les 10 secondes */

 struct UPS_CONFIG                                                    /* Communication entre DLS et l'UPS */
  { struct LIBRAIRIE *lib;
    GSList *Modules_UPS;
    gboolean enable;                                                           /* Thread enable at boot ? */
    gchar tech_id[32];                                                                                /* Tech_id du telephone */
  } Cfg_ups;

 #define NBR_CARAC_HOST_UPS           32
 #define NBR_CARAC_HOST_UPS_UTF8      (2*NBR_CARAC_HOST_UPS)

 #define NBR_CARAC_NAME_UPS           32
 #define NBR_CARAC_NAME_UPS_UTF8      (2*NBR_CARAC_NAME_UPS)

 #define NBR_CARAC_LIBELLE_UPS        60
 #define NBR_CARAC_LIBELLE_UPS_UTF8   (2*NBR_CARAC_LIBELLE_UPS)

 #define NBR_CARAC_USERNAME_UPS        20
 #define NBR_CARAC_USERNAME_UPS_UTF8   (2*NBR_CARAC_USERNAME_UPS)

 #define NBR_CARAC_PASSWORD_UPS        20
 #define NBR_CARAC_PASSWORD_UPS_UTF8   (2*NBR_CARAC_PASSWORD_UPS)

 struct MODULE_UPS
  { gboolean enable;                                                                   /* Le module doit-il tourner au boot ? */
    gint  id;
    gchar tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];                                                          /* Tech_id du module */
    gchar libelle[NBR_CARAC_LIBELLE_UPS_UTF8+1];                                                           /* Libelle associé */
    gchar host[NBR_CARAC_HOST_UPS_UTF8+1];                                                       /* Adresses IP du module UPS */
    gchar name[NBR_CARAC_NAME_UPS_UTF8+1];                                                        /* Nom de l'UPS sur le HOST */
    gchar username[NBR_CARAC_USERNAME_UPS_UTF8+1];                                                        /* Username associé */
    gchar password[NBR_CARAC_PASSWORD_UPS_UTF8+1];                                                        /* Password associé */
    gpointer bit_comm;                                                            /* Pointer de raccourci pour le bit de comm */
    gint  nbr_connexion;                                                              /* Nombre de connexion OK dans le temps */
    UPSCONN_t upsconn;                                                                               /* Connexion UPS à l'ups */
    gboolean started;                                                                                      /* Est-il actif ?? */
    time_t date_next_connexion;
    gpointer ai_load;
    gpointer ai_realpower;
    gpointer ai_battery_charge;
    gpointer ai_battery_runtime;
    gpointer ai_battery_voltage;
    gpointer ai_input_voltage;
    gpointer ai_input_frequency;
    gpointer ai_output_current;
    gpointer ai_output_voltage;
    gpointer ai_output_frequency;
    gpointer di_outlet_1_status;
    gpointer di_outlet_2_status;
    gpointer di_ups_online;
    gpointer di_ups_charging;
    gpointer di_ups_on_batt;
    gpointer do_load_off;
    gpointer do_load_on;
    gpointer do_outlet_1_off;
    gpointer do_outlet_1_on;
    gpointer do_outlet_2_off;
    gpointer do_outlet_2_on;
    gpointer do_start_deep_bat;
    gpointer do_start_quick_bat;
    gpointer do_stop_test_bat;
  };

/************************************************* Déclaration des prototypes *************************************************/

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/

