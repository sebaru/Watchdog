/******************************************************************************************************************************/
/* Watchdogd/mqtt_local.c        Fonctions communes de gestion des requetes MQTT locales                                      */
/* Projet Abls-Habitat version 4.5       Gestion d'habitat                                                17.08.2024 12:31:26 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mqtt_local.c
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

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* MQTT_local_on_connect_CB: appelé par la librairie quand le broker est connecté                                             */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_local_on_connect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { Info_new( __func__, Config.log_bus, LOG_NOTICE, "Connected with return code %d: %s",
              return_code, mosquitto_connack_string( return_code ) );
    if (return_code == 0)
     { if (Config.instance_is_master)
        { MQTT_Subscribe ( Partage->MQTT_local_session, "SET_DI/#" );
          MQTT_Subscribe ( Partage->MQTT_local_session, "SET_DI_PULSE/#" );
          MQTT_Subscribe ( Partage->MQTT_local_session, "SET_AI/#" );
          MQTT_Subscribe ( Partage->MQTT_local_session, "SET_WATCHDOG/#" );
        }
       MQTT_Subscribe ( Partage->MQTT_local_session, "SET_BUS/%s", g_get_host_name() );
     }
  }
/******************************************************************************************************************************/
/* MQTT_local_on_disconnect_CB: appelé par la librairie quand le broker est déconnecté                                        */
/* Entrée: les parametres d'affichage de log de la librairie                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_local_on_disconnect_CB( struct mosquitto *mosq, void *obj, int return_code )
  { Info_new( __func__, Config.log_bus, LOG_NOTICE, "Disconnected with return code %d: %s",
              return_code, mosquitto_connack_string( return_code ) );
  }
/******************************************************************************************************************************/
/* MQTT_local_on_message_CB: Appelé par mosquitto lorsque l'on recoit un message MQTT de la part du MQTT local                */
/* Entrée: les parametres MQTT                                                                                                */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MQTT_local_on_message_CB ( struct mosquitto *MQTT_session, void *obj, const struct mosquitto_message *msg )
  { gchar **tokens = g_strsplit ( msg->topic, "/", 3 );
    if (!tokens)    { Info_new( __func__, Config.log_bus, LOG_ERR, "Tokens is null" ); return; }
    if (!tokens[0]) { Info_new( __func__, Config.log_bus, LOG_ERR, "Token[0] is null" ); goto end; }
    if (!tokens[1]) { Info_new( __func__, Config.log_bus, LOG_ERR, "Token[0] is null" ); goto end; }

    JsonNode *request = Json_get_from_string ( msg->payload );
    if (!request)
     { Info_new( __func__, Config.log_bus, LOG_WARNING, "MQTT Message from LOCAL dropped: not JSON or no payload" );
       goto end;
     }

    gchar *topic = tokens[0];
    if ( !strcmp ( topic, "SET_AI" ) )
     { if (!tokens[2]) goto end; /* L'acronyme */
       Json_node_add_string ( request, "thread_tech_id", tokens[1] );
       Json_node_add_string ( request, "thread_acronyme", tokens[2] );
       Dls_data_set_AI_from_thread_ai ( request );
     }
    else if ( !strcmp ( topic, "SET_DI" ) )
     { if (!tokens[2]) goto end; /* L'acronyme */
       Json_node_add_string ( request, "thread_tech_id", tokens[1] );
       Json_node_add_string ( request, "thread_acronyme", tokens[2] );
       Dls_data_set_DI_from_thread_di ( request );
     }
    else if ( !strcmp ( topic, "SET_WATCHDOG" ) )
     { if (!tokens[2]) goto end; /* L'acronyme */
       Json_node_add_string ( request, "thread_tech_id", tokens[1] );
       Json_node_add_string ( request, "thread_acronyme", tokens[2] );
       Dls_data_set_WATCHDOG_from_thread_watchdog ( request );
     }
    else if ( !strcmp ( topic, "SET_DI_PULSE" ) )
     { if (!tokens[2]) goto end; /* L'acronyme */
       gchar *thread_tech_id = Json_get_string ( request, "thread_tech_id" );
       Info_new( __func__, Config.log_bus, LOG_INFO, "SET_DI_PULSE from '%s': '%s:%s'=PULSE", thread_tech_id, tokens[1], tokens[2] );
       struct DLS_DI *bit = Dls_data_lookup_DI ( tokens[1], tokens[2] );
       Dls_data_set_DI_pulse ( NULL, bit );
     }
    else if ( !strcmp ( topic, "SET_BUS" ) )
     { gchar *commande = Json_get_string ( request, "commande" );
       if (commande)
        { Info_new( __func__, Config.log_bus, LOG_NOTICE, "SET_BUS: Executing '%s'", commande );
          system( commande );
        }
       else Info_new( __func__, Config.log_bus, LOG_ERR, "SET_BUS: 'commande' is missing" );
     }
    else Info_new( __func__, Config.log_bus, LOG_ERR, "tag inconnu: %s sur topic %s", topic, msg->topic );
    Json_node_unref ( request );

end:
    g_strfreev( tokens );                                                                      /* Libération des tokens topic */
  }
