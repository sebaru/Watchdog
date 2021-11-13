/******************************************************************************************************************************/
/* Watchdogd/Http/Http.c        Gestion des connexions HTTP WebService de watchdog                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Http.c
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

 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"
 #include "Http.h"
 struct HTTP_CONFIG Cfg_http;
/******************************************************************************************************************************/
/* Http_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                   */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Http_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_http.lib->Thread_debug = FALSE;                                                        /* Settings default parameters */
    Cfg_http.tcp_port          = HTTP_DEFAUT_TCP_PORT;
    Cfg_http.ssl_enable        = FALSE;
    Cfg_http.authenticate      = TRUE;
    Cfg_http.nbr_max_connexion = HTTP_DEFAUT_MAX_CONNEXION;
    Cfg_http.max_upload_bytes  = HTTP_DEFAUT_MAX_UPLOAD_BYTES;
    Cfg_http.lws_debug_level   = HTTP_DEFAUT_LWS_DEBUG_LEVEL;
    g_snprintf( Cfg_http.ssl_cert_filepath,        sizeof(Cfg_http.ssl_cert_filepath), "%s", HTTP_DEFAUT_FILE_CERT );
    g_snprintf( Cfg_http.ssl_private_key_filepath, sizeof(Cfg_http.ssl_private_key_filepath), "%s", HTTP_DEFAUT_FILE_KEY );
    g_snprintf( Cfg_http.ssl_ca_filepath,          sizeof(Cfg_http.ssl_ca_filepath), "%s", HTTP_DEFAUT_FILE_CA  );
    g_snprintf( Cfg_http.ssl_cipher_list,          sizeof(Cfg_http.ssl_cipher_list), "%s", HTTP_DEFAUT_SSL_CIPHER );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     {      if ( ! g_ascii_strcasecmp ( nom, "ssl_file_cert" ) )
        { g_snprintf( Cfg_http.ssl_cert_filepath, sizeof(Cfg_http.ssl_cert_filepath), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_file_ca" ) )
        { g_snprintf( Cfg_http.ssl_ca_filepath, sizeof(Cfg_http.ssl_ca_filepath), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_file_key" ) )
        { g_snprintf( Cfg_http.ssl_private_key_filepath, sizeof(Cfg_http.ssl_private_key_filepath), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_cipher" ) )
        { g_snprintf( Cfg_http.ssl_cipher_list, sizeof(Cfg_http.ssl_cipher_list), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "max_connexion" ) )
        { Cfg_http.nbr_max_connexion = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "max_upload_bytes" ) )
        { Cfg_http.max_upload_bytes = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "lws_debug_level" ) )
        { Cfg_http.lws_debug_level = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_http.ssl_enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "tcp_port" ) )
        { Cfg_http.tcp_port = atoi(valeur);  }
       else if ( ! g_ascii_strcasecmp ( nom, "authenticate" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "false" ) ) Cfg_http.authenticate = FALSE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_http.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Http_json_get_int : Récupère un entier dans l'object JSON passé en paramètre, dont le nom est name                         */
/* Entrées : l'object JSON et le name                                                                                         */
/* Sortie : un entier, ou -1 si erreur                                                                                        */
/******************************************************************************************************************************/
 gint Http_json_get_int ( JsonObject *object, gchar *name )
  { JsonNode *node;
    GValue valeur = G_VALUE_INIT;

    if (!object) return(-1);
    node = json_object_get_member(object, name );
    if(!node) return(-1);
    if(json_node_get_node_type (node) != JSON_NODE_VALUE) return(-1);
    json_node_get_value (node, &valeur);
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: Parsing value type %d ('%s') for attribut '%s'", __func__,
              G_VALUE_TYPE(&valeur), G_VALUE_TYPE_NAME(&valeur), name );
    switch( G_VALUE_TYPE(&valeur) )
     { case G_TYPE_BOOLEAN:
            return( json_node_get_boolean(node) );
       case G_TYPE_INT64:
       case G_TYPE_INT:
            return( json_node_get_int(node) );
       case G_TYPE_STRING:
            return (atoi ( json_node_get_string(node) ));
       default:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                     "%s: Valeur type unknown (%d) for name '%s'", __func__, json_node_get_value_type (node), name );
     }
    return(-1);
  }
/******************************************************************************************************************************/
/* Http_Send_response_code: Utiliser pour renvoyer un code reponse                                                            */
/* Entrée: La structure wsi de reference                                                                                      */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 gint Http_Send_response_code ( struct lws *wsi, gint code )
  { unsigned char header[256], *header_cur, *header_end;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, (code == HTTP_200_OK ? LOG_INFO : LOG_WARNING), "%s:  Sending Response code '%d'", __func__, code );

    header_cur = header;
    header_end = header + sizeof(header);

    lws_add_http_header_status( wsi, code, &header_cur, header_end );
    lws_finalize_write_http_header ( wsi, header, &header_cur, header_end );
    return(1);
  }
/******************************************************************************************************************************/
/* Http_Send_response_code: Utiliser pour renvoyer un code reponse                                                            */
/* Entrée: La structure wsi de reference                                                                                      */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 gint Http_Send_response_code_with_buffer ( struct lws *wsi, gint code, gchar *content_type, gchar *buffer, gint taille_buf )
  { unsigned char header[256], *header_cur, *header_end;
   	struct HTTP_PER_SESSION_DATA *pss;

    pss = lws_wsi_user ( wsi );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: Sending Response code '%d' (taille_buf=%d)", __func__, code, taille_buf
            );

    header_cur = header;
    header_end = header + sizeof(header);

    lws_add_http_header_status( wsi, code, &header_cur, header_end );
    lws_add_http_header_by_token ( wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
                                  (const unsigned char *)content_type, strlen(content_type),
                                  &header_cur, header_end );
    lws_add_http_header_content_length ( wsi, taille_buf, &header_cur, header_end );
    lws_finalize_write_http_header ( wsi, header, &header_cur, header_end );
    pss->send_buffer = buffer;
    pss->size_buffer = taille_buf;
    lws_callback_on_writable(wsi);
    return(0);
  }
