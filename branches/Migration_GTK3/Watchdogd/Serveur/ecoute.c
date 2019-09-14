/**********************************************************************************************************/
/* Watchdogd/ecoute.c        Gestion des connexions au serveur watchdog                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                      mar 03 jun 2003 11:09:03 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ecoute.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien LEFEVRE
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
 #include <stdlib.h>
 #include <string.h>
 #include <sys/socket.h>
 #include <netinet/tcp.h>
 #include <sys/wait.h>
 #include <netinet/in.h>                                          /* Pour les structures d'entrées SOCKET */
 #include <fcntl.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <errno.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: -1 si erreur                                                                                   */
/**********************************************************************************************************/
 gint Activer_ecoute ( void )
  { struct sockaddr_in local;
    gint opt, ecoute;

    if ( (ecoute = socket ( AF_INET, SOCK_STREAM, 0 )) == -1)                           /* Protocol = TCP */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "Socket failure (%s)", strerror(errno) ); return(-1); }

    opt = 1;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE,
                     (char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "Set option failed (%s)", strerror(errno) ); return(-1); }

    opt = 16834;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_SNDBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "SO_SNDBUF failed (%s)", strerror(errno) ); return(-1); }
    if ( setsockopt( ecoute, SOL_SOCKET, SO_RCVBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "SO_RCVBUF failed (%s)", strerror(errno) ); return(-1); }

    opt = 1;
    if ( setsockopt( ecoute, SOL_TCP, TCP_NODELAY,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "TCP_NODELAY failed (%s)", strerror(errno) ); return(-1); }

    memset( &local, 0, sizeof(local) );
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(Cfg_ssrv.port);                      /* Attention: en mode network, pas host !!! */
    if (bind( ecoute, (struct sockaddr *)&local, sizeof(local)) == -1)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "Bind failure (%s)", strerror(errno) );
       close(ecoute);
       return(-1);
     }

    if (listen(ecoute, 1) == -1)                                       /* On demande d'écouter aux portes */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "Listen failure (%s)", strerror(errno));
       close(ecoute);
       return(-1);
     }
    fcntl( ecoute, F_SETFL, O_NONBLOCK );        /* Mode non bloquant, ça aide pour une telle application */
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
              "Ecoute du port %d with socket %d", Cfg_ssrv.port, ecoute );
    return( ecoute );                                                            /* Tout s'est bien passé */
  }
/*--------------------------------------------------------------------------------------------------------*/
