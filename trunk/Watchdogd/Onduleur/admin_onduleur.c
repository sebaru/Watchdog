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
  { Json_add_object ( builder, module->tech_id );
    Json_add_string ( builder, "host", module->host );
    Json_add_string ( builder, "name", module->name );
    Json_add_string ( builder, "libelle", module->libelle );

    Json_add_bool   ( builder, "enable", module->enable );
    Json_add_bool   ( builder, "started", module->started );
    Json_add_int    ( builder, "nbr_connexion", module->nbr_connexion );

    Json_add_string ( builder, "admin_username", module->username );
    Json_add_string ( builder, "admin_password", module->password );
    Json_end_object ( builder );
  }
/******************************************************************************************************************************/
/* Admin_ups_list : Affichage de toutes les infos opérationnelles de tous les onduleurs                                       */
/* Entrée: La response response ADMIN                                                                                       */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_ups_list ( JsonBuilder *builder )
  { GSList *liste_modules;

    pthread_mutex_lock ( &Cfg_ups.lib->synchro );
    liste_modules = Cfg_ups.Modules_UPS;
    while ( liste_modules )
     { struct MODULE_UPS *module;
       module = (struct MODULE_UPS *)liste_modules->data;
       Admin_json_ups_print ( builder, module );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_ups.lib->synchro );
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