/******************************************************************************************************************************/
/* Http_send_histo : envoie un histo au client                                                                                */
/* Entrée : la connexion client WebSocket et l'histo a envoyer                                                                */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void WS_send_histo ( struct lws *wsi, struct CMD_TYPE_HISTO *histo )
  { struct WS_PER_SESSION_DATA *pss;
    gchar *buf;
    JsonBuilder *builder;
    gsize taille_buf;

    pss = lws_wsi_user ( wsi );
    builder = Json_create ();
    if (!builder) return;

    json_builder_begin_object (builder);                                                                  /* Contenu du Histo */
    Json_add_bool ( builder, "alive", histo->alive );
    Json_add_string ( builder, "date_create",   histo->date_create );
    Json_add_string ( builder, "nom_ack",       histo->nom_ack );
    Json_add_string ( builder, "libelle",       histo->msg.libelle );
    Json_add_string ( builder, "syn_groupe",    histo->msg.syn_parent_page );
    Json_add_string ( builder, "syn_page",      histo->msg.syn_page );
    Json_add_string ( builder, "syn_libelle",   histo->msg.syn_libelle );
    Json_add_string ( builder, "dls_shortname", histo->msg.dls_shortname );
    json_builder_end_object (builder);                                                                           /* End Histo */

    buf = Json_get_buf ( builder, &taille_buf );
#ifdef bouh
    buf_to_send = g_try_malloc0( taille_buf + LWS_PRE );
    if (buf_to_send)
     { memcpy( buf_to_send + LWS_PRE, buf, taille_buf );
       lws_write(wsi, &buf_to_send[LWS_PRE], taille_buf, LWS_WRITE_TEXT );
       g_free(buf_to_send);                                           /* Libération du buffer dont nous n'avons plus besoin ! */
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
              "%s: send %d byte to '%s' ('%s')", __func__, taille_buf, pss->sid, pss->util );
#endif
    g_free(buf);
  }
