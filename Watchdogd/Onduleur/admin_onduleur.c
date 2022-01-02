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

/******************************************************************************************************************************/
/* Admin_config : fonction appelé par le thread http lors d'une requete POST sur config PROCESS                               */
/* Entrée : la librairie, et le Json recu                                                                                     */
/* Sortie : la base de données est mise à jour                                                                                */
/******************************************************************************************************************************/
 void Admin_config ( struct PROCESS *lib, gpointer msg, JsonNode *request )
  {
    if ( Json_has_member ( request, "uuid" ) && Json_has_member ( request, "tech_id" ) &&
         Json_has_member ( request, "id" ) && Json_has_member ( request, "enable" ) )
     { SQL_Write_new ( "UPDATE %s SET enable='%d' WHERE id='%d'", lib->name, Json_get_bool(request, "enable"),
                       Json_get_int ( request, "id" ) );
       Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' updated.", __func__,
                 Json_get_string ( request, "uuid" ), Json_get_string ( request, "tech_id" ) );
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }

    if ( ! (Json_has_member ( request, "uuid" ) && Json_has_member ( request, "tech_id" ) &&
            Json_has_member ( request, "host" ) && Json_has_member ( request, "name" ) &&
            Json_has_member ( request, "admin_username" ) && Json_has_member ( request, "admin_password" ) &&
            Json_has_member ( request, "enable" )
           )
        )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *uuid           = Normaliser_chaine ( Json_get_string( request, "uuid" ) );
    gchar *tech_id        = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *host           = Normaliser_chaine ( Json_get_string( request, "host" ) );
    gchar *name           = Normaliser_chaine ( Json_get_string( request, "name" ) );
    gchar *admin_username = Normaliser_chaine ( Json_get_string( request, "admin_username" ) );
    gchar *admin_password = Normaliser_chaine ( Json_get_string( request, "admin_password" ) );
    gboolean enable       = Json_get_bool ( request, "enable" );

    if (Json_has_member ( request, "id" ))
     { SQL_Write_new ( "UPDATE %s SET uuid='%s', tech_id='%s', host='%s', name='%s', admin_username='%s', admin_password='%s', "
                       "enable='%d' "
                       "WHERE id='%d'",
                       lib->name, uuid, tech_id, host, name, admin_username, admin_password, enable,
                       Json_get_int ( request, "id" ) );
       Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' updated.", __func__, uuid, tech_id );
     }
    else
     { SQL_Write_new ( "INSERT INTO %s SET uuid='%s', tech_id='%s', host='%s', name='%s', admin_username='%s', admin_password='%s' "
                       "enable='%d' ",
                       lib->name, uuid, tech_id, host, name, admin_username, admin_password, enable );
       Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' created.", __func__, uuid, tech_id );
     }

    g_free(uuid);
    g_free(tech_id);
    g_free(host);
    g_free(name);
    g_free(admin_username);
    g_free(admin_password);

    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
