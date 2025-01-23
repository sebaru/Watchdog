/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_dmx.c        Gestion des responses Admin MODBUS au serveur watchdog                              */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                   dim. 05 sept. 2010 12:01:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_dmx.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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
 #include "Dmx.h"

#ifdef bouh
/******************************************************************************************************************************/
/* Admin_config : fonction appelé par le thread http lors d'une requete POST sur config PROCESS                               */
/* Entrée : la librairie, et le Json recu                                                                                     */
/* Sortie : la base de données est mise à jour                                                                                */
/******************************************************************************************************************************/
 void Admin_config ( struct PROCESS *lib, gpointer msg, JsonNode *request )
  { if ( ! (Json_has_member ( request, "uuid" ) && Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "description" ) &&
            Json_has_member ( request, "device" )
           ) )
     { soup_server_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *uuid        = Normaliser_chaine ( Json_get_string( request, "uuid" ) );
    gchar *tech_id     = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *device      = Normaliser_chaine ( Json_get_string( request, "device" ) );

    if (Json_has_member ( request, "id" ))
     { SQL_Write_new ( "UPDATE %s SET uuid='%s', tech_id='%s', description='%s', device='%s' "
                       "WHERE id='%d'",
                       lib->name, uuid, tech_id, description, device,
                       Json_get_int ( request, "id" ) );
       Info_new( __func__, lib->Thread_debug, LOG_NOTICE, "thread '%s/%s' updated.", uuid, tech_id );
     }
    else
     { SQL_Write_new ( "INSERT INTO %s SET uuid='%s', tech_id='%s', description='%s', device='%s'",
                       lib->name, uuid, tech_id, description, device );
       Info_new( __func__, lib->Thread_debug, LOG_NOTICE, "thread '%s/%s' created.", uuid, tech_id );
     }

    g_free(uuid);
    g_free(tech_id);
    g_free(description);
    g_free(device);

    soup_server_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
#endif
