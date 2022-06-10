/******************************************************************************************************************************/
/* Watchdogd/api_sync.c        Interconnexion avec l'API                                                                      */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    10.06.2022 10:04:24 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * api_sync.c
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
 #include <sys/prctl.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Main: Fonction principale du thread                                                                                        */
/******************************************************************************************************************************/
 void Run_api_sync ( void )
  { prctl(PR_SET_NAME, "W-APISYNC", 0, 0, 0 );

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );

    while(Partage->com_msrv.Thread_run == TRUE)                                              /* On tourne tant que necessaire */
     {
/*---------------------------------------------- Report des visuels ----------------------------------------------------------*/
      if (Partage->com_msrv.liste_visuel) API_Send_visuels ();                                 /* Traitement des I dynamiques */
/*---------------------------------------------- Report des messages ---------------------------------------------------------*/
      else if (Partage->com_msrv.liste_msg) API_Send_MSGS();
      else if (Partage->archive_liste) API_Send_ARCHIVE();
      else { sched_yield(); sleep(2); }
     }

    API_Clear_ARCHIVE();                                            /* Suppression des enregistrements restants dans la liste */

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Down (%p)", __func__, pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
