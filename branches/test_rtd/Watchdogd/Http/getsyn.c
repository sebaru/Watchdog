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
                                 "SELECT syn.*, psyn.page as ppage, psyn.libelle AS plibelle, psyn.id AS pid, "
                                         "(SELECT COUNT(*) FROM dls WHERE dls.syn_id=syn.id) AS dls_count, "
                                         "(SELECT COUNT(*) FROM syns AS sub_syn WHERE syn.id=sub_syn.parent_id) AS subsyn_count "
                                 "FROM syns AS syn "
                                 "INNER JOIN syns AS psyn ON psyn.id=syn.parent_id "
                                 "WHERE syn.access_level<='%d' ORDER BY syn.page", session->access_level ) == FALSE)
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

    if ( Json_has_member ( request, "syn_id" ) )
     { if (Json_has_member ( request, "image" ) )
        { gchar *chaine = Normaliser_chaine ( Json_get_string ( request, "image" ) );
          SQL_Write_new ( "UPDATE syns SET image='%s' WHERE id='%d' AND access_level<='%d'",
                          chaine, Json_get_int ( request, "syn_id" ), session->access_level );
          g_free(chaine);
        }

       if ( Json_has_member ( request, "libelle" ) )
        { gchar *chaine = Normaliser_chaine ( Json_get_string ( request, "libelle" ) );
          SQL_Write_new ( "UPDATE syns SET libelle='%s' WHERE id='%d' AND access_level<='%d'",
                          chaine, Json_get_int ( request, "syn_id" ), session->access_level );
          g_free(chaine);
        }

       if ( Json_has_member ( request, "page" ) )
        { gchar *chaine = Normaliser_chaine ( Json_get_string ( request, "page" ) );
          SQL_Write_new ( "UPDATE syns SET page='%s' WHERE id='%d' AND access_level<='%d'",
                          chaine, Json_get_int ( request, "syn_id" ), session->access_level );
          g_free(chaine);
        }

       if ( Json_has_member ( request, "access_level" ) )
        { SQL_Write_new ( "UPDATE syns SET access_level='%d' WHERE id='%d' AND access_level<='%d'",
                          Json_get_int ( request, "access_level" ), Json_get_int ( request, "syn_id" ), session->access_level );
        }

       if ( Json_has_member ( request, "parent_id" ) )
        { SQL_Write_new ( "UPDATE syns SET parent_id='%d' WHERE id='%d' AND access_level<='%d'",
                          Json_get_int ( request, "parent_id" ), Json_get_int ( request, "syn_id" ), session->access_level );
          Partage->com_dls.Thread_reload = TRUE;                                                               /* Relance DLS */
        }
       soup_message_set_status (msg, SOUP_STATUS_OK);
       goto end;
     }
    else if ( ! (Json_has_member ( request, "libelle" ) && Json_has_member ( request, "page" ) &&                 /* Si ajout */
                 Json_has_member ( request, "parent_id" ) && Json_has_member ( request, "access_level" ) &&
                 Json_has_member ( request, "image" )
                ) )
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

    gint access_level  = Json_get_int ( request, "access_level" );
    if (access_level>session->access_level)
     { json_node_unref(request);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }
    gint   parent_id   = Json_get_int ( request, "parent_id" );
    gchar *libelle     = Normaliser_chaine ( Json_get_string( request, "libelle" ) );
    gchar *page        = Normaliser_chaine ( Json_get_string( request, "page" ) );
    gchar *image       = Normaliser_chaine ( Json_get_string( request, "image" ) );

    g_snprintf( requete, sizeof(requete),
               "INSERT INTO syns SET libelle='%s', parent_id=%d, page='%s', image='%s', "
                "access_level='%d'", libelle, parent_id, page, image, access_level );

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

