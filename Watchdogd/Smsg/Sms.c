/******************************************************************************************************************************/
/* Watchdogd/sms.c        Gestion des SMS de Watchdog v2.0                                                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       ven. 02 avril 2010 20:37:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Sms.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
/* Smsg_Send_CB: Appelé par le téléphone quand le SMS est parti                                                               */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Smsg_Send_CB (GSM_StateMachine *sm, int status, int MessageReference, void * user_data)
  { struct THREAD *module = user_data;
    struct SMS_VARS *vars = module->vars;
    if (status==0) { vars->gammu_send_status = ERR_NONE; }
    else vars->gammu_send_status = ERR_UNKNOWN;
    Info_new( __func__, module->Thread_debug, LOG_DEBUG,
              "status = %d, vars->gammu_send_status=%d", status, vars->gammu_send_status );
  }
/******************************************************************************************************************************/
/* Smsg_disconnect: Se deconnecte du telephone ou de la clef 3G                                                               */
/* Entrée: Rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Smsg_disconnect ( struct THREAD *module )
  { struct SMS_VARS *vars = module->vars;
    GSM_Error error;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    if (GSM_IsConnected(vars->gammu_machine))
     { error = GSM_TerminateConnection(vars->gammu_machine);                                  /* Terminate connection */
       if (error != ERR_NONE)
        { Info_new( __func__, module->Thread_debug, LOG_ERR,
                   "%s: TerminateConnection Failed (%s)", thread_tech_id, GSM_ErrorString(error) );
        }
     }
    GSM_FreeStateMachine(vars->gammu_machine);                                                 /* Free up used memory */
    vars->gammu_machine = NULL;
    Info_new( __func__, module->Thread_debug, LOG_INFO, "%s: Disconnected", thread_tech_id );
  }
/******************************************************************************************************************************/
/* smsg_connect: Ouvre une connexion vers le téléphone ou la clef 3G                                                          */
/* Entrée: Rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Smsg_connect ( struct THREAD *module )
  { struct SMS_VARS *vars = module->vars;
    GSM_Error error;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    Info_new( __func__, module->Thread_debug, LOG_INFO, "%s: Trying to connect", thread_tech_id );

    GSM_InitLocales(NULL);
    if ( (vars->gammu_machine = GSM_AllocStateMachine()) == NULL )                         /* Allocates state machine */
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: AllocStateMachine Error", thread_tech_id );
       return(FALSE);
     }

    error = GSM_FindGammuRC(&vars->gammu_cfg, NULL);
    if (error != ERR_NONE)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: FindGammuRC Failed (%s)", thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    error = GSM_ReadConfig(vars->gammu_cfg, GSM_GetConfig(vars->gammu_machine, 0), 0);
    if (error != ERR_NONE)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                "%s: ReadConfig Failed (%s)", thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    INI_Free(vars->gammu_cfg);
    GSM_SetConfigNum(vars->gammu_machine, 1);

    error = GSM_InitConnection(vars->gammu_machine, 1);
    if (error != ERR_NONE)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: InitConnection Failed (%s)", thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }
    GSM_SetSendSMSStatusCallback(vars->gammu_machine, Smsg_Send_CB, module);

    gchar constructeur[64];
    error = GSM_GetManufacturer(vars->gammu_machine, constructeur);
    if (error != ERR_NONE)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: GSM_GetManufacturer Failed (%s)", thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    gchar model[64];
    error = GSM_GetModel(vars->gammu_machine, model);
    if (error != ERR_NONE)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: GSM_GetModel Failed (%s)", thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    Info_new( __func__, module->Thread_debug, LOG_INFO,
              "%s: Connection OK with '%s/%s'", thread_tech_id, constructeur, model );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Envoi_sms_gsm: Envoi un sms par le gsm                                                                                     */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Envoi_sms_gsm ( struct THREAD *module, JsonNode *msg, gchar *telephone )
  { struct SMS_VARS *vars = module->vars;
    GSM_SMSMessage sms;
    GSM_SMSC PhoneSMSC;
    gchar libelle[256];
    GSM_Error error;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (!telephone)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: telephone is NULL", thread_tech_id );
       return(FALSE);
     }

    if (!Smsg_connect(module))
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Connect failed, cannot send SMS to '%s'", thread_tech_id, telephone );
       return(FALSE);
     }

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
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                "%s: GetSMSC Failed (%s)", thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    CopyUnicodeString(sms.SMSC.Number, PhoneSMSC.Number);                                       /* Set SMSC number in message */

    Info_new( __func__, module->Thread_debug, LOG_DEBUG,
              "%s: Try to send to %s (%s)", thread_tech_id, telephone, libelle );

    vars->gammu_send_status = ERR_TIMEOUT;
    error = GSM_SendSMS(vars->gammu_machine, &sms);                                                        /* Send message */
    if (error != ERR_NONE)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: SendSMS Failed (%s)", thread_tech_id, GSM_ErrorString(error) );
       Smsg_disconnect(module);
       return(FALSE);
     }

    while ( module->Thread_run == TRUE && vars->gammu_send_status == ERR_TIMEOUT )
     { GSM_ReadDevice(vars->gammu_machine, TRUE); }

    Smsg_disconnect(module);

    if (vars->gammu_send_status == ERR_NONE)
     { Info_new( __func__, module->Thread_debug, LOG_NOTICE,
                 "%s: Envoi SMS Ok to %s (%s)", thread_tech_id, telephone, libelle );
       vars->nbr_sms++;
       return(TRUE);
     }
    Info_new( __func__, module->Thread_debug, LOG_WARNING,
             "%s: Envoi SMS Nok to %s (%s) -> error '%s'", thread_tech_id, telephone, libelle, GSM_ErrorString(error) );
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Envoi_sms_smsbox: Envoi un sms par SMSBOX                                                                                  */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoi_sms_ovh ( struct THREAD *module, JsonNode *msg, gchar *telephone )
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

    gchar *method = "POST";
    g_snprintf( query, sizeof(query), "https://eu.api.ovh.com/1.0/sms/%s/jobs", Json_get_string ( module->config, "ovh_service_name" ) );
    gchar timestamp[20];
    g_snprintf( timestamp, sizeof(timestamp), "%ld", time(NULL) );

