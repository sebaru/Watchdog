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

 struct IMSGS_CONFIG Cfg_imsgs;
/******************************************************************************************************************************/
/* Imsgs_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                  */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Imsgs_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_imsgs.lib->Thread_debug = FALSE;                                                       /* Settings default parameters */
    Cfg_imsgs.enable            = FALSE;
    g_snprintf( Cfg_imsgs.username, sizeof(Cfg_imsgs.username), IMSGS_DEFAUT_USERNAME );
    g_snprintf( Cfg_imsgs.password, sizeof(Cfg_imsgs.password), IMSGS_DEFAUT_PASSWORD );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_INFO,                                           /* Print Config */
                "%s: '%s' = %s", __func__, nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "username" ) )
        { g_snprintf( Cfg_imsgs.username, sizeof(Cfg_imsgs.username), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "password" ) )
        { g_snprintf( Cfg_imsgs.password, sizeof(Cfg_imsgs.password), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_imsgs.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_imsgs.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Recuperer_imsgsDB: Recupération de la liste des users users IM                                                             */
/* Entrée: Un pointeur vers une database                                                                                      */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Recuperer_imsgsDB ( struct DB *db )
  { gchar requete[512];
    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,username,enable,comment,imsg_enable,imsg_jabberid,imsg_allow_cde,imsg_available "
                " FROM users as user ORDER BY username" );

    return ( Lancer_requete_SQL ( db, requete ) );                                             /* Execution de la requete SQL */
  }
/******************************************************************************************************************************/
/* Recuperer_all_available_imsgDB: Recupération de la liste des uses IM actifs                                                */
/* Entrée: Un pointeur vers une database                                                                                      */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Recuperer_all_available_imsgDB ( struct DB *db )
  { gchar requete[512];
    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,username,enable,comment,imsg_enable,imsg_jabberid,imsg_allow_cde,imsg_available "
                " FROM users as user WHERE enable=1 AND imsg_enable=1 AND imsg_available=1 ORDER BY username" );

    return ( Lancer_requete_SQL ( db, requete ) );                                             /* Execution de la requete SQL */
  }
/******************************************************************************************************************************/
/* Recuperer_imsgDB_suite: Recupération de la liste des champs des users                                                      */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct IMSGSDB *Recuperer_imsgsDB_suite( struct DB *db )
  { struct IMSGSDB *imsg;

    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( db );
       return(NULL);
     }

    imsg = (struct IMSGSDB *)g_try_malloc0( sizeof(struct IMSGSDB) );
    if (!imsg) Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
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
/* Imsgs_Envoi_message_to : Envoi un message a un contact xmpp                                                                */
/* Entrée: le nom du destinataire et le message                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_Envoi_message_to ( const gchar *dest, gchar *message )
  { xmpp_stanza_t *stanza = xmpp_message_new ( Cfg_imsgs.ctx, "normal", dest, NULL );
    xmpp_message_set_body ( stanza, message );
    xmpp_send ( Cfg_imsgs.conn , stanza ) ;
    xmpp_stanza_release ( stanza );
  }
/******************************************************************************************************************************/
/* Imsgs_recipient_allow_command : Renvoie un contact IMSGDB si delui-ci dispose du flag allow_cde                            */
/* Entrée: le jabber_id                                                                                                       */
/* Sortie: struct IMSGSDB *imsg                                                                                                */
/******************************************************************************************************************************/
 static struct IMSGSDB *Imsgs_recipient_allow_command ( gchar *jabber_id )
  { gchar *jabberid, requete[512], hostonly[80], *ptr;
    struct IMSGSDB *imsg;
    struct DB *db;

    g_snprintf( hostonly, sizeof(hostonly), "%s", jabber_id );
    ptr = strstr( hostonly, "/" );
    if (ptr) *ptr=0;

    jabberid = Normaliser_chaine ( hostonly );
    if (!jabberid)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation jabberid impossible", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,username,enable,comment,imsg_enable,imsg_jabberid,imsg_allow_cde,imsg_available "
                " FROM users as user WHERE enable=1 AND imsg_allow_cde=1 AND imsg_jabberid LIKE '%s' LIMIT 1", jabberid );
    g_free(jabberid);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING, "%s: Database Connection Failed", __func__ );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )                     /* Execution de la requete SQL */
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING, "%s: Requete failed", __func__ );
       Libere_DB_SQL( &db );
       return(NULL);
     }

    imsg = Recuperer_imsgsDB_suite ( db );
    Libere_DB_SQL( &db );
    return(imsg);
  }
