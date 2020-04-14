/******************************************************************************************************************************/
/* Watchdogd/Http/memory.c       Gestion des request memory pour le thread HTTP de watchdog                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mer. 20 nov. 2013 18:18:49 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * memory.c
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
/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;


/******************************************************************************************************************************/
/* Http_Memory_print_CI_to_json : Formate un CI au format JSON                                                                */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_CI_to_json ( JsonBuilder *builder, struct DLS_CI *bit )
  { Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_int    ( builder, "valeur", bit->valeur );
    Json_add_int    ( builder, "imp_par_minute", bit->imp_par_minute );
    Json_add_double ( builder, "multi",  bit->multi );
    Json_add_string ( builder, "unite",  bit->unite );
  };
/******************************************************************************************************************************/
/* Http_Memory_print_CH_to_json : Formate un CH au format JSON                                                                */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_CH_to_json ( JsonBuilder *builder, struct DLS_CH *bit )
  { Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_int  ( builder, "valeur", bit->valeur );
    Json_add_bool ( builder, "etat",   bit->etat );
    Json_add_int  ( builder, "last_arch", bit->last_arch );
  };
/******************************************************************************************************************************/
/* Http_Memory_print_BOOL_to_json : Formate un bit au format JSON                                                             */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_BOOL_to_json ( JsonBuilder *builder, struct DLS_BOOL *bit )
  { Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_bool ( builder, "etat", bit->etat );
  }
/******************************************************************************************************************************/
/* Http_Memory_print_DI_to_json : Formate un bit au format JSON                                                               */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_DI_to_json ( JsonBuilder *builder, struct DLS_DI *bit )
  { Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_bool ( builder, "etat", bit->etat );
  }
/******************************************************************************************************************************/
/* Http_Memory_print_DO_to_json : Formate un bit au format JSON                                                               */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_DO_to_json ( JsonBuilder *builder, struct DLS_DO *bit )
  { Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_bool ( builder, "etat", bit->etat );
  }
/******************************************************************************************************************************/
/* Http_Memory_print_AI_to_json : Formate un bit au format JSON                                                               */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_AI_to_json ( JsonBuilder *builder, struct DLS_AI *bit )
  { Json_add_string ( builder, "acronyme",     bit->acronyme );
    Json_add_double ( builder, "valeur_brute", bit->val_avant_ech );
    Json_add_double ( builder, "valeur_min",   bit->min );
    Json_add_double ( builder, "valeur_max",   bit->max );
    Json_add_double ( builder, "valeur",       bit->val_ech );
    Json_add_int    ( builder, "type",         bit->type );
    Json_add_int    ( builder, "in_range",     bit->inrange );
    Json_add_int    ( builder, "last_arch",    bit->last_arch );
  }
/******************************************************************************************************************************/
/* Http_Memory_print_AO_to_json : Formate un bit au format JSON                                                               */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_AO_to_json ( JsonBuilder *builder, struct DLS_AO *bit )
  { Json_add_string ( builder, "acronyme",     bit->acronyme );
    Json_add_double ( builder, "valeur_brute", bit->val_avant_ech );
    Json_add_double ( builder, "valeur_min",   bit->min );
    Json_add_double ( builder, "valeur_max",   bit->max );
    Json_add_double ( builder, "valeur",       bit->val_ech );
    Json_add_int    ( builder, "type",         bit->type );
  }
/******************************************************************************************************************************/
/* Http_Memory_print_TEMPO_to_json : Formate un bit au format JSON                                                               */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_TEMPO_to_json ( JsonBuilder *builder, struct DLS_TEMPO *bit )
  { Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_bool ( builder, "etat", bit->state );
    Json_add_int  ( builder, "status", bit->status );
    Json_add_int  ( builder, "daa", bit->delai_on );
    Json_add_int  ( builder, "dma", bit->min_on );
    Json_add_int  ( builder, "dMa", bit->max_on );
    Json_add_int  ( builder, "dad", bit->delai_off );
    Json_add_int  ( builder, "date_on", bit->date_on );
    Json_add_int  ( builder, "date_off", bit->date_off );
  }
/******************************************************************************************************************************/
/* Http_Memory_print_VISUEL_to_json : Formate un bit au format JSON                                                           */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_VISUEL_to_json ( JsonBuilder *builder, struct DLS_VISUEL *bit )
  { Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_int    ( builder, "mode",   bit->mode  );
    Json_add_string ( builder, "color",  bit->color );
    Json_add_bool   ( builder, "cligno", bit->cligno );
  }
