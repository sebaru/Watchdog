/******************************************************************************************************************************/
/* Watchdogd/Teleinfo/admin_teleinfo.c        Gestion des connexions Admin TLEINFO au serveur watchdog                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        mer. 13 juin 2012 23:02:08 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_teleinfo.c
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

 #include <glib.h>
 #include "watchdogd.h"
 #include "Teleinfo.h"

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
     { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
                                                                      /* Lancement de la requete de recuperation des messages */
    if (!strcmp(commande, "/status"))
				 { json_builder_begin_object(builder);Json_add_string ( builder, "tech_id", Cfg_teleinfo.tech_id );
				   Json_add_string ( builder, "port", Cfg_teleinfo.port );
				   Json_add_int ( builder, "mode", Cfg_teleinfo.mode );
				   Json_add_int ( builder, "retry_in", (Partage->top - Cfg_teleinfo.date_next_retry)/10.0 );
				   Json_add_int ( builder, "last_view", (Partage->top - Cfg_teleinfo.last_view)/10.0 );
				   Json_add_int ( builder, "adco", Dls_data_get_AI( Cfg_teleinfo.tech_id, "ADCO", &Cfg_teleinfo.adco) );
				   Json_add_int ( builder, "base", Dls_data_get_AI( Cfg_teleinfo.tech_id, "BASE", &Cfg_teleinfo.base) );
       json_builder_end_object(builder);
     }
/************************************************ Génération du JSON **********************************************************/
    *buffer_p = Json_get_buf ( builder, &taille_buf );
    *taille_p = taille_buf;
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
