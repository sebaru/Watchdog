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
  { struct CMD_TYPE_HISTO *histo;
    GSList *liste;

    pthread_mutex_lock( &Partage->com_msrv.synchro );
    liste = Partage->com_msrv.liste_msg_repeat;
    while (liste)
     { struct MESSAGES_EVENT *event;
       histo = (struct CMD_TYPE_HISTO *)liste->data;                                         /* Recuperation du numero de msg */

       if (Partage->g[histo->msg.num].next_repeat <= Partage->top)
        { event = (struct MESSAGES_EVENT *)g_try_malloc0( sizeof ( struct MESSAGES_EVENT ) );
          if (event)
           { event->num  = histo->msg.num;
             event->etat = 0;
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
           }

          event = (struct MESSAGES_EVENT *)g_try_malloc0( sizeof ( struct MESSAGES_EVENT ) );
          if (event)
           { event->num  = histo->msg.num;
             event->etat = 1;
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }
          Partage->g[histo->msg.num].next_repeat = Partage->top + histo->msg.time_repeat*600;                   /* En minutes */
        }
       liste = liste->next;
     }
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Gerer_arrive_MSGxxx_dls_on ( gint num )
  { struct CMD_TYPE_MESSAGE *msg;
    struct CMD_TYPE_HISTO histo;
    struct timeval tv;

    msg = Rechercher_messageDB( num );
    if (!msg)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Message %03d not found", __func__, num );
       return;                                                            /* On n'a pas trouvé le message, alors on s'en va ! */
     }
    memset( &histo, 0, sizeof(histo) );
    memcpy( &histo.msg, msg, sizeof(struct CMD_TYPE_MESSAGE) );                                           /* Ajout dans la DB */
    g_free( msg );                                                                     /* On a plus besoin de cette reference */

    if (!histo.msg.enable)                                                       /* Distribution du message aux sous serveurs */
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Message %03d not enabled !", __func__, num );
       return;
     }
/***************************************** Création de la structure interne de stockage ***************************************/
    gettimeofday( &tv, NULL );
    histo.alive            = TRUE;
    histo.date_create_sec  = tv.tv_sec;
    histo.date_create_usec = tv.tv_usec;
    g_snprintf( histo.nom_ack, sizeof(histo.nom_ack), "None" );
    Ajouter_histo_msgsDB( &histo );                                                                    /* Si ajout dans DB OK */

/******************************************************* Envoi du message aux librairies abonnées *****************************/
    Send_zmq ( Partage->com_msrv.zmq_msg, &histo, sizeof(struct CMD_TYPE_HISTO) );
    Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, TAG_ZMQ_TO_HISTO, NULL, NULL, &histo, sizeof(struct CMD_TYPE_HISTO) );
