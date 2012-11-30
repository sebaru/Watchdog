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
 #include "Imsg.h"

 static GMainContext *MainLoop;                             /* Contexte pour attendre les evenements xmpp */
 static void Imsg_Fermer_connexion ( void );
 static gboolean Imsg_Ouvrir_connexion ( void );
 static void Imsg_Connexion_close (LmConnection *connection, LmDisconnectReason reason, gpointer user_data);

/**********************************************************************************************************/
/* Imsg_Lire_config : Lit la config Watchdog et rempli la structure mémoire                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Imsg_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Imsg_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_imsg.lib->Thread_debug = g_key_file_get_boolean ( gkf, "IMSG", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */
    chaine = g_key_file_get_string ( gkf, "IMSG", "username", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_imsg.lib->Thread_debug, LOG_ERR,
                  "Imsg_Lire_config: username is missing. Using default." );
       g_snprintf( Cfg_imsg.username, sizeof(Cfg_imsg.username), DEFAUT_USERNAME_IMSG );
     }
    else
     { g_snprintf( Cfg_imsg.username, sizeof(Cfg_imsg.username), "%s", chaine );
       g_free(chaine);
     }

    chaine = g_key_file_get_string ( gkf, "IMSG", "server", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_imsg.lib->Thread_debug, LOG_ERR,
                  "Imsg_Lire_config: server is missing. Using default." );
       g_snprintf( Cfg_imsg.server, sizeof(Cfg_imsg.username), DEFAUT_SERVER_IMSG );
     }
    else
     { g_snprintf( Cfg_imsg.server, sizeof(Cfg_imsg.username), "%s", chaine );
       g_free(chaine);
     }

    chaine = g_key_file_get_string ( gkf, "IMSG", "password", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_imsg.lib->Thread_debug, LOG_ERR,
                  "Imsg_Lire_config: password is missing. Using default." );
       g_snprintf( Cfg_imsg.password, sizeof(Cfg_imsg.password), DEFAUT_PASSWORD_IMSG );
     }
    else
     { g_snprintf( Cfg_imsg.password, sizeof(Cfg_imsg.password), "%s", chaine );
       g_free(chaine);
     }

    Cfg_imsg.recipients = g_key_file_get_string_list ( gkf, "IMSG", "recipients", NULL, NULL );
    if (!Cfg_imsg.recipients)
     { Info_new ( Config.log, Cfg_imsg.lib->Thread_debug, LOG_ERR,
                  "Imsg_Lire_config: recipients are missing." );
     }

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Imsg_Liberer_config : Libere la mémoire allouer précédemment pour lire la config imsg                  */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Imsg_Liberer_config ( void )
  { g_strfreev ( Cfg_imsg.recipients );
  }
/**********************************************************************************************************/
/* Imsg_Gerer_message: Fonction d'abonné appellé lorsqu'un message est disponible.                        */
/* Entrée: une structure CMD_TYPE_HISTO                                                                   */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Imsg_Gerer_message ( struct CMD_TYPE_MESSAGE *msg )
  { gint taille;

    pthread_mutex_lock( &Cfg_imsg.lib->synchro );                        /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_imsg.Messages );
    pthread_mutex_unlock( &Cfg_imsg.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_WARNING,
                "Imsg_Gerer_message: DROP message %d (length = %d > 150)", msg->num, taille);
       g_free(msg);
       return;
     }

    pthread_mutex_lock ( &Cfg_imsg.lib->synchro );
    Cfg_imsg.Messages = g_slist_append ( Cfg_imsg.Messages, msg );                    /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_imsg.lib->synchro );
  }
/**********************************************************************************************************/
/* Imsg_Sauvegarder_statut_contact : Sauvegarde en mémoire le statut du contact en paremetre              */
/* Entrée: le contact et le statut                                                                        */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Imsg_Liberer_liste_contacts ( void )
  { 
    pthread_mutex_lock ( &Cfg_imsg.lib->synchro );
    while(Cfg_imsg.contacts)
     { struct IMSG_CONTACT *contact;
       contact  =(struct IMSG_CONTACT *)Cfg_imsg.contacts->data;
       Cfg_imsg.contacts = g_slist_remove ( Cfg_imsg.contacts, contact );             /* Ajout a la liste */
       g_free(contact);
     }
    pthread_mutex_unlock ( &Cfg_imsg.lib->synchro );
  }
