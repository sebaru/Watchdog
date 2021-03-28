/******************************************************************************************************************************/
/* Watchdogd/Http/getsyn.c       Gestion des requests sur l'URI /syn du webservice                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                 06.05.2020 10:57:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getsyn.c
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
#ifdef bouh
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_Acquitter_synoptique_reel ( gpointer user_data, struct DLS_SYN *dls_syn )
  { gint syn_id = GPOINTER_TO_INT(user_data);
    if (dls_syn->syn_vars.syn_id == syn_id)
     { GSList *liste = dls_syn->Dls_plugins;
       while (liste)
        { struct DLS_PLUGIN *plugin = liste->data;
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Synoptique %d -> plugin '%s' (%s) acquitté", __func__,
                    plugin->syn_id, plugin->tech_id, plugin->shortname );
          plugin->vars.bit_acquit = TRUE;
          liste = g_slist_next( liste );
        }
     }
  }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Acquitter_synoptique ( gint id )
  { Dls_foreach_syns ( GINT_TO_POINTER(id), Http_Acquitter_synoptique_reel ); }
#endif
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_clic ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                              SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       json_node_unref(request);
       return;
     }

    gchar *tech_id  = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *acronyme = Normaliser_chaine ( Json_get_string( request, "acronyme" ) );
    json_node_unref( request );
    Envoyer_commande_dls_data ( tech_id, acronyme );
    Audit_log ( session, "Clic Synoptique : %s:%s", tech_id, acronyme );
    g_free(tech_id);
    g_free(acronyme);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_list ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
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
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, "synoptiques",
                                 "SELECT syn.*, psyn.page as ppage, psyn.libelle AS plibelle, psyn.id AS pid FROM syns AS syn"
                                 " INNER JOIN syns as psyn ON psyn.id=syn.parent_id"
                                 " WHERE syn.access_level<='%d' ORDER BY syn.page", session->access_level ) == FALSE)
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
 void Http_traiter_syn_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar chaine[256];

    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;


    if ( ! (Json_has_member ( request, "syn_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint syn_id = Json_get_int ( request, "syn_id" );
    json_node_unref(request);

    if (syn_id==1)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Syn 1 can not be deleted");
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "DELETE from syns WHERE id=%d AND access_level<='%d'",
               syn_id, (session ? session->access_level : 10));
    if (SQL_Write (chaine)==FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Delete Error");
       return;
     }

/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    Audit_log ( session, "Synoptique '%d' deleted", syn_id );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar requete[256];

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "libelle" ) && Json_has_member ( request, "page" ) &&
            Json_has_member ( request, "parent_id" ) && Json_has_member ( request, "access_level" ) &&
            Json_has_member ( request, "image" )
           ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint  access_level = Json_get_int ( request, "access_level" );
    if (access_level>session->access_level)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    gint  parent_id    = Json_get_int ( request, "parent_id" );
    gchar *libelle     = Normaliser_chaine ( Json_get_string( request, "libelle" ) );
    gchar *page        = Normaliser_chaine ( Json_get_string( request, "page" ) );
    gchar *image       = Normaliser_chaine ( Json_get_string( request, "image" ) );

    if ( Json_has_member ( request, "syn_id" ) )                                                                   /* Edition */
     { gint syn_id = Json_get_int(request,"syn_id");
       if (syn_id==1) parent_id = 1;                                                 /* On ne peut changer le parent du syn 1 */
       g_snprintf( requete, sizeof(requete),
                  "UPDATE syns SET libelle='%s', page='%s', parent_id=%d, image='%s', access_level='%d' WHERE id='%d' AND access_level<='%d'",
                   libelle, page, parent_id, image, access_level, syn_id, session->access_level );
     }
    else
     { g_snprintf( requete, sizeof(requete),
                  "INSERT INTO syns SET libelle='%s', parent_id=%d, page='%s', image='%s', "
                  "access_level='%d'", libelle, parent_id, page, image, access_level );
     }

    if (SQL_Write (requete))
     { soup_message_set_status (msg, SOUP_STATUS_OK);
       Partage->com_dls.Thread_reload = TRUE;                                                                  /* Relance DLS */
     }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );

    if ( Json_has_member ( request, "syn_id" ) )                                                                   /* Edition */
     { Audit_log ( session, "Synoptique %s - '%s' changed", page, libelle ); }
    else
     { Audit_log ( session, "Synoptique %s - '%s' created", page, libelle ); }
    g_free(libelle);
    g_free(page);
    g_free(image);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  {if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "syn_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint syn_id = Json_get_int ( request, "syn_id" );
    json_node_unref(request);

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, NULL,
                                 "SELECT s.*, ps.page AS ppage FROM syns AS s INNER JOIN syns AS ps ON s.parent_id = ps.id "
                                 "WHERE s.id=%d AND s.access_level<=%d ORDER BY s.page", syn_id, session->access_level ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref ( RootNode );
       return;
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
    Audit_log ( session, "Synoptique '%d' get", syn_id );
  }
