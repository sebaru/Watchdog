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
/* Admin_json_ups_list : fonction appelée pour lister les upss ups                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_ups_status ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_bool ( builder, "thread_is_running", Lib->Thread_run );
/*    if (Lib->Thread_run)                                    /* Warning : Cfg_ups does not exist if thread is not running ! */
/*     { Json_add_int ( builder, "nbr_request_par_sec", Cfg_ups.nbr_request_par_sec ); }*/

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Admin_json_ups_list : fonction appelée pour lister les upss ups                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_ups_list ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { GSList *liste_upss;
    JsonBuilder *builder;
    gsize taille_buf;
    gchar *buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_array ( builder, "ups" );
    if (Lib->Thread_run)                                    /* Warning : Cfg_ups does not exist if thread is not running ! */
     { pthread_mutex_lock( &Lib->synchro );
       liste_upss = Cfg_ups.Modules_UPS;
       while ( liste_upss )
        { struct MODULE_UPS *ups = liste_upss->data;

          Json_add_object ( builder, NULL );
          Json_add_string ( builder, "tech_id", ups->tech_id );
          Json_add_string ( builder, "host", ups->host );
          Json_add_string ( builder, "name", ups->name );
          Json_add_string ( builder, "description", ups->description );
          Json_add_bool   ( builder, "enable", ups->enable );
          Json_add_bool   ( builder, "started", ups->started );
          Json_add_int    ( builder, "nbr_connexion", ups->nbr_connexion );
          Json_add_string ( builder, "admin_username", ups->admin_username );
          Json_add_string ( builder, "admin_password", ups->admin_password );
          Json_add_string ( builder, "date_create", ups->date_create );
          Json_end_object ( builder );                                                                       /* End Module Array */

          liste_upss = liste_upss->next;                                                      /* Passage au ups suivant */
        }
       pthread_mutex_unlock( &Lib->synchro );
     }
    Json_end_array (builder);                                                                                 /* End Document */

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getdlslist: Traite une requete sur l'URI dlslist                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static void Admin_json_ups_del ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { GBytes *request_brute;
    gsize taille;
    gchar chaine[256];
    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No Request");
       return;
     }

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *tech_id = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    if (!tech_id)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Normalized failed");
       return;
     }
    json_node_unref(request);

    g_snprintf( chaine, sizeof(chaine), "DELETE FROM ups WHERE tech_id='%s'", tech_id );
    g_free(tech_id);
    SQL_Write ( chaine );
    soup_message_set_status (msg, SOUP_STATUS_OK);
    Lib->Thread_reload = TRUE;
    sleep(1);
  }
/******************************************************************************************************************************/
/* Admin_json_ups_set: Met à jour une entrée WAGO                                                                             */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_ups_set_add ( gboolean ajout, struct LIBRAIRIE *Lib, SoupMessage *msg )
  { GBytes *request_brute;
    gchar requete[256];
    gsize taille;

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No Request");
       return;
     }

    if ( ! (Json_has_member ( request, "tech_id" ) &&
            Json_has_member ( request, "host" ) && Json_has_member ( request, "name" ) &&
            Json_has_member ( request, "admin_username" ) && Json_has_member ( request, "admin_password" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id        = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *host           = Normaliser_chaine ( Json_get_string( request, "host" ) );
    gchar *name           = Normaliser_chaine ( Json_get_string( request, "name" ) );
    gchar *admin_username = Normaliser_chaine ( Json_get_string( request, "admin_username" ) );
    gchar *admin_password = Normaliser_chaine ( Json_get_string( request, "admin_password" ) );
    json_node_unref(request);

    if (ajout)
     { g_snprintf( requete, sizeof(requete),
                  "INSERT INTO ups SET tech_id='%s', host='%s', name='%s', admin_username='%s', admin_password='%s'",
                   tech_id, host, name, admin_username, admin_password );
     }
    else
     { g_snprintf( requete, sizeof(requete),
                  "UPDATE ups SET host='%s', name='%s', admin_username='%s', admin_password='%s' WHERE tech_id='%s'",
                   host, name, admin_username, admin_password, tech_id );
     }
    g_free(tech_id);
    g_free(host);
    g_free(name);
    g_free(admin_username);
    g_free(admin_password);

    if (SQL_Write (requete))
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Lib->Thread_reload = TRUE;
       sleep(1);
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
  }
/******************************************************************************************************************************/
/* Admin_json_ups_set: Met à jour une entrée WAGO                                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_ups_start ( struct LIBRAIRIE *Lib, SoupMessage *msg, gboolean start )
  { GBytes *request_brute;
    gchar requete[256];
    gsize taille;

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No Request");
       return;
     }

    if ( ! (Json_has_member ( request, "tech_id" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    json_node_unref(request);

    g_snprintf( requete, sizeof(requete), "UPDATE ups SET enable='%d' WHERE tech_id='%s'", (start ? 1 : 0), tech_id );
    if (SQL_Write (requete))
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Lib->Thread_reload = TRUE;
       sleep(1);
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 void Admin_json ( struct LIBRAIRIE *lib, SoupMessage *msg, const char *path, GHashTable *query, gint access_level )
  { if (access_level < 6)
     { soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Pas assez de privileges");
       return;
     }
         if (!strcasecmp(path, "/list"))     { Admin_json_ups_list ( lib, msg ); }
    else if (!strcasecmp(path, "/status"))   { Admin_json_ups_status ( lib, msg ); }
    else if (!strcasecmp(path, "/del"))      { Admin_json_ups_del ( lib, msg ); }
    else if (!strcasecmp(path, "/set"))      { Admin_json_ups_set_add ( FALSE, lib, msg ); }
    else if (!strcasecmp(path, "/add"))      { Admin_json_ups_set_add ( TRUE, lib, msg ); }
    else if (!strcasecmp(path, "/start"))    { Admin_json_ups_start ( lib, msg, TRUE ); }
    else if (!strcasecmp(path, "/stop"))     { Admin_json_ups_start ( lib, msg, FALSE ); }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
