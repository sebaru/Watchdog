/******************************************************************************************************************************/
/* Watchdogd/Http/getmnemos.c       Gestion des requests sur l'URI /syn du webservice                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    23.07.2020 17:30:10 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getmnemos.c
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

/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_mnemos_tech_id ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                    SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "tech_ids", "SELECT DISTINCT tech_id FROM dictionnaire" )==FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref ( RootNode );
       return;
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_mnemos_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data )
  { gchar chaine[256];
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gchar *tech_id = g_hash_table_lookup ( query, "tech_id" );
    if (!tech_id)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres tech_id");
       return;
     }
    Normaliser_as_ascii ( tech_id );

    gchar *classe = g_hash_table_lookup ( query, "classe" );
    if (!classe)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres classe");
       return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (!strcasecmp ( classe, "DI" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_DI AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "DO" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_DO AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "AI" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_AI AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "AO" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_AO AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "R" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_R AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "CI" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_CI AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "CH" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_CH AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "HORLOGE" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_HORLOGE AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "TEMPO" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_Tempo AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "MONO" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_MONO AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "BI" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_BI AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "WATCHDOG" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from mnemos_WATCHDOG AS m WHERE m.tech_id='%s'", tech_id ); }
    else if (!strcasecmp ( classe, "MSG" ) )
     { g_snprintf(chaine, sizeof(chaine), "SELECT m.* from msgs AS m WHERE m.tech_id='%s'", tech_id ); }

    if (SQL_Select_to_json_node ( RootNode, classe, chaine )==FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref ( RootNode );
       return;
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_Traiter_mnemos_set: Modifie la config d'un mnemonique                                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : HTTP Response code                                                                                                */
/******************************************************************************************************************************/
 void Http_traiter_mnemos_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
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

    if ( ! (request && Json_has_member ( request, "classe" ) && Json_has_member ( request, "tech_id" ) &&
                       Json_has_member ( request, "acronyme" ) ) )
     { if (request) json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar   *tech_id  = Normaliser_as_ascii ( Json_get_string ( request,"tech_id" ) );
    gchar   *acronyme = Normaliser_as_ascii ( Json_get_string ( request,"acronyme" ) );
    gchar   *classe   = Normaliser_as_ascii ( Json_get_string ( request,"classe" ) );
    soup_message_set_status (msg, SOUP_STATUS_OK);

    if ( ! strcasecmp ( classe, "CI" ) )
     { struct DLS_CI *ci=NULL;
       Dls_data_get_CI ( tech_id, acronyme, (gpointer)&ci );
       if ( Json_has_member ( request, "archivage" ) )
        { gchar chaine[128];
          gint archivage = Json_get_int ( request, "archivage" );
          if (ci) { ci->archivage = archivage; }                            /* Si le bit existe, on change sa running config */
          g_snprintf( chaine, sizeof(chaine), "UPDATE mnemos_CI SET archivage='%d' WHERE tech_id='%s' AND acronyme='%s'",
                      archivage, tech_id, acronyme );
          SQL_Write ( chaine );                                                   /* Qu'il existe ou non, ou met a jour la DB */
          Audit_log ( session, "Mnemos %s:%s -> archivage = '%d'", tech_id, acronyme, archivage );
        }
	      else soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais arguments" );
     }
    else if ( ! strcasecmp ( classe, "R" ) )
     { struct DLS_REGISTRE *reg=NULL;
       Dls_data_get_REGISTRE ( tech_id, acronyme, (gpointer)&reg );
       if ( Json_has_member ( request, "archivage" ) )
        { gchar chaine[128];
          gint archivage = Json_get_int ( request, "archivage" );
          if (reg) { reg->archivage = archivage; }                           /* Si le bit existe, on change sa running config */
          g_snprintf( chaine, sizeof(chaine), "UPDATE mnemos_R SET archivage='%d' WHERE tech_id='%s' AND acronyme='%s'",
                      archivage, tech_id, acronyme );
          SQL_Write ( chaine );                                                   /* Qu'il existe ou non, ou met a jour la DB */
          Audit_log ( session, "Mnemos %s:%s -> archivage = '%d'", tech_id, acronyme, archivage );
        }
	      else soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais arguments" );
     }
    else if ( ! strcasecmp ( classe, "AI" ) )
     { struct DLS_AI *ai=NULL;
       Dls_data_get_AI ( tech_id, acronyme, (gpointer)&ai );
       if ( Json_has_member ( request, "archivage" ) )
        { gchar chaine[128];
          gint archivage = Json_get_int ( request, "archivage" );
          if (ai) { ai->archivage = archivage; }                             /* Si le bit existe, on change sa running config */
          g_snprintf( chaine, sizeof(chaine), "UPDATE mnemos_AI SET archivage='%d' WHERE tech_id='%s' AND acronyme='%s'",
                      archivage, tech_id, acronyme );
          SQL_Write ( chaine );                                                   /* Qu'il existe ou non, ou met a jour la DB */
          Audit_log ( session, "Mnemos %s:%s -> archivage = '%d'", tech_id, acronyme, archivage );
        }
	      else soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais arguments" );
     }
    else if ( ! strcasecmp ( classe, "MSG" ) )
     { if ( Json_has_member ( request, "rate_limit" ) && Json_has_member ( request, "sms" ) &&
            Json_has_member ( request, "audio_profil" ) && Json_has_member ( request, "audio_libelle" )
          )
        { gchar chaine[1024];
          gint rate_limit      = Json_get_int ( request, "rate_limit" );
          gint sms             = Json_get_int ( request, "sms" );
          gchar *audio_libelle = Normaliser_chaine ( Json_get_string ( request, "audio_libelle" ) );
          gchar *audio_profil  = Normaliser_chaine ( Json_get_string ( request, "audio_profil" ) );
          g_snprintf( chaine, sizeof(chaine), "UPDATE msgs SET rate_limit='%d', sms_notification='%d', audio_profil='%s', audio_libelle='%s' "
                                              "WHERE tech_id='%s' AND acronyme='%s'",
                      rate_limit, sms, audio_profil, audio_libelle, tech_id, acronyme );
          SQL_Write ( chaine );                                                   /* Qu'il existe ou non, ou met a jour la DB */
          Audit_log ( session, "Mnemos %s:%s -> Sms_notification = '%d'", tech_id, acronyme, sms );
          Audit_log ( session, "Mnemos %s:%s -> Profil_AUDIO = '%s'", tech_id, acronyme, audio_profil );
          Audit_log ( session, "Mnemos %s:%s -> Libelle_AUDIO = '%s'", tech_id, acronyme, audio_libelle );
          g_free(audio_profil);
          g_free(audio_libelle);
        }
	      else soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais arguments" );
     }
    else if ( ! strcasecmp ( classe, "HORLOGE" ) )
     { if ( Json_has_member ( request, "access_level" ) )
        { gchar chaine[1024];
          gint access_level = Json_get_int ( request, "access_level" );
          g_snprintf( chaine, sizeof(chaine), "UPDATE mnemos_HORLOGE SET access_level=%d WHERE tech_id='%s' AND acronyme='%s'",
                                              access_level, tech_id, acronyme );
          SQL_Write ( chaine );                                                   /* Qu'il existe ou non, ou met a jour la DB */
          Audit_log ( session, "Mnemos %s:%s -> access_level set to %d", tech_id, acronyme, access_level );
        }
	      else soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais arguments" );
     }
/*************************************************** Envoi au client **********************************************************/
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_traiter_mnemos_validate: Valide la presence ou non d'un tech_id/acronyme dans le dico                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_mnemos_validate ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                     SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
            Json_has_member ( request, "classe" )
           )
       )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *tech_id  = Normaliser_as_ascii ( Json_get_string ( request, "tech_id" ) );
    gchar *acronyme = Normaliser_as_ascii ( Json_get_string ( request, "acronyme" ) );
    gchar *classe   = Normaliser_as_ascii ( Json_get_string ( request, "classe" ) );

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       json_node_unref(request);
       return;
     }

    SQL_Select_to_json_node ( RootNode, "tech_ids_found",
                              "SELECT tech_id, name FROM dls WHERE tech_id LIKE '%%%s%%' ORDER BY tech_id", tech_id);

    SQL_Select_to_json_node ( RootNode, "acronymes_found",
                              "SELECT acronyme,libelle FROM dictionnaire WHERE tech_id='%s' AND acronyme LIKE '%%%s%%' "
                              "AND classe='%s' ORDER BY acronyme",
                              tech_id, acronyme, (classe ? classe : "")
                            );
    json_node_unref(request);

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