/**********************************************************************************************************/
/* Imsg_Sauvegarder_statut_contact : Sauvegarde en mémoire le statut du contact en paremetre              */
/* Entrée: le contact et le statut                                                                        */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Imsg_Sauvegarder_statut_contact ( const gchar *nom, gboolean available )
  { struct IMSG_CONTACT *contact;
    gboolean found;
    GSList *liste;

    found = FALSE;
    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_DEBUG,
              "Imsg_Sauvegarder_statut_contact : searching for user %s", nom );
    pthread_mutex_lock ( &Cfg_imsg.lib->synchro );
    liste = Cfg_imsg.contacts;
    while(liste)
     { contact  =(struct IMSG_CONTACT *)liste->data;
       if ( ! strcmp( contact->nom, nom ) )
        { contact->available = available; 
          Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_DEBUG,
                    "Imsg_Sauvegarder_statut_contact : user %s found in list. Availability updated to %d.",
                    nom, available );
          found = TRUE;
          break;
        }
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Cfg_imsg.lib->synchro );
    if (found==TRUE) return;

    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_DEBUG,
              "Imsg_Sauvegarder_statut_contact : user %s(availability=%d) not found in list. Prepending...",
              nom, available );
                                       /* Si on arrive la, c'est que le contact n'est pas dans la liste ! */
    contact = (struct IMSG_CONTACT *)g_try_malloc0( sizeof(struct IMSG_CONTACT) );
    if (!contact) return;
    g_snprintf( contact->nom, sizeof(contact->nom), "%s", nom );
    contact->available = available;
    pthread_mutex_lock ( &Cfg_imsg.lib->synchro );
    Cfg_imsg.contacts = g_slist_prepend ( Cfg_imsg.contacts, contact );               /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_imsg.lib->synchro );
  }
/**********************************************************************************************************/
/* Imsg_recipient_authorized : Renvoi TRUE si Watchdog peut envoyer au destinataire en parametre          */
/* Entrée: le nom du destinataire                                                                         */
/* Sortie : booléen, TRUE/FALSE                                                                           */
/**********************************************************************************************************/
 static gboolean Imsg_recipient_authorized ( const gchar *nom )
  { gchar **liste;
    gint cpt = 0;
    liste = Cfg_imsg.recipients;
    while (liste[cpt])
     { if ( ! strncmp ( nom, liste[cpt], strlen(liste[cpt]) ) ) return(TRUE);
       cpt++;
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Mode_presence : Change la presence du server watchdog aupres du serveur XMPP                           */
/* Entrée: la connexion xmpp                                                                              */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Imsg_Mode_presence ( gchar *type, gchar *show, gchar *status )
  { LmMessage *m;
    GError *error;

    m = lm_message_new ( NULL, LM_MESSAGE_TYPE_PRESENCE );
    if (type)   lm_message_node_set_attribute (m->node, "type", type );
    if (show)   lm_message_node_add_child (m->node, "show", show );
    if (status) lm_message_node_add_child (m->node, "status", status );
    if (!lm_connection_send (Cfg_imsg.connection, m, &error)) 
     { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_CRIT,
                 "Imsg_Mode_presence: Unable to send presence %s / %s / %s -> %s",
                 type, show, status, error->message );
     }
    lm_message_unref (m);
  }
/**********************************************************************************************************/
/* Imsg_Envoi_message_to : Envoi un message a un contact xmpp                                             */
/* Entrée: le nom du destinataire et le message                                                           */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Imsg_Envoi_message_to ( const gchar *dest, gchar *message )
  { GError *error;
    LmMessage *m;

    if( ! Imsg_recipient_authorized ( dest ) ) return;

    m = lm_message_new ( dest, LM_MESSAGE_TYPE_MESSAGE );
    lm_message_node_add_child (m->node, "body", message );
    if (!lm_connection_send (Cfg_imsg.connection, m, &error)) 
     { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_CRIT,
                 "Envoi_message_to_ismg: Unable to send message %s to %s -> %s", message, dest, error->message );
     }
    lm_message_unref (m);
  }