/******************************************************************************************************************************/
/* Http_Memory_print_MESSAGE_to_json : Formate un bit au format JSON                                                          */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_MESSAGE_to_json ( JsonBuilder *builder, struct DLS_MESSAGES *bit )
  { Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_bool ( builder, "etat", bit->etat );
  }
/******************************************************************************************************************************/
/* Http_Memory_print_REGISTRE_to_json : Formate un bit au format JSON                                                           */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_Memory_print_REGISTRE_to_json ( JsonBuilder *builder, struct DLS_REGISTRE *bit )
  { Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_double ( builder, "valeur", bit->valeur );
    Json_add_string ( builder, "unite", bit->unite );
    Json_add_bool   ( builder, "archivage", bit->archivage );
    Json_add_int    ( builder, "last_arch", bit->last_arch );
  }
/******************************************************************************************************************************/
/* Http_Memory_get : Renvoi, au format json, la valeur d'un bit interne                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gint Http_Memory_get_all ( struct lws *wsi, gchar *tech_id )
  { JsonBuilder *builder;
    gsize taille_buf;
    GSList *liste;
	   gchar *buf;

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: HTTP/ request for GET_ALL '%s'", __func__, tech_id );
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
    Json_add_string ( builder, "tech_id", tech_id );
    Json_add_int    ( builder, "top", Partage->top );
/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    Json_add_array ( builder, "CI" );
    liste = Partage->Dls_data_CI;
    while(liste)
     { struct DLS_CI *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_CI_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    Json_add_array ( builder, "BOOL" );
    liste = Partage->Dls_data_BOOL;
    while(liste)
     { struct DLS_BOOL *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_BOOL_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*--------------------------------------------------- Compteur horaires ------------------------------------------------------*/
    Json_add_array ( builder, "CH" );
    liste = Partage->Dls_data_CH;
    while(liste)
     { struct DLS_CH *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_CH_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Entrée Analogique ----------------------------------------------------------*/
    Json_add_array ( builder, "AI" );
    liste = Partage->Dls_data_AI;
    while(liste)
     { struct DLS_AI *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_AI_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Sortie Analogique ----------------------------------------------------------*/
    Json_add_array ( builder, "AO" );
    liste = Partage->Dls_data_AO;
    while(liste)
     { struct DLS_AO *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_AO_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Temporisations -------------------------------------------------------------*/
    Json_add_array ( builder, "T" );
    liste = Partage->Dls_data_TEMPO;
    while(liste)
     { struct DLS_TEMPO *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_TEMPO_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Entrées TOR ----------------------------------------------------------------*/
    Json_add_array ( builder, "DI" );
    liste = Partage->Dls_data_DI;
    while(liste)
     { struct DLS_DI *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_DI_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Sortie TOR -----------------------------------------------------------------*/
    Json_add_array ( builder, "DO" );
    liste = Partage->Dls_data_DO;
    while(liste)
     { struct DLS_DO *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_DO_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Visuels --------------------------------------------------------------------*/
    Json_add_array ( builder, "I" );
    liste = Partage->Dls_data_VISUEL;
    while(liste)
     { struct DLS_VISUEL *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_VISUEL_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );

/*----------------------------------------------- Messages -------------------------------------------------------------------*/
    Json_add_array ( builder, "MSG" );
    liste = Partage->Dls_data_MSG;
    while(liste)
     { struct DLS_MESSAGES *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_MESSAGE_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*----------------------------------------------- Registre -------------------------------------------------------------------*/
    Json_add_array ( builder, "R" );
    liste = Partage->Dls_data_REGISTRE;
    while(liste)
     { struct DLS_REGISTRE *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Http_Memory_print_REGISTRE_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*------------------------------------------------------- fin ----------------------------------------------------------------*/
    json_builder_end_object (builder);                                                                        /* End Document */

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
    return(Http_Send_response_code_with_buffer ( wsi, HTTP_200_OK, HTTP_CONTENT_JSON, buf, taille_buf ));
  }
/******************************************************************************************************************************/
/* Http_Memory_get : Renvoi, au format json, la valeur d'un bit interne                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gint Http_Memory_get ( struct lws *wsi, JsonObject *request, gchar *type, gchar *tech_id, gchar *acronyme )
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
    Json_add_string ( builder, "tech_id", tech_id );
    Json_add_string ( builder, "acronyme", acronyme );
    Json_add_int    ( builder, "top", Partage->top );

/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    if (!strcasecmp(type,"CI"))
     { struct DLS_CI *cpt_imp=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET CI %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_CI ( tech_id, acronyme, (gpointer *)&cpt_imp );
       if (!cpt_imp)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: cpt_imp '%s:%s' non trouvée", __func__, tech_id, acronyme );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_bool   ( builder, "etat",   cpt_imp->etat );
       Json_add_int    ( builder, "valeur", cpt_imp->valeur );
       Json_add_int    ( builder, "imp_par_minute", cpt_imp->imp_par_minute );
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
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: cpth '%s:%s' non trouvée", __func__, tech_id, acronyme );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_int  ( builder, "valeur", cpt_h->valeur );
       Json_add_bool ( builder, "etat",   cpt_h->etat );
       Json_add_int  ( builder, "last_arch", cpt_h->last_arch );
     }
/*----------------------------------------------- Entrée Analogique ----------------------------------------------------------*/
    else if (!strcasecmp(type,"EA"))
     { struct DLS_AI *ai=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET EA %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_AI ( tech_id, acronyme, (gpointer *)&ai );
       if (!ai)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: ai '%s:%s' non trouvée", __func__, tech_id, acronyme );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_double ( builder, "valeur_brute", ai->val_avant_ech );
       Json_add_double ( builder, "valeur_min",   ai->min );
       Json_add_double ( builder, "valeur_max",   ai->max );
       Json_add_double ( builder, "valeur",       ai->val_ech );
       Json_add_int    ( builder, "type",         ai->type );
       Json_add_int    ( builder, "in_range",     ai->inrange );
       Json_add_int    ( builder, "last_arch",    ai->last_arch );
     }
/*----------------------------------------------- Entrée Analogique ----------------------------------------------------------*/
    else if (!strcasecmp(type,"AO"))
     { struct DLS_AO *ao=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET AO %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_AO ( tech_id, acronyme, (gpointer *)&ao );
       if (!ao)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: ao '%s:%s' non trouvée", __func__, tech_id, acronyme );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_double ( builder, "valeur_brute", ao->val_avant_ech );
       Json_add_double ( builder, "valeur_min",   ao->min );
       Json_add_double ( builder, "valeur_max",   ao->max );
       Json_add_double ( builder, "valeur",       ao->val_ech );
       Json_add_int    ( builder, "type",         ao->type );
     }
/*----------------------------------------------- Bistable et Monostables ----------------------------------------------------*/
    else if (!strcasecmp(type,"B") || !strcasecmp(type,"M"))
     { struct DLS_BOOL *bool=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET B/M %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_bool ( tech_id, acronyme, (gpointer *)&bool );
       if (!bool)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: bool '%s:%s' non trouvé", __func__, tech_id, acronyme );
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
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: tempo '%s:%s' non trouvée", __func__, tech_id, acronyme );
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
     { struct DLS_DI *di=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET DI %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_DI ( tech_id, acronyme, (gpointer *)&di );
       if (!di)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: DI '%s:%s' non trouvé", __func__, tech_id, acronyme );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_bool  ( builder, "etat", di->etat );
     }
/*---------------------------------------------------- Visuels ---------------------------------------------------------------*/
    else if (!strcasecmp(type,"I"))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: HTTP/ request for GET I. cheking num", __func__ );
       gchar *num_s = json_object_get_string_member ( request, "num" );
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
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: visu '%s:%s' non trouvé", __func__, tech_id, acronyme );
             g_object_unref(builder);
             return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
           }
          Json_add_int    ( builder, "mode",   visu->mode  );
          Json_add_string ( builder, "color",  visu->color );
          Json_add_bool   ( builder, "cligno", visu->cligno );
        }
     }
/*---------------------------------------------------- Messages --------------------------------------------------------------*/
    else if (!strcasecmp(type,"MSG"))
     { struct DLS_MESSAGES *msg=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET MSG %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_MSG ( tech_id, acronyme, (gpointer *)&msg );
       if (!msg)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: msg '%s:%s' non trouvée", __func__, tech_id, acronyme );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_bool ( builder, "etat",   msg->etat );
     }
/*--------------------------------------------------- Registres --------------------------------------------------------------*/
    else if (!strcasecmp(type,"R"))
     { struct DLS_REGISTRE *r=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET R %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_R ( tech_id, acronyme, (gpointer *)&r );
       if (!r)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: r '%s:%s' non trouvée", __func__, tech_id, acronyme );
          g_object_unref(builder);
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Json_add_double ( builder, "valeur", r->valeur );
       Json_add_string ( builder, "unite", r->unite );
       Json_add_bool   ( builder, "archivage", r->archivage );
       Json_add_int    ( builder, "last_arch", r->last_arch );
     }
/*------------------------------------------------------- sinon --------------------------------------------------------------*/
    else { Json_add_bool ( builder, "found", FALSE ); }

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
 static gint Http_Memory_set ( struct lws *wsi, JsonObject *request, gchar *type, gchar *tech_id, gchar *acronyme )
  {

/************************************************ Préparation du buffer JSON **************************************************/
    if (!strcasecmp(type,"DI"))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET DI %s:%s", __func__, tech_id, acronyme );
       Envoyer_commande_dls_data ( tech_id, acronyme );
       return(Http_Send_response_code ( wsi, HTTP_200_OK ));
     }
/************************************************ Préparation du buffer JSON **************************************************/
    else if (!strcasecmp(type,"CI"))
     { struct DLS_CI *cpt_imp=NULL;
       gchar *valeur = json_object_get_string_member ( request, "valeur" );
       if (!valeur)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: valeur non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       gchar *unite = json_object_get_string_member ( request, "unite" );
       if (!unite)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: unite non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       gchar *multi = json_object_get_string_member ( request, "multi" );
       if (!multi)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: multi non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
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
       gchar *valeur = json_object_get_string_member ( request, "valeur" );
       if (!valeur)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: valeur non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                           /* Bad Request */
        }
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET CH %s:%s = %s", __func__, tech_id, acronyme, valeur );
       Dls_data_get_CH ( tech_id, acronyme, (gpointer *)&cpt_h );
       if (cpt_h)
        { cpt_h->valeur = atoi(valeur); }
       return(Http_Send_response_code ( wsi, HTTP_200_OK ));
     }
/************************************************ Préparation du buffer JSON **************************************************/
    else if (!strcasecmp(type,"AO"))
     { struct DLS_AO *ao=NULL;
       gchar *valeur = json_object_get_string_member ( request, "valeur" );
       if (!valeur)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: valeur non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
        }
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET AO '%s:%s' = %s", __func__, tech_id, acronyme, valeur );
       Dls_data_get_AO ( tech_id, acronyme, (gpointer *)&ao );
       if (ao)
        { ao->val_avant_ech = atof(valeur); }
       return(Http_Send_response_code ( wsi, HTTP_200_OK ));
     }
/************************************************ Préparation du buffer JSON **************************************************/
    else if (!strcasecmp(type,"R"))
     { struct DLS_REGISTRE *r=NULL;
       gboolean archivage = json_object_get_boolean_member ( request, "archivage" );
       gchar *unite = json_object_get_string_member ( request, "unite" );
       if (!unite)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: unite non trouvée", __func__ );
          return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
        }
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET R '%s:%s' -> archiavge=%d unite='%s'", __func__, tech_id, acronyme, archivage, unite );
       Dls_data_get_R ( tech_id, acronyme, (gpointer *)&r );
       if (r)
        { r->archivage = archivage;
          g_snprintf( r->unite, sizeof(r->unite), "%s", unite );
        }
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
    JsonObject *request;
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

    request = json_node_get_object (Query);
    if (!request)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Object non trouvé", __func__ );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    tech_id = json_object_get_string_member ( request, "tech_id" );
    if (!tech_id)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: tech_id non trouvé", __func__ );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    mode = json_object_get_string_member ( request, "mode" );
    if (!mode)
     { retour = Http_Memory_get_all ( wsi, tech_id );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(retour);
     }

    type = json_object_get_string_member ( request, "type" );
    if (!type)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: type non trouvé", __func__ );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }

    acronyme = json_object_get_string_member ( request, "acronyme" );
    if (!acronyme)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: acronyme non trouvé", __func__ );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                              /* Bad Request */
     }
    else if (!strcasecmp(mode, "get"))
     { retour = Http_Memory_get ( wsi, request, type, tech_id, acronyme );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(retour);
     }
    else if (!strcasecmp(mode, "set"))
     { retour = Http_Memory_set ( wsi, request, type, tech_id, acronyme );
       json_node_unref (Query);
       g_free(pss->post_data);
       return(retour);
     }

    json_node_unref (Query);
    g_free(pss->post_data);
    return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                                 /* Bad Request */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
