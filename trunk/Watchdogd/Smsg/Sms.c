/******************************************************************************************************************************/
/* Watchdogd/sms.c        Gestion des SMS de Watchdog v2.0                                                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       ven. 02 avril 2010 20:37:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Sms.c
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

 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <string.h>
 #include <unistd.h>
 #include <gammu.h>
 #include <curl/curl.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Sms.h"

 static gboolean sending_is_disabled = FALSE;                      /* Variable permettant d'interdire l'envoi de sms si panic */
 static GSM_Error sms_send_status;
 static GSM_StateMachine *s=NULL;
 static INI_Section *cfg=NULL;
 struct SMS_CONFIG Cfg_smsg;
/******************************************************************************************************************************/
/* Smsg_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                   */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Smsg_Lire_config ( void )
  { gchar *result, requete[256];
    struct DB *db;

    Creer_configDB ( NOM_THREAD, "debug", "false" );
    result = Recuperer_configDB_by_nom ( NOM_THREAD, "debug" );
    Cfg_smsg.lib->Thread_debug = !g_ascii_strcasecmp(result, "true");
    g_free(result);

    SQL_Write_new ( "INSERT IGNORE %s SET tech_id='GSM01', description='DEFAULT', ovh_service_name='DEFAULT', "
                    "ovh_application_key='DEFAULT',ovh_application_secret='DEFAULT', ovh_consumer_key='DEFAULT', "
                    "instance='%s'", NOM_THREAD, g_get_host_name() );

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),
               "SELECT tech_id, description, ovh_service_name, ovh_application_key, ovh_application_secret, ovh_consumer_key, nbr_sms "
               " FROM %s WHERE instance='%s' LIMIT 1", NOM_THREAD, g_get_host_name());
    if (!Lancer_requete_SQL ( db, requete ))
     { Libere_DB_SQL ( &db );
       Info_new( Config.log, Config.log_db, LOG_ERR, "%s: DB Requete failed", __func__ );
       return(FALSE);
     }

    if (Recuperer_ligne_SQL ( db ))
     { g_snprintf( Cfg_smsg.tech_id,                sizeof(Cfg_smsg.tech_id),                "%s", db->row[0] );
       g_snprintf( Cfg_smsg.description,            sizeof(Cfg_smsg.description),            "%s", db->row[1] );
       g_snprintf( Cfg_smsg.ovh_service_name,       sizeof(Cfg_smsg.ovh_service_name),       "%s", db->row[2] );
       g_snprintf( Cfg_smsg.ovh_application_key,    sizeof(Cfg_smsg.ovh_application_key),    "%s", db->row[3] );
       g_snprintf( Cfg_smsg.ovh_application_secret, sizeof(Cfg_smsg.ovh_application_secret), "%s", db->row[4] );
       g_snprintf( Cfg_smsg.ovh_consumer_key,       sizeof(Cfg_smsg.ovh_consumer_key),       "%s", db->row[5] );
       Cfg_smsg.nbr_sms = atoi ( db->row[6] );
     }
    else Info_new( Config.log, Config.log_db, LOG_ERR, "%s: DB Get Result failed", __func__ );
    Libere_DB_SQL ( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Modbus_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                 */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Smsg_Creer_DB ( void )
  { gint database_version;

    gchar *database_version_string = Recuperer_configDB_by_nom( NOM_THREAD, "database_version" );
    if (database_version_string)
     { database_version = atoi( database_version_string );
       g_free(database_version_string);
     } else database_version=0;

    Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'. Thread_Version '%s'.", __func__, database_version, WTD_VERSION );

    if (database_version==0)
     { SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                       "`id` int(11) NOT NULL AUTO_INCREMENT,"
                       "`instance` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT 'localhost',"
                       "`date_create` datetime NOT NULL DEFAULT NOW(),"
                       "`tech_id` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                       "`description` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "`ovh_service_name` VARCHAR(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "`ovh_application_key` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "`ovh_application_secret` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "`ovh_consumer_key` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "`nbr_sms` int(11) NOT NULL DEFAULT 0,"
                       "PRIMARY KEY (`id`)"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", NOM_THREAD );
     }

    database_version = 1;
end:
    Modifier_configDB_int ( NOM_THREAD, "database_version", database_version );
  }
/******************************************************************************************************************************/
/* Recuperer_smsDB: récupère la liste des utilisateurs et de leur numéro de téléphone                                         */
/* Entrée: une structure DB                                                                                                   */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Smsg_Recuperer_smsDB ( struct DB *db )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,username,enable,comment,notification,phone,allow_cde "
                " FROM users as user ORDER BY username" );

    return ( Lancer_requete_SQL ( db, requete ) );                                             /* Execution de la requete SQL */
  }