/******************************************************************************************************************************/
/* MQTT_Start_MQTT_LOCAL: Appelé pour démarrer les interactions MQTT du master avec les slaves                                */
/* Entrée: Néant                                                                                                              */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean MQTT_Start_MQTT_LOCAL ( void )
  { gint retour;
    gchar *agent_uuid = Json_get_string ( Config.config, "agent_uuid" );

    Partage->MQTT_local_session = mosquitto_new( agent_uuid, TRUE, NULL );
    if (!Partage->MQTT_local_session)
     { Info_new( __func__, Config.log_bus, LOG_ERR, "MQTT_local session error." ); return(FALSE); }

    mosquitto_log_callback_set        ( Partage->MQTT_local_session, MQTT_on_log_CB );
    mosquitto_connect_callback_set    ( Partage->MQTT_local_session, MQTT_local_on_connect_CB );
    mosquitto_disconnect_callback_set ( Partage->MQTT_local_session, MQTT_local_on_disconnect_CB );
    mosquitto_message_callback_set    ( Partage->MQTT_local_session, MQTT_local_on_message_CB );
    mosquitto_username_pw_set         ( Partage->MQTT_local_session, Json_get_string ( Config.config, "agent_uuid" ), NULL );

    /*if (Config.mqtt_over_ssl)
     { mosquitto_tls_set( Partage->MQTT_local_session, NULL, "/etc/ssl/certs", NULL, NULL, NULL ); }*/

    retour = mosquitto_connect( Partage->MQTT_local_session, Config.master_hostname, 1883, 60 );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, Config.log_bus, LOG_ERR, "MQTT_local connection to '%s:1883' error: %s",
                 Config.master_hostname, mosquitto_strerror ( retour ) );
       return(FALSE);
     }

    retour = mosquitto_loop_start( Partage->MQTT_local_session );
    if ( retour != MOSQ_ERR_SUCCESS )
     { Info_new( __func__, Config.log_bus, LOG_ERR, "MQTT_local loop not started: %s", mosquitto_strerror ( retour ) );
       return(FALSE);
     }

    Info_new( __func__, Config.log_bus, LOG_NOTICE, "MQTT_local loop started" );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* MQTT_Stop_MQTT_API: Appelé pour stopper les interactions MQTT du master avec l'API                                         */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Stop_MQTT_LOCAL ( void )
  { mosquitto_disconnect( Partage->MQTT_local_session );
    mosquitto_loop_stop( Partage->MQTT_local_session, FALSE );
    mosquitto_destroy( Partage->MQTT_local_session );
    Partage->MQTT_local_session = NULL;
  }
