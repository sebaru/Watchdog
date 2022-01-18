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
/* Imsgs_Creer_DB : Creation de la table du thread                                                                            */
/* Entrée: le pointeur sur la PROCESS                                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_Creer_DB ( struct PROCESS *lib )
  { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'.", __func__, lib->database_version );

    SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                    "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                    "`date_create` datetime NOT NULL DEFAULT NOW(),"
                    "`uuid` varchar(37) COLLATE utf8_unicode_ci NOT NULL,"
                    "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                    "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`jabberid` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`password` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "FOREIGN KEY (`uuid`) REFERENCES `processes` (`uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                    ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", lib->name );

    Process_set_database_version ( lib, 1 );
  }
/******************************************************************************************************************************/
/* Imsgs_Envoi_message_to : Envoi un message a un contact xmpp                                                                */
/* Entrée: le nom du destinataire et le message                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_Envoi_message_to ( struct SUBPROCESS *module, const gchar *dest, gchar *message )
  { struct IMSGS_VARS *vars = module->vars;
    xmpp_stanza_t *stanza = xmpp_message_new ( vars->ctx, "normal", dest, NULL );
    xmpp_message_set_body ( stanza, message );
    xmpp_send ( vars->conn , stanza ) ;
    xmpp_stanza_release ( stanza );
  }
/******************************************************************************************************************************/
/* Imsgs_recipient_allow_command : Renvoie un contact IMSGDB si delui-ci dispose du flag allow_cde                            */
/* Entrée: le jabber_id                                                                                                       */
/* Sortie: struct IMSGSDB *imsg                                                                                               */
/******************************************************************************************************************************/
 static gboolean Imsgs_recipient_is_allow_command ( struct SUBPROCESS *module, gchar *jabber_id )
  { gchar *jabberid, hostonly[80], *ptr;

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Memory Error", __func__ );
       return(FALSE);
     }

    g_snprintf( hostonly, sizeof(hostonly), "%s", jabber_id );
    ptr = strstr( hostonly, "/" );
    if (ptr) *ptr=0;

    jabberid = Normaliser_chaine ( hostonly );
    if (!jabberid)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation jabberid impossible", __func__ );
       json_node_unref(RootNode);
       return(FALSE);
     }

    SQL_Select_to_json_node ( RootNode, NULL,
                             "SELECT allow_cde "
                             "FROM users AS user WHERE enable=1 AND allow_cde=1 AND xmpp LIKE '%s' LIMIT 1", jabberid );
    g_free(jabberid);
    gboolean retour = FALSE;
    if (Json_has_member ( RootNode, "allow_cde" ) && Json_get_bool ( RootNode, "allow_cde" ) == TRUE) retour = TRUE;
    json_node_unref ( RootNode );
    return( retour );
  }
/******************************************************************************************************************************/
/* Imsgs_Envoi_message_to_all_available : Envoi un message aux contacts disponibles                                           */
/* Entrée: le message                                                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Imsgs_Envoi_message_to_all_available ( struct SUBPROCESS *module, gchar *message )
  { JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: Memory Error", __func__ );
       return;
     }
/********************************************* Chargement des informations en bases *******************************************/
    SQL_Select_to_json_node ( RootNode, "recipients",
                              "SELECT id,username,enable,comment,notification,xmpp,allow_cde,imsg_available "
                              "FROM users AS user WHERE enable=1 AND notification=1 AND imsg_available=1 ORDER BY username" );

    GList *recipients = json_array_get_elements ( Json_get_array ( RootNode, "recipients" ) );
    while(recipients)
     { JsonNode *element = recipients->data;
       Imsgs_Envoi_message_to ( module, Json_get_string ( element, "xmpp" ), message );
       recipients = g_list_next(recipients);
     }
    g_list_free(recipients);
    json_node_unref ( RootNode );
  }