/******************************************************************************************************************************/
/* Rechercher_utilisateurDB_by_sid: Recuperation de tous les champs de l'utilisateur dont le sid est en parametre             */
/* Entrées: le Session ID                                                                                                     */
/* Sortie: une structure utilisateur, ou null si erreur                                                                       */
/******************************************************************************************************************************/
 struct DB *Rechercher_utilisateurDB_by_sid( gchar *sid_brut )
  { gchar requete[512], *sid;
    struct DB *db;

    sid = Normaliser_chaine ( sid_brut );                                                    /* Formatage correct des chaines */
    if (!sid)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT username,id,access_level,enable "
                "FROM users WHERE session_id='%s' LIMIT 1", sid );
    g_free(sid);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed for sid '%s'", __func__, sid_brut );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }
    return(db);
  }
/******************************************************************************************************************************/
/* CB_ws_histos : Gere le protocole WS histos (appellée par libwebsockets)                                                    */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 static gint CB_ws_live_motifs ( struct lws *wsi, enum lws_callback_reasons tag, void *user, void *data, size_t taille )
  { struct WS_PER_SESSION_DATA *pss;
    gchar buffer[256];
    struct DB*db;
    pss = lws_wsi_user ( wsi );
/*    gchar *util;*/
    switch (tag)
     { case LWS_CALLBACK_ESTABLISHED:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: Checking cookie", __func__ );
            if ( lws_hdr_total_length( wsi, WSI_TOKEN_HTTP_COOKIE ) <= 0)
             { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: No Cookie found", __func__ );
               return(1);
             }
            lws_callback_on_writable(wsi);
            if ( lws_hdr_copy( wsi, buffer, sizeof(buffer), WSI_TOKEN_HTTP_COOKIE ) != -1 )/* Récupération de la valeur du token */
             { gchar *cookies, *cookie, *savecookies;
               gchar *cookie_name, *cookie_value, *savecookie;
               cookies = buffer;
               while ( (cookie=strtok_r( cookies, ";", &savecookies)) != NULL )                          /* Découpage par ';' */
                { cookies=NULL;
                  cookie_name=strtok_r( cookie, "=", &savecookie);                                       /* Découpage par "=" */
                  if (cookie_name)
                   { cookie_value = strtok_r ( NULL, "=", &savecookie );
                     if (!strcasecmp(cookie_name, "ci_session"))
                      { g_snprintf( pss->sid, sizeof(pss->sid), "%s", cookie_value ); }
                   }
                  Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                           "%s: Cookie found for: %s=%s", __func__,
                           (cookie_name ? cookie_name : "none"), (cookie_value ? cookie_value : "none") );
                }
             }
            db = Rechercher_utilisateurDB_by_sid ( pss->sid );
            if (!db)
             { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: No user found for session %s.", __func__, pss->sid );
               return(1);
             }
            g_snprintf( pss->username, sizeof(pss->username), "%s", db->row[0] );
            pss->user_id     = atoi(db->row[1]);
            pss->user_level  = atoi(db->row[2]);
            pss->user_enable = atoi(db->row[3]);
            Libere_DB_SQL ( &db );
            if (!pss->user_enable)
             { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                         "%s: %s: user found but disabled for session %s.", __func__, pss->username, pss->sid );
               return(1);
             }

            pss->zmq = Connect_zmq ( ZMQ_SUB, "listen-to-motifs", "inproc", ZMQUEUE_LIVE_MOTIFS, 0 );
            pss->zmq_local_bus = Connect_zmq ( ZMQ_SUB, "listen-to-bus",   "inproc", ZMQUEUE_LOCAL_BUS, 0 );
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: %s: WS callback established", __func__, pss->username );
            break;
       case LWS_CALLBACK_RECEIVE:
             { gchar *buffer;
               buffer = g_try_malloc0(taille+1);
               if (!buffer)
                { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: %s: Malloc Error %s.", __func__, pss->username );
                  return(1);
                }
               memcpy ( buffer, data, taille );
               Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "%s: WS callback receive %d bytes: %s", __func__, taille, buffer );
               g_free(buffer);
               lws_callback_on_writable(wsi);
             }
            break;
       case LWS_CALLBACK_CLOSED:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: %s: WS callback closed", __func__, pss->username );
            if (pss->zmq) Close_zmq(pss->zmq);
            if (pss->zmq_local_bus) Close_zmq(pss->zmq_local_bus);
            break;
       case LWS_CALLBACK_SERVER_WRITEABLE:
             { struct DLS_VISUEL visu;
               gchar json_buffer[2048];
               gint taille_buf;
               if ( pss->zmq && Recv_zmq ( pss->zmq, &visu, sizeof(struct DLS_VISUEL) ) == sizeof(struct DLS_VISUEL) )
                { JsonBuilder *builder;
                  gsize taille_buf;
                  gchar *buf, *result;
                  Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                            "%s: %s: Visuel %s:%s received", __func__, pss->username, visu.tech_id, visu.acronyme );
                  builder = Json_create ();
                  if (builder == NULL)
                   { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: JSon builder creation failed", __func__ );
                     return(1);
                   }
                  json_builder_begin_object (builder);                                         /* Création du noeud principal */
                  Json_add_string ( builder, "tech_id",  visu.tech_id );
                  Json_add_string ( builder, "acronyme", visu.acronyme );
                  Json_add_int    ( builder, "mode",     visu.mode );
                  Json_add_string ( builder, "color",    visu.color );
                  Json_add_bool   ( builder, "cligno",   visu.cligno );
                  json_builder_end_object (builder);                                                          /* End Document */
                  buf = Json_get_buf ( builder, &taille_buf );
                  result = (gchar *)g_malloc(LWS_SEND_BUFFER_PRE_PADDING + taille_buf);
                  if (result == NULL)
                   { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                               "%s: %s: JSon Result creation failed", __func__, pss->username );
                     g_free(buf);
                     return(1);
                   }
                  memcpy ( result + LWS_SEND_BUFFER_PRE_PADDING , buf, taille_buf );
                  g_free(buf);
                  lws_write	(	wsi,	result+LWS_SEND_BUFFER_PRE_PADDING, taille_buf, LWS_WRITE_TEXT );
                  g_free(result);
                }
               if ( pss->zmq_local_bus && (taille_buf = Recv_zmq ( pss->zmq_local_bus, &json_buffer, sizeof(json_buffer) )) > 0 )
                { gchar *result = (gchar *)g_malloc(LWS_SEND_BUFFER_PRE_PADDING + taille_buf);
                  if (result == NULL)
                   { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon Result creation failed", __func__ );
                     return(1);
                   }
                  memcpy ( result + LWS_SEND_BUFFER_PRE_PADDING , json_buffer, taille_buf );
                  lws_write	(	wsi,	result+LWS_SEND_BUFFER_PRE_PADDING, taille_buf, LWS_WRITE_TEXT );
                  g_free(result);
                }
             }
            lws_callback_on_writable(wsi);
            break;
       default: return(0);
     }
    return(0);
  }
