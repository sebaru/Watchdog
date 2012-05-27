/**********************************************************************************************************/
/* Watchdogd/Rfxcom/Rfxcom.c  Gestion des tellivages bit_internes Watchdog 2.0                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 27 mai 2012 12:52:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Rfxcom.c
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

/**********************************************************************************************************/
/* Ajouter_rfxcom: Ajoute une demande d'envoi RF rfxcom dans la liste des envoi rfxcom                    */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                */
/**********************************************************************************************************/
 void Ajouter_rfxcom( gint id, gint val )
  { struct RFXCOMDB *tell;
#ifdef bouh
    if (id < Config.rfxcom_a_min || Config.rfxcom_a_max < id) return;             /* Test d'echelle */
     
    if (Partage->com_rfxcom.taille_tell > 150)
     { Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Ajouter_tell: DROP tell (taille>150)  id", id );
       return;
     }

    tell = (struct RFXCOMDB *)g_malloc( sizeof(struct RFXCOMDB) );
    if (!tell) return;

    tell->id  = id;
    tell->val = val;

    pthread_mutex_lock( &Partage->com_rfxcom.synchro );       /* Ajout dans la liste de tell a traiter */
    Partage->com_rfxcom.liste_tell = g_list_append( Partage->com_rfxcom.liste_tell, tell );
    Partage->com_rfxcom.taille_tell++;
    pthread_mutex_unlock( &Partage->com_rfxcom.synchro );
#endif
  }
/**********************************************************************************************************/
/* Admin_rfxcom_learn: Envoi une commande de LEARN rfxcom                                           */
/* Entrée: Le client admin et le numéro ID du rfxcom                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_rfxcom_learn ( struct CLIENT_ADMIN *client, gint num )
  { int methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Envoi de la commande LEARN RfxcomS\n" );
    Write_admin ( client->connexion, chaine );
#ifdef bouh
    methods = tdMethods( num, RFXCOM_LEARN );                                 /* Get methods of device */

    if ( methods | RFXCOM_LEARN )
     { Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Learning", num );
       tdLearn ( num );
     }
#endif
    g_snprintf( chaine, sizeof(chaine), "   Rfxcom -> Learning of device = %d\n", num );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_rfxcom_start: Envoi une commande de START rfxcom                                                 */
/* Entrée: Le client admin et le numéro ID du rfxcom                                                      */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_rfxcom_start ( struct CLIENT_ADMIN *client, gint num )
  { int methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demande d'activation d'un device Rfxcom\n" );
    Write_admin ( client->connexion, chaine );
#ifdef bouh
    methods = tdMethods( num, RFXCOM_TURNON );                                /* Get methods of device */

    if ( methods | RFXCOM_TURNON )
     { Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Starting", num );
       tdTurnOn ( num );
     }

    g_snprintf( chaine, sizeof(chaine), "   Rfxcom -> Starting device = %d\n", num );
    Write_admin ( client->connexion, chaine );
#endif
  }
/**********************************************************************************************************/
/* Admin_rfxcom_stop : Envoi une commande de STOP  rfxcom                                           */
/* Entrée: Le client admin et le numéro ID du rfxcom                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_rfxcom_stop ( struct CLIENT_ADMIN *client, gint num )
  { int methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demande de desactivation d'un deviece Rfxcom\n" );
    Write_admin ( client->connexion, chaine );
#ifdef bouh
    methods = tdMethods( num, RFXCOM_TURNOFF );                               /* Get methods of device */

    if ( methods | RFXCOM_TURNOFF )
     { Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Stopping", num );
       tdTurnOff ( num );
     }

    g_snprintf( chaine, sizeof(chaine), "   Rfxcom -> Stoppping device = %d\n", num );
    Write_admin ( client->connexion, chaine );
#endif
  }
