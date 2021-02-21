/******************************************************************************************************************************/
/* Watchdogd/Http/ws_motifs.c        Gestion des echanges des elements visuels de watchdog                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    06.05.2020 09:53:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ws_motifs.c
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
/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;

/******************************************************************************************************************************/
 struct WS_CADRAN
  { gchar tech_id[32];
    gchar acronyme[64];
    gchar unite[32];
    gint type;
    gpointer dls_data;
    gfloat old_valeur;
    gfloat valeur;
    gboolean in_range;
    gint last_update;
  };

/******************************************************************************************************************************/
/* Envoi_au_serveur: Envoi une requete web au serveur Watchdogd                                                               */
/* Entrée: des infos sur le paquet à envoyer                                                                                  */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_ws_au_client ( struct WS_CLIENT_SESSION *client, JsonBuilder *builder )
  { gsize taille_buf;
    gchar *buf = Json_get_buf (builder, &taille_buf);
    GBytes *gbytes = g_bytes_new_take ( buf, taille_buf );
    soup_websocket_connection_send_message ( client->connexion, SOUP_WEBSOCKET_DATA_TEXT, gbytes );
    g_bytes_unref( gbytes );
  }
/******************************************************************************************************************************/
/* Envoyer_un_visuel: Envoi un update motif au client                                                                         */
/* Entrée: une reference sur la session en cours, et le cadran a envoyer                                                      */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Envoyer_un_visuel ( JsonNode *visuel )
  { gsize taille_buf;
    gchar *buf = Json_node_to_string ( visuel, &taille_buf );
    json_node_unref ( visuel );
    GSList *liste = Cfg_http.liste_ws_motifs_clients;
    while (liste)
     { struct WS_CLIENT_SESSION *client = liste->data;
       soup_websocket_connection_send_text ( client->connexion, buf );
       liste = g_slist_next(liste);
     }
    g_free(buf);
  }
/******************************************************************************************************************************/
/* Http_ws_send_to_all: Envoi d'un buffer a tous les clients connectés à la websocket                                         */
/* Entrée: Le buffer                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_ws_send_to_all ( JsonNode *node )
  { gsize taille_buf;
    gchar *buf = Json_node_to_string ( node, &taille_buf );
    json_node_unref ( node );
    GSList *liste = Cfg_http.liste_ws_motifs_clients;
    while (liste)
     { struct WS_CLIENT_SESSION *client = liste->data;
       soup_websocket_connection_send_text ( client->connexion, buf );
       liste = g_slist_next(liste);
     }
    g_free(buf);
  }
/******************************************************************************************************************************/
/* Recuperer_histo_msgsDB_alive: Recupération de l'ensemble des messages encore Alive dans le BDD                             */
/* Entrée: La base de données de travail                                                                                      */
/* Sortie: False si probleme                                                                                                  */
/******************************************************************************************************************************/
 void Http_ws_send_pulse_to_all ( void )
  { JsonBuilder *builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: JSon builder creation failed", __func__ );
       return;
     }
    Json_add_string( builder, "zmq_tag", "PULSE" );
    JsonNode *node = Json_end ( builder );
    Http_ws_send_to_all ( node );
  }
