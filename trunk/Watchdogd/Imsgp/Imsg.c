/******************************************************************************************************************************/
/* Watchdogd/Imsg/Imsg.c  Gestion des Instant Messaging IMSG Watchdog 2.0                                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    20.02.2018 17:58:31 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
 
 #include <purple.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Imsg.h"

 static GMainLoop *MainLoop;                                                    /* Contexte pour attendre les evenements xmpp */
 static void Imsgp_Fermer_connexion ( void );
 static gboolean Imsgp_Ouvrir_connexion ( void );
 static void Imsgp_Connexion_close (LmConnection *connection, LmDisconnectReason reason, gpointer user_data);

/******************************************************************************************************************************/
/* Imsgp_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                  */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Imsgp_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_imsgp.lib->Thread_debug = FALSE;                                                       /* Settings default parameters */
    Cfg_imsgp.enable            = FALSE; 
    g_snprintf( Cfg_imsgp.username, sizeof(Cfg_imsgp.username), DEFAUT_USERNAME_IMSG );
    g_snprintf( Cfg_imsgp.server,   sizeof(Cfg_imsgp.server  ), DEFAUT_SERVER_IMSG );
    g_snprintf( Cfg_imsgp.password, sizeof(Cfg_imsgp.password), DEFAUT_PASSWORD_IMSG );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_INFO,                                           /* Print Config */
                "%s: '%s' = %s", __func__, nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "username" ) )
        { g_snprintf( Cfg_imsgp.username, sizeof(Cfg_imsgp.username), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "server" ) )
        { g_snprintf( Cfg_imsgp.server, sizeof(Cfg_imsgp.server), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "password" ) )
        { g_snprintf( Cfg_imsgp.password, sizeof(Cfg_imsgp.password), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_imsgp.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_imsgp.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_imsgDB: Recupération de la liste des ids des imsgs                                  */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_imsgDB ( struct DB *db )
  { gchar requete[512];
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,name,enable,comment,imsg_enable,imsg_jabberid,imsg_allow_cde,imsg_available "
                " FROM %s as user ORDER BY user.name",
                NOM_TABLE_UTIL );

    return ( Lancer_requete_SQL ( db, requete ) );                         /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_imsgDB: Recupération de la liste des ids des imsgs                                  */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_all_available_imsgDB ( struct DB *db )
  { gchar requete[512];
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,name,enable,comment,imsg_enable,imsg_jabberid,imsg_allow_cde,imsg_available "
                " FROM %s as user WHERE enable=1 AND imsg_enable=1 AND imsg_available=1 ORDER BY user.name",
                NOM_TABLE_UTIL );

    return ( Lancer_requete_SQL ( db, requete ) );                         /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_imsgDB: Recupération de la liste des ids des imsgs                                  */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct IMSGDB *Recuperer_imsgDB_suite( struct DB *db )
  { struct IMSGDB *imsg;

    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( db );
       return(NULL);
     }

    imsg = (struct IMSGDB *)g_try_malloc0( sizeof(struct IMSGDB) );
    if (!imsg) Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_ERR, "Recuperer_imsgDB_suite: Erreur allocation mémoire" );
    else
     { g_snprintf( imsg->user_jabberid, sizeof(imsg->user_jabberid), "%s", db->row[5] );
       g_snprintf( imsg->user_name,     sizeof(imsg->user_name),     "%s", db->row[1] );
       g_snprintf( imsg->user_comment,  sizeof(imsg->user_comment),  "%s", db->row[3] );
       imsg->user_id           = atoi(db->row[0]);
       imsg->user_enable       = atoi(db->row[2]);
       imsg->user_imsg_enable  = atoi(db->row[4]);
       imsg->user_allow_cde    = atoi(db->row[6]);
       imsg->user_available    = atoi(db->row[7]);
     }
    return(imsg);
  }