/******************************************************************************************************************************/
/* Imsgs_Envoi_message_to_all_available : Envoi un message aux contacts disponibles                                           */
/* Entrée: le message                                                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Imsgs_Envoi_message_to_all_available ( gchar *message )
  { struct IMSGSDB *imsg;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING, "%s: Database Connection Failed", __func__ );
       return;
     }

/*********************************************** Chargement des informations en bases ******************************************/
    if ( ! Recuperer_all_available_imsgDB( db ) )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING, "%s: Recuperer_imsg Failed", __func__ );
       return;
     }

    while ( (imsg = Recuperer_imsgsDB_suite( db )) != NULL)
     { Imsgs_Envoi_message_to ( imsg->user_jabberid, message ); }

    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Imsgs_recevoir_imsg : CB appellé lorsque l'on recoit un message xmpp                                                       */
/* Entrée : Le Handler, la connexion, le message                                                                              */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static int Imsgs_recevoir_imsg (xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata)
  { struct IMSGSDB *imsg;
    struct DB *db;
    const char *from;
    gchar *message;

    from = xmpp_stanza_get_attribute	( stanza, "from" );
    message = xmpp_message_get_body 	( stanza );
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s: From '%s' -> '%s'", __func__, from, message );

    imsg = Imsgs_recipient_allow_command ( from );
    if ( imsg == NULL )
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING,
                "%s : unknown sender '%s' or not allow to send command. Dropping message...", __func__, from );
       goto end;
     }
    g_free(imsg);


    if ( ! strcasecmp( message, "ping" ) )                                                             /* Interfacage de test */
     { Imsgs_Envoi_message_to( from, "Pong !" );
       goto end;
     }

    if ( ! Recuperer_mnemos_DI_by_text ( &db, NOM_THREAD, message ) )
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_ERR, "%s: Error searching Database for '%s'", __func__, message );
       Imsgs_Envoi_message_to( from, "Error searching Database .. Sorry .." );
       goto end;
     }

    if ( db->nbr_result == 0 )                                                /* Si pas d'enregistrement, demande de préciser */
     { Imsgs_Envoi_message_to( from, "Error... No result found .. Sorry .." );
       Libere_DB_SQL ( &db );
     }
    else if ( db->nbr_result > 1 )                                           /* Si trop d'enregistrement, demande de préciser */
     { Imsgs_Envoi_message_to( from, " Need to choose ... :" );
       while ( Recuperer_mnemos_DI_suite( &db ) )
        { gchar *tech_id = db->row[0], *acro = db->row[1], *libelle = db->row[3], *src_text = db->row[2];
          Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_INFO, "%s: Match found '%s' '%s:%s' - %s", __func__,
                    src_text, tech_id, acro, libelle );
          Imsgs_Envoi_message_to( from, src_text );
        }
     }
    else while ( Recuperer_mnemos_DI_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *libelle = db->row[3], *src_text = db->row[2];
       Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_INFO, "%s: Match found '%s' '%s:%s' - %s", __func__,
                 src_text, tech_id, acro, libelle );
        if (Config.instance_is_master==TRUE)                                                      /* si l'instance est Maitre */
        { Envoyer_commande_dls_data ( tech_id, acro ); }
     }
end:
    xmpp_free(Cfg_imsgs.ctx, message);
    return(1);
  }
/******************************************************************************************************************************/
/* Imsgs_Sauvegarder_statut_contact : Sauvegarde en mémoire le statut du contact en parametre                                 */
/* Entrée: le contact et le statut                                                                                            */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_Sauvegarder_statut_contact ( const gchar *jabber_id, gboolean available )
  { gchar *jabberid, requete[512], hostonly[80], *ptr;
    struct DB *db;
#ifdef bouh
    g_snprintf( hostonly, sizeof(hostonly), "%s", jabber_id );
    ptr = strstr( hostonly, "/" );
    if (ptr) *ptr=0;

    jabberid = Normaliser_chaine ( hostonly );
    if (!jabberid)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation jabberid impossible", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE users SET imsg_available=%d WHERE imsg_jabberid='%s'", available, jabberid );
    g_free(jabberid);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING,
                "%s: Database Connection Failed", __func__ );
       return;
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )                                         /* Execution de la requete SQL */
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING,
                "%s: Requete failed", __func__ );
     }
    else { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_DEBUG,
                    "%s : jabber_id %s -> Availability updated to %d.", __func__, jabber_id, available );
         }
    Libere_DB_SQL( &db );