/******************************************************************************************************************************/
/* Formater_cadran: Formate la structure dédiée cadran pour envoi au client                                                   */
/* Entrée: un cadran                                                                                                          */
/* Sortie: une structure prete à l'envoie                                                                                     */
/******************************************************************************************************************************/
 static void Formater_cadran( struct WS_CADRAN *cadran )
  {
    if (!cadran) return;
    switch(cadran->type)
     { /*case MNEMO_BISTABLE:
            cadran->in_range = TRUE;
            cadran->valeur = 1.0 * Dls_data_get_bool ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
            break;
       case MNEMO_ENTREE:
            cadran->in_range = TRUE;
            cadran->valeur = 1.0 * Dls_data_get_DI ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
            break;*/
       case MNEMO_ENTREE_ANA:
             { struct DLS_AI *ai;
               cadran->valeur = Dls_data_get_AI(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
               if (!cadran->dls_data)                            /* si AI pas trouvée, on remonte le nom du cadran en libellé */
                { cadran->in_range = FALSE;
                  break;
                }
               ai = (struct DLS_AI *)cadran->dls_data;
               cadran->in_range = ai->inrange;
               cadran->valeur = ai->val_ech;
               g_snprintf( cadran->unite, sizeof(cadran->unite), "%s", ai->unite );
             }
            break;
       case MNEMO_CPTH:
             { cadran->in_range = TRUE;
               cadran->valeur = Dls_data_get_CH(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
             }
            break;
       case MNEMO_CPT_IMP:
             { cadran->valeur = Dls_data_get_CI(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
               struct DLS_CI *ci=cadran->dls_data;
               if (!ci)                                          /* si AI pas trouvée, on remonte le nom du cadran en libellé */
                { cadran->in_range = FALSE;
                  break;
                }
               cadran->in_range = TRUE;
               cadran->valeur *= ci->multi;                                                               /* Multiplication ! */
               g_snprintf( cadran->unite, sizeof(cadran->unite), "%s", ci->unite );
             }
            break;
       case MNEMO_REGISTRE:
            cadran->valeur = -1.0;
            g_snprintf( cadran->unite, sizeof(cadran->unite), "?" );
            cadran->in_range = TRUE;
            break;
       case MNEMO_TEMPO:
            Dls_data_get_tempo ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
            struct DLS_TEMPO *tempo = cadran->dls_data;
            if (!tempo)
             { cadran->in_range = FALSE;
               break;
             }
            cadran->in_range = FALSE;

            if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON)                     /* Temporisation Retard en train de compter */
             { cadran->valeur = (tempo->date_on - Partage->top); }
            else if (tempo->status == DLS_TEMPO_NOT_COUNTING)                  /* Tempo ne compte pas: on affiche la consigne */
             { cadran->valeur = tempo->delai_on; }
            break;
       default:
            cadran->in_range = FALSE;
            break;
      }
  }
/******************************************************************************************************************************/
/* Envoyer_un_cadran: Envoi un update cadran au client                                                                        */
/* Entrée: une reference sur la session en cours, et le cadran a envoyer                                                      */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void WS_CADRAN_to_json ( JsonBuilder *builder, struct WS_CADRAN *ws_cadran )
  { Formater_cadran ( ws_cadran );
    Json_add_string ( builder, "tech_id",  ws_cadran->tech_id );
    Json_add_string ( builder, "acronyme", ws_cadran->acronyme );
    Json_add_int    ( builder, "type",     ws_cadran->type );
    Json_add_bool   ( builder, "in_range", ws_cadran->in_range );
    Json_add_double ( builder, "valeur",   ws_cadran->valeur );
    Json_add_string ( builder, "unite",    ws_cadran->unite );
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_ws_motifs_send_synoptique ( struct WS_CLIENT_SESSION *client, gint syn_id )
  { gchar chaine[256];

    struct DB *db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: DB connexion failed for user '%s'", __func__, client->http_session->username );
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT access_level FROM syns WHERE id=%d", syn_id );
    if ( Lancer_requete_SQL ( db, chaine ) == FALSE )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: DB request failed for user '%s'",__func__, client->http_session->username );
       return;
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: Syn '%d' unknown", __func__, syn_id );
       return;
     }
    gint syn_access_level = atoi(db->row[0]);
    Liberer_resultat_SQL (db);
    Libere_DB_SQL( &db );

    if (client->http_session->access_level < syn_access_level )
     { Audit_log ( client->http_session, "Access to '%d' forbidden", syn_id );
       return;
     }

    JsonBuilder *builder = Json_create ();
    if (!builder) return;

    Json_add_string ( builder, "zmq_tag", "INIT_SYN" );

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns WHERE id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, NULL, chaine ) == FALSE)
     { g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_motifs WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "motifs", chaine ) == FALSE)
     { g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT sp.*,syn.page,syn.libelle FROM syns_pass as sp "
                                       "INNER JOIN syns as syn ON sp.syn_cible_id=syn.id "
                                       "WHERE sp.syn_id=%d AND syn.access_level<=%d", syn_id, client->http_session->access_level );
    if (SQL_Select_to_JSON ( builder, "passerelles", chaine ) == FALSE)
     { g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_liens WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "liens", chaine ) == FALSE)
     { g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_rectangles WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "rectangles", chaine ) == FALSE)
     { g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT * from syns_comments WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "comments", chaine ) == FALSE)
     { g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT *,src.location,src.libelle from syns_camerasup AS cam "
                                       "INNER JOIN cameras AS src ON cam.camera_src_id=src.id WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "cameras", chaine ) == FALSE)
     { g_object_unref(builder);
       return;
     }

    g_snprintf(chaine, sizeof(chaine), "SELECT syns_cadrans.* FROM syns_cadrans WHERE syn_id=%d", syn_id );
    if (SQL_Select_to_JSON ( builder, "cadrans", chaine ) == FALSE)
     { g_object_unref(builder);
       return;
     }

/*----------------------------------------------- Visuels --------------------------------------------------------------------*/
    Json_add_array ( builder, "visuels" );
    db = Init_DB_SQL();
    if (db)
     { g_snprintf(chaine, sizeof(chaine), "SELECT tech_id, acronyme FROM syns_motifs WHERE syn_id=%d", syn_id );
       Lancer_requete_SQL ( db, chaine );                                                      /* Execution de la requete SQL */
       while(Recuperer_ligne_SQL(db))                                                      /* Chargement d'une ligne resultat */
        { gchar *tech_id  = db->row[0];
          gchar *acronyme = db->row[1];
          struct DLS_VISUEL *visuel = NULL;
          Dls_data_get_VISUEL ( tech_id, acronyme, (gpointer)&visuel );
          if ( visuel && !g_slist_find ( client->Liste_bit_visuels, visuel ) )
           { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: user '%s': Abonné au VISUEL %s:%s", __func__,
                       client->http_session->username, visuel->tech_id, visuel->acronyme );
             Json_add_object ( builder, NULL );
             Dls_VISUEL_to_json ( builder, visuel );
             Json_end_object ( builder );
           }
          client->Liste_bit_visuels = g_slist_prepend( client->Liste_bit_visuels, visuel );
        }
       Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
     }
    Json_end_array ( builder );