/******************************************************* Calcul signature *****************************************************/
    gchar *body = Json_node_to_string( RootNode );
    g_snprintf( clair, sizeof(clair), "%s+%s+%s+%s+%s+%s",
                Json_get_string ( module->config, "ovh_application_secret" ),
                Json_get_string ( module->config, "ovh_consumer_key" ),
                method, query, body, timestamp );
    Info_new ( __func__, module->Thread_debug, LOG_DEBUG, "Sending to OVH : %s", body );
    g_free(body);

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
    SoupMessage *soup_msg = soup_message_new ( method, query );
    SoupMessageHeaders *headers = soup_message_get_request_headers( soup_msg );
    soup_message_headers_append ( headers, "X-Ovh-Application", Json_get_string ( module->config, "ovh_application_key" ) );
    soup_message_headers_append ( headers, "X-Ovh-Consumer",    Json_get_string ( module->config, "ovh_consumer_key" ) );
    soup_message_headers_append ( headers, "X-Ovh-Signature",   signature );
    soup_message_headers_append ( headers, "X-Ovh-Timestamp",   timestamp );
    JsonNode *response = Http_Send_json_request_from_thread ( module, soup_msg, RootNode );
    Json_node_unref ( response );
    Json_node_unref ( RootNode );

    gint status_code = soup_message_get_status ( soup_msg );
    if (status_code!=200)
     { gchar *reason_phrase = soup_message_get_reason_phrase ( soup_msg );
       Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Status %d, reason %s", thread_tech_id, status_code, reason_phrase );
     }
    else Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: '%s' sent to '%s'", thread_tech_id, libelle, telephone );
    g_object_unref( soup_msg );
  }
