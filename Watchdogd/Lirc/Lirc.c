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

 #include <glib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/prctl.h>
 #include <lirc/lirc_client.h>

 #include "watchdogd.h"

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */

/**********************************************************************************************************/
/* Run_lirc : Vérifie si le serveur a recu une commande IR                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: néant. Les bits DLS sont positionnés                                                           */
/**********************************************************************************************************/
 void Run_lirc ( void )
  { struct lirc_config *config;
    guint fd;
    prctl(PR_SET_NAME, "W-Lirc", 0, 0, 0 );

    Info( Config.log, DEBUG_LIRC, "LIRC: demarrage" );

    if ( (fd=lirc_init("Watchdogd",1))==-1)
     { Info_n( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: Unable to open LIRCD... stopping...", pthread_self() );
       Partage->com_lirc.TID = 0;                         /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(0));
     }

    fcntl ( fd, F_SETFL, O_NONBLOCK );

    if (lirc_readconfig ( ".lircrc", &config, NULL)!=0)
     { Info_n( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: Unable to read config... stopping...", pthread_self() );
       Partage->com_lirc.TID = 0;                         /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(0));
     }

    Partage->com_lirc.Thread_run = TRUE;                         /* On dit au maitre que le thread tourne */
    while(Partage->com_lirc.Thread_run == TRUE)                       /* On tourne tant que l'on a besoin */
     { gchar *code;
       gchar *c;
       gint ret;

       if (Partage->com_lirc.Thread_reload)                                           /* On a recu RELOAD */
        { Info( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: RELOAD" );
          lirc_freeconfig(config);
          if (lirc_readconfig ( NULL, &config, NULL)!=0)
           { config = NULL;
             Info_n( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: Unable to read config... stopping...", pthread_self() );
             Partage->com_lirc.Thread_run = FALSE;                        /* On demande l'arret du thread */
             break;
           }
          Partage->com_lirc.Thread_reload = FALSE;
        }

       if (Partage->com_lirc.Thread_sigusr1)                                      /* On a recu sigusr1 ?? */
        { Info( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: SIGUSR1" );
          Partage->com_lirc.Thread_sigusr1 = FALSE;
        }

       if (lirc_nextcode(&code)==0)                          /* Si un code est présent sur le socket lirc */
        { if(code!=NULL)
           { while( (ret=lirc_code2char(config,code,&c))==0)
              { gint m;
                if (c == NULL) break;
                m = atoi (c);
		Info_c( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: Recu commande", code );
		Info_c( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: -------------", c );
                Envoyer_commande_dls(m);
              }
             free(code);
             if(ret==-1) break;
           }
        }
       usleep(1000);
     }

    if (config) lirc_freeconfig(config);
    lirc_deinit();
    Info_n( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: Down", pthread_self() );
    Partage->com_lirc.TID = 0;                            /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
