/**********************************************************************************************************/
/* Watchdogd/distrib_Axxx.c        Distribution des changements d'etats motif                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 09 sept. 2012 12:09:07 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * distrib_Axxx.c
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

 #include <string.h>
 #include <unistd.h>
 #include <time.h>


/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"

 static GSList *Liste_clients_sortie = NULL;

/**********************************************************************************************************/
/* Abonner_distribution_sortie: Abonnement d'un thread aux diffusions d'un sortie                       */
/* Entrée : une fonction permettant de gerer l'arrivée d'un histo                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void Abonner_distribution_sortie ( void (*Gerer_sortie) (gint num) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Liste_clients_sortie = g_slist_prepend( Liste_clients_sortie, Gerer_sortie );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Desabonner_distribution_sortie: Desabonnement d'un thread aux diffusions d'un sortie                 */
/* Entrée : une fonction permettant de gerer l'arrivée d'un histo                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void Desabonner_distribution_sortie ( void (*Gerer_sortie) (gint num) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Liste_clients_sortie = g_slist_remove( Liste_clients_sortie, Gerer_sortie );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Envoyer_message_aux_abonnes: Envoi le message en parametre aux abonnes                                 */
/* Entrée : le message a envoyer                                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Envoyer_sortie_aux_abonnes ( gint num )
  { GSList *liste;

    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    liste = Liste_clients_sortie;
    while (liste)                                                              /* Pour chacun des abonnes */
     { void (*Gerer_sortie) (gint num);
       Gerer_sortie = liste->data;
       Gerer_sortie ( num );
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Gerer_arrive_Axxx_dls: Gestion de l'arrive des sorties depuis DLS                                      */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_arrive_Axxx_dls ( void )
  { gint num, reste;

    if (!Partage->com_msrv.liste_a) return;                                   /* Si pas de a, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );             /* Ajout dans la liste de msg a traiter */
    num = GPOINTER_TO_INT(Partage->com_msrv.liste_a->data);                /* Recuperation du numero de a */
    Partage->com_msrv.liste_a = g_slist_remove ( Partage->com_msrv.liste_a, GINT_TO_POINTER(num) );
    reste = g_slist_length(Partage->com_msrv.liste_a);
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Info_new( Config.log, Config.log_all, LOG_DEBUG,
              "Gerer_arrive_Axxx_dls: Recu A(%03d)=%d. Reste a traiter %03d",
              num, A(num), reste
            );
    Envoyer_sortie_aux_abonnes ( num );
  }
/*--------------------------------------------------------------------------------------------------------*/