/******************************************************************************************************************************/
/* Smsg_send_to_all_authorized_recipients : Envoi à tous les portables autorisés                                              */
/* Entrée: le message                                                                                                         */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Smsg_send_to_all_authorized_recipients ( struct THREAD *module, JsonNode *msg )
  { struct SMS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (vars->sending_is_disabled == TRUE)                                   /* Si envoi désactivé, on sort de suite de la fonction */
     { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending is disabled. Dropping message", thread_tech_id );
       return;
     }

/********************************************* Chargement des informations en bases *******************************************/
    JsonNode *UsersNode = Http_Get_from_global_API ( "/run/users/wanna_be_notified", NULL );
    if (!UsersNode || Json_get_int ( UsersNode, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Could not get USERS from API", thread_tech_id );
       return;
     }

    gint txt_notification = Json_get_int ( msg, "txt_notification" );
    GList *Recipients = json_array_get_elements ( Json_get_array ( UsersNode, "recipients" ) );
    GList *recipients = Recipients;
    while(recipients)
     { JsonNode *user = recipients->data;
       gchar *user_phone = Json_get_string ( user, "phone" );
       if (!user_phone)
        { Info_new( __func__, module->Thread_debug, LOG_ERR,
                    "%s: Warning: User %s does not have an Phone number", thread_tech_id, Json_get_string ( user, "email" ) );
        }
       else if (!strlen(user_phone))
        { Info_new( __func__, module->Thread_debug, LOG_ERR,
                    "%s: Warning: User %s has an empty Phone number", thread_tech_id, Json_get_string ( user, "email" ) );
        }
       else switch (txt_notification)
        { case TXT_NOTIF_YES:
               if ( Envoi_sms_gsm ( module, msg, user_phone ) == FALSE )
                { Info_new( __func__, module->Thread_debug, LOG_ERR,
                            "%s: Error sending with GSM. Falling back to OVH", thread_tech_id );
                  Envoi_sms_ovh( module, msg, user_phone );
                }
               break;
          case TXT_NOTIF_GSM_ONLY:
               Envoi_sms_gsm ( module, msg, user_phone );
               break;
          case TXT_NOTIF_OVH_ONLY:
               Envoi_sms_ovh ( module, msg, user_phone );
               break;
        }
       recipients = g_list_next(recipients);
     }
    g_list_free(Recipients);
    Json_node_unref ( UsersNode );
    Http_Post_thread_AI_to_local_BUS ( module, vars->ai_nbr_sms, vars->nbr_sms, TRUE );
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un texte au format UTF8 si possible                                                                                */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_smsg_ovh_text ( struct THREAD *module, gchar *texte )
  { JsonNode *RootNode = Json_node_create();
    Json_node_add_string ( RootNode, "libelle", texte );
    Json_node_add_string ( RootNode, "dls_shortname", Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_add_int    ( RootNode, "txt_notification", TXT_NOTIF_OVH_ONLY );
    Smsg_send_to_all_authorized_recipients( module, RootNode );
    Json_node_unref(RootNode);
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un texte au format UTF8 si possible                                                                                */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_smsg_gsm_text ( struct THREAD *module, gchar *texte )
  { JsonNode *RootNode = Json_node_create();
    Json_node_add_string ( RootNode, "libelle", texte );
    Json_node_add_string ( RootNode, "dls_shortname", Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_add_int    ( RootNode, "txt_notification", TXT_NOTIF_GSM_ONLY );
    Smsg_send_to_all_authorized_recipients( module, RootNode );
    Json_node_unref(RootNode);
  }
/******************************************************************************************************************************/
/* Traiter_commande_sms: Fonction appelée pour traiter la commande sms recu par le telephone                                  */
/* Entrée: le message text à traiter                                                                                          */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Traiter_commande_sms ( struct THREAD *module, gchar *from, gchar *texte )
  { struct SMS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    JsonNode *RootNode = Json_node_create();
    if ( RootNode == NULL )
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Memory Error for '%s'", thread_tech_id, from );
       return;
     }
    Json_node_add_string ( RootNode, "phone", from );

    JsonNode *UserNode = Http_Post_to_global_API ( "/run/user/can_send_txt_cde", RootNode );
    Json_node_unref ( RootNode );
    if (!UserNode || Json_get_int ( UserNode, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Could not get USER from API for '%s'", thread_tech_id, from );
       goto end_user;
     }

    if ( !Json_has_member ( UserNode, "email" ) )
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                "%s: %s is not an known user. Dropping command '%s'...", thread_tech_id, from, texte );
       goto end_user;
     }

    if ( !Json_has_member ( UserNode, "can_send_txt_cde" ) || Json_get_bool ( UserNode, "can_send_txt_cde" ) == FALSE )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: %s ('%s') is not allowed to send txt_cde. Dropping command '%s'...", thread_tech_id,
                from, Json_get_string ( UserNode, "email" ), texte );
       goto end_user;
     }

    if ( ! strcasecmp( texte, "ping" ) )                                                               /* Interfacage de test */
     { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Ping Received from '%s'. Sending Pong", thread_tech_id, from );
       Envoyer_smsg_gsm_text ( module, "Pong !" );
       goto end_user;
     }

    if ( ! strcasecmp( texte, "smsoff" ) )                                                                      /* Smspanic ! */
     { vars->sending_is_disabled = TRUE;
       Envoyer_smsg_gsm_text ( module, "Sending SMS is off !" );
       Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending SMS is DISABLED by '%s'", thread_tech_id, from );
       goto end_user;
     }

    if ( ! strcasecmp( texte, "smson" ) )                                                                       /* Smspanic ! */
     { Envoyer_smsg_gsm_text ( module, "Sending SMS is on !" );
       Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending SMS is ENABLED by '%s'", thread_tech_id, from );
       vars->sending_is_disabled = FALSE;
       goto end_user;
     }

    RootNode = Json_node_create();
    if ( RootNode == NULL )
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: MapNode Error for '%s'", thread_tech_id, from );
       goto end_user;
     }
    Json_node_add_string ( RootNode, "thread_tech_id", "_COMMAND_TEXT" );
    Json_node_add_string ( RootNode, "thread_acronyme", texte );

    JsonNode *MapNode = Http_Post_to_global_API ( "/run/mapping/search_txt", RootNode );
    Json_node_unref ( RootNode );
    if (!MapNode || Json_get_int ( MapNode, "api_status" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Could not get USER from API for '%s'", thread_tech_id, from );
       goto end_map;
     }

    if ( Json_has_member ( MapNode, "nbr_results" ) == FALSE )
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': Error searching Database for '%s'", thread_tech_id, texte );
       Envoyer_smsg_gsm_text ( module, "Error searching Database .. Sorry .." );
       goto end_map;
     }

    gint nbr_results = Json_get_int ( MapNode, "nbr_results" );
    if ( nbr_results == 0 )
     { Envoyer_smsg_gsm_text ( module, "Je n'ai pas trouvé, désolé." ); }
    else
     { if ( nbr_results > 1 )                                               /* Si trop d'enregistrements, demande de préciser */
        { Envoyer_smsg_gsm_text ( module, "Aîe, plusieurs choix sont possibles ... :" ); }

       GList *Results = json_array_get_elements ( Json_get_array ( MapNode, "results" ) );
       if ( nbr_results > 1 )
        { GList *results = Results;
          while(results)
           { JsonNode *element = results->data;
             gchar *thread_acronyme = Json_get_string ( element, "thread_acronyme" );
             gchar *tech_id         = Json_get_string ( element, "tech_id" );
             gchar *acronyme        = Json_get_string ( element, "acronyme" );
             gchar *libelle         = Json_get_string ( element, "libelle" );
             Info_new( __func__, module->Thread_debug, LOG_INFO, "'%s': From '%s' map found for '%s' -> '%s:%s' - %s",
                       thread_tech_id, from, thread_acronyme, tech_id, acronyme, libelle );
             Envoyer_smsg_gsm_text ( module, thread_acronyme );                                 /* Envoi des différents choix */
             results = g_list_next(results);
           }
        }
       else if ( nbr_results == 1)
        { JsonNode *element = Results->data;
          gchar *thread_acronyme = Json_get_string ( element, "thread_acronyme" );
          gchar *tech_id         = Json_get_string ( element, "tech_id" );
          gchar *acronyme        = Json_get_string ( element, "acronyme" );
          gchar *libelle         = Json_get_string ( element, "libelle" );
          Info_new( __func__, module->Thread_debug, LOG_INFO, "'%s': From '%s' map found for '%s' (%s)-> '%s:%s' - %s",
                    thread_tech_id, from, Json_get_string( UserNode, "email" ), thread_acronyme, tech_id, acronyme, libelle );
          Http_Post_to_local_BUS_CDE ( module, tech_id, acronyme );
          gchar chaine[256];
          g_snprintf ( chaine, sizeof(chaine), "'%s' fait.", texte );
          Envoyer_smsg_gsm_text ( module, chaine );
        }
       g_list_free(Results);
     }
end_map:
    Json_node_unref ( MapNode );
end_user:
    Json_node_unref ( UserNode );
  }
