/******************************************************************************************************************************/
/* Watchdogd/distrib.c        Distribution des messages DLS aux clients                                                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        mar. 14 août 2012 19:05:42 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * distrib_MSGxxx.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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
/* Gerer_message_repeat: Gestion de la répétition des messages                                                                */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Gerer_histo_repeat ( void )
  { struct DLS_MESSAGES *msg;
    GSList *liste;

    pthread_mutex_lock( &Partage->com_msrv.synchro );
    liste = Partage->com_msrv.liste_msg_repeat;
    while (liste)
     { msg = (struct DLS_MESSAGES *)liste->data;                                             /* Recuperation du numero de msg */
       liste = liste->next;

       if (msg->next_repeat <= Partage->top)
        { struct MESSAGES_EVENT *event = g_try_malloc0( sizeof ( struct MESSAGES_EVENT ) );
          if (event)
           { event->msg  = msg;
             event->etat = 0;
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
           }

          event = (struct MESSAGES_EVENT *)g_try_malloc0( sizeof ( struct MESSAGES_EVENT ) );
          if (event)
           { event->msg  = msg;
             event->etat = 1;
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
           }
          Partage->com_msrv.liste_msg_repeat = g_slist_remove ( Partage->com_msrv.liste_msg_repeat, msg );   /* Repeat traité */
        }
     }
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Gerer_arrive_MSG_event_dls_on ( struct DLS_MESSAGES *msg )
  { struct CMD_TYPE_MNEMO_FULL *message;
    struct CMD_TYPE_HISTO histo;
    struct timeval tv;

    message = Rechercher_mnemo_fullDB_by_acronyme ( msg->tech_id, msg->acronyme );
    if (!message) return;

    memset ( &histo, 0, sizeof(struct CMD_TYPE_HISTO) );
    memcpy( &histo.msg, msg, sizeof(struct CMD_TYPE_MNEMO_FULL) );                                        /* Ajout dans la DB */
    g_free( message );                                                                 /* On a plus besoin de cette reference */

    if (!histo.msg.mnemo_msg.enable)                                             /* Distribution du message aux sous serveurs */
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Message %s:%s not enabled !", __func__,
                 msg->tech_id, msg->acronyme);
       return;
     }
/***************************************** Création de la structure interne de stockage ***************************************/
    gettimeofday( &tv, NULL );
    histo.alive            = TRUE;
    histo.date_create_sec  = tv.tv_sec;
    histo.date_create_usec = tv.tv_usec;
    g_snprintf( histo.nom_ack, sizeof(histo.nom_ack), "None" );
    Ajouter_histo_msgsDB( &histo );                                                                    /* Si ajout dans DB OK */

/****************************************** Ajout aux messages repeat si besoin ***********************************************/
    if (histo.msg.mnemo_msg.time_repeat) 
     { msg->next_repeat = Partage->top + histo.msg.mnemo_msg.time_repeat*600;                                   /* En minutes */
       pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Retrait de la liste des messages en REPEAT */
       Partage->com_msrv.liste_msg_repeat = g_slist_prepend ( Partage->com_msrv.liste_msg_repeat, msg );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
     }

/******************************************************* Envoi du message aux librairies abonnées *****************************/
    Send_zmq ( Partage->com_msrv.zmq_msg, &histo, sizeof(struct CMD_TYPE_HISTO) );
    Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, TAG_ZMQ_TO_HISTO, NULL, NULL, &histo, sizeof(struct CMD_TYPE_HISTO) );
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Gerer_arrive_MSG_event_dls_off ( struct DLS_MESSAGES *msg )
  { struct CMD_TYPE_HISTO histo;

    memset ( &histo, 0, sizeof(struct CMD_TYPE_HISTO) );
    histo.alive   = FALSE;
    g_snprintf( histo.msg.mnemo_base.dls_tech_id, sizeof(histo.msg.mnemo_base.dls_tech_id), "%s", msg->tech_id );
    g_snprintf( histo.msg.mnemo_base.acronyme, sizeof(histo.msg.mnemo_base.acronyme), "%s", msg->acronyme );
    time ( (time_t *)&histo.date_fin );

    pthread_mutex_lock( &Partage->com_msrv.synchro );                           /* Retrait de la liste des messages en REPEAT */
    Partage->com_msrv.liste_msg_repeat = g_slist_remove ( Partage->com_msrv.liste_msg_repeat, msg );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Modifier_histo_msgsDB ( &histo );
    Send_zmq ( Partage->com_msrv.zmq_msg, &histo, sizeof(struct CMD_TYPE_HISTO) );
    Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, TAG_ZMQ_TO_HISTO, NULL, NULL, &histo, sizeof(struct CMD_TYPE_HISTO) );
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Gerer_arrive_MSGxxx_dls ( void )
  { while (Partage->com_msrv.liste_msg)
     { struct MESSAGES_EVENT *event;
       pthread_mutex_lock( &Partage->com_msrv.synchro );                         /* Parcours de la liste de message a traiter */
       event = Partage->com_msrv.liste_msg->data;                                              /* Recuperation de l'evenement */
       Partage->com_msrv.liste_msg = g_slist_remove ( Partage->com_msrv.liste_msg, event );
       Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: Handle MSG '%s:%s'=%d, Reste a %d a traiter", __func__,
                 event->msg->tech_id, event->msg->acronyme, event->etat, g_slist_length(Partage->com_msrv.liste_msg) );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );

            if (event->etat == 0) Gerer_arrive_MSG_event_dls_off( event->msg );
       else if (event->etat == 1) Gerer_arrive_MSG_event_dls_on ( event->msg );
       g_free(event);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