/******************************************************************************************************************************/
/* Recuperer_recipient_authorized: Recupere la liste des users habilité a recevoir des SMS                                    */
/* Entrée: une structure DB                                                                                                   */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 static gboolean Smsg_Recuperer_recipient_authorized_smsDB ( struct DB *db )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,username,enable,comment,notification,phone,allow_cde "
                " FROM users as user WHERE enable=1 AND notification=1 ORDER BY username" );

    return ( Lancer_requete_SQL ( db, requete ) );                                             /* Execution de la requete SQL */
  }
/******************************************************************************************************************************/
/* Recuperer_liste_id_smsDB: Recupération de la liste des ids des smss                                                        */
/* Entrée: une structure DB                                                                                                   */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 struct SMSDB *Smsg_Recuperer_smsDB_suite( struct DB *db )
  { struct SMSDB *sms;

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( db );
       return(NULL);
     }

    sms = (struct SMSDB *)g_try_malloc0( sizeof(struct SMSDB) );
    if (!sms) Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else
     { g_snprintf( sms->user_phone, sizeof(sms->user_phone), "%s", db->row[5] );
       g_snprintf( sms->user_name,      sizeof(sms->user_name),      "%s", db->row[1] );
       g_snprintf( sms->user_comment,   sizeof(sms->user_comment),   "%s", db->row[3] );
       sms->user_id            = atoi(db->row[0]);
       sms->user_enable        = atoi(db->row[2]);
       sms->user_notification  = atoi(db->row[4]);
       sms->user_allow_cde     = atoi(db->row[6]);
     }
    return(sms);
  }
/******************************************************************************************************************************/
/* Smsg_is_recipient_authorized : Renvoi TRUE si le telephone en parametre peut set ou reset un bit interne                   */
/* Entrée: le nom du destinataire                                                                                             */
/* Sortie : booléen, TRUE/FALSE                                                                                               */
/******************************************************************************************************************************/
 static struct SMSDB *Smsg_is_recipient_authorized ( gchar *tel )
  { struct SMSDB *sms;
    gchar *phone, requete[512];
    struct DB *db;

    phone = Normaliser_chaine ( tel );
    if (!phone)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation phone impossible", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,username,enable,comment,notification,phone,allow_cde "
                " FROM users as user WHERE enable=1 AND allow_cde=1 AND phone LIKE '%s'"
                " ORDER BY username LIMIT 1", phone );
    g_free(phone);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_WARNING,
                "%s: Database Connection Failed", __func__ );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )                                         /* Execution de la requete SQL */
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_WARNING,
                "%s: Requete failed", __func__ );
       Libere_DB_SQL( &db );
       return(NULL);
     }

    sms = Smsg_Recuperer_smsDB_suite( db );
    Libere_DB_SQL( &db );
    return(sms);
  }
/******************************************************************************************************************************/
/* Smsg_Send_CB: Appelé par le téléphone quand le SMS est parti                                                               */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Smsg_Send_CB (GSM_StateMachine *sm, int status, int MessageReference, void * user_data)
  {	if (status==0) {	sms_send_status = ERR_NONE; }
    else sms_send_status = ERR_UNKNOWN;
  }