#endif
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Imsgs_buddy_signed_on : Un contact bien d'arriver                                                                          */
/* Entrée: le buddy                                                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_buddy_signed_on(PurpleBuddy *buddy)
  { Imsgs_Sauvegarder_statut_contact ( purple_buddy_get_name(buddy), TRUE ); }
/******************************************************************************************************************************/
/* Imsgs_buddy_signed_off : Un contact bien de partir                                                                         */
/* Entrée: le buddy                                                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_buddy_signed_off(PurpleBuddy *buddy)
  { Imsgs_Sauvegarder_statut_contact ( purple_buddy_get_name(buddy), FALSE ); }

/******************************************************************************************************************************/
/* Imsgs_buddy_xxx : Ensemble de fonctions de notification sur le statut des contacts                                         */
/* Entrée: le buddy                                                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_buddy_away(PurpleBuddy *buddy, PurpleStatus *old_status, PurpleStatus *status)
  { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_INFO,
             "%s: '%s' with proto %s", __func__,
              purple_buddy_get_name(buddy), purple_account_get_protocol_id(purple_buddy_get_account(buddy)));
  }

 static void Imsgs_buddy_idle(PurpleBuddy *buddy, gboolean old_idle, gboolean idle)
  { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_INFO,
             "%s: '%s' with proto %s", __func__,
              purple_buddy_get_name(buddy), purple_account_get_protocol_id(purple_buddy_get_account(buddy)));
  }

 static void Imsgs_buddy_typing(PurpleAccount *account, const char *name)
  { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_DEBUG,
             "%s: '%s' with proto %s", __func__,
              name, purple_account_get_protocol_id(account));
  }

 static void Imsgs_buddy_typed(PurpleAccount *account, const char *name) //not supported on all protocols
  { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_DEBUG,
             "%s: '%s' with proto %s", __func__,
              name, purple_account_get_protocol_id(account));
  }

 static void Imsgs_buddy_typing_stopped(PurpleAccount *account, const char *name)
  { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_DEBUG,
             "%s: '%s' with proto %s", __func__,
              name, purple_account_get_protocol_id(account));
  }
/******************************************************************************************************************************/
/* Imsgs_account_authorization_requested : Fonction appellé quand un user veut nous ajouter dans sa liste de buddy            */
/* Entrée: le compte et le user                                                                                               */
/* Sortie: 1 = OK pour ajouter                                                                                                */
/******************************************************************************************************************************/
 static int Imsgs_account_authorization_requested(PurpleAccount *account, const char *user)
  { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE,
             "%s: Buddy authorization request from '%s' for protocol '%s'", __func__,
              user, purple_account_get_protocol_id(account) );
    purple_account_add_buddy( account, purple_buddy_new 	( account, user, user ) );
    return 1; //authorize buddy request automatically (-1 denies it)
  }
/******************************************************************************************************************************/
/* Imsgs_signed_on: Appelé lorsque nous venons de nous connecter                                                              */
/* Entrée: La connextion Purple                                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_signed_on ( PurpleConnection *gc, gpointer null )
  {	PurpleAccount *account = purple_connection_get_account(gc);
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE,
             "%s: Account '%s' connected for protocol id '%s'", __func__,
              purple_account_get_username(account), purple_account_get_protocol_id(account));
  }
/******************************************************************************************************************************/
/* Imsgs_signed_off: Appelé lorsque nous venons de nous déconnecter                                                           */
/* Entrée: La connextion Purple                                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgs_signed_off ( PurpleConnection *gc, gpointer null )
  {	PurpleAccount *account = purple_connection_get_account(gc);
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE,
             "%s: Account '%s' disconnected for protocol id '%s'", __func__,
              purple_account_get_username(account), purple_account_get_protocol_id(account));
    Cfg_imsgs.signed_off = TRUE;
  }
/******************************************************************************************************************************/
/* Définition spécifique pour la librairie libpurple                                                                          */
/******************************************************************************************************************************/
 #define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
 #define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

 typedef struct _PurpleGLibIOClosure
  {	PurpleInputFunction function;
	   guint result;
	   gpointer data;
  } PurpleGLibIOClosure;

 static void purple_glib_io_destroy(gpointer data)
  {	g_free(data); }

 static gboolean purple_glib_io_invoke(GIOChannel *source, GIOCondition condition, gpointer data)
  {	PurpleGLibIOClosure *closure = data;
	   PurpleInputCondition purple_cond = 0;

	   if (condition & PURPLE_GLIB_READ_COND)	 purple_cond |= PURPLE_INPUT_READ;
	   if (condition & PURPLE_GLIB_WRITE_COND)	purple_cond |= PURPLE_INPUT_WRITE;

	   closure->function(closure->data, g_io_channel_unix_get_fd(source), purple_cond);

   	return TRUE;
  }

 static guint glib_input_add(gint fd, PurpleInputCondition condition, PurpleInputFunction function, gpointer data)
  { PurpleGLibIOClosure *closure = g_new0(PurpleGLibIOClosure, 1);
	   GIOChannel *channel;
	   GIOCondition cond = 0;

	   closure->function = function;
	   closure->data = data;

	   if (condition & PURPLE_INPUT_READ)  cond |= PURPLE_GLIB_READ_COND;
	   if (condition & PURPLE_INPUT_WRITE)	cond |= PURPLE_GLIB_WRITE_COND;

   	channel = g_io_channel_unix_new(fd);
   	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, cond, purple_glib_io_invoke, closure, purple_glib_io_destroy);

   	g_io_channel_unref(channel);
	   return closure->result;
  }

 static PurpleEventLoopUiOps glib_eventloops =
  {	g_timeout_add,	g_source_remove,	glib_input_add,	g_source_remove,
	   NULL,	g_timeout_add_seconds,
	   /* padding */
	   NULL,	NULL,	NULL
  };


