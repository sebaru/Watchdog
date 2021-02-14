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

/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Gerer_arrive_MSG_event_dls_on ( struct DLS_MESSAGES *msg )
  { gchar libelle[128], chaine[512], *date_create;
    gchar prefixe[128], tech_id[32], acronyme[64], suffixe[128];
    struct CMD_TYPE_MESSAGE *message;
    struct CMD_TYPE_HISTO histo;
    struct timeval tv;
    struct tm *temps;

    message = Rechercher_messageDB_par_acronyme ( msg->tech_id, msg->acronyme );
    if (!message) return;
    memset ( &histo, 0, sizeof(struct CMD_TYPE_HISTO) );
    memcpy( &histo.msg, message, sizeof(struct CMD_TYPE_MESSAGE) );                                       /* Ajout dans la DB */

    g_snprintf ( libelle, sizeof(libelle), "%s", message->libelle );
    memset ( suffixe, 0, sizeof(suffixe) );
/************************************* Conversation du lessage dynalique ******************************************************/
    while ( sscanf ( libelle, "%128[^$]$%32[^:]:%64[a-zA-Z0-9_]%128c", prefixe, tech_id, acronyme, suffixe ) == 4 )
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
              { g_snprintf( chaine, sizeof(chaine), "%.2f %s", ai->val_ech, ai->unite ); }
           }
          else g_snprintf( chaine, sizeof(chaine), "erreur" );
          g_strlcat ( result, chaine, sizeof(result) );
        }
       g_strlcat ( result, suffixe, sizeof(result) );
       g_snprintf( libelle, sizeof(libelle), "%s", result );
       memset ( suffixe, 0, sizeof(suffixe) );
/*               g_snprintf( chaine, sizeof(chaine), "%d %s", ci->valeur, ci->unite ); /* Row1 = unite */
/*               g_snprintf( chaine, sizeof(chaine), "%d heure et %d minute", tm.tm_hour, tm.tm_min );*/
/*            break;
       case MNEMO_REGISTRE:
             { Dls_data_get_R ( tech_id, acronyme, dlsdata_p );
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
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Message parsé final: %s", __func__, libelle );
    g_snprintf( histo.msg.libelle, sizeof(histo.msg.libelle), "%s", libelle );                /* Ecrasement libelle d'origine */
/***************************************** Création de la structure interne de stockage ***************************************/
    histo.alive = TRUE;
    gettimeofday( &tv, NULL );
    temps = localtime( (time_t *)&tv.tv_sec );
    strftime( chaine, sizeof(chaine), "%F %T", temps );
    date_create = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
    g_snprintf( histo.date_create, sizeof(histo.date_create), "%s.%02d", date_create, (gint)tv.tv_usec/10000 );
    g_free( date_create );
    g_snprintf( histo.nom_ack, sizeof(histo.nom_ack), "None" );
    Ajouter_histo_msgsDB( &histo );                                                                    /* Si ajout dans DB OK */
/******************************************************* Envoi du message aux librairies abonnées *****************************/
    JsonBuilder *builder = Json_create ();
    g_snprintf( chaine, sizeof(chaine),
               "SELECT msgs.*,dls.shortname as dls_shortname, dls.syn_id,"
               "parent_syn.page as syn_parent_page, syn.page as syn_page "
               "FROM msgs "
               "INNER JOIN dls ON msgs.tech_id = dls.tech_id "
               "INNER JOIN syns as syn ON syn.id = dls.syn_id "
               "INNER JOIN syns as parent_syn ON parent_syn.id = syn.parent_id "
               "WHERE msgs.tech_id='%s' AND msgs.acronyme='%s'", msg->tech_id, msg->acronyme );
    SQL_Select_to_JSON ( builder, NULL, chaine );
    Json_add_bool   ( builder, "alive",            TRUE );
    Json_add_string ( builder, "date_create",      histo.date_create );
    Json_add_string ( builder, "nom_ack",          histo.nom_ack );

    Send_double_zmq_with_json ( Partage->com_msrv.zmq_to_slave, Partage->com_msrv.zmq_to_bus,
                                "msrv", "*", "*","DLS_HISTO", builder );
    Send_zmq_as_raw ( Partage->com_msrv.zmq_msg, &histo, sizeof(struct CMD_TYPE_HISTO) );      /* Pour thread SSRV uniquement */
    g_free( message );                                                                 /* On a plus besoin de cette reference */
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Gerer_arrive_MSG_event_dls_off ( struct DLS_MESSAGES *msg )
  { struct CMD_TYPE_MESSAGE *message;
    gchar chaine[256], *date_fin;
    struct CMD_TYPE_HISTO histo;
    struct timeval tv;
    struct tm *temps;

    message = Rechercher_messageDB_par_acronyme ( msg->tech_id, msg->acronyme );
    if (!message) return;

    memset ( &histo, 0, sizeof(struct CMD_TYPE_HISTO) );
    memcpy( &histo.msg, message, sizeof(struct CMD_TYPE_MESSAGE) );                                       /* Ajout dans la DB */
    JsonBuilder *builder = Json_create ();
    histo.alive  = FALSE;
    gettimeofday( &tv, NULL );
    temps = localtime( (time_t *)&tv.tv_sec );
    strftime( chaine, sizeof(chaine), "%F %T", temps );
    date_fin = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
    g_snprintf( histo.date_fin, sizeof(histo.date_fin), "%s.%02d", date_fin, (gint)tv.tv_usec/10000 );
    g_free( date_fin );
/******************************************************* Envoi du message aux librairies abonnées *****************************/
    g_snprintf( chaine, sizeof(chaine),
               "SELECT msgs.*, syn.id as syn_id, syn.page as syn_page "
               "FROM msgs "
               "INNER JOIN dls ON msgs.tech_id = dls.tech_id "
               "INNER JOIN syns as syn ON syn.id = dls.syn_id "
               "WHERE msgs.tech_id='%s' AND msgs.acronyme='%s'", msg->tech_id, msg->acronyme );
    SQL_Select_to_JSON ( builder, NULL, chaine );
    Json_add_bool   ( builder, "alive", FALSE );
    Json_add_string ( builder, "date_fin", histo.date_fin );

    Modifier_histo_msgsDB ( &histo );
    Send_double_zmq_with_json ( Partage->com_msrv.zmq_to_slave, Partage->com_msrv.zmq_to_bus,
                                "msrv", "*", "*","DLS_HISTO", builder );

    Send_zmq_as_raw ( Partage->com_msrv.zmq_msg, &histo, sizeof(struct CMD_TYPE_HISTO) );
    g_free( message );                                                                 /* On a plus besoin de cette reference */
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
