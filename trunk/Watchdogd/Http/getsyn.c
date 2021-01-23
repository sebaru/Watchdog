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
  { gchar *buf, chaine[256];
    gsize taille_buf;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT syn.*, psyn.page as ppage, psyn.libelle AS plibelle, psyn.id AS pid FROM syns AS syn"
                                       " INNER JOIN syns as psyn ON psyn.id=syn.parent_id"
                                       " WHERE syn.access_level<='%d' ORDER BY syn.page", session->access_level);
    if (SQL_Select_to_JSON ( builder, "synoptiques", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
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
  { gchar *buf, chaine[256];
    gsize taille_buf;

    if (msg->method != SOUP_METHOD_PUT)
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

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT s.*, ps.page AS ppage FROM syns AS s INNER JOIN syns AS ps ON s.parent_id = ps.id "
                                       "WHERE s.id=%d AND s.access_level<=%d ORDER BY s.page", syn_id, session->access_level );
    if (SQL_Select_to_JSON ( builder, NULL, chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
    Audit_log ( session, "Synoptique '%d' get", syn_id );
  }
/******************************************************************************************************************************/
/* Http_get_syn_save_un_motif: Enregistre un motif en base de données                                                         */
/* Entrée: les données JSON recu de la requete HTTP                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_syn_save_un_motif (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct HTTP_CLIENT_SESSION *session = user_data;
    gchar requete[256];
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

    g_snprintf( requete, sizeof(requete),
               "UPDATE syns_motifs AS m INNER JOIN syns AS s ON m.syn_id = s.id SET m.libelle='%s', "
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

    SQL_Write (requete);
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
  { gchar *buf, chaine[256], page[33];
    gsize taille_buf;
    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 0 )) return;

    gchar *page_src = g_hash_table_lookup ( query, "page" );
    if (page_src)
     { gchar *temp = Normaliser_chaine ( page_src );
       g_snprintf( page, sizeof(page), "%s", temp );
       g_free(temp);
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }

    if (page_src)
     { g_snprintf(chaine, sizeof(chaine), "SELECT * FROM syns"
                                          " WHERE page='%s' AND access_level<='%d'", page, session->access_level);
     }
    else
     { g_snprintf(chaine, sizeof(chaine), "SELECT * FROM syns"
                                          " WHERE id='1' AND access_level<='%d'", session->access_level);
     }

    if (SQL_Select_to_JSON ( builder, NULL, chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    if (page_src)
     { g_snprintf(chaine, sizeof(chaine), "SELECT s.* FROM syns AS s INNER JOIN syns as s2 ON s.parent_id=s2.id"
                                          " WHERE s2.page='%s' AND s.access_level<='%d'", page, session->access_level);
     }
    else
     { g_snprintf(chaine, sizeof(chaine), "SELECT * FROM syns WHERE parent_id=1 AND id!=1 AND access_level<='%d'", session->access_level);
     }

    if (SQL_Select_to_JSON ( builder, "child_syns", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    if (page_src)
     { g_snprintf(chaine, sizeof(chaine), "SELECT tech_id, shortname, name FROM dls INNER JOIN syns ON dls.syn_id = syns.id "
                                          "WHERE syns.page='%s' AND syns.access_level<=%d", page, session->access_level);
     }
    else
     { g_snprintf(chaine, sizeof(chaine), "SELECT tech_id, shortname, name FROM dls INNER JOIN syns ON dls.syn_id = syns.id "
                                          "WHERE syns.id='1' AND syns.access_level<=%d", session->access_level);
     }
    if (SQL_Select_to_JSON ( builder, "child_dls", chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    Json_add_array ( builder, "syn_vars" );
    Dls_foreach_syns ( builder, Dls_syn_vars_to_json );
    Json_end_array ( builder );

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
