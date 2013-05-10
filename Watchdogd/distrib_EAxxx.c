/**********************************************************************************************************/
/* Watchdogd/distrib_EAxxx.c        Distribution des changements d'etats motif                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 07 mai 2013 13:09:39 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * distrib_EAxxx.c
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

 static GSList *Liste_clients_EANA = NULL;

/**********************************************************************************************************/
/* Abonner_distribution_EANA: Abonnement d'un thread aux diffusions d'une entree ANA                      */
/* Entrée : une fonction permettant de gerer l'arrivée d'un histo                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void Abonner_distribution_entreeANA ( void (*Gerer_EANA) (gint num) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Liste_clients_EANA = g_slist_prepend( Liste_clients_EANA, Gerer_EANA );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Desabonner_distribution_EANA: Desabonnement d'un thread aux diffusions d'une entree ANA                */
/* Entrée : une fonction permettant de gerer l'arrivée d'un histo                                         */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void Desabonner_distribution_entreeANA ( void (*Gerer_EANA) (gint num) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Liste_clients_EANA = g_slist_remove( Liste_clients_EANA, Gerer_EANA );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Envoyer_message_aux_abonnes: Envoi le message en parametre aux abonnes                                 */
/* Entrée : le message a envoyer                                                                          */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Envoyer_EANA_aux_abonnes ( gint num )
  { GSList *liste;

    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    liste = Liste_clients_EANA;
    while (liste)                                                              /* Pour chacun des abonnes */
     { void (*Gerer_EANA) (gint num);
       Gerer_EANA = liste->data;
       Gerer_EANA ( num );
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Gerer_arrive_EAxxx_dls: Gestion de l'arrive des entrees ANAlogiques depuis DLS                         */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_arrive_EAxxx_dls ( void )
  { gint num, reste;

    if (!Partage->com_msrv.liste_ea) return;                                 /* Si pas de ea, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );              /* Ajout dans la liste de ea a traiter */
    num = GPOINTER_TO_INT(Partage->com_msrv.liste_ea->data);              /* Recuperation du numero de ea */
    reste = g_slist_length(Partage->com_msrv.liste_ea) - 1;
    Partage->com_msrv.liste_ea = g_slist_remove ( Partage->com_msrv.liste_ea, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
              "Gerer_arrive_EAxxx_dls: Recu EA(%03d)=%d(int). Reste a traiter %03d",
              num, Partage->ea[num].val_avant_ech, reste
            );
    Envoyer_EANA_aux_abonnes ( num );
  }
/*--------------------------------------------------------------------------------------------------------*/
