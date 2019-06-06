/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_ups.c        Gestion des responses Admin ONDULEUR au serveur watchdog                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mer. 11 nov. 2009 11:28:29 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_ups.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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

/******************************************************************************************************************************/
/* Admin_json_list : fonction appelée pour lister les modules modbus                                                          */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_ups_print ( JsonBuilder *builder, struct MODULE_UPS *module )
  { json_builder_begin_object (builder);                                                       /* Création du noeud principal */

    json_builder_set_member_name  ( builder, "tech_id" );
    json_builder_add_string_value ( builder, module->tech_id );

    json_builder_set_member_name  ( builder, "host" );
    json_builder_add_string_value ( builder, module->host );
    json_builder_set_member_name  ( builder, "ups" );
    json_builder_add_string_value ( builder, module->ups );
    json_builder_set_member_name  ( builder, "libelle" );
    json_builder_add_string_value ( builder, module->libelle );

    json_builder_set_member_name  ( builder, "enable" );
    json_builder_add_boolean_value ( builder, module->enable );

    json_builder_set_member_name  ( builder, "started" );
    json_builder_add_boolean_value ( builder, module->started );


    json_builder_set_member_name  ( builder, "admin_username" );
    json_builder_add_string_value ( builder, module->username );
    json_builder_set_member_name  ( builder, "admin_password" );
    json_builder_add_string_value ( builder, module->password );

    json_builder_set_member_name  ( builder, "comm_status" );
    json_builder_add_boolean_value ( builder, module->comm_status );

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
    JsonGenerator *gen;
    gsize taille_buf;
    gchar *buf;

    *buffer_p = NULL;
    *taille_p = 0;

    builder = json_builder_new ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
                                                                      /* Lancement de la requete de recuperation des messages */
    if (!strcmp(commande, "/list")) { Admin_json_ups_list ( builder ); }

/************************************************ Génération du JSON **********************************************************/
    gen = json_generator_new ();
    json_generator_set_root ( gen, json_builder_get_root(builder) );
    json_generator_set_pretty ( gen, TRUE );
    buf = json_generator_to_data (gen, &taille_buf);
    g_object_unref(builder);
    g_object_unref(gen);

    *buffer_p = buf;
    *taille_p = taille_buf;
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