/******************************************************************************************************************************/
/* Envoi_sms_gsm: Envoi un sms par le gsm                                                                                     */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Envoi_sms_gsm ( JsonNode *msg, gchar *telephone )
  { GSM_StateMachine *s;
    GSM_SMSMessage sms;
    GSM_SMSC PhoneSMSC;
    gchar libelle[256];
    INI_Section *cfg;
    GSM_Error error;
    gint wait;

    GSM_InitLocales(NULL);
   	memset(&sms, 0, sizeof(sms));                                                                       /* Préparation du SMS */
    g_snprintf( libelle, sizeof(libelle), "%s: %s", Json_get_string ( msg, "dls_shortname" ), Json_get_string( msg, "libelle") );
	  	EncodeUnicode( sms.Text, libelle, strlen(libelle));                                                /* Encode message text */
    EncodeUnicode( sms.Number, telephone, strlen(telephone));

	   sms.PDU = SMS_Submit;                                                                        /* We want to submit message */
	   sms.UDH.Type = UDH_NoUDH;                                                                 /* No UDH, just a plain message */
	   sms.Coding = SMS_Coding_Default_No_Compression;                                        /* We used default coding for text */
   	sms.Class = 1;                                                                                /* Class 1 message (normal) */


	   if ( (s = GSM_AllocStateMachine()) == NULL )                                                   /* Allocates state machine */
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR, "%s: AllocStateMachine Error", __func__ );
       return(FALSE);
     }

	/*debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(FALSE, debug_info);
	GSM_SetDebugFileDescriptor(stderr, TRUE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);*/

	   error = GSM_FindGammuRC(&cfg, NULL);
	   if (error != ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                "%s: FindGammuRC Failed (%s)", __func__, GSM_ErrorString(error) );
       if (GSM_IsConnected(s))	GSM_TerminateConnection(s);
       goto end;
     }

   	error = GSM_ReadConfig(cfg, GSM_GetConfig(s, 0), 0);
	   if (error != ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                "%s: ReadConfig Failed (%s)", __func__, GSM_ErrorString(error) );
       if (GSM_IsConnected(s))	GSM_TerminateConnection(s);
       goto end;
     }

   	INI_Free(cfg);
   	GSM_SetConfigNum(s, 1);

   	error = GSM_InitConnection(s, 3);
	   if (error != ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                "%s: InitConnection Failed (%s)", __func__, GSM_ErrorString(error) );
       if (GSM_IsConnected(s))	GSM_TerminateConnection(s);
       goto end;
     }

    GSM_SetSendSMSStatusCallback(s, Smsg_Send_CB, NULL);

	   PhoneSMSC.Location = 1;                                                                   	/* We need to know SMSC number */
   	error = GSM_GetSMSC(s, &PhoneSMSC);
	   if (error != ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                "%s: GetSMSC Failed (%s)", __func__, GSM_ErrorString(error) );
       if (GSM_IsConnected(s))	GSM_TerminateConnection(s);
       goto end;
     }

	   CopyUnicodeString(sms.SMSC.Number, PhoneSMSC.Number);                                       /* Set SMSC number in message */

   	sms_send_status = ERR_TIMEOUT;
	   error = GSM_SendSMS(s, &sms); 	                                                                           /* Send message */
	   if (error != ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                "%s: SendSMS Failed (%s)", __func__, GSM_ErrorString(error) );
       if (GSM_IsConnected(s))	GSM_TerminateConnection(s);
       goto end;
     }

    wait = Partage->top; 	                                                                          /* Wait for network reply */
	   while ( (Partage->top < wait+150) && sms_send_status == ERR_TIMEOUT )
     {	GSM_ReadDevice(s, TRUE); }

    if (sms_send_status == ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_INFO,
                "%s: Envoi SMS Ok to %s (%s)", __func__, telephone, libelle );
     }
    else
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_WARNING,
                "%s: Envoi SMS Nok to %s (error '%s')", __func__, telephone, GSM_ErrorString(error) ); }

   	error = GSM_TerminateConnection(s); 	                                                             /* Terminate connection */
	   if (error != ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                "%s: TerminateConnection Failed (%s)", __func__, GSM_ErrorString(error) );
       if (GSM_IsConnected(s))	GSM_TerminateConnection(s);
     }

end:
	   GSM_FreeStateMachine(s);                                                                          	/* Free up used memory */
    if (sms_send_status == ERR_NONE) { Cfg_smsg.nbr_sms++; return(TRUE); }
    else return(FALSE);
  }
