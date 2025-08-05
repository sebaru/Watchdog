/******************************************************************************************************************************/
/* Watchdogd/Mnemonique/Mnemonique.c        Déclaration des fonctions pour la gestion des mnemoniques                         */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                      dim 19 avr 2009 15:15:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemonique.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Mnemo_create_thread_AI: Créer un JSON pour une AI                                                                          */
/* Entrée: la structure THREAD, les parametres de l'AI                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_thread_AI ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage )
  { JsonNode *node = Json_node_create();
    if (!node) return(NULL);
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Json_node_add_string ( node, "classe", "AI" );
    Json_node_add_string ( node, "thread_tech_id", thread_tech_id );
    Json_node_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_node_add_string ( node, "libelle", libelle );
    Json_node_add_string ( node, "unite", unite );
    Json_node_add_int    ( node, "archivage", archivage );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/ai", node );
    if (!api_result || Json_get_int ( api_result, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Could not add AI %s to API", thread_tech_id, thread_acronyme );
     }
    Json_node_unref ( api_result );
    Json_node_add_bool ( node, "need_sync", TRUE );       /* Ajoute un flag first turn pour envoyer au master des le 1er tour */
    Json_array_add_element ( Json_get_array ( module->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_thread_DI: Créé un JSON pour une DI                                                                           */
/* Entrée: la structure THREAD, les parametres de la DI                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_thread_DI ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle )
  { JsonNode *node = Json_node_create();
    if (!node) return(NULL);
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Json_node_add_string ( node, "classe", "DI" );
    Json_node_add_string ( node, "thread_tech_id", thread_tech_id );
    Json_node_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_node_add_string ( node, "libelle", libelle );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/di", node );
    if (!api_result || Json_get_int ( api_result, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Could not add DI %s to API", thread_tech_id, thread_acronyme );
     }
    Json_node_unref ( api_result );
    Json_node_add_bool ( node, "need_sync", TRUE );       /* Ajoute un flag first turn pour envoyer au master des le 1er tour */
    Json_array_add_element ( Json_get_array ( module->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_thread_DO: Créé un JSON pour une DI                                                                           */
/* Entrée: la structure THREAD, les parametres de la DI                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_thread_DO ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gboolean mono )
  { JsonNode *node = Json_node_create();
    if (!node) return(NULL);
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Json_node_add_string ( node, "classe", "DO" );
    Json_node_add_string ( node, "thread_tech_id", thread_tech_id );
    Json_node_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_node_add_string ( node, "libelle", libelle );
    Json_node_add_bool   ( node, "mono", mono );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/do", node );
    if (!api_result || Json_get_int ( api_result, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Could not add DO %s to API", thread_tech_id, thread_acronyme );
     }
    Json_node_unref ( api_result );
    Json_array_add_element ( Json_get_array ( module->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_thread_AO: Créer un JSON pour une AO                                                                          */
/* Entrée: la structure THREAD, les parametres de l'AO                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_thread_AO ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage )
  { JsonNode *node = Json_node_create();
    if (!node) return(NULL);
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Json_node_add_string ( node, "classe", "AO" );
    Json_node_add_string ( node, "thread_tech_id", thread_tech_id );
    Json_node_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_node_add_string ( node, "libelle", libelle );
    Json_node_add_string ( node, "unite", unite );
    Json_node_add_int    ( node, "archivage", archivage );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/ao", node );
    if (!api_result || Json_get_int ( api_result, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Could not add AO %s to API", thread_tech_id, thread_acronyme );
     }
    Json_node_unref ( api_result );
    Json_array_add_element ( Json_get_array ( module->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_thread_HORLOGE: Créé un JSON pour une Horloge                                                                 */
/* Entrée: la structure THREAD, les parametres de l'HORLOGE                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_thread_HORLOGE ( struct THREAD *module, gchar *acronyme, gchar *libelle )
  { JsonNode *node = Json_node_create();
    if (!node) return(NULL);
    gchar *tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Json_node_add_string ( node, "classe", "HORLOGE" );
    Json_node_add_string ( node, "tech_id", tech_id );
    Json_node_add_string ( node, "acronyme", acronyme );
    Json_node_add_string ( node, "libelle", libelle );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/horloge/add", node );
    if (!api_result || Json_get_int ( api_result, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Could not add HORLOGE %s to API", tech_id, acronyme );
     }
    Json_node_unref ( api_result );
    Json_array_add_element ( Json_get_array ( module->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_thread_HORLOGE_tick: Créé un tick sur une horloge donnée                                                      */
/* Entrée: la structure THREAD, les parametres de l'HORLOGE                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mnemo_create_thread_HORLOGE_tick ( struct THREAD *module, JsonNode *bit, gint heure, gint minute )
  { JsonNode *node = Json_node_create();
    if (!node) return;
    Json_node_add_string ( node, "classe", "HORLOGE" );
    Json_node_add_string ( node, "tech_id", Json_get_string ( bit, "tech_id" ) );
    Json_node_add_string ( node, "acronyme", Json_get_string ( bit, "acronyme" ) );
    Json_node_add_int    ( node, "heure", heure );
    Json_node_add_int    ( node, "minute", minute );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/horloge/add/tick", node );
    if (!api_result || Json_get_int ( api_result, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Could not add HORLOGE tick %s:%d:%d to API",
                 Json_get_string ( bit, "tech_id" ), Json_get_string ( bit, "acronyme" ), heure, minute );
     }
    Json_node_unref ( api_result );
    Json_node_unref ( node );
  }
/******************************************************************************************************************************/
/* Mnemo_create_thread_HORLOGE_tick: Créé un tick sur une horloge donnée                                                      */
/* Entrée: la structure THREAD, les parametres de l'HORLOGE                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mnemo_delete_thread_HORLOGE_tick ( struct THREAD *module, JsonNode *bit )
  { if (!bit) return;
    Json_node_add_string ( bit, "classe", "HORLOGE" );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/horloge/del/tick", bit );
    if (!api_result || Json_get_int ( api_result, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Could not DEL HORLOGE tick for '%s'",
                 Json_get_string ( bit, "tech_id" ), Json_get_string ( bit, "acronyme" ) );
     }
    Json_node_unref ( api_result );
  }
/******************************************************************************************************************************/
/* Mnemo_create_thread_WATCHDOG: Créer un JSON pour un WATCHDOG                                                               */
/* Entrée: la structure THREAD, les parametres du WATCHDOG                                                                    */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_thread_WATCHDOG ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle )
  { JsonNode *node = Json_node_create();
    if (!node) return(NULL);
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Json_node_add_string ( node, "classe", "WATCHDOG" );
    Json_node_add_string ( node, "thread_tech_id", thread_tech_id );
    Json_node_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_node_add_string ( node, "libelle", libelle );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/watchdog", node );
    if (!api_result || Json_get_int ( api_result, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Could not add WATCHDOG %s to API", thread_tech_id, thread_acronyme );
     }
    Json_node_unref ( api_result );
    Json_array_add_element ( Json_get_array ( module->IOs, "IOs" ), node );
    return(node);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
