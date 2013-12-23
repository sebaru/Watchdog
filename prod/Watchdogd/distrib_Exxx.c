/**********************************************************************************************************/
/* Watchdogd/distrib_Exxx.c        Distribution des changements des états des entrées                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                    mer. 02 oct. 2013 17:50:13 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * distrib_Exxx.c
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

 static GSList *Liste_clients_E = NULL;

/**********************************************************************************************************/
/* Abonner_distribution_E: Abonnement d'un thread aux diffusions d'une entree                             */
/* Entrée : une fonction permettant de gerer l'arrivée d'un histo                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void Abonner_distribution_entree ( void (*Gerer_E) (gint num) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Liste_clients_E = g_slist_prepend( Liste_clients_E, Gerer_E );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Desabonner_distribution_E: Desabonnement d'un thread aux diffusions d'une entree                       */
/* Entrée : une fonction permettant de gerer l'arrivée d'un histo                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void Desabonner_distribution_entree ( void (*Gerer_E) (gint num) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Liste_clients_E = g_slist_remove( Liste_clients_E, Gerer_E );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Envoyer_message_aux_abonnes: Envoi le message en parametre aux abonnes                                 */
/* Entrée : le message a envoyer                                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Envoyer_E_aux_abonnes ( gint num )
  { GSList *liste;

    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    liste = Liste_clients_E;
    while (liste)                                                              /* Pour chacun des abonnes */
     { void (*Gerer_E) (gint num);
       Gerer_E = liste->data;
       Gerer_E ( num );
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Gerer_arrive_Exxx_dls: Gestion de l'arrive des entrees logiques depuis DLS                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_arrive_Exxx_dls ( void )
  { gint num, reste;

    if (!Partage->com_msrv.liste_e) return;                                 /* Si pas de ea, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );              /* Ajout dans la liste de ea a traiter */
    num = GPOINTER_TO_INT(Partage->com_msrv.liste_e->data);              /* Recuperation du numero de ea */
    Partage->com_msrv.liste_e = g_slist_remove ( Partage->com_msrv.liste_e, GINT_TO_POINTER(num) );
    reste = g_slist_length(Partage->com_msrv.liste_e);
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
              "Gerer_arrive_Exxx_dls: Recu E(%03d)=%d. Reste a traiter %03d",
              num, E(num), reste
            );
    Envoyer_E_aux_abonnes ( num );
  }
/*--------------------------------------------------------------------------------------------------------*/
