/******************************************************************************************************************************/
/* Watchdogd/distrib_Ixxx.c        Distribution des changements d'etats motif                                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          lun 10 mai 2004 11:31:17 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * distrib_Ixxx.c
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

 #include <glib.h>
 #include <string.h>
 #include <unistd.h>
 #include <time.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Gerer_arrive_Ixxx_dls: Gestion de l'arrive des motifs depuis DLS                                                           */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Gerer_arrive_Ixxx_dls ( void )
  { gint num, reste;

    if (Partage->com_msrv.liste_i)                                                              /* Traitement des I statiques */
     { pthread_mutex_lock( &Partage->com_msrv.synchro );                            /* Récupération depuis la liste a traiter */
       num = GPOINTER_TO_INT(Partage->com_msrv.liste_i->data);                                 /* Recuperation du numero de i */
       Partage->com_msrv.liste_i = g_slist_remove ( Partage->com_msrv.liste_i, GINT_TO_POINTER(num) );
       reste = g_slist_length(Partage->com_msrv.liste_i);
       pthread_mutex_unlock( &Partage->com_msrv.synchro );

       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "%s: Recu I(%03d)=%d, r%03d v%03d, b%03d. Reste a traiter %03d", __func__,
                 num, Partage->i[num].etat,
                 Partage->i[num].rouge, Partage->i[num].vert, Partage->i[num].bleu, reste
               );
       Send_zmq ( Partage->com_msrv.zmq_motif, &num, sizeof(num) );
     }

    if (Partage->com_msrv.liste_new_i)                                                         /* Traitement des I dynamiques */
     { struct DLS_VISUEL *visu;
       pthread_mutex_lock( &Partage->com_msrv.synchro );                            /* Récupération depuis la liste a traiter */
       visu = Partage->com_msrv.liste_new_i->data;                                        /* Recuperation du numero du visuel */
       Partage->com_msrv.liste_new_i = g_slist_remove ( Partage->com_msrv.liste_new_i, visu );
       reste = g_slist_length(Partage->com_msrv.liste_new_i);
       pthread_mutex_unlock( &Partage->com_msrv.synchro );

       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "%s: Recu VISUEL %s:%s mode=%d, color=%s, cligno=%d. Reste a traiter %03d", __func__,
                 visu->tech_id, visu->acronyme, visu->mode, visu->color, visu->cligno, reste
               );
       Send_zmq ( Partage->com_msrv.zmq_motif, visu, sizeof(struct DLS_VISUEL) );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
