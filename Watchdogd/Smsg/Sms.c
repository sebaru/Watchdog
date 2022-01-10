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

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Sms.h"

/******************************************************************************************************************************/
/* Smsg_Creer_DB : Creation de la database du process                                                                         */
/* Entrée: le pointeur sur la structure PROCESS                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Smsg_Creer_DB ( struct PROCESS *lib )
  {
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'.", __func__, lib->database_version );

    SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                    "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                    "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                    "`uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
                    "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                    "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`ovh_service_name` VARCHAR(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`ovh_application_key` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`ovh_application_secret` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`ovh_consumer_key` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`nbr_sms` int(11) NOT NULL DEFAULT 0,"
                    "FOREIGN KEY (`uuid`) REFERENCES `processes` (`uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                    ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", lib->name );

    Process_set_database_version ( lib, 1 );
  }
/******************************************************************************************************************************/
/* Smsg_is_allow_cde : Renvoi TRUE si le telephone en parametre peut set ou reset un bit interne                              */
/* Entrée: le nom du destinataire                                                                                             */
/* Sortie : booléen, TRUE/FALSE                                                                                               */
/******************************************************************************************************************************/
 static gboolean Smsg_is_allow_cde ( struct SUBPROCESS *module, gchar *tel )
  { gchar *phone;
    gboolean retour;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    phone = Normaliser_chaine ( tel );
    if (!phone)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                 "%s: %s: Normalisation phone impossible", __func__ , thread_tech_id);
       return(FALSE);
     }

    JsonNode *RootNode = Json_node_create();
    if (!RootNode)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                 "%s: %s: Memory error", __func__, thread_tech_id );
       g_free(phone);
       return(FALSE);
     }

    SQL_Select_to_json_node ( RootNode, "recipient",
                              "SELECT id,username,enable,comment,notification,phone,allow_cde "
                              "FROM users as user WHERE enable=1 AND allow_cde=1 AND phone LIKE '%s'"
                              " ORDER BY username LIMIT 1", phone );
    g_free(phone);

    retour = (Json_get_int ( RootNode, "nbr_recipient" ) == 1 ? TRUE : FALSE);
    json_node_unref(RootNode);
    return(retour);
  }
/******************************************************************************************************************************/
/* Smsg_Send_CB: Appelé par le téléphone quand le SMS est parti                                                               */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Smsg_Send_CB (GSM_StateMachine *sm, int status, int MessageReference, void * user_data)
  { struct SUBPROCESS *module = user_data;
    struct SMS_VARS *vars = module->vars;
    if (status==0) { vars->gammu_send_status = ERR_NONE; }
    else vars->gammu_send_status = ERR_UNKNOWN;
  }
/******************************************************************************************************************************/
/* Smsg_disconnect: Se deconnecte du telephone ou de la clef 3G                                                               */
/* Entrée: Rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Smsg_disconnect ( struct SUBPROCESS *module )
  { struct SMS_VARS *vars = module->vars;
    GSM_Error error;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    if (GSM_IsConnected(vars->gammu_machine))
     { error = GSM_TerminateConnection(vars->gammu_machine);                                  /* Terminate connection */
       if (error != ERR_NONE)
        { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                   "%s: %s: TerminateConnection Failed (%s)", __func__, thread_tech_id, GSM_ErrorString(error) );
        }
     }
    GSM_FreeStateMachine(vars->gammu_machine);                                                 /* Free up used memory */
    vars->gammu_machine = NULL;
    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: %s: Disconnected", __func__, thread_tech_id );
    SubProcess_send_comm_to_master_new ( module, FALSE );
  }
