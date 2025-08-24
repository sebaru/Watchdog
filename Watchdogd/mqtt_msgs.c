/******************************************************************************************************************************/
/* Watchdogd/mqtt_MSGxxx.c        Distribution des messages DLS à l'API                                                       */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                    mar. 14 août 2012 19:05:42 CEST */
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

/************************************* Converstion du histo dynamique *******************************************************/
 void Convert_libelle_dynamique ( gchar *libelle, gint taille_max )
  { gchar prefixe[128], tech_id[32], acronyme[64], suffixe[128];

encore:
    memset ( prefixe,  0, sizeof(prefixe)  );                       /* Mise à zero pour gérer correctement les fins de tampon */
    memset ( suffixe,  0, sizeof(suffixe)  );
    memset ( tech_id,  0, sizeof(tech_id)  );
    memset ( acronyme, 0, sizeof(acronyme) );

    sscanf ( libelle, "%128[^$]$%32[^:]:%64[a-zA-Z0-9_]%128[^\n]", prefixe, tech_id, acronyme, suffixe );

    if (prefixe[0] == '\0')                                                        /* si pas de prefixe, on retente en direct */
     { sscanf ( libelle, "$%32[^:]:%64[a-zA-Z0-9_]%128[^\n]", tech_id, acronyme, suffixe ); }

    if (tech_id[0] != '\0' && acronyme[0] != '\0')                               /* Si on a trouvé un couple tech_id:acronyme */
     { struct DLS_REGISTRE *reg;
       struct DLS_AI *ai;
       gchar result[128], chaine[32];
       g_snprintf( result, sizeof(result), "%s", prefixe );                                                       /* Prologue */
       if ( (ai = Dls_data_lookup_AI ( tech_id, acronyme )) != NULL )
        { /*if (ai->val_ech-roundf(ai->val_ech) == 0.0)
           { g_snprintf( chaine, sizeof(chaine), "%.0f %s", ai->val_ech, ai->unite ); }
          else*/
           { g_snprintf( chaine, sizeof(chaine), "%.2f %s", ai->valeur, ai->unite ); }
        }
       else if ( (reg = Dls_data_lookup_REGISTRE ( tech_id, acronyme )) != NULL )
        { g_snprintf( chaine, sizeof(chaine), "%.02f %s", reg->valeur, reg->unite ); }
       else g_snprintf( chaine, sizeof(chaine), "erreur" );
       g_strlcat ( result, chaine, sizeof(result) );
       g_strlcat ( result, suffixe, sizeof(result) );
       g_snprintf( libelle, taille_max, "%s", result );
       goto encore;
     }
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Message parsé final: %s", libelle );
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void MSGS_Convert_msg_on_to_histo ( struct DLS_MESSAGE *msg )
  { gchar libelle[128], chaine[512], date_create[128];
    struct timeval tv;
    struct tm *temps;

    gettimeofday( &tv, NULL );
    temps = localtime( (time_t *)&tv.tv_sec );
    strftime( chaine, sizeof(chaine), "%F %T", temps );
    gchar *date_utf8 = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
    g_snprintf( date_create, sizeof(date_create), "%s.%02d", date_utf8, (gint)tv.tv_usec/10000 );
    g_free( date_utf8 );

    g_snprintf ( libelle, sizeof(libelle), "%s", Json_get_string(msg->source_node, "libelle_src") );
    Convert_libelle_dynamique ( libelle, sizeof(libelle) );
/***************************************** Création de la structure interne de stockage ***************************************/
    Json_node_add_string ( msg->source_node, "libelle", libelle );
    Json_node_add_string ( msg->source_node, "date_create", date_create );
    Json_node_add_bool   ( msg->source_node, "alive", TRUE );
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static JsonNode *MSGS_Convert_msg_off_to_histo ( struct DLS_MESSAGE *msg )
  { gchar chaine[256], date_fin[128];
    struct timeval tv;
    struct tm *temps;

    gettimeofday( &tv, NULL );
    temps = localtime( (time_t *)&tv.tv_sec );
    strftime( chaine, sizeof(chaine), "%F %T", temps );
    gchar *date_utf8 = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
    g_snprintf( date_fin, sizeof(date_fin), "%s.%02d", date_utf8, (gint)tv.tv_usec/10000 );
    g_free( date_utf8 );

    JsonNode *histo = Json_node_create ();
    if (histo)
     { Json_node_add_string ( histo, "tech_id",  msg->tech_id );
       Json_node_add_string ( histo, "acronyme", msg->acronyme );
       Json_node_add_string ( histo, "date_fin", date_fin );
       Json_node_add_bool   ( histo, "alive", FALSE );
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
       Partage->Liste_msg = g_slist_remove ( Partage->Liste_msg, event );
       pthread_rwlock_unlock( &Partage->Liste_msg_synchro );                          /* Ajout dans la liste de msg a traiter */
       Info_new( __func__, Config.log_msrv, LOG_INFO,
                "Handle MSG'%s:%s'=%d, Reste a %d a traiter",
                 event->msg->tech_id, event->msg->acronyme, event->etat, g_slist_length(Partage->Liste_msg) );

       if (event->etat == 1)
        { MSGS_Convert_msg_on_to_histo ( event->msg );
          gint rate_limit = Json_get_int ( event->msg->source_node, "rate_limit" );
          if ( !event->msg->last_on || (Partage->top >= event->msg->last_on + rate_limit*10 ) )
           { event->msg->last_on = Partage->top;
             MQTT_Send_to_API ( event->msg->source_node, "DLS_HISTO" );
             MQTT_Send_to_topic ( Partage->MQTT_local_session, "threads", "DLS_HISTO", event->msg->source_node );
             if ( Json_has_member ( event->msg->source_node, "audio_libelle" ) &&
                  strlen(Json_get_string ( event->msg->source_node, "audio_libelle" )) &&
                  strcasecmp ( Json_get_string ( event->msg->source_node, "audio_zone_name" ), "ZD_NONE" )
                )
              { gchar *audio_zone_name = Json_get_string ( event->msg->source_node, "audio_zone_name" );
                gchar *audio_libelle   = Json_get_string ( event->msg->source_node, "audio_libelle" );
                AUDIO_Send_to_zone ( audio_zone_name, audio_libelle );
              }
           }
          else
           { Info_new( __func__, Config.log_msrv, LOG_WARNING, "Rate limit (=%d) for '%s:%s' reached: not sending",
                       rate_limit, event->msg->tech_id, event->msg->acronyme );
           }
        }
       else if (event->etat == 0)
        { JsonNode *histo = MSGS_Convert_msg_off_to_histo ( event->msg );
          if(histo)
           { MQTT_Send_to_API ( histo, "DLS_HISTO" );
             MQTT_Send_to_topic ( Partage->MQTT_local_session, "threads", "DLS_HISTO", histo );
             Json_node_unref ( histo );
           } else Info_new( __func__, Config.log_msrv, LOG_ERR, "Error when convert '%s:%s' from msg off to histo",
                            event->msg->tech_id, event->msg->acronyme );
        }
       g_free(event);
       cpt++;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
