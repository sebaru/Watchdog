/******************************************************************************************************************************/
/* Watchdogd/api_MSGxxx.c        Distribution des messages DLS à l'API                                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        mar. 14 août 2012 19:05:42 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * api_MSGxxx.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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

 static GSList *Liste_Histo_to_send = NULL;

/************************************* Converstion du histo dynamique *******************************************************/
 void Convert_libelle_dynamique ( gchar *local_tech_id, gchar *libelle, gint taille_max )
  { gchar chaine[512], prefixe[128], tech_id[32], acronyme[64], suffixe[128];
    memset ( suffixe, 0, sizeof(suffixe) );
    while ( sscanf ( libelle, "%128[^$]$%32[^:]:%64[a-zA-Z0-9_]%128[^\n]", prefixe, tech_id, acronyme, suffixe ) == 4 )
     { gchar result[128];
       g_snprintf( result, sizeof(result), "%s", prefixe );                                                       /* Prologue */
       struct DLS_AI *ai = Dls_data_lookup_AI ( tech_id, acronyme );
       if (ai)
        { /*if (ai->val_ech-roundf(ai->val_ech) == 0.0)
           { g_snprintf( chaine, sizeof(chaine), "%.0f %s", ai->val_ech, ai->unite ); }
          else*/
           { g_snprintf( chaine, sizeof(chaine), "%.2f %s", ai->valeur, ai->unite ); }
        }
       else g_snprintf( chaine, sizeof(chaine), "erreur" );
       g_strlcat ( result, chaine, sizeof(result) );
       g_strlcat ( result, suffixe, sizeof(result) );
       g_snprintf( libelle, taille_max, "%s", result );
       memset ( suffixe, 0, sizeof(suffixe) );
/*               g_snprintf( chaine, sizeof(chaine), "%d %s", ci->valeur, ci->unite ); /* Row1 = unite */
/*               g_snprintf( chaine, sizeof(chaine), "%d heure et %d minute", tm.tm_hour, tm.tm_min );*/
/*            break;
       case MNEMO_REGISTRE:
             { Dls_data_get_REGISTRE ( tech_id, acronyme, dlsdata_p );
               struct DLS_REGISTRE *reg = *dlsdata_p;
               if (reg)
                { if (reg->valeur-roundf(reg->valeur) == 0.0)
                   { g_snprintf( chaine, sizeof(chaine), "%.0f %s", reg->valeur, reg->unite ); }
                  else
                   { g_snprintf( chaine, sizeof(chaine), "%.2f %s", reg->valeur, reg->unite ); }
                }
               else g_snprintf( chaine, sizeof(chaine), "erreur" );
             }
            break;
       default: return(NULL);
     }
    * */
     }
    while ( sscanf ( libelle, "$THIS%128[^\n]", suffixe ) == 1 )
     { gchar result[128];
       g_snprintf( result, sizeof(result), "%s", local_tech_id );                                                 /* Prologue */
       g_strlcat ( result, suffixe, sizeof(result) );
       g_snprintf( libelle, taille_max, "%s", result );
       memset ( suffixe, 0, sizeof(suffixe) );
     }
    while ( sscanf ( libelle, "%128[^$]$THIS%128[^\n]", prefixe, suffixe ) == 2 )
     { gchar result[128];
       g_snprintf( result, sizeof(result), "%s", prefixe );                                                       /* Prologue */
       g_strlcat ( result, local_tech_id, sizeof(result) );
       g_strlcat ( result, suffixe, sizeof(result) );
       g_snprintf( libelle, taille_max, "%s", result );
       memset ( suffixe, 0, sizeof(suffixe) );
     }
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Message parsé final: %s", libelle );
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static JsonNode *MSGS_Convert_msg_on_to_histo ( struct DLS_MESSAGE *msg )
  { gchar libelle[128], chaine[512], date_create[128];
    struct timeval tv;
    struct tm *temps;

    JsonNode *histo = Http_Get_from_global_API ( "/run/message", "tech_id=%s&acronyme=%s", msg->tech_id, msg->acronyme );
    if (histo == NULL || Json_get_int ( histo, "api_status" ) != SOUP_STATUS_OK)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "API Request for /run/message '%s:%s' failed. Dropping message.",
                 msg->tech_id, msg->acronyme );
       Json_node_unref ( histo );
       return(NULL);
     }

    gettimeofday( &tv, NULL );
    temps = localtime( (time_t *)&tv.tv_sec );
    strftime( chaine, sizeof(chaine), "%F %T", temps );
    gchar *date_utf8 = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
    g_snprintf( date_create, sizeof(date_create), "%s.%02d", date_utf8, (gint)tv.tv_usec/10000 );
    g_free( date_utf8 );

    g_snprintf ( libelle, sizeof(libelle), "%s", Json_get_string(histo, "libelle") );
    Convert_libelle_dynamique ( msg->tech_id, libelle, sizeof(libelle) );
