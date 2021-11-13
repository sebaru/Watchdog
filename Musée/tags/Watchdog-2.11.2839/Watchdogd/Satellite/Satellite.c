/******************************************************************************************************************************/
/* Watchdogd/Satellite/Satellite.c        Gestion des SATELLITE de Watchdog v2.0                                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        lun. 18 févr. 2013 18:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Satellite.c
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
 #include <string.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <sys/stat.h>
 #include <netinet/in.h>
 #include <fcntl.h>
 #include <netdb.h>

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"
 #include "Satellite.h"

/******************************************************************************************************************************/
/* Satellite_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                              */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Satellite_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_satellite.lib->Thread_debug = FALSE;                                                   /* Settings default parameters */
    Cfg_satellite.enable            = FALSE; 
    Cfg_satellite.master_port       = SATELLITE_DEFAUT_PORT; 
    g_snprintf( Cfg_satellite.master_host,   sizeof(Cfg_satellite.master_host),
               "%s", SATELLITE_DEFAUT_HOST );
    g_snprintf( Cfg_satellite.ssl_file_cert, sizeof(Cfg_satellite.ssl_file_cert),
               "%s", SATELLITE_DEFAUT_FILE_SERVER );
    g_snprintf( Cfg_satellite.ssl_file_key,  sizeof(Cfg_satellite.ssl_file_key),
               "%s", SATELLITE_DEFAUT_FILE_KEY );
    g_snprintf( Cfg_satellite.ssl_file_ca,   sizeof(Cfg_satellite.ssl_file_ca),
               "%s", SATELLITE_DEFAUT_FILE_CA );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )              /* Cfg_satellite.Cfg_satellite.Connexion a la base de données */
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                "Satellite_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,                                       /* Print Config */
                "Satellite_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "ssl_file_cert" ) )
        { g_snprintf( Cfg_satellite.ssl_file_cert, sizeof(Cfg_satellite.ssl_file_cert), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_file_key" ) )
        { g_snprintf( Cfg_satellite.ssl_file_key,  sizeof(Cfg_satellite.ssl_file_key),  "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "ssl_file_ca" ) )
        { g_snprintf( Cfg_satellite.ssl_file_ca,   sizeof(Cfg_satellite.ssl_file_ca),   "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "master_port" ) )
        { Cfg_satellite.master_port = atoi( valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "master_host" ) )
        { g_snprintf( Cfg_satellite.master_host,     sizeof(Cfg_satellite.master_host),            "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_satellite.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_satellite.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
                   "Satellite_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Satellite_Gerer_events: Fonction d'abonné appellé par MSRV lorsqu'un EVENT est disponible                                  */
/* Entrée: l'event associé                                                                                                    */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Satellite_Gerer_events ( struct CMD_TYPE_MSRV_EVENT *event )
  { gint taille;

    pthread_mutex_lock( &Cfg_satellite.lib->synchro );                                       /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_satellite.liste_Events );
    pthread_mutex_unlock( &Cfg_satellite.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                "Satellite_Gerer_events: DROPPING (length = %03d > 150)", taille );
       g_free( event );
       return;
     }
    else if (Cfg_satellite.lib->Thread_run == FALSE)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,
                "Satellite_Gerer_events: Thread is down. Dropping Event" );
       g_free( event );
       return;
     }

    pthread_mutex_lock ( &Cfg_satellite.lib->synchro );                                                   /* Ajout a la liste */
    Cfg_satellite.liste_Events = g_slist_append ( Cfg_satellite.liste_Events, event );
    pthread_mutex_unlock ( &Cfg_satellite.lib->synchro );
  }
/******************************************************************************************************************************/
/* Envoyer_les_infos_au_master : se connecte au master pour lui envoyer les infos                                             */
/* Entrée : rien, sortie : rien                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_les_infos_au_master ( void )
  { gint taille;
   	while (Cfg_satellite.liste_Events)
     { struct CMD_TYPE_MSRV_EVENT *event;
       pthread_mutex_lock( &Cfg_satellite.lib->synchro );
       event = Cfg_satellite.liste_Events->data;
       Cfg_satellite.liste_Events = g_slist_remove( Cfg_satellite.liste_Events, event );
       taille = g_slist_length( Cfg_satellite.liste_Events );
       pthread_mutex_unlock( &Cfg_satellite.lib->synchro );

#ifdef bouh
       if ( event->type == EVENT_TYPE_EA &&                                    /* No man's land des EA pour la partie SYSteme */
            100<=event->num && event->num<128 )
        { /* Ignoring NoMan'sLand */ }
       else
#endif
        { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_DEBUG,
                   "Envoyer_les_infos_au_master: Sending EVENT %s:%s:%s:%f (%03d to proceed)!",
                    event->instance, event->thread, event->objet, event->val_float, taille );

          Satellite_Envoyer_maitre( TAG_SATELLITE, SSTAG_CLIENT_SAT_SET_INTERNAL,
                                   (gchar *)event, sizeof(struct CMD_TYPE_MSRV_EVENT) );
        }
       g_free(event);
     }
  }
