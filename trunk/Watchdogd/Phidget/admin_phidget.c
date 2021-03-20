/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_phidget.c        Gestion des responses Admin PHIDGET au serveur watchdog                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       dim. 05 sept. 2010 12:01:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_phidget.c
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
 #include "Phidget.h"

 extern struct PHIDGET_CONFIG Cfg_phidget;
/******************************************************************************************************************************/
/* Admin_json_phidget_list : fonction appelée pour lister les modules phidget                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_phidget_status ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_node_add_bool ( RootNode, "thread_is_running", Lib->Thread_run );
/*    if (Lib->Thread_run)                                    /* Warning : Cfg_phidget does not exist if thread is not running ! */
/*     { Json_add_int ( RootNode, "nbr_request_par_sec", Cfg_phidget.nbr_request_par_sec ); }*/

    gchar *buf = Json_node_to_string(RootNode);
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Admin_json_phidget_list : fonction appelée pour lister les modules phidget                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_phidget_modules_status ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { GSList *liste_modules;
    JsonBuilder *RootNode;
    gsize taille_buf;
    gchar *buf;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_array ( RootNode, "modules" );
    if (Lib->Thread_run)                                    /* Warning : Cfg_phidget does not exist if thread is not running ! */
     { pthread_mutex_lock( &Lib->synchro );
       liste_modules = Cfg_phidget.Modules_PHIDGET;
       while ( liste_modules )
        { struct MODULE_PHIDGET *module = liste_modules->data;

          Json_add_object ( RootNode, NULL );
          Json_add_string ( RootNode, "tech_id", module->phidget.tech_id );
          Json_add_string ( RootNode, "mode", Modbus_mode_to_string(module) );
          Json_add_bool   ( RootNode, "started", module->started );
          Json_add_int    ( RootNode, "nbr_entree_tor", module->nbr_entree_tor );
          Json_add_int    ( RootNode, "nbr_sortie_tor", module->nbr_sortie_tor );
          Json_add_int    ( RootNode, "nbr_entree_ana", module->nbr_entree_ana );
          Json_add_int    ( RootNode, "nbr_sortie_ana", module->nbr_sortie_ana );
          Json_add_bool   ( RootNode, "comm", Dls_data_get_MONO( NULL, NULL, &module->bit_comm) );
          Json_add_int    ( RootNode, "transaction_id", module->transaction_id );
          Json_add_int    ( RootNode, "nbr_request_par_sec", module->nbr_request_par_sec );
          Json_add_int    ( RootNode, "delai", module->delai );
          Json_add_int    ( RootNode, "nbr_deconnect", module->nbr_deconnect );
          Json_add_int    ( RootNode, "last_reponse", (Partage->top - module->date_last_reponse)/10 );
          Json_add_int    ( RootNode, "date_next_eana", (module->date_next_eana > Partage->top ? (module->date_next_eana - Partage->top)/10 : -1) );
          Json_add_int    ( RootNode, "date_retente", (module->date_retente > Partage->top   ? (module->date_retente   - Partage->top)/10 : -1) );
          Json_end_object ( RootNode );                                                                       /* End Module Array */

          liste_modules = liste_modules->next;                                                      /* Passage au module suivant */
        }
       pthread_mutex_unlock( &Lib->synchro );
     }
    Json_end_array (RootNode);                                                                                 /* End Document */

    buf = Json_get_buf ( RootNode, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(taille_buf) );
  }
#endif
/******************************************************************************************************************************/
/* Admin_json_phidget_hub_list: Renvoie la liste des hub Phidget configurés                                                   */
/* Entrées: la connexion Websocket destinataire                                                                               */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_phidget_hub_list ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "hubs", "SELECT * FROM phidget_hub" ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(RootNode);
       return;
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref(RootNode);
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getdlslist: Traite une requete sur l'URI dlslist                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static void Admin_json_phidget_hub_del ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  {
    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    SQL_Write_new( "DELETE FROM phidget_hub WHERE id='%d'", Json_get_int ( request, "id" ) );
    soup_message_set_status (msg, SOUP_STATUS_OK);
    Lib->Thread_reload = TRUE;
  }
/******************************************************************************************************************************/
/* Admin_json_phidget_set: Met à jour une entrée WAGO                                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_phidget_hub_set ( struct LIBRAIRIE *Lib, SoupMessage *msg )
  { gboolean retour;

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "hostname" ) && Json_has_member ( request, "description" ) &&
            Json_has_member ( request, "password" ) && Json_has_member ( request, "enable" )
           )
       )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *hostname    = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *password    = Normaliser_chaine ( Json_get_string( request, "password" ) );

    if (Json_has_member ( request, "id" ))
     { retour = SQL_Write_new ( "UPDATE phidget_hub SET description='%s', hostname='%s', password='%s' WHERE id='%d'",
                                description, hostname, password, Json_get_int ( request, "id" ) );
     }
    else
     { retour = SQL_Write_new ( "INSERT INTO phidget_hub SET description='%s', hostname='%s', password='%s'",
                                description, hostname, password );
     }
    json_node_unref(request);

    g_free(description);
    g_free(hostname);
    g_free(password);
    if (retour)
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Lib->Thread_reload = TRUE;
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );
  }
/******************************************************************************************************************************/
/* Admin_json_phidget_hub_start_stop: Start ou Stop un Hub Phidget                                                            */
/* Entrées: la connexion Websocket, le champ start/stop                                                                       */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Admin_json_phidget_hub_start_stop ( struct LIBRAIRIE *Lib, SoupMessage *msg, gboolean start )
  { if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "id" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gint id = Json_get_int ( request, "id" );
    json_node_unref(request);


    if (SQL_Write_new ( "UPDATE phidget_hub SET enable='%d' WHERE id='%d'", start, id ) )
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
         if (!strcasecmp(path, "/hub_list"))    { Admin_json_phidget_hub_list ( lib, msg ); }
    /*else if (!strcasecmp(path, "/module_list")) { Admin_json_phidget_modules_status ( lib, msg ); }*/
    else if (!strcasecmp(path, "/status"))      { Admin_json_phidget_status ( lib, msg ); }
    else if (!strcasecmp(path, "/hub_del"))     { Admin_json_phidget_hub_del ( lib, msg ); }
    else if (!strcasecmp(path, "/hub_set"))     { Admin_json_phidget_hub_set ( lib, msg ); }
    else if (!strcasecmp(path, "/hub_start"))   { Admin_json_phidget_hub_start_stop ( lib, msg, TRUE ); }
    else if (!strcasecmp(path, "/hub_stop"))    { Admin_json_phidget_hub_start_stop ( lib, msg, FALSE ); }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
