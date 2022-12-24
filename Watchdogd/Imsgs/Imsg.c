/******************************************************************************************************************************/
/* Watchdogd/Imsg/Imsg.c  Gestion des Instant Messaging IMSG Watchdog 2.0                                                     */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    20.02.2018 17:58:31 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Imsg.c
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

 #include <sys/prctl.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Imsg.h"

/******************************************************************************************************************************/
/* Imsgs_Envoi_message_to : Envoi un message a un contact xmpp                                                                */
/* Entrée: le nom du destinataire et le message                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_Envoi_message_to ( struct THREAD *module, const gchar *dest, gchar *message )
  { struct IMSGS_VARS *vars = module->vars;
    xmpp_stanza_t *stanza = xmpp_message_new ( vars->ctx, "normal", dest, NULL );
    xmpp_message_set_body ( stanza, message );
    xmpp_send ( vars->conn, stanza ) ;
    xmpp_stanza_release ( stanza );
    Info_new( Config.log, module->Thread_debug, LOG_INFO, "%s: Send '%s' to '%s'", __func__, message, dest );
  }
/******************************************************************************************************************************/
/* Imsgs_Envoi_message_to_all_available : Envoi un message aux contacts disponibles                                           */
/* Entrée: le message                                                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_Envoi_message_to_all_available ( struct THREAD *module, gchar *message )
  {
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

/********************************************* Chargement des informations en bases *******************************************/
    JsonNode *UsersNode = Http_Get_from_global_API ( "/run/users/wanna_be_notified", NULL );
    if (!UsersNode || Json_get_int ( UsersNode, "api_status" ) != 200)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: Could not get USERS from API", __func__, thread_tech_id );
       return;
     }

    GList *recipients = json_array_get_elements ( Json_get_array ( UsersNode, "recipients" ) );
    while(recipients)
     { JsonNode *user = recipients->data;
       gchar *xmpp = Json_get_string ( user, "xmpp" );
       if (!xmpp)
        { Info_new( Config.log, module->Thread_debug, LOG_ERR,
                    "%s: %s: Warning: User %s does not have an XMPP id", __func__, thread_tech_id, Json_get_string ( user, "email" ) );
        }
       else if (!strlen(xmpp))
        { Info_new( Config.log, module->Thread_debug, LOG_ERR,
                    "%s: %s: Warning: User %s has an empty XMPP id", __func__, thread_tech_id, Json_get_string ( user, "email" ) );
        }
       else Imsgs_Envoi_message_to ( module, xmpp, message );
       recipients = g_list_next(recipients);
     }
    g_list_free(recipients);
    Json_node_unref ( UsersNode );
  }
