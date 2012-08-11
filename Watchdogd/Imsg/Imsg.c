/**********************************************************************************************************/
/* Watchdogd/Imsg/Imsg.c  Gestion des Instant Messaging IMSG Watchdog 2.0                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                   sam. 28 juil. 2012 16:37:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Imsg.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <loudmouth/loudmouth.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */

 /*static GList *Modules_IMSG;                                  /* Liste des actionneurs/capteurs IMSG */
 static struct CONFIG_RFXCOM
  { gchar username[80];
    gchar server  [80];
    gchar password[80];
    gchar **recipients;
  } Cfg_imsg;

/**********************************************************************************************************/
/* Lire_config_imsg : Lit la config Watchdog et rempli la structure mémoire                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Lire_config_imsg ( struct LIBRAIRIE *lib )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Lire_config_imsg : unable to load config file %s", Config.config_file );
       return;
     }

    lib->Thread_debug = g_key_file_get_boolean ( gkf, "IMSG", "debug", NULL ); /* Positionnement du debug */

    chaine = g_key_file_get_string ( gkf, "IMSG", "username", NULL );
    if (!chaine)
     { Info_new ( Config.log, lib->Thread_debug, LOG_ERR,
                  "Lire_config_imsg: username is missing. Using default." );
       g_snprintf( Cfg_imsg.username, sizeof(Cfg_imsg.username), "defaultuser" );
     }
    else
     { g_snprintf( Cfg_imsg.username, sizeof(Cfg_imsg.username), "%s", chaine );
       g_free(chaine);
     }

    chaine = g_key_file_get_string ( gkf, "IMSG", "server", NULL );
    if (!chaine)
     { Info_new ( Config.log, lib->Thread_debug, LOG_ERR,
                  "Lire_config_imsg: server is missing. Using default." );
       g_snprintf( Cfg_imsg.server, sizeof(Cfg_imsg.username), "defaultserver.org" );
     }
    else
     { g_snprintf( Cfg_imsg.server, sizeof(Cfg_imsg.username), "%s", chaine );
       g_free(chaine);
     }

    chaine = g_key_file_get_string ( gkf, "IMSG", "password", NULL );
    if (!chaine)
     { Info_new ( Config.log, lib->Thread_debug, LOG_ERR,
                  "Lire_config_imsg: password is missing. Using default." );
       g_snprintf( Cfg_imsg.password, sizeof(Cfg_imsg.password), "defaultpassword" );
     }
    else
     { g_snprintf( Cfg_imsg.password, sizeof(Cfg_imsg.password), "%s", chaine );
       g_free(chaine);
     }

    Cfg_imsg.recipients = g_key_file_get_string_list ( gkf, "IMSG", "recipients", NULL, NULL );
    if (!Cfg_imsg.recipients)
     { Info_new ( Config.log, lib->Thread_debug, LOG_ERR,
                  "Lire_config_imsg: recipients are is missing." );
     }

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Liberer_config_imsg : Libere la mémoire allouer précédemment pour lire la config imsg                  */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Liberer_config_imsg ( struct LIBRAIRIE *lib )
  { g_strfreev ( Cfg_imsg.recipients );
  }
/**********************************************************************************************************/
/* Mode_presence : Change la presence du server watchdog aupres du serveur XMPP                           */
/* Entrée: la connexion xmpp                                                                              */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Mode_presence_imsg ( struct LIBRAIRIE *lib, LmConnection *connection, gchar *type, gchar *presence )
  { LmMessage *m;
    GError *error;

    m = lm_message_new ( NULL, LM_MESSAGE_TYPE_PRESENCE );
    lm_message_node_set_attribute (m->node, "type", type );
    lm_message_node_add_child (m->node, "status", presence );
    if (!lm_connection_send (connection, m, &error)) 
     { Info_new( Config.log, lib->Thread_debug, LOG_CRIT,
                 "Mode_presence_imsg: Unable to send presence %s / %s -> %s", type, presence, error->message );
     }
    lm_message_unref (m);
  }
/**********************************************************************************************************/
/* Mode_presence : Change la presence du server watchdog aupres du serveur XMPP                           */
/* Entrée: la connexion xmpp                                                                              */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoi_message_to_imsg ( struct LIBRAIRIE *lib, LmConnection *connection, const gchar *dest, gchar *message )
  { GError *error;
    LmMessage *m;

    m = lm_message_new ( dest, LM_MESSAGE_TYPE_MESSAGE );
    lm_message_node_add_child (m->node, "body", message );
    if (!lm_connection_send (connection, m, &error)) 
     { Info_new( Config.log, lib->Thread_debug, LOG_CRIT,
                 "Envoi_message_to_ismg: Unable to send message %s to %s -> %s", message, dest, error->message );
     }
    lm_message_unref (m);
  }
