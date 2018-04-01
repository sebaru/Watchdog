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
 
 #include <sys/prctl.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Imsg.h"

 static GMainLoop *MainLoop;                                                    /* Contexte pour attendre les evenements xmpp */
 static PurpleCoreUiOps Imsgp_core_uiops =
  {	NULL,	NULL, NULL, NULL, /* padding */	NULL, NULL, NULL,	NULL };

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
    g_snprintf( Cfg_imsgp.username, sizeof(Cfg_imsgp.username), IMSGP_DEFAUT_USERNAME );
    g_snprintf( Cfg_imsgp.password, sizeof(Cfg_imsgp.password), IMSGP_DEFAUT_PASSWORD );

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
/******************************************************************************************************************************/
/* Recuperer_imsgpDB: Recupération de la liste des users users IM                                                             */
/* Entrée: Un pointeur vers une database                                                                                      */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Recuperer_imsgpDB ( struct DB *db )
  { gchar requete[512];
    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,name,enable,comment,imsg_enable,imsg_jabberid,imsg_allow_cde,imsg_available "
                " FROM %s as user ORDER BY user.name",
                NOM_TABLE_UTIL );

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
                "SELECT id,name,enable,comment,imsg_enable,imsg_jabberid,imsg_allow_cde,imsg_available "
                " FROM %s as user WHERE enable=1 AND imsg_enable=1 AND imsg_available=1 ORDER BY user.name",
                NOM_TABLE_UTIL );

    return ( Lancer_requete_SQL ( db, requete ) );                                             /* Execution de la requete SQL */
  }
/******************************************************************************************************************************/
/* Recuperer_imsgDB_suite: Recupération de la liste des champs des users                                                      */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct IMSGPDB *Recuperer_imsgpDB_suite( struct DB *db )
  { struct IMSGPDB *imsg;

    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( db );
       return(NULL);
     }

    imsg = (struct IMSGPDB *)g_try_malloc0( sizeof(struct IMSGPDB) );
    if (!imsg) Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
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
/* Imsgp_Envoi_message_to : Envoi un message a un contact xmpp                                                                */
/* Entrée: le nom du destinataire et le message                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Imsgp_Envoi_message_to ( const gchar *dest, gchar *message )
  { PurpleConversation *conv;

   	conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, dest, Cfg_imsgp.account);
   	if (conv == NULL) { conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, Cfg_imsgp.account, dest); }

    PurpleConvIm *im = PURPLE_CONV_IM(conv);
    purple_conv_im_set_typing_state(im, PURPLE_TYPING);
    purple_conv_im_send(im, message);
    purple_conv_im_set_typing_state(im, PURPLE_NOT_TYPING);
  }
/******************************************************************************************************************************/
/* Imsgp_recipient_allow_command : Renvoie un contact IMSGDB si delui-ci dispose du flag allow_cde                            */
/* Entrée: le jabber_id                                                                                                       */
/* Sortie: struct IMSGPDB *imsg                                                                                                */
/******************************************************************************************************************************/
 static struct IMSGPDB *Imsgp_recipient_allow_command ( gchar *jabber_id )
  { gchar *jabberid, requete[512], hostonly[80], *ptr;
    struct IMSGPDB *imsg;
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
                "SELECT id,name,enable,comment,imsg_enable,imsg_jabberid,imsg_allow_cde,imsg_available "
                " FROM %s as user WHERE enable=1 AND imsg_allow_cde=1 AND imsg_jabberid LIKE '%s' LIMIT 1",
                NOM_TABLE_UTIL, jabberid );
    g_free(jabberid);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING, "%s: Database Connection Failed", __func__ );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )                     /* Execution de la requete SQL */
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING, "%s: Requete failed", __func__ );
       Libere_DB_SQL( &db );
       return(NULL);
     }

    imsg = Recuperer_imsgpDB_suite ( db );
    Libere_DB_SQL( &db );
    return(imsg);
  }
/******************************************************************************************************************************/
/* Imsgp_Envoi_message_to_all_available : Envoi un message aux contacts disponibles                                           */
/* Entrée: le message                                                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgp_Envoi_message_to_all_available ( gchar *message )
  { struct IMSGPDB *imsg;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING, "%s: Database Connection Failed", __func__ );
       return;
     }

/*********************************************** Chargement des informations en bases ******************************************/
    if ( ! Recuperer_all_available_imsgDB( db ) )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING, "%s: Recuperer_imsg Failed", __func__ );
       return;
     }

    while ( (imsg = Recuperer_imsgpDB_suite( db )) != NULL)
     { Imsgp_Envoi_message_to ( imsg->user_jabberid, message ); }

    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Imsgp_recevoir_imsg : CB appellé lorsque l'on recoit un message xmpp                                                       */