/******************************************************************************************************************************/
/* Envoi_sms_smsbox: Envoi un sms par SMSBOX                                                                                  */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoi_sms_ovh ( JsonNode *msg, gchar *telephone )
  { gchar clair[512], hash_string[48], signature[48], query[128];
    unsigned char hash_bin[EVP_MAX_MD_SIZE];
    EVP_MD_CTX *mdctx;
    gsize taille_buf;
    int md_len;

    JsonBuilder *builder = Json_create();
    Json_add_bool  ( builder, "noStopClause", TRUE );
    Json_add_string( builder, "priority", "high" );
    Json_add_bool  ( builder, "senderForResponse", TRUE );
    Json_add_int   ( builder, "validityPeriod", 2880 ); /* 2 jours */
    Json_add_string( builder, "charset", "UTF-8" );
    Json_add_array ( builder, "receivers" );
    json_builder_add_string_value( builder, telephone );
    Json_end_array ( builder );
    gchar libelle[128];
    g_snprintf( libelle, sizeof(libelle), "%s: %s", Json_get_string ( msg, "dls_shortname" ), Json_get_string( msg, "libelle") );
    Json_add_string( builder, "message", libelle );
    gchar *body = Json_get_buf( builder, &taille_buf );

    gchar *method = "POST";
    g_snprintf( query, sizeof(query), "https://eu.api.ovh.com/1.0/sms/%s/jobs", Cfg_smsg.ovh_service_name );
    gchar timestamp[20];
    g_snprintf( timestamp, sizeof(timestamp), "%ld", time(NULL) );

/******************************************************* Calcul signature *****************************************************/
    g_snprintf( clair, sizeof(clair), "%s+%s+%s+%s+%s+%s", Cfg_smsg.ovh_application_secret, Cfg_smsg.ovh_consumer_key,
                method, query, body, timestamp );

    mdctx = EVP_MD_CTX_new();                                                                               /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL);
    EVP_DigestUpdate(mdctx, clair, strlen(clair));
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);

    memset( hash_string, 0, sizeof(hash_string));                                                       /* Conversion en Hexa */
    for (gint i=0; i<20; i++)
     { gchar chaine[3];
       g_snprintf(chaine, sizeof(chaine), "%02x", hash_bin[i] );
       g_strlcat(hash_string, chaine, sizeof(hash_string) );
     }

    g_snprintf( signature, sizeof(signature), "$1$%s", hash_string );

/********************************************************* Envoi de la requete ************************************************/
    SoupSession *connexion = soup_session_new();
    SoupMessage *soup_msg = soup_message_new ( method, query );
    soup_message_set_request ( soup_msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, body, taille_buf );
    SoupMessageHeaders *headers;
    g_object_get ( G_OBJECT(soup_msg), "request_headers", &headers, NULL );
    soup_message_headers_append ( headers, "X-Ovh-Application", Cfg_smsg.ovh_application_key );
    soup_message_headers_append ( headers, "X-Ovh-Consumer",    Cfg_smsg.ovh_consumer_key );
    soup_message_headers_append ( headers, "X-Ovh-Signature",   signature );
    soup_message_headers_append ( headers, "X-Ovh-Timestamp",   timestamp );
    soup_session_send_message (connexion, soup_msg);

    GBytes *response_brute;
    gchar *reason_phrase;
    gint status_code;

    g_object_get ( soup_msg, "status-code", &status_code, "reason-phrase", &reason_phrase, "response-body-data", &response_brute, NULL );
    Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_DEBUG, "%s: Status %d, reason %s", __func__, status_code, reason_phrase );
    if (status_code!=200)
     { gsize taille;
       gchar *error = g_bytes_get_data ( response_brute, &taille );
       Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR, "%s: Error: %s\n", __func__, error );
       g_free(error);
     }
    else Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_DEBUG, "%s: '%s' sent to '%s'", __func__, libelle, telephone );
    g_object_unref( soup_msg );
    soup_session_abort ( connexion );
  }
/******************************************************************************************************************************/
/* Smsg_send_to_all_authorized_recipients : Envoi à tous les portables autorisés                                              */
/* Entrée: le message                                                                                                         */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Smsg_send_to_all_authorized_recipients ( JsonNode *msg )
  { struct SMSDB *sms;
    struct DB *db;

    if (sending_is_disabled == TRUE)                                   /* Si envoi désactivé, on sort de suite de la fonction */
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_NOTICE, "%s: Sending is disabled. Dropping message", __func__ );
       return;
     }

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_WARNING, "%s: Database Connection Failed", __func__ );
       return;
     }