/******************************************************************************************************************************/
/* CB_ws_histos : Gere le protocole WS histos (appellée par libwebsockets)                                                    */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 static gint CB_ws_histos ( struct lws *wsi, enum lws_callback_reasons tag, void *user, void *data, size_t taille )
  {
    struct WS_PER_SESSION_DATA *pss;
  /*  gchar *util;*/
    pss = lws_wsi_user ( wsi );
    switch (tag)
     { case LWS_CALLBACK_ESTABLISHED: lws_callback_on_writable(wsi);
/*            if (Get_phpsessionid_cookie(wsi)==FALSE)                                              /* Recupere le PHPSessionID */
/*             { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: No PHPSESSID. Killing.", __func__ );
               return(1);
             }
/*            util = Rechercher_util_by_phpsessionid ( pss->sid );*/
/*            if (!util)
             { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: No user found for session %s.", __func__, pss->sid );
               return(1);
             }
            g_snprintf( pss->util, sizeof(pss->util), "%s", util );
            g_free(util);

            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: WS callback established for %s", __func__, pss->util );*/
            pss->zmq = Connect_zmq ( ZMQ_SUB, "listen-to-msgs",   "inproc", ZMQUEUE_LIVE_MSGS, 0 );
            break;
       case LWS_CALLBACK_CLOSED:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: WS callback closed", __func__ );
            if (pss->zmq) Close_zmq(pss->zmq);
            break;
       case LWS_CALLBACK_SERVER_WRITEABLE:
             { struct CMD_TYPE_HISTO histo_buf;
               if ( pss->zmq && Recv_zmq ( pss->zmq, &histo_buf, sizeof(struct CMD_TYPE_HISTO) ) == sizeof(struct CMD_TYPE_HISTO) )
                { WS_send_histo ( wsi, &histo_buf );
                }
             }
            lws_callback_on_writable(wsi);
            break;
       default: return(0);
     }
    return(0);
  }
