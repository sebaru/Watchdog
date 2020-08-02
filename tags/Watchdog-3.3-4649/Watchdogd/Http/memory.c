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
/* Http_Memory_get : Renvoi, au format json, la valeur d'un bit interne                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 void Http_Memory_get_all ( SoupMessage *msg, gchar *tech_id )
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
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    Json_add_string ( builder, "tech_id", tech_id );
    Json_add_int    ( builder, "top", Partage->top );
/*------------------------------------------------ Compteur d'impulsions -----------------------------------------------------*/
    Json_add_array ( builder, "CI" );
    liste = Partage->Dls_data_CI;
    while(liste)
     { struct DLS_CI *bit=liste->data;
       if (!strcasecmp(bit->tech_id, tech_id))
        { Json_add_object ( builder, bit->acronyme );
          Dls_CI_to_json ( builder, bit );
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
          Dls_BOOL_to_json ( builder, bit );
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
          Dls_CH_to_json ( builder, bit );
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
          Dls_AI_to_json ( builder, bit );
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
          Dls_AO_to_json ( builder, bit );
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
          Dls_TEMPO_to_json ( builder, bit );
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
          Dls_DI_to_json ( builder, bit );
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
          Dls_DO_to_json ( builder, bit );
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
          Dls_VISUEL_to_json ( builder, bit );
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
          Dls_MESSAGE_to_json ( builder, bit );
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
          Dls_REGISTRE_to_json ( builder, bit );
          Json_end_object( builder );
        }
       liste = g_slist_next(liste);
     }
    Json_end_array( builder );
/*------------------------------------------------------- fin ----------------------------------------------------------------*/
    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Memory_get : Renvoi, au format json, la valeur d'un bit interne                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static void Http_Memory_get ( SoupMessage *msg, JsonObject *request, gchar *type, gchar *tech_id, gchar *acronyme )
  { JsonBuilder *builder;
    gsize taille_buf;
	   gchar *buf;

/************************************************ Préparation du buffer JSON **************************************************/
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }
                                                                      /* Lancement de la requete de recuperation des messages */
/*------------------------------------------------------- Dumping status -----------------------------------------------------*/

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
   	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not found in running memory");
          return;
        }
       Dls_CI_to_json ( builder, cpt_imp );
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
   	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not found in running memory");
          return;
        }
       Dls_CH_to_json ( builder, cpt_h );
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
   	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not found in running memory");
          return;
        }
       Dls_AI_to_json ( builder, ai );
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
   	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not found in running memory");
          return;
        }
       Dls_AO_to_json ( builder, ao );
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
   	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not found in running memory");
          return;
        }
       Dls_BOOL_to_json ( builder, bool );
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
   	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not found in running memory");
          return;
        }
       Dls_TEMPO_to_json ( builder, tempo );
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
   	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not found in running memory");
          return;
        }
       Dls_DI_to_json ( builder, di );
     }
/*---------------------------------------------------- Visuels ---------------------------------------------------------------*/
    else if (!strcasecmp(type,"I"))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET VISUEL %s:%s", __func__, tech_id, acronyme );
       struct DLS_VISUEL *visu=NULL;
       Dls_data_get_VISUEL ( tech_id, acronyme, (gpointer *)&visu );
       if (!visu)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: visu '%s:%s' non trouvé", __func__, tech_id, acronyme );
          g_object_unref(builder);
          soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not found in running memory");
          return;
        }
       Dls_VISUEL_to_json ( builder, visu );
     }
/*---------------------------------------------------- Messages --------------------------------------------------------------*/
    else if (!strcasecmp(type,"MSG"))
     { struct DLS_MESSAGES *dls_msg=NULL;
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                 "%s: HTTP/ request for GET MSG %s:%s", __func__, tech_id, acronyme );
       Dls_data_get_MSG ( tech_id, acronyme, (gpointer *)&dls_msg );
       if (!dls_msg)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: msg '%s:%s' non trouvée", __func__, tech_id, acronyme );
          g_object_unref(builder);
   	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not found in running memory");
          return;
        }
       Dls_MESSAGE_to_json ( builder, dls_msg );
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
   	      soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not found in running memory");
          return;
        }
       Dls_REGISTRE_to_json ( builder, r );
     }