/* Entrée : Le Handler, la connexion, le message                                                                              */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Imsgp_recevoir_imsg(PurpleAccount *account, char *from, char *message, PurpleConversation *conv, PurpleMessageFlags flags)
  { struct IMSGPDB *imsg;
    struct DB *db;
    if (conv==NULL)
     { conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, from); }
  
    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
             "%s: (%s) %s %s: %s", __func__,
		           purple_conversation_get_name(conv),
		           purple_utf8_strftime("(%H:%M:%S)", NULL), from, message );

    imsg = Imsgp_recipient_allow_command ( from );
    if ( imsg == NULL )
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "%s : unknown sender '%s' or not allow to send command. Dropping message...", __func__, from );
       return;
     }
    g_free(imsg);

    if ( ! strcasecmp( message, "ping" ) )                                                             /* Interfacage de test */
     { Imsgp_Envoi_message_to( from, "Pong !" ); }   
    else if ( ! Recuperer_mnemo_baseDB_by_event_text ( &db, NOM_THREAD, message ) )
     { Imsgp_Envoi_message_to( from, "Error searching Database .. Sorry .." ); }   
    else 
     { struct CMD_TYPE_MNEMO_BASE *mnemo, *result_mnemo = NULL;
          
       if ( db->nbr_result == 0 )                                             /* Si pas d'enregistrement, demande de préciser */
        { Imsgp_Envoi_message_to( from, "Error... No result found .. Sorry .." ); }   
       if ( db->nbr_result > 1 )                                             /* Si trop d'enregistrement, demande de préciser */
        { Imsgp_Envoi_message_to( from, " Need to choose ... :" ); }

       while ( (mnemo = Recuperer_mnemo_baseDB_suite( &db )) != NULL )
        { if (db->nbr_result>1) Imsgp_Envoi_message_to( from, mnemo->ev_text );
          if (db->nbr_result==1) result_mnemo = mnemo;
                            else g_free(mnemo);
        }
       if (result_mnemo)
        { gchar chaine[80];
          switch ( result_mnemo->type )
           { case MNEMO_MONOSTABLE:                                                          /* Positionnement du bit interne */
                  g_snprintf( chaine, sizeof(chaine), "Mise a un du bit M%03d by %s", result_mnemo->num, imsg->user_name );
                  Imsgp_Envoi_message_to( from, chaine );
                  Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                             "%s: Mise a un du bit M%03d by %s", __func__, result_mnemo->num, imsg->user_name );
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
                               "%s: Cannot handle commande type %d (num=%03d) for %s", __func__,
                                result_mnemo->type, result_mnemo->num, imsg->user_name );
                      break;
           }
          g_free(result_mnemo);
        }
     }
  }
/******************************************************************************************************************************/
/* Imsgp_Sauvegarder_statut_contact : Sauvegarde en mémoire le statut du contact en parametre                                 */
/* Entrée: le contact et le statut                                                                                            */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgp_Sauvegarder_statut_contact ( const gchar *jabber_id, gboolean available )
  { gchar *jabberid, requete[512], hostonly[80], *ptr;
    struct DB *db;

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
                "UPDATE %s SET imsg_available=%d WHERE imsg_jabberid='%s'", NOM_TABLE_UTIL, available, jabberid );
    g_free(jabberid);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "%s: Database Connection Failed", __func__ );
       return;
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )                                         /* Execution de la requete SQL */
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING,
                "%s: Requete failed", __func__ );
     }
    else { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_DEBUG,
                    "%s : jabber_id %s -> Availability updated to %d.", __func__, jabber_id, available );
         } 
    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Imsgp_buddy_signed_on : Un contact bien d'arriver                                                                          */