/******************************************************************************************************************************/
/* Http_CB_file_upload : Récupère les data de la requete POST en cours                                                        */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 gint Http_CB_file_upload( struct lws *wsi, char *buffer, int taille )
  { struct HTTP_PER_SESSION_DATA *pss;
    pss = lws_wsi_user ( wsi );

    if (pss->post_data_length >= Cfg_http.max_upload_bytes)                  /* Si taille de fichier trop importante, on vire */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s:  file too long (%d/%d), aborting", __func__,
                 pss->post_data_length, Cfg_http.max_upload_bytes );
       return(1);
     }

    pss->post_data = g_try_realloc ( pss->post_data, pss->post_data_length + taille + 1 );
    memcpy ( pss->post_data + pss->post_data_length, buffer, taille );
    pss->post_data_length += taille;
    pss->post_data[pss->post_data_length] = 0;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s:  received %d bytes (total length=%d, max %d)", __func__,
              taille, pss->post_data_length, Cfg_http.max_upload_bytes );
    return 0;
  }
/******************************************************************************************************************************/
/* Http_get_arg_int : Recupere l'argument de l'URI en parametre                                                               */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : la valeur du péramètre                                                                                            */
/******************************************************************************************************************************/
 gint Http_get_arg_int ( struct lws *wsi, gchar *arg )
  { gchar token_id[12];
    const gchar *id_s;

    id_s = lws_get_urlarg_by_name	( wsi, "id=", token_id, sizeof(token_id) );                      /* Recup du param get 'ID' */
    if (id_s) return(atoi(id_s));
    return(0);
  }
