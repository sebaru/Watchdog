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

    json_builder_begin_object (builder);                                                                 /* Contenu du Status */

    liste = Partage->com_msrv.Librairies;                                                /* Parcours de toutes les librairies */
    while(liste)
     { struct LIBRAIRIE *lib = liste->data;
       json_builder_set_member_name  ( builder, lib->admin_prompt );
       json_builder_begin_object (builder);                                                    /* Contenu du Noeud Passerelle */
       json_builder_set_member_name  ( builder, "Running" );
       json_builder_add_boolean_value ( builder, lib->Thread_run );
       json_builder_set_member_name  ( builder, "Fichier" );
       json_builder_add_string_value ( builder, lib->nom_fichier );
       json_builder_set_member_name  ( builder, "Objet" );
       json_builder_add_string_value ( builder, lib->admin_help );
       json_builder_end_object (builder);                                                                       /* End Module */

       liste = liste->next;
     }
    json_builder_end_object (builder);                                                                        /* End Document */

    gen = json_generator_new ();
    json_generator_set_root ( gen, json_builder_get_root(builder) );
    json_generator_set_pretty ( gen, TRUE );
    buf = json_generator_to_data (gen, &taille_buf);
    g_object_unref(builder);
    g_object_unref(gen);

/*************************************************** Envoi au client **********************************************************/
    Http_Send_response_code_with_buffer ( wsi, HTTP_200_OK, HTTP_CONTENT_JSON, buf, taille_buf );
    g_free(buf);                                                      /* Libération du buffer dont nous n'avons plus besoin ! */
    return(lws_http_transaction_completed(wsi));
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess_start_stop: Traite une requete sur l'URI process/stop|start                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gint Http_Traiter_request_getprocess_start_stop ( struct lws *wsi, gchar *thread, gboolean status )
  { if ( ! strcmp ( thread, "arch" ) )
     { if (status==FALSE) { Partage->com_arch.Thread_run = FALSE; }
       else Demarrer_arch();                                                                   /* Demarrage gestion Archivage */
     } else
    if ( ! strcmp ( thread, "dls"  ) )
     { if (status==FALSE) { Partage->com_dls.Thread_run  = FALSE; }
       else Demarrer_dls();                                                                               /* Démarrage D.L.S. */
     }
    else
     { GSList *liste;
       gint found;
       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       found = 0;
       while(liste)
        { struct LIBRAIRIE *lib;
          lib = (struct LIBRAIRIE *)liste->data;
          if ( ! strcmp( lib->admin_prompt, thread ) )
           { if (status) Start_librairie(lib); else Stop_librairie(lib); }
          liste = liste->next;
        }
     }
    Http_Send_response_code ( wsi, HTTP_200_OK );
    return(lws_http_transaction_completed(wsi));
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_getprocess: Traite une requete sur l'URI process                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gint Http_Traiter_request_getprocess ( struct lws *wsi, gchar *url )
  { 
/************************************************ Préparation du buffer JSON **************************************************/
    if (!strcmp(url, "list"))
     { return(Http_Traiter_request_getprocess_list(wsi));}
    else if (!strncmp(url, "stop/", 5))
     { return(Http_Traiter_request_getprocess_start_stop(wsi,url+5,FALSE));}
    else if (!strncmp(url, "start/", 6))
     { return(Http_Traiter_request_getprocess_start_stop(wsi,url+6,TRUE));}
    Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );
    return(1);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/