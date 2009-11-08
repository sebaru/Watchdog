/**********************************************************************************************************/
/* Watchdogd/distrib.c        Distribution des messages DLS aux clients                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       jeu 22 jan 2009 20:01:50 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * distrib_MSGxxx.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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

 #include <glib.h>
 #include <sys/time.h>
 #include <bonobo/bonobo-i18n.h>
 #include <string.h>
 #include <unistd.h>
 #include <time.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Gerer_arrive_MSGxxx_dls_on ( struct DB *Db_watchdog, gint num )
  { struct timeval tv;
    struct CMD_TYPE_MESSAGE *msg;
    msg = Rechercher_messageDB( Config.log, Db_watchdog, num );
    if (!msg)
     { Info_n( Config.log, DEBUG_INFO,
               "MSRV: Gerer_arrive_message_dls_on: Message non trouvé", num );
       return;                                        /* On n'a pas trouvé le message, alors on s'en va ! */
     }
    else if (msg->not_inhibe)                                /* Distribution du message aux sous serveurs */
     { struct HISTODB histo;

/***************************************** Envoi de SMS/AUDIO le cas echeant ******************************/
       if (msg->sms) Envoyer_sms ( msg->libelle );
       if (msg->num_voc) Ajouter_audio ( msg->num );

/***************************** Création de la structure interne de stockage *******************************/   
       gettimeofday( &tv, NULL );
       Partage->new_histo.date_create_sec  = tv.tv_sec;
       Partage->new_histo.date_create_usec = tv.tv_usec;
       Partage->new_histo.type    = msg->type;
       Partage->new_histo.num_syn = msg->num_syn;

       memcpy( &histo.msg, msg, sizeof(struct CMD_TYPE_MESSAGE) );
       memset( &histo.nom_ack, 0, sizeof(histo.nom_ack) );
       histo.date_create_sec  = Partage->new_histo.date_create_sec;
       histo.date_create_usec = Partage->new_histo.date_create_usec;
       histo.date_fixe = 0;

       if (Ajouter_histoDB( Config.log, Db_watchdog, &histo ))                     /* Si ajout dans DB OK */
        {
/***************************** Création de la structure passée aux clients ********************************/
          Partage->new_histo.id = msg->num;
          memcpy( &Partage->new_histo.libelle, msg->libelle, sizeof(msg->libelle) );
          memcpy( &Partage->new_histo.objet, msg->objet, sizeof(msg->objet) );
        }
     }
    g_free( msg );                                                 /* On a plus besoin de cette reference */
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Gerer_arrive_MSGxxx_dls_off ( struct DB *Db_watchdog, gint num )
  { /* Le virer de la base histoDB */
    struct HISTO_HARDDB histo_hard;
    struct HISTODB *histo;
    Partage->del_histo.id = num;
    histo = Rechercher_histoDB( Config.log, Db_watchdog, num );
    Retirer_histoDB( Config.log, Db_watchdog, &Partage->del_histo );
    if (histo)
     { memcpy( &histo_hard, histo, sizeof(struct HISTODB) );
       time ( &histo_hard.date_fin );
       Ajouter_histo_hardDB( Config.log, Db_watchdog, &histo_hard );
       g_free(histo);
     }
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_arrive_MSGxxx_dls ( struct DB *Db_watchdog )
  { gint i, num, etat;

    if (! (Partage->com_msrv.liste_msg_on  ||
           Partage->com_msrv.liste_msg_off)
       ) return;                                                        /* Si pas de message, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );         /* Ajout dans la liste de msg a traiter */
    if (Partage->com_msrv.liste_msg_off)
     { num = GPOINTER_TO_INT(Partage->com_msrv.liste_msg_off->data); /* Recuperation du numero de msg */
       etat  = FALSE;
       Partage->com_msrv.liste_msg_off = g_list_remove ( Partage->com_msrv.liste_msg_off,
                                                             GINT_TO_POINTER(num) );
       Info_n( Config.log, DEBUG_DLS, "MSRV: Gerer_arrive_message_dls: Reste a traiter OFF",
                                      g_list_length(Partage->com_msrv.liste_msg_off) );
     }
    else
     { num = GPOINTER_TO_INT(Partage->com_msrv.liste_msg_on->data);  /* Recuperation du numero de msg */
       etat  = TRUE;
       Partage->com_msrv.liste_msg_on = g_list_remove ( Partage->com_msrv.liste_msg_on,
                                                            GINT_TO_POINTER(num) );
       Info_n( Config.log, DEBUG_DLS, "MSRV: Gerer_arrive_message_dls: Reste a traiter ON",
                                      g_list_length(Partage->com_msrv.liste_msg_on) );
     }
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Info_n( Config.log, DEBUG_DLS, "MSRV: Gerer_arrive_message_dls: Recu message DLS", num );

    if (etat)                                                         /* Le message est une apparition ?? */
     { Gerer_arrive_MSGxxx_dls_on( Db_watchdog, num );
     }
    else                                                   /* Le message doit disparaitre de l'historique */
     { Gerer_arrive_MSGxxx_dls_off( Db_watchdog, num );
     }

    for (i=0; i<Config.max_serveur; i++)                         /* Pour tous les eventuels fils serveurs */
     { if (Partage->Sous_serveur[i].pid == -1 || 
           Partage->Sous_serveur[i].nb_client == 0)
           continue;                                                               /* Si offline, on swap */
       Partage->Sous_serveur[i].type_info = (etat ? TYPE_INFO_NEW_HISTO : TYPE_INFO_DEL_HISTO);
     }
    for (i=0; i<Config.max_serveur; i++)                              /* Attente traitement info par fils */
     { if (Partage->Sous_serveur[i].pid == -1 || 
           Partage->Sous_serveur[i].nb_client == 0)
           continue;                                                               /* Si offline, on swap */
       while(Partage->Arret<FIN && Partage->Sous_serveur[i].type_info != TYPE_INFO_VIDE) sched_yield();
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