/********************************************* Chargement des informations en bases *******************************************/
    if ( ! Smsg_Recuperer_recipient_authorized_smsDB( db ) )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_WARNING, "%s: Recuperer_sms Failed", __func__ );
       return;
     }

    gint sms_notification = Json_get_int ( msg, "sms_notification" );
    while ( (sms = Smsg_Recuperer_smsDB_suite( db )) != NULL)
     { switch (sms_notification)
        { case MESSAGE_SMS_YES:
               if ( Envoi_sms_gsm ( msg, sms->user_phone ) == FALSE )
                { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                            "%s: First Error sending with GSM. Trying another one in 5 sec", __func__ );
                  sleep(5);
                  if ( Envoi_sms_gsm ( msg, sms->user_phone ) == FALSE )
                   { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                               "%s: Second Error sending with GSM. Falling back to OVH", __func__ );
                     Envoi_sms_ovh( msg, sms->user_phone );
                   }
                }
               break;
          case MESSAGE_SMS_GSM_ONLY:
               Envoi_sms_gsm   ( msg, sms->user_phone );
               break;
          case MESSAGE_SMS_OVH_ONLY:
               Envoi_sms_ovh( msg, sms->user_phone );
               break;
        }
       /*sleep(5);*/
     }

    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un texte au format UTF8 si possible                                                                                */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_smsg_ovh_text ( gchar *texte )
  { JsonBuilder *builder = Json_create();
    Json_add_string ( builder, "libelle", texte );
    Json_add_string ( builder, "dls_shortname", Cfg_smsg.tech_id );
    Json_add_int    ( builder, "sms_notification", MESSAGE_SMS_OVH_ONLY );
    JsonNode *msg = Json_end ( builder );
    Smsg_send_to_all_authorized_recipients( msg );
    json_node_unref(msg);
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un texte au format UTF8 si possible                                                                                */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_smsg_gsm_text ( gchar *texte )
  { JsonBuilder *builder = Json_create();
    Json_add_string ( builder, "libelle", texte );
    Json_add_string ( builder, "dls_shortname", Cfg_smsg.tech_id );
    Json_add_int    ( builder, "sms_notification", MESSAGE_SMS_GSM_ONLY );
    JsonNode *msg = Json_end ( builder );
    Smsg_send_to_all_authorized_recipients( msg );
    json_node_unref(msg);
  }
/******************************************************************************************************************************/
/* Traiter_commande_sms: Fonction appelée pour traiter la commande sms recu par le telephone                                  */
/* Entrée: le message text à traiter                                                                                          */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Traiter_commande_sms ( gchar *from, gchar *texte )
  { gboolean found = FALSE;
    struct SMSDB *sms;
    gchar chaine[160];
    struct DB *db;

    sms = Smsg_is_recipient_authorized ( from );
    if ( sms == NULL )
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_NOTICE,
                "%s : unknown sender %s. Dropping message %s...", __func__, from, texte );
       return;
     }

    if ( ! strcasecmp( texte, "ping" ) )                                                               /* Interfacage de test */
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_NOTICE, "%s: Ping Received from '%s'. Sending Pong", __func__, from );
       Envoyer_smsg_gsm_text ( "Pong !" );
       return;
     }

    if ( ! strcasecmp( texte, "smsoff" ) )                                                                      /* Smspanic ! */
     { sending_is_disabled = TRUE;
       Envoyer_smsg_gsm_text ( "Sending SMS is off !" );
       Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_NOTICE, "%s: Sending SMS is DISABLED by '%s'", __func__, from );
       return;
     }

    if ( ! strcasecmp( texte, "smson" ) )                                                                       /* Smspanic ! */
     { Envoyer_smsg_gsm_text ( "Sending SMS is on !" );
       Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_NOTICE, "%s: Sending SMS is ENABLED by '%s'", __func__, from );
       sending_is_disabled = FALSE;
       return;
     }

    if ( ! Recuperer_mnemos_DI_by_tag ( &db, Cfg_smsg.tech_id, texte ) )
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR, "%s: Error searching Database for '%s'", __func__, texte ); }
    else while ( Recuperer_mnemos_DI_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *libelle = db->row[3], *src_text = db->row[2];
       Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_NOTICE, "%s: Dyn Match found '%s' '%s:%s' - %s", __func__,
                 src_text, tech_id, acro, libelle );

       found=TRUE;
       if (Config.instance_is_master==TRUE)                                                       /* si l'instance est Maitre */
        { Envoyer_commande_dls_data ( tech_id, acro ); }
       else /* Envoi au master via thread HTTP */
        { Send_zmq_CDE_to_master ( Cfg_smsg.zmq_to_master, NOM_THREAD, tech_id, acro ); }
     }

    if (found)
         { g_snprintf(chaine, sizeof(chaine), "'%s' done.", texte ); }
    else { g_snprintf(chaine, sizeof(chaine), "'%s' not found in database.", texte ); }    /* Envoi de l'erreur si pas trouvé */
    Envoyer_smsg_gsm_text ( chaine );
  }