/******************************************************************************************************************************/
/* Imsgp_recevoir_imsg: Appelé lors de la reception d'un message depuis le reseau                                             */
/* Entrée: La conversation et le message IMSG                                                                                 */
/******************************************************************************************************************************/
 static void Imsgp_recevoir_imsg (PurpleConversation *conv, PurpleMessage *msg)
  { time_t mtime = purple_message_get_time(msg);

    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
             "%s: (%s) %s %s: %s", __func__,
		           purple_conversation_get_name(conv),
		           purple_utf8_strftime("(%H:%M:%S)", localtime(&mtime)),
		           purple_message_get_author_alias(msg),
		           purple_message_get_contents(msg));
  }

 static PurpleConversationUiOps Imsg_conv_uiops =
  {	NULL,                      /* create_conversation  */
   	NULL,                      /* destroy_conversation */
   	NULL,                      /* write_chat           */
   	NULL,                      /* write_im             */
   	Imsgp_recevoir_imsg,       /* write_conv           */
   	NULL,                      /* chat_add_users       */
   	NULL,                      /* chat_rename_user     */
   	NULL,                      /* chat_remove_users    */
   	NULL,                      /* chat_update_user     */
   	NULL,                      /* present              */
   	NULL,                      /* has_focus            */
   	NULL,                      /* send_confirm         */
   	NULL,
   	NULL,
   	NULL,
   	NULL
  };
/******************************************************************************************************************************/
/* Imsgp_ui_init: Initialisation de la librairie libpurple                                                                    */
/* Entrée/sortie: Néant                                                                                                       */
/******************************************************************************************************************************/
 static void Imsgp_ui_init ( void )
  {	purple_conversations_set_ui_ops(&Imsg_conv_uiops);
  }

 static PurpleCoreUiOps null_core_uiops =
  {	NULL,
   	NULL,
	   Imsgp_ui_init,
	   NULL,
	 /* padding */
   	NULL,
	   NULL,
	   NULL,
   	NULL,
   	NULL
  };

static void
init_libpurple(void)
{
	/* Set a custom user directory (optional) */
	purple_util_set_user_dir(CUSTOM_USER_DIRECTORY);

	/* We do not want any debugging for now to keep the noise to a minimum. */
	purple_debug_set_enabled(FALSE);

	/* Set the core-uiops, which is used to
	 * 	- initialize the ui specific preferences.
	 * 	- initialize the debug ui.
	 * 	- initialize the ui components for all the modules.
	 * 	- uninitialize the ui components for all the modules when the core terminates.
	 */
	purple_core_set_ui_ops(&null_core_uiops);

	/* Now that all the essential stuff has been set, let's try to init the core. It's
	 * necessary to provide a non-NULL name for the current ui to the core. This name
	 * is used by stuff that depends on this ui, for example the ui-specific plugins. */
	if (!purple_core_init(UI_ID)) {
		/* Initializing the core failed. Terminate. */
		fprintf(stderr,
				"libpurple initialization failed. Dumping core.\n"
				"Please report this!\n");
		abort();
	}

	/* Set path to search for plugins. The core (libpurple) takes care of loading the
	 * core-plugins, which includes the in-tree protocols. So it is not essential to add
	 * any path here, but it might be desired, especially for ui-specific plugins. */
	purple_plugins_add_search_path(CUSTOM_PLUGIN_PATH);
	purple_plugins_refresh();

	/* Load the preferences. */
	purple_prefs_load();

	/* Load the desired plugins. The client should save the list of loaded plugins in
	 * the preferences using purple_plugins_save_loaded(PLUGIN_SAVE_PREF) */
	purple_plugins_load_saved(PLUGIN_SAVE_PREF);
}

 static void Imsgp_signed_on(PurpleConnection *gc, gpointer null)
  {	PurpleAccount *account = purple_connection_get_account(gc);
    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
             "%s: Account '%s' connected for protocol id '%s'", __func__,
              purple_account_get_username(account), purple_account_get_protocol_id(account));
  }



/**********************************************************************************************************/
/* Imsgp_Gerer_message: Fonction d'abonné appellé lorsqu'un message est disponible.                        */
/* Entrée: une structure CMD_TYPE_HISTO                                                                   */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Imsgp_Gerer_histo ( struct CMD_TYPE_HISTO *histo )
  { gint taille;

    pthread_mutex_lock( &Cfg_imsgp.lib->synchro );                        /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_imsgp.Liste_histos );
    pthread_mutex_unlock( &Cfg_imsgp.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "Imsgp_Gerer_histo: DROP message %d (length = %d > 150)", histo->msg.num, taille);
       g_free(histo);
       return;
     }
    else if (Cfg_imsgp.lib->Thread_run == FALSE)
     { Info_new( Config.log, Config.log_arch, LOG_INFO,
                "Imsgp_Gerer_histo: Thread is down. Dropping msg %d", histo->msg.num );
       g_free(histo);
       return;
     }

    pthread_mutex_lock ( &Cfg_imsgp.lib->synchro );
    Cfg_imsgp.Liste_histos = g_slist_append ( Cfg_imsgp.Liste_histos, histo );          /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_imsgp.lib->synchro );
  }