/**********************************************************************************************************/
/* Reception_message : CB appellé lorsque l'on recoit un message xmpp                                     */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 LmHandlerResult Reception_message ( LmMessageHandler *handler, LmConnection *connection,
                                     LmMessage *message, struct LIBRAIRIE *lib )
  { LmMessageNode *node, *body;
    const gchar *from;
    struct DB *db;

    node = lm_message_get_node ( message );

    Info_new( Config.log, lib->Thread_debug, LOG_DEBUG,
              "Reception_message : recu un msg xmpp : value = %s attr = %s", 
              lm_message_node_get_value ( node ),
              lm_message_node_to_string ( node )
            );

    body = lm_message_node_find_child ( node, "body" );
    if (!body) return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);

    from = lm_message_node_get_attribute ( node, "from" );

    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
              "Reception_message : recu un msg xmpp de %s : %s", 
              from, lm_message_node_get_value ( body )
            );

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, lib->Thread_debug, LOG_WARNING,
                 "Reception_message : Connexion DB failed. imsg not handled" );
       return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
     }
    if ( ! Recuperer_mnemoDB_by_command_text ( Config.log, db, (gchar *)lm_message_node_get_value ( body ) ) )
     { Envoi_message_to_imsg( lib, connection, from, "Error searching Database .. Sorry .." ); }   
    else
     { gboolean none;
       for ( none = TRUE; ; )
        { struct CMD_TYPE_MNEMONIQUE *mnemo;
          mnemo = Recuperer_mnemoDB_suite( Config.log, db );
          if (!mnemo) break;

          Envoi_message_to_imsg( lib, connection, from, mnemo->libelle );
          g_free(mnemo);
          none = FALSE;
        }
       if (none == TRUE)
        { Envoi_message_to_imsg( lib, connection, from, "Error... No result found .. Sorry .." ); }   
     }
    Libere_DB_SQL( Config.log, &db );
    return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
  }
/**********************************************************************************************************/
/* Reception_message : CB appellé lorsque l'on recoit un message xmpp                                     */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 LmHandlerResult Reception_presence ( LmMessageHandler *handler, LmConnection *connection,
                                      LmMessage *message, struct LIBRAIRIE *lib )
  { LmMessageNode *node_presence;
    const gchar *type, *from;
    LmMessage *m;
    GError *error;

    node_presence = lm_message_get_node ( message );
    Info_new( Config.log, lib->Thread_debug, LOG_DEBUG,
              "Reception_presence : recu un msg xmpp : string= %s", 
              lm_message_node_to_string (node_presence)
            );

    type = lm_message_node_get_attribute ( node_presence, "type" );
    from = lm_message_node_get_attribute ( node_presence, "from" );

    Info_new( Config.log, lib->Thread_debug, LOG_INFO,
              "Reception_presence: Recu %s from %s", type, from );

    if ( type && ( ! strcmp ( type, "subscribe" ) ) )
     { m = lm_message_new ( from, LM_MESSAGE_TYPE_PRESENCE );
       lm_message_node_set_attribute ( m->node, "type", "subscribed" );
       if (!lm_connection_send (connection, m, &error)) 
        { Info_new( Config.log, lib->Thread_debug, LOG_WARNING,
                    "Reception_presence: Unable send subscribed to %s -> %s", from, error->message );
        }
       else
        { Info_new( Config.log, lib->Thread_debug, LOG_INFO,
                    "Reception_presence: Sending Subscribed OK to %s", from );
        }
       lm_message_unref (m);
       m = lm_message_new ( from, LM_MESSAGE_TYPE_PRESENCE );
       lm_message_node_set_attribute (m->node, "type", "subscribe" );
       if (!lm_connection_send (connection, m, &error)) 
        { Info_new( Config.log, lib->Thread_debug, LOG_WARNING,
                    "Reception_presence: Unable send subscribe to %s -> %s", from, error->message );
        }
       else
        { Info_new( Config.log, lib->Thread_debug, LOG_INFO,
                    "Reception_presence: Sending Subscribe OK to %s", from );
        }
       lm_message_unref (m);
       return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
     }
    return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);
  }
