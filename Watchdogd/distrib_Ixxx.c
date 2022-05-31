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
  { if (!Partage->com_msrv.liste_visuel) return;                                               /* Traitement des I dynamiques */

    JsonNode *RootNode  = Json_node_create();
    if (!RootNode) return;
    JsonArray *visuels = Json_node_add_array ( RootNode, "visuels" );
    if (!visuels) return;

    gint top = Partage->top;
    gint nb_enreg = 0;
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                        /* lockage futex */
    while (Partage->com_msrv.liste_visuel && Partage->com_msrv.Thread_run == TRUE && nb_enreg<100)
     { struct DLS_VISUEL *visuel = Partage->com_msrv.liste_visuel->data;                            /* Recuperation du visuel */
       Partage->com_msrv.liste_visuel = g_slist_remove ( Partage->com_msrv.liste_visuel, visuel );
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                "%s: Recu VISUEL %s:%s mode=%s, color=%s, cligno=%d.", __func__,
                 visuel->tech_id, visuel->acronyme, visuel->mode, visuel->color, visuel->cligno
               );
       JsonNode *element = Json_node_create ();
       Dls_VISUEL_to_json ( element, visuel );
       Json_node_add_string ( element, "zmq_tag", "DLS_VISUEL" ); /* A virer une fois full API */
       Http_ws_send_to_all( element ); /* A virer une fois full API */
       Json_array_add_element ( visuels, element );
       nb_enreg++;                           /* Permet de limiter a au plus 100 enregistrements histoire de limiter la famine */
     }
    gint reste = g_slist_length(Partage->com_msrv.liste_visuel);
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Json_node_add_int ( RootNode, "nbr_visuels", nb_enreg );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/visuels/set", RootNode );
    if (api_result && Json_get_int ( api_result, "api_status" ) == SOUP_STATUS_OK )
     { gint nbr_saved = Json_get_int ( api_result, "nbr_visuels_saved" );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Traitement de %05d visuel(s) en %06.1fs. Reste %05d.", __func__,
                 nbr_saved, (Partage->top-top)/10.0, reste );
     }
    else
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s: API Error. Reste %05d", __func__, reste ); }
    Json_node_unref ( RootNode );
    Json_node_unref ( api_result );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