/**********************************************************************************************************/
/* Imsgp_Sauvegarder_statut_contact : Sauvegarde en mémoire le statut du contact en paremetre              */
/* Entrée: le contact et le statut                                                                        */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Imsgp_Sauvegarder_statut_contact ( gchar *jabber_id, gboolean available )
  { gchar *jabberid, requete[512], hostonly[80], *ptr;
    struct DB *db;

    g_snprintf( hostonly, sizeof(hostonly), "%s", jabber_id );
    ptr = strstr( hostonly, "/" );
    if (ptr) *ptr=0;

    jabberid = Normaliser_chaine ( hostonly );
    if (!jabberid)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Imsgp_Sauvegarder_statut_contact: Normalisation jabberid impossible" );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET imsg_available=%d WHERE imsg_jabberid='%s'",
                NOM_TABLE_UTIL, available, jabberid );
    g_free(jabberid);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "Imsgp_Sauvegarder_statut_contact: Database Connection Failed" );
       return;
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )                     /* Execution de la requete SQL */
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "Imsgp_Sauvegarder_statut_contact: Requete failed" );
     }
    else { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_DEBUG,
                    "Imsgp_Sauvegarder_statut_contact : jabber_id %s -> Availability updated to %d.",
                     jabber_id, available );
         } 
    Libere_DB_SQL( &db );
  }
/**********************************************************************************************************/
/* Imsgp_recipient_allow_command : Renvoie un contact IMSGDB si delui-ci dispose du flag allow_cde         */
/* Entrée: le jabber_id                                                                                   */
/* Sortie: struct IMSGDB *imsg                                                                            */
/**********************************************************************************************************/
 static struct IMSGDB *Imsgp_recipient_allow_command ( gchar *jabber_id )
  { gchar *jabberid, requete[512], hostonly[80], *ptr;
    struct IMSGDB *imsg;
    struct DB *db;

    g_snprintf( hostonly, sizeof(hostonly), "%s", jabber_id );
    ptr = strstr( hostonly, "/" );
    if (ptr) *ptr=0;

    jabberid = Normaliser_chaine ( hostonly );
    if (!jabberid)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Imsgp_recipient_allow_command: Normalisation jabberid impossible" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,name,enable,comment,imsg_enable,imsg_jabberid,imsg_allow_cde,imsg_available "
                " FROM %s as user WHERE enable=1 AND imsg_allow_cde=1 AND imsg_jabberid LIKE '%s' LIMIT 1",
                NOM_TABLE_UTIL, jabberid );
    g_free(jabberid);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "Imsgp_recipient_allow_command: Database Connection Failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )                     /* Execution de la requete SQL */
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "Imsgp_recipient_allow_command: Requete failed" );
       Libere_DB_SQL( &db );
       return(NULL);
     }

    imsg = Recuperer_imsgDB_suite ( db );
    Libere_DB_SQL( &db );
    return(imsg);
  }
/**********************************************************************************************************/
/* Mode_presence : Change la presence du server watchdog aupres du serveur XMPP                           */
/* Entrée: la connexion xmpp                                                                              */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Imsgp_Mode_presence ( gchar *type, gchar *show, gchar *status )
  { LmMessage *m;
    GError *error = NULL;

    if ( lm_connection_get_state ( Cfg_imsgp.connection ) != LM_CONNECTION_STATE_AUTHENTICATED )
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_CRIT,
                 "Imsgp_Mode_presence: Not connected .. cannot send presence %s / %s / %s -> %s",
                 type, show, status, error->message );
       return;
     }

    m = lm_message_new ( NULL, LM_MESSAGE_TYPE_PRESENCE );
    if (type)   lm_message_node_set_attribute (m->node, "type", type );
    if (show)   lm_message_node_add_child (m->node, "show", show );
    if (status) lm_message_node_add_child (m->node, "status", status );
    if (!lm_connection_send (Cfg_imsgp.connection, m, &error)) 
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_CRIT,
                 "Imsgp_Mode_presence: Unable to send presence %s / %s / %s -> %s",
                 type, show, status, error->message );
     }
    lm_message_unref (m);
  }