/******************************************************************************************************************************/
/* Http_get_syn_save_un_motif: Enregistre un motif en base de données                                                         */
/* Entrée: les données JSON recu de la requete HTTP                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_syn_save_un_motif (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct HTTP_CLIENT_SESSION *session = user_data;
    if ( ! (Json_has_member ( element, "id" ) &&
            Json_has_member ( element, "posx" ) &&
            Json_has_member ( element, "posy" ) &&
            Json_has_member ( element, "def_color" ) &&
            Json_has_member ( element, "tech_id" ) &&
            Json_has_member ( element, "acronyme" ) &&
            Json_has_member ( element, "clic_tech_id" ) &&
            Json_has_member ( element, "clic_acronyme" ) &&
            Json_has_member ( element, "angle" ) &&
            Json_has_member ( element, "gestion" ) &&
            Json_has_member ( element, "libelle" ) &&
            Json_has_member ( element, "scale" )
           ) )
     { return; }

    gchar *libelle = Normaliser_chaine( Json_get_string ( element, "libelle" ) );
    gchar *tech_id  = Normaliser_chaine( Json_get_string ( element, "tech_id" ) );
    gchar *acronyme = Normaliser_chaine( Json_get_string ( element, "acronyme" ) );
    gchar *clic_tech_id  = Normaliser_chaine( Json_get_string ( element, "clic_tech_id" ) );
    gchar *clic_acronyme = Normaliser_chaine( Json_get_string ( element, "clic_acronyme" ) );
    gchar *def_color = Normaliser_chaine( Json_get_string ( element, "def_color" ) );

    SQL_Write_new( "UPDATE syns_motifs AS m INNER JOIN syns AS s ON m.syn_id = s.id SET m.libelle='%s', "
                   "m.tech_id='%s', m.acronyme='%s', "
                   "m.clic_tech_id='%s', m.clic_acronyme='%s', "
                   "m.def_color='%s', m.angle='%s', m.scale='%s', m.gestion='%d' "
                   " WHERE id='%d' AND s.access_level<'%d'",
                   libelle, tech_id, acronyme, clic_tech_id, clic_acronyme, def_color,
                   Json_get_string( element, "angle" ), Json_get_string(element,"scale"), Json_get_int(element,"gestion"),
                   Json_get_int(element,"id"), session->access_level );

    g_free(libelle);
    g_free(tech_id);
    g_free(acronyme);
    g_free(clic_tech_id);
    g_free(clic_acronyme);
    g_free(def_color);
 }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_update_motifs ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                       SoupClientContext *client, gpointer user_data )
  { if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( Json_has_member ( request, "motifs" ) )
     { json_array_foreach_element ( Json_get_array ( request, "motifs" ), Http_syn_save_un_motif, session ); }

    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_show ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                              SoupClientContext *client, gpointer user_data )
  { gint syn_id;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;

    gchar *syn_id_src = g_hash_table_lookup ( query, "syn_id" );
    if (syn_id_src) { syn_id = atoi (syn_id_src); }
                 else syn_id=1;

    gchar *full_syn = g_hash_table_lookup ( query, "full" );

/*-------------------------------------------------- Test autorisation d'accès -----------------------------------------------*/
    JsonNode *result = Json_node_create();
    if (!result)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Select_to_json_node ( result, NULL, "SELECT access_level,libelle FROM syns WHERE id=%d", syn_id );
    if ( !(Json_has_member ( result, "access_level" ) && Json_has_member ( result, "libelle" )) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: Syn '%d' unknown", __func__, syn_id );
       soup_message_set_status_full (msg, SOUP_STATUS_NOT_FOUND, "Syn not found");
       json_node_unref ( result );
       return;
     }
    if (session->access_level < Json_get_int ( result, "access_level" ))
     { Audit_log ( session, "Access to synoptique '%s' (id '%d') forbidden",
                   Json_get_string ( result, "libelle" ), syn_id );
       soup_message_set_status_full (msg, SOUP_STATUS_FORBIDDEN, "Access Denied");
       json_node_unref ( result );
       return;
     }
    json_node_unref ( result );
