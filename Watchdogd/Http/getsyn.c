/******************************************************************************************************************************/
/* Watchdogd/Http/getsyn.c       Gestion des requests sur l'URI /syn du webservice                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                 06.05.2020 10:57:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getsyn.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
 #include <locale.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"

/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_clic ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data )
  { JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;

    if ( ! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) ) )
     { soup_server_message_set_status (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       Json_node_unref(request);
       return;
     }

    gchar *tech_id  = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *acronyme = Normaliser_chaine ( Json_get_string( request, "acronyme" ) );
    Json_node_unref( request );
    Envoyer_commande_dls_data ( tech_id, acronyme );
    g_free(tech_id);
    g_free(acronyme);
/*************************************************** Envoi au client **********************************************************/
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, NULL );
  }
/******************************************************************************************************************************/
/* Http_add_etat_visuel: Ajoute les états de chaque visuels du tableau                                                        */
/* Entrées : Le tableau, l'element a compléter                                                                                */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_add_etat_visuel_to_json ( JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct DLS_VISUEL *dls_visuel = Dls_data_lookup_VISUEL ( Json_get_string(element, "tech_id"), Json_get_string(element, "acronyme") );
    if (dls_visuel) Dls_VISUEL_to_json ( element, dls_visuel );
  }
/******************************************************************************************************************************/
/* Http_traiter_syn_show: Fourni une list JSON des elements d'un synoptique                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_syn_show ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data )
  { gint syn_id;

    gchar *syn_id_src = g_hash_table_lookup ( query, "syn_id" );
    if (syn_id_src) { syn_id = atoi (syn_id_src); }
                 else syn_id=1;


/*-------------------------------------------------- Test autorisation d'accès -----------------------------------------------*/
    JsonNode *result = Json_node_create();
    if (!result)
     { soup_server_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
       return;
     }

    SQL_Select_to_json_node ( result, NULL, "SELECT access_level,libelle FROM syns WHERE syn_id=%d", syn_id );
    if ( !(Json_has_member ( result, "access_level" ) && Json_has_member ( result, "libelle" )) )
     { Info_new( __func__, Config.log_msrv, LOG_WARNING, "Syn '%d' unknown", syn_id );
       soup_server_message_set_status (msg, SOUP_STATUS_NOT_FOUND, "Syn not found");
       Json_node_unref ( result );
       return;
     }

    Json_node_unref ( result );
/*---------------------------------------------- Envoi les données -----------------------------------------------------------*/
    JsonNode *synoptique = Json_node_create();
    if (!synoptique)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
       return;
     }

    JsonArray *parents = Json_node_add_array ( synoptique, "parent_syns" );
    gint cur_syn_id = syn_id;
    while ( cur_syn_id != 1 )
     { JsonNode *cur_syn = Json_node_create();
       if (!cur_syn) break;
       SQL_Select_to_json_node ( cur_syn, NULL, "SELECT syn_id, parent_id, image, libelle FROM syns WHERE syn_id=%d", cur_syn_id );
       Json_array_add_element ( parents, cur_syn );
       cur_syn_id = Json_get_int ( cur_syn, "parent_id" );
     }

    if (SQL_Select_to_json_node ( synoptique, NULL,
                                 "SELECT * FROM syns WHERE syn_id='%d'",
                                  syn_id) == FALSE)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
       Json_node_unref(synoptique);
       return;
     }
    gint full_syn = Json_get_int ( synoptique, "mode_affichage" );
/*-------------------------------------------------- Envoi les data des synoptiques fils -------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "child_syns",
                                 "SELECT s.* FROM syns AS s INNER JOIN syns as s2 ON s.parent_id=s2.syn_id "
                                 "WHERE s2.syn_id='%d' AND s.syn_id!=1",
                                 syn_id) == FALSE)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
       Json_node_unref(synoptique);
       return;
     }
/*-------------------------------------------------- Envoi les syn_vars ------------------------------------------------------*/
    /*JsonArray *syn_vars = Json_node_add_array ( synoptique, "syn_vars" );*/

/*-------------------------------------------------- Envoi les passerelles ---------------------------------------------------*/
    if (full_syn)
     { if (SQL_Select_to_json_node ( synoptique, "passerelles",
                                    "SELECT pass.*,syn.page,syn.libelle FROM syns_pass as pass "
                                    "INNER JOIN syns as syn ON pass.syn_cible_id=syn.syn_id "
                                    "WHERE pass.syn_id=%d",
                                     syn_id ) == FALSE)
        { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
          Json_node_unref(synoptique);
          return;
        }
     }
/*-------------------------------------------------- Envoi les liens ---------------------------------------------------------*/
    if (full_syn)
     { if (SQL_Select_to_json_node ( synoptique, "liens",
                                     "SELECT lien.* FROM syns_liens AS lien "
                                     "INNER JOIN syns as syn ON lien.syn_id=syn.syn_id "
                                     "WHERE lien.syn_id=%d",
                                     syn_id ) == FALSE)
        { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
          Json_node_unref(synoptique);
          return;
        }
     }
/*-------------------------------------------------- Envoi les rectangles ----------------------------------------------------*/
    if (full_syn)
     { if (SQL_Select_to_json_node ( synoptique, "rectangles",
                                    "SELECT rectangle.* FROM syns_rectangles AS rectangle "
                                    "INNER JOIN syns as syn ON rectangle.syn_id=syn.syn_id "
                                    "WHERE rectangle.syn_id=%d",
                                    syn_id ) == FALSE)
        { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
          Json_node_unref(synoptique);
          return;
        }
     }