/**********************************************************************************************************/
/* Imsgp_Envoi_message_to : Envoi un message a un contact xmpp                                             */
/* Entrée: le nom du destinataire et le message                                                           */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Imsgp_Envoi_message_to ( const gchar *dest, gchar *message )
  { GError *error= NULL;
    LmMessage *m;

    if ( lm_connection_get_state ( Cfg_imsgp.connection ) != LM_CONNECTION_STATE_AUTHENTICATED )
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_CRIT,
                 "Imsgp_Envoi_message_to: Not connected .. cannot send %s to %s",
                 message, dest );
       return;
     }

    m = lm_message_new ( dest, LM_MESSAGE_TYPE_MESSAGE );
    lm_message_node_add_child (m->node, "body", message );
    if (!lm_connection_send (Cfg_imsgp.connection, m, &error)) 
     { if (error)
        { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_CRIT,
                   "Imsgp_Envoi_message_to: Unable to send message %s to %s -> %s", message, dest, error->message );
        }
       else
        { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_CRIT,
                   "Imsgp_Envoi_message_to: Unable to send message %s to %s -> Unknown error", message, dest );
        }
     }
    lm_message_unref (m);
  }
/**********************************************************************************************************/
/* Imsgp_Envoi_message_to_all_available : Envoi un message aux contacts disponibles                        */
/* Entrée: le message                                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Imsgp_Envoi_message_to_all_available ( gchar *message )
  { struct IMSGDB *imsg;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "Imsgp_Envoi_message_to_all_available: Database Connection Failed" );
       return;
     }

/************************************* Chargement des informations en bases *******************************/
    if ( ! Recuperer_all_available_imsgDB( db ) )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "Imsgp_Envoi_message_to_all_available: Recuperer_imsg Failed" );
       return;
     }

    while ( (imsg = Recuperer_imsgDB_suite( db )) != NULL)
     { Imsgp_Envoi_message_to ( imsg->user_jabberid, message ); }

    Libere_DB_SQL( &db );
  }
/**********************************************************************************************************/
/* Imsgp_Reception_message : CB appellé lorsque l'on recoit un message xmpp                                */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static LmHandlerResult Imsgp_Reception_message ( LmMessageHandler *handler, LmConnection *connection,
                                                 LmMessage *message, gpointer data )
  { LmMessageNode *node, *body;
    struct IMSGDB *imsg;
    gchar *from, *type;
    struct DB *db;

    node = lm_message_get_node ( message );

    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_DEBUG,
              "Imsgp_Reception_message : recu un msg xmpp : value = %s attr = %s", 
              lm_message_node_get_value ( node ),
              lm_message_node_to_string ( node )
            );

    body = lm_message_node_find_child ( node, "body" );
    if (!body) return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);

    from = lm_message_node_get_attribute ( node, "from" );
    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
              "Imsgp_Reception_message : recu un msg xmpp de %s : %s", 
              from, lm_message_node_get_value ( body )
            );
    type = lm_message_node_get_attribute ( node, "type" );
    if ( type && !strcmp ( type, "error" ) )
     { return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS); }

                                 /* On drop les messages du serveur jabber et des interlocuteurs inconnus */
    if ( (!from) || (!strncmp ( from, Cfg_imsgp.username, strlen(Cfg_imsgp.username) )) )
     { return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS); }

    imsg = Imsgp_recipient_allow_command ( from );
    if ( imsg == NULL )
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "Imsgp_Reception_message : unknown sender %s. Dropping message...", from );
       return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);
     }

    if ( ! strcasecmp( (gchar *)lm_message_node_get_value ( body ), "ping" ) )     /* Interfacage de test */
     { Imsgp_Envoi_message_to( from, "Pong !" ); }   
    else if ( ! Recuperer_mnemo_baseDB_by_event_text ( &db, NOM_THREAD, (gchar *)lm_message_node_get_value ( body ) ) )
     { Imsgp_Envoi_message_to( from, "Error searching Database .. Sorry .." ); }   
    else 
     { struct CMD_TYPE_MNEMO_BASE *mnemo, *result_mnemo = NULL;
          
       if ( db->nbr_result == 0 )                         /* Si pas d'enregistrement, demande de préciser */
        { Imsgp_Envoi_message_to( from, "Error... No result found .. Sorry .." ); }   
       if ( db->nbr_result > 1 )                         /* Si trop d'enregistrement, demande de préciser */
        { Imsgp_Envoi_message_to( from, " Need to choose ... :" ); }

       while ( (mnemo = Recuperer_mnemo_baseDB_suite( &db )) != NULL )
        { if (db->nbr_result>1) Imsgp_Envoi_message_to( from, mnemo->ev_text );
          if (db->nbr_result==1) result_mnemo = mnemo;
                            else g_free(mnemo);
        }
       if (result_mnemo)
        { gchar chaine[80];
          switch ( result_mnemo->type )
           { case MNEMO_MONOSTABLE:                                      /* Positionnement du bit interne */
                  g_snprintf( chaine, sizeof(chaine), "Mise a un du bit M%03d by %s",
                              result_mnemo->num, imsg->user_name );
                  Imsgp_Envoi_message_to( from, chaine );
                  Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                             "Imsgp_Reception_message: Mise a un du bit M%03d by %s",
                            result_mnemo->num, imsg->user_name );
                  Envoyer_commande_dls(result_mnemo->num); 
                  break;
             case MNEMO_ENTREE:
                  g_snprintf( chaine, sizeof(chaine), " Result = %d", E(result_mnemo->num) );
                  Imsgp_Envoi_message_to( from, chaine );
                  break;
             case MNEMO_ENTREE_ANA:
                  g_snprintf( chaine, sizeof(chaine), " Result = %f", EA_ech(result_mnemo->num) );
                  Imsgp_Envoi_message_to( from, chaine );
                  break;
             default: g_snprintf( chaine, sizeof(chaine), "Cannot handle command... Check Mnemo !" );
                      Imsgp_Envoi_message_to( from, chaine );
                      Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                               "Imsgp_Reception_message: Cannot handle commande type %d (num=%03d) for %s",
                                result_mnemo->type, result_mnemo->num, imsg->user_name );
                      break;
           }
          g_free(result_mnemo);
        }
     }
    g_free(imsg);
    return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
  }