/*---------------------------------------------- Envoi les données -----------------------------------------------------------*/
    JsonNode *synoptique = Json_node_create();
    if (!synoptique)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }

    if (SQL_Select_to_json_node ( synoptique, NULL,
                                 "SELECT * FROM syns WHERE id='%d' AND access_level<='%d'",
                                  syn_id, session->access_level) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(synoptique);
       return;
     }
/*-------------------------------------------------- Envoi les data des synoptiques fils -------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "child_syns",
                                 "SELECT s.* FROM syns AS s INNER JOIN syns as s2 ON s.parent_id=s2.id "
                                 "WHERE s2.id='%d' AND s.id!=1 AND s.access_level<='%d'",
                                 syn_id, session->access_level) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(synoptique);
       return;
     }
/*-------------------------------------------------- Envoi les syn_vars ------------------------------------------------------*/
    JsonArray *syn_vars = Json_node_add_array ( synoptique, "syn_vars" );
    Dls_foreach_syns ( syn_vars, Dls_syn_vars_to_json );

/*-------------------------------------------------- Envoi les passerelles ---------------------------------------------------*/
    if (full_syn)
     { if (SQL_Select_to_json_node ( synoptique, "passerelles",
                                    "SELECT pass.*,syn.page,syn.libelle FROM syns_pass as pass "
                                    "INNER JOIN syns as syn ON pass.syn_cible_id=syn.id "
                                    "WHERE pass.syn_id=%d AND syn.access_level<=%d",
                                     syn_id, session->access_level ) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref(synoptique);
          return;
        }
     }
/*-------------------------------------------------- Envoi les liens ---------------------------------------------------------*/
    if (full_syn)
     { if (SQL_Select_to_json_node ( synoptique, "liens",
                                     "SELECT lien.* FROM syns_liens AS lien "
                                     "INNER JOIN syns as syn ON lien.syn_id=syn.id "
                                     "WHERE lien.syn_id=%d AND syn.access_level<=%d",
                                     syn_id, session->access_level ) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref(synoptique);
          return;
        }
     }
/*-------------------------------------------------- Envoi les rectangles ----------------------------------------------------*/
    if (full_syn)
     { if (SQL_Select_to_json_node ( synoptique, "rectangles",
                                    "SELECT rectangle.* FROM syns_rectangles AS rectangle "
                                    "INNER JOIN syns as syn ON rectangle.syn_id=syn.id "
                                    "WHERE rectangle.syn_id=%d AND syn.access_level<=%d",
                                    syn_id, session->access_level ) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref(synoptique);
          return;
        }
     }
/*-------------------------------------------------- Envoi les commennts -----------------------------------------------------*/
    if (full_syn)
     { if (SQL_Select_to_json_node ( synoptique, "comments",
                                    "SELECT comment.* FROM syns_comments AS comment "
                                    "INNER JOIN syns as syn ON comment.syn_id=syn.id "
                                    "WHERE comment.syn_id=%d AND syn.access_level<=%d",
                                    syn_id, session->access_level ) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref(synoptique);
          return;
        }
     }
