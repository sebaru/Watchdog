/******************************************************************************************************************************/
/* Watchdogd/Http/getprocess.c       Gestion des request getprocess pour le thread HTTP de watchdog                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    30.12.2018 01:59:26 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getprocess.c
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

 #include <string.h>
 #include <unistd.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_list: Traite une requete sur l'URI process/list                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gint Http_Traiter_request_getprocess_list ( struct lws *wsi )
  { JsonBuilder *builder;
    JsonGenerator *gen;
    gsize taille_buf;
    GSList *liste;
    gchar *buf;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = json_builder_new ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       return(1);
     }

    json_builder_begin_array (builder);                                                                  /* Contenu du Status */

    json_builder_begin_object (builder);                                                                 /* Contenu du Status */
    json_builder_set_member_name  ( builder, "thread" );
    json_builder_add_string_value ( builder, "msrv" );
    json_builder_set_member_name  ( builder, "debug" );
    json_builder_add_boolean_value( builder, Config.log_msrv );
    json_builder_set_member_name  ( builder, "started" );
    json_builder_add_boolean_value( builder, Partage->com_msrv.Thread_run );
    json_builder_set_member_name  ( builder, "objet" );
    json_builder_add_string_value ( builder, "Local Master Server" );
    json_builder_set_member_name  ( builder, "fichier" );
    json_builder_add_string_value ( builder, "built-in" );
    json_builder_end_object (builder);                                                                        /* End Document */

    json_builder_begin_object (builder);                                                                 /* Contenu du Status */
    json_builder_set_member_name  ( builder, "thread" );
    json_builder_add_string_value ( builder, "dls" );
    json_builder_set_member_name  ( builder, "debug" );
    json_builder_add_boolean_value( builder, Partage->com_dls.Thread_debug );
    json_builder_set_member_name  ( builder, "started" );
    json_builder_add_boolean_value( builder, Partage->com_dls.Thread_run );
    json_builder_set_member_name  ( builder, "objet" );
    json_builder_add_string_value ( builder, "D.L.S" );
    json_builder_set_member_name  ( builder, "fichier" );
    json_builder_add_string_value ( builder, "built-in" );
    json_builder_end_object (builder);                                                                        /* End Document */

    json_builder_begin_object (builder);                                                                 /* Contenu du Status */
    json_builder_set_member_name  ( builder, "thread" );
    json_builder_add_string_value ( builder, "arch" );
    json_builder_set_member_name  ( builder, "debug" );
    json_builder_add_boolean_value( builder, Config.log_arch );
    json_builder_set_member_name  ( builder, "started" );
    json_builder_add_boolean_value( builder, Partage->com_arch.Thread_run );
    json_builder_set_member_name  ( builder, "objet" );
    json_builder_add_string_value ( builder, "Archivage" );
    json_builder_set_member_name  ( builder, "fichier" );
    json_builder_add_string_value ( builder, "built-in" );
    json_builder_end_object (builder);                                                                        /* End Document */

    liste = Partage->com_msrv.Librairies;                                                /* Parcours de toutes les librairies */
    while(liste)
     { struct LIBRAIRIE *lib = liste->data;
       json_builder_begin_object (builder);                                                                 /* Contenu du Status */
       json_builder_set_member_name  ( builder, "thread" );
       json_builder_add_string_value ( builder, lib->admin_prompt );
       json_builder_set_member_name  ( builder, "debug" );
       json_builder_add_boolean_value ( builder, lib->Thread_debug );
       json_builder_set_member_name  ( builder, "started" );
       json_builder_add_boolean_value ( builder, lib->Thread_run );
       json_builder_set_member_name  ( builder, "objet" );
       json_builder_add_string_value ( builder, lib->admin_help );
       json_builder_set_member_name  ( builder, "fichier" );
       json_builder_add_string_value ( builder, lib->nom_fichier );
       json_builder_end_object (builder);                                                                        /* End Document */

       liste = liste->next;
     }
    json_builder_end_array (builder);                                                                         /* End Document */

    gen = json_generator_new ();
    json_generator_set_root ( gen, json_builder_get_root(builder) );
    json_generator_set_pretty ( gen, TRUE );
    buf = json_generator_to_data (gen, &taille_buf);
    g_object_unref(builder);
    g_object_unref(gen);

