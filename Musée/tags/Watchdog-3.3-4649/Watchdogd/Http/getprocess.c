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
 static void Http_traiter_process_list ( SoupMessage *msg )
  { JsonBuilder *builder;
    gsize taille_buf;
    GSList *liste;
    gchar *buf;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_array ( builder, "Process" );                                                               /* Contenu du Status */

    Json_add_object ( builder, NULL );                                                                /* Contenu du Status */
    Json_add_string ( builder, "thread",  "msrv" );
    Json_add_bool   ( builder, "debug",   Config.log_msrv );
    Json_add_bool   ( builder, "started", Partage->com_msrv.Thread_run );
    Json_add_string ( builder, "objet",   "Local Master Server" );
    Json_add_string ( builder, "fichier", "built-in" );
    Json_end_object ( builder );                                                                              /* End Document */

    Json_add_object ( builder, NULL );                                                                /* Contenu du Status */
    Json_add_string ( builder, "thread",  "dls" );
    Json_add_bool   ( builder, "debug",   Partage->com_dls.Thread_debug );
    Json_add_bool   ( builder, "started", Partage->com_dls.Thread_run );
    Json_add_string ( builder, "objet",   "D.L.S" );
    Json_add_string ( builder, "fichier", "built-in" );
    Json_end_object ( builder );                                                                              /* End Document */

    Json_add_object ( builder, NULL );                                                                /* Contenu du Status */
    Json_add_string ( builder, "thread",  "arch" );
    Json_add_bool   ( builder, "debug",   Config.log_arch );
    Json_add_bool   ( builder, "started", Partage->com_arch.Thread_run );
    Json_add_string ( builder, "objet",   "Archivage" );
    Json_add_string ( builder, "fichier", "built-in" );
    Json_end_object ( builder );                                                                              /* End Document */

    Json_add_object ( builder, NULL );                                                                /* Contenu du Status */
    Json_add_string ( builder, "thread",  "db" );
    Json_add_bool   ( builder, "debug",   Config.log_db );
    Json_add_bool   ( builder, "started", TRUE );
    Json_add_string ( builder, "objet",   "Database Access" );
    Json_add_string ( builder, "fichier", "built-in" );
    Json_end_object ( builder );                                                                              /* End Document */

    liste = Partage->com_msrv.Librairies;                                                /* Parcours de toutes les librairies */
    while(liste)
     { struct LIBRAIRIE *lib = liste->data;
       Json_add_object ( builder, NULL );                                                                /* Contenu du Status */
       Json_add_string ( builder, "thread",  lib->admin_prompt );
       Json_add_bool   ( builder, "debug",   lib->Thread_debug );
       Json_add_bool   ( builder, "started", lib->Thread_run );
       Json_add_string ( builder, "objet",   lib->admin_help );
       Json_add_string ( builder, "fichier", lib->nom_fichier );
       Json_end_object ( builder );                                                                           /* End Document */

       liste = liste->next;
     }
    Json_end_array ( builder );

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_debug: Active ou non le debug d'un process                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 static void Http_traiter_process_debug ( SoupMessage *msg,  gchar *thread, gboolean status )
  { gsize taille_buf;
         if ( ! strcasecmp ( thread, "arch" ) ) { Config.log_arch = status; }
    else if ( ! strcasecmp ( thread, "dls"  ) ) { Partage->com_dls.Thread_debug = status; }
    else if ( ! strcasecmp ( thread, "db" ) )   { Config.log_db = status; }
    else if ( ! strcasecmp ( thread, "msrv" ) ) { Config.log_msrv = status; }
    else
     { GSList *liste;
       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       while(liste)
        { struct LIBRAIRIE *lib;
          lib = (struct LIBRAIRIE *)liste->data;
          if ( ! strcasecmp( lib->admin_prompt, thread ) ) { lib->Thread_debug = status; }
          liste = liste->next;
        }
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Setting '%s' debug to %d", __func__, thread, status );
/************************************************ Préparation du buffer JSON **************************************************/
    JsonBuilder *builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_string ( builder, "thread",  thread );
    Json_add_bool   ( builder, "debug", status );

    gchar *buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_start_stop: Traite une requete sur l'URI process/stop|start                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 static void Http_traiter_process_start_stop ( SoupMessage *msg, gchar *thread, gboolean status )
  { gsize taille_buf;

    if ( ! strcasecmp ( thread, "arch" ) )
     { if (status==FALSE) { Partage->com_arch.Thread_run = FALSE; }
       else Demarrer_arch();                                                                   /* Demarrage gestion Archivage */
     } else
    if ( ! strcasecmp ( thread, "dls"  ) )
     { if (status==FALSE) { Partage->com_dls.Thread_run  = FALSE; }
       else Demarrer_dls();                                                                               /* Démarrage D.L.S. */
     }
    if ( ! strcasecmp ( thread, "db"  ) )
     { status=TRUE;                                                                       /* Le thread DB ne peut etre arreté */
     }
    else
     { GSList *liste;
       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       while(liste)
        { struct LIBRAIRIE *lib;
          lib = (struct LIBRAIRIE *)liste->data;
          if ( ! strcasecmp( lib->admin_prompt, thread ) )
           { if (status) Start_librairie(lib); else Stop_librairie(lib); }
          liste = liste->next;
        }
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Setting '%s' to '%s'",
                          __func__, thread, (status ? "start" : "stop") );
/************************************************ Préparation du buffer JSON **************************************************/
    JsonBuilder *builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_string ( builder, "thread",  thread );
    Json_add_bool   ( builder, "started", status );

    gchar *buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess: Traite une requete sur l'URI process                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_traiter_process ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );

    if ( ! (session && session->access_level >= 6) )
     { soup_message_set_status (msg, SOUP_STATUS_FORBIDDEN);
       return;
     }
         if (!strcasecmp(path, "/process/list"))          { Http_traiter_process_list(msg); }
    else if (g_str_has_prefix(path, "/process/stop/"))    { Http_traiter_process_start_stop( msg, path+14, FALSE ); }
    else if (g_str_has_prefix(path, "/process/start/"))   { Http_traiter_process_start_stop( msg, path+15, TRUE );  }
    else if (g_str_has_prefix(path, "/process/undebug/")) { Http_traiter_process_debug( msg, path+17, FALSE );      }
    else if (g_str_has_prefix(path, "/process/debug/"))   { Http_traiter_process_debug( msg, path+15, TRUE );       }
/*************************************************** WS Reload library ********************************************************/
    else if (g_str_has_prefix(path, "/process/reload/"))
     { gchar *target = path+16;
       GSList *liste;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Reloading start for %s", __func__, target );
       if ( ! strcasecmp( target, "dls" ) )
        { Partage->com_dls.Thread_reload = TRUE;
          soup_message_set_status (msg, SOUP_STATUS_OK);
          return;
        }
       else if ( ! strcasecmp( target, "arch" ) )
        { Partage->com_arch.Thread_reload = TRUE;
          soup_message_set_status (msg, SOUP_STATUS_OK);
          return;
        }

       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       while(liste)
        { struct LIBRAIRIE *lib = liste->data;
          if ( ! strcasecmp( target, lib->admin_prompt ) )
           { if (lib->Thread_run == FALSE)
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                         "%s: reloading %s -> Library found but not started. Please Start %s before reload",
                         __func__, target, target );
              }
             else
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                         "%s: reloading %s -> Library found. Sending Reload.", __func__, target );
                lib->Thread_reload = TRUE;
              }
           }
          liste = g_slist_next(liste);
        }
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }
/****************************************** WS get Running config library *****************************************************/
    else
     { GSList *liste;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: Searching for CLI commande %s", __func__, path );
       path=path+9;
       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       while(liste)
        { struct LIBRAIRIE *lib = liste->data;
          if ( ! strncasecmp( path, lib->admin_prompt, strlen(lib->admin_prompt) ) )
           { if (!lib->Admin_json)
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                          "%s: library %s do not have Admin_json.", __func__, lib->admin_prompt );
              }
             else
              { gint taille_buf;
                gchar *buffer;
                Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Admin_json call for %s%s.", __func__,
                          lib->admin_prompt, path+strlen(lib->admin_prompt) );
                lib->Admin_json ( path+strlen(lib->admin_prompt), &buffer, &taille_buf );
                soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buffer, taille_buf );
                soup_message_set_status ( msg, SOUP_STATUS_OK) ;
                return;
              }
            }
           liste = g_slist_next(liste);
        }
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
