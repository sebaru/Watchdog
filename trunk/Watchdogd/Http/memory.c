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
    gsize taille_buf;
	   gchar *buf;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       return(1);
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    json_builder_begin_object (builder);                                                                 /* Contenu du Status */

/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    if (!strcasecmp(type,"CI"))
     { struct DLS_CI *cpt_imp=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET CI %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_CI ( tech_id, acronyme, (gpointer *)&cpt_imp );
       if (!cpt_imp)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: cpt_imp non trouvée", __func__ );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_bool   ( builder, "etat",   cpt_imp->etat );
       Json_add_int    ( builder, "valeur", cpt_imp->valeur );
       Json_add_double ( builder, "multi",  cpt_imp->multi );
       Json_add_string ( builder, "unite",  cpt_imp->unite );
     }
/*------------------------------------------------ Compteur horaire ----------------------------------------------------------*/
    else if (!strcasecmp(type,"CH"))
     { struct DLS_CH *cpt_h=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET CH %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_CH ( tech_id, acronyme, (gpointer *)&cpt_h );
       if (!cpt_h)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: cpth non trouvée", __func__ );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_int  ( builder, "valeur", cpt_h->valeur );
       Json_add_bool ( builder, "etat",   cpt_h->etat );
     }
/*----------------------------------------------- Entrée Analogique ----------------------------------------------------------*/
    else if (!strcasecmp(type,"EA"))
     { struct DLS_AI *ai=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET EA %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_AI ( tech_id, acronyme, (gpointer *)&ai );
       if (!ai)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: ai non trouvée", __func__ );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_double ( builder, "valeur_brute", ai->val_avant_ech );
       Json_add_double ( builder, "valeur_min",   ai->min );
       Json_add_double ( builder, "valeur_max",   ai->max );
       Json_add_double ( builder, "valeur",       ai->val_ech );
       Json_add_int    ( builder, "type",         ai->type );
     }
/*----------------------------------------------- Bistable et Monostables ----------------------------------------------------*/
    else if (!strcasecmp(type,"B") || !strcasecmp(type,"M"))
     { struct DLS_BOOL *bool=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET B/M %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_bool ( tech_id, acronyme, (gpointer *)&bool );
       if (!bool)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: bool non trouvé", __func__ );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_bool ( builder, "etat", bool->etat );
     }
/*---------------------------------------------------------- Tempo -----------------------------------------------------------*/
    else if (!strcasecmp(type,"T"))
     { struct DLS_TEMPO *tempo=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET T %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_tempo ( tech_id, acronyme, (gpointer *)&tempo );
       if (!tempo)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: tempo %s:%s non trouvée", __func__, tech_id, acronyme );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_bool ( builder, "etat", tempo->state );
       Json_add_int  ( builder, "status", tempo->status );
       Json_add_int  ( builder, "daa", tempo->delai_on );
       Json_add_int  ( builder, "dma", tempo->min_on );
       Json_add_int  ( builder, "dMa", tempo->max_on );
       Json_add_int  ( builder, "dad", tempo->delai_off );
       Json_add_int  ( builder, "date_on", tempo->date_on );
       Json_add_int  ( builder, "date_off", tempo->date_off );
       Json_add_int  ( builder, "top", Partage->top );
     }
/*---------------------------------------------------------- Tempo -----------------------------------------------------------*/
    else if (!strcasecmp(type,"E"))
     { struct DLS_BOOL *bool=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET E %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_bool ( tech_id, acronyme, (gpointer *)&bool );
       if (!bool)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: bool %s:%s non trouvé", __func__, tech_id, acronyme );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_bool  ( builder, "etat", bool->etat );
     }
/*---------------------------------------------------- Visuels ---------------------------------------------------------------*/
    else if (!strcasecmp(type,"I"))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: HTTP/ request for GET I. cheking num", __func__ );
       gchar *num_s = json_object_get_string_member ( object, "num" );
       gint num;
       if (!num_s)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: num non trouvée", __func__ );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       num = atoi(num_s);
       if (num!=-1)
        { Json_add_int ( builder, "etat",   Partage->i[num].etat );
          Json_add_int ( builder, "rouge",  Partage->i[num].rouge);
          Json_add_int ( builder, "vert",   Partage->i[num].vert );
          Json_add_int ( builder, "bleu",   Partage->i[num].bleu );
          Json_add_int ( builder, "cligno", Partage->i[num].cligno );
        }
       else
        { struct DLS_VISUEL *visu=NULL;
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                    "%s: HTTP/ request for GET I %s:%s", __func__, tech_id, acronyme );
          Dls_data_get_VISUEL ( tech_id, acronyme, (gpointer *)&visu );
          if (!visu)
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: visu %s:%s non trouvé", __func__, tech_id, acronyme );
             g_object_unref(builder);
             return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
           }
          Json_add_int    ( builder, "mode",   visu->mode  );
          Json_add_string ( builder, "color",  visu->color );
          Json_add_bool   ( builder, "cligno", visu->cligno );
        }
     }

    json_builder_end_object (builder);                                                                        /* End Document */

    buf = Json_get_buf ( builder, &taille_buf );
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
    if (!strcasecmp(type,"M"))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET M %s:%s", __func__, tech_id, acronyme );
       Envoyer_commande_dls_data ( tech_id, acronyme );
       return(Http_Send_response_code ( wsi, HTTP_200_OK ));
     }
/************************************************ Préparation du buffer JSON **************************************************/
    else if (!strcasecmp(type,"CI"))
     { struct DLS_CI *cpt_imp=NULL;
       gchar *valeur = json_object_get_string_member ( object, "valeur" );
       if (!valeur)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: valeur non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
        }
       gchar *unite = json_object_get_string_member ( object, "unite" );
       if (!unite)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: unite non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
        }
       gchar *multi = json_object_get_string_member ( object, "multi" );
       if (!multi)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: multi non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
        }
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET CI %s:%s = %s %s multi %s", __func__,
                  tech_id, acronyme, valeur, unite, multi );
       Dls_data_get_CI ( tech_id, acronyme, (gpointer *)&cpt_imp );
       if (cpt_imp)
        { cpt_imp->valeur = atoi(valeur);
          cpt_imp->multi  = atof(multi);
          g_snprintf( cpt_imp->unite, sizeof(cpt_imp->unite), "%s", unite );
        }
       return(Http_Send_response_code ( wsi, HTTP_200_OK ));
     }
/************************************************ Préparation du buffer JSON **************************************************/
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