/*----------------------------------------------- Cadrans --------------------------------------------------------------------*/
    Json_add_array ( builder, "cadrans" );
    db = Init_DB_SQL();
    if (db)
     { g_snprintf(chaine, sizeof(chaine), "SELECT tech_id,acronyme FROM syns_cadrans WHERE syn_id=%d", syn_id );
       Lancer_requete_SQL ( db, chaine );                                                      /* Execution de la requete SQL */
       while(Recuperer_ligne_SQL(db))                                                      /* Chargement d'une ligne resultat */
        { gchar *tech_id  = db->row[0];
          gchar *acronyme = db->row[1];
          GSList *liste = client->Liste_bit_cadrans;
          while(liste)
           { struct WS_CADRAN *cadran=liste->data;
             if ( !strcasecmp( tech_id, cadran->tech_id ) && !strcasecmp( acronyme, cadran->acronyme ) ) break;
             liste = g_slist_next(liste);
           }
          if (!liste)                                       /* si le cadran n'est pas trouvé, on l'ajoute a la liste d'abonné */
           { struct WS_CADRAN *ws_cadran;
             ws_cadran = (struct WS_CADRAN *)g_try_malloc0(sizeof(struct WS_CADRAN));
             if (ws_cadran)
              { g_snprintf( ws_cadran->tech_id,  sizeof(ws_cadran->tech_id),  "%s", tech_id  );
                g_snprintf( ws_cadran->acronyme, sizeof(ws_cadran->acronyme), "%s", acronyme );
                ws_cadran->type = Rechercher_DICO_type ( ws_cadran->tech_id, ws_cadran->acronyme );
                if (ws_cadran->type!=-1)
                 { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: user '%s': Abonné au CADRAN %s:%s", __func__,
                             client->http_session->username, ws_cadran->tech_id, ws_cadran->acronyme );
                   Json_add_object ( builder, NULL );
                   WS_CADRAN_to_json ( builder, ws_cadran );
                   Json_end_object ( builder );
                   client->Liste_bit_cadrans = g_slist_prepend( client->Liste_bit_cadrans, ws_cadran );
                 }
                else { g_free(ws_cadran); }                                                               /* Si pas trouvé... */
              }
           }
        }
       Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
     }
    Json_end_array ( builder );

    Envoyer_ws_au_client ( client, builder );
    Audit_log ( client->http_session, "Synoptique '%d' sent", syn_id );
  }
