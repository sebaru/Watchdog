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

    gchar *prefix = "/syn/clic/";
    if ( ! g_str_has_prefix ( path, prefix ) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Prefix");
       return;
     }

    if (!strlen (path+strlen(prefix)))
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       return;
     }

    gchar *temp = g_utf8_strup( path+strlen(prefix), -1 );
    gchar **params = g_strsplit ( temp, "/", 2 );
    g_free(temp);
    if( ! (params && params[0] && params[1]) )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Bad Argument");
       g_strfreev( params );
       return;
     }

    gchar *tech_id  = Normaliser_chaine ( params[0] );
    gchar *acronyme = Normaliser_chaine ( params[1] );
    g_strfreev( params );
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

    if ( strcmp ( path, "/syn/list" ) )
     { soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
       return;
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT syn.*,psyn.page as ppage FROM syns AS syn"
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
 void Http_traiter_syn_show ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                              SoupClientContext *client, gpointer user_data )
  { gchar *buf, chaine[256];
    gsize taille_buf;
    GBytes *request_brute;
    gsize taille;

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

    if ( ! (Json_has_member ( request, "syn_id" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint syn_id = Json_get_int ( request, "syn_id" );
    json_node_unref(request);

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns WHERE id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, NULL, chaine ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_motifs WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "motifs", chaine ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT sp.*,syn.page,syn.libelle FROM syns_pass as sp "
                                       "INNER JOIN syns as syn ON sp.syn_cible_id=syn.id WHERE sp.syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "passerelles", chaine ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_liens WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "liens", chaine ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_rectangles WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "rectangles", chaine ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_comments WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "comments", chaine ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT *,src.location,src.libelle from syns_camerasup AS cam "
                                       "INNER JOIN cameras AS src ON cam.camera_src_id=src.id WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "cameras", chaine ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT syns_cadrans.* FROM syns_cadrans WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "cadrans", chaine ) == FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error");
       g_object_unref(builder);
       return;
     }

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
    Audit_log ( session, "Synoptique '%d' showed", id );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_del ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { gchar *buf, chaine[256];
    GBytes *request_brute;
    gsize taille_buf;
    gsize taille;

    if (msg->method != SOUP_METHOD_DELETE)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

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

    g_snprintf(chaine, sizeof(chaine), "DELETE from syns WHERE id=%d AND access_level<'%d'",
               syn_id, (session ? session->access_level : 10));
    if (SQL_Write (chaine)==FALSE)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Delete Error");
       return;
     }

    JsonBuilder *builder = Json_create ();
    if (!builder)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    Json_add_int ( builder, "id", syn_id );
    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
    Audit_log ( session, "Synoptique '%d' deleted", id );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_set ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                             SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gchar requete[256];
    gsize taille;

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( ! (Json_has_member ( request, "libelle" ) && Json_has_member ( request, "page" ) &&
            Json_has_member ( request, "ppage" ) && Json_has_member ( request, "access_level" ) ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gchar *libelle     = Normaliser_chaine ( Json_get_string( request, "libelle" ) );
    gchar *page        = Normaliser_chaine ( Json_get_string( request, "page" ) );
    gchar *ppage       = Normaliser_chaine ( Json_get_string( request, "ppage" ) );
    gint  access_level = Json_get_int ( request, "access_level" );
    if (access_level>=session->access_level) access_level = session->access_level-1;

    if ( Json_has_member ( request, "id" ) )                                                                       /* Edition */
     { g_snprintf( requete, sizeof(requete),
                  "UPDATE syns SET libelle='%s', page='%s', access_level='%d' WHERE id='%d' AND access_level<'%d'",
                   libelle, page, access_level, Json_get_int(request,"id"), session->access_level );
     }
    else
     {
       g_snprintf( requete, sizeof(requete),
                  "INSERT INTO syns SET libelle='%s', page='%s', parent_id=(SELECT psyn.id FROM syns AS psyn WHERE psyn.page='%s'), "
                  "access_level='%d'",
                   libelle, page, ppage, access_level );
     }
    if (SQL_Write (requete)) { soup_message_set_status (msg, SOUP_STATUS_OK); }
    else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "SQL Error" );

    g_free(libelle);
    g_free(page);
    g_free(ppage);
    Audit_log ( session, "Synoptique '%d' - %s '%s'changed", id, page, libelle );
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
    GBytes *request_brute;
    gsize taille;

    if (msg->method != SOUP_METHOD_PUT)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( !request)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "No request");
       return;
     }

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
                                       "WHERE s.id=%d AND s.access_level<%d", syn_id, (session ? session->access_level : 10) );
    if (SQL_Select_to_JSON ( builder, NULL, chaine ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       g_object_unref(builder);
       return;
     }

    buf = Json_get_buf (builder, &taille_buf);
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
    Audit_log ( session, "Synoptique '%d' sent", id );
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
  { GBytes *request_brute;
    gsize taille;

    if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );

    if ( Json_has_member ( request, "motifs" ) )
     { json_array_foreach_element ( Json_get_array ( request, "motifs" ), Http_syn_save_un_motif, session ); }

    json_node_unref(request);
  }

/*----------------------------------------------------------------------------------------------------------------------------*/
