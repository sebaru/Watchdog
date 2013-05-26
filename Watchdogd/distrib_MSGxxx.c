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
 void Abonner_distribution_message ( void (*Gerer_message) (struct CMD_TYPE_MESSAGE *msg) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
    Partage->com_msrv.Liste_abonne_msg = g_slist_prepend( Partage->com_msrv.Liste_abonne_msg, Gerer_message );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
  }
/**********************************************************************************************************/
/* Desabonner_distribution_message: Desabonnement d'un thread aux diffusions d'un message                 */
/* Entrée : une fonction permettant de gerer l'arrivée d'un histo                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void Desabonner_distribution_message ( void (*Gerer_message) (struct CMD_TYPE_MESSAGE *msg) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
    Partage->com_msrv.Liste_abonne_msg = g_slist_remove( Partage->com_msrv.Liste_abonne_msg, Gerer_message );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
  }
/**********************************************************************************************************/
/* Envoyer_message_aux_abonnes: Envoi le message en parametre aux abonnes                                 */
/* Entrée : le message a envoyer                                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Envoyer_message_aux_abonnes ( struct CMD_TYPE_MESSAGE *msg )
  { GSList *liste;

    pthread_mutex_lock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
    liste = Partage->com_msrv.Liste_abonne_msg;
    while (liste)                                                              /* Pour chacun des abonnes */
     { void (*Gerer_message) (struct CMD_TYPE_MESSAGE *msg);
       struct CMD_TYPE_MESSAGE *dup_msg;
       dup_msg = (struct CMD_TYPE_MESSAGE *)g_try_malloc0(sizeof(struct CMD_TYPE_MESSAGE));
       if (dup_msg)
        { Gerer_message = liste->data;
          memcpy ( dup_msg, msg, sizeof(struct CMD_TYPE_MESSAGE) );
          Gerer_message (dup_msg);
        }
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Partage->com_msrv.synchro_Liste_abonne_msg );
  }
/**********************************************************************************************************/
/* Gerer_message_repeat: Gestion de la répétition des messages                                            */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_message_repeat ( void )
  { struct CMD_TYPE_MESSAGE *msg;
    struct DB *db;
    GSList *liste;
    gint num;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "Gerer_message_repeat: Connexion DB impossible" );
       return;
     }
          
    pthread_mutex_lock( &Partage->com_msrv.synchro );
    liste = Partage->com_msrv.liste_msg_repeat;
    while (liste)
     { num = GPOINTER_TO_INT(liste->data);                               /* Recuperation du numero de msg */

       if (Partage->g[num].next_repeat <= Partage->top)
        { msg = Rechercher_messageDB( Config.log, db, num );
          Envoyer_message_aux_abonnes ( msg );
          Partage->g[num].next_repeat = Partage->top + msg->time_repeat*600;                /* En minutes */
          g_free(msg);
        }
       liste = liste->next;
     }
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    Libere_DB_SQL( Config.log, &db );
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Gerer_arrive_MSGxxx_dls_on ( struct DB *db, gint num )
  { struct CMD_TYPE_MESSAGE *msg;
    struct HISTODB histo;
    struct timeval tv;

    msg = Rechercher_messageDB( Config.log, db, num );
    if (!msg)
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, 
                "Gerer_arrive_message_dls_on: Message %03d not found", num );
       return;                                        /* On n'a pas trouvé le message, alors on s'en va ! */
     }
    else if (!msg->enable)                                   /* Distribution du message aux sous serveurs */
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, 
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

    Ajouter_histoDB( Config.log, db, &histo );                            /* Si ajout dans DB OK */

/********************************************* Envoi du message aux librairies abonnées *******************/
    Envoyer_message_aux_abonnes ( msg );

    g_free( msg );                                                 /* On a plus besoin de cette reference */
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Gerer_arrive_MSGxxx_dls_off ( struct DB *db, gint num )
  { /* Le virer de la base histoDB */
    struct CMD_TYPE_MESSAGE *msg;
    struct HISTO_HARDDB histo_hard;
    struct HISTODB *histo;

    msg = Rechercher_messageDB( Config.log, db, num );
    if (!msg)
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, 
                "Gerer_arrive_message_dls_off: Message %03d not found", num );
       return;                                        /* On n'a pas trouvé le message, alors on s'en va ! */
     }

    pthread_mutex_lock( &Partage->com_msrv.synchro );       /* Retrait de la liste des messages en REPEAT */
    Partage->com_msrv.liste_msg_repeat = g_slist_remove ( Partage->com_msrv.liste_msg_repeat,
                                                          GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

/********************************************* Envoi du message aux librairies abonnées *******************/
    Envoyer_message_aux_abonnes ( msg );
    g_free(msg);

    histo = Rechercher_histoDB( Config.log, db, num );
    if (!histo) return;

    memcpy( &histo_hard, histo, sizeof(struct HISTODB) );
    time ( &histo_hard.date_fin );
    Ajouter_histo_hardDB( Config.log, db, &histo_hard );
    g_free(histo);

    Retirer_histoDB( Config.log, db, num );
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_arrive_MSGxxx_dls ( void )
  { gint num, val;
    struct DB *db;

    if (Partage->com_msrv.liste_msg)
     { pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = GPOINTER_TO_INT(Partage->com_msrv.liste_msg->data);         /* Recuperation du numero de msg */
       val = Partage->g[num].etat;
       Partage->com_msrv.liste_msg = g_slist_remove ( Partage->com_msrv.liste_msg,
                                                      GINT_TO_POINTER(num) );
       Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "Gerer_arrive_message_dls: Handle MSG%03d=%d, Reste a %d a traiter",
               num, val, g_slist_length(Partage->com_msrv.liste_msg) );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );

       db = Init_DB_SQL();       
       if (!db)
        { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                   "Gerer_arrive_MSGxxx_dls: Connexion DB impossible" );
          return;
        }
   
            if (val == 0) Gerer_arrive_MSGxxx_dls_off( db, num );
       else if (val == 1) Gerer_arrive_MSGxxx_dls_on( db, num );
       Libere_DB_SQL( Config.log, &db );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
