/**********************************************************************************************************/
/* Watchdogd/master.c        Gestion des MASTER de Watchdog v2.0                                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                   ven. 02 avril 2010 20:37:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Sms.c
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
 #include <gnokii.h>
 #include <curl/curl.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Master.h"
 #define PREMASTER   "CDE:"

/**********************************************************************************************************/
/* Sms_Lire_config : Lit la config Watchdog et rempli la structure mémoire                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Sms_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;
#ifdef bouh
    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Sms_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_master.lib->Thread_debug = g_key_file_get_boolean ( gkf, "MASTER", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */
    chaine = g_key_file_get_string ( gkf, "MASTER", "masterbox_username", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                  "Sms_Lire_config: masterbox_username is missing. Using default." );
       g_snprintf( Cfg_master.masterbox_username, sizeof(Cfg_master.masterbox_username), DEFAUT_MASTERBOX_USERNAME );
     }
    else
     { g_snprintf( Cfg_master.masterbox_username, sizeof(Cfg_master.masterbox_username), "%s", chaine );
       g_free(chaine);
     }

    chaine = g_key_file_get_string ( gkf, "MASTER", "masterbox_password", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                  "Sms_Lire_config: masterbox_password is missing. Using default." );
       g_snprintf( Cfg_master.masterbox_password, sizeof(Cfg_master.masterbox_password), DEFAUT_MASTERBOX_PASSWORD );
     }
    else
     { g_snprintf( Cfg_master.masterbox_password, sizeof(Cfg_master.masterbox_password), "%s", chaine );
       g_free(chaine);
     }

    Cfg_master.recipients = g_key_file_get_string_list ( gkf, "MASTER", "recipients", NULL, NULL );
    if (!Cfg_master.recipients)
     { Info_new ( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                  "Sms_Lire_config: recipients are missing." );
     }

    g_key_file_free(gkf);
#endif
  }
/**********************************************************************************************************/
/* Sms_Liberer_config : Libere la mémoire allouer précédemment pour lire la config master                  */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Sms_Liberer_config ( void )
  {
 #ifdef bouh
g_strfreev ( Cfg_master.recipients );
#endif
  }
/**********************************************************************************************************/
/* Envoyer_master: Envoi un master                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_master ( struct CMD_TYPE_MESSAGE *msg )
  { struct CMD_TYPE_MESSAGE *copie;
    gint nbr;
#ifdef bouh
    pthread_mutex_lock( &Cfg_master.lib->synchro );      /* On recupere le nombre de master en attente */
    nbr = g_slist_length(Cfg_master.Liste_master);
    pthread_mutex_unlock( &Cfg_master.lib->synchro );

    if (nbr > 50)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Envoyer_master: liste d'attente pleine" );
       return;
     }

    copie = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!copie) { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                 "Envoyer_master: pas assez de mémoire pour copie" );
                  return;
                }
    memcpy ( copie, msg, sizeof(struct CMD_TYPE_MESSAGE) );

    pthread_mutex_lock( &Cfg_master.lib->synchro );
    Cfg_master.Liste_master = g_slist_append ( Cfg_master.Liste_master, copie );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );
#endif
  }
/**********************************************************************************************************/
/* Envoyer_master: Envoi un master                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_master_masterbox_text ( gchar *texte )
  { struct CMD_TYPE_MESSAGE *msg;
#ifdef bouh
    msg = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg) { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                         "Envoyer_master_masterbox_text: pas assez de mémoire pour copie" );
                return;
              }

    g_snprintf(msg->libelle_master, sizeof(msg->libelle_master), "%s", texte );
    msg->id  = 0;
    msg->num = 0;
    msg->enable = TRUE;
    msg->master = MSG_MASTER_MASTERBOX;

    Envoyer_master ( msg );
#endif
  }
/**********************************************************************************************************/
/* Sms_Gerer_message: Fonction d'abonné appellé lorsqu'un message est disponible.                         */
/* Entrée: une structure CMD_TYPE_HISTO                                                                   */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Sms_Gerer_message ( struct CMD_TYPE_MESSAGE *msg )
  { gint taille;
#ifdef bouh
    if ( ! msg->master ) { g_free(msg); return; }                           /* Si flag = 0; on return direct */

    pthread_mutex_lock( &Cfg_master.lib->synchro );                         /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_master.Liste_master );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Sms_Gerer_message: DROP message %D (length = %d > 150)", msg->num, taille);
       g_free(msg);
       return;
     }

    pthread_mutex_lock ( &Cfg_master.lib->synchro );
    Cfg_master.Liste_master = g_slist_append ( Cfg_master.Liste_master, msg );                    /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_master.lib->synchro );