/******************************************************************************************************************************/
/* CB_http : Gere les connexion HTTP pures (appellée par libwebsockets)                                                       */
/* Entrées : le contexte, le message, l'URL                                                                                   */
/* Sortie : 1 pour clore, 0 pour continuer                                                                                    */
/******************************************************************************************************************************/
 static gint CB_http ( struct lws *wsi, enum lws_callback_reasons tag, void *user, void *data, size_t taille )
  { gchar remote_name[80], remote_ip[80];
    struct HTTP_PER_SESSION_DATA *pss = (struct HTTP_PER_SESSION_DATA *)user;

 /*   Http_Log_request(connection, url, method, version, upload_data_size, con_cls);*/
    switch (tag)
     { case LWS_CALLBACK_ESTABLISHED:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "CB_http: connexion established" );
		          break;
       case LWS_CALLBACK_CLOSED_HTTP:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "CB_http: connexion closed" );
            break;
       case LWS_CALLBACK_PROTOCOL_INIT:
            break;
       case LWS_CALLBACK_PROTOCOL_DESTROY:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "CB_http: Destroy protocol" );
		          break;
       case LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "CB_http: Need SSL Private key" );
		          break;
       case LWS_CALLBACK_RECEIVE:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "CB_http: data received" );
		          break;
       case LWS_CALLBACK_HTTP_BODY:
             { if ( ! strcasecmp ( pss->url, "/postfile" ) )
                { return( Http_Traiter_request_body_postfile ( wsi, data, taille ) ); }             /* Utilisation du lws_spa */
               else if ( ! strcasecmp ( pss->url, "/bus" ) )
                { return( Http_CB_file_upload( wsi, data, taille ) ); }     /* Sinon, c'est un buffer type json ou un fichier */
               else if ( ! strcasecmp ( pss->url, "/memory" ) )
                { return( Http_CB_file_upload( wsi, data, taille ) ); }     /* Sinon, c'est un buffer type json ou un fichier */
               return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));
             }
            break;
       case LWS_CALLBACK_HTTP_BODY_COMPLETION:
             { if ( ! strcasecmp ( pss->url, "/postfile" ) )
                { return( Http_Traiter_request_body_completion_postfile ( wsi ) ); }
               else if ( ! strcasecmp ( pss->url, "/bus" ) )
                { return( Http_Traiter_request_body_completion_bus ( wsi ) ); }
               else if ( ! strcasecmp ( pss->url, "/memory" ) )
                { return( Http_Traiter_request_body_completion_memory ( wsi ) ); }
               return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));
              }
            break;
       case LWS_CALLBACK_HTTP:
             { struct HTTP_PER_SESSION_DATA *pss;
               gchar *url = (gchar *)data;
               gint retour;
               lws_get_peer_addresses ( wsi, lws_get_socket_fd(wsi),
                                        (char *)&remote_name, sizeof(remote_name),
                                        (char *)&remote_ip, sizeof(remote_ip) );

               pss = lws_wsi_user ( wsi );
               pss->post_data = NULL;
               pss->post_data_length = 0;
               Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: processing request '%s'.", __func__, url );
               if ( ! strcasecmp ( url, "/favicon.ico" ) )
                { retour = lws_serve_http_file ( wsi, "WEB/favicon.gif", "image/gif", NULL, 0);
                  if (retour < 0) return(1);                              /* Si erreur (<0) ou si ok (>0), on ferme la socket */
                  return(0);
                }
               else if ( ! strcasecmp ( url, "/status" ) )         { return( Http_Traiter_request_getstatus ( wsi ) ); }
               else if ( ! strncasecmp ( url, "/audio/", 10 ) ) { return( Http_Traiter_request_getaudio ( wsi, remote_name, remote_ip, url+7 ) ); }
               else if ( ! strncasecmp ( url, "/process/", 9 ) ) { return( Http_Traiter_request_getprocess ( wsi, url+9 ) ); }
               else if ( ! strncasecmp ( url, "/dls/", 5 ) ) { return( Http_Traiter_request_getdls ( wsi, url+5 ) ); }
               else if ( ! strncasecmp ( url, "/memory", 7 ) )
                { g_snprintf( pss->url, sizeof(pss->url), "/memory" );
                  return(0);
                }
               else if ( ! strcasecmp ( url, "/bus" ) )
                { g_snprintf( pss->url, sizeof(pss->url), "/bus" );
                  return(0);
                }
               else if ( ! strcasecmp ( url, "/postfile" ) )
                { g_snprintf( pss->url, sizeof(pss->url), "/postfile" );
                  return(0);
                }
               else if ( ! strcasecmp ( url, "/log/debug" ) )   { Info_change_log_level ( Config.log, LOG_DEBUG   ); return(1); }
               else if ( ! strcasecmp ( url, "/log/notice" ) )  { Info_change_log_level ( Config.log, LOG_NOTICE  ); return(1); }
               else if ( ! strcasecmp ( url, "/log/info" ) )    { Info_change_log_level ( Config.log, LOG_INFO    ); return(1); }
               else if ( ! strcasecmp ( url, "/log/warning" ) ) { Info_change_log_level ( Config.log, LOG_WARNING ); return(1); }
               else if ( ! strcasecmp ( url, "/log/error" ) )   { Info_change_log_level ( Config.log, LOG_ERR     ); return(1); }
               Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Unknown Request from %s/%s : %s",
                         __func__, remote_name, remote_ip, url );
               return(Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ));                                      /* Bad Request */
             }
		          break;
       case LWS_CALLBACK_HTTP_WRITEABLE:
             { gint retour;
               Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s: Http_Writeable !", __func__ );
               retour = lws_write(wsi, pss->send_buffer, pss->size_buffer, LWS_WRITE_HTTP_FINAL);
               g_free(pss->send_buffer);
               if (retour != pss->size_buffer) return(1);
				           if (lws_http_transaction_completed(wsi))	return -1;
               else lws_callback_on_writable(wsi);
               return 0;
             }
       case LWS_CALLBACK_HTTP_FILE_COMPLETION:
             { lws_get_peer_addresses ( wsi, lws_get_socket_fd(wsi),
                                        (char *)&remote_name, sizeof(remote_name),
                                        (char *)&remote_ip, sizeof(remote_ip) );
               Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "CB_http: file sent for %s(%s)", remote_name, remote_ip );
             }
            break;
       case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
            Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "CB_http: need to verify Client SSL Certs" );
		          break;
       default: return(0);
     }
    return(0);                                                      /* Poursuite du processus de prise en charge des requetes */
  }