#endif



#ifdef bouh

int message_handler(xmpp_conn_t *const conn,
                    xmpp_stanza_t *const stanza,
                    void *const userdata)
{
    xmpp_ctx_t *ctx = (xmpp_ctx_t *)userdata;
    xmpp_stanza_t *new_msg, *reply;
    const char *type;
    char *intext, *replytext;
    int quit = 0;

    new_msg = xmpp_stanza_get_child_by_name(stanza, "new_msg");
    if (new_msg == NULL)
        return 1;
    type = xmpp_stanza_get_type(stanza);
    if (type != NULL && strcmp(type, "error") == 0)
        return 1;

    intext = xmpp_stanza_get_text(new_msg);

    printf("Incoming message from %s: %s\n", xmpp_stanza_get_from(stanza),
           intext);

    reply = xmpp_stanza_reply(stanza);
    if (xmpp_stanza_get_type(reply) == NULL)
        xmpp_stanza_set_type(reply, "chat");

    if (strcmp(intext, "quit") == 0) {
        replytext = strdup("bye!");
        quit = 1;
    } else {
        replytext = (char *)malloc(strlen(" to you too!") + strlen(intext) + 1);
        strcpy(replytext, intext);
        strcat(replytext, " to you too!");
    }
    xmpp_free(ctx, intext);
    xmpp_message_set_new_msg(reply, replytext);

    xmpp_send(conn, reply);
    xmpp_stanza_release(reply);
    free(replytext);

    if (quit)
        xmpp_disconnect(conn);

    return 1;
}

stanza = xmpp_stanza_new ( Cfg_imsgs.ctx );
xmpp_stanza_set_name ( stanza,"message" ) ;
xmpp_stanza_set_type ( stanza,"normal" ) ;
xmpp_stanza_set_attribute ( stanza , "to" , jid ) ;
xmpp_stanza_set_attribute ( stanza , "xmlns" , "jabber:client ");

body = xmpp_stanza_new ( Cfg_imsgs.ctx ) ;
xmpp_stanza_set_name ( new_msg , "body" ) ;
text = xmpp_stanza_new ( Cfg_imsgs.ctx ) ;
xmpp_stanza_set_text ( text , "my message" ) ;
xmpp_stanza_add_child ( stanza , body ) ;
xmpp_stanza_add_child ( stanza , text ) ;
xmpp_send ( conn , stanza ) ;
xmpp_stanza_release ( stanza )
#endif
 void Imsgs_connexion_CB ( xmpp_conn_t *const conn, const xmpp_conn_event_t status, const int error,
                           xmpp_stream_error_t *const stream_error, void *const userdata )
  { if (status == XMPP_CONN_CONNECT)
     {
       Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s: Account '%s' connected and %s secure",
                 __func__, Cfg_imsgs.username, (xmpp_conn_is_secured (conn) ? "IS" : "IS NOT") );
       xmpp_handler_add	( Cfg_imsgs.conn, Imsgs_recevoir_imsg, NULL, "message", NULL, NULL );

       xmpp_stanza_t *pres = xmpp_presence_new(Cfg_imsgs.ctx);
       xmpp_send(conn, pres);
       xmpp_stanza_release(pres);

       xmpp_stanza_t *stanza = xmpp_message_new ( Cfg_imsgs.ctx, "msg type", "lefevre.seb@jabber.fr", "this is id" );
       xmpp_message_set_body ( stanza, "premier message avec libstrophe !");
       xmpp_send ( conn , stanza ) ;
       xmpp_stanza_release ( stanza );
     }
    else
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s: Account '%s' disconnected",
                  __func__, Cfg_imsgs.username );
     }
  }

