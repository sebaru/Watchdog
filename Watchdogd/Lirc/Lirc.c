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
       pthread_exit(GINT_TO_POINTER(0));
     }

    fcntl ( fd, F_SETFL, O_NONBLOCK );

    if (lirc_readconfig ( ".lircrc", &config, NULL)!=0)
     { Info_n( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: Unable to read config... stopping...", pthread_self() );
       pthread_exit(GINT_TO_POINTER(0));
     }

    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { gchar *code;
       gchar *c;
       gint ret;

       if (Partage->com_lirc.sigusr1)                                             /* On a recu sigusr1 ?? */
        { Partage->com_lirc.sigusr1 = FALSE;
          Info( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: SIGUSR1" );
          lirc_freeconfig(config);
          if (lirc_readconfig ( NULL, &config, NULL)!=0)
           { Info_n( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: Unable to read config... stopping...", pthread_self() );
             break;
           }
        }

       if (lirc_nextcode(&code)==0)                          /* Si un code est présent sur le socket lirc */
        { if(code!=NULL)
           { printf("LIRC ------ Code recu = %s\n", code );
             while( (ret=lirc_code2char(config,code,&c))==0)
              { gint m;
                if (c == NULL) break;
                printf(" c = %s\n", c );
                m = atoi (c);
		Info_n( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: Recu commande. Positionnement du monostable", m );
                Envoyer_commande_dls(m);
              }
             printf("LIRC ------ ret = %d , c = %p\n", ret, c );
             free(code);
             if(ret==-1) break;
           }
        }
       usleep(1000);
     }

    lirc_freeconfig(config);
    lirc_deinit();
    Info_n( Config.log, DEBUG_LIRC, "LIRC: Run_lirc: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
