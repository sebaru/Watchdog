/**********************************************************************************************************/
/* Watchdogd/distrib_Ixxx.c        Distribution des changements d'etats motif                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 10 mai 2004 11:31:17 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * distrib_Ixxx.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
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
 void Gerer_arrive_Ixxx_dls ( void )
  { gint i, num;

    if (!Partage->com_msrv.liste_i) return;                               /* Si pas de i, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );         /* Ajout dans la liste de msg a traiter */
    num = GPOINTER_TO_INT(Partage->com_msrv.liste_i->data);            /* Recuperation du numero de i */
    Info_n( Config.log, DEBUG_DLS, "MSRV: Gerer_arrive_Ixxx_dls: Reste a traiter",
                                   g_list_length(Partage->com_msrv.liste_i) );
    Partage->com_msrv.liste_i = g_list_remove ( Partage->com_msrv.liste_i, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Info_n( Config.log, DEBUG_DLS, "MSRV: Gerer_arrive_Ixxx_dls: Recu I DLS", num );
    Info_n( Config.log, DEBUG_DLS, "MSRV: Gerer_arrive_Ixxx_dls:       mode", Partage->i[num].etat );

/***************************** Création de la structure passée aux clients ********************************/
    Partage->new_motif.num    = num;
    Partage->new_motif.etat   = Partage->i[num].etat;
    Partage->new_motif.rouge  = Partage->i[num].rouge;
    Partage->new_motif.vert   = Partage->i[num].vert;
    Partage->new_motif.bleu   = Partage->i[num].bleu;
    Partage->new_motif.cligno = Partage->i[num].cligno;

    for (i=0; i<Config.max_serveur; i++)                         /* Pour tous les eventuels fils serveurs */
     { if (Partage->Sous_serveur[i].pid == -1 || 
           Partage->Sous_serveur[i].nb_client == 0)
           continue;                                                               /* Si offline, on swap */
       Partage->Sous_serveur[i].type_info = TYPE_INFO_NEW_MOTIF;
     }
    for (i=0; i<Config.max_serveur; i++)                              /* Attente traitement info par fils */
     { if (Partage->Sous_serveur[i].pid == -1 || 
           Partage->Sous_serveur[i].nb_client == 0)
           continue;                                                               /* Si offline, on swap */
       while(Partage->Arret<FIN && Partage->Sous_serveur[i].type_info != TYPE_INFO_VIDE)
        { sched_yield(); usleep(1); }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