/**********************************************************************************************************/
/* Imsg_Envoi_message_to_all_available : Envoi un message aux contacts disponible                         */
/* Entrée: le message                                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Imsg_Envoi_message_to_all_available ( gchar *message )
  { GSList *liste;
    pthread_mutex_lock ( &Cfg_imsg.lib->synchro );
    liste = Cfg_imsg.contacts;
    while(liste)
     { struct IMSG_CONTACT *contact;
       contact = (struct IMSG_CONTACT *)liste->data;
       if ( contact->available == TRUE )
        { Imsg_Envoi_message_to ( contact->nom, message ); }
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Cfg_imsg.lib->synchro );
  }
/**********************************************************************************************************/
/* Imsg_Reception_message : CB appellé lorsque l'on recoit un message xmpp                                */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static LmHandlerResult Imsg_Reception_message ( LmMessageHandler *handler, LmConnection *connection,
                                                 LmMessage *message, gpointer data )
  { LmMessageNode *node, *body;
    const gchar *from;
    struct DB *db;

    node = lm_message_get_node ( message );

    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_DEBUG,
              "Imsg_Reception_message : recu un msg xmpp : value = %s attr = %s", 
              lm_message_node_get_value ( node ),
              lm_message_node_to_string ( node )
            );

    body = lm_message_node_find_child ( node, "body" );
    if (!body) return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);

    from = lm_message_node_get_attribute ( node, "from" );
    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
              "Imsg_Reception_message : recu un msg xmpp de %s : %s", 
              from, lm_message_node_get_value ( body )
            );

                                 /* On drop les messages du serveur jabber et des interlocuteurs inconnus */
    if ( (!from) || (!strncmp ( from, Cfg_imsg.username, strlen(Cfg_imsg.username) )) )
     { return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS); }

    if ( Imsg_recipient_authorized ( from ) == FALSE )
     { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_WARNING,
                "Imsg_Reception_message : unknown sender %s. Dropping message...", from );
       return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);
     }

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_WARNING,
                 "Imsg_Reception_message : Connexion DB failed. imsg not handled" );
       return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
     }
    if ( ! Recuperer_mnemoDB_by_command_text ( Config.log, db, (gchar *)lm_message_node_get_value ( body ) ) )
     { Imsg_Envoi_message_to( from, "Error searching Database .. Sorry .." ); }   
    else 
     { struct CMD_TYPE_MNEMONIQUE *mnemo, *result_mnemo;
          
       if ( db->nbr_result == 0 )                         /* Si pas d'enregistrement, demande de préciser */
        { Imsg_Envoi_message_to( from, "Error... No result found .. Sorry .." ); }   
       if ( db->nbr_result > 1 )                         /* Si trop d'enregistrement, demande de préciser */
        { Imsg_Envoi_message_to( from, " Need to choose ... :" ); }

       for ( result_mnemo = NULL ; ; )
        { mnemo = Recuperer_mnemoDB_suite( Config.log, db );
          if (!mnemo) break;

          if (db->nbr_result>1) Imsg_Envoi_message_to( from, mnemo->command_text );
          if (db->nbr_result!=1) g_free(mnemo);
                            else result_mnemo = mnemo;
        }
       if (result_mnemo)
        { gchar chaine[80];
          switch ( result_mnemo->type )
           { case MNEMO_MONOSTABLE:                                      /* Positionnement du bit interne */
                  g_snprintf( chaine, sizeof(chaine), "Mise a un du bit M%03d", result_mnemo->num );
                  Imsg_Envoi_message_to( from, chaine );
                  Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                             "Imsg_Reception_message: Mise a un du bit M%03d = 1", result_mnemo->num );
                  Envoyer_commande_dls(result_mnemo->num); 
                  break;
             case MNEMO_ENTREE:
                  g_snprintf( chaine, sizeof(chaine), " Result = %d", E(result_mnemo->num) );
                  Imsg_Envoi_message_to( from, chaine );
                  break;
             case MNEMO_ENTREE_ANA:
                  g_snprintf( chaine, sizeof(chaine), " Result = %f", EA_ech(result_mnemo->num) );
                  Imsg_Envoi_message_to( from, chaine );
                  break;
             default: g_snprintf( chaine, sizeof(chaine), "Cannot handle command... Check Mnemo !" );
                      Imsg_Envoi_message_to( from, chaine );
                      Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                               "Imsg_Reception_message: Cannot handle commande type %d (num=%03d)",
                                result_mnemo->type, result_mnemo->num );
                      break;
           }
          g_free(result_mnemo);
        }
     }
    Libere_DB_SQL( Config.log, &db );
    return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
  }
