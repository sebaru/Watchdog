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
    Creer_configDB ( Cfg_imsgs.lib->name, "debug", "false" );

    g_snprintf( Cfg_imsgs.tech_id,  sizeof(Cfg_imsgs.tech_id ), Cfg_imsgs.lib->name );
    Creer_configDB ( Cfg_imsgs.lib->name, "tech_id", Cfg_imsgs.tech_id );

    g_snprintf( Cfg_imsgs.username, sizeof(Cfg_imsgs.username), IMSGS_DEFAUT_USERNAME );
    Creer_configDB ( Cfg_imsgs.lib->name, "username", Cfg_imsgs.username );

    g_snprintf( Cfg_imsgs.password, sizeof(Cfg_imsgs.password), IMSGS_DEFAUT_PASSWORD );
    Creer_configDB ( Cfg_imsgs.lib->name, "password", Cfg_imsgs.password );

    if ( ! Recuperer_configDB( &db, Cfg_imsgs.lib->name ) )                                          /* Connexion a la base de données */
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
       else if ( ! g_ascii_strcasecmp ( nom, "tech_id" ) )
        { g_snprintf( Cfg_imsgs.tech_id,  sizeof(Cfg_imsgs.tech_id),  "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_imsgs.lib->Thread_debug = TRUE;  }
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
                "SELECT id,username,enable,comment,notification,xmpp,allow_cde,imsg_available "
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
                "SELECT id,username,enable,comment,notification,xmpp,allow_cde,imsg_available "
                " FROM users as user WHERE enable=1 AND notification=1 AND imsg_available=1 ORDER BY username" );

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
       imsg->user_notification  = atoi(db->row[4]);
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
                "SELECT id,username,enable,comment,notification,xmpp,allow_cde,imsg_available "
                " FROM users as user WHERE enable=1 AND allow_cde=1 AND xmpp LIKE '%s' LIMIT 1", jabberid );
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
/* Imsgs_Envoi_message_to_by_json : Envoi un message json au client en parametre data                                         */
/* Entrée : Le tableau, l'index, l'element json et le destinataire                                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Imsgs_Envoi_message_to_by_json ( JsonArray *array, guint index, JsonNode *element, void *data )
  { gchar *from = data;
    gchar *tech_id = Json_get_string ( element, "tech_id" );
    gchar *acro    = Json_get_string ( element, "acronyme" );
    gchar *libelle = Json_get_string ( element, "libelle" );
    gchar *map_tag = Json_get_string ( element, "map_tag" );
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_INFO, "%s: Match found '%s' '%s:%s' - %s", __func__,
              map_tag, tech_id, acro, libelle );
    Imsgs_Envoi_message_to( from, map_tag );
  }
/******************************************************************************************************************************/
/* Imsgs_Envoi_message_to_by_json : Envoi un message json au client en parametre data                                         */
/* Entrée : Le tableau, l'index, l'element json et le destinataire                                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Imsgs_Envoyer_commande_dls_data ( JsonArray *array, guint index, JsonNode *element, void *data )
  { gchar *from = data;
    gchar *tech_id = Json_get_string ( element, "tech_id" );
    gchar *acro    = Json_get_string ( element, "acronyme" );
    gchar *libelle = Json_get_string ( element, "libelle" );
    gchar *map_tag = Json_get_string ( element, "map_tag" );
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_INFO, "%s: Match found from '%s' -> '%s' '%s:%s' - %s", __func__,
              from, map_tag, tech_id, acro, libelle );
    Envoyer_commande_dls_data ( tech_id, acro );
  }
/******************************************************************************************************************************/
/* Imsgs_handle_message_CB : CB appellé lorsque l'on recoit un message xmpp                                                   */
/* Entrée : Le Handler, la connexion, le message                                                                              */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static int Imsgs_handle_message_CB (xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata)
  { struct IMSGSDB *imsg;
    const char *from;
    gchar *message;

    from = xmpp_stanza_get_attribute    ( stanza, "from" );
    message = xmpp_message_get_body     ( stanza );
    if (!from || !message)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING, "%s: Error : from or message = NULL", __func__ );
       return(1);
     }

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

    JsonNode *RootNode = Json_node_create();
    if ( RootNode == NULL )
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_ERR, "%s : Memory Error for '%s'", __func__, from );
       goto end;
     }
    SQL_Select_to_json_node ( RootNode, "results",
                              "SELECT * FROM mnemos_DI WHERE map_thread='COMMAND_TEXT' AND map_tag LIKE '%%%s%%'", message );

    if ( Json_has_member ( RootNode, "nbr_results" ) == FALSE )
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_ERR, "%s: Error searching Database for '%s'", __func__, message );
       Imsgs_Envoi_message_to( from, "Error searching Database .. Sorry .." );
       goto end;
     }

    gint nbr_results = Json_get_int ( RootNode, "nbr_results" );
    if ( nbr_results == 0 )
     { Imsgs_Envoi_message_to( from, "Je n'ai pas trouvé, désolé." ); }
    else if ( nbr_results > 1 )                                              /* Si trop d'enregistrement, demande de préciser */
     { Imsgs_Envoi_message_to( from, "Aîe, plusieurs choix sont possibles ... :" );
       Json_node_foreach_array_element ( RootNode, "results", Imsgs_Envoi_message_to_by_json, from );
     }
    else
     { Json_node_foreach_array_element ( RootNode, "results", Imsgs_Envoyer_commande_dls_data, from ); }