/******************************************************************************************************************************/
/* Http_ws_motifs_on_message: Appelé par libsoup lorsque l'on recoit un message sur la websocket                              */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_ws_motifs_on_message ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { struct WS_CLIENT_SESSION *client = user_data;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Message received !", __func__ );
    gsize taille;

    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: WebSocket Message Dropped (not JSON) !", __func__ );
       return;
     }

    if (!Json_has_member ( response, "zmq_tag" ))
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: WebSocket Message Dropped (no 'zmq_tag') !", __func__ );
       json_node_unref(response);
       return;
     }
    gchar *zmq_tag = Json_get_string( response, "zmq_tag" );

    if(!strcasecmp(zmq_tag,"GET_SYNOPTIQUE"))
     { if ( ! (Json_has_member( response, "wtd_session") && Json_has_member ( response, "syn_id" ) ))
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: WebSocket 'Send_wtd_session' without wtd_session !", __func__ ); }
       else
        { gchar *wtd_session = Json_get_string ( response, "wtd_session");
          GSList *liste = Cfg_http.liste_http_clients;
          while ( liste )
           { struct HTTP_CLIENT_SESSION *http_session = liste->data;
             if (!strcmp(http_session->wtd_session, wtd_session))
              { client->http_session = http_session;
                Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: session found for '%s' !", __func__, http_session->username );
                Http_ws_motifs_send_synoptique ( client, Json_get_int ( response, "syn_id" ) );
                break;
              }
             liste = g_slist_next ( liste );
           }
        }
     }
    else if (!client->http_session)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: Not authorized !", __func__ ); }
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Http_Envoyer_les_cadrans: Envoi les cadrans aux clients                                                                    */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_Envoyer_les_cadrans ( void )
  { pthread_mutex_lock( &Cfg_http.lib->synchro );
    GSList *clients = Cfg_http.liste_ws_motifs_clients;
    while (clients)
     { struct WS_CLIENT_SESSION *client = clients->data;
       if (client->http_session)
        { GSList *cadrans = client->Liste_bit_cadrans;
          while (cadrans)
           { struct WS_CADRAN *cadran = cadrans->data;
             if (cadran->last_update +10 <= Partage->top)
              { JsonBuilder *builder = Json_create();
                if (builder)
                 { Json_add_string ( builder, "zmq_tag", "DLS_CADRAN" );
                   WS_CADRAN_to_json ( builder, cadran );
                   Envoyer_ws_au_client ( client, builder );
                 }
                cadran->last_update = Partage->top;
              }
             cadrans = g_slist_next(cadrans);
           }
        }
       clients = g_slist_next(clients);
     }
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
  }
/******************************************************************************************************************************/
/* Http_ws_destroy_session: Supprime une session WS                                                                           */
/* Entrée: la WS                                                                                                              */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_ws_destroy_session ( struct WS_CLIENT_SESSION *client )
  { pthread_mutex_lock( &Cfg_http.lib->synchro );
    Cfg_http.liste_ws_motifs_clients = g_slist_remove ( Cfg_http.liste_ws_motifs_clients, client );
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Session closed !", __func__ );
    g_object_unref(client->connexion);
    g_slist_foreach ( client->Liste_bit_cadrans, (GFunc)g_free, NULL );
    g_slist_free ( client->Liste_bit_visuels );
    g_free(client);
  }
/******************************************************************************************************************************/
/* Http_ws_motifs_on_closed: Traite une deconnexion                                                                           */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Http_ws_motifs_on_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { struct WS_CLIENT_SESSION *client = user_data;
    Http_ws_destroy_session ( client );
  }
 static void Http_ws_motifs_on_error ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Error received %p!", __func__, self );
  }
/******************************************************************************************************************************/
/* Http_traiter_websocket: Traite une requete websocket                                                                       */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_traiter_open_websocket_motifs_CB ( SoupServer *server, SoupWebsocketConnection *connexion, const char *path,
                                              SoupClientContext *context, gpointer user_data)
  { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Opened %p state %d!", __func__, connexion,
              soup_websocket_connection_get_state (connexion) );
    struct WS_CLIENT_SESSION *client = g_try_malloc0( sizeof(struct WS_CLIENT_SESSION) );
    if(!client)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s: WebSocket Memory error. Closing !", __func__ );
       return;
     }
    client->connexion = connexion;
    client->context = context;
    g_signal_connect ( connexion, "message", G_CALLBACK(Http_ws_motifs_on_message), client );
    g_signal_connect ( connexion, "closed",  G_CALLBACK(Http_ws_motifs_on_closed), client );
    g_signal_connect ( connexion, "error",   G_CALLBACK(Http_ws_motifs_on_error), client );
    /*soup_websocket_connection_send_text ( connexion, "Welcome on Watchdog WebSocket !" );*/
    pthread_mutex_lock( &Cfg_http.lib->synchro );
    Cfg_http.liste_ws_motifs_clients = g_slist_prepend ( Cfg_http.liste_ws_motifs_clients, client );
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
    g_object_ref(connexion);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
