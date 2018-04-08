/******************************************************************************************************************************/
/* Watchdogd/distrib_Events.c        Distribution des changements d'etats motif                                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        sam. 24 janv. 2015 13:53:26 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * distrib_Events.c
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

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Map_event_to_mnemo: Associe l'event en parametre aux mnemoniques D.L.S                                                     */
/* Entrée: l'evenement à traiter                                                                                              */
/* Sortie: le mnemo en question, ou NULL si non-trouvé (ou multi trouvailles)                                                 */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_BASE *Map_event_to_mnemo( gchar *thread, gchar *event, gint *retour_nbr )
  { struct CMD_TYPE_MNEMO_BASE *mnemo, *result_mnemo = NULL;
    gint nbr_result;
    struct DB *db;

    if ( ! Recuperer_mnemo_baseDB_by_event_text ( &db, thread, event ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "%s: Error searching Database for '%s'", __func__, event );
       return(NULL);
     }
    *retour_nbr = nbr_result = db->nbr_result;
          
    if ( nbr_result == 0 )                                                                  /* Si pas d'enregistrement trouvé */
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: No match found for '%s'", __func__, event );
       return(NULL);
     }

    while ( (mnemo = Recuperer_mnemo_baseDB_suite( &db )) != NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: Match found for '%s' Type %d Num %d - %s", __func__,
                 event, mnemo->type, mnemo->num, mnemo->libelle );
       if (result_mnemo != NULL) { g_free(result_mnemo); }
       result_mnemo = mnemo;                                                                                /* Last result OK */
     }

    return (result_mnemo);                                           /* A-t'on le seul et unique Mnemo associé à cet event ?? */
  }
/******************************************************************************************************************************/
/* Gerer_arrive_Axxx_dls: Gestion de l'arrive des sorties depuis DLS (Axxx = 1)                                               */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Gerer_arrive_Axxx_dls ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE critere;
    struct CMD_TYPE_MNEMO_BASE *mnemo;
    gint num, reste;

    if (!Partage->com_msrv.liste_a) return;                                                       /* Si pas de a, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );                                 /* Ajout dans la liste de msg a traiter */
    num = GPOINTER_TO_INT(Partage->com_msrv.liste_a->data);                                    /* Recuperation du numero de a */
    Partage->com_msrv.liste_a = g_slist_remove ( Partage->com_msrv.liste_a, GINT_TO_POINTER(num) );
    reste = g_slist_length(Partage->com_msrv.liste_a);
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    critere.type = MNEMO_SORTIE;                                                /* Recherche du ev_text associé au mnemonique */
    critere.num  = num;
    mnemo = Rechercher_mnemo_baseDB_type_num ( &critere );
    if (!mnemo)
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Mnemo not found for A%03d", __func__, num );
       return;
     }

    if ( strlen ( mnemo->ev_text ) > 0 )                           /* Existe t'il un evenement associé ? (implique furtivité) */
     { if ( !strcmp(mnemo->ev_host, g_get_host_name()) || !strcmp(mnemo->ev_host, "*"))
        { Send_zmq_with_tag ( Partage->com_msrv.zmq_to_threads, TAG_ZMQ_TO_THREADS,
                              mnemo->ev_host, mnemo->ev_thread, mnemo->ev_text, strlen(mnemo->ev_text)+1 );
        }
       else
        { Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, TAG_ZMQ_TO_THREADS,
                           mnemo->ev_host, mnemo->ev_thread, mnemo->ev_text, strlen(mnemo->ev_text)+1 );
        }

       Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Recu A(%03d) (%s/%s/%s). Reste a traiter %03d", __func__,
                 num, mnemo->ev_host, mnemo->ev_thread, mnemo->ev_text, reste
               );
       SA ( num, 0 );                                                   /* L'evenement est traité, on fait retomber la sortie */
     }
    g_free(mnemo);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