end:
    json_node_unref( RootNode );
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

    g_snprintf( hostonly, sizeof(hostonly), "%s", jabber_id );
    ptr = strstr( hostonly, "/" );
    if (ptr) *ptr=0;

    jabberid = Normaliser_chaine ( hostonly );
    if (!jabberid)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation jabberid '%s' impossible", __func__, hostonly );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE users SET imsg_available='%d' WHERE xmpp='%s'", available, jabberid );
    g_free(jabberid);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING, "%s: Database Connection Failed", __func__ );
       return;
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )                                         /* Execution de la requete SQL */
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING, "%s: Requete failed", __func__ );
     }
    else { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_DEBUG,
                    "%s : jabber_id %s -> Availability updated to %d.", __func__, jabber_id, available );
         }
    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Imsgs_set_presence: Défini le statut présenté aux partenaires                                                              */
/* Entrée : le statut au format chaine de caratères                                                                           */
/******************************************************************************************************************************/
  static void Imsgs_set_presence ( const char *status_to_send )
  { xmpp_stanza_t *pres = xmpp_presence_new(Cfg_imsgs.ctx);

    xmpp_stanza_t *show = xmpp_stanza_new(Cfg_imsgs.ctx);
    xmpp_stanza_set_name  ( show,"show" ) ;
    xmpp_stanza_t *show_text = xmpp_stanza_new(Cfg_imsgs.ctx);
    xmpp_stanza_set_text  ( show_text, "chat" );
    xmpp_stanza_add_child ( show, show_text ) ;
    xmpp_stanza_add_child ( pres, show ) ;

    xmpp_stanza_t *status = xmpp_stanza_new(Cfg_imsgs.ctx);
    xmpp_stanza_set_name  ( status,"status" ) ;
    xmpp_stanza_t *status_text = xmpp_stanza_new(Cfg_imsgs.ctx);
    xmpp_stanza_set_text  ( status_text, status_to_send );
    xmpp_stanza_add_child ( status, status_text ) ;
    xmpp_stanza_add_child ( pres, status ) ;

    gchar *buf; size_t buflen;
    xmpp_stanza_to_text ( pres, &buf, &buflen );
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s: '%s'", __func__, buf );
    xmpp_free(Cfg_imsgs.ctx, buf);

    xmpp_send(Cfg_imsgs.conn, pres);
    xmpp_stanza_release(pres);
  }
/******************************************************************************************************************************/
/* Imsgs_handle_presence_CB: appellé par libstrophe lors d'un changement de statut d'un partenaire                            */
/* Entrée : les infos de la librairie                                                                                         */
/******************************************************************************************************************************/
 static int Imsgs_handle_presence_CB ( xmpp_conn_t *const conn, xmpp_stanza_t *const stanza, void *const userdata )
  { const char *type, *from;
    type = xmpp_stanza_get_type ( stanza );
    from = xmpp_stanza_get_from ( stanza );
    if (type && !strcmp(type,"unavailable") ) Imsgs_Sauvegarder_statut_contact ( from, FALSE );
    else Imsgs_Sauvegarder_statut_contact ( from, TRUE );

    if (type && !strcmp(type,"subscribe"))                            /* Demande de souscription de la part d'un utilisateur */
     { xmpp_stanza_t *pres;
       pres = xmpp_presence_new(Cfg_imsgs.ctx);
       xmpp_stanza_set_to  ( pres, from );
       xmpp_stanza_set_type( pres, "subscribed" );
       xmpp_send(Cfg_imsgs.conn, pres);
       xmpp_stanza_release(pres);
       pres = xmpp_presence_new(Cfg_imsgs.ctx);
       xmpp_stanza_set_to  ( pres, Cfg_imsgs.username );
       xmpp_stanza_set_type( pres, "subscribe" );
       xmpp_send(Cfg_imsgs.conn, pres);
       xmpp_stanza_release(pres);
     }
    gchar *buf; size_t buflen;
    xmpp_stanza_to_text ( stanza, &buf, &buflen );
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s: '%s'", __func__, buf );
    xmpp_free(Cfg_imsgs.ctx, buf);

    return(1);
  }