/******************************************************************************************************************************/
/* Imsgs_handle_message_CB : CB appellé lorsque l'on recoit un message xmpp                                                   */
/* Entrée : Le Handler, la connexion, le message                                                                              */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static int Imsgs_handle_message_CB (xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata)
  { struct THREAD *module = userdata;
    struct IMSGS_VARS *vars = module->vars;

    gchar *thread_tech_id  = Json_get_string ( module->config, "thread_tech_id" );

    const gchar *from = xmpp_stanza_get_attribute ( stanza, "from" );
    if (!from)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': Error: from is NULL", __func__, thread_tech_id );
       return(1);
     }

    const gchar *type = xmpp_stanza_get_type ( stanza );
    if (!strcmp ( type, "error" ))
     { xmpp_stanza_t *error          = xmpp_stanza_get_child_by_name(stanza, "error");
       const gchar   *error_type     = xmpp_stanza_get_attribute ( error, "type" );
       xmpp_stanza_t *condition      = xmpp_stanza_get_children ( error );
       const gchar   *condition_name = xmpp_stanza_get_name ( condition );
       Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': From '%s' -> Stanza Error '%s'->'%s'",
                 __func__, thread_tech_id, from, error_type, condition_name );
       return(1);
     }

    gchar *message    = xmpp_message_get_body ( stanza );
    if (!message)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': From '%s' -> Error: message is NULL", __func__, thread_tech_id, from );
       gchar *buf; size_t buflen;
       xmpp_stanza_to_text ( stanza, &buf, &buflen );
       Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': From '%s' -> Received Stanza '%s'", __func__, thread_tech_id, from, buf );
       xmpp_free(vars->ctx, buf);
       return(1);
     }

    Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: '%s': From '%s' -> '%s'", __func__, thread_tech_id, from, message );

    JsonNode *RootNode = Json_node_create();
    if ( RootNode == NULL )
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: Memory Error for '%s'", __func__, thread_tech_id, from );
       goto end_message;
     }

    gchar hostonly[128];
    g_snprintf( hostonly, sizeof(hostonly), "%s", from ); /* Remove all char after '/' */
    gchar *ptr = strstr( hostonly, "/" );
    if (ptr) *ptr=0;

    Json_node_add_string ( RootNode, "xmpp", hostonly );

    JsonNode *UserNode = Http_Post_to_global_API ( "/run/user/can_send_txt_cde", RootNode );
    Json_node_unref ( RootNode );
    if (!UserNode || Json_get_int ( UserNode, "api_status" ) != 200)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: Could not get USER from API for '%s'", __func__, thread_tech_id, from );
       goto end_user;
     }

    if ( !Json_has_member ( UserNode, "email" ) )
     { Info_new( Config.log, module->Thread_debug, LOG_ERR,
                "%s: %s: %s is not an known user. Dropping command '%s'...", __func__, thread_tech_id, from, message );
       goto end_user;
     }

    if ( !Json_has_member ( UserNode, "can_send_txt_cde" ) || Json_get_bool ( UserNode, "can_send_txt_cde" ) == FALSE )
     { Info_new( Config.log, module->Thread_debug, LOG_WARNING,
                "%s: %s: %s ('%s') is not allowed to send txt_cde. Dropping command '%s'...", __func__, thread_tech_id,
                from, Json_get_string ( UserNode, "email" ), message );
       goto end_user;
     }

    if ( ! strcasecmp( message, "ping" ) )                                                             /* Interfacage de test */
     { Imsgs_Envoi_message_to( module, from, "Pong !" );
       goto end_user;
     }

    RootNode = Json_node_create();
    if ( RootNode == NULL )
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: MapNode Error for '%s'", __func__, thread_tech_id, from );
       goto end_user;
     }
    Json_node_add_string ( RootNode, "thread_tech_id", "_COMMAND_TEXT" );
    Json_node_add_string ( RootNode, "thread_acronyme", message );

    JsonNode *MapNode = Http_Post_to_global_API ( "/run/mapping/search_txt", RootNode );
    Json_node_unref ( RootNode );
    if (!MapNode || Json_get_int ( MapNode, "api_status" ) != 200)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: Could not get USER '%s' from API for '%s'", __func__, thread_tech_id, from, message );
       goto end_map;
     }

    if ( Json_has_member ( MapNode, "nbr_results" ) == FALSE )
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': Error searching Database for user '%s', '%s'", __func__, thread_tech_id, from, message );
       Imsgs_Envoi_message_to( module, from,"Error searching Database .. Sorry .." );
       goto end_map;
     }

    gint nbr_results = Json_get_int ( MapNode, "nbr_results" );
    if ( nbr_results == 0 )
     { Imsgs_Envoi_message_to( module, from, "Je n'ai pas trouvé, désolé." ); }
    else
     { if ( nbr_results > 1 )                                               /* Si trop d'enregistrements, demande de préciser */
        { Imsgs_Envoi_message_to( module, from, "Aîe, plusieurs choix sont possibles ... :" ); }

       GList *Results = json_array_get_elements ( Json_get_array ( MapNode, "results" ) );
       if ( nbr_results > 1 )
        { GList *results = Results;
          while(results)
           { JsonNode *element = results->data;
             gchar *thread_acronyme = Json_get_string ( element, "thread_acronyme" );
             gchar *tech_id         = Json_get_string ( element, "tech_id" );
             gchar *acronyme        = Json_get_string ( element, "acronyme" );
             gchar *libelle         = Json_get_string ( element, "libelle" );
             Info_new( Config.log, module->Thread_debug, LOG_INFO, "%s: '%s': From '%s' map found for '%s' -> '%s:%s' - %s", __func__,
                       thread_tech_id, from, thread_acronyme, tech_id, acronyme, libelle );
             Imsgs_Envoi_message_to ( module, from, thread_acronyme );                          /* Envoi des différents choix */
             results = g_list_next(results);
           }
        }
       else if ( nbr_results == 1)
        { JsonNode *element = Results->data;
          gchar *thread_acronyme = Json_get_string ( element, "thread_acronyme" );
          gchar *tech_id         = Json_get_string ( element, "tech_id" );
          gchar *acronyme        = Json_get_string ( element, "acronyme" );
          gchar *libelle         = Json_get_string ( element, "libelle" );
          Info_new( Config.log, module->Thread_debug, LOG_INFO, "%s: '%s': From '%s' map found for '%s' (%s)-> '%s:%s' - %s", __func__,
                    thread_tech_id, from, Json_get_string( UserNode, "email" ), thread_acronyme, tech_id, acronyme, libelle );
          Http_Post_to_local_BUS_CDE ( module, tech_id, acronyme );
          gchar chaine[256];
          g_snprintf ( chaine, sizeof(chaine), "'%s' fait.", message );
          Imsgs_Envoi_message_to ( module, from, chaine );
        }
       g_list_free(Results);
     }