/**********************************************************************************************************/
/* Imsgp_Reception_message : CB appellé lorsque l'on recoit un message de type presence                    */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static LmHandlerResult Imsgp_Reception_presence ( LmMessageHandler *handler, LmConnection *connection,
                                                  LmMessage *message, gpointer data )
  { LmMessageNode *node_presence, *node_show, *node_status;
    gchar *type, *from, *show, *status;
    GError *error = NULL;
    LmMessage *m;

    node_presence = lm_message_get_node ( message );
    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_DEBUG,
              "Imsgp_Reception_presence : recu un msg presence : string= %s", 
              lm_message_node_to_string (node_presence)
            );

    type = lm_message_node_get_attribute ( node_presence, "type" );
    from = lm_message_node_get_attribute ( node_presence, "from" );

    node_show = lm_message_node_get_child( node_presence, "show" );
    if (node_show) show = lm_message_node_get_value( node_show );
              else show = NULL;

    node_status = lm_message_node_get_child( node_presence, "status" );
    if (node_status) status = lm_message_node_get_value( node_status );
                else status = NULL;

    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_INFO,
              "Imsgp_Reception_presence: Recu type=%s, show=%s, status=%s from %s", type, show, status, from );

    if ( type &&  ( ! strcmp ( type, "subscribe" ) ) ) /* Demande de subscription à notre status presence */
     { m = lm_message_new ( from, LM_MESSAGE_TYPE_PRESENCE );
       lm_message_node_set_attribute ( m->node, "type", "subscribed" );
       if (!lm_connection_send (Cfg_imsgp.connection, m, &error)) 
        { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                    "Imsgp_Reception_presence: Unable send subscribed to %s -> %s", from, error->message );
        }
       else
        { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_INFO,
                    "Imsgp_Reception_presence: Sending Subscribed OK to %s", from );
        }
       lm_message_unref (m);
                                                       /* Demande de subscription au status du partenaire */
       m = lm_message_new ( from, LM_MESSAGE_TYPE_PRESENCE );
       lm_message_node_set_attribute (m->node, "type", "subscribe" );
       if (!lm_connection_send (Cfg_imsgp.connection, m, &error)) 
        { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                    "Imsgp_Reception_presence: Unable send subscribe to %s -> %s", from, error->message );
        }
       else
        { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_INFO,
                    "Imsgp_Reception_presence: Sending Subscribe OK to %s", from );
        }
       lm_message_unref (m);

       Imsgp_Sauvegarder_statut_contact ( from, FALSE );         /* Par défaut, le contact est unavailable */
       return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
     }
    else if ( type && ( ! strcmp ( type, "unavailable" ) ) )    /* Gestion de la deconnexion des contacts */
     { Imsgp_Sauvegarder_statut_contact ( from, FALSE );                     /* Le contact est unavailable */
     }
    else if ( ! type )                                            /* Gestion de la connexion des contacts */
     { Imsgp_Sauvegarder_statut_contact ( from, TRUE );                      /* Le contact est unavailable */
     }
    return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);
  }