/******************************************************************************************************************************/
/* Imsgs_handle_message_CB : CB appellé lorsque l'on recoit un message xmpp                                                   */
/* Entrée : Le Handler, la connexion, le message                                                                              */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static int Imsgs_handle_message_CB (xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata)
  { struct SUBPROCESS *module = userdata;
    struct IMSGS_VARS *vars = module->vars;

    gchar *thread_tech_id  = Json_get_string ( module->config, "thread_tech_id" );

    const gchar *from = xmpp_stanza_get_attribute ( stanza, "from" );
    gchar *message    = xmpp_message_get_body ( stanza );
    if (!from || !message)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: '%s': Error : from or message = NULL", __func__, thread_tech_id );
       return(1);
     }

    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: '%s': From '%s' -> '%s'", __func__, thread_tech_id, from, message );

    if (Imsgs_recipient_is_allow_command ( module, from ) == FALSE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                "%s: '%s': unknown sender '%s' or not allow to send command. Dropping message...", __func__, thread_tech_id, from );
       goto end;
     }

    if ( ! strcasecmp( message, "ping" ) )                                                             /* Interfacage de test */
     { Imsgs_Envoi_message_to( module, from, "Pong !" );
       goto end;
     }

    JsonNode *RootNode = Json_node_create();
    if ( RootNode == NULL )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Memory Error for '%s'", __func__, thread_tech_id, from );
       goto end;
     }
    SQL_Select_to_json_node ( RootNode, "results",
                              "SELECT * FROM mnemos_DI AS m "
                              "INNER JOIN mappings AS map ON m.tech_id = map.tech_id AND m.acronyme = map.acronyme "
                              "WHERE map.thread_tech_id='_COMMAND_TEXT' AND map.thread_acronyme LIKE '%%%s%%'", message );

    if ( Json_has_member ( RootNode, "nbr_results" ) == FALSE )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Error searching Database for '%s'", __func__, thread_tech_id, message );
       Imsgs_Envoi_message_to( module, from, "Error searching Database .. Sorry .." );
       goto end;
     }

    gint nbr_results = Json_get_int ( RootNode, "nbr_results" );
    if ( nbr_results == 0 )
     { Imsgs_Envoi_message_to( module, from, "Je n'ai pas trouvé, désolé." ); }
    else
     { if ( nbr_results > 1 )                                               /* Si trop d'enregistrements, demande de préciser */
        { Imsgs_Envoi_message_to( module, from, "Aîe, plusieurs choix sont possibles ... :" ); }

       GList *Results = json_array_get_elements ( Json_get_array ( RootNode, "results" ) );
       if ( nbr_results > 1 )
        { GList *results = Results;
          while(results)
           { JsonNode *element = results->data;
             gchar *thread_acronyme = Json_get_string ( element, "thread_acronyme" );
             gchar *tech_id         = Json_get_string ( element, "tech_id" );
             gchar *acronyme        = Json_get_string ( element, "acronyme" );
             gchar *libelle         = Json_get_string ( element, "libelle" );
             Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': From '%s' map found for '%s' -> '%s:%s' - %s", __func__,
                       thread_tech_id, from, thread_acronyme, tech_id, acronyme, libelle );
             Imsgs_Envoi_message_to ( module, from, thread_acronyme );                              /* Envoi des différents choix */
             results = g_list_next(results);
           }
        }
       else if ( nbr_results == 1)
        { JsonNode *element = Results->data;
          gchar *thread_acronyme = Json_get_string ( element, "thread_acronyme" );
          gchar *tech_id         = Json_get_string ( element, "tech_id" );
          gchar *acronyme        = Json_get_string ( element, "acronyme" );
          gchar *libelle         = Json_get_string ( element, "libelle" );
          Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': From '%s' map found for '%s' -> '%s:%s' - %s", __func__,
                    thread_tech_id, from, thread_acronyme, tech_id, acronyme, libelle );
          Zmq_Send_CDE_to_master_new ( module, tech_id, acronyme );
          Imsgs_Envoi_message_to ( module, from, "Fait." );                                     /* Envoi des différents choix */
        }
       g_list_free(Results);
     }
end:
    json_node_unref( RootNode );
    xmpp_free(vars->ctx, message);
    return(1);
  }
/******************************************************************************************************************************/
/* Imsgs_Sauvegarder_statut_contact : Sauvegarde en mémoire le statut du contact en parametre                                 */
/* Entrée: le contact et le statut                                                                                            */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_Sauvegarder_statut_contact ( struct SUBPROCESS *module, const gchar *jabber_id, gboolean available )
  { gchar *jabberid, hostonly[80], *ptr;

    g_snprintf( hostonly, sizeof(hostonly), "%s", jabber_id );
    ptr = strstr( hostonly, "/" );
    if (ptr) *ptr=0;

    jabberid = Normaliser_chaine ( hostonly );
    if (!jabberid)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation jabberid '%s' impossible", __func__, hostonly );
       return;
     }

    if (SQL_Write_new ( "UPDATE users SET imsg_available='%d' WHERE xmpp='%s'", available, jabberid ) == FALSE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: Requete failed", __func__ ); }
    else { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
                    "%s : jabber_id %s -> Availability updated to %d.", __func__, jabber_id, available );
         }
  }
