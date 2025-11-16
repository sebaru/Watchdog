/******************************************************************************************************************************/
/* Watchdogd/mqtt_MSGxxx.c        Distribution des messages DLS à l'API                                                       */
/* Projet Abls-Habitat version 4.6       Gestion d'habitat                                    mar. 14 août 2012 19:05:42 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mqtt_MSGxxx.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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
 #include <string.h>
 #include <unistd.h>
 #include <time.h>

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static gchar *MSGS_Get_datetime_usec ( gchar *buffer, gint taille_buffer )
  { gchar chaine[256];
    struct timeval tv;
    struct tm local;
    gettimeofday( &tv, NULL );
    localtime_r( (time_t *)&tv.tv_sec, &local );
    strftime( chaine, sizeof(chaine), "%F %T", &local );
    gchar *date_utf8 = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
    g_snprintf( buffer, taille_buffer, "%s.%02d", date_utf8, (gint)tv.tv_usec/10000 );
    return(buffer);
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static JsonNode *MSGS_Convert_msg_off_to_histo ( struct DLS_MESSAGE *msg )
  { JsonNode *histo = Json_node_create ();
    if (histo)
     { Json_node_add_string ( histo, "tech_id",  msg->tech_id );
       Json_node_add_string ( histo, "acronyme", msg->acronyme );
       Json_node_add_bool   ( histo, "alive", FALSE );
       gchar date_fin[256];
       MSGS_Get_datetime_usec ( date_fin, sizeof(date_fin) );
       Json_node_add_string ( histo, "date_fin", date_fin );
     }
    return( histo );
  }
/******************************************************************************************************************************/
/* MQTT_Send_MSGS_to_API: Envoi les messages a l'API                                                                          */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void MQTT_Send_MSGS_to_API ( void )
  { gint cpt = 0;

    while (Partage->Liste_msg && Partage->Thread_run == TRUE && cpt<100)
     { pthread_rwlock_wrlock( &Partage->Liste_msg_synchro );                          /* Ajout dans la liste de msg a traiter */
       struct DLS_MESSAGE_EVENT *event = Partage->Liste_msg->data;                           /* Recuperation du numero de msg */
       struct DLS_MESSAGE *msg = event->msg;
       Partage->Liste_msg = g_slist_remove ( Partage->Liste_msg, event );
       gint reste_a_faire = g_slist_length(Partage->Liste_msg);
       pthread_rwlock_unlock( &Partage->Liste_msg_synchro );                          /* Ajout dans la liste de msg a traiter */
       Info_new( __func__, Config.log_msrv, LOG_INFO, "Handle MSG'%s:%s'=%d, Reste a %d a traiter",
                 msg->tech_id, msg->acronyme, event->etat, reste_a_faire );

       if (event->etat == 1)
        { gint rate_limit = Json_get_int ( msg->source_node, "rate_limit" );
          if ( !msg->last_on || (Partage->top >= msg->last_on + rate_limit*10 ) )
           { msg->last_on = Partage->top;
             gchar date_create[128];
             MSGS_Get_datetime_usec ( date_create, sizeof(date_create) );            /* Mise à jour de de la date de création */
             Json_node_add_string ( msg->source_node, "date_create", date_create );

                                                                                  /* Préparation du nouveau libelle dynamique */
             gchar *libelle_new = Convert_libelle_dynamique ( Json_get_string(msg->source_node, "libelle_src") );
             Json_node_add_string ( msg->source_node, "libelle", libelle_new );
             g_free(libelle_new);
             gchar *tech_id       = Json_get_string ( msg->source_node, "tech_id" );
             gchar *acronyme      = Json_get_string ( msg->source_node, "acronyme" );
             gchar *dls_shortname = Json_get_string ( msg->source_node, "dls_shortname" );
             gchar *libelle       = Json_get_string ( msg->source_node, "libelle" );
/*------------------------------------------------ Envoi vers API ------------------------------------------------------------*/
             JsonNode *MSGNode = Json_node_create();
             if (MSGNode)
              { Json_node_add_string ( MSGNode, "tech_id", tech_id );
                Json_node_add_string ( MSGNode, "acronyme", acronyme );
                Json_node_add_string ( MSGNode, "libelle", libelle );
                Json_node_add_string ( MSGNode, "date_create", date_create );
                Json_node_add_bool   ( MSGNode, "alive", TRUE );
                MQTT_Send_to_API ( MSGNode, "DLS_HISTO" );
                Json_node_unref ( MSGNode );
              }
             else Info_new( __func__, Config.log_msrv, LOG_ERR, "Cannot send DLS_HISTO: memory error" );
/*---------------------------------------------------- Envoi IMSG ------------------------------------------------------------*/
             gint notif_chat = Json_get_int ( msg->source_node, "notif_chat" );
             if (notif_chat == TXT_NOTIF_BY_DLS) { notif_chat = Json_get_int ( msg->source_node, "notif_chat_by_dls" ); }
             if (notif_chat == TXT_NOTIF_YES)
              { JsonNode *IMSGNode = Json_node_create();
                if (IMSGNode)
                 { Json_node_add_string ( IMSGNode, "tech_id", tech_id );
                   Json_node_add_string ( IMSGNode, "acronyme", acronyme );
                   Json_node_add_string ( IMSGNode, "dls_shortname", dls_shortname );
                   Json_node_add_string ( IMSGNode, "libelle", libelle );
                   MQTT_Send_to_topic ( Partage->MQTT_local_session, IMSGNode, FALSE, "SEND_IMSG" );
                   Json_node_unref ( IMSGNode );
                 }
                else Info_new( __func__, Config.log_msrv, LOG_ERR, "Cannot send SMS: memory error" );
              }
/*---------------------------------------------------- Envoi SMS -------------------------------------------------------------*/
             gint notif_sms = Json_get_int ( msg->source_node, "notif_sms" );
             if (notif_sms == TXT_NOTIF_BY_DLS) { notif_sms = Json_get_int ( msg->source_node, "notif_sms_by_dls" ); }
             if (notif_sms == TXT_NOTIF_YES || notif_sms == TXT_NOTIF_OVH_ONLY)
              { JsonNode *SMSNode = Json_node_create();
                if (SMSNode)
                 { Json_node_add_string ( SMSNode, "tech_id", tech_id );
                   Json_node_add_string ( SMSNode, "acronyme", acronyme );
                   Json_node_add_string ( SMSNode, "dls_shortname", dls_shortname );
                   Json_node_add_string ( SMSNode, "libelle", libelle );
                   Json_node_add_int    ( SMSNode, "notif_sms", notif_sms );
                   MQTT_Send_to_topic ( Partage->MQTT_local_session, SMSNode, FALSE, "SEND_SMS" );
                   Json_node_unref ( SMSNode );
                 }
                else Info_new( __func__, Config.log_msrv, LOG_ERR, "Cannot send SMS: memory error" );
              }
/*---------------------------------------------------- Envoi AUDIO -----------------------------------------------------------*/
             gchar *audio_zone_name = Json_get_string ( msg->source_node, "audio_zone_name" );
             if (strcasecmp ( audio_zone_name, "ZD_NONE"))
              { gchar *audio_libelle = Json_get_string ( msg->source_node, "audio_libelle" );
                if (strlen(audio_libelle)) AUDIO_Send_to_zone ( audio_zone_name, audio_libelle );
              }
           }
          else
           { Info_new( __func__, Config.log_msrv, LOG_WARNING, "Rate limit (=%d) for '%s:%s' reached: not sending",
                       rate_limit, msg->tech_id, msg->acronyme );
           }
        }
       else if (event->etat == 0)
        { JsonNode *histo = MSGS_Convert_msg_off_to_histo ( msg );
          if(histo)
           { MQTT_Send_to_API ( histo, "DLS_HISTO" );
             Json_node_unref ( histo );
           } else Info_new( __func__, Config.log_msrv, LOG_ERR, "Error when convert '%s:%s' from msg off to histo",
                            msg->tech_id, msg->acronyme );
        }
       g_free(event);
       cpt++;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
