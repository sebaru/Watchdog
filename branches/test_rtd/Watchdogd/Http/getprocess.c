/******************************************************************************************************************************/
/* Watchdogd/Http/getprocess.c       Gestion des request getprocess pour le thread HTTP de watchdog                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    30.12.2018 01:59:26 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getprocess.c
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

 #include <string.h>
 #include <unistd.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_list: Traite une requete sur l'URI process/list                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_process_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                  SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gpointer instance = g_hash_table_lookup ( query, "instance" );
    if ( instance && strcasecmp ( instance, "MASTER" ) && strcasecmp ( instance, g_get_host_name() ) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s : Redirecting /process/list vers %s", __func__, instance );
       Http_redirect_to_slave ( msg, instance );
       return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    JsonArray *process = Json_node_add_array ( RootNode, "Process" );                                    /* Contenu du Status */

    JsonNode *element = Json_node_create();
    Json_node_add_string ( element, "thread",  "msrv" );
    Json_node_add_bool   ( element, "debug",   Config.log_msrv );
    Json_node_add_bool   ( element, "started", Partage->com_msrv.Thread_run );
    Json_node_add_string ( element, "version", WTD_VERSION );
    Json_node_add_int    ( element, "start_time", Partage->start_time );
    Json_node_add_string ( element, "objet",   "Local Master Server" );
    Json_array_add_element ( process, element );

    element = Json_node_create();
    Json_node_add_string ( element, "thread",  "dls" );
    Json_node_add_bool   ( element, "debug",   Partage->com_dls.Thread_debug );
    Json_node_add_bool   ( element, "started", Partage->com_dls.Thread_run );
    Json_node_add_string ( element, "version", WTD_VERSION );
    Json_node_add_int    ( element, "start_time", Partage->start_time );
    Json_node_add_string ( element, "objet",   "D.L.S" );
    Json_array_add_element ( process, element );

    element = Json_node_create();
    Json_node_add_string ( element, "thread",  "archive" );
    Json_node_add_bool   ( element, "debug",   Config.log_arch );
    Json_node_add_bool   ( element, "started", Partage->com_arch.Thread_run );
    Json_node_add_string ( element, "version", WTD_VERSION );
    Json_node_add_int    ( element, "start_time", Partage->start_time );
    Json_node_add_string ( element, "objet",   "Archivage" );
    Json_array_add_element ( process, element );

    GSList *liste = Partage->com_msrv.Librairies;                                        /* Parcours de toutes les librairies */
    while(liste)
     { struct LIBRAIRIE *lib = liste->data;
       element = Json_node_create();
       Json_node_add_string ( element, "thread",  lib->name );
       Json_node_add_bool   ( element, "debug",   lib->Thread_debug );
       Json_node_add_bool   ( element, "started", lib->Thread_run );
       Json_node_add_string ( element, "version", lib->version );
       Json_node_add_int    ( element, "start_time", lib->start_time );
       Json_node_add_string ( element, "objet",   lib->description );
       Json_array_add_element ( process, element );

       liste = liste->next;
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_debug: Active ou non le debug d'un process                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_process_debug ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille;

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( !(Json_has_member ( request, "thread" ) && Json_has_member ( request, "status" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    if ( Config.instance_is_master && Json_has_member ( request, "instance" ) &&
         strcasecmp ( Json_get_string(request,"instance"), "MASTER" ) &&
         strcasecmp ( Json_get_string(request,"instance"), g_get_host_name() ) )
     { Http_redirect_to_slave ( msg, Json_get_string(request,"instance") );
       //soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
       json_node_unref(request);
       return;
     }

    gchar   *thread = Json_get_string ( request,"thread" );
    gboolean status = Json_get_bool ( request, "status" );
    Modifier_configDB ( thread, "debug", (status ? "TRUE" : "FALSE") );

         if ( ! strcasecmp ( thread, "archive" ) ) { Config.log_arch = status; }
    else if ( ! strcasecmp ( thread, "dls"  ) ) { Partage->com_dls.Thread_debug = status; }
    else if ( ! strcasecmp ( thread, "msrv" ) ) { Config.log_msrv = status; }
    else
     { GSList *liste;
       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       while(liste)
        { struct LIBRAIRIE *lib;
          lib = (struct LIBRAIRIE *)liste->data;
          if ( ! strcasecmp( lib->name, thread ) ) { lib->Thread_debug = status; }
          liste = liste->next;
        }
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Setting '%s' debug to '%s'", __func__,
              thread, (status ? "TRUE" : "FALSE" ) );
/*************************************************** Envoi au client **********************************************************/
    Audit_log ( session, "Processus '%s' debug set to %s", thread, (status ? "TRUE" : "FALSE" ) );
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_start_stop: Traite une requete sur l'URI process/stop|start                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_process_start ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                   SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille;

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( ! (Json_has_member ( request, "thread" ) && Json_has_member ( request, "status" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    if ( Config.instance_is_master && Json_has_member ( request, "instance" ) &&
         strcasecmp ( Json_get_string(request,"instance"), "MASTER" ) &&
         strcasecmp ( Json_get_string(request,"instance"), g_get_host_name() ) )
     { Http_redirect_to_slave ( msg, Json_get_string(request,"instance") );
       //soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
       json_node_unref(request);
       return;
     }

    gchar   *thread = Json_get_string ( request,"thread" );
    gboolean status = Json_get_bool ( request, "status" );
    Modifier_configDB ( thread, "enable", (status ? "TRUE" : "FALSE") );

    if ( ! strcasecmp ( thread, "archive" ) )
     { if (status==FALSE) { Partage->com_arch.Thread_run = FALSE; }
       else Demarrer_arch();                                                                   /* Demarrage gestion Archivage */
     } else
    if ( ! strcasecmp ( thread, "dls"  ) )
     { if (status==FALSE) { Partage->com_dls.Thread_run  = FALSE; }
       else Demarrer_dls();                                                                               /* Démarrage D.L.S. */
     }
    else
     { GSList *liste;
       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       while(liste)
        { struct LIBRAIRIE *lib;
          lib = (struct LIBRAIRIE *)liste->data;
          if ( ! strcasecmp( lib->name, thread ) )
           { if (status) Start_librairie(lib); else Stop_librairie(lib);
           }
          liste = liste->next;
        }
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Setting '%s' to '%s'",
              __func__, thread, (status ? "START" : "STOP") );
/*************************************************** Envoi au client **********************************************************/
    Audit_log ( session, "Processus '%s' enable set to %s", thread, (status ? "TRUE" : "FALSE" ) );
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_reload: Traite une requete sur l'URI process/reload                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_process_reload ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gsize taille;

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( ! Json_has_member ( request, "thread" ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    if ( Config.instance_is_master && Json_has_member ( request, "instance" ) &&
         strcasecmp ( Json_get_string(request,"instance"), "MASTER" ) &&
         strcasecmp ( Json_get_string(request,"instance"), g_get_host_name() ) )
     { Http_redirect_to_slave ( msg, Json_get_string(request,"instance") );
       //soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
       json_node_unref(request);
       return;
     }

    gchar   *thread = Json_get_string ( request,"thread" );
    gboolean   hard = (Json_has_member ( request, "hard" ) && Json_get_bool ( request, "hard" ) );

/*************************************************** WS Reload library ********************************************************/
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Reloading start for '%s' (hard=%d)", __func__, thread, hard );
         if ( ! strcasecmp( thread, "dls" ) )     { Partage->com_dls.Thread_reload  = TRUE; }
    else if ( ! strcasecmp( thread, "archive" ) ) { Partage->com_arch.Thread_reload = TRUE; }
    else if ( ! strcasecmp( thread, "msrv" ) )    { Partage->com_msrv.Thread_run    = FALSE; }
    else if ( ! strcasecmp( thread, "http" ) )    { Cfg_http.lib->Thread_reload     = TRUE; }
    else if ( hard )
     { Decharger_librairie_par_prompt ( thread );
       Charger_librairie_par_prompt ( thread );
       sleep(1);                                                                           /* lui laisse le temps de demarrer */
     }
    else
     { Reload_librairie_par_prompt ( thread ); }
/*************************************************** Envoi au client **********************************************************/
    Audit_log ( session, "Processus '%s' %s Reloaded", thread, (hard ? "Hard" : "Soft") );
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess: Traite une requete sur l'URI process                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_process ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  {
    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;


    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: Searching for CLI commande %s", __func__, path );
    path = path + strlen("/api/process/");

/****************************************** WS get Running config library *****************************************************/
    if (!strncasecmp ( path, "archive/", strlen("archive/")))
     { Admin_arch_json ( msg, path+strlen("archive/"), query, session->access_level );
       Audit_log ( session, "Processus 'Archive': %s", path );
     }
    else
     { GSList *liste;
       if (msg->method == SOUP_METHOD_GET)                                               /* Test si Slave redirect necessaire */
        { gpointer instance = g_hash_table_lookup ( query, "instance" );
          if ( Config.instance_is_master && instance && strcasecmp ( instance, "MASTER" ) && strcasecmp ( instance, g_get_host_name() ) )
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s : Redirecting %s vers %s", __func__, path, instance );
             Http_redirect_to_slave ( msg, instance );
             return;
           }
        }
       else if (msg->method == SOUP_METHOD_PUT || msg->method == SOUP_METHOD_POST || msg->method == SOUP_METHOD_DELETE)
        { GBytes *request_brute;
          gsize taille;
          g_object_get ( msg, "request-body-data", &request_brute, NULL );
          JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
          if ( !request)
           { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
             return;
           }

          if ( Config.instance_is_master && Json_has_member ( request, "instance" ) &&
               strcasecmp ( Json_get_string(request,"instance"), "MASTER" ) &&
               strcasecmp ( Json_get_string(request,"instance"), g_get_host_name() ) )
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                       "%s : Redirecting %s vers %s", __func__, path, Json_get_string(request,"instance") );
             Http_redirect_to_slave ( msg, Json_get_string(request,"instance") );
             //soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
             json_node_unref(request);
             return;
           }
        }
       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       while(liste)
        { struct LIBRAIRIE *lib = liste->data;
          if ( ! strncasecmp( path, lib->name, strlen(lib->name) ) )
           { if (!lib->Admin_json)
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                          "%s: library %s do not have Admin_json.", __func__, lib->name );
                soup_message_set_status_full (msg, SOUP_STATUS_NOT_IMPLEMENTED, "Missing function admin_json" );
                return;
              }
             else
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Admin_json called by '%s' for %s.",
                          __func__, session->username, path );
                lib->Admin_json ( lib, msg, path+strlen(lib->name), query, session->access_level );
                Audit_log ( session, "Processus '%s': %s", lib->name, path );
                return;
              }
            }
           liste = g_slist_next(liste);
        }
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Unknown Thread" );
       return;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