/*************************************************** Envoi au client **********************************************************/
    return(Http_Send_response_code_with_buffer ( wsi, HTTP_200_OK, HTTP_CONTENT_JSON, buf, taille_buf ));
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_debug: Active ou non le debug d'un process                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 static gint Http_Traiter_request_getprocess_debug ( struct lws *wsi, gchar *thread, gboolean status )
  { if ( ! strcasecmp ( thread, "arch" ) ) { Config.log_arch = status; }
    else
    if ( ! strcasecmp ( thread, "dls"  ) ) { Partage->com_dls.Thread_debug = status; }
    else
    if ( ! strcasecmp ( thread, "db" ) )   { Config.log_db = status; }
    else
    if ( ! strcasecmp ( thread, "msrv" ) ) { Config.log_msrv = status; }
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
    return(Http_Send_response_code ( wsi, HTTP_200_OK ));
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_start_stop: Traite une requete sur l'URI process/stop|start                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 static gint Http_Traiter_request_getprocess_start_stop ( struct lws *wsi, gchar *thread, gboolean status )
  { if ( ! strcasecmp ( thread, "arch" ) )
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
          if ( ! strcasecmp( lib->admin_prompt, thread ) )
           { if (status) Start_librairie(lib); else Stop_librairie(lib); }
          liste = liste->next;
        }
     }
    return(Http_Send_response_code ( wsi, HTTP_200_OK ));
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess: Traite une requete sur l'URI process                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gint Http_Traiter_request_getprocess ( struct lws *wsi, gchar *url )
  {
    if (!strcasecmp(url, "list"))
     { return(Http_Traiter_request_getprocess_list(wsi)); }
    else if (!strncmp(url, "stop/", 5))
     { return(Http_Traiter_request_getprocess_start_stop(wsi, url+5, FALSE));}
    else if (!strncmp(url, "start/", 6))
     { return(Http_Traiter_request_getprocess_start_stop(wsi, url+6, TRUE));}
    else if (!strncmp(url, "undebug/", 8))
     { return(Http_Traiter_request_getprocess_debug(wsi, url+8, FALSE));}
    else if (!strncmp(url, "debug/", 6))
     { return(Http_Traiter_request_getprocess_debug(wsi, url+6, TRUE));}
/*************************************************** WS Reload library ********************************************************/
    else if ( ! strncasecmp( url, "reload/", 7 ) )
     { gchar *target = url+7;
       GSList *liste;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Reloading start for %s", __func__, target );
       if ( ! strcasecmp( target, "dls" ) )
        { Partage->com_dls.Thread_reload = TRUE;
          return(Http_Send_response_code ( wsi, HTTP_200_OK ));
        }
       else if ( ! strcasecmp( target, "arch" ) )
        { Partage->com_arch.Thread_reload = TRUE;
          return(Http_Send_response_code ( wsi, HTTP_200_OK ));
        }

       liste = Partage->com_msrv.Librairies;                                  /* Parcours de toutes les librairies */
       while(liste)
        { struct LIBRAIRIE *lib = liste->data;
          if ( ! strcasecmp( target, lib->admin_prompt ) )
           { if (lib->Thread_run == FALSE)
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                         "%s: reloading %s -> Library found but not started.", __func__, target );
              }
             else
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                         "%s: reloading %s -> Library found. Sending Reload.", __func__, target );
                lib->Thread_reload = TRUE;
              }
           }
          liste = g_slist_next(liste);
        }
       return(Http_Send_response_code ( wsi, HTTP_200_OK ));
     }
    return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