/**********************************************************************************************************/
/* Imsgp_Reception_message : CB appellé lorsque l'on recoit un message xmpp de type IQ                     */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static LmHandlerResult Imsgp_Reception_contact ( LmMessageHandler *handler, LmConnection *connection,
                                                 LmMessage *message, gpointer data )
  { LmMessageNode *node_iq;
    node_iq = lm_message_get_node ( message );
    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_DEBUG,
              "Imsgp_Reception_contact : recu un msg xmpp : iq = %s", 
              lm_message_node_to_string (node_iq)
            );
    return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);
  }
/**********************************************************************************************************/
/* Imsgp_Fermer_connexion : Ferme une connexion avec le serveur Jabber                                     */
/* Entrée : Void                                                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Imsgp_Fermer_connexion ( void )
  { if ( ! lm_connection_is_authenticated  ( Cfg_imsgp.connection ) )
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                 "Imsgp_Fermer_connexion: Strange, connexion is not authenticated...");
     }
    else                                                           /* Fermeture de la Cfg_imsgp.connection */
     { Imsgp_Mode_presence( "unavailable", "xa", "Server is down" );
     }
    lm_connection_close (Cfg_imsgp.connection, NULL);
    sleep(2);
                                                                  /* Destruction de la structure associée */
    lm_connection_unref (Cfg_imsgp.connection);
    Cfg_imsgp.connection = NULL;
    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_DEBUG,
              "Imsgp_Fermer_connexion: Connexion is closed.");
  }
/**********************************************************************************************************/
/* Imsgp_Ouvrir_connexion : Ouvre une connexion avec le serveur Jabber                                     */
/* Entrée : Void                                                                                          */
/* Sortie : TRUE ou FALSE si probleme                                                                     */
/**********************************************************************************************************/
 static gboolean Imsgp_Ouvrir_connexion ( void )
  { LmMessageHandler *lmMsgHandler;
    GError *error = NULL;

    Cfg_imsgp.connection = lm_connection_new_with_context ( Cfg_imsgp.server, MainLoop );
    if ( Cfg_imsgp.connection == NULL )
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_CRIT,
                 "Imsgp_Ouvrir_connexion: Error creating connection" );
       return(FALSE);
     }

    if ( Cfg_imsgp.connection && (lm_ssl_is_supported () == TRUE) )
     { LmSSL *ssl;
       ssl = lm_ssl_new ( NULL, NULL, NULL, NULL );
       lm_ssl_use_starttls ( ssl, TRUE, TRUE );
       lm_connection_set_ssl ( Cfg_imsgp.connection, ssl );
       lm_ssl_unref ( ssl );
       Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                 "Imsgp_Ouvrir_connexion: SSL is available" );
     }
    else { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                     "Imsgp_Ouvrir_connexion: SSL is _Not_ available" );
         }
                                                                             /* Connexion au serveur XMPP */
    if ( lm_connection_open_and_block (Cfg_imsgp.connection, &error) == FALSE )
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_CRIT,
                 "Imsgp_Ouvrir_connexion: Unable to connect to xmpp server %s -> %s", Cfg_imsgp.server, error->message );
       lm_connection_unref (Cfg_imsgp.connection);
       return(FALSE);
     }

    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_INFO,
              "Imsgp_Ouvrir_connexion: Connexion to xmpp server %s OK", Cfg_imsgp.server );
    if ( lm_connection_authenticate_and_block ( Cfg_imsgp.connection, Cfg_imsgp.username, Cfg_imsgp.password,
                                                "server", &error) == FALSE )
    { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_CRIT,
                "Imsgp_Ouvrir_connexion: Unable to authenticate to xmpp server -> %s", error->message );
      lm_connection_close (Cfg_imsgp.connection, NULL);
      lm_connection_unref (Cfg_imsgp.connection);
      return(FALSE);
    }

     
   Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_INFO,
             "Imsgp_Ouvrir_connexion: Authentication to xmpp server OK (%s@%s)",
             Cfg_imsgp.username, Cfg_imsgp.server );

   lm_connection_set_disconnect_function ( Cfg_imsgp.connection,                /* Fonction de deconnexion */
                                           (LmDisconnectFunction)Imsgp_Connexion_close,
                                           NULL, NULL );

   lm_connection_set_keep_alive_rate ( Cfg_imsgp.connection, 60 );       /* Ping toutes les minutes */
                                               /* Set up message handler to handle incoming text messages */
   lmMsgHandler = lm_message_handler_new( (LmHandleMessageFunction)Imsgp_Reception_message, NULL, NULL );
   lm_connection_register_message_handler( Cfg_imsgp.connection, lmMsgHandler, 
                                           LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);
   lm_message_handler_unref(lmMsgHandler);

                                           /* Set up message handler to handle incoming presence messages */
   lmMsgHandler = lm_message_handler_new( (LmHandleMessageFunction)Imsgp_Reception_presence, NULL, NULL );
   lm_connection_register_message_handler( Cfg_imsgp.connection, lmMsgHandler, 
                                           LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);
   lm_message_handler_unref(lmMsgHandler);

                                                /* Set up message handler to handle incoming contact list */
   lmMsgHandler = lm_message_handler_new( (LmHandleMessageFunction)Imsgp_Reception_contact, NULL, NULL );
   lm_connection_register_message_handler( Cfg_imsgp.connection, lmMsgHandler, 
                                           LM_MESSAGE_TYPE_IQ, LM_HANDLER_PRIORITY_NORMAL);
   lm_message_handler_unref(lmMsgHandler);
   Imsgp_Mode_presence ( NULL, "chat", "Waiting for commands" );
   return(TRUE);
 }