/***************************************** Création de la structure interne de stockage ***************************************/
    Json_node_add_string ( histo, "libelle", libelle );                                       /* Ecrasement libelle d'origine */
    Json_node_add_string ( histo, "date_create", date_create );                               /* Ecrasement libelle d'origine */
    Json_node_add_bool   ( histo, "alive", TRUE );
    return(histo);
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
/* API_Send_MSGS: Envoi les messages a l'API                                                                                  */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void API_Send_MSGS ( void )
  { struct DLS_MESSAGE_EVENT *event;

    while (Partage->com_msrv.liste_msg)
     { pthread_mutex_lock( &Partage->com_msrv.synchro );                              /* Ajout dans la liste de msg a traiter */
       event = Partage->com_msrv.liste_msg->data;                                            /* Recuperation du numero de msg */
       Partage->com_msrv.liste_msg = g_slist_remove ( Partage->com_msrv.liste_msg, event );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       Info_new( __func__, Config.log_msrv, LOG_INFO,
                "Handle MSG'%s:%s'=%d, Reste a %d a traiter",
                 event->msg->tech_id, event->msg->acronyme, event->etat, g_slist_length(Partage->com_msrv.liste_msg) );

       if (event->etat == 1)
        { JsonNode *histo = MSGS_Convert_msg_on_to_histo ( event->msg );
          if (histo)
           { if (Partage->top >= event->msg->last_on + Json_get_int ( histo, "rate_limit" )*10 )
              { event->msg->last_on = Partage->top;
                Json_node_add_string ( histo, "tag", "DLS_HISTO" );
                Http_Send_to_slaves ( NULL, histo );
                Json_node_add_string ( histo, "zmq_tag", "DLS_HISTO" );
                Http_ws_send_to_all( histo );
                Liste_Histo_to_send = g_slist_append ( Liste_Histo_to_send, histo );
              }
             else
              { Info_new( __func__, Config.log_msrv, LOG_WARNING, "Rate limit (=%d) for '%s:%s' reached: not sending",
                          Json_get_int ( histo, "rate_limit" ), event->msg->tech_id, event->msg->acronyme );
                Json_node_unref(histo);
              }
           } else Info_new( __func__, Config.log_msrv, LOG_ERR, "Error when convert '%s:%s' from msg on to histo",
                            event->msg->tech_id, event->msg->acronyme );
        }
       else if (event->etat == 0)
        { JsonNode *histo = MSGS_Convert_msg_off_to_histo ( event->msg );
          if(histo)
           { Json_node_add_string ( histo, "tag", "DLS_HISTO" );
             Http_Send_to_slaves ( NULL, histo );
             Json_node_add_string ( histo, "zmq_tag", "DLS_HISTO" );
             Http_ws_send_to_all( histo );
             Liste_Histo_to_send = g_slist_append ( Liste_Histo_to_send, histo );
           } else Info_new( __func__, Config.log_msrv, LOG_ERR, "Error when convert '%s:%s' from msg off to histo",
                            event->msg->tech_id, event->msg->acronyme );
        }
       g_free(event);
     }

/******************************************************* Envoi à l'API ********************************************************/
    static gint next_try = 0;
    gint cpt=0, top = Partage->top;
    while (Liste_Histo_to_send && next_try <= Partage->top)
     { JsonNode *histo = Liste_Histo_to_send->data;
       Liste_Histo_to_send = g_slist_remove ( Liste_Histo_to_send, histo );
       JsonNode *api_result = Http_Post_to_global_API ( "/run/histo", histo );
       if (api_result == NULL || Json_get_int ( api_result, "api_status" ) != SOUP_STATUS_OK)
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "API Post '%s:%s' for /run/histo failed. Retry %04d MSGS in 60 seconds.",
                    Json_get_string ( histo, "tech_id"), Json_get_string ( histo, "acronyme" ), g_slist_length(Liste_Histo_to_send) );
          Json_node_unref ( api_result );
          Json_node_unref ( histo );
          next_try = Partage->top + 600;
          break;
        } else cpt++;
       Json_node_unref ( api_result );
       Json_node_unref ( histo );
     }
    if (cpt) Info_new( __func__, Config.log_msrv, LOG_INFO, "%d MSGS sent to API in %06.1fs.", cpt, (Partage->top-top)/10.0 );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