/* Entrée: le buddy                                                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgp_buddy_signed_on(PurpleBuddy *buddy)
  { Imsgp_Sauvegarder_statut_contact ( purple_buddy_get_name(buddy), TRUE ); }
/******************************************************************************************************************************/
/* Imsgp_buddy_signed_off : Un contact bien de partir                                                                         */
/* Entrée: le buddy                                                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgp_buddy_signed_off(PurpleBuddy *buddy)
  { Imsgp_Sauvegarder_statut_contact ( purple_buddy_get_name(buddy), FALSE ); }
 
/******************************************************************************************************************************/
/* Imsgp_buddy_xxx : Ensemble de fonctions de notification sur le statut des contacts                                         */
/* Entrée: le buddy                                                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgp_buddy_away(PurpleBuddy *buddy, PurpleStatus *old_status, PurpleStatus *status)
  { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
             "%s: '%s' with proto %s", __func__,
              purple_buddy_get_name(buddy), purple_account_get_protocol_id(purple_buddy_get_account(buddy)));
  }
 
 static void Imsgp_buddy_idle(PurpleBuddy *buddy, gboolean old_idle, gboolean idle)
  { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
             "%s: '%s' with proto %s", __func__,
              purple_buddy_get_name(buddy), purple_account_get_protocol_id(purple_buddy_get_account(buddy)));
  }

 static void Imsgp_buddy_typing(PurpleAccount *account, const char *name)
  { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
             "%s: '%s' with proto %s", __func__,
              name, purple_account_get_protocol_id(account));
  }
 
 static void Imsgp_buddy_typed(PurpleAccount *account, const char *name) //not supported on all protocols
  { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
             "%s: '%s' with proto %s", __func__,
              name, purple_account_get_protocol_id(account));
  }
 
 static void Imsgp_buddy_typing_stopped(PurpleAccount *account, const char *name)
  { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
             "%s: '%s' with proto %s", __func__,
              name, purple_account_get_protocol_id(account));
  }
/******************************************************************************************************************************/
/* Imsgp_account_authorization_requested : Fonction appellé quand un user veut nous ajouter dans sa liste de buddy            */
/* Entrée: le compte et le user                                                                                               */
/* Sortie: 1 = OK pour ajouter                                                                                                */
/******************************************************************************************************************************/
 static int Imsgp_account_authorization_requested(PurpleAccount *account, const char *user)
  { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
             "%s: Buddy authorization request from '%s' for protocol '%s'", __func__,
              user, purple_account_get_protocol_id(account) );
    purple_account_add_buddy( account, purple_buddy_new 	( account, user, user ) );
    return 1; //authorize buddy request automatically (-1 denies it)
  }