/*-------------------------------------------------- Envoi les cameras -------------------------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "cameras",
                                 "SELECT cam.*,src.location,src.libelle FROM syns_camerasup AS cam "
                                 "INNER JOIN cameras AS src ON cam.camera_src_id=src.id "
                                 "INNER JOIN syns as syn ON cam.syn_id=syn.id "
                                 "WHERE cam.syn_id=%d AND syn.access_level<=%d",
                                 syn_id, session->access_level ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(synoptique);
       return;
     }

/*-------------------------------------------------- Envoi les cadrans de la page --------------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "cadrans",
                                 "SELECT cadran.* FROM syns_cadrans AS cadran "
                                 "INNER JOIN syns as syn ON cadran.syn_id=syn.id "
                                 "WHERE cadran.syn_id=%d AND syn.access_level<=%d",
                                 syn_id, session->access_level ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(synoptique);
       return;
     }

/*-------------------------------------------------- Envoi les tableaux de la page -------------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "tableaux",
                                 "SELECT tableau.* FROM tableau "
                                 "INNER JOIN syns as syn ON tableau.syn_id=syn.id "
                                 "WHERE tableau.syn_id=%d AND syn.access_level<=%d",
                                 syn_id, session->access_level ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(synoptique);
       return;
     }
/*-------------------------------------------------- Envoi les tableaux_map de la page ---------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "tableaux_map",
                                 "SELECT tableau_map.* FROM tableau_map "
                                 "INNER JOIN tableau ON tableau_map.tableau_id=tableau.id "
                                 "INNER JOIN syns as syn ON tableau.syn_id=syn.id "
                                 "WHERE tableau.syn_id=%d AND syn.access_level<=%d",
                                 syn_id, session->access_level ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(synoptique);
       return;
     }

/*-------------------------------------------------- Envoi les visuels de la page --------------------------------------------*/
    if (full_syn)
     { if (SQL_Select_to_json_node ( synoptique, "visuels",
                                    "SELECT visu.* FROM syns_motifs AS visu "
                                    "INNER JOIN syns AS syn ON visu.syn_id=syn.id "
                                    "WHERE syn.id='%d' AND syn.access_level<=%d ORDER BY layer",
                                     syn_id, session->access_level) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref(synoptique);
          return;
        }
     }
    else
     { if (SQL_Select_to_json_node ( synoptique, "visuels",
                                    "SELECT visu.*,i.*,dls.shortname AS dls_shortname FROM syns_motifs AS visu "
                                    "INNER JOIN dls on dls.tech_id=visu.tech_id "
                                    "INNER JOIN icone AS i ON i.forme=visu.forme "
                                    "INNER JOIN syns AS s ON dls.syn_id=s.id "
                                    "WHERE visu.auto_create=1 AND s.id='%d' AND s.access_level<=%d",
                                    syn_id, session->access_level) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref(synoptique);
          return;
        }
     }
/*------------------------------------------------- Envoi l'état de tous les visuels du synoptique ---------------------------*/
    JsonArray *etat_visuels = Json_node_add_array ( synoptique, "etat_visuels" );
    GList *syn_visuels      = Json_get_array_as_list ( synoptique, "visuels" );
    GSList *dls_visuels     = Partage->Dls_data_VISUEL;
    while(dls_visuels)                      /* Parcours tous les visuels et envoie ceux relatifs aux DLS du synoptique chargé */
     { struct DLS_VISUEL *dls_visuel=dls_visuels->data;
       GList *visuels = syn_visuels;
       while (visuels)
        { JsonNode *visuel = visuels->data;
          if (!strcasecmp( Json_get_string(visuel, "tech_id"), dls_visuel->tech_id))
           { JsonNode *element = Json_node_create ();
             Dls_VISUEL_to_json ( element, dls_visuel );
             Json_array_add_element ( etat_visuels, element );
           }
          visuels = g_list_next(visuels);
        }
       dls_visuels = g_slist_next(dls_visuels);
     }
    g_list_free(syn_visuels);

    gchar *buf = Json_node_to_string ( synoptique );
    json_node_unref(synoptique);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Http_traiter_log: Répond aux requetes sur l'URI log                                                                        */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_syn_ack ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data)
  { if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    gint syn_id  = Json_get_int( request, "syn_id" );

    Dls_acquitter_synoptique(syn_id);
    soup_message_set_status (msg, SOUP_STATUS_OK);
    json_node_unref(request);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