end:
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    gint syn_id;
    gchar *syn_id_src = g_hash_table_lookup ( query, "syn_id" );
    if (syn_id_src) { syn_id = atoi (syn_id_src); }
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       return;
     }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (RootNode == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : JSon RootNode creation failed", __func__ );
       soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    if (SQL_Select_to_json_node ( RootNode, NULL,
                                 "SELECT s.*, ps.page AS ppage "
                                 "FROM syns AS s "
                                 "INNER JOIN syns AS ps ON s.parent_id = ps.id "
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
/* Http_get_syn_save_un_visuel: Enregistre un motif en base de données                                                        */
/* Entrée: les données JSON recu de la requete HTTP                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_syn_save_un_visuel (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct HTTP_CLIENT_SESSION *session = user_data;
    if ( ! (Json_has_member ( element, "visuel_id" ) &&
            Json_has_member ( element, "posx" ) &&
            Json_has_member ( element, "posy" ) &&
            Json_has_member ( element, "def_color" ) &&
            Json_has_member ( element, "angle" ) &&
            Json_has_member ( element, "gestion" ) &&
            Json_has_member ( element, "libelle" ) &&
            Json_has_member ( element, "groupe" ) &&
            Json_has_member ( element, "scale" )
           ) )
     { return; }

    gchar *def_color = Normaliser_chaine( Json_get_string ( element, "def_color" ) );

    SQL_Write_new( "UPDATE syns_visuels AS visu "
                   "INNER JOIN dls ON dls.tech_id=syns_visuel.tech_id "
                   "INNER JOIN syns AS s ON dls.syn_id = s.id SET "
                   "visu.posx='%d', visu.posy='%d', visu.groupe='%d',"
                   "visu.def_color='%s', visu.angle='%d', visu.scale='%f', visu.gestion='%d' "
                   " WHERE visu.visuel_id='%d' AND s.access_level<'%d'",
                   Json_get_int( element, "posx" ), Json_get_int( element, "posy" ), Json_get_int( element, "groupe" ),
                   def_color,
                   Json_get_int( element, "angle" ), Json_get_double(element,"scale"), Json_get_int( element," gestion" ),
                   Json_get_int( element, "visuel_id" ), session->access_level );

    g_free(def_color);
 }
/******************************************************************************************************************************/
/* Http_get_syn_save_un_cadran: Enregistre un cadran en base de données                                                       */
/* Entrée: les données JSON recu de la requete HTTP                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_syn_save_un_cadran (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct HTTP_CLIENT_SESSION *session = user_data;
    if ( ! (Json_has_member ( element, "id" ) &&
            Json_has_member ( element, "posx" ) &&
            Json_has_member ( element, "posy" ) &&
            Json_has_member ( element, "groupe" ) &&
            Json_has_member ( element, "angle" )
           ) )
     { return; }

    SQL_Write_new( "UPDATE syns_cadrans "
                   "INNER JOIN dls ON syns_cadrans.dls_id=dls.id "
                   "INNER JOIN syns ON syns.id=dls.syn_id "
                   "SET posx='%d', posy='%d', groupe='%d', angle='%d', scale='%f' "
                   "WHERE syns_cadrans.id='%d' AND syns.access_level<='%d'",
                   Json_get_int( element, "posx" ), Json_get_int( element, "posy" ), Json_get_int( element, "angle" ),
                   Json_get_double( element, "scale" ),
                   Json_get_int( element, "groupe" ), Json_get_int( element, "id" ), session->access_level );
 }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_save ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                              SoupClientContext *client, gpointer user_data )
  { if ( msg->method != SOUP_METHOD_POST )
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( Json_has_member ( request, "visuels" ) )
     { Json_node_foreach_array_element ( request, "visuels", Http_syn_save_un_visuel, session ); }

    if ( Json_has_member ( request, "cadrans" ) )
     { Json_node_foreach_array_element ( request, "cadrans", Http_syn_save_un_cadran, session ); }

    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* Formater_cadran: Formate la structure dédiée cadran pour envoi au client                                                   */
/* Entrée: un cadran                                                                                                          */
/* Sortie: une structure prete à l'envoie                                                                                     */
/******************************************************************************************************************************/
 void Http_Formater_cadran( struct HTTP_CADRAN *cadran )
  { if (!cadran) return;
    if ( ! strcmp ( cadran->classe, "AI" ) )
     { cadran->valeur = Dls_data_get_AI(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
       struct DLS_AI *ai=cadran->dls_data;
       if (!ai)                                                  /* si AI pas trouvée, on remonte le nom du cadran en libellé */
        { cadran->in_range = FALSE; }
       else
        { cadran->in_range = ai->inrange;
          cadran->valeur   = ai->val_ech;
          g_snprintf( cadran->unite, sizeof(cadran->unite), "%s", ai->unite );
        }
     }
    else if ( !strcmp ( cadran->classe, "CH" ) )
     { cadran->in_range = TRUE;
       cadran->valeur = Dls_data_get_CH(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
     }
    else if ( !strcmp ( cadran->classe, "CI" ) )
     { cadran->valeur = Dls_data_get_CI(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
       struct DLS_CI *ci=cadran->dls_data;
       if (!ci)                                                  /* si AI pas trouvée, on remonte le nom du cadran en libellé */
        { cadran->in_range = FALSE; }
       else
        { cadran->in_range = TRUE;
          cadran->valeur *= ci->multi;                                                                    /* Multiplication ! */
          g_snprintf( cadran->unite, sizeof(cadran->unite), "%s", ci->unite );
        }
     }
    else if ( !strcmp ( cadran->classe, "REGISTRE" ) )
     { cadran->valeur = Dls_data_get_REGISTRE(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
       struct DLS_REGISTRE *registre=cadran->dls_data;
       if (!registre)                                      /* si Registre pas trouvée, on remonte le nom du cadran en libellé */
        { cadran->in_range = FALSE; }
       else
        { cadran->in_range = TRUE;
          cadran->valeur = registre->valeur;
          g_snprintf( cadran->unite, sizeof(cadran->unite), "%s", registre->unite );
        }
     }
    else if ( !strcmp ( cadran->classe, "WATCHDOG" ) )
     { Dls_data_get_WATCHDOG(cadran->tech_id, cadran->acronyme, &cadran->dls_data );
       struct DLS_WATCHDOG *wtd=cadran->dls_data;
       if (!wtd)                                      /* si Registre pas trouvée, on remonte le nom du cadran en libellé */
        { cadran->in_range = FALSE; }
       else
        { cadran->in_range = TRUE;
          gint gap = wtd->top - Partage->top;
          if (gap>=0) cadran->valeur = gap/10;
                 else cadran->valeur = 0;
          g_snprintf( cadran->unite, sizeof(cadran->unite), "s" );
        }
     }
    else if ( !strcmp ( cadran->classe, "T" ) )
     { Dls_data_get_tempo ( cadran->tech_id, cadran->acronyme, &cadran->dls_data );
       struct DLS_TEMPO *tempo = cadran->dls_data;
       if (!tempo)
        { cadran->in_range = FALSE; }
       else
        { cadran->in_range = TRUE;
          if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON)                     /* Temporisation Retard en train de compter */
           { cadran->valeur = (tempo->date_on - Partage->top); }
          else if (tempo->status == DLS_TEMPO_NOT_COUNTING)                  /* Tempo ne compte pas: on affiche la consigne */
           { cadran->valeur = tempo->delai_on; }
        }
      }
     else { cadran->in_range = FALSE; }
  }
/******************************************************************************************************************************/
/* Http_abonner_cadran: Abonne un client à un cadran particulier                                                              */
/* Entrées: les données du cadran en Json et la connexion HTTP cliente                                                        */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_abonner_cadran (JsonArray *array, guint index, JsonNode *new_cadran, gpointer user_data)
  { struct HTTP_CLIENT_SESSION *session = user_data;
    gchar *tech_id  = Json_get_string ( new_cadran, "tech_id" );
    gchar *acronyme = Json_get_string ( new_cadran, "acronyme" );
    GSList *liste = session->Liste_bit_cadrans;                                      /* Le cadran est-il déjà dans la liste ? */
    while(liste)
     { struct HTTP_CADRAN *cadran=liste->data;
       if ( !strcasecmp( tech_id, cadran->tech_id ) && !strcasecmp( acronyme, cadran->acronyme ) ) break;
       liste = g_slist_next(liste);
     }
    if (liste) return;                                      /* si le cadran n'est pas trouvé, on l'ajoute à la liste d'abonné */
    struct HTTP_CADRAN *http_cadran = g_try_malloc0(sizeof(struct HTTP_CADRAN));
    if (!http_cadran) return;
    g_snprintf( http_cadran->tech_id,  sizeof(http_cadran->tech_id),  "%s", tech_id  );
    g_snprintf( http_cadran->acronyme, sizeof(http_cadran->acronyme), "%s", acronyme );
    g_snprintf( http_cadran->classe,   sizeof(http_cadran->classe),   "%s", Json_get_string( new_cadran, "classe" ) );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO, "%s: user '%s': Abonné au CADRAN %s:%s", __func__,
              session->username, http_cadran->tech_id, http_cadran->acronyme );
    session->Liste_bit_cadrans = g_slist_prepend( session->Liste_bit_cadrans, http_cadran );
  }
/******************************************************************************************************************************/
/* Http_add_etat_visuel: Ajoute les états de chaque visuels du tableau                                                        */
/* Entrées : Le tableau, l'element a compléter                                                                                */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_add_etat_visuel_to_json ( JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { GSList *dls_visuels = Partage->Dls_data_VISUEL;
    while(dls_visuels)                      /* Parcours tous les visuels et envoie ceux relatifs aux DLS du synoptique chargé */
     { struct DLS_VISUEL *dls_visuel = dls_visuels->data;
       if ( !strcasecmp( dls_visuel->tech_id, Json_get_string(element, "tech_id") ) &&
            !strcasecmp( dls_visuel->acronyme, Json_get_string(element, "acronyme") )
          )
        { Dls_VISUEL_to_json ( element, dls_visuel ); return; }
       dls_visuels = g_slist_next(dls_visuels);
     }
  }
/******************************************************************************************************************************/
/* Http_traiter_syn_show: Fourni une list JSON des elements d'un synoptique                                                   */
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
                                 "SELECT cadran.*, dico.classe, dico.libelle FROM syns_cadrans AS cadran "
                                 "INNER JOIN dls AS dls ON cadran.dls_id=dls.id "
                                 "INNER JOIN syns AS syn ON dls.syn_id=syn.id "
                                 "INNER JOIN dictionnaire AS dico ON (cadran.tech_id=dico.tech_id AND cadran.acronyme=dico.acronyme) "
                                 "WHERE syn.id=%d AND syn.access_level<=%d",
                                 syn_id, session->access_level ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(synoptique);
       return;
     }

    Json_node_foreach_array_element ( synoptique, "cadrans", Http_abonner_cadran, session );
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
                                    "SELECT m.*,v.*,i.*,dls.shortname AS dls_shortname FROM syns_visuels AS v "
                                    "LEFT JOIN mnemos_VISUEL AS m ON v.mnemo_id = m.id "
                                    "LEFT JOIN dls ON dls.tech_id=v.tech_id "
                                    "LEFT JOIN icone AS i ON i.forme=m.forme "
                                    "LEFT JOIN syns AS s ON dls.syn_id=s.id "
                                    "WHERE (s.id='%d' AND s.access_level<=%d AND m.access_level<=%d) OR v.syn_id='%d'",
                                     syn_id, session->access_level, session->access_level, syn_id) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref(synoptique);
          return;
        }
     }
    else
     { if (SQL_Select_to_json_node ( synoptique, "visuels",
                                    "SELECT m.*,v.*,i.*,dls.shortname AS dls_shortname FROM syns_visuels AS v "
                                    "INNER JOIN mnemos_VISUEL AS m ON v.mnemo_id = m.id "
                                    "INNER JOIN dls ON dls.tech_id=v.tech_id "
                                    "INNER JOIN icone AS i ON i.forme=m.forme "
                                    "INNER JOIN syns AS s ON dls.syn_id=s.id "
                                    "WHERE s.id='%d' AND s.access_level<=%d AND m.access_level<=%d",
                                    syn_id, session->access_level, session->access_level) == FALSE)
        { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
          json_node_unref(synoptique);
          return;
        }
     }
/*------------------------------------------------- Envoi l'état de tous les visuels du synoptique ---------------------------*/
    Json_node_foreach_array_element ( synoptique, "visuels", Http_add_etat_visuel_to_json, NULL );

/*-------------------------------------------------- Envoi les horloges de la page -------------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "horloges",
                                 "SELECT DISTINCT horloge.tech_id, dls.name as dls_name FROM mnemos_HORLOGE AS horloge "
                                 "INNER JOIN dls ON dls.tech_id=horloge.tech_id "
                                 "INNER JOIN syns as syn ON dls.syn_id=syn.id "
                                 "WHERE dls.syn_id=%d AND syn.access_level<=%d",
                                 syn_id, session->access_level ) == FALSE)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       json_node_unref(synoptique);
       return;
     }

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