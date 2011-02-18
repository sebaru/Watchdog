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
 #include <telldus-core.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */

/**********************************************************************************************************/
/* Ajouter_tell: Ajoute une tellive dans la base de données                                               */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                */
/**********************************************************************************************************/
 void Ajouter_tellstick( gint id, gint val )
  { struct TELLSTICKDB *tell;
    if (id < Config.tellstick_a_min || Config.tellstick_a_max < id) return;             /* Test d'echelle */
     
    if (Partage->com_tellstick.taille_tell > 150)
     { Info_n( Config.log, DEBUG_TELLSTICK, "TELLSTICK: Ajouter_tell: DROP tell (taille>150)  id", id );
       return;
     }

    tell = (struct TELLSTICKDB *)g_malloc( sizeof(struct TELLSTICKDB) );
    if (!tell) return;

    tell->id  = id;
    tell->val = val;

    pthread_mutex_lock( &Partage->com_tellstick.synchro );       /* Ajout dans la liste de tell a traiter */
    Partage->com_tellstick.liste_tell = g_list_append( Partage->com_tellstick.liste_tell, tell );
    Partage->com_tellstick.taille_tell++;
    pthread_mutex_unlock( &Partage->com_tellstick.synchro );
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Tellstick                                                          */
/**********************************************************************************************************/
 void Run_tellstick ( void )
  { guint top, methods;
    prctl(PR_SET_NAME, "W-Tellstick", 0, 0, 0 );

    Info( Config.log, DEBUG_TELLSTICK, "TELLSTICK: demarrage" );

    Partage->com_tellstick.liste_tell = NULL;                             /* Initialisation des variables */
    top = Partage->top;                                        /* Initialisation des tellivages temporels */
    tdInit();
    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { struct TELLSTICKDB *tell;
       if (Partage->com_tellstick.sigusr1)                                        /* On a recu sigusr1 ?? */
        { Partage->com_tellstick.sigusr1 = FALSE;
          Info( Config.log, DEBUG_TELLSTICK, "TELLSTICK: Run_tellstick: SIGUSR1" );
        }

       if (!Partage->com_tellstick.liste_tell)                            /* Si pas de message, on tourne */
        { sched_yield();
          usleep(10000);
          continue;
        }

       pthread_mutex_lock( &Partage->com_tellstick.synchro );                            /* lockage futex */
       tell = Partage->com_tellstick.liste_tell->data;                            /* Recuperation du tell */
       Partage->com_tellstick.liste_tell = g_list_remove ( Partage->com_tellstick.liste_tell, tell );
       Info_n( Config.log, DEBUG_TELLSTICK, "TELLSTICK: Run_tellstick: Reste a traiter",
                                       g_list_length(Partage->com_tellstick.liste_tell) );
       Partage->com_tellstick.taille_tell--;
       pthread_mutex_unlock( &Partage->com_tellstick.synchro );

       methods = tdMethods( tell->id, TELLSTICK_TURNON | TELLSTICK_TURNOFF );    /* Get methods of device */

       if ( tell->val == 1 && (methods | TELLSTICK_TURNON) )
        { Info_n( Config.log, DEBUG_TELLSTICK, "TELLSTICK: Run_tellstick: Turning ON", tell->id );
          tdTurnOn ( tell-> id );
        }
       else if ( tell->val == 0 && (methods | TELLSTICK_TURNOFF) )
        { Info_n( Config.log, DEBUG_TELLSTICK, "TELLSTICK: Run_tellstick: Turning OFF", tell->id );
          tdTurnOff ( tell-> id );
        }

       g_free(tell);
     }
    tdClose();
    Info_n( Config.log, DEBUG_TELLSTICK, "TELLSTICK: Run_tellstick: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