/******************************************************************************************************************************/
/* Imsgs_set_presence: Défini le statut présenté aux partenaires                                                              */
/* Entrée : le statut au format chaine de caratères                                                                           */
/******************************************************************************************************************************/
  static void Imsgs_set_presence ( struct SUBPROCESS *module, const char *status_to_send )
  { struct IMSGS_VARS *vars = module->vars;
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
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: '%s'", __func__, buf );
    xmpp_free(vars->ctx, buf);

    xmpp_send(vars->conn, pres);
    xmpp_stanza_release(pres);
  }
/******************************************************************************************************************************/
/* Imsgs_handle_presence_CB: appellé par libstrophe lors d'un changement de statut d'un partenaire                            */
/* Entrée : les infos de la librairie                                                                                         */
/******************************************************************************************************************************/
 static int Imsgs_handle_presence_CB ( xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata )
  { struct SUBPROCESS *module = userdata;
    struct IMSGS_VARS *vars = module->vars;
    const char *type, *from;
    type = xmpp_stanza_get_type ( stanza );
    from = xmpp_stanza_get_from ( stanza );
    if (type && !strcmp(type,"unavailable") ) Imsgs_Sauvegarder_statut_contact ( module, from, FALSE );
    else Imsgs_Sauvegarder_statut_contact ( module, from, TRUE );

    if (type && !strcmp(type,"subscribe"))                            /* Demande de souscription de la part d'un utilisateur */
     { xmpp_stanza_t *pres;
       pres = xmpp_presence_new(vars->ctx);
       xmpp_stanza_set_to  ( pres, from );
       xmpp_stanza_set_type( pres, "subscribed" );
       xmpp_send(vars->conn, pres);
       xmpp_stanza_release(pres);
       pres = xmpp_presence_new(vars->ctx);
       xmpp_stanza_set_to  ( pres, Json_get_string ( module->config, "jabberid" ) );
       xmpp_stanza_set_type( pres, "subscribe" );
       xmpp_send(vars->conn, pres);
       xmpp_stanza_release(pres);
     }
    gchar *buf; size_t buflen;
    xmpp_stanza_to_text ( stanza, &buf, &buflen );
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: '%s'", __func__, buf );
    xmpp_free(vars->ctx, buf);

    return(1);
  }
/******************************************************************************************************************************/
/* Imsgs_connexion_CB: appellé par libstrophe lors d'un changement de statut de la connexion                                  */
/* Entrée : les infos de la librairie                                                                                         */
/******************************************************************************************************************************/
 static void Imsgs_connexion_CB ( xmpp_conn_t *const conn, const xmpp_conn_event_t status, const int error,
                                  xmpp_stream_error_t *const stream_error, void *const userdata )
  { struct SUBPROCESS *module = userdata;
    struct IMSGS_VARS *vars = module->vars;
    if (status == XMPP_CONN_CONNECT)
     {
       Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: '%s': Account connected and %s secure",
                 __func__, Json_get_string ( module->config, "jabberid" ), (xmpp_conn_is_secured (conn) ? "IS" : "IS NOT") );
       xmpp_handler_add ( vars->conn, Imsgs_handle_message_CB,  NULL, "message",  NULL, module );
       xmpp_handler_add ( vars->conn, Imsgs_handle_presence_CB, NULL, "presence", NULL, module );
       /*xmpp_handler_add   ( vars->conn, Imsgs_test, NULL, NULL, NULL, NULL );*/

       Imsgs_set_presence( module, "A votre écoute !" );
       Imsgs_Envoi_message_to_all_available ( module, "Instance démarrée. A l'écoute !" );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: '%s': Account disconnected",
                 __func__, Json_get_string ( module->config, "jabberid" ) );
       vars->signed_off = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct IMSGS_VARS) );
    struct IMSGS_VARS *vars = module->vars;

    gchar *thread_tech_id   = Json_get_string ( module->config, "thread_tech_id" );
    gchar *jabber_id = Json_get_string ( module->config, "jabberid" );

    if ( ! (thread_tech_id && jabber_id) )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: No thread_tech_id Or Jabber_id. Stopping.", __func__ );
       goto end;
     }