/******************************************************************************************************************************/
/* Imsgp_signed_on: Appelé lorsque nous venons de nous connecter                                                              */
/* Entrée: La connextion Purple                                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Imsgp_signed_on ( PurpleConnection *gc, gpointer null )
  {	PurpleAccount *account = purple_connection_get_account(gc);
    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
             "%s: Account '%s' connected for protocol id '%s'", __func__,
              purple_account_get_username(account), purple_account_get_protocol_id(account));
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
/******************************************************************************************************************************/
/* Main: Fonction principale du thread Imsg                                                                                   */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { PurpleSavedStatus *status;
   	struct ZMQUEUE *zmq_msg;
    int handle;

    prctl(PR_SET_NAME, "W-IMSGP", 0, 0, 0 );
    memset( &Cfg_imsgp, 0, sizeof(Cfg_imsgp) );                                     /* Mise a zero de la structure de travail */
    Cfg_imsgp.lib = lib;                                           /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_imsgp.lib->TID = pthread_self();                                                    /* Sauvegarde du TID pour le pere */
    Imsgp_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
              "%s: Demarrage v%s . . . TID = %p", __func__, VERSION, pthread_self() );

    g_snprintf( Cfg_imsgp.lib->admin_prompt, sizeof(Cfg_imsgp.lib->admin_prompt), NOM_THREAD );
    g_snprintf( Cfg_imsgp.lib->admin_help,   sizeof(Cfg_imsgp.lib->admin_help),   "Manage Instant Messaging system (libpurple)" );

    if (!Cfg_imsgp.enable)
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE,
                "%s: Thread is not enabled in config. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    Cfg_imsgp.lib->Thread_run = TRUE;                                                                   /* Le thread tourne ! */
    MainLoop = g_main_loop_new( NULL, FALSE );

	/* libpurple's built-in DNS resolution forks processes to perform
	 * blocking lookups without blocking the main process.  It does not
	 * handle SIGCHLD itself, so if the UI does not you quickly get an army
	 * of zombie subprocesses marching around.
	 */
   	signal(SIGCHLD, SIG_IGN);

   	purple_util_set_user_dir(g_get_home_dir());
   	purple_debug_set_enabled(FALSE);
   	purple_core_set_ui_ops(&Imsgp_core_uiops);
	   purple_eventloop_set_ui_ops(&glib_eventloops);
    if (!purple_core_init("Watchdog"))
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_ERR,
                "%s: LibPurple Init failed. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }
   	purple_plugins_add_search_path("");
   	purple_plugins_get_all();
   	purple_prefs_load();
   	purple_plugins_load_saved("");
    purple_set_blist(purple_blist_new());
    purple_blist_load();

   	Cfg_imsgp.account = purple_account_new( Cfg_imsgp.username, "prpl-jabber" );
   	purple_account_set_password(Cfg_imsgp.account, Cfg_imsgp.password);

   	purple_account_set_enabled(Cfg_imsgp.account, "Watchdog", TRUE);           /* It's necessary to enable the account first. */
   	status = purple_savedstatus_new(NULL, PURPLE_STATUS_AVAILABLE);/* to connect the account, create a status and activate it */
	   purple_savedstatus_activate(status);

	   purple_signal_connect( purple_connections_get_handle(), "signed-on", &handle,
                          	PURPLE_CALLBACK(Imsgp_signed_on), NULL );
    purple_signal_connect( purple_conversations_get_handle(), "received-im-msg", &handle,
                           PURPLE_CALLBACK(Imsgp_recevoir_imsg), NULL );
    purple_signal_connect( purple_accounts_get_handle(), "account-authorization-requested", &handle,
                           PURPLE_CALLBACK(Imsgp_account_authorization_requested), NULL );
    purple_signal_connect( purple_blist_get_handle(), "buddy-signed-on", &handle,
                           PURPLE_CALLBACK(Imsgp_buddy_signed_on), NULL);
    purple_signal_connect( purple_blist_get_handle(), "buddy-signed-off", &handle,
                           PURPLE_CALLBACK(Imsgp_buddy_signed_off), NULL);
    purple_signal_connect( purple_blist_get_handle(), "buddy-status-changed", &handle,
                           PURPLE_CALLBACK(Imsgp_buddy_away), NULL);
    purple_signal_connect( purple_blist_get_handle(), "buddy-idle-changed", &handle,
                           PURPLE_CALLBACK(Imsgp_buddy_idle), NULL);
    purple_signal_connect( purple_conversations_get_handle(), "buddy-typing", &handle,
                           PURPLE_CALLBACK(Imsgp_buddy_typing), NULL);
    purple_signal_connect( purple_conversations_get_handle(), "buddy-typed", &handle,
                           PURPLE_CALLBACK(Imsgp_buddy_typed), NULL);

    zmq_msg = New_zmq ( ZMQ_SUB, "listen-to-msgs" );
    Connect_zmq (zmq_msg, "inproc", ZMQUEUE_LIVE_MSGS, 0 );

    while( Cfg_imsgp.lib->Thread_run == TRUE )                                               /* On tourne tant que necessaire */
     { struct CMD_TYPE_HISTO *histo, histo_buf;
       g_usleep(10000);
       sched_yield();

       if (Cfg_imsgp.lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE, "%s: recu signal SIGUSR1", __func__ );
          Imsgp_Lire_config ();                                             /* Lecture de la configuration logiciel du thread */
          Cfg_imsgp.lib->Thread_sigusr1 = FALSE;
        }

       if ( Recv_zmq ( zmq_msg, &histo_buf, sizeof(struct CMD_TYPE_HISTO) ) == sizeof(struct CMD_TYPE_HISTO) )
        { histo = &histo_buf;
          if (histo->alive) Imsgp_Envoi_message_to_all_available ( histo->msg.libelle );
        }

       g_main_context_iteration ( g_main_loop_get_context (MainLoop), FALSE );

     }                                                                                         /* Fin du while partage->arret */

    g_main_context_unref (g_main_loop_get_context (MainLoop));
    Close_zmq ( zmq_msg );
    purple_core_quit();
end:
    Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    Cfg_imsgp.lib->Thread_run = FALSE;                                                          /* Le thread ne tourne plus ! */
    Cfg_imsgp.lib->TID = 0;                                                   /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