/**********************************************************************************************************/
/* Imsg_Reception_message : CB appellé lorsque l'on recoit un message de type presence                    */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static LmHandlerResult Imsg_Reception_presence ( LmMessageHandler *handler, LmConnection *connection,
                                                  LmMessage *message, gpointer data )
  { LmMessageNode *node_presence, *node_show, *node_status;
    const gchar *type, *from, *show, *status;
    LmMessage *m;
    GError *error;

    node_presence = lm_message_get_node ( message );
    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_DEBUG,
              "Imsg_Reception_presence : recu un msg presence : string= %s", 
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

    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_INFO,
              "Imsg_Reception_presence: Recu type=%s, show=%s, status=%s from %s", type, show, status, from );

    if ( type &&  ( ! strcmp ( type, "subscribe" ) ) ) /* Demande de subscription à notre status presence */
     { m = lm_message_new ( from, LM_MESSAGE_TYPE_PRESENCE );
       lm_message_node_set_attribute ( m->node, "type", "subscribed" );
       if (!lm_connection_send (Cfg_imsg.connection, m, &error)) 
        { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_WARNING,
                    "Imsg_Reception_presence: Unable send subscribed to %s -> %s", from, error->message );
        }
       else
        { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_INFO,
                    "Imsg_Reception_presence: Sending Subscribed OK to %s", from );
        }
       lm_message_unref (m);
                                                       /* Demande de subscription au status du partenaire */
       m = lm_message_new ( from, LM_MESSAGE_TYPE_PRESENCE );
       lm_message_node_set_attribute (m->node, "type", "subscribe" );
       if (!lm_connection_send (Cfg_imsg.connection, m, &error)) 
        { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_WARNING,
                    "Imsg_Reception_presence: Unable send subscribe to %s -> %s", from, error->message );
        }
       else
        { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_INFO,
                    "Imsg_Reception_presence: Sending Subscribe OK to %s", from );
        }
       lm_message_unref (m);

       Imsg_Sauvegarder_statut_contact ( from, FALSE );         /* Par défaut, le contact est unavailable */
       return(LM_HANDLER_RESULT_REMOVE_MESSAGE);
     }
    else if ( type && ( ! strcmp ( type, "unavailable" ) ) )    /* Gestion de la deconnexion des contacts */
     { Imsg_Sauvegarder_statut_contact ( from, FALSE );                     /* Le contact est unavailable */
     }
    else if ( ! type )                                            /* Gestion de la connexion des contacts */
     { Imsg_Sauvegarder_statut_contact ( from, TRUE );                      /* Le contact est unavailable */
     }
    return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);
  }
/**********************************************************************************************************/
/* Imsg_Reception_message : CB appellé lorsque l'on recoit un message xmpp de type IQ                     */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static LmHandlerResult Imsg_Reception_contact ( LmMessageHandler *handler, LmConnection *connection,
                                                 LmMessage *message, gpointer data )
  { LmMessageNode *node_iq;
    node_iq = lm_message_get_node ( message );
    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_DEBUG,
              "Imsg_Reception_contact : recu un msg xmpp : iq = %s", 
              lm_message_node_to_string (node_iq)
            );
    return(LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS);
  }
/**********************************************************************************************************/
/* Imsg_Fermer_connexion : Ferme une connexion avec le serveur Jabber                                     */
/* Entrée : Void                                                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Imsg_Fermer_connexion ( void )
  { if ( ! lm_connection_is_authenticated  ( Cfg_imsg.connection ) )
     { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                 "Imsg_Fermer_connexion: Strange, connexion is not authenticated...");
     }
    else                                                           /* Fermeture de la Cfg_imsg.connection */
     { Imsg_Mode_presence( "unavailable", "xa", "Server is down" );
       lm_connection_close (Cfg_imsg.connection, NULL);
     }
    sleep(2);
                                                                  /* Destruction de la structure associée */
    lm_connection_unref (Cfg_imsg.connection);
    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_DEBUG,
              "Imsg_Fermer_connexion: Connexion is closed.");
  }