/******************************************************************************************************************************/
/* Lire_sms_gsm: Lecture de tous les SMS du GSM                                                                               */
/* Entrée: Rien                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Lire_sms_gsm ( struct THREAD *module )
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
           { Info_new( __func__, module->Thread_debug, LOG_NOTICE,
                      "%s: Recu '%s' from '%s' Location %d/%d Folder %d !",
                       thread_tech_id, texte, from, i, sms.SMS[i].Location, sms.SMS[i].Folder );
             Traiter_commande_sms ( module, from, texte );
           }
          error = GSM_DeleteSMS( vars->gammu_machine, &sms.SMS[i] );
          if (error != ERR_NONE)
           { Info_new( __func__, module->Thread_debug, LOG_ERR,
                      "%s: Delete '%s' from '%s' Location %d/%d Folder %d Failed ('%s')!",
                       thread_tech_id, texte, from, i, sms.SMS[i].Location, sms.SMS[i].Folder, GSM_ErrorString(error) );
           }
         }
      }
     else if (error != ERR_EMPTY)
      { Info_new( __func__, module->Thread_debug, LOG_ERR,
                  "%s: Error Reading SMS: '%s' !", thread_tech_id, GSM_ErrorString(error) );
      }
    if ( (error == ERR_NONE) || (error == ERR_EMPTY) ) { return(TRUE); }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct SMS_VARS) );
    struct SMS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    vars->sending_is_disabled = FALSE;                                               /* A l'init, l'envoi de SMS est autorisé */
    vars->ai_nbr_sms        = Mnemo_create_thread_AI ( module, "NBR_SMS", "Nombre de SMS envoyés", "sms", ARCHIVE_1_HEURE );
    vars->ai_signal_quality = Mnemo_create_thread_AI ( module, "SIGNAL_QUALITY", "Qualité du signal", "%", ARCHIVE_1_HEURE );
    vars->nbr_sms  = 0;
    gint next_read = 0;
    Envoyer_smsg_gsm_text ( module, "SMS System is running" );

    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */

/****************************************************** Ecoute du master ******************************************************/
       while ( module->WS_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *message = module->WS_messages->data;
          module->WS_messages = g_slist_remove ( module->WS_messages, message );
          pthread_mutex_unlock ( &module->synchro );
          gchar *tag = Json_get_string ( message, "tag" );
          if ( !strcasecmp( tag, "DLS_HISTO" ) && Json_get_bool ( message, "alive" ) == TRUE &&
               Json_get_int ( message, "txt_notification" ) != TXT_NOTIF_NONE )
           { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending msg '%s:%s' (%s)", thread_tech_id,
                       Json_get_string ( message, "tech_id" ), Json_get_string ( message, "acronyme" ),
                       Json_get_string ( message, "libelle" ) );

/*************************************************** Envoi en mode GSM ********************************************************/
             Smsg_send_to_all_authorized_recipients( module, message );
           }
          else if ( !strcasecmp ( tag, "test_gsm" ) ) Envoyer_smsg_gsm_text ( module, "Test SMS GSM OK !" );
          else if ( !strcasecmp ( tag, "test_ovh" ) ) Envoyer_smsg_ovh_text ( module, "Test SMS OVH OK !" );
          else
           { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "%s: tag '%s' not for this thread", thread_tech_id, tag ); }
          Json_node_unref(message);
        }
/****************************************************** Lecture de SMS ********************************************************/
       if (Partage->top < next_read) continue;
       next_read = Partage->top + 50;
       if (Smsg_connect(module))
        { Thread_send_comm_to_master ( module, TRUE );
          Lire_sms_gsm(module);
          GSM_SignalQuality sig;
          GSM_GetSignalQuality( vars->gammu_machine, &sig );
          Http_Post_thread_AI_to_local_BUS ( module, vars->ai_signal_quality, 1.0*sig.SignalPercent, TRUE );
        }
       else
        { Info_new( __func__, module->Thread_debug, LOG_INFO, "Connect failed" );
          Thread_send_comm_to_master ( module, FALSE );
        }
       Smsg_disconnect(module);
     }
    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
