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
  } Cfg_imsg;

/**********************************************************************************************************/
/* Lire_config : Lit la config Watchdog et rempli la structure mémoire                                    */
/* Entrée: le nom de fichier à lire                                                                       */
/* Sortie: La structure mémoire est à jour                                                                */
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

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Reception_message : CB appellé lorsque l'on recoit un message xmpp                                     */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 LmHandlerResult Reception_message ( LmMessageHandler *handler, LmConnection *connection,
                                     LmMessage *message, struct LIBRAIRIE *lib )
  { LmMessageNode *node;
    node = lm_message_get_node ( message );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
              "Reception_message : recu un msg xmpp : value = %s", 
              lm_message_node_get_value ( node )
            );
    return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
  }
/**********************************************************************************************************/
/* Reception_message : CB appellé lorsque l'on recoit un message xmpp                                     */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 LmHandlerResult Reception_presence ( LmMessageHandler *handler, LmConnection *connection,
                                      LmMessage *message, struct LIBRAIRIE *lib )
  { LmMessageNode *node;
    node = lm_message_get_node ( message );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
              "Reception_presence : recu un msg xmpp : value = %s", 
              lm_message_node_get_value ( node )
            );
    return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
  }
/**********************************************************************************************************/
/* Reception_message : CB appellé lorsque l'on recoit un message xmpp                                     */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 LmHandlerResult Reception_contact ( LmMessageHandler *handler, LmConnection *connection,
                                     LmMessage *message, struct LIBRAIRIE *lib )
  { LmMessageNode *node;
    node = lm_message_get_node ( message );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
              "Reception_contact : recu un msg xmpp : value = %s", 
              lm_message_node_get_value ( node )
            );
    return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
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
          LmMessage *m;
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


              m = lm_message_new ("lefevre.seb@jabber.fr", LM_MESSAGE_TYPE_MESSAGE);
                lm_message_node_add_child (m->node, "body", "message");
                if (!lm_connection_send (connection, m, &error)) {
                  Info_new( Config.log, lib->Thread_debug, LOG_CRIT,
                      "Run_thread: Unable to send message %s -> %s", Cfg_imsg.server, error->message );
        
             }
              else { Info_new( Config.log, lib->Thread_debug, LOG_INFO,
                     "Run_thread: Message sent to seb" );
       
                   }
          lm_message_unref (m);


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

    lm_connection_close (connection, NULL);                                 /* Fermeture de la connection */
    lm_connection_unref (connection);                             /* Destruction de la structure associée */
    g_main_context_unref (MainLoop);

    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    lib->TID = 0;                                         /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
