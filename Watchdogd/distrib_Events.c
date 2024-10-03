/******************************************************************************************************************************/
/* Watchdogd/distrib_Events.c        Distribution des changements d'etats motif                                               */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                    sam. 24 janv. 2015 13:53:26 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * distrib_Events.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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
    gint cpt;

    cpt=0;
    while ( Partage->com_msrv.Liste_DO && cpt < 50 )
     { pthread_mutex_lock( &Partage->com_msrv.synchro );                                 /* Ajout dans la liste de msg a traiter */
       RootNode = Partage->com_msrv.Liste_DO->data;                                               /* Recuperation du numero de a */
       Partage->com_msrv.Liste_DO = g_slist_remove ( Partage->com_msrv.Liste_DO, RootNode );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );

       if (MSRV_Map_to_thread ( RootNode ))
        { gchar topic[256];
          g_snprintf ( topic, sizeof(topic), "thread/%s", Json_get_string ( RootNode, "thread_tech_id" ) );
          MQTT_Send_to_topic ( Partage->com_msrv.MQTT_local_session, topic, "SET_DO", RootNode );
        }
       else Info_new( __func__, Config.log_msrv, LOG_NOTICE,
                      "'%s:%s' is not mapped. dropping",
                       Json_get_string ( RootNode, "tech_id" ), Json_get_string ( RootNode, "acronyme" ) );
       Json_node_unref ( RootNode );
       cpt++;
     }

    cpt=0;
    while ( Partage->com_msrv.Liste_AO && cpt < 50 )
     { pthread_mutex_lock( &Partage->com_msrv.synchro );                              /* Ajout dans la liste de msg a traiter */
       RootNode = Partage->com_msrv.Liste_AO->data;                                            /* Recuperation du numero de a */
       Partage->com_msrv.Liste_AO = g_slist_remove ( Partage->com_msrv.Liste_AO, RootNode );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );

       if (MSRV_Map_to_thread ( RootNode ))
        { gchar topic[256];
          g_snprintf ( topic, sizeof(topic), "thread/%s", Json_get_string ( RootNode, "thread_tech_id" ) );
          MQTT_Send_to_topic ( Partage->com_msrv.MQTT_local_session, topic, "SET_AO", RootNode );
        }
       else Info_new( __func__, Config.log_msrv, LOG_NOTICE,
                      "'%s:%s' is not mapped. dropping",
                       Json_get_string ( RootNode, "tech_id" ), Json_get_string ( RootNode, "acronyme" ) );
       Json_node_unref ( RootNode );
       cpt++;
     }

  }
/*----------------------------------------------------------------------------------------------------------------------------*/