reconnect:
    vars->signed_off = FALSE;
    vars->ctx  = xmpp_ctx_new(NULL, xmpp_get_default_logger(XMPP_LEVEL_INFO));
    if (!vars->ctx)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: Ctx Init failed", __func__ ); goto end; }

    vars->conn = xmpp_conn_new(vars->ctx);
    if (!vars->conn)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: Connection New failed", __func__ ); goto end; }

    xmpp_conn_set_keepalive(vars->conn, 60, 1);
    xmpp_conn_set_jid (vars->conn, jabber_id );
    xmpp_conn_set_pass(vars->conn, Json_get_string ( module->config, "password" ));

    gint retour = xmpp_connect_client ( vars->conn, NULL, 0, Imsgs_connexion_CB, module );
    if ( retour != XMPP_EOK)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                  "%s: '%s': Connexion failed with error %d", __func__, jabber_id, retour );
       vars->signed_off = TRUE;
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Connexion in progress.", __func__, jabber_id );
       SubProcess_send_comm_to_master_new ( module, TRUE );
     }

    while(module->lib->Thread_run == TRUE && vars->signed_off == FALSE && module->lib->Thread_reload == FALSE)/* On tourne tant que necessaire */
     { /*g_usleep(200000);*/
       sched_yield();

       xmpp_run_once ( vars->ctx, 500 ); /* En milliseconde */

       SubProcess_send_comm_to_master_new ( module, module->comm_status );         /* Périodiquement envoie la comm au master */
/********************************************************* Envoi de SMS *******************************************************/
       JsonNode *request;
       while ( (request = SubProcess_Listen_to_master_new ( module ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "DLS_HISTO" ) &&
               Json_get_bool ( request, "alive" ) == TRUE )
           { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: '%s': Sending msg '%s:%s' (%s)", __func__,
                       jabber_id,
                       Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ),
                       Json_get_string ( request, "libelle" ) );
             gchar chaine[256];
             g_snprintf( chaine, sizeof(chaine), "%s: %s", Json_get_string ( request, "dls_shortname" ), Json_get_string ( request, "libelle" ) );
             Imsgs_Envoi_message_to_all_available ( module, chaine );
           }
          else if ( !strcasecmp( zmq_tag, "test" ) ) Imsgs_Envoi_message_to_all_available ( module, "Test OK" );
          else
           { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': zmq_tag '%s' not for this thread", __func__,
                       jabber_id, zmq_tag ); }
          json_node_unref(request);
        }
     }                                                                                         /* Fin du while partage->arret */

end:
    if (vars->conn)
     { xmpp_disconnect(vars->conn);
       Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': Disconnect OK", __func__, jabber_id );
       xmpp_conn_release(vars->conn);
       Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': Connection Release OK", __func__, jabber_id );
     }
    if (vars->ctx)
     { xmpp_ctx_free(vars->ctx);
       Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': Ctx Free OK", __func__, jabber_id );
     }
    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': XMPPshutdown OK", __func__, jabber_id );
    SubProcess_send_comm_to_master_new ( module, FALSE );

    if (module->lib->Thread_run == TRUE && vars->signed_off == TRUE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Account signed off. Why ?? Reconnect in 2s!", __func__, jabber_id );
       vars->signed_off = FALSE;
       sleep(2);
       goto reconnect;
     }

    SubProcess_end(module);
  }
/******************************************************************************************************************************/
/* Run_process: Run du Process                                                                                                */
/* Entrée: la structure PROCESS associée                                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_process ( struct PROCESS *lib )
  {
reload:
    Imsgs_Creer_DB ( lib );                                                                    /* Création de la DB du thread */
    Thread_init ( "imsgs", "USER", lib, WTD_VERSION, "Manage Instant Messaging system (libstrophe)" );

    lib->config = Json_node_create();
    if(lib->config) SQL_Select_to_json_node ( lib->config, "subprocess", "SELECT * FROM %s WHERE uuid='%s'", lib->name, lib->uuid );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: %d subprocess to load", __func__, Json_get_int ( lib->config, "nbr_subprocess" ) );

    xmpp_initialize();
    Json_node_foreach_array_element ( lib->config, "subprocess", Process_Load_one_subprocess, lib );   /* Chargement des modules */
    while( lib->Thread_run == TRUE && lib->Thread_reload == FALSE) sleep(1);                 /* On tourne tant que necessaire */
    Process_Unload_all_subprocess ( lib );
    xmpp_shutdown();

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }

    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