/*------------------------------------------------------- sinon --------------------------------------------------------------*/
    else { Json_add_bool ( builder, "found", FALSE ); }

    buf = Json_get_buf ( builder, &taille_buf );
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/******************************************************************************************************************************/
/* Http_Memory_set: Positionne le bit interne en parametre                                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static void Http_Memory_set ( SoupMessage *msg, JsonObject *request, gchar *type, gchar *tech_id, gchar *acronyme )
  {

/************************************************ Préparation du buffer JSON **************************************************/
    if (!strcasecmp(type,"DI"))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET DI %s:%s", __func__, tech_id, acronyme );
       Envoyer_commande_dls_data ( tech_id, acronyme );
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    else if (!strcasecmp(type,"CI"))
     { struct DLS_CI *cpt_imp=NULL;
       gchar *valeur = json_object_get_string_member ( request, "valeur" );
       if (!valeur)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: valeur non trouvée", __func__ );
          soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
          return;
        }
       gchar *unite = json_object_get_string_member ( request, "unite" );
       if (!unite)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: unite non trouvée", __func__ );
          soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
          return;
        }
       gchar *multi = json_object_get_string_member ( request, "multi" );
       if (!multi)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: multi non trouvée", __func__ );
          soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
          return;
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
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    else if (!strcasecmp(type,"CH"))
     { struct DLS_CH *cpt_h=NULL;
       gchar *valeur = json_object_get_string_member ( request, "valeur" );
       if (!valeur)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: valeur non trouvée", __func__ );
          soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
          return;
        }
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET CH %s:%s = %s", __func__, tech_id, acronyme, valeur );
       Dls_data_get_CH ( tech_id, acronyme, (gpointer *)&cpt_h );
       if (cpt_h)
        { cpt_h->valeur = atoi(valeur); }
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    else if (!strcasecmp(type,"AO"))
     { struct DLS_AO *ao=NULL;
       gchar *valeur = json_object_get_string_member ( request, "valeur" );
       if (!valeur)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: valeur non trouvée", __func__ );
          soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
          return;
        }
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET AO '%s:%s' = %s", __func__, tech_id, acronyme, valeur );
       Dls_data_get_AO ( tech_id, acronyme, (gpointer *)&ao );
       if (ao)
        { ao->val_avant_ech = atof(valeur); }
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
    else if (!strcasecmp(type,"R"))
     { struct DLS_REGISTRE *r=NULL;
       gboolean archivage = json_object_get_boolean_member ( request, "archivage" );
       gchar *unite = json_object_get_string_member ( request, "unite" );
       if (!unite)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: unite non trouvée", __func__ );
          soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
          return;
        }
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                 "%s: HTTP/ request for SET R '%s:%s' -> archiavge=%d unite='%s'", __func__, tech_id, acronyme, archivage, unite );
       Dls_data_get_R ( tech_id, acronyme, (gpointer *)&r );
       if (r)
        { r->archivage = archivage;
          g_snprintf( r->unite, sizeof(r->unite), "%s", unite );
        }
       soup_message_set_status (msg, SOUP_STATUS_OK);
       return;
     }
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_memory: le payload est arrivé, il faut traiter le json                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 void Http_traiter_memory ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  { gchar *mode, *type, *tech_id, *acronyme;
    JsonObject *object;
    GBytes *request;
    JsonNode *Query;
    gchar * data;
    gsize taille;

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    /*struct HTTP_CLIENT_SESSION *session =*/ Http_print_request ( server, msg, path, client );

    g_object_get ( msg, "request-body-data", &request, NULL );
    if (!request)
     { soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }

    data = g_bytes_unref_to_data ( request, &taille );                     /* Récupération du buffer et ajout d'un \0 d'arret */
    data = g_try_realloc( data, taille + 1 );
    data [taille] = 0;
    Query = json_from_string ( data, NULL );
    if (!Query)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: requete non Json '%s'", __func__, data );
       g_free(data);
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }
    g_free(data);

    object = json_node_get_object (Query);
    if (!request)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: Object non trouvé", __func__ );
       json_node_unref (Query);
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }

    tech_id = json_object_get_string_member ( object, "tech_id" );
    if (!tech_id)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: tech_id non trouvé", __func__ );
       json_node_unref (Query);
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }

    mode = json_object_get_string_member ( object, "mode" );
    if (!mode)
     { Http_Memory_get_all ( msg, tech_id );
       json_node_unref (Query);
       return;
     }

    type = json_object_get_string_member ( object, "type" );
    if (!type)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: type non trouvé", __func__ );
       json_node_unref (Query);
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }

    acronyme = json_object_get_string_member ( object, "acronyme" );
    if (!acronyme)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: acronyme non trouvé", __func__ );
       json_node_unref (Query);
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }
    else if (!strcasecmp(mode, "get"))
     { Http_Memory_get ( msg, object, type, tech_id, acronyme );
       json_node_unref (Query);
       return;
     }
    else if (!strcasecmp(mode, "set"))
     { Http_Memory_set ( msg, object, type, tech_id, acronyme );
       json_node_unref (Query);
       return;
     }

    json_node_unref (Query);
    soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