end_map:
    Json_node_unref ( MapNode );
end_user:
    Json_node_unref ( UserNode );
end_message:
    xmpp_free(vars->ctx, message);
    return(1);
  }
/******************************************************************************************************************************/
/* Imsgs_set_presence: Défini le statut présenté aux partenaires                                                              */
/* Entrée : le statut au format chaine de caratères                                                                           */
/******************************************************************************************************************************/
  static void Imsgs_set_presence ( struct THREAD *module, const char *status_to_send )
  { struct IMSGS_VARS *vars = module->vars;
    gchar *thread_tech_id  = Json_get_string ( module->config, "thread_tech_id" );

    xmpp_stanza_t *pres = xmpp_presence_new(vars->ctx);

    xmpp_stanza_t *show = xmpp_stanza_new(vars->ctx);
    xmpp_stanza_set_name  ( show,"show" ) ;
    xmpp_stanza_t *show_text = xmpp_stanza_new(vars->ctx);
    xmpp_stanza_set_text  ( show_text, "chat" );
    xmpp_stanza_add_child ( show, show_text ) ;
    xmpp_stanza_add_child ( pres, show ) ;

    xmpp_stanza_t *status = xmpp_stanza_new(vars->ctx);
    xmpp_stanza_set_name  ( status,"status" ) ;
    xmpp_stanza_t *status_text = xmpp_stanza_new(vars->ctx);
    xmpp_stanza_set_text  ( status_text, status_to_send );
    xmpp_stanza_add_child ( status, status_text ) ;
    xmpp_stanza_add_child ( pres, status ) ;

    gchar *buf; size_t buflen;
    xmpp_stanza_to_text ( pres, &buf, &buflen );
    Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: '%s': '%s'", __func__, thread_tech_id, buf );
    xmpp_free(vars->ctx, buf);

    xmpp_send(vars->conn, pres);
    xmpp_stanza_release(pres);
  }
/******************************************************************************************************************************/
/* Imsgs_handle_presence_CB: appellé par libstrophe lors d'un changement de statut d'un partenaire                            */
/* Entrée : les infos de la librairie                                                                                         */
/******************************************************************************************************************************/
 static int Imsgs_handle_presence_CB ( xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata )
  { struct THREAD *module = userdata;
    struct IMSGS_VARS *vars = module->vars;
    const char *type, *from;
    gchar *thread_tech_id  = Json_get_string ( module->config, "thread_tech_id" );

    type = xmpp_stanza_get_type ( stanza );
    from = xmpp_stanza_get_from ( stanza );

    gchar *buf; size_t buflen;
    xmpp_stanza_to_text ( stanza, &buf, &buflen );
    Info_new( Config.log, module->Thread_debug, LOG_INFO, "%s: '%s': From '%s' -> Received Stanza '%s'", __func__, thread_tech_id, from, buf );
    xmpp_free(vars->ctx, buf);


    if (type && !strcmp(type,"subscribe"))                             /* Demande de souscription de la part d'un utilisateur */
     { xmpp_stanza_t *pres;
       pres = xmpp_presence_new(vars->ctx);
       xmpp_stanza_set_to  ( pres, from );
       xmpp_stanza_set_type( pres, "subscribed" );
       xmpp_send(vars->conn, pres);
       xmpp_stanza_release(pres);
       pres = xmpp_presence_new(vars->ctx);
       xmpp_stanza_set_to  ( pres, from );
       xmpp_stanza_set_type( pres, "subscribe" );
       xmpp_send(vars->conn, pres);
       xmpp_stanza_release(pres);
       Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: '%s': Sending 'subscribe' to '%s'", __func__, thread_tech_id, from );
     }

    return(1);
  }
