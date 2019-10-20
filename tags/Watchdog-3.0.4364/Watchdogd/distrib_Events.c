/******************************************************************************************************************************/
/* Watchdogd/distrib_Events.c        Distribution des changements d'etats motif                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        sam. 24 janv. 2015 13:53:26 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * distrib_Events.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien LEFEVRE
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

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Gerer_arrive_Axxx_dls: Gestion de l'arrive des sorties depuis DLS (Axxx = 1)                                               */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Gerer_arrive_Axxx_dls ( void )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;
    struct DLS_BOOL *bool;
    gint reste;

    if (!Partage->com_msrv.Liste_DO) return;                                                      /* Si pas de a, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );                                 /* Ajout dans la liste de msg a traiter */
    bool = (struct DLS_BOOL *)Partage->com_msrv.Liste_DO->data;                                /* Recuperation du numero de a */
    Partage->com_msrv.Liste_DO = g_slist_remove ( Partage->com_msrv.Liste_DO, bool );
    reste = g_slist_length(Partage->com_msrv.Liste_DO);
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    mnemo = Rechercher_mnemo_baseDB_by_acronyme ( bool->tech_id, bool->acronyme );
    if (!mnemo)
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Mnemo not found for %s:%s", __func__, bool->tech_id, bool->acronyme );
       return;
     }

    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Recu EVENT %s:%s -> %s:%s:%s. Reste a traiter %03d", __func__,
              bool->tech_id, bool->acronyme, mnemo->ev_host, mnemo->ev_thread, mnemo->ev_text, reste );

    if ( strlen ( mnemo->ev_text ) > 0 )                           /* Existe t'il un evenement associé ? (implique furtivité) */
     { Send_zmq_with_tag ( Partage->com_msrv.zmq_to_bus, NULL, "msrv",
                           mnemo->ev_host, mnemo->ev_thread, "dls_event", mnemo->ev_text, strlen(mnemo->ev_text)+1 );
     }
    g_free(mnemo);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