/******************************************************************************************************************************/
/* Smsg_disconnect: Se deconnecte du telephone ou de la clef 3G                                                               */
/* Entrée: Rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Smsg_disconnect ( void )
  { GSM_Error error;
   	error = GSM_TerminateConnection(s); 	                                                             /* Terminate connection */
	   if (error != ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                "%s: TerminateConnection Failed (%s)", __func__, GSM_ErrorString(error) );
     }
    GSM_FreeStateMachine(s);                                                                          	/* Free up used memory */
    s = NULL;
    Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_DEBUG, "%s: Disconnected", __func__ );
  }
/******************************************************************************************************************************/
/* smsg_connect: Ouvre une connexion vers le téléphone ou la clef 3G                                                          */
/* Entrée: Rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Smsg_connect ( void )
  { GSM_Error error;

    GSM_InitLocales(NULL);
    if ( (s = GSM_AllocStateMachine()) == NULL )                                                   /* Allocates state machine */
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR, "%s: AllocStateMachine Error", __func__ );
       return(FALSE);
     }

	   error = GSM_FindGammuRC(&cfg, NULL);
	   if (error != ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                "%s: FindGammuRC Failed (%s)", __func__, GSM_ErrorString(error) );
       if (GSM_IsConnected(s))	GSM_TerminateConnection(s);
   	   Smsg_disconnect();
       sleep(2);
       return(FALSE);
     }

   	error = GSM_ReadConfig(cfg, GSM_GetConfig(s, 0), 0);
	   if (error != ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                "%s: ReadConfig Failed (%s)", __func__, GSM_ErrorString(error) );
       if (GSM_IsConnected(s))	GSM_TerminateConnection(s);
   	   Smsg_disconnect();
       sleep(2);
       return(FALSE);
     }

   	INI_Free(cfg);
   	GSM_SetConfigNum(s, 1);

   	error = GSM_InitConnection(s, 1);
	   if (error != ERR_NONE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                "%s: InitConnection Failed (%s)", __func__, GSM_ErrorString(error) );
       if (GSM_IsConnected(s))	GSM_TerminateConnection(s);
   	   Smsg_disconnect();
       sleep(2);
       return(FALSE);
     }
    Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_DEBUG, "%s: Connection OK", __func__ );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Lire_sms_gsm: Lecture de tous les SMS du GSM                                                                               */
/* Entrée: Rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Lire_sms_gsm ( void )
  { gchar from[80], texte[180];
    GSM_MultiSMSMessage sms;
    gboolean found = FALSE;
    GSM_Error error;

   	memset(&sms, 0, sizeof(sms));                                                                       /* Préparation du SMS */

    if (Smsg_connect()==FALSE) return;
