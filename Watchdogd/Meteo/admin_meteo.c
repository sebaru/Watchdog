/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_sms.c        Gestion des responses Admin SET au serveur watchdog                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        sam. 19 mai 2012 11:03:52 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_sms.c
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

 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"
 #include "Meteo.h"

/******************************************************************************************************************************/
/* Admin_config : fonction appelé par le thread http lors d'une requete POST sur config PROCESS                               */
/* Entrée : la librairie, et le Json recu                                                                                     */
/* Sortie : la base de données est mise à jour                                                                                */
/******************************************************************************************************************************/
 void Admin_config ( struct PROCESS *lib, gpointer msg, JsonNode *request )
  {
    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "description" ) &&
            Json_has_member ( request, "token" ) && Json_has_member ( request, "code_insee" )
           ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *tech_id     = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *token       = Normaliser_chaine ( Json_get_string( request, "token" ) );
    gchar *code_insee  = Normaliser_chaine ( Json_get_string( request, "code_insee" ) );
    gchar *uuid        = Normaliser_chaine ( Json_get_string( request, "uuid" ) );

    if (Json_has_member ( request, "id" ))
     { SQL_Write_new ( "UPDATE %s SET uuid='%s', tech_id='%s', description='%s', token='%s', code_insee='%s' WHERE id='%d'",
                       lib->name, uuid, tech_id, description, token, code_insee, Json_get_int ( request, "id" ) );
       Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' updated.", __func__, lib->uuid, tech_id );
     }
    else
     { gchar *uuid     = Normaliser_chaine ( Json_get_string( request, "uuid" ) );
       SQL_Write_new ( "INSERT INTO %s SET uuid='%s', tech_id='%s', description='%s', token='%s', code_insee='%s'",
                       lib->name, uuid, tech_id, description, token, code_insee );
       Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: subprocess '%s/%s' created.", __func__, lib->uuid, tech_id );
       g_free(uuid);
     }

    g_free(tech_id);
    g_free(description);
    g_free(token);
    g_free(code_insee);
    g_free(uuid);

    soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