/**********************************************************************************************************/
/* Imsgp_Connexion_close : Appellé lorsque la connexion est fortuitement close..                           */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Imsgp_Connexion_close (LmConnection *connection, LmDisconnectReason reason, gpointer user_data)
  { switch (reason)
     { case LM_DISCONNECT_REASON_OK:
            Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                     "Imsgp_Connexion_close : Connexion lost = User requested disconnect."
                    );
            return;                          /* Dans ce cas la, il ne faut pas tenter de se reconnecter ! */
            break;
       case LM_DISCONNECT_REASON_PING_TIME_OUT:
            Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                     "Imsgp_Connexion_close : Connexion lost = Connexion to the server timed out."
                    );
            break;
       case LM_DISCONNECT_REASON_HUP:
            Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                     "Imsgp_Connexion_close : Connexion lost = The socket emitted that the connection was hung up."
                    );
            break;
       case LM_DISCONNECT_REASON_ERROR:
            Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                     "Imsgp_Connexion_close : Connexion lost = A generic error somewhere in the transport layer."
                    );
            break;
       case LM_DISCONNECT_REASON_RESOURCE_CONFLICT:
            Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                     "Imsgp_Connexion_close : Connexion lost = Another connection was made to the server with the same resource."
                    );
            break;
       case LM_DISCONNECT_REASON_INVALID_XML:
            Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                     "Imsgp_Connexion_close : Connexion lost = Invalid XML was sent from the client."
                    );
            break;
       case LM_DISCONNECT_REASON_UNKNOWN:
            Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                     "Imsgp_Connexion_close : Connexion lost = An unknown error."
                    );
            break;
       default:
            Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                     "Imsgp_Connexion_close : Connexion lost = A very unknown error."
                    );
            break;
    }
   Imsgp_Fermer_connexion();
   Cfg_imsgp.date_retente = Partage->top + TIME_RECONNECT_IMSG;
  }