/******************************************************************************************************************************/
/* Run_thread: Thread principal                                                                                               */
/* Entrée: une structure LIBRAIRIE                                                                                            */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct lws_protocols WS_PROTOS[] =
     { { "http-only", CB_http, sizeof(struct HTTP_PER_SESSION_DATA), 0 },       /* first protocol must always be HTTP handler */
       { "histos", CB_ws_histos, sizeof(struct WS_PER_SESSION_DATA), 0 },
       { "live-motifs", CB_ws_live_motifs, sizeof(struct WS_PER_SESSION_DATA), 0 },
       { NULL, NULL, 0, 0 } /* terminator */
     };
    struct stat sbuf;

    prctl(PR_SET_NAME, "W-HTTP", 0, 0, 0 );
reload:
    memset( &Cfg_http, 0, sizeof(Cfg_http) );                                       /* Mise a zero de la structure de travail */
    Cfg_http.lib = lib;                                            /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_http.lib->TID = pthread_self();                                                     /* Sauvegarde du TID pour le pere */
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );
    Http_Lire_config ();                                                    /* Lecture de la configuration logiciel du thread */

    g_snprintf( Cfg_http.lib->admin_prompt, sizeof(Cfg_http.lib->admin_prompt), NOM_THREAD );
    g_snprintf( Cfg_http.lib->admin_help,   sizeof(Cfg_http.lib->admin_help),   "Manage Web Services with external Devices" );

    lws_set_log_level(Cfg_http.lws_debug_level, lwsl_emit_syslog);

    Cfg_http.ws_info.iface = NULL;                                                      /* Configuration du serveur Websocket */
	   Cfg_http.ws_info.protocols = WS_PROTOS;
    Cfg_http.ws_info.port = Cfg_http.tcp_port;
	   Cfg_http.ws_info.gid = -1;
	   Cfg_http.ws_info.uid = -1;
	   Cfg_http.ws_info.max_http_header_pool = Cfg_http.nbr_max_connexion;
	   Cfg_http.ws_info.options |= LWS_SERVER_OPTION_VALIDATE_UTF8;
	   Cfg_http.ws_info.extensions = NULL;
	   Cfg_http.ws_info.timeout_secs = 30;

    if (Cfg_http.ssl_enable)                                                                           /* Configuration SSL ? */
     { if ( stat ( Cfg_http.ssl_cert_filepath, &sbuf ) == -1)                                     /* Test présence du fichier */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "%s: unable to load '%s' (error '%s'). Setting ssl=FALSE", __func__,
                    Cfg_http.ssl_cert_filepath, strerror(errno) );
          Cfg_http.ssl_enable=FALSE;
        }
       else if ( stat ( Cfg_http.ssl_private_key_filepath, &sbuf ) == -1)                         /* Test présence du fichier */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "%s: unable to load '%s' (error '%s'). Setting ssl=FALSE", __func__,
                    Cfg_http.ssl_private_key_filepath, strerror(errno) );
          Cfg_http.ssl_enable=FALSE;
        }
       else if ( stat ( Cfg_http.ssl_ca_filepath, &sbuf ) == -1)                                  /* Test présence du fichier */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "%s: unable to load '%s' (error '%s'). Setting ssl=FALSE", __func__,
                    Cfg_http.ssl_ca_filepath, strerror(errno) );
          Cfg_http.ssl_enable=FALSE;
        }
       else
        { Cfg_http.ws_info.ssl_cipher_list          = Cfg_http.ws_info.ssl_cipher_list;
   	      Cfg_http.ws_info.ssl_cert_filepath        = Cfg_http.ssl_cert_filepath;
	         Cfg_http.ws_info.ssl_ca_filepath          = Cfg_http.ssl_ca_filepath;
   	      Cfg_http.ws_info.ssl_private_key_filepath = Cfg_http.ssl_private_key_filepath;
          Cfg_http.ws_info.options |= LWS_SERVER_OPTION_PEER_CERT_NOT_REQUIRED;
          /*Cfg_http.ws_info.options |= LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS;*/
          /*Cfg_http.ws_info.options |= LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;*/
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                   "%s: Stat '%s' OK", __func__, Cfg_http.ws_info.ssl_cert_filepath );
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                   "%s: Stat '%s' OK", __func__, Cfg_http.ws_info.ssl_private_key_filepath );
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                   "%s: Stat '%s' OK", __func__, Cfg_http.ws_info.ssl_ca_filepath );
        }
     }

	   Cfg_http.ws_context = lws_create_context(&Cfg_http.ws_info);

    if (!Cfg_http.ws_context)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: WebSocket Create Context creation error (%s). Shutting Down %p", __func__,
                 strerror(errno), pthread_self() );
       goto end;
     }

    /*Cfg_http.zmq_from_bus = New_zmq ( ZMQ_SUB, "listen-to-bus" );
    Connect_zmq ( Cfg_http.zmq_from_bus, "inproc", ZMQUEUE_LOCAL_BUS, 0 );*/

    Cfg_http.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
             "%s: WebSocket Create OK. Listening on port %d with ssl=%d", __func__, Cfg_http.tcp_port, Cfg_http.ssl_enable );

    Cfg_http.lib->Thread_run = TRUE;                                                                    /* Le thread tourne ! */
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_http.lib->Thread_reload)                                                      /* A-t'on recu un signal USR1 ? */
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Thread Reload !", __func__ );
          break;
        }

   	   lws_service( Cfg_http.ws_context, 1000);                                 /* On lance l'écoute des connexions websocket */
     }

    lws_context_destroy(Cfg_http.ws_context);                                                   /* Arret du serveur WebSocket */
    Cfg_http.ws_context = NULL;

    /*Close_zmq ( Cfg_http.zmq_from_bus );*/
    Close_zmq ( Cfg_http.zmq_to_master );
end:
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    if (lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Cfg_http.lib->Thread_run = FALSE;                                                           /* Le thread ne tourne plus ! */
    Cfg_http.lib->TID = 0;                                                    /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
