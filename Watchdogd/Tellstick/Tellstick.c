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
/* Ajouter_tellstick: Ajoute une demande d'envoi RF tellstick dans la liste des envoi tellstick           */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                */
/**********************************************************************************************************/
 void Ajouter_tellstick( gint id, gint val )
  { struct TELLSTICKDB *tell;

    if (id < Config.tellstick_a_min || Config.tellstick_a_max < id) return;             /* Test d'echelle */
     
    if (Partage->com_tellstick.taille_tell > 150)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "Ajouter_tell: DROP tell (taille>150)  id=%d", id );
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
/* Admin_tellstick_learn: Envoi une commande de LEARN tellstick                                           */
/* Entrée: Le client admin et le numéro ID du tellstick                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_tellstick_learn ( struct CLIENT_ADMIN *client, gint num )
  { int methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Envoi de la commande LEARN TellstickS\n" );
    Write_admin ( client->connexion, chaine );

    methods = tdMethods( num, TELLSTICK_LEARN );                                 /* Get methods of device */

    if ( methods | TELLSTICK_LEARN )
     { Info_new( Config.log, Config.log_all, LOG_INFO, "Run_tellstick: Learning %d", num );
       tdLearn ( num );
     }

    g_snprintf( chaine, sizeof(chaine), "   Tellstick -> Learning of device = %d\n", num );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_tellstick_start: Envoi une commande de START tellstick                                           */
/* Entrée: Le client admin et le numéro ID du tellstick                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_tellstick_start ( struct CLIENT_ADMIN *client, gint num )
  { int methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demande d'activation d'un device Tellstick\n" );
    Write_admin ( client->connexion, chaine );

    methods = tdMethods( num, TELLSTICK_TURNON );                                /* Get methods of device */

    if ( methods | TELLSTICK_TURNON )
     { Info_new( Config.log, Config.log_all, LOG_INFO, "Run_tellstick: Starting %d", num );
       tdTurnOn ( num );
     }

    g_snprintf( chaine, sizeof(chaine), "   Tellstick -> Starting device = %d\n", num );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_tellstick_stop : Envoi une commande de STOP  tellstick                                           */
/* Entrée: Le client admin et le numéro ID du tellstick                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_tellstick_stop ( struct CLIENT_ADMIN *client, gint num )
  { int methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demande de desactivation d'un deviece Tellstick\n" );
    Write_admin ( client->connexion, chaine );

    methods = tdMethods( num, TELLSTICK_TURNOFF );                               /* Get methods of device */

    if ( methods | TELLSTICK_TURNOFF )
     { Info_new( Config.log, Config.log_all, LOG_INFO, "Run_tellstick: Stopping %d", num );
       tdTurnOff ( num );
     }

    g_snprintf( chaine, sizeof(chaine), "   Tellstick -> Stoppping device = %d\n", num );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_tellstick_list ( struct CLIENT_ADMIN *client )
  { int nbrDevice, i, supportedMethods, methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des device Tellstick\n" );
    Write_admin ( client->connexion, chaine );

    nbrDevice = tdGetNumberOfDevices();
    g_snprintf( chaine, sizeof(chaine), "   Tellstick -> Number of devices = %d\n", nbrDevice );
    Write_admin ( client->connexion, chaine );

    for (i= 0; i<nbrDevice; i++)
     { char *name, *proto, *house, *unit;
       int id;
       id    = tdGetDeviceId( i );
       name  = tdGetName( id );
       proto = tdGetProtocol( id );
       house = tdGetDeviceParameter( id, "house", "NULL" );
       unit  = tdGetDeviceParameter( id, "unit", "NULL" );
       supportedMethods = TELLSTICK_TURNON | TELLSTICK_TURNOFF | TELLSTICK_BELL | TELLSTICK_LEARN;
       methods = tdMethods( id, supportedMethods );

       g_snprintf( chaine, sizeof(chaine),
                   "   Tellstick [%d] -> proto=%s, house=%s, unit=%s, methods=%s-%s-%s-%s, name=%s\n",
                   id, proto, house, unit,
                   ( methods & TELLSTICK_TURNON  ? "ON"    : "  "     ),
                   ( methods & TELLSTICK_TURNOFF ? "OFF"   : "   "    ),
                   ( methods & TELLSTICK_BELL    ? "BELL"  : "    "   ),
                   ( methods & TELLSTICK_LEARN   ? "LEARN" : "      " ),
                   name
                 );
       Write_admin ( client->connexion, chaine );
       tdReleaseString(name);
       tdReleaseString(proto);
       tdReleaseString(house);
       tdReleaseString(unit);
     }
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Tellstick                                                          */
/**********************************************************************************************************/
 void Run_tellstick ( void )
  { guint methods;
    prctl(PR_SET_NAME, "W-Tellstick", 0, 0, 0 );

    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Starting" );

    Partage->com_tellstick.liste_tell = NULL;                             /* Initialisation des variables */
    tdInit();

    Partage->com_tellstick.Thread_run = TRUE;                    /* On dit au maitre que le thread tourne */
    while(Partage->com_tellstick.Thread_run == TRUE)                  /* On tourne tant que l'on a besoin */
     { struct TELLSTICKDB *tell;
       if (Partage->com_tellstick.Thread_reload)                                      /* On a recu reload */
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Run_tellstick: RELOAD" );
          Partage->com_tellstick.Thread_reload = FALSE;
        }

       if (Partage->com_tellstick.Thread_sigusr1)                                 /* On a recu sigusr1 ?? */
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Run_tellstick: SIGUSR1" );
          Info_new( Config.log, Config.log_all, LOG_INFO, "Run_tellstick: Reste a traiter %d",
                  Partage->com_tellstick.taille_tell );
          Partage->com_tellstick.Thread_sigusr1 = FALSE;
        }

       if (!Partage->com_tellstick.liste_tell)                            /* Si pas de message, on tourne */
        { sched_yield();
          usleep(10000);
          continue;
        }

       pthread_mutex_lock( &Partage->com_tellstick.synchro );                            /* lockage futex */
       tell = Partage->com_tellstick.liste_tell->data;                            /* Recuperation du tell */
       Partage->com_tellstick.liste_tell = g_list_remove ( Partage->com_tellstick.liste_tell, tell );
       Info_new( Config.log, Config.log_all, LOG_INFO, "Run_tellstick: Reste a traiter %d",
                                       g_list_length(Partage->com_tellstick.liste_tell) );
       Partage->com_tellstick.taille_tell--;
       pthread_mutex_unlock( &Partage->com_tellstick.synchro );

       methods = tdMethods( tell->id, TELLSTICK_TURNON | TELLSTICK_TURNOFF );    /* Get methods of device */

       if ( tell->val == 1 && (methods | TELLSTICK_TURNON) )
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Run_tellstick: Turning %d ON", tell->id );
          tdTurnOn ( tell->id );
        }
       else if ( tell->val == 0 && (methods | TELLSTICK_TURNOFF) )
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Run_tellstick: Turning %d OFF", tell->id );
          tdTurnOff ( tell->id );
        }

       g_free(tell);
     }
    tdClose();
    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_tellstick: Down (%d)", pthread_self() );
    Partage->com_tellstick.TID = 0;                       /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
