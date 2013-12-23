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
 
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <unistd.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 #include "Tellstick.h"

/**********************************************************************************************************/
/* Tellstick_Lire_config : Lit la config Watchdog et rempli la structure mémoire                          */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Tellstick_Lire_config ( void )
  { GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Tellstick_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_tellstick.lib->Thread_debug = g_key_file_get_boolean ( gkf, "TELLSTICK", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Tellstick_Liberer_config : Libere la mémoire allouer précédemment pour lire la config tellstick        */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Tellstick_Liberer_config ( void )
  { 
  }
/**********************************************************************************************************/
/* Ajouter_tellstick: Ajoute une demande d'envoi RF tellstick dans la liste des envoi tellstick           */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                */
/**********************************************************************************************************/
 void Tellstick_Gerer_sortie( gint num_a )                                 /* Num_a est l'id du tellstick */
  { gint taille;

    pthread_mutex_lock( &Cfg_tellstick.lib->synchro );           /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_tellstick.Liste_tell );
    pthread_mutex_unlock( &Cfg_tellstick.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_tellstick.lib->Thread_debug, LOG_WARNING,
                "Ajouter_tell: DROP tell (taille>150)  id=%d", num_a );
       return;
     }

    pthread_mutex_lock( &Cfg_tellstick.lib->synchro );       /* Ajout dans la liste de tell a traiter */
    Cfg_tellstick.Liste_tell = g_slist_prepend( Cfg_tellstick.Liste_tell, GINT_TO_POINTER(num_a) );
    pthread_mutex_unlock( &Cfg_tellstick.lib->synchro );
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Tellstick                                                          */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { guint methods;

    prctl(PR_SET_NAME, "W-Tellstick", 0, 0, 0 );
    memset( &Cfg_tellstick, 0, sizeof(Cfg_tellstick) );         /* Mise a zero de la structure de travail */
    Cfg_tellstick.lib = lib;                   /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_tellstick.lib->TID = pthread_self();                            /* Sauvegarde du TID pour le pere */
    Tellstick_Lire_config ();                           /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_tellstick.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );
    Cfg_tellstick.lib->Thread_run = TRUE;                                           /* Le thread tourne ! */

    g_snprintf( Cfg_tellstick.lib->admin_prompt, sizeof(Cfg_tellstick.lib->admin_prompt), "tellstick" );
    g_snprintf( Cfg_tellstick.lib->admin_help,   sizeof(Cfg_tellstick.lib->admin_help),   "Manage Tellstick system" );

    sleep(10);                                                 /* attente 10 secondes pour initialisation */
    tdInit();                                                 /* Initialisation de la librairie tellstick */
    Abonner_distribution_sortie ( Tellstick_Gerer_sortie );     /* Abonnement de la diffusion des sorties */
    while(lib->Thread_run == TRUE)                                    /* On tourne tant que l'on a besoin */
     { gint id;

       if (lib->Thread_sigusr1)                                                   /* On a recu sigusr1 ?? */
        { Info_new( Config.log, lib->Thread_debug, LOG_INFO, "Run_tellstick: SIGUSR1" );
          pthread_mutex_lock ( &Cfg_tellstick.lib->synchro );
          Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
                    "Run_thread: USR1 -> Nbr of Tellstick to send=%d",
                    g_slist_length ( Cfg_tellstick.Liste_tell )
                  );
          pthread_mutex_unlock ( &Cfg_tellstick.lib->synchro );
          lib->Thread_sigusr1 = FALSE;
        }

       if (!Cfg_tellstick.Liste_tell)                            /* Si pas de message, on tourne */
        { sched_yield();
          usleep(10000);
          continue;
        }

       pthread_mutex_lock( &Cfg_tellstick.lib->synchro );                                /* lockage futex */
       id = GPOINTER_TO_INT(Cfg_tellstick.Liste_tell->data);                      /* Recuperation du tell */
       Cfg_tellstick.Liste_tell = g_slist_remove ( Cfg_tellstick.Liste_tell, GINT_TO_POINTER(id) );
       Info_new( Config.log, Cfg_tellstick.lib->Thread_debug, LOG_INFO,
                "Run_tellstick: Reste a traiter %d",
                 g_slist_length(Cfg_tellstick.Liste_tell) );
       pthread_mutex_unlock( &Cfg_tellstick.lib->synchro );

       methods = tdMethods( id, TELLSTICK_TURNON | TELLSTICK_TURNOFF );          /* Get methods of device */

       if ( A(id) == 1 && (methods | TELLSTICK_TURNON) )
        { Info_new( Config.log, Cfg_tellstick.lib->Thread_debug, LOG_INFO, "Run_tellstick: Turning %d ON", id );
          tdTurnOn ( id );
        }
       else if ( A(id) == 0 && (methods | TELLSTICK_TURNOFF) )
        { Info_new( Config.log, Cfg_tellstick.lib->Thread_debug, LOG_INFO, "Run_tellstick: Turning %d OFF", id );
          tdTurnOff ( id );
        }
     }
    Desabonner_distribution_sortie ( Tellstick_Gerer_sortie );/* Desabonnement de la diffusion des sorties */
    tdClose();
    Tellstick_Liberer_config();                               /* Liberation de la configuration du thread */

    Info_new( Config.log, Cfg_tellstick.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_tellstick.lib->TID = 0;                           /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
