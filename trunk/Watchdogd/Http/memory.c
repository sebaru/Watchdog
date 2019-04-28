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
/* Http_Memory_get : Renvoi, au format json, la valeur d'un bit interne                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gint Http_Memory_get ( struct lws *wsi, JsonObject *object, gchar *type, gchar *tech_id, gchar *acronyme )
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
     { struct DLS_CI *cpt_imp=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET CI %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_CI ( tech_id, acronyme, (gpointer *)&cpt_imp );
       if (cpt_imp)
        { json_builder_set_member_name  ( builder, "valeur" );
          json_builder_add_int_value    ( builder, cpt_imp->valeur );
          json_builder_set_member_name  ( builder, "etat" );
          json_builder_add_boolean_value ( builder, cpt_imp->etat );
        }
     }
    if (!strcasecmp(type,"CH"))
     { struct DLS_CH *cpt_h=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET CH %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_CH ( tech_id, acronyme, (gpointer *)&cpt_h );
       if (cpt_h)
        { json_builder_set_member_name  ( builder, "valeur" );
          json_builder_add_int_value    ( builder, cpt_h->valeur );
          json_builder_set_member_name  ( builder, "etat" );
          json_builder_add_boolean_value ( builder, cpt_h->etat );
        }
     }
    else if (!strcasecmp(type,"I"))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: HTTP/ request for GET I. cheking num", __func__ );
       gchar *num_s = json_object_get_string_member ( object, "num" );
       gint num;
       if (!num_s)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: num non trouvée", __func__ );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
        }
       num = atoi(num_s);
       json_builder_set_member_name  ( builder, "etat" );
       json_builder_add_int_value    ( builder, Partage->i[num].etat );
       json_builder_set_member_name  ( builder, "rouge" );
       json_builder_add_int_value    ( builder, Partage->i[num].rouge);
       json_builder_set_member_name  ( builder, "vert" );
       json_builder_add_int_value    ( builder, Partage->i[num].vert );
       json_builder_set_member_name  ( builder, "bleu" );
       json_builder_add_int_value    ( builder, Partage->i[num].bleu );
       json_builder_set_member_name  ( builder, "cligno" );
       json_builder_add_int_value    ( builder, Partage->i[num].cligno );
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
/* Http_Memory_set: Positionne le bit interne en parametre                                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gint Http_Memory_set ( struct lws *wsi, JsonObject *object, gchar *type, gchar *tech_id, gchar *acronyme )
  {

/************************************************ Préparation du buffer JSON **************************************************/
    if (!strcasecmp(type,"CI"))
     { struct DLS_CI *cpt_imp=NULL;
       gchar *valeur = json_object_get_string_member ( object, "valeur" );
       if (!valeur)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: valeur non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
        }
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET CI %s:%s = %s", __func__, tech_id, acronyme, valeur );
       Dls_data_get_CI ( tech_id, acronyme, (gpointer *)&cpt_imp );
       if (cpt_imp)
        { cpt_imp->valeur = atoi(valeur); }
       return(Http_Send_response_code ( wsi, HTTP_200_OK ));
     }
    else if (!strcasecmp(type,"CH"))
     { struct DLS_CH *cpt_h=NULL;
       gchar *valeur = json_object_get_string_member ( object, "valeur" );
       if (!valeur)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: valeur non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
        }
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET CH %s:%s = %s", __func__, tech_id, acronyme, valeur );
       Dls_data_get_CH ( tech_id, acronyme, (gpointer *)&cpt_h );
       if (cpt_h)
        { cpt_h->valeur = atoi(valeur); }
       return(Http_Send_response_code ( wsi, HTTP_200_OK ));
     }
/*************************************************** Envoi au client **********************************************************/
    return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));
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
    gint retour;

    pss = lws_wsi_user ( wsi );
    Query = json_from_string ( pss->post_data, NULL );
    pss->post_data_length = 0;

    if (!Query)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: requete non Json", __func__ );
       g_free(pss->post_data);
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    object = json_node_get_object (Query);
    if (!object)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Object non trouvé", __func__ );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    mode = json_object_get_string_member ( object, "mode" );
    if (!mode)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: mode non trouvé", __func__ );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    type = json_object_get_string_member ( object, "type" );
    if (!type)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: type non trouvé", __func__ );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    tech_id = json_object_get_string_member ( object, "tech_id" );
    if (!tech_id)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: tech_id non trouvé", __func__ );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    acronyme = json_object_get_string_member ( object, "acronyme" );
    if (!acronyme)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: acronyme non trouvé", __func__ );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    if (!strcasecmp(mode, "get"))
     { retour = Http_Memory_get ( wsi, object, type, tech_id, acronyme );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(retour);
     }
    else if (!strcasecmp(mode, "set"))
     { retour = Http_Memory_set ( wsi, object, type, tech_id, acronyme );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(retour);
     }

    json_node_unref (Query);
    g_free(pss->post_data);
    return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                                 /* Bad Request */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