/******************************************************************************************************************************/
/* Imsgs_connexion_CB: appellé par libstrophe lors d'un changement de statut de la connexion                                  */
/* Entrée : les infos de la librairie                                                                                         */
/******************************************************************************************************************************/
 static void Imsgs_connexion_CB ( xmpp_conn_t *const conn, const xmpp_conn_event_t status, const int error,
                                  xmpp_stream_error_t *const stream_error, void *const userdata )
  { if (status == XMPP_CONN_CONNECT)
     {
       Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s: Account '%s' connected and %s secure",
                 __func__, Cfg_imsgs.username, (xmpp_conn_is_secured (conn) ? "IS" : "IS NOT") );
       xmpp_handler_add ( Cfg_imsgs.conn, Imsgs_handle_message_CB,  NULL, "message",  NULL, NULL );
       xmpp_handler_add ( Cfg_imsgs.conn, Imsgs_handle_presence_CB, NULL, "presence", NULL, NULL );
       /*xmpp_handler_add   ( Cfg_imsgs.conn, Imsgs_test, NULL, NULL, NULL, NULL );*/

       Imsgs_set_presence( "A votre écoute !" );
       Imsgs_Envoi_message_to_all_available ( "Instance démarrée. A l'écoute !" );
     }
    else
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s: Account '%s' disconnected",
                 __func__, Cfg_imsgs.username );
       Cfg_imsgs.signed_off = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du thread Imsg                                                                                   */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct ZMQUEUE *zmq_from_bus;
    gint retour;

reload:
    memset( &Cfg_imsgs, 0, sizeof(Cfg_imsgs) );                                     /* Mise a zero de la structure de travail */
    Cfg_imsgs.lib = lib;                                           /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "imsgs", "USER", lib, WTD_VERSION, "Manage Instant Messaging system (libstrophe)" );
    Imsgs_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */
    if (Config.instance_is_master==FALSE)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE,
                "%s: Instance is not Master. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    Cfg_imsgs.lib->Thread_run = TRUE;                                                                   /* Le thread tourne ! */
    zmq_from_bus           = Zmq_Connect ( ZMQ_SUB, "listen-to-bus",  "inproc", ZMQUEUE_LOCAL_BUS, 0 );
reconnect:
    Cfg_imsgs.signed_off = FALSE;
    xmpp_initialize();
    Cfg_imsgs.ctx  = xmpp_ctx_new(NULL, xmpp_get_default_logger(XMPP_LEVEL_INFO));
    if (!Cfg_imsgs.ctx)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_ERR, "%s: Ctx Init failed", __func__ ); }

    Cfg_imsgs.conn = xmpp_conn_new(Cfg_imsgs.ctx);
    if (!Cfg_imsgs.conn)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_ERR, "%s: Connection New failed", __func__ ); }

    xmpp_conn_set_keepalive(Cfg_imsgs.conn, 60, 1);
    xmpp_conn_set_jid (Cfg_imsgs.conn, Cfg_imsgs.username);
    xmpp_conn_set_pass(Cfg_imsgs.conn, Cfg_imsgs.password);

    retour = xmpp_connect_client ( Cfg_imsgs.conn, NULL, 0, Imsgs_connexion_CB, NULL );
    if ( retour != XMPP_EOK)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_ERR,
                             "%s: Connexion failed with error %d for '%s'", __func__, retour, Cfg_imsgs.username );
       Cfg_imsgs.signed_off = TRUE;
     }
    else
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_INFO, "%s: Connexion to '%s' in progress.", __func__, Cfg_imsgs.username ); }

    while(lib->Thread_run == TRUE && Cfg_imsgs.signed_off == FALSE && lib->Thread_reload == FALSE)/* On tourne tant que necessaire */
     { /*g_usleep(200000);*/
       sched_yield();

       xmpp_run_once ( Cfg_imsgs.ctx, 500 ); /* En milliseconde */

/********************************************************* Envoi de SMS *******************************************************/
       JsonNode *request;
       while ( (request = Thread_Listen_to_master ( lib ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "DLS_HISTO" ) &&
               Json_get_bool ( request, "alive" ) == TRUE )
           { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_NOTICE, "%s : Sending msg '%s:%s' (%s)", __func__,
                       Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ),
                       Json_get_string ( request, "libelle" ) );
             gchar chaine[256];
             g_snprintf( chaine, sizeof(chaine), "%s : %s", Json_get_string ( request, "dls_shortname" ), Json_get_string ( request, "libelle" ) );
             Imsgs_Envoi_message_to_all_available ( chaine );
           }
          else
           { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_DEBUG, "%s : zmq_tag '%s' not for this thread", __func__, zmq_tag ); }
          json_node_unref(request);
        }
     }                                                                                         /* Fin du while partage->arret */
    xmpp_disconnect(Cfg_imsgs.conn);
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_DEBUG, "%s: Disconnect OK", __func__ );
    xmpp_conn_release(Cfg_imsgs.conn);
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_DEBUG, "%s: Connection Release OK", __func__ );
    xmpp_ctx_free(Cfg_imsgs.ctx);
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_DEBUG, "%s: Ctx Free OK", __func__ );
    xmpp_shutdown();
    Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_DEBUG, "%s: XMPPshutdown OK", __func__ );

    if (Cfg_imsgs.lib->Thread_run == TRUE && Cfg_imsgs.signed_off == TRUE)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_ERR, "%s: Account signed off. Why ?? Reconnect in 5s!", __func__ );
       Cfg_imsgs.signed_off = FALSE;
       sleep(5);
       goto reconnect;
     }

    Zmq_Close ( zmq_from_bus );

end:
    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