/******************************************************************************************************************************/
/* Main: Fonction principale du thread Imsg                                                                                   */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct ZMQUEUE *zmq_msg;
    int handle;

    prctl(PR_SET_NAME, "W-IMSGS", 0, 0, 0 );
    memset( &Cfg_imsgs, 0, sizeof(Cfg_imsgs) );                                     /* Mise a zero de la structure de travail */
    Cfg_imsgs.lib = lib;                                           /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_imsgs.lib->TID = pthread_self();                                                    /* Sauvegarde du TID pour le pere */
    Imsgs_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE,
              "%s: Demarrage v%s . . . TID = %p", __func__, VERSION, pthread_self() );

    g_snprintf( Cfg_imsgs.lib->admin_prompt, sizeof(Cfg_imsgs.lib->admin_prompt), NOM_THREAD );
    g_snprintf( Cfg_imsgs.lib->admin_help,   sizeof(Cfg_imsgs.lib->admin_help),   "Manage Instant Messaging system (libstrophe)" );

    if (!Cfg_imsgs.enable)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE,
                "%s: Thread is not enabled in config. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    Cfg_imsgs.lib->Thread_run = TRUE;                                                                   /* Le thread tourne ! */
    zmq_msg = Connect_zmq ( ZMQ_SUB, "listen-to-msgs", "inproc", ZMQUEUE_LIVE_MSGS, 0 );

reconnect:
    xmpp_initialize();
    Cfg_imsgs.ctx  = xmpp_ctx_new(NULL, xmpp_get_default_logger(XMPP_LEVEL_DEBUG));
    Cfg_imsgs.conn = xmpp_conn_new(Cfg_imsgs.ctx);
    xmpp_conn_set_keepalive(Cfg_imsgs.conn, 60, 1);
    xmpp_conn_set_jid (Cfg_imsgs.conn, Cfg_imsgs.username);
    xmpp_conn_set_pass(Cfg_imsgs.conn, Cfg_imsgs.password);

/*    if (!purple_core_init("Watchdog"))
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_ERR,
                "%s: LibPurple Init failed. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }
*/
    xmpp_connect_client ( Cfg_imsgs.conn, NULL, 0, Imsgs_connexion_CB, NULL );

    while( Cfg_imsgs.lib->Thread_run == TRUE && Cfg_imsgs.signed_off == FALSE)               /* On tourne tant que necessaire */
     { struct CMD_TYPE_HISTO *histo, histo_buf;
       /*g_usleep(200000);*/
       sched_yield();

       xmpp_run_once ( Cfg_imsgs.ctx, 500 ); /* En milliseconde */
       if (Cfg_imsgs.lib->Thread_reload == TRUE)
        { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s: recu signal SIGUSR1", __func__ );
          Imsgs_Lire_config ();                                             /* Lecture de la configuration logiciel du thread */
          Cfg_imsgs.lib->Thread_reload = FALSE;
        }

/*       if ( Recv_zmq ( zmq_msg, &histo_buf, sizeof(struct CMD_TYPE_HISTO) ) == sizeof(struct CMD_TYPE_HISTO) )
        { histo = &histo_buf;
          gchar chaine[256];
          if (histo->alive)
           { g_snprintf( chaine, sizeof(chaine), "%s : %s", histo->msg.dls_shortname, histo->msg.libelle );
             Imsgs_Envoi_message_to_all_available ( chaine );
           }
        }

       g_main_context_iteration ( g_main_loop_get_context (MainLoop), FALSE );
*/
     }                                                                                         /* Fin du while partage->arret */
    xmpp_disconnect(Cfg_imsgs.conn);
    xmpp_conn_release(Cfg_imsgs.conn);
    xmpp_ctx_free(Cfg_imsgs.ctx);
    xmpp_shutdown();

    if (Cfg_imsgs.lib->Thread_run == TRUE && Cfg_imsgs.signed_off == TRUE)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s: Account signed off. Why ?? Reconnect !", __func__ );
       Cfg_imsgs.signed_off = FALSE;
       goto reconnect;
     }

    Close_zmq ( zmq_msg );
end:
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    Cfg_imsgs.lib->Thread_run = FALSE;                                                          /* Le thread ne tourne plus ! */
    Cfg_imsgs.lib->TID = 0;                                                   /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
