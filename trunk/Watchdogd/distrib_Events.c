/******************************************************************************************************************************/
/* Watchdogd/distrib_Events.c        Distribution des changements d'etats motif                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        sam. 24 janv. 2015 13:53:26 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * distrib_Events.c
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
  { JsonNode *RootNode;
    struct DLS_DO *dout;
    struct DLS_AO *ao;
    gint reste;

    if (!Partage->com_msrv.Liste_DO) goto suite_AO;                                               /* Si pas de a, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );                                 /* Ajout dans la liste de msg a traiter */
    dout = (struct DLS_DO *)Partage->com_msrv.Liste_DO->data;                                  /* Recuperation du numero de a */
    Partage->com_msrv.Liste_DO = g_slist_remove ( Partage->com_msrv.Liste_DO, dout );
    reste = g_slist_length(Partage->com_msrv.Liste_DO);
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Sending SET_DO '%s':'%s' to Slave/Bus (reste %d)", __func__,
              dout->tech_id, dout->acronyme, reste );

    RootNode = Json_node_create ();
    if (RootNode)
     { Dls_DO_to_json ( RootNode, dout );
       Zmq_Send_json_node ( Partage->com_msrv.zmq_to_slave, "msrv", "*", "SET_DO", RootNode );
       Zmq_Send_json_node ( Partage->com_msrv.zmq_to_bus,   "msrv", "*", "SET_DO", RootNode );
       json_node_unref ( RootNode );
     }
    else { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ ); }

suite_AO:
    if (!Partage->com_msrv.Liste_AO) return;                                                      /* Si pas de a, on se barre */
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                 /* Ajout dans la liste de msg a traiter */
    ao = (struct DLS_AO *)Partage->com_msrv.Liste_AO->data;                                    /* Recuperation du numero de a */
    Partage->com_msrv.Liste_AO = g_slist_remove ( Partage->com_msrv.Liste_AO, ao );
    reste = g_slist_length(Partage->com_msrv.Liste_AO);
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Sending SET_AO '%s':'%s' = %f to Slave/Bus (reste %d)", __func__,
              ao->tech_id, ao->acronyme, ao->val_ech, reste );

    RootNode = Json_node_create ();
    if (RootNode)
     { Dls_AO_to_json ( RootNode, ao );
       Zmq_Send_json_node ( Partage->com_msrv.zmq_to_slave, "msrv", "*", "SET_AO", RootNode );
       Zmq_Send_json_node ( Partage->com_msrv.zmq_to_bus,   "msrv", "*", "SET_AO", RootNode );
       json_node_unref ( RootNode );
     }
    else { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon RootNode creation failed", __func__ ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