/************************************************** Gestion des repeat ********************************************************/
    if (histo.msg.time_repeat) 
     { struct CMD_TYPE_HISTO *dup_histo;
       GSList *liste;
       pthread_mutex_lock( &Partage->com_msrv.synchro );                                 /* Vérification de la liste actuelle */
       liste = Partage->com_msrv.liste_msg_repeat;
       while (liste)
        { dup_histo = (struct CMD_TYPE_HISTO *)liste->data;                                  /* Recuperation du numero de msg */
          if (dup_histo->msg.num == histo.msg.num) break;
          liste = liste->next;
        }
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       if (liste) return;                                                         /* Si deja dans la liste, on ne fait rien ! */

       dup_histo = (struct CMD_TYPE_HISTO *)g_try_malloc0(sizeof(struct CMD_TYPE_HISTO));
       if (dup_histo)
        { memcpy ( dup_histo, &histo, sizeof(struct CMD_TYPE_HISTO) );

          Partage->g[histo.msg.num].next_repeat = Partage->top + histo.msg.time_repeat*600;                     /* En minutes */
          pthread_mutex_lock( &Partage->com_msrv.synchro );
          Partage->com_msrv.liste_msg_repeat = g_slist_prepend ( Partage->com_msrv.liste_msg_repeat, dup_histo );
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
        }
     }
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Gerer_arrive_MSG_event_dls_on ( struct MESSAGES *msg )
  { struct CMD_TYPE_MNEMO_FULL *message;
    struct CMD_TYPE_HISTO histo;
    struct timeval tv;

    message = Rechercher_mnemo_fullDB_by_acronyme ( msg->tech_id, msg->acronyme );
    if (!message) return;

    memset ( &histo, 0, sizeof(struct CMD_TYPE_HISTO) );
    g_snprintf( histo.msg.libelle, sizeof(histo.msg.libelle),"%s", message->mnemo_base.libelle );
    g_snprintf( histo.msg.syn_page, sizeof(histo.msg.syn_page), "%s", message->mnemo_base.syn_page );
    g_snprintf( histo.msg.syn_parent_page, sizeof(histo.msg.syn_parent_page), "%s", message->mnemo_base.syn_parent_page );
    g_snprintf( histo.msg.dls_shortname, sizeof(histo.msg.dls_shortname), "%s", message->mnemo_base.dls_shortname );
    histo.msg.id = message->mnemo_msg.id;
    histo.msg.num = 0;
    histo.msg.enable = message->mnemo_msg.enable;
    histo.msg.type = message->mnemo_msg.type;
    histo.msg.syn_id = message->mnemo_msg.syn_id;

    g_free( message );                                                                 /* On a plus besoin de cette reference */

    if (!histo.msg.enable)                                                       /* Distribution du message aux sous serveurs */
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

/******************************************************* Envoi du message aux librairies abonnées *****************************/
    Send_zmq ( Partage->com_msrv.zmq_msg, &histo, sizeof(struct CMD_TYPE_HISTO) );
    Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, TAG_ZMQ_TO_HISTO, NULL, NULL, &histo, sizeof(struct CMD_TYPE_HISTO) );
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Gerer_arrive_MSGxxx_dls_off ( gint num )
  { struct CMD_TYPE_HISTO histo;
    GSList *liste;

    memset ( &histo, 0, sizeof(struct CMD_TYPE_HISTO) );
    histo.alive   = FALSE;
    histo.msg.num = num;
    time ( (time_t *)&histo.date_fin );

    pthread_mutex_lock( &Partage->com_msrv.synchro );                           /* Retrait de la liste des messages en REPEAT */
    liste = Partage->com_msrv.liste_msg_repeat;
    while (liste)
     { struct CMD_TYPE_HISTO *del_histo;
       del_histo = (struct CMD_TYPE_HISTO *)liste->data;                                     /* Recuperation du numero de msg */

       if (del_histo->msg.num == num)
        { Partage->com_msrv.liste_msg_repeat = g_slist_remove ( Partage->com_msrv.liste_msg_repeat, del_histo );
          g_free(del_histo);
          break;
        }
       liste = liste->next;
     }
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Modifier_histo_msgsDB ( &histo );
    Send_zmq ( Partage->com_msrv.zmq_msg, &histo, sizeof(struct CMD_TYPE_HISTO) );
    Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, TAG_ZMQ_TO_HISTO, NULL, NULL, &histo, sizeof(struct CMD_TYPE_HISTO) );
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Gerer_arrive_MSG_event_dls_off ( struct MESSAGES *msg )
  { struct CMD_TYPE_MNEMO_FULL *message;
    struct CMD_TYPE_HISTO histo;
    GSList *liste;

    message = Rechercher_mnemo_fullDB_by_acronyme ( msg->tech_id, msg->acronyme );
    if (!message) return;

    memset ( &histo, 0, sizeof(struct CMD_TYPE_HISTO) );
    histo.alive  = FALSE;
    histo.msg.num = 0;
    histo.msg.id = message->mnemo_msg.id;
    time ( (time_t *)&histo.date_fin );

    Modifier_histo_msgsDB ( &histo );
    Send_zmq ( Partage->com_msrv.zmq_msg, &histo, sizeof(struct CMD_TYPE_HISTO) );
    Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, TAG_ZMQ_TO_HISTO, NULL, NULL, &histo, sizeof(struct CMD_TYPE_HISTO) );
    g_free(message);
  }
/******************************************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                                      */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Gerer_arrive_MSGxxx_dls ( void )
  { struct MESSAGES_EVENT *event;

    while (Partage->com_msrv.liste_msg)
     { pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       event = Partage->com_msrv.liste_msg->data;                        /* Recuperation du numero de msg */
       Partage->com_msrv.liste_msg = g_slist_remove ( Partage->com_msrv.liste_msg, event );
       Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: Handle MSG%03d=%d, Reste a %d a traiter", __func__,
                 event->num, event->etat, g_slist_length(Partage->com_msrv.liste_msg) );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );

            if (event->etat == 0) Gerer_arrive_MSGxxx_dls_off( event->num );
       else if (event->etat == 1) Gerer_arrive_MSGxxx_dls_on ( event->num );
       g_free(event);
     }

    while (Partage->com_msrv.liste_event_msg)
     { struct MESSAGES *msg;
       pthread_mutex_lock( &Partage->com_msrv.synchro );                              /* Ajout dans la liste de msg a traiter */
       msg = Partage->com_msrv.liste_event_msg->data;                                        /* Recuperation du numero de msg */
       Partage->com_msrv.liste_event_msg = g_slist_remove ( Partage->com_msrv.liste_event_msg, msg );
       Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: Handle MSG %s:%s=%d, Reste a %d a traiter", __func__,
                 msg->tech_id, msg->acronyme, msg->etat, g_slist_length(Partage->com_msrv.liste_event_msg) );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );

            if (msg->etat == 0) Gerer_arrive_MSG_event_dls_off( msg );
       else if (msg->etat == 1) Gerer_arrive_MSG_event_dls_on ( msg );
       g_free(event);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