/**********************************************************************************************************/
/* Main: Fonction principale du thread Imsg                                                               */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { PurpleAccount *account;
   	PurpleSavedStatus *status;
   	static int handle;

    prctl(PR_SET_NAME, "W-IMSGP", 0, 0, 0 );
    memset( &Cfg_imsgp, 0, sizeof(Cfg_imsgp) );                   /* Mise a zero de la structure de travail */
    Cfg_imsgp.lib = lib;                        /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_imsgp.lib->TID = pthread_self();                                 /* Sauvegarde du TID pour le pere */
    Imsgp_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
              "%s: Demarrage v%s . . . TID = %p", __func__, VERSION, pthread_self() );

    g_snprintf( Cfg_imsgp.lib->admin_prompt, sizeof(Cfg_imsgp.lib->admin_prompt), "imsgp" );
    g_snprintf( Cfg_imsgp.lib->admin_help,   sizeof(Cfg_imsgp.lib->admin_help),   "Manage Instant Messaging system (libpurple)" );

    if (!Cfg_imsgp.enable)
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                "%s: Thread is not enabled in config. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    Cfg_imsgp.lib->Thread_run = TRUE;                                                                   /* Le thread tourne ! */
    Cfg_imsgp.date_retente = Partage->top + 100;                                         /* On se connectera dans 10 secondes */
    MainLoop = g_main_loop_new( NULL, FALSE );

	/* libpurple's built-in DNS resolution forks processes to perform
	 * blocking lookups without blocking the main process.  It does not
	 * handle SIGCHLD itself, so if the UI does not you quickly get an army
	 * of zombie subprocesses marching around.
	 */
   	signal(SIGCHLD, SIG_IGN);

   	purple_util_set_user_dir(g_get_home_dir());
   	purple_debug_set_enabled(TRUE);
   	purple_core_set_ui_ops(&Imsgp_core_uiops);
   	if (!purple_core_init("Watchdog"))
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_ERR,
                "%s: LibPurple Init failed. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }
   	purple_plugins_add_search_path("");
   	purple_plugins_refresh();
   	purple_prefs_load();
   	purple_plugins_load_saved(PLUGIN_SAVE_PREF);

   	account = purple_account_new( Cfg_imsgp.username, "prpl-jabber" );
   	purple_account_set_password(account, Cfg_imsgp.password, NULL, NULL);

   	/* It's necessary to enable the account first. */
	   purple_account_set_enabled(account, "Watchdog", TRUE);

   	/* Now, to connect the account(s), create a status and activate it. */
	   status = purple_savedstatus_new(NULL, PURPLE_STATUS_AVAILABLE);
	   purple_savedstatus_activate(status);

	   purple_signal_connect(purple_connections_get_handle(), "signed-on", &handle,	PURPLE_CALLBACK(Imsgp_signed_on), NULL);

/*    Abonner_distribution_histo ( Imsgp_Gerer_histo );              /* Abonnement à la diffusion des histos */
    while( Cfg_imsgp.lib->Thread_run == TRUE )                            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_imsgp.lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE, "Run_thread: recu signal SIGUSR1" );
          pthread_mutex_lock ( &Cfg_imsgp.lib->synchro );
          Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                    "Run_thread: USR1 -> Nbr of IMSG to send=%d, Number of contacts=%d",
                    g_slist_length ( Cfg_imsgp.Liste_histos ),
                    g_slist_length ( Cfg_imsgp.Contacts )
                  );
          pthread_mutex_unlock ( &Cfg_imsgp.lib->synchro );
          Cfg_imsgp.lib->Thread_sigusr1 = FALSE;
        }

       if (Cfg_imsgp.reload == TRUE)
        { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE, "Run_thread: Reloading conf" );
          Imsgp_Lire_config ();                          /* Lecture de la configuration logiciel du thread */
          Cfg_imsgp.reload = FALSE;
        }

       if ( Cfg_imsgp.Liste_histos )                        /* Gestion de la listes des messages a traiter */
        { struct CMD_TYPE_HISTO *histo;
          pthread_mutex_lock ( &Cfg_imsgp.lib->synchro );
          histo = Cfg_imsgp.Liste_histos->data;
          Cfg_imsgp.Liste_histos = g_slist_remove ( Cfg_imsgp.Liste_histos, histo ); /* Retrait de la liste */
          pthread_mutex_unlock ( &Cfg_imsgp.lib->synchro );
          if (histo->alive) Imsgp_Envoi_message_to_all_available ( histo->msg.libelle );
          g_free(histo);                     /* Fin d'utilisation de la structure donc liberation memoire */
        }

       if (Cfg_imsgp.set_status)
        { Cfg_imsgp.set_status = FALSE;
          Imsgp_Mode_presence( NULL, "chat", Cfg_imsgp.new_status );
        }

       if (Cfg_imsgp.date_retente && Partage->top >= Cfg_imsgp.date_retente)
        { Cfg_imsgp.date_retente = 0;
          Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                   "Run_thread : Trying to reconnect..."
                  );
          if ( Imsgp_Ouvrir_connexion() == FALSE )
           { Cfg_imsgp.date_retente = Partage->top + TIME_RECONNECT_IMSG; }        /* Si probleme, retente */
        }

       g_main_context_iteration ( g_main_loop_get_context (MainLoop), FALSE );

     }                                                                     /* Fin du while partage->arret */
/*    Desabonner_distribution_histo ( Imsgp_Gerer_histo );       /* Desabonnement de la diffusion des histos */
    Imsgp_Fermer_connexion ();                              /* Fermeture de la connexion au serveur Jabber */
    g_main_context_unref (MainLoop);
end:
    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_imsgp.lib->Thread_run = FALSE;                                       /* Le thread ne tourne plus ! */
    Cfg_imsgp.lib->TID = 0;                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
