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
  };

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
               if (!ci)                                       /* si AI pas trouvée, on remonte le nom du cadran en libellé */
                { cadran->in_range = FALSE;
                  break;
                }
               cadran->in_range = TRUE;
               cadran->valeur *= ci->multi;                                                            /* Multiplication ! */
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
/* Envoyer_un_cadran: Envoi nu update cadran au client                                                                        */
/* Entrée: une reference sur la session en cours, et le cadran a envoyer                                                      */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Envoyer_un_cadran ( struct WS_CLIENT_SESSION *client, struct WS_CADRAN *ws_cadran )
  { gsize taille_buf;
    JsonBuilder *builder = Json_create();
    if (!builder) return;
    Json_add_string ( builder, "msg_type", "update_cadran" );
    Json_add_string ( builder, "tech_id",  ws_cadran->tech_id );
    Json_add_string ( builder, "acronyme", ws_cadran->acronyme );
    Json_add_int    ( builder, "type",     ws_cadran->type );
    Json_add_bool   ( builder, "in_range", ws_cadran->in_range );
    Json_add_double ( builder, "valeur",   ws_cadran->valeur );
    Json_add_string ( builder, "unite",    ws_cadran->unite );
    gchar *buf = Json_get_buf (builder, &taille_buf);
    GBytes *gbytes = g_bytes_new_take ( buf, taille_buf );
    soup_websocket_connection_send_message ( client->connexion, SOUP_WEBSOCKET_DATA_TEXT, gbytes );
    g_bytes_unref( gbytes );
  }
/******************************************************************************************************************************/
/* Afficher_un_cadran: Ajoute un cadran sur la trame                                                                          */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Abonner_un_cadran (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct WS_CLIENT_SESSION *client = user_data;
    struct DB *db;

    struct WS_CADRAN *ws_cadran;
    ws_cadran = (struct WS_CADRAN *)g_try_malloc0(sizeof(struct WS_CADRAN));
    if (!ws_cadran) { return; }

    g_snprintf( ws_cadran->tech_id,  sizeof(ws_cadran->tech_id), "%s", Json_get_string(element, "tech_id") );
    g_snprintf( ws_cadran->acronyme, sizeof(ws_cadran->acronyme), "%s", Json_get_string(element, "acronyme") );
    if ( (db=Rechercher_CI ( ws_cadran->tech_id, ws_cadran->acronyme )) != NULL )
     { ws_cadran->type = MNEMO_CPT_IMP;
       Libere_DB_SQL (&db);
     }
    else if ( (db=Rechercher_AI ( ws_cadran->tech_id, ws_cadran->acronyme )) != NULL )
     { ws_cadran->type = MNEMO_ENTREE_ANA;
       Libere_DB_SQL (&db);
     }
    else if ( (db=Rechercher_Tempo ( ws_cadran->tech_id, ws_cadran->acronyme )) != NULL )
     { ws_cadran->type = MNEMO_TEMPO;
       Libere_DB_SQL (&db);
     }
    else if ( (db=Rechercher_CH ( ws_cadran->tech_id, ws_cadran->acronyme )) != NULL )
     { ws_cadran->type = MNEMO_CPTH;
       Libere_DB_SQL (&db);
     }
    else { g_free(ws_cadran); return; }                                                                   /* Si pas trouvé... */

    Formater_cadran(ws_cadran);                                                            /* Formatage de la chaine associée */
    Envoyer_un_cadran ( client, ws_cadran );                                                        /* Envoi de l'init cadran */

    client->Liste_bit_cadrans = g_slist_prepend( client->Liste_bit_cadrans, ws_cadran );
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

    gchar *msg_type = Json_get_string( response, "msg_type" );
    if (!msg_type)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING, "%s: WebSocket Message Dropped (no 'msg_type') !", __func__ );
       json_node_unref(response);
       return;
     }
    if(!strcmp(msg_type,"abonnements"))
     { JsonArray *array;
       array = Json_get_array ( response, "cadrans" );
       if (array) { json_array_foreach_element ( array, Abonner_un_cadran, client ); }
     }
    json_node_unref(response);
  }

 static void Http_ws_motifs_on_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { struct WS_CLIENT_SESSION *client = user_data;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: WebSocket Close Connexion received !", __func__ );
    g_object_unref(connexion);
    Cfg_http.liste_ws_motifs_clients = g_slist_remove ( Cfg_http.liste_ws_motifs_clients, client );
    g_free(client);
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
    Cfg_http.liste_ws_motifs_clients = g_slist_prepend ( Cfg_http.liste_ws_motifs_clients, client );
    g_object_ref(connexion);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/