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
 #include <lirc/lirc_client.h>

 #include "watchdogd.h"

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */

/**********************************************************************************************************/
/* Camera_check_motion : Vérifie si l'outil motion a donner un bit a activer                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: néant. Les bits DLS sont positionnés                                                           */
/**********************************************************************************************************/
 void Lirc_check ( struct LOG *log, struct DB *db )
  { struct lirc_config *config;
    gint fd;

    if ( (fd=lirc_init("Watchdogd",1))==-1) return;
    fcntl ( fd, F_SETFL, O_NONBLOCK );

    if (lirc_readconfig ( NULL, &config, NULL)==0)
     { gchar *code;
       gchar *c;
       gint ret;

       while(lirc_nextcode(&code)==0)
        { if(code==NULL) continue;
          while( (ret=lirc_code2char(config,code,&c))==0 &&
                 c!=NULL)
           {
				printf("Execing command \"%s\"\n",c);
           }
          free(code);
          if(ret==-1) break;
        }
       lirc_freeconfig(config);
     }
    lirc_deinit();
  }
/*--------------------------------------------------------------------------------------------------------*/