/******************************************************************************************************************************/
/* Run_thread: Thread principal                                                                                               */
/* Entrée: une structure LIBRAIRIE                                                                                            */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { prctl(PR_SET_NAME, "W-SATELLITE", 0, 0, 0 );
    memset( &Cfg_satellite, 0, sizeof(Cfg_satellite) );                             /* Mise a zero de la structure de travail */
    Cfg_satellite.lib = lib;                                       /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_satellite.lib->TID = pthread_self();                                                /* Sauvegarde du TID pour le pere */
    Satellite_Lire_config ();                                               /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );

    g_snprintf( Cfg_satellite.lib->admin_prompt, sizeof(Cfg_satellite.lib->admin_prompt), NOM_THREAD );
    g_snprintf( Cfg_satellite.lib->admin_help,   sizeof(Cfg_satellite.lib->admin_help),   "Manage communications with Master Watchdog" );

    if (!Cfg_satellite.enable)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread is not enabled in config. Shutting Down %p",
                 pthread_self() );
       goto end;
     }

    Abonner_distribution_events ( Satellite_Gerer_events, NOM_THREAD );               /* Abonnement à la diffusion des events */

    Cfg_satellite.lib->Thread_run = TRUE;                                                               /* Le thread tourne ! */
    Cfg_satellite.Mode = SAT_DISCONNECTED;
    while(Cfg_satellite.lib->Thread_run == TRUE)                                             /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_satellite.lib->Thread_sigusr1)                                                 /* A-t'on recu un signal USR1 ? */
        { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
          Cfg_satellite.lib->Thread_sigusr1 = FALSE;
        }
       switch (Cfg_satellite.Mode)
        { case SAT_DISCONNECTED:
                { Cfg_satellite.date_next_retry = Partage->top + SATELLITE_TIME_NEXT_RETRY;
                  Cfg_satellite.Mode = SAT_RETRY_CONNECT;
                  Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,
                           "Run_thread: Waiting %d sec before trying to re-connect",
                            SATELLITE_TIME_NEXT_RETRY/10 );
                }
               break;
          case SAT_RETRY_CONNECT:                                                            /* Connexion à l'instance maitre */
               if (Partage->top >= Cfg_satellite.date_next_retry)
                { if (Satellite_Connecter() == FALSE)                                       /* Connecter de base à l'instance */
                   { Satellite_Deconnecter_sale(); }
                  else
                   { Cfg_satellite.Mode = SAT_ATTENTE_INTERNAL;
                     Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,
                              "Run_thread: Satellite SAT_ATTENTE_INTERNAL" );
                   }
                }
               break;
          case SAT_ATTENTE_CONNEXION_SSL:
               if (Satellite_Connecter_ssl() == FALSE)
                { Satellite_Deconnecter_sale(); }
               else
                { struct REZO_CLI_IDENT ident;
                  g_snprintf( ident.nom,     sizeof(ident.nom), "Watchdog Satellite on %s", Config.instance_id );
                  g_snprintf( ident.version, sizeof(ident.version), "%s", VERSION );

                  Satellite_Envoyer_maitre( TAG_CONNEXION, SSTAG_CLIENT_IDENT,
                                            (gchar *)&ident, sizeof(struct REZO_CLI_IDENT) );
                }
               break;
          case SAT_CONNECTED:
                { if ( Cfg_satellite.liste_Events )                                         /* Si changement, envoi au master */
                   { Envoyer_les_infos_au_master(); } 
                }
               break;
        }

       if (Cfg_satellite.Mode >= SAT_ATTENTE_INTERNAL) Satellite_Ecouter_maitre();
     }

   Desabonner_distribution_events ( Satellite_Gerer_events );                         /* Abonnement à la diffusion des events */

end:
    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
             "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_satellite.lib->TID = 0;                                            /* On indique au satellite que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
