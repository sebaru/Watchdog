/******************************************************************************************************************************/
/* Watchdogd/Imsg/admin_imsg.c        Gestion des connexions Admin IMSG au serveur watchdog                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                      sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_imsg.c
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
 #include "Audio.h"

/******************************************************************************************************************************/
/* Admin_config : fonction appelé par le thread http lors d'une requete POST sur config PROCESS                               */
/* Entrée : la librairie, et le Json recu                                                                                     */
/* Sortie : la base de données est mise à jour                                                                                */
/******************************************************************************************************************************/
 void Admin_config ( struct PROCESS *lib, gpointer msg, JsonNode *request )
  { if ( ! (Json_has_member ( request, "uuid" ) && Json_has_member ( request, "tech_id" ) &&
            Json_has_member ( request, "language" ) && Json_has_member ( request, "device" ) &&
            Json_has_member ( request, "description" )
           ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *uuid        = Normaliser_chaine ( Json_get_string( request, "uuid" ) );
    gchar *tech_id     = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *language    = Normaliser_chaine ( Json_get_string( request, "language" ) );
    gchar *device      = Normaliser_chaine ( Json_get_string( request, "device" ) );
    gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );

    if (Json_has_member ( request, "id" ))
     { SQL_Write_new ( "UPDATE %s SET uuid='%s', tech_id='%s', language='%s', device='%s', description='%s' WHERE id='%d'",
                       lib->name, uuid, tech_id, language, device, description,
                       Json_get_int ( request, "id" ) );
       Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' updated.", __func__, uuid, tech_id );
     }
    else
     { SQL_Write_new ( "INSERT INTO %s SET uuid='%s', tech_id='%s', language='%s', device='%s', description='%s' ",
                       lib->name, uuid, tech_id, language, device, description );
       Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' created.", __func__, uuid, tech_id );
     }

    g_free(uuid);
    g_free(tech_id);
    g_free(language);
    g_free(device);
    g_free(description);

    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/