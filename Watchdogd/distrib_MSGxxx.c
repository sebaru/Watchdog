/******************************************************************************************************************************/
/* Watchdogd/distrib.c        Distribution des messages DLS aux clients                                                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        mar. 14 août 2012 19:05:42 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * distrib_MSGxxx.c
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

/************************************* Converstion du histo dynamique *******************************************************/
 void Convert_libelle_dynamique ( gchar *local_tech_id, gchar *libelle, gint taille_max )
  { gchar chaine[512], prefixe[128], tech_id[32], acronyme[64], suffixe[128];
    memset ( suffixe, 0, sizeof(suffixe) );
    while ( sscanf ( libelle, "%128[^$]$%32[^:]:%64[a-zA-Z0-9_]%128[^\n]", prefixe, tech_id, acronyme, suffixe ) == 4 )
     { gchar result[128];
       gpointer dls_data_p = NULL;
       g_snprintf( result, sizeof(result), "%s", prefixe );                                                       /* Prologue */
       gint type = Rechercher_DICO_type ( tech_id, acronyme );
       if (type == MNEMO_ENTREE_ANA)
        { Dls_data_get_AI ( tech_id, acronyme, &dls_data_p );
          struct DLS_AI *ai = dls_data_p;
          if (ai)
           { /*if (ai->val_ech-roundf(ai->val_ech) == 0.0)
              { g_snprintf( chaine, sizeof(chaine), "%.0f %s", ai->val_ech, ai->unite ); }
             else*/
              { g_snprintf( chaine, sizeof(chaine), "%.2f %s", ai->valeur, ai->unite ); }
           }
          else g_snprintf( chaine, sizeof(chaine), "erreur" );
          g_strlcat ( result, chaine, sizeof(result) );
        }
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
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Message parsé final: %s", __func__, libelle );
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Gerer_arrive_MSG_event_dls_on ( struct DLS_MESSAGES *msg )
  { gchar libelle[128], chaine[512], date_create[128];
    struct timeval tv;
    struct tm *temps;

    JsonNode *histo = Json_node_create ();
    if (!histo) return;
    SQL_Select_to_json_node ( histo, NULL,
                             "SELECT msgs.*,dls.shortname as dls_shortname, dls.syn_id,"
                             "parent_syn.page as syn_parent_page, syn.page as syn_page "
                             "FROM msgs "
                             "INNER JOIN dls ON msgs.tech_id = dls.tech_id "
                             "INNER JOIN syns as syn ON syn.syn_id = dls.syn_id "
                             "INNER JOIN syns as parent_syn ON parent_syn.syn_id = syn.parent_id "
                             "WHERE msgs.tech_id='%s' AND msgs.acronyme='%s'", msg->tech_id, msg->acronyme            /* Where */
                            );

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
    Ajouter_histo_msgsDB( histo );                                                                     /* Si ajout dans DB OK */
/******************************************************* Envoi du histo aux librairies abonnées *******************************/
    if (Partage->top >= msg->last_on + Json_get_int ( histo, "rate_limit" )*10 )
     { msg->last_on = Partage->top;
       Json_node_add_string ( histo, "zmq_tag", "DLS_HISTO" );
       Zmq_Send_json_node ( Partage->com_msrv.zmq_to_slave, g_get_host_name(), "*", histo );
       Zmq_Send_json_node ( Partage->com_msrv.zmq_to_bus,   g_get_host_name(), "*", histo );
       Http_ws_send_to_all( histo );
     }
    Json_node_unref( histo );                                                          /* On a plus besoin de cette reference */
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Gerer_arrive_MSG_event_dls_off ( struct DLS_MESSAGES *msg )
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
    if (!histo) return;
    SQL_Select_to_json_node ( histo, NULL,
                             "SELECT msgs.*, syn.syn_id as syn_id, syn.page as syn_page "
                             "FROM msgs "
                             "INNER JOIN dls ON msgs.tech_id = dls.tech_id "
                             "INNER JOIN syns as syn ON syn.syn_id = dls.syn_id "
                             "WHERE msgs.tech_id='%s' AND msgs.acronyme='%s'", msg->tech_id, msg->acronyme );

    Json_node_add_string ( histo, "date_fin", date_fin );
    Json_node_add_bool   ( histo, "alive", FALSE );
    Retirer_histo_msgsDB( histo );
/******************************************************* Envoi du histo aux librairies abonnées *******************************/
    Json_node_add_string ( histo, "zmq_tag", "DLS_HISTO" );
    Zmq_Send_json_node ( Partage->com_msrv.zmq_to_slave, g_get_host_name(), "*", histo );
    Zmq_Send_json_node ( Partage->com_msrv.zmq_to_bus,   g_get_host_name(), "*", histo );
    Http_ws_send_to_all( histo );
    Json_node_unref( histo );                                                          /* On a plus besoin de cette reference */
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Gerer_arrive_MSGxxx_dls ( void )
  { struct DLS_MESSAGES_EVENT *event;

    while (Partage->com_msrv.liste_msg)
     { pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       event = Partage->com_msrv.liste_msg->data;                        /* Recuperation du numero de msg */
       Partage->com_msrv.liste_msg = g_slist_remove ( Partage->com_msrv.liste_msg, event );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                "%s: Handle MSG'%s:%s'=%d, Reste a %d a traiter", __func__,
                 event->msg->tech_id, event->msg->acronyme, event->etat, g_slist_length(Partage->com_msrv.liste_msg) );

            if (event->etat == 0) Gerer_arrive_MSG_event_dls_off( event->msg );
       else if (event->etat == 1) Gerer_arrive_MSG_event_dls_on ( event->msg );
       g_free(event);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
