/**********************************************************************************************************/
/* Watchdogd/lirc.c        Connexion à LIRC et gestion des commandes associées                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 22 août 2010 21:38:27 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * lirc.c
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

 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/prctl.h>
 #include <lirc/lirc_client.h>

 #include "watchdogd.h"
 #include "Lirc.h"

/**********************************************************************************************************/
/* Lirc_Lire_config : Lit la config Watchdog et rempli la structure mémoire                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Lirc_Lire_config ( void )
  { GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Lirc_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_lirc.lib->Thread_debug = g_key_file_get_boolean ( gkf, "LIRC", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Lirc_Liberer_config : Libere la mémoire allouer précédemment pour lire la config lirc                  */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Lirc_Liberer_config ( void )
  {
  }
/**********************************************************************************************************/
/* Admin_command : Fonction appelé lorsque le connexion envoi une commande d'admin pour LIRC                 */
/* Entrée: Le connexion d'admin et la ligne de commande                                                      */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_command ( struct CONNEXION *connexion, gchar *ligne )
  {
  }
/**********************************************************************************************************/
/* Run_lirc : Vérifie si le serveur a recu une commande IR                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: néant. Les bits DLS sont positionnés                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { 

    prctl(PR_SET_NAME, "W-Lirc", 0, 0, 0 );
    memset( &Cfg_lirc, 0, sizeof(Cfg_lirc) );                   /* Mise a zero de la structure de travail */
    Cfg_lirc.lib = lib;                        /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_lirc.lib->TID = pthread_self();                                 /* Sauvegarde du TID pour le pere */
    Lirc_Lire_config ();                                /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_lirc.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );
    Cfg_lirc.lib->Thread_run = TRUE;                                                /* Le thread tourne ! */

    g_snprintf( Cfg_lirc.lib->admin_prompt, sizeof(Cfg_lirc.lib->admin_prompt), "lirc" );
    g_snprintf( Cfg_lirc.lib->admin_help,   sizeof(Cfg_lirc.lib->admin_help),   "Manage InfraRed LIRC system" );

    if (lirc_readconfig ( ".lircrc", &Cfg_lirc.config, NULL)!=0)
     { Info_new( Config.log, Cfg_lirc.lib->Thread_debug, LOG_WARNING,
                "Run_lirc: Unable to read config... stopping...(%p)", pthread_self() );
       Cfg_lirc.lib->Thread_run = FALSE;
     }
    else if ( (Cfg_lirc.fd=lirc_init("Watchdogd",1))==-1)
     { Info_new( Config.log, Cfg_lirc.lib->Thread_debug, LOG_ERR,
                "Run_lirc: Unable to open LIRCD... stopping...(%p)", pthread_self() );
       Cfg_lirc.lib->Thread_run = FALSE;
     }
    else { fcntl ( Cfg_lirc.fd, F_SETFL, O_NONBLOCK ); }

    while(lib->Thread_run == TRUE)                                    /* On tourne tant que l'on a besoin */
     { gchar *code;
       gchar *c;
       gint ret;

       if (lib->Thread_sigusr1)                                                   /* On a recu sigusr1 ?? */
        { Info_new( Config.log, Cfg_lirc.lib->Thread_debug, LOG_NOTICE, "Run_lirc: SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       if (lirc_nextcode(&code)==0)                          /* Si un code est présent sur le socket lirc */
        { if(code!=NULL)
           { while( (ret=lirc_code2char(Cfg_lirc.config, code, &c))==0)        /* Tant qu'on a des char ! */
              { gint m;
                if (c == NULL) break;
                m = atoi (c);
		Info_new( Config.log, Cfg_lirc.lib->Thread_debug, LOG_INFO,
                         "Run_lirc: Recu commande %s : %s (M%03d)", code, c, m );
                Envoyer_commande_dls(m);
              }
             free(code);
             if(ret==-1) break;
           }
        }
       usleep(10000);
     }

    if (Cfg_lirc.config) lirc_freeconfig(Cfg_lirc.config);
    lirc_deinit();
    Lirc_Liberer_config();                                    /* Liberation de la configuration du thread */

    Info_new( Config.log, Cfg_lirc.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_lirc.lib->TID = 0;                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