/**********************************************************************************************************/
/* Admin_rfxcom_list: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_rfxcom_list ( struct CLIENT_ADMIN *client )
  { int nbrDevice, i, supportedMethods, methods;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des device Rfxcom\n" );
    Write_admin ( client->connexion, chaine );
#ifdef bouh
    nbrDevice = tdGetNumberOfDevices();
    g_snprintf( chaine, sizeof(chaine), "   Rfxcom -> Number of devices = %d\n", nbrDevice );
    Write_admin ( client->connexion, chaine );

    for (i= 0; i<nbrDevice; i++)
     { char *name, *proto, *house, *unit;
       int id;
       id    = tdGetDeviceId( i );
       name  = tdGetName( id );
       proto = tdGetProtocol( id );
       house = tdGetDeviceParameter( id, "house", "NULL" );
       unit  = tdGetDeviceParameter( id, "unit", "NULL" );
       supportedMethods = RFXCOM_TURNON | RFXCOM_TURNOFF | RFXCOM_BELL | RFXCOM_LEARN;
       methods = tdMethods( id, supportedMethods );

       g_snprintf( chaine, sizeof(chaine),
                   "   Rfxcom [%d] -> proto=%s, house=%s, unit=%s, methods=%s-%s-%s-%s, name=%s\n",
                   id, proto, house, unit,
                   ( methods & RFXCOM_TURNON  ? "ON"    : "  "     ),
                   ( methods & RFXCOM_TURNOFF ? "OFF"   : "   "    ),
                   ( methods & RFXCOM_BELL    ? "BELL"  : "    "   ),
                   ( methods & RFXCOM_LEARN   ? "LEARN" : "      " ),
                   name
                 );
       Write_admin ( client->connexion, chaine );
       tdReleaseString(name);
       tdReleaseString(proto);
       tdReleaseString(house);
       tdReleaseString(unit);
     }
#endif
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Rfxcom                                                          */
/**********************************************************************************************************/
 void Run_rfxcom ( void )
  { guint methods;
    prctl(PR_SET_NAME, "W-Rfxcom", 0, 0, 0 );

    Info( Config.log, DEBUG_RFXCOM, "RFXCOM: demarrage" );

#ifdef bouh
    Partage->com_rfxcom.liste_tell = NULL;                             /* Initialisation des variables */
    tdInit();

    Partage->com_rfxcom.Thread_run = TRUE;                    /* On dit au maitre que le thread tourne */
    while(Partage->com_rfxcom.Thread_run == TRUE)                  /* On tourne tant que l'on a besoin */
     { struct RFXCOMDB *tell;
       if (Partage->com_rfxcom.Thread_reload)                                      /* On a recu reload */
        { Info( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: RELOAD" );
          Partage->com_rfxcom.Thread_reload = FALSE;
        }

       if (Partage->com_rfxcom.Thread_sigusr1)                                 /* On a recu sigusr1 ?? */
        { Info( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: SIGUSR1" );
          Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Reste a traiter",
                  Partage->com_rfxcom.taille_tell );
          Partage->com_rfxcom.Thread_sigusr1 = FALSE;
        }

       if (!Partage->com_rfxcom.liste_tell)                            /* Si pas de message, on tourne */
        { sched_yield();
          usleep(10000);
          continue;
        }

       pthread_mutex_lock( &Partage->com_rfxcom.synchro );                            /* lockage futex */
       tell = Partage->com_rfxcom.liste_tell->data;                            /* Recuperation du tell */
       Partage->com_rfxcom.liste_tell = g_list_remove ( Partage->com_rfxcom.liste_tell, tell );
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Reste a traiter",
                                       g_list_length(Partage->com_rfxcom.liste_tell) );
       Partage->com_rfxcom.taille_tell--;
       pthread_mutex_unlock( &Partage->com_rfxcom.synchro );

       methods = tdMethods( tell->id, RFXCOM_TURNON | RFXCOM_TURNOFF );    /* Get methods of device */

       if ( tell->val == 1 && (methods | RFXCOM_TURNON) )
        { Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Turning ON", tell->id );
          tdTurnOn ( tell->id );
        }
       else if ( tell->val == 0 && (methods | RFXCOM_TURNOFF) )
        { Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Turning OFF", tell->id );
          tdTurnOff ( tell->id );
        }

       g_free(tell);
     }
    tdClose();
#endif
    Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Down", pthread_self() );
    Partage->com_rfxcom.TID = 0;                       /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
