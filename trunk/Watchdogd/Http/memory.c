/******************************************************************************************************************************/
/* Watchdogd/Http/memory.c       Gestion des request memory pour le thread HTTP de watchdog                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mer. 20 nov. 2013 18:18:49 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * memory.c
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
/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Http.h"

/******************************************************************************************************************************/
/* Http_Traiter_request_getstatus: Traite une requete sur l'URI status                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gint Http_Memory_send_json ( struct lws *wsi, gchar *type, gchar *tech_id, gchar *acronyme )
  { JsonBuilder *builder;
    JsonGenerator *gen;
    gsize taille_buf;
	   gchar *buf;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = json_builder_new ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       return(1);
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    json_builder_begin_object (builder);                                                                 /* Contenu du Status */

    if (!strcasecmp(type,"CI"))
     { struct DLS_CPT_IMP *cpt_imp=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for GET CI %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_CPT_IMP ( tech_id, acronyme, (gpointer *)&cpt_imp );
       if (cpt_imp)
        { json_builder_set_member_name  ( builder, "valeur" );
          json_builder_add_int_value    ( builder, cpt_imp->valeur );
          json_builder_set_member_name  ( builder, "etat" );
          json_builder_add_boolean_value ( builder, cpt_imp->etat );
        }
     }

    json_builder_end_object (builder);                                                                        /* End Document */

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
/* Http_Traiter_request_body_completion_memory: le payload est arrivé, il faut traiter le json                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_completion_memory ( struct lws *wsi )
  { gchar *mode, *type, *tech_id, *acronyme;
    struct HTTP_PER_SESSION_DATA *pss;
    JsonObject *object;
    JsonNode *Query;

    pss = lws_wsi_user ( wsi );
    pss->post_data [ pss->post_data_length ] = 0;
    Query = json_from_string ( pss->post_data, NULL );
    pss->post_data_length = 0;
    g_free(pss->post_data);

    if (!Query)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: requete non Json", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    object = json_node_get_object (Query);
    if (!object)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Object non trouvé", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    mode = json_object_get_string_member ( object, "mode" );
    if (!mode)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: mode non trouvé", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    type = json_object_get_string_member ( object, "type" );
    if (!type)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: type non trouvé", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    tech_id = json_object_get_string_member ( object, "tech_id" );
    if (!tech_id)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: tech_id non trouvé", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    acronyme = json_object_get_string_member ( object, "acronyme" );
    if (!acronyme)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: acronyme non trouvé", __func__ );
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    if (!strcasecmp(mode, "get"))
     { return(Http_Memory_send_json ( wsi, type, tech_id, acronyme )); }
    else if (!strcasecmp(mode, "set"))
     { return(Http_Send_response_code ( wsi, HTTP_200_OK ));
     }
    return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                                 /* Bad Request */

  }
/*----------------------------------------------------------------------------------------------------------------------------*/
