/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_modbus.c        Gestion des responses Admin MODBUS au serveur watchdog                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       dim. 05 sept. 2010 12:01:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_modbus.c
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
 #include "Modbus.h"

 extern struct MODBUS_CONFIG Cfg_modbus;

/******************************************************************************************************************************/
/* Modbus_mode_to_string: Convertit le mode modbus (int) en sa version chaine de caractere                                    */
/* Entrée : le module_modbus                                                                                                  */
/* Sortie : char *mode_char                                                                                                   */
/******************************************************************************************************************************/
 static gchar *Modbus_mode_to_string ( struct MODULE_MODBUS *module )
  { static gchar chaine[32];
    if (!module)                     return("Wrong Module   ");
    if (module->date_retente > Partage->top)
     { g_snprintf( chaine, sizeof(chaine),  "Next Try : %03ds", (module->date_retente - Partage->top)/10 );
       return(chaine);
     }
    if (!module->started)            return("Disconnected   ");

    switch ( module->mode )
     {
       case MODBUS_GET_DESCRIPTION : return("Get_Description");
       case MODBUS_GET_FIRMWARE    : return("Get_firmware   ");
       case MODBUS_INIT_WATCHDOG1  : return("Init_Watchdog_1");
       case MODBUS_INIT_WATCHDOG2  : return("Init_Watchdog_2");
       case MODBUS_INIT_WATCHDOG3  : return("Init_Watchdog_3");
       case MODBUS_INIT_WATCHDOG4  : return("Init_Watchdog_4");
       case MODBUS_GET_NBR_AI      : return("Init_Get_Nbr_AI");
       case MODBUS_GET_NBR_AO      : return("Init_Get_Nbr_AO");
       case MODBUS_GET_NBR_DI      : return("Init_Get_Nbr_DI");
       case MODBUS_GET_NBR_DO      : return("Init_Get_Nbr_DO");
       case MODBUS_GET_DI          : return("Get DI State   ");
       case MODBUS_GET_AI          : return("Get AI State   ");
       case MODBUS_SET_DO          : return("Set DO State   ");
       case MODBUS_SET_AO          : return("Set AO State   ");
       default :                     return("Unknown mode   ");
     }
  }
/******************************************************************************************************************************/
/* Admin_json_modbus_list : fonction appelée pour lister les modules modbus                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_modbus_status ( struct LIBRAIRIE *Lib, SoupMessage *msg )
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
/*    if (Lib->Thread_run)                                    /* Warning : Cfg_modbus does not exist if thread is not running ! */
/*     { Json_add_int ( builder, "nbr_request_par_sec", Cfg_modbus.nbr_request_par_sec ); }*/

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Admin_json_modbus_list : fonction appelée pour lister les modules modbus                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_modbus_modules_status ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { GSList *liste_modules;
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

    Json_add_array ( builder, "modules" );
    if (Lib->Thread_run)                                    /* Warning : Cfg_modbus does not exist if thread is not running ! */
     { pthread_mutex_lock( &Lib->synchro );
       liste_modules = Cfg_modbus.Modules_MODBUS;
       while ( liste_modules )
        { struct MODULE_MODBUS *module = liste_modules->data;

          Json_add_object ( builder, NULL );
          Json_add_string ( builder, "tech_id", module->modbus.tech_id );
          Json_add_string ( builder, "mode", Modbus_mode_to_string(module) );
          Json_add_bool   ( builder, "started", module->started );
          Json_add_int    ( builder, "nbr_entree_tor", module->nbr_entree_tor );
          Json_add_int    ( builder, "nbr_sortie_tor", module->nbr_sortie_tor );
          Json_add_int    ( builder, "nbr_entree_ana", module->nbr_entree_ana );
          Json_add_int    ( builder, "nbr_sortie_ana", module->nbr_sortie_ana );
          Json_add_bool   ( builder, "comm", Dls_data_get_MONO( NULL, NULL, &module->bit_comm) );
          Json_add_int    ( builder, "transaction_id", module->transaction_id );
          Json_add_int    ( builder, "nbr_request_par_sec", module->nbr_request_par_sec );
          Json_add_int    ( builder, "delai", module->delai );
          Json_add_int    ( builder, "nbr_deconnect", module->nbr_deconnect );
          Json_add_int    ( builder, "last_reponse", (Partage->top - module->date_last_reponse)/10 );
          Json_add_int    ( builder, "date_next_eana", (module->date_next_eana > Partage->top ? (module->date_next_eana - Partage->top)/10 : -1) );
          Json_add_int    ( builder, "date_retente", (module->date_retente > Partage->top   ? (module->date_retente   - Partage->top)/10 : -1) );
          Json_end_object ( builder );                                                                       /* End Module Array */

          liste_modules = liste_modules->next;                                                      /* Passage au module suivant */
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
 static void Admin_json_modbus_list ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { JsonBuilder *builder;
    gchar *buf, chaine[512];
    gsize taille_buf;

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
    g_snprintf(chaine, sizeof(chaine), "SELECT * FROM modbus_modules" );
    if (SQL_Select_to_JSON ( builder, "modules", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }
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
 static void Admin_json_modbus_del ( struct LIBRAIRIE *Lib, SoupMessage *msg )
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

    g_snprintf( chaine, sizeof(chaine), "DELETE FROM modbus_modules WHERE tech_id='%s'", tech_id );
    g_free(tech_id);
    SQL_Write ( chaine );
    soup_message_set_status (msg, SOUP_STATUS_OK);
    Lib->Thread_reload = TRUE;
  }
/******************************************************************************************************************************/
/* Admin_json_modbus_set: Met à jour une entrée WAGO                                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_modbus_set ( struct LIBRAIRIE *Lib, SoupMessage *msg )
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

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "hostname" ) &&
            Json_has_member ( request, "description" ) && Json_has_member ( request, "watchdog" ) &&
            Json_has_member ( request, "max_request_par_sec" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id     = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *hostname    = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gint  watchdog     = Json_get_int ( request, "watchdog" );
    gint  max_request_par_sec = Json_get_int ( request, "max_request_par_sec" );
    json_node_unref(request);

    g_snprintf( requete, sizeof(requete),
               "UPDATE modbus_modules SET description='%s', hostname='%s', watchdog='%d', max_request_par_sec='%d' WHERE tech_id='%s'",
                description, hostname, watchdog, max_request_par_sec, tech_id );
    g_free(tech_id);
    g_free(description);
    g_free(hostname);
    if (SQL_Write (requete))
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Lib->Thread_reload = TRUE;
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
  }
/******************************************************************************************************************************/
/* Admin_json_modbus_set: Met à jour une entrée WAGO                                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_modbus_start ( struct LIBRAIRIE *Lib, SoupMessage *msg, gboolean start )
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

    gchar *tech_id     = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    json_node_unref(request);

    g_snprintf( requete, sizeof(requete), "UPDATE modbus_modules SET enable='%d' WHERE tech_id='%s'", (start ? 1 : 0), tech_id );
    if (SQL_Write (requete))
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Lib->Thread_reload = TRUE;
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
  }
/******************************************************************************************************************************/
/* Admin_json_modbus_set: Ajoute un composant WAGO dans système                                                               */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_modbus_add ( struct LIBRAIRIE *Lib, SoupMessage *msg )
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

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "hostname" ) &&
            Json_has_member ( request, "description" ) && Json_has_member ( request, "watchdog" ) &&
            Json_has_member ( request, "max_request_par_sec" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id     = Normaliser_as_ascii ( Json_get_string( request, "tech_id" ) );
    gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *hostname    = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gint  watchdog     = Json_get_int ( request, "watchdog" );
    gint  max_request_par_sec = Json_get_int ( request, "max_request_par_sec" );

    g_snprintf( requete, sizeof(requete),
               "INSERT INTO modbus_modules SET tech_id='%s', description='%s', hostname='%s', watchdog='%d', "
               "max_request_par_sec='%d', enable=0, date_create=NOW()",
                tech_id, description, hostname, watchdog, max_request_par_sec );
    g_free(description);
    g_free(hostname);
    json_node_unref(request);

    if (SQL_Write (requete))
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Lib->Thread_reload = TRUE;
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
         if (!strcasecmp(path, "/list"))     { Admin_json_modbus_list ( lib, msg ); }
    else if (!strcasecmp(path, "/modules_status")) { Admin_json_modbus_modules_status ( lib, msg ); }
    else if (!strcasecmp(path, "/status"))   { Admin_json_modbus_status ( lib, msg ); }
    else if (!strcasecmp(path, "/del"))      { Admin_json_modbus_del ( lib, msg ); }
    else if (!strcasecmp(path, "/set"))      { Admin_json_modbus_set ( lib, msg ); }
    else if (!strcasecmp(path, "/add"))      { Admin_json_modbus_add ( lib, msg ); }
    else if (!strcasecmp(path, "/start"))    { Admin_json_modbus_start ( lib, msg, TRUE ); }
    else if (!strcasecmp(path, "/stop"))     { Admin_json_modbus_start ( lib, msg, FALSE ); }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