#endif
  }
/**********************************************************************************************************/
/* Envoyer_master: Envoi un master                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct CMD_TYPE_MESSAGE *msg;
    GSList *Liste_master;
#ifdef bouh    
    prctl(PR_SET_NAME, "W-MASTER", 0, 0, 0 );
    memset( &Cfg_master, 0, sizeof(Cfg_master) );                     /* Mise a zero de la structure de travail */
    Cfg_master.lib = lib;                         /* Sauvegarde de la structure pointant sur cette librairie */
    Sms_Lire_config ();                                 /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    Cfg_master.lib->Thread_run = TRUE;                                                 /* Le thread tourne ! */

    g_snprintf( Cfg_master.lib->admin_prompt, sizeof(Cfg_master.lib->admin_prompt), "master" );
    g_snprintf( Cfg_master.lib->admin_help,   sizeof(Cfg_master.lib->admin_help),   "Manage MASTER system" );

    Abonner_distribution_message ( Sms_Gerer_message );               /* Abonnement à la diffusion des messages */

    while(Cfg_master.lib->Thread_run == TRUE)                               /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_master.lib->Thread_sigusr1)                                   /* A-t'on recu un signal USR1 ? */
        { int nbr;

          Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
          pthread_mutex_lock( &Cfg_master.lib->synchro );         /* On recupere le nombre de master en attente */
          nbr = g_slist_length(Cfg_master.Liste_master);
          pthread_mutex_unlock( &Cfg_master.lib->synchro );
          Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Run_thread: Nbr MASTER a envoyer = %d", nbr );
          Cfg_master.lib->Thread_sigusr1 = FALSE;
        }

/********************************************** Lecture de MASTER ********************************************/
       Lire_master_gsm();

/************************************************ Envoi de MASTER ********************************************/
       if ( !Cfg_master.Liste_master )                                        /* Attente de demande d'envoi MASTER */
        { sleep(5);
          sched_yield();
          continue;
        }

       pthread_mutex_lock( &Cfg_master.lib->synchro );
       Liste_master = Cfg_master.Liste_master;                         /* Sauvegarde du ptr master a envoyer */
       msg = Liste_master->data;
       pthread_mutex_unlock( &Cfg_master.lib->synchro );
       Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                "Run_thread : Sending msg %d (%s)", msg->num, msg->libelle_master );
      
/**************************************** Envoi en mode GSM ***********************************************/

       if (Partage->top < TOP_MIN_ENVOI_MASTER)
        { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                   "Envoi_master_gsm: Envoi trop tot !! (%s)", msg->libelle_master ); }
       else 
        { gchar **liste;
          gint cpt = 0;
          liste = Cfg_master.recipients;
          while (liste[cpt])
           { if (msg->master == MSG_MASTER_GSM)    Envoi_master_gsm   ( msg, liste[cpt] );
             if (msg->master == MSG_MASTER_MASTERBOX) Envoi_master_masterbox( msg, liste[cpt] );
             cpt++;
           }
        }

       pthread_mutex_lock( &Cfg_master.lib->synchro );
       Cfg_master.Liste_master = g_slist_remove ( Cfg_master.Liste_master, msg );
       Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Run_thread: Reste %d a envoyer",
                 g_slist_length(Cfg_master.Liste_master) );
       pthread_mutex_unlock( &Cfg_master.lib->synchro );
       g_free( msg );
     }

    Desabonner_distribution_message ( Sms_Gerer_message );  /* Desabonnement de la diffusion des messages */
    Sms_Liberer_config();                                     /* Liberation de la configuration du thread */

    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_master.lib->TID = 0;                                 /* On indique au master que le thread est mort. */
#endif
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