/******************************************************************************************************************************/
/* MQTT_Send_to_topic: Envoie un node sur un topic MQTT via le broker                                                     */
/* Entrée: la structure MQTT, le topic, le node, le flag de retenu                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_to_topic ( struct mosquitto *mqtt_session, JsonNode *node, gboolean retain, gchar *format, ... )
  { va_list ap;
    if (! (mqtt_session && format) ) return;

    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound ( format, ap );
    va_end ( ap );
    gchar *topic = g_try_malloc(taille+1);
    if (!topic)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Memory Error for '%s'", format );
       return;
     }

    va_start( ap, format );
    g_vsnprintf ( topic, taille, format, ap );
    va_end ( ap );

    gboolean free_node=FALSE;
    if (!node) { node = Json_node_create(); free_node = TRUE; }
    gchar *buffer = Json_node_to_string ( node );
    if (buffer)
     { mosquitto_publish( mqtt_session, NULL, topic, strlen(buffer), buffer, 2, retain );
       g_free(buffer);
     }
    if (free_node) Json_node_unref(node);
    g_free(topic);
  }
/******************************************************************************************************************************/
/* Mqtt_Send_AI: Envoie le bit AI au master                                                                                   */
/* Entrée: la structure MQTT, l'AI, la valeur et le range                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_AI ( struct THREAD *module, JsonNode *thread_ai, gdouble valeur, gboolean in_range )
  { if (! (module && thread_ai)) return;
    gdouble  old_valeur   = Json_get_double ( thread_ai, "valeur" );
    gboolean old_in_range = Json_get_bool   ( thread_ai, "in_range" );
    gboolean need_sync    = Json_get_bool   ( thread_ai, "need_sync" );

    if ( need_sync || old_valeur != valeur || old_in_range != in_range )
     { Json_node_add_double ( thread_ai, "valeur", valeur );
       Json_node_add_bool   ( thread_ai, "in_range", in_range );
       Json_node_add_bool   ( thread_ai, "need_sync", FALSE );
       gchar *thread_tech_id = Json_get_string ( thread_ai, "thread_tech_id" );
       gchar *thread_acronyme = Json_get_string ( thread_ai, "thread_acronyme" );
       Info_new( __func__, module->Thread_debug, LOG_DEBUG, "'%s:%s' = %f (in_range=%d)", thread_tech_id, thread_acronyme, valeur, in_range );
       JsonNode *RootNode = Json_node_create();
       if (!RootNode) return;
       Json_node_add_double ( RootNode, "valeur", valeur );
       Json_node_add_bool   ( RootNode, "in_range", in_range );
       MQTT_Send_to_topic ( module->MQTT_session, RootNode, TRUE, "SET_AI/%s/%s", thread_tech_id, thread_acronyme );
       Json_node_unref ( RootNode );
     }
  }
/******************************************************************************************************************************/
/* MQTT_Send_DI: Envoie le bit DI au master                                                                                   */
/* Entrée: la structure MQTT, la DI, la valeur                                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_DI ( struct THREAD *module, JsonNode *thread_di, gboolean etat )
  { if (! (module && thread_di)) return;
    gboolean old_etat   = Json_get_bool ( thread_di, "etat" );
    gboolean need_sync  = Json_get_bool ( thread_di, "need_sync" );

    if ( need_sync || (old_etat != etat) )
     { Json_node_add_bool ( thread_di, "etat", (etat ? TRUE : FALSE) );
       Json_node_add_bool ( thread_di, "need_sync", FALSE );
       gchar *thread_tech_id = Json_get_string ( thread_di, "thread_tech_id" );
       gchar *thread_acronyme = Json_get_string ( thread_di, "thread_acronyme" );
       Info_new( __func__, module->Thread_debug, LOG_DEBUG, "'%s:%s' = %d", thread_tech_id, thread_acronyme, etat );
       JsonNode *RootNode = Json_node_create();
       if (!RootNode) return;
       Json_node_add_bool ( RootNode, "etat", etat );
       MQTT_Send_to_topic ( module->MQTT_session, RootNode, TRUE, "SET_DI/%s/%s", thread_tech_id, thread_acronyme );
       Json_node_unref ( RootNode );
     }
  }
/******************************************************************************************************************************/
/* MQTT_Send_DI: Envoie le bit DI au master, au format pulse                                                                  */
/* Entrée: la structure MQTT, la DI, la valeur                                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_DI_pulse ( struct THREAD *module, gchar *tech_id, gchar *acronyme )
  { if (! (module && tech_id && acronyme)) return;
    JsonNode *thread_di = Json_node_create();
    if (!thread_di) return;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Json_node_add_string ( thread_di, "thread_tech_id", thread_tech_id );
    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "'%s:%s' = PULSE", tech_id, acronyme );
    MQTT_Send_to_topic ( module->MQTT_session, thread_di, FALSE, "SET_DI_PULSE/%s/%s", tech_id, acronyme );
    Json_node_unref ( thread_di );
  }
/******************************************************************************************************************************/
/* MQTT_Send_WATCHDOG: Envoie le WATCHDOG au master                                                                           */
/* Entrée: la structure MQTT, le watchdog, la consigne                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MQTT_Send_WATCHDOG ( struct THREAD *module, gchar *thread_acronyme, gint consigne )
  { if (! (module && thread_acronyme)) return;
    JsonNode *thread_watchdog = Json_node_create ();
    if(!thread_watchdog) return;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Json_node_add_int    ( thread_watchdog, "consigne", consigne );

    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "'%s:%s' = %d", thread_tech_id, thread_acronyme, consigne );
    MQTT_Send_to_topic ( module->MQTT_session, thread_watchdog, TRUE, "SET_WATCHDOG/%s/%s", thread_tech_id, thread_acronyme );
    Json_node_unref ( thread_watchdog );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