/* Read all messages */
   	error = ERR_NONE;
	   sms.Number = 0;
	   sms.SMS[0].Location = 0;
	   sms.SMS[0].Folder = 0;
    error = GSM_GetNextSMS(s, &sms, TRUE);
    if (error == ERR_NONE)
     { gint i;
       for (i = 0; i < sms.Number; i++)
			     { g_snprintf( from, sizeof(from), "%s", DecodeUnicodeConsole(sms.SMS[i].Number) );
          g_snprintf( texte, sizeof(texte), "%s", DecodeUnicodeConsole(sms.SMS[i].Text) );
          sms.SMS[0].Folder = 0;/* https://github.com/gammu/gammu/blob/ed2fec4a382e7ac4b5dfc92f5b10811f76f4817e/gammu/message.c */
          error = GSM_DeleteSMS( s, &sms.SMS[i] );
          if (error != ERR_NONE)
           { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                      "%s: Delete '%s' from '%s' Location %d/%d Folder %d Failed ('%s')!", __func__,
                       texte, from, i, sms.SMS[i].Location, sms.SMS[i].Folder, GSM_ErrorString(error) );
           }
          else
           { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_DEBUG,
                      "%s: Delete '%s' from '%s' Location %d/%d Folder %d OK !", __func__,
                       texte, from, i, sms.SMS[i].Location, sms.SMS[i].Folder );
           }
          if (sms.SMS[i].State == SMS_UnRead)                                /* Pour tout nouveau message, nous le processons */
           { found = TRUE; break; }
         }
	     }
     else if (error != ERR_EMPTY)
      { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR,
                  "%s: No sms received ('%s')!", __func__, GSM_ErrorString(error) );
      }
    Smsg_disconnect();                                                                                	/* Free up used memory */
    if (found) Traiter_commande_sms ( from, texte );

    if ( (error == ERR_NONE) || (error == ERR_EMPTY) )
     { Send_zmq_WATCHDOG_to_master ( Cfg_smsg.zmq_to_master, NOM_THREAD, Cfg_smsg.tech_id, "IO_COMM", 600 );
       Cfg_smsg.comm_status = TRUE;
     }
    else
     { Send_zmq_WATCHDOG_to_master ( Cfg_smsg.zmq_to_master, NOM_THREAD, Cfg_smsg.tech_id, "IO_COMM", 0 );
       Cfg_smsg.comm_status = FALSE;
     }
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un client et un utilisateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct ZMQUEUE *zmq_from_bus;
reload:
    memset( &Cfg_smsg, 0, sizeof(Cfg_smsg) );                                       /* Mise a zero de la structure de travail */
    Cfg_smsg.lib = lib;                                            /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "W-SMSG", "USER", lib, WTD_VERSION, "Manage SMS system (libgammu)" );
    Smsg_Creer_DB ();                                                                       /* Création de la base de données */
    Smsg_Lire_config ();                                                    /* Lecture de la configuration logiciel du thread */

    if (Dls_auto_create_plugin( Cfg_smsg.tech_id, "Gestion du GSM" ) == FALSE)
     { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, Cfg_smsg.tech_id ); }

    Mnemo_auto_create_WATCHDOG ( FALSE, Cfg_smsg.tech_id, "IO_COMM", "Statut de la communication avec le GSM" );

    zmq_from_bus           = Connect_zmq ( ZMQ_SUB, "listen-to-bus",  "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    Cfg_smsg.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master",  "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

    Envoyer_smsg_gsm_text ( "SMS System is running" );
    sending_is_disabled = FALSE;                                                     /* A l'init, l'envoi de SMS est autorisé */
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { gchar buffer[1024];

/****************************************************** Lecture de SMS ********************************************************/
       Lire_sms_gsm();

/****************************************************** SMS de test ! *********************************************************/
       if (Cfg_smsg.send_test_OVH)
        { Envoyer_smsg_ovh_text ( "Test SMS OVH OK !" );
          Cfg_smsg.send_test_OVH = FALSE;
        }
       if (Cfg_smsg.send_test_GSM)
        { Envoyer_smsg_gsm_text ( "Test SMS GSM OK !" );
          Cfg_smsg.send_test_GSM = FALSE;
        }
/********************************************************* Envoi de SMS *******************************************************/
       JsonNode *request;
       while ( (lib->Thread_run == TRUE && lib->Thread_reload == FALSE) &&
              (request=Recv_zmq_with_json( zmq_from_bus, NOM_THREAD, (gchar *)&buffer, sizeof(buffer) )) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "DLS_HISTO" ) &&
               Json_get_bool ( request, "alive" ) == TRUE &&
               Json_get_int  ( request, "sms_notification" ) != MESSAGE_SMS_NONE )
           { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_NOTICE, "%s : Sending msg '%s:%s' (%s)", __func__,
                       Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ),
                       Json_get_string ( request, "libelle" ) );

/*************************************************** Envoi en mode GSM ********************************************************/
             Smsg_send_to_all_authorized_recipients( request );
           }
          else
           { Info_new( Config.log, Cfg_smsg.lib->Thread_debug, LOG_DEBUG, "%s : zmq_tag '%s' not for this thread", __func__, zmq_tag ); }
          json_node_unref(request);
        }
     }

    Send_zmq_WATCHDOG_to_master ( Cfg_smsg.zmq_to_master, NOM_THREAD, Cfg_smsg.tech_id, "IO_COMM", 0 );
    Close_zmq ( zmq_from_bus );
    Close_zmq ( Cfg_smsg.zmq_to_master );

    SQL_Write_new ( "UPDATE %s SET nbr_sms='%d' WHERE instance='%s'", NOM_THREAD, Cfg_smsg.nbr_sms, g_get_host_name() );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