/******************************************************************************************************************************/
/* smsg_connect: Ouvre une connexion vers le téléphone ou la clef 3G                                                          */
/* Entrée: Rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Smsg_connect ( struct SUBPROCESS *module )
  { struct SMS_VARS *vars = module->vars;
    GSM_Error error;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: %s: Trying to connect", __func__, thread_tech_id );

    GSM_InitLocales(NULL);
    if ( (vars->gammu_machine = GSM_AllocStateMachine()) == NULL )                         /* Allocates state machine */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: AllocStateMachine Error", __func__, thread_tech_id );
       return(FALSE);
     }

    error = GSM_FindGammuRC(&vars->gammu_cfg, NULL);
    if (error != ERR_NONE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                 "%s: %s: FindGammuRC Failed (%s)", __func__, thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    error = GSM_ReadConfig(vars->gammu_cfg, GSM_GetConfig(vars->gammu_machine, 0), 0);
    if (error != ERR_NONE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                "%s: %s: ReadConfig Failed (%s)", __func__, thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    INI_Free(vars->gammu_cfg);
    GSM_SetConfigNum(vars->gammu_machine, 1);

    error = GSM_InitConnection(vars->gammu_machine, 1);
    if (error != ERR_NONE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                 "%s: %s: InitConnection Failed (%s)", __func__, thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }
    GSM_SetSendSMSStatusCallback(vars->gammu_machine, Smsg_Send_CB, module);

    gchar constructeur[64];
    error = GSM_GetManufacturer(vars->gammu_machine, constructeur);
    if (error != ERR_NONE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                 "%s: %s: GSM_GetManufacturer Failed (%s)", __func__, thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    gchar model[64];
    error = GSM_GetModel(vars->gammu_machine, model);
    if (error != ERR_NONE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                 "%s: %s: GSM_GetModel Failed (%s)", __func__, thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
              "%s: %s: Connection OK with '%s/%s'", __func__, thread_tech_id, constructeur, model );
    SubProcess_send_comm_to_master_new ( module, TRUE );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Envoi_sms_gsm: Envoi un sms par le gsm                                                                                     */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Envoi_sms_gsm ( struct SUBPROCESS *module, JsonNode *msg, gchar *telephone )
  { struct SMS_VARS *vars = module->vars;
    GSM_SMSMessage sms;
    GSM_SMSC PhoneSMSC;
    gchar libelle[256];
    GSM_Error error;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (module->comm_status == FALSE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: COMM is FALSE", __func__, thread_tech_id );
       return(FALSE);
     }

    if (!telephone)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: telephone is NULL", __func__, thread_tech_id );
       return(FALSE);
     }

    GSM_InitLocales(NULL);
    memset(&sms, 0, sizeof(sms));                                                                       /* Préparation du SMS */
    sms.PDU = SMS_Submit;                                                                        /* We want to submit message */
    sms.UDH.Type = UDH_NoUDH;                                                                 /* No UDH, just a plain message */
    sms.Coding = SMS_Coding_Unicode_No_Compression;                                        /* We used default coding for text */
    sms.Class = 1;                                                                                /* Class 1 message (normal) */
    gchar *dls_shortname = Json_get_string ( msg, "dls_shortname" );
    if (dls_shortname) g_snprintf( libelle, sizeof(libelle), "%s: %s", dls_shortname, Json_get_string( msg, "libelle") );
                  else g_snprintf( libelle, sizeof(libelle), "%s", Json_get_string( msg, "libelle") );
    EncodeUnicode( sms.Text, libelle, strlen(libelle));                                                /* Encode message text */
    EncodeUnicode( sms.Number, telephone, strlen(telephone));

 /*debug_info = GSM_GetDebug(s);
 GSM_SetDebugGlobal(FALSE, debug_info);
 GSM_SetDebugFileDescriptor(stderr, TRUE, debug_info);
 GSM_SetDebugLevel("textall", debug_info);*/

    PhoneSMSC.Location = 1;                                                                    /* We need to know SMSC number */
    error = GSM_GetSMSC(vars->gammu_machine, &PhoneSMSC);
    if (error != ERR_NONE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                "%s: %s: GetSMSC Failed (%s)", __func__, thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    CopyUnicodeString(sms.SMSC.Number, PhoneSMSC.Number);                                       /* Set SMSC number in message */

    vars->gammu_send_status = ERR_TIMEOUT;
    error = GSM_SendSMS(vars->gammu_machine, &sms);                                                        /* Send message */
    if (error != ERR_NONE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                 "%s: %s: SendSMS Failed (%s)", __func__, thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    while ( vars->gammu_send_status == ERR_TIMEOUT ) { GSM_ReadDevice(vars->gammu_machine, TRUE); }

    if (vars->gammu_send_status == ERR_NONE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE,
                 "%s: %s: Envoi SMS Ok to %s (%s)", __func__, thread_tech_id, telephone, libelle );
       vars->nbr_sms++;
       return(TRUE);
     }
    Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
             "%s: %s: Envoi SMS Nok to %s (%s) -> error '%s'", __func__, thread_tech_id, telephone, libelle, GSM_ErrorString(error) );
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Envoi_sms_smsbox: Envoi un sms par SMSBOX                                                                                  */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoi_sms_ovh ( struct SUBPROCESS *module, JsonNode *msg, gchar *telephone )
  { gchar clair[512], hash_string[48], signature[48], query[128];
    unsigned char hash_bin[EVP_MAX_MD_SIZE];
    EVP_MD_CTX *mdctx;
    int md_len;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    JsonNode *RootNode = Json_node_create();
    Json_node_add_bool  ( RootNode, "noStopClause", TRUE );
    Json_node_add_string( RootNode, "priority", "high" );
    Json_node_add_bool  ( RootNode, "senderForResponse", TRUE );
    Json_node_add_int   ( RootNode, "validityPeriod", 2880 ); /* 2 jours */
    Json_node_add_string( RootNode, "charset", "UTF-8" );

    JsonArray *receivers = Json_node_add_array ( RootNode, "receivers" );
    Json_array_add_element ( receivers, json_node_init_string ( json_node_alloc(), telephone ) );

    gchar libelle[128];
    g_snprintf( libelle, sizeof(libelle), "%s: %s", Json_get_string ( msg, "dls_shortname" ), Json_get_string( msg, "libelle") );
    Json_node_add_string( RootNode, "message", libelle );
    gchar *body = Json_node_to_string( RootNode );
    json_node_unref(RootNode);

    gchar *method = "POST";
    g_snprintf( query, sizeof(query), "https://eu.api.ovh.com/1.0/sms/%s/jobs", Json_get_string ( module->config, "ovh_service_name" ) );
    gchar timestamp[20];
    g_snprintf( timestamp, sizeof(timestamp), "%ld", time(NULL) );

/******************************************************* Calcul signature *****************************************************/
    g_snprintf( clair, sizeof(clair), "%s+%s+%s+%s+%s+%s",
                Json_get_string ( module->config, "ovh_application_secret" ),
                Json_get_string ( module->config, "ovh_consumer_key" ),
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
    Info_new ( Config.log, module->lib->Thread_debug, LOG_DEBUG, "Sending to OVH : %s", body );
    soup_message_set_request ( soup_msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, body, strlen(body) );
    SoupMessageHeaders *headers;
    g_object_get ( G_OBJECT(soup_msg), "request_headers", &headers, NULL );
    soup_message_headers_append ( headers, "X-Ovh-Application", Json_get_string ( module->config, "ovh_application_key" ) );
    soup_message_headers_append ( headers, "X-Ovh-Consumer",    Json_get_string ( module->config, "ovh_consumer_key" ) );
    soup_message_headers_append ( headers, "X-Ovh-Signature",   signature );
    soup_message_headers_append ( headers, "X-Ovh-Timestamp",   timestamp );
    soup_session_send_message (connexion, soup_msg);

    GBytes *response_brute;
    gchar *reason_phrase;
    gint status_code;

    g_object_get ( soup_msg, "status-code", &status_code, "reason-phrase", &reason_phrase, "response-body-data", &response_brute, NULL );
    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: %s: Status %d, reason %s", __func__, thread_tech_id, status_code, reason_phrase );
    if (status_code!=200)
     { gsize taille;
       gchar *error = g_bytes_get_data ( response_brute, &taille );
       Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: Error: %s\n", __func__, thread_tech_id, error );
       g_free(error);
     }
    else Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: %s: '%s' sent to '%s'", __func__, thread_tech_id, libelle, telephone );
    g_object_unref( soup_msg );
    soup_session_abort ( connexion );
  }
/******************************************************************************************************************************/
/* Smsg_send_to_all_authorized_recipients : Envoi à tous les portables autorisés                                              */
/* Entrée: le message                                                                                                         */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Smsg_send_to_all_authorized_recipients ( struct SUBPROCESS *module, JsonNode *msg )
  { struct SMS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (vars->sending_is_disabled == TRUE)                                   /* Si envoi désactivé, on sort de suite de la fonction */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: %s: Sending is disabled. Dropping message", __func__, thread_tech_id );
       return;
     }

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: %s: Memory Error", __func__, thread_tech_id );
       return;
     }

/********************************************* Chargement des informations en bases *******************************************/
    SQL_Select_to_json_node ( RootNode, "recipients",
                              "SELECT id,username,enable,comment,notification,phone,allow_cde "
                              "FROM users AS user WHERE enable=1 AND notification=1 ORDER BY username" );

    gint sms_notification = Json_get_int ( msg, "sms_notification" );
    GList *recipients = json_array_get_elements ( Json_get_array ( RootNode, "recipients" ) );
    while(recipients)
     { JsonNode *element = recipients->data;
       gchar *user_phone = Json_get_string ( element, "phone" );
       switch (sms_notification)
        { case MESSAGE_SMS_YES:
               if ( Envoi_sms_gsm ( module, msg, user_phone ) == FALSE )
                { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                            "%s: %s: Error sending with GSM. Falling back to OVH", __func__, thread_tech_id );
                  Envoi_sms_ovh( module, msg, user_phone );
                }
               break;
          case MESSAGE_SMS_GSM_ONLY:
               Envoi_sms_gsm ( module, msg, user_phone );
               break;
          case MESSAGE_SMS_OVH_ONLY:
               Envoi_sms_ovh ( module, msg, user_phone );
               break;
        }
       recipients = g_list_next(recipients);
     }
    g_list_free(recipients);
    json_node_unref ( RootNode );
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un texte au format UTF8 si possible                                                                                */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_smsg_ovh_text ( struct SUBPROCESS *module, gchar *texte )
  { JsonNode *RootNode = Json_node_create();
    Json_node_add_string ( RootNode, "libelle", texte );
    Json_node_add_string ( RootNode, "dls_shortname", Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_add_int    ( RootNode, "sms_notification", MESSAGE_SMS_OVH_ONLY );
    Smsg_send_to_all_authorized_recipients( module, RootNode );
    json_node_unref(RootNode);
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un texte au format UTF8 si possible                                                                                */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_smsg_gsm_text ( struct SUBPROCESS *module, gchar *texte )
  { JsonNode *RootNode = Json_node_create();
    Json_node_add_string ( RootNode, "libelle", texte );
    Json_node_add_string ( RootNode, "dls_shortname", Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_add_int    ( RootNode, "sms_notification", MESSAGE_SMS_GSM_ONLY );
    Smsg_send_to_all_authorized_recipients( module, RootNode );
    json_node_unref(RootNode);
  }
/******************************************************************************************************************************/
/* Sms_Envoyer_commande_dls_data : Envoi un message json au client en parametre data                                         */
/* Entrée : Le tableau, l'index, l'element json et le destinataire                                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Sms_Envoyer_commande_dls_data ( JsonArray *array, guint index, JsonNode *element, void *user_data )
  { struct SUBPROCESS *module = user_data;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *tech_id = Json_get_string ( element, "tech_id" );
    gchar *acro    = Json_get_string ( element, "acronyme" );
    gchar *libelle = Json_get_string ( element, "libelle" );
    gchar *map_tag = Json_get_string ( element, "map_tag" );
    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: %s: Match found -> '%s' '%s:%s' - %s", __func__,
              thread_tech_id, map_tag, tech_id, acro, libelle );
    Zmq_Send_CDE_to_master_new ( module, thread_tech_id, acro );
  }
/******************************************************************************************************************************/
/* Traiter_commande_sms: Fonction appelée pour traiter la commande sms recu par le telephone                                  */
/* Entrée: le message text à traiter                                                                                          */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Traiter_commande_sms ( struct SUBPROCESS *module, gchar *from, gchar *texte )
  { struct SMS_VARS *vars = module->vars;
    gchar chaine[160];

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    if ( Smsg_is_allow_cde ( module, from ) == FALSE )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE,
                "%s: %s: unknown sender %s. Dropping message %s...", __func__, thread_tech_id, from, texte );
       return;
     }

    if ( ! strcasecmp( texte, "ping" ) )                                                               /* Interfacage de test */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: %s: Ping Received from '%s'. Sending Pong", __func__, thread_tech_id, from );
       Envoyer_smsg_gsm_text ( module, "Pong !" );
       return;
     }

    if ( ! strcasecmp( texte, "smsoff" ) )                                                                      /* Smspanic ! */
     { vars->sending_is_disabled = TRUE;
       Envoyer_smsg_gsm_text ( module, "Sending SMS is off !" );
       Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: %s: Sending SMS is DISABLED by '%s'", __func__, thread_tech_id, from );
       return;
     }

    if ( ! strcasecmp( texte, "smson" ) )                                                                       /* Smspanic ! */
     { Envoyer_smsg_gsm_text ( module, "Sending SMS is on !" );
       Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: %s: Sending SMS is ENABLED by '%s'", __func__, thread_tech_id, from );
       vars->sending_is_disabled = FALSE;
       return;
     }

    JsonNode *RootNode = Json_node_create();
    if ( RootNode == NULL )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: Memory Error for '%s'", __func__, thread_tech_id, from );
       return;
     }
    SQL_Select_to_json_node ( RootNode, "results",
                              "SELECT * FROM mnemos_DI AS m "
                              "INNER JOIN mappings_text AS map ON m.tech_id = map.tech_id AND m.acronyme = map.acronyme "
                              "WHERE map.tag LIKE '%%%s%%'", texte );

    if ( Json_has_member ( RootNode, "nbr_results" ) == FALSE )
     { g_snprintf(chaine, sizeof(chaine), "'%s' not found.", texte );
       Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: Error searching Database for '%s'", __func__, thread_tech_id, texte );
     }
    else
     { gint nbr_results = Json_get_int ( RootNode, "nbr_results" );
       if ( nbr_results == 0 )
        { g_snprintf(chaine, sizeof(chaine), "'%s' not found.", texte ); }                 /* Envoi de l'erreur si pas trouvé */
       else if ( nbr_results > 1 )                                           /* Si trop d'enregistrement, demande de préciser */
        { g_snprintf(chaine, sizeof(chaine), "Trop de résultats pour '%s'.", texte ); }    /* Envoi de l'erreur si pas trouvé */
       else
        { Json_node_foreach_array_element ( RootNode, "results", Sms_Envoyer_commande_dls_data, module );
          g_snprintf(chaine, sizeof(chaine), "'%s': fait.", texte );
        }
      }
    json_node_unref( RootNode );
    Envoyer_smsg_gsm_text ( module, chaine );
  }
/******************************************************************************************************************************/
/* Lire_sms_gsm: Lecture de tous les SMS du GSM                                                                               */
/* Entrée: Rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Lire_sms_gsm ( struct SUBPROCESS *module )
  { struct SMS_VARS *vars = module->vars;
    gchar from[80], texte[180];
    GSM_MultiSMSMessage sms;
    GSM_Error error;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    memset(&sms, 0, sizeof(sms));                                                                       /* Préparation du SMS */

/* Read all messages */
    error = ERR_NONE;
    sms.Number = 0;
    sms.SMS[0].Location = 0;
    sms.SMS[0].Folder = 0;
    error = GSM_GetNextSMS(vars->gammu_machine, &sms, TRUE);
    if (error == ERR_NONE)
     { gint i;
       for (i = 0; i < sms.Number; i++)
        { g_snprintf( from, sizeof(from), "%s", DecodeUnicodeConsole(sms.SMS[i].Number) );
          g_snprintf( texte, sizeof(texte), "%s", DecodeUnicodeConsole(sms.SMS[i].Text) );
          sms.SMS[0].Folder = 0;/* https://github.com/gammu/gammu/blob/ed2fec4a382e7ac4b5dfc92f5b10811f76f4817e/gammu/message.c */
          if (sms.SMS[i].State == SMS_UnRead)                                /* Pour tout nouveau message, nous le processons */
           { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE,
                      "%s: %s: Recu '%s' from '%s' Location %d/%d Folder %d !", __func__,
                       thread_tech_id, texte, from, i, sms.SMS[i].Location, sms.SMS[i].Folder );
             Traiter_commande_sms ( module, from, texte );
           }
          error = GSM_DeleteSMS( vars->gammu_machine, &sms.SMS[i] );
          if (error != ERR_NONE)
           { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                      "%s: %s: Delete '%s' from '%s' Location %d/%d Folder %d Failed ('%s')!", __func__,
                       thread_tech_id, texte, from, i, sms.SMS[i].Location, sms.SMS[i].Folder, GSM_ErrorString(error) );
           }
         }
      }
     else if (error != ERR_EMPTY)
      { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                  "%s: %s: Error Reading SMS: '%s' !", __func__, thread_tech_id, GSM_ErrorString(error) );
      }
    if ( (error == ERR_NONE) || (error == ERR_EMPTY) ) { return(TRUE); }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct SMS_VARS) );
    struct SMS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    /*Envoyer_smsg_gsm_text ( "SMS System is running" );*/
    vars->sending_is_disabled = FALSE;                                                /* A l'init, l'envoi de SMS est autorisé */
    gint next_try = 0;

    while(module->lib->Thread_run == TRUE && module->lib->Thread_reload == FALSE)            /* On tourne tant que necessaire */
     { usleep(100000);
       sched_yield();

       SubProcess_send_comm_to_master_new ( module, module->comm_status );         /* Périodiquement envoie la comm au master */

/****************************************************** Tentative de connexion ************************************************/
       if (module->comm_status == FALSE && Partage->top >= next_try )
        { if (Smsg_connect(module)==FALSE)
           { next_try = Partage->top + 300;
             Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: %s: Connect failed, trying in 30s", __func__, thread_tech_id );
           }
        }

/****************************************************** Lecture de SMS ********************************************************/
       if (module->comm_status == TRUE)
        { if (Lire_sms_gsm(module)==FALSE) { Smsg_disconnect(module); }
        }
/********************************************************* Envoi de SMS *******************************************************/
       JsonNode *request;
       while ( (request = SubProcess_Listen_to_master_new ( module ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "DLS_HISTO" ) &&
               Json_get_bool ( request, "alive" ) == TRUE &&
               Json_get_int  ( request, "sms_notification" ) != MESSAGE_SMS_NONE )
           { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: %s: Sending msg '%s:%s' (%s)", __func__, thread_tech_id,
                       Json_get_string ( request, "thread_tech_id" ), Json_get_string ( request, "acronyme" ),
                       Json_get_string ( request, "libelle" ) );

/*************************************************** Envoi en mode GSM ********************************************************/
             Smsg_send_to_all_authorized_recipients( module, request );
           }
          else if ( !strcasecmp ( zmq_tag, "test_gsm" ) ) Envoyer_smsg_gsm_text ( module, "Test SMS GSM OK !" );
          else if ( !strcasecmp ( zmq_tag, "test_ovh" ) ) Envoyer_smsg_ovh_text ( module, "Test SMS OVH OK !" );
          else
           { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: %s: zmq_tag '%s' not for this thread", __func__, thread_tech_id, zmq_tag ); }
          json_node_unref(request);
        }
     }
    Smsg_disconnect(module);

    SQL_Write_new ( "UPDATE %s SET nbr_sms='%d' WHERE uuid='%s'", module->lib->name, vars->nbr_sms,  module->lib->uuid );

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
    Smsg_Creer_DB ( lib );                                                                     /* Création de la DB du thread */
    Thread_init ( "smsg", "USER", lib, WTD_VERSION, "Manage SMS system (libgammu)" );

    lib->config = Json_node_create();
    if(lib->config) SQL_Select_to_json_node ( lib->config, "subprocess", "SELECT * FROM %s WHERE uuid='%s'", lib->name, lib->uuid );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: %d subprocess to load", __func__, Json_get_int ( lib->config, "nbr_subprocess" ) );

    Json_node_foreach_array_element ( lib->config, "subprocess", Process_Load_one_subprocess, lib );   /* Chargement des modules */
    while( lib->Thread_run == TRUE && lib->Thread_reload == FALSE) sleep(1);                 /* On tourne tant que necessaire */
    Process_Unload_all_subprocess ( lib );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }

    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
