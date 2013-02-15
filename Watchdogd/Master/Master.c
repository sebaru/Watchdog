/**********************************************************************************************************/
/* Watchdogd/master.c        Gestion des MASTER de Watchdog v2.0                                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                   ven. 02 avril 2010 20:37:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Master.c
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

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Master.h"

 static GSList *Liste_slave;                                       /* Liste des slaves attachés au master */

/**********************************************************************************************************/
/* Master_Lire_config : Lit la config Watchdog et rempli la structure mémoire                             */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Master_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Master_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_master.lib->Thread_debug = g_key_file_get_boolean ( gkf, "MASTER", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */

    Cfg_master.enable = g_key_file_get_boolean ( gkf, "MASTER", "enable", NULL ); 

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Master_Liberer_config : Libere la mémoire allouer précédemment pour lire la config master              */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Master_Liberer_config ( void )
  {
  }
/**********************************************************************************************************/
/* Master_Gerer_message: Fonction d'abonné appellé lorsqu'un message est disponible.                      */
/* Entrée: une structure CMD_TYPE_HISTO                                                                   */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Master_Gerer_message ( struct CMD_TYPE_MESSAGE *msg )
  { gint taille;

    pthread_mutex_lock( &Cfg_master.lib->synchro );                      /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_master.Liste_message );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Master_Gerer_message: DROP message %d (length = %d > 150)", msg->num, taille);
       g_free(msg);
       return;
     }

    pthread_mutex_lock ( &Cfg_master.lib->synchro );
    Cfg_master.Liste_message = g_slist_append ( Cfg_master.Liste_message, msg );      /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_master.lib->synchro );
  }
/**********************************************************************************************************/
/* Master_Gerer_sortie: Ajoute une demande d'envoi RF dans la liste des envois RFXCOM                     */
/* Entrées: le numéro de la sortie                                                                        */
/**********************************************************************************************************/
 void Master_Gerer_sortie( gint num_a )                                    /* Num_a est l'id de la sortie */
  { gint taille;

    pthread_mutex_lock( &Cfg_master.lib->synchro );              /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_master.Liste_sortie );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Master_Gerer_sortie: DROP sortie %d (length = %d > 150)", num_a, taille );
       return;
     }

    pthread_mutex_lock( &Cfg_master.lib->synchro );       /* Ajout dans la liste de tell a traiter */
    Cfg_master.Liste_sortie = g_slist_prepend( Cfg_master.Liste_sortie, GINT_TO_POINTER(num_a) );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );
  }
/**********************************************************************************************************/
/* Run_thread: Thread principal                                                                           */
/* Entrée: une structure LIBRAIRIE                                                                        */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct CMD_TYPE_MESSAGE *msg;

    prctl(PR_SET_NAME, "W-MASTER", 0, 0, 0 );
    memset( &Cfg_master, 0, sizeof(Cfg_master) );               /* Mise a zero de la structure de travail */
    Cfg_master.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    Master_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );

    g_snprintf( Cfg_master.lib->admin_prompt, sizeof(Cfg_master.lib->admin_prompt), "master" );
    g_snprintf( Cfg_master.lib->admin_help,   sizeof(Cfg_master.lib->admin_help),   "Manage MASTER system" );

    if (!Cfg_master.enable)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread not enable in config. Shuting Donwn %d", pthread_self() );
       goto end;
     }

    Cfg_master.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */

    Abonner_distribution_message ( Master_Gerer_message );      /* Abonnement à la diffusion des messages */
    Abonner_distribution_sortie  ( Master_Gerer_sortie );        /* Abonnement à la diffusion des sorties */

    while(Cfg_master.lib->Thread_run == TRUE)                            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_master.lib->Thread_sigusr1)                                /* A-t'on recu un signal USR1 ? */
        { int nbr_msg, nbr_sortie;

          Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
          pthread_mutex_lock( &Cfg_master.lib->synchro );     /* On recupere le nombre de msgs en attente */
          nbr_msg    = g_slist_length(Cfg_master.Liste_message);
          nbr_sortie = g_slist_length(Cfg_master.Liste_sortie);
          pthread_mutex_unlock( &Cfg_master.lib->synchro );
          Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                    "Run_thread: In Queue : %d MSGS, %d A", nbr_msg, nbr_sortie );
          Cfg_master.lib->Thread_sigusr1 = FALSE;
        }

     }

    Desabonner_distribution_sortie  ( Master_Gerer_sortie ); /* Desabonnement de la diffusion des sorties */
    Desabonner_distribution_message ( Master_Gerer_message );/* Desabonnement de la diffusion des messages */
    Master_Liberer_config();                                  /* Liberation de la configuration du thread */

end:
    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_master.lib->TID = 0;                              /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