/**********************************************************************************************************/
/* Reception_message : CB appellé lorsque l'on recoit un message xmpp                                     */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 LmHandlerResult Reception_contact ( LmMessageHandler *handler, LmConnection *connection,
                                     LmMessage *message, struct LIBRAIRIE *lib )
  { LmMessageNode *node_iq;
    node_iq = lm_message_get_node ( message );
    Info_new( Config.log, lib->Thread_debug, LOG_DEBUG,
              "Reception_contact : recu un msg xmpp : iq = %s", 
              lm_message_node_to_string (node_iq)
            );
    return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Imsg                                                               */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { GMainContext *MainLoop;
    LmConnection *connection;
    GError       *error = NULL;

    prctl(PR_SET_NAME, "W-IMSG", 0, 0, 0 );
    Lire_config_imsg ( lib );                         /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    lib->Thread_run = TRUE;                                                         /* Le thread tourne ! */

    g_snprintf( lib->admin_prompt, sizeof(lib->admin_prompt), "imsg" );
    g_snprintf( lib->admin_help,   sizeof(lib->admin_help),   "Manage Instant Messaging system" );

    MainLoop = g_main_context_new();
                                                                 /* Preparation de la connexion au server */
    connection = lm_connection_new_with_context ( Cfg_imsg.server, MainLoop );
    if ( lm_connection_open_and_block (connection, &error) == FALSE )        /* Connexion au serveur XMPP */
     { Info_new( Config.log, lib->Thread_debug, LOG_CRIT,
                 "Run_thread: Unable to connect to xmpp server %s -> %s", Cfg_imsg.server, error->message );
       lib->Thread_run = FALSE;                                                        /* Arret du thread */
     }
    else
     { Info_new( Config.log, lib->Thread_debug, LOG_INFO,
                 "Run_thread: Connection to xmpp server %s OK", Cfg_imsg.server );
       if ( lm_connection_authenticate_and_block ( connection, Cfg_imsg.username, Cfg_imsg.password,
                                                        "resource", &error) == FALSE )
        { Info_new( Config.log, lib->Thread_debug, LOG_CRIT,
                    "Run_thread: Unable to authenticate to xmpp server -> %s", error->message );
          lib->Thread_run = FALSE;                                                        /* Arret du thread */
        }
       else
        { LmMessageHandler *lmMsgHandler;
          Info_new( Config.log, lib->Thread_debug, LOG_INFO,
                    "Run_thread: Authentication to xmpp server OK (%s@%s)", Cfg_imsg.username, Cfg_imsg.server );

                                               /* Set up message handler to handle incoming text messages */
          lmMsgHandler = lm_message_handler_new( (LmHandleMessageFunction)Reception_message, lib, NULL );
          lm_connection_register_message_handler( connection, lmMsgHandler, 
                                                  LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);
          lm_message_handler_unref(lmMsgHandler);

                                           /* Set up message handler to handle incoming presence messages */
          lmMsgHandler = lm_message_handler_new( (LmHandleMessageFunction)Reception_presence, lib, NULL );
          lm_connection_register_message_handler( connection, lmMsgHandler, 
                                                  LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);
          lm_message_handler_unref(lmMsgHandler);

                                                /* Set up message handler to handle incoming contact list */
          lmMsgHandler = lm_message_handler_new( (LmHandleMessageFunction)Reception_contact, lib, NULL );
          lm_connection_register_message_handler( connection, lmMsgHandler, 
                                                  LM_MESSAGE_TYPE_IQ, LM_HANDLER_PRIORITY_NORMAL);
          lm_message_handler_unref(lmMsgHandler);

          Mode_presence_imsg ( lib, connection, "available", "Waiting for commands" );
          Envoi_message_to_imsg ( lib, connection, "lefevre.seb@jabber.fr", "Server is Up and Running" );
        }
     }

    while( lib->Thread_run == TRUE )                                     /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "Run_thread: recu signal SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       g_main_context_iteration ( MainLoop, FALSE );

     }                                                                     /* Fin du while partage->arret */

    Envoi_message_to_imsg ( lib, connection, "lefevre.seb@jabber.fr", "Server is stopping.." );
    Mode_presence_imsg( lib, connection, "unavailable", "Server is down" );
    sleep(2);


    lm_connection_close (connection, NULL);                                 /* Fermeture de la connection */
    lm_connection_unref (connection);                             /* Destruction de la structure associée */
    g_main_context_unref (MainLoop);
    Liberer_config_imsg( lib );                   /* Liberation de la configuration de l'InstantMessaging */

    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    lib->TID = 0;                                         /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