/*-------------------------------------------------- Envoi les commennts -----------------------------------------------------*/
    if (full_syn)
     { if (SQL_Select_to_json_node ( synoptique, "comments",
                                    "SELECT comment.* FROM syns_comments AS comment "
                                    "INNER JOIN syns as syn ON comment.syn_id=syn.syn_id "
                                    "WHERE comment.syn_id=%d",
                                    syn_id ) == FALSE)
        { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
          Json_node_unref(synoptique);
          return;
        }
     }
/*-------------------------------------------------- Envoi les cameras -------------------------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "cameras",
                                 "SELECT cam.*,src.location,src.libelle FROM syns_camerasup AS cam "
                                 "INNER JOIN cameras AS src ON cam.camera_src_id=src.id "
                                 "INNER JOIN syns as syn ON cam.syn_id=syn.syn_id "
                                 "WHERE cam.syn_id=%d",
                                 syn_id ) == FALSE)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
       Json_node_unref(synoptique);
       return;
     }

/*-------------------------------------------------- Envoi les cadrans de la page --------------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "cadrans",
                                 "SELECT cadran.*, dico.classe, dico.libelle FROM syns_cadrans AS cadran "
                                 "INNER JOIN dls AS dls ON cadran.dls_id=dls.dls_id "
                                 "INNER JOIN syns AS syn ON dls.syn_id=syn.syn_id "
                                 "INNER JOIN dictionnaire AS dico ON (cadran.tech_id=dico.tech_id AND cadran.acronyme=dico.acronyme) "
                                 "WHERE syn.syn_id=%d",
                                 syn_id ) == FALSE)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
       Json_node_unref(synoptique);
       return;
     }

/*-------------------------------------------------- Envoi les tableaux de la page -------------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "tableaux",
                                 "SELECT tableau.* FROM tableau "
                                 "INNER JOIN syns as syn ON tableau.syn_id=syn.syn_id "
                                 "WHERE tableau.syn_id=%d",
                                 syn_id ) == FALSE)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
       Json_node_unref(synoptique);
       return;
     }
/*-------------------------------------------------- Envoi les tableaux_map de la page ---------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "tableaux_map",
                                 "SELECT tableau_map.* FROM tableau_map "
                                 "INNER JOIN tableau ON tableau_map.tableau_id=tableau.tableau_id "
                                 "INNER JOIN syns as syn ON tableau.syn_id=syn.syn_id "
                                 "WHERE tableau.syn_id=%d",
                                 syn_id ) == FALSE)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
       Json_node_unref(synoptique);
       return;
     }

/*-------------------------------------------------- Envoi les visuels de la page --------------------------------------------*/
    if (full_syn)
     { if (SQL_Select_to_json_node ( synoptique, "visuels",
                                    "SELECT v.*,m.*,i.*,dls.shortname AS dls_shortname, "
                                    "  IF(i.layer IS NULL, 200, i.layer) AS layer,"
                                    "  IF(m.tech_id IS NULL, v.tech_id, m.tech_id) AS tech_id,"
                                    "  IF(m.acronyme IS NULL, v.acronyme, m.acronyme) AS acronyme, "
                                    "  IF(m.color IS NULL, v.color, m.color) AS color "
                                    "FROM syns_visuels AS v "
                                    "LEFT JOIN mnemos_VISUEL AS m ON v.mnemo_id = m.id "
                                    "LEFT JOIN dls ON dls.dls_id=v.dls_id "
                                    "LEFT JOIN icone AS i ON i.forme=m.forme "
                                    "LEFT JOIN syns AS s ON dls.syn_id=s.syn_id "
                                    "WHERE (s.syn_id='%d' AND v.syn_id='%d' "
                                    "ORDER BY layer",
                                     syn_id, syn_id) == FALSE)
        { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
          Json_node_unref(synoptique);
          return;
        }
     }
    else
     { if (SQL_Select_to_json_node ( synoptique, "visuels",
                                    "SELECT v.*,m.*,i.*,dls.tech_id AS dls_tech_id, dls.shortname AS dls_shortname, dls_owner.shortname AS dls_owner_shortname "
                                    "FROM syns_visuels AS v "
                                    "INNER JOIN mnemos_VISUEL AS m ON v.mnemo_id = m.id "
                                    "INNER JOIN dls ON dls.dls_id=v.dls_id "
                                    "INNER JOIN icone AS i ON i.forme=m.forme "
                                    "INNER JOIN syns AS s ON dls.syn_id=s.syn_id "
                                    "INNER JOIN dls AS dls_owner ON dls_owner.tech_id=m.tech_id "
                                    "WHERE s.syn_id='%d' "
                                    "ORDER BY layer",
                                    syn_id) == FALSE)
        { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
          Json_node_unref(synoptique);
          return;
        }
     }
/*------------------------------------------------- Envoi l'état de tous les visuels du synoptique ---------------------------*/
    Json_node_foreach_array_element ( synoptique, "visuels", Http_add_etat_visuel_to_json, NULL );

/*-------------------------------------------------- Envoi les horloges de la page -------------------------------------------*/
    if (SQL_Select_to_json_node ( synoptique, "horloges",
                                 "SELECT DISTINCT horloge.tech_id, dls.name as dls_name FROM mnemos_HORLOGE AS horloge "
                                 "INNER JOIN dls ON dls.tech_id=horloge.tech_id "
                                 "INNER JOIN syns as syn ON dls.syn_id=syn.syn_id "
                                 "WHERE dls.syn_id=%d",
                                 syn_id ) == FALSE)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, NULL, NULL );
       Json_node_unref(synoptique);
       return;
     }

    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, synoptique );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