/**********************************************************************************************************/
/* Imsg_Ouvrir_connexion : Ouvre une connexion avec le serveur Jabber                                     */
/* Entrée : Void                                                                                          */
/* Sortie : TRUE ou FALSE si probleme                                                                     */
/**********************************************************************************************************/
 static gboolean Imsg_Ouvrir_connexion ( void )
  { LmMessageHandler *lmMsgHandler;
    GError       *error = NULL;

    Cfg_imsg.connection = lm_connection_new_with_context ( Cfg_imsg.server, MainLoop );
    if ( Cfg_imsg.connection == NULL )
     { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_CRIT,
                 "Imsg_Ouvrir_connexion: Error creating connection" );
       return(FALSE);
     }

    if ( Cfg_imsg.connection && (lm_ssl_is_supported () == TRUE) )
     { LmSSL *ssl;
       ssl = lm_ssl_new ( NULL, NULL, NULL, NULL );
       lm_ssl_use_starttls ( ssl, TRUE, TRUE );
       lm_connection_set_ssl ( Cfg_imsg.connection, ssl );
       lm_ssl_unref ( ssl );
       Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                 "Imsg_Ouvrir_connexion: SSL is available" );
     }
    else { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_WARNING,
                     "Imsg_Ouvrir_connexion: SSL is _Not_ available" );
         }
                                                                             /* Connexion au serveur XMPP */
    if ( lm_connection_open_and_block (Cfg_imsg.connection, &error) == FALSE )
     { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_CRIT,
                 "Imsg_Ouvrir_connexion: Unable to connect to xmpp server %s -> %s", Cfg_imsg.server, error->message );
       lm_connection_unref (Cfg_imsg.connection);
       return(FALSE);
     }

    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_INFO,
              "Imsg_Ouvrir_connexion: Connexion to xmpp server %s OK", Cfg_imsg.server );
    if ( lm_connection_authenticate_and_block ( Cfg_imsg.connection, Cfg_imsg.username, Cfg_imsg.password,
                                                "server", &error) == FALSE )
    { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_CRIT,
                "Imsg_Ouvrir_connexion: Unable to authenticate to xmpp server -> %s", error->message );
      lm_connection_close (Cfg_imsg.connection, NULL);
      lm_connection_unref (Cfg_imsg.connection);
      return(FALSE);
    }

     
   Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_INFO,
             "Imsg_Ouvrir_connexion: Authentication to xmpp server OK (%s@%s)",
             Cfg_imsg.username, Cfg_imsg.server );

   lm_connection_set_disconnect_function ( Cfg_imsg.connection,                /* Fonction de deconnexion */
                                           (LmDisconnectFunction)Imsg_Connexion_close,
                                           NULL, NULL );

   lm_connection_set_keep_alive_rate ( Cfg_imsg.connection, 60 );       /* Ping toutes les minutes */
                                               /* Set up message handler to handle incoming text messages */
   lmMsgHandler = lm_message_handler_new( (LmHandleMessageFunction)Imsg_Reception_message, NULL, NULL );
   lm_connection_register_message_handler( Cfg_imsg.connection, lmMsgHandler, 
                                           LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);
   lm_message_handler_unref(lmMsgHandler);

                                           /* Set up message handler to handle incoming presence messages */
   lmMsgHandler = lm_message_handler_new( (LmHandleMessageFunction)Imsg_Reception_presence, NULL, NULL );
   lm_connection_register_message_handler( Cfg_imsg.connection, lmMsgHandler, 
                                           LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);
   lm_message_handler_unref(lmMsgHandler);

                                                /* Set up message handler to handle incoming contact list */
   lmMsgHandler = lm_message_handler_new( (LmHandleMessageFunction)Imsg_Reception_contact, NULL, NULL );
   lm_connection_register_message_handler( Cfg_imsg.connection, lmMsgHandler, 
                                           LM_MESSAGE_TYPE_IQ, LM_HANDLER_PRIORITY_NORMAL);
   lm_message_handler_unref(lmMsgHandler);
   Imsg_Mode_presence ( NULL, "chat", "Waiting for commands" );
   return(TRUE);
 }
