/**********************************************************************************************************/
/* Watchdogd/ecoute.c        Gestion des connexions au serveur watchdog                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 03 jun 2003 11:09:03 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ecoute.c
 * This file is part of Watchdogd
 *
 * Copyright (C) 2009 - 
 *
 * Watchdogd is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdogd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdogd; if not, write to the Free Software
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

 #include "Erreur.h"
 #include "Config.h"

 extern struct CONFIG Config;

 #include "prototype.h"                                      /* Mise en place des prototypes de fonctions */

/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: -1 si erreur                                                                                   */
/**********************************************************************************************************/
 gint Activer_ecoute ( void )
  { struct sockaddr_in local;
    gint opt, ecoute;

    signal( SIGPIPE, SIG_IGN );
    if ( (ecoute = socket ( AF_INET, SOCK_STREAM, 0 )) == -1)                           /* Protocol = TCP */
     { Info_c( Config.log, DEBUG_NETWORK, "Socket failure...", strerror(errno) ); return(-1); }

    opt = 1;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_REUSEADDR | SO_KEEPALIVE,
                     (char*)&opt, sizeof(opt) ) == -1 )
     { Info_c( Config.log, DEBUG_NETWORK, "Set option failed", strerror(errno) ); return(-1); }

    opt = 16834;
    if ( setsockopt( ecoute, SOL_SOCKET, SO_SNDBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_c( Config.log, DEBUG_NETWORK, "SO_SNDBUF failed", strerror(errno) ); return(-1); }
    if ( setsockopt( ecoute, SOL_SOCKET, SO_RCVBUF,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_c( Config.log, DEBUG_NETWORK, "SO_RCVBUF failed", strerror(errno) ); return(-1); }

    opt = 1;
    if ( setsockopt( ecoute, SOL_TCP, TCP_NODELAY,(char*)&opt, sizeof(opt) ) == -1 )
     { Info_c( Config.log, DEBUG_NETWORK, "TCP_NODELAY failed", strerror(errno) ); return(-1); }

    memset( &local, 0, sizeof(local) );
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(Config.port);                      /* Attention: en mode network, pas host !!! */
    if (bind( ecoute, (struct sockaddr *)&local, sizeof(local)) == -1)
     { Info_c( Config.log, DEBUG_NETWORK, "Bind failure...", strerror(errno) );
       close(ecoute);
       return(-1);
     }

    if (listen(ecoute, 1) == -1)                                       /* On demande d'écouter aux portes */
     { Info_c( Config.log, DEBUG_NETWORK, "Listen failure...", strerror(errno));
       close(ecoute);
       return(-1);
     }
    fcntl( ecoute, F_SETFL, O_NONBLOCK );        /* Mode non bloquant, ça aide pour une telle application */
    Info_n( Config.log, DEBUG_NETWORK, "Ecoute du port", Config.port );
    Info_n( Config.log, DEBUG_NETWORK, "        socket", ecoute );
    return( ecoute );                                                            /* Tout s'est bien passé */
  }
/*--------------------------------------------------------------------------------------------------------*/
