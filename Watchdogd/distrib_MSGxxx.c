/**********************************************************************************************************/
/* Watchdogd/distrib.c        Distribution des messages DLS aux clients                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                    mar. 14 août 2012 19:05:42 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Abonner_distribution_message: Abonnement d'un thread aux diffusions d'un message                       */
/* Entrée : une fonction permettant de gerer l'arrivée d'un histo                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void Abonner_distribution_message ( void (*Gerer_message) (guint num) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
    Partage->com_msrv.Liste_abonne_msg = g_slist_prepend( Partage->com_msrv.Liste_abonne_msg, Gerer_message );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
  }
/**********************************************************************************************************/
/* Desabonner_distribution_message: Desabonnement d'un thread aux diffusions d'un message                 */
/* Entrée : une fonction permettant de gerer l'arrivée d'un histo                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void Desabonner_distribution_message ( void (*Gerer_message) (guint num) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
    Partage->com_msrv.Liste_abonne_msg = g_slist_remove( Partage->com_msrv.Liste_abonne_msg, Gerer_message );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
  }
/**********************************************************************************************************/
/* Envoyer_message_aux_abonnes: Envoi le message en parametre aux abonnes                                 */
/* Entrée : le message a envoyer                                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Envoyer_message_aux_abonnes ( guint num )
  { GSList *liste;

    pthread_mutex_lock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
    liste = Partage->com_msrv.Liste_abonne_msg;
    while (liste)                                                              /* Pour chacun des abonnes */
     { void (*Gerer_message) (guint num);
       Gerer_message = liste->data;
       Gerer_message ( num );
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
  }
/**********************************************************************************************************/
/* Gerer_message_repeat: Gestion de la répétition des messages                                            */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_message_repeat ( struct DB *Db_watchdog )
  { GSList *liste;
    gint num;

    pthread_mutex_lock( &Partage->com_msrv.synchro );
    liste = Partage->com_msrv.liste_msg_repeat;
    while (liste)
     { num = GPOINTER_TO_INT(liste->data);                               /* Recuperation du numero de msg */

       if (Partage->g[num].next_repeat <= Partage->top)
        { Envoyer_message_aux_abonnes ( num );
#warning To Clean !!
#ifdef bouh
          Partage->g[num].next_repeat = Partage->top + msg->time_repeat*600;                /* En minutes */
#endif
        }
       liste = liste->next;
     }
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Gerer_arrive_MSGxxx_dls_on ( struct DB *Db_watchdog, gint num )
  { struct CMD_TYPE_HISTO *new_histo;
    struct CMD_TYPE_MESSAGE *msg;
    struct HISTODB histo;
    struct timeval tv;

    msg = Rechercher_messageDB( Config.log, Db_watchdog, num );
    if (!msg)
     { Info_new( Config.log, Config.log_all, LOG_INFO, 
                "Gerer_arrive_message_dls_on: Message %03d not found", num );
       return;                                        /* On n'a pas trouvé le message, alors on s'en va ! */
     }
    else if (!msg->enable)                                   /* Distribution du message aux sous serveurs */
     { Info_new( Config.log, Config.log_all, LOG_INFO, 
                "Gerer_arrive_message_dls_on: Message %03d not enabled !", num );
       return;
     }

/***************************************** Envoi de SMS/AUDIO le cas echeant ******************************/
    if (msg->time_repeat) 
     { pthread_mutex_lock( &Partage->com_msrv.synchro );
       Partage->com_msrv.liste_msg_repeat = g_slist_prepend ( Partage->com_msrv.liste_msg_repeat,
                                                              GINT_TO_POINTER(msg->num) );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       Partage->g[num].next_repeat = Partage->top + msg->time_repeat*600;                   /* En minutes */
     }
/***************************** Création de la structure interne de stockage *******************************/   
    gettimeofday( &tv, NULL );

    memcpy( &histo.msg, msg, sizeof(struct CMD_TYPE_MESSAGE) );                       /* Ajout dans la DB */
    memset( &histo.nom_ack, 0, sizeof(histo.nom_ack) );
    histo.date_create_sec  = tv.tv_sec;
    histo.date_create_usec = tv.tv_usec;
    histo.date_fixe = 0;

    Ajouter_histoDB( Config.log, Db_watchdog, &histo );                            /* Si ajout dans DB OK */

/********************************************* Envoi du message aux librairies abonnées *******************/
    Envoyer_message_aux_abonnes ( num );

    g_free( msg );                                                 /* On a plus besoin de cette reference */
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Gerer_arrive_MSGxxx_dls_off ( struct DB *Db_watchdog, gint num )
  { /* Le virer de la base histoDB */
    struct CMD_TYPE_HISTO *del_histo;
    struct HISTO_HARDDB histo_hard;
    struct HISTODB *histo;
    guint i;

    pthread_mutex_lock( &Partage->com_msrv.synchro );       /* Retrait de la liste des messages en REPEAT */
    Partage->com_msrv.liste_msg_repeat = g_slist_remove ( Partage->com_msrv.liste_msg_repeat,
                                                          GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

/********************************************* Envoi du message aux librairies abonnées *******************/
    Envoyer_message_aux_abonnes ( num );

    histo = Rechercher_histoDB( Config.log, Db_watchdog, num );
    if (!histo) return;

    memcpy( &histo_hard, histo, sizeof(struct HISTODB) );
    time ( &histo_hard.date_fin );
    Ajouter_histo_hardDB( Config.log, Db_watchdog, &histo_hard );
    g_free(histo);

    Retirer_histoDB( Config.log, Db_watchdog, num );
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_arrive_MSGxxx_dls ( struct DB *Db_watchdog )
  { gint num, val;

    if (Partage->com_msrv.liste_msg)
     { pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = GPOINTER_TO_INT(Partage->com_msrv.liste_msg->data);         /* Recuperation du numero de msg */
       val = Partage->g[num].etat;
       Partage->com_msrv.liste_msg = g_slist_remove ( Partage->com_msrv.liste_msg,
                                                      GINT_TO_POINTER(num) );
       Info_new( Config.log, Config.log_all, LOG_DEBUG,
                "Gerer_arrive_message_dls: Handle MSG%03d=%d, Reste a %d a traiter",
               num, val, g_slist_length(Partage->com_msrv.liste_msg) );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );

            if (val == 0) Gerer_arrive_MSGxxx_dls_off( Db_watchdog, num );
       else if (val == 1) Gerer_arrive_MSGxxx_dls_on( Db_watchdog, num );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