/**********************************************************************************************************/
/* Imsg_Connexion_close : Appellé lorsque la connexion est fortuitement close..                           */
/* Entrée : Le Handler, la connexion, le message                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Imsg_Connexion_close (LmConnection *connection, LmDisconnectReason reason, gpointer user_data)
  { switch (reason)
     { case LM_DISCONNECT_REASON_OK:
            Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                     "Imsg_Connexion_close : Connexion lost = User requested disconnect."
                    );
            return;                          /* Dans ce cas la, il ne faut pas tenter de se reconnecter ! */
            break;
       case LM_DISCONNECT_REASON_PING_TIME_OUT:
            Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                     "Imsg_Connexion_close : Connexion lost = Connexion to the server timed out."
                    );
            break;
       case LM_DISCONNECT_REASON_HUP:
            Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                     "Imsg_Connexion_close : Connexion lost = The socket emitted that the connection was hung up."
                    );
            break;
       case LM_DISCONNECT_REASON_ERROR:
            Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                     "Imsg_Connexion_close : Connexion lost = A generic error somewhere in the transport layer."
                    );
            break;
       case LM_DISCONNECT_REASON_RESOURCE_CONFLICT:
            Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                     "Imsg_Connexion_close : Connexion lost = Another connection was made to the server with the same resource."
                    );
            break;
       case LM_DISCONNECT_REASON_INVALID_XML:
            Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                     "Imsg_Connexion_close : Connexion lost = Invalid XML was sent from the client."
                    );
            break;
       case LM_DISCONNECT_REASON_UNKNOWN:
            Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                     "Imsg_Connexion_close : Connexion lost = An unknown error."
                    );
            break;
       default:
            Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                     "Imsg_Connexion_close : Connexion lost = A very unknown error."
                    );
            break;
    }
   Imsg_Fermer_connexion();
   Cfg_imsg.date_retente = Partage->top + TIME_RECONNECT_IMSG;
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Imsg                                                               */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { prctl(PR_SET_NAME, "W-IMSG", 0, 0, 0 );
    memset( &Cfg_imsg, 0, sizeof(Cfg_imsg) );                   /* Mise a zero de la structure de travail */
    Cfg_imsg.lib = lib;                        /* Sauvegarde de la structure pointant sur cette librairie */
    Imsg_Lire_config ();                                /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    Cfg_imsg.lib->Thread_run = TRUE;                                                /* Le thread tourne ! */
    Cfg_imsg.date_retente = 0;

    g_snprintf( Cfg_imsg.lib->admin_prompt, sizeof(Cfg_imsg.lib->admin_prompt), "imsg" );
    g_snprintf( Cfg_imsg.lib->admin_help,   sizeof(Cfg_imsg.lib->admin_help),   "Manage Instant Messaging system" );

    MainLoop = g_main_context_new();
                                                                 /* Preparation de la connexion au server */
    if ( Imsg_Ouvrir_connexion() == FALSE ) { Cfg_imsg.lib->Thread_run = FALSE; }      /* Arret du thread */

    Abonner_distribution_message ( Imsg_Gerer_message );        /* Abonnement à la diffusion des messages */

    while( Cfg_imsg.lib->Thread_run == TRUE )                            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_imsg.lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE, "Run_thread: recu signal SIGUSR1" );
          pthread_mutex_lock ( &Cfg_imsg.lib->synchro );
          Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                    "Run_thread: USR1 -> Nbr of IMSG to send=%d, Number of contacts=%d",
                    g_slist_length ( Cfg_imsg.Messages ),
                    g_slist_length ( Cfg_imsg.contacts )
                  );
          pthread_mutex_unlock ( &Cfg_imsg.lib->synchro );
          Cfg_imsg.lib->Thread_sigusr1 = FALSE;
        }

       if ( Cfg_imsg.Messages )                            /* Gestion de la listes des messages a traiter */
        { struct CMD_TYPE_MESSAGE *msg;
          pthread_mutex_lock ( &Cfg_imsg.lib->synchro );
          msg = Cfg_imsg.Messages->data;
          Cfg_imsg.Messages = g_slist_remove ( Cfg_imsg.Messages, msg );           /* Retrait de la liste */
          pthread_mutex_unlock ( &Cfg_imsg.lib->synchro );
          Imsg_Envoi_message_to_all_available ( msg->libelle );
          g_free(msg);                       /* Fin d'utilisation de la structure donc liberation memoire */
        }

       if (Cfg_imsg.set_status)
        { Cfg_imsg.set_status = FALSE;
          Imsg_Mode_presence( NULL, "chat", Cfg_imsg.new_status );
        }

       if (Cfg_imsg.date_retente && Partage->top >= Cfg_imsg.date_retente)
        { Cfg_imsg.date_retente = 0;
          Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE,
                   "Run_thread : Trying to reconnect..."
                  );
          if ( Imsg_Ouvrir_connexion() == FALSE )
           { Cfg_imsg.date_retente = Partage->top + TIME_RECONNECT_IMSG; }        /* Si probleme, retente */
        }

       g_main_context_iteration ( MainLoop, FALSE );

     }                                                                     /* Fin du while partage->arret */

    Desabonner_distribution_message ( Imsg_Gerer_message ); /* Desabonnement de la diffusion des messages */
    Imsg_Fermer_connexion ();                              /* Fermeture de la connexion au serveur Jabber */
    g_main_context_unref (MainLoop);
    Imsg_Liberer_config();                        /* Liberation de la configuration de l'InstantMessaging */
    Imsg_Liberer_liste_contacts();                                     /* Liberation de la liste contacts */

    Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_imsg.lib->TID = 0;                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