/******************************************************************************************************************************/
/* Imsgs_connexion_CB: appellé par libstrophe lors d'un changement de statut de la connexion                                  */
/* Entrée : les infos de la librairie                                                                                         */
/******************************************************************************************************************************/
 static void Imsgs_connexion_CB ( xmpp_conn_t *const conn, const xmpp_conn_event_t status, const int error,
                                  xmpp_stream_error_t *const stream_error, void *const userdata )
  { struct THREAD *module = userdata;
    struct IMSGS_VARS *vars = module->vars;
    if (status == XMPP_CONN_CONNECT)
     {
       Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: '%s': Account connected and %s secure",
                 __func__, Json_get_string ( module->config, "jabberid" ), (xmpp_conn_is_secured (conn) ? "IS" : "IS NOT") );
       xmpp_handler_add ( vars->conn, Imsgs_handle_message_CB,  NULL, "message",  NULL, module );
       xmpp_handler_add ( vars->conn, Imsgs_handle_presence_CB, NULL, "presence", NULL, module );
       /*xmpp_handler_add   ( vars->conn, Imsgs_test, NULL, NULL, NULL, NULL );*/

       Imsgs_set_presence( module, "A votre écoute !" );
       Imsgs_Envoi_message_to_all_available ( module, "Agent démarré. A l'écoute !" );
     }
    else
     { Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: '%s': Account disconnected",
                 __func__, Json_get_string ( module->config, "jabberid" ) );
       vars->signed_off = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct IMSGS_VARS) );
    struct IMSGS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *jabber_id = Json_get_string ( module->config, "jabberid" );

    if ( ! (thread_tech_id && jabber_id) )
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: No thread_tech_id Or Jabber_id. Stopping.", __func__ );
       goto end;
     }

reconnect:
    vars->signed_off = FALSE;
    vars->ctx  = xmpp_ctx_new(NULL, xmpp_get_default_logger(XMPP_LEVEL_INFO));
    if (!vars->ctx)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: Ctx Init failed", __func__ ); goto end; }

    vars->conn = xmpp_conn_new(vars->ctx);
    if (!vars->conn)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: Connection New failed", __func__ ); goto end; }

    xmpp_conn_set_keepalive(vars->conn, 60, 1);
    xmpp_conn_set_jid (vars->conn, jabber_id );
    xmpp_conn_set_pass(vars->conn, Json_get_string ( module->config, "password" ));

    gint retour = xmpp_connect_client ( vars->conn, NULL, 0, Imsgs_connexion_CB, module );
    if ( retour != XMPP_EOK)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR,
                  "%s: '%s': Connexion failed with error %d", __func__, jabber_id, retour );
       vars->signed_off = TRUE;
     }
    else
     { Info_new( Config.log, module->Thread_debug, LOG_INFO, "%s: '%s': Connexion in progress.", __func__, jabber_id );
       Thread_send_comm_to_master ( module, TRUE );
     }

    while(module->Thread_run == TRUE && vars->signed_off == FALSE)                           /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
       xmpp_run_once ( vars->ctx, 500 ); /* En milliseconde */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->WS_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->WS_messages->data;
          module->WS_messages = g_slist_remove ( module->WS_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *tag = Json_get_string ( request, "tag" );

          if ( !strcasecmp( tag, "DLS_HISTO" ) && Json_get_bool ( request, "alive" ) == TRUE )
           { Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: '%s': Sending msg '%s:%s' (%s)", __func__,
                       jabber_id,
                       Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ),
                       Json_get_string ( request, "libelle" ) );
             gchar chaine[256];
             g_snprintf( chaine, sizeof(chaine), "%s: %s", Json_get_string ( request, "dls_shortname" ), Json_get_string ( request, "libelle" ) );
             Imsgs_Envoi_message_to_all_available ( module, chaine );
           }
          else if ( !strcasecmp( tag, "test" ) ) Imsgs_Envoi_message_to_all_available ( module, "Test OK" );
          else Info_new( Config.log, module->Thread_debug, LOG_DEBUG, "%s: '%s': tag '%s' not for this thread", __func__, thread_tech_id, tag );
          Json_node_unref(request);
        }
     }                                                                                         /* Fin du while partage->arret */

end:
    if (vars->conn)
     { xmpp_disconnect(vars->conn);
       Info_new( Config.log, module->Thread_debug, LOG_DEBUG, "%s: '%s': Disconnect OK", __func__, jabber_id );
       xmpp_conn_release(vars->conn);
       Info_new( Config.log, module->Thread_debug, LOG_DEBUG, "%s: '%s': Connection Release OK", __func__, jabber_id );
     }
    if (vars->ctx)
     { xmpp_ctx_free(vars->ctx);
       Info_new( Config.log, module->Thread_debug, LOG_DEBUG, "%s: '%s': Ctx Free OK", __func__, jabber_id );
     }
    Info_new( Config.log, module->Thread_debug, LOG_DEBUG, "%s: '%s': XMPPshutdown OK", __func__, jabber_id );
    Thread_send_comm_to_master ( module, FALSE );

    if (module->Thread_run == TRUE && vars->signed_off == TRUE)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: '%s': Account signed off. Why ?? Reconnect in 2s!", __func__, jabber_id );
       vars->signed_off = FALSE;
       sleep(2);
       goto reconnect;
     }

    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
