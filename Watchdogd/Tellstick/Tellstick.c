/**********************************************************************************************************/
/* Watchdogd/Tellstick/Tellstick.c  Gestion des tellivages bit_internes Watchdog 2.0                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 08 jui 2006 11:56:48 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Tellstick.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <unistd.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 /*#define DEBUG*/
/**********************************************************************************************************/
/* Ajouter_tell: Ajoute une tellive dans la base de données                                               */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                */
/**********************************************************************************************************/
 void Ajouter_tellstick( gint num )
  {
    if (Partage->com_tellstick.taille_tell > 150)
     { Info_n( Config.log, DEBUG_TELLSTICK, "TELLSTICK: Ajouter_tell: DROP tell (taille>150)  num", num );
       return;
     }

    pthread_mutex_lock( &Partage->com_tellstick.synchro );            /* Ajout dans la liste de tell a traiter */
    Partage->com_tellstick.liste_tell = g_list_append( Partage->com_tellstick.liste_tell, GINT_TO_POINTER(num) );
    Partage->com_tellstick.taille_tell++;
    pthread_mutex_unlock( &Partage->com_tellstick.synchro );
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Tellstick                                                          */
/**********************************************************************************************************/
 void Run_tellstick ( void )
  { guint top, num;
    prctl(PR_SET_NAME, "W-Tellstick", 0, 0, 0 );

    Info( Config.log, DEBUG_TELLSTICK, "TELLSTICK: demarrage" );

    Partage->com_tellstick.liste_tell = NULL;                                  /* Initialisation des variables */
    top = Partage->top;                                        /* Initialisation des tellivages temporels */
    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { if (Partage->com_tellstick.sigusr1)                                             /* On a recu sigusr1 ?? */
        { Partage->com_tellstick.sigusr1 = FALSE;
          Info( Config.log, DEBUG_TELLSTICK, "TELLSTICK: Run_tellstick: SIGUSR1" );
        }

       if (!Partage->com_tellstick.liste_tell)                                 /* Si pas de message, on tourne */
        { sched_yield();
          usleep(10000);
          continue;
        }

       pthread_mutex_lock( &Partage->com_tellstick.synchro );                                 /* lockage futex */
       num = GPOINTER_TO_INT(Partage->com_tellstick.liste_tell->data);                /* Recuperation du tell */
       Partage->com_tellstick.liste_tell = g_list_remove ( Partage->com_tellstick.liste_tell,
                                                      Partage->com_tellstick.liste_tell->data );
       Info_n( Config.log, DEBUG_TELLSTICK, "TELLSTICK: Run_tellstick: Reste a traiter",
                                       g_list_length(Partage->com_tellstick.liste_tell) );
       Partage->com_tellstick.taille_tell--;
       pthread_mutex_unlock( &Partage->com_tellstick.synchro );


       /*Ajouter_tellDB ( Config.log, db, tell );*/
     }
    Info_n( Config.log, DEBUG_TELLSTICK, "TELLSTICK: Run_tellstick: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
