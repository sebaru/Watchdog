/******************************************************************************************************************************/
/* Watchdogd/zmq.c        Gestion des echanges ZMQ                                                                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    06.01.2018 11:42:29 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * zmq.c
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

 #include "watchdogd.h"
/******************************************************************************************************************************/
/* New_zmq_socket: Initialise une socket dont le pattern                                                                      */
/* Entrée: le pattern                                                                                                         */
/* Sortie: une socket ZMQ ou NUL si erreur                                                                                    */
/******************************************************************************************************************************/
 void *New_zmq_socket ( gint pattern )
  { void *socket;

    if ( (socket = zmq_socket ( Partage->zmq_ctx, pattern )) == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: New ZMQ Socket Failed (%s)", __func__, zmq_strerror(errno) );
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: New ZMQ Socket '%s' MSG OK", __func__ );
    return(socket);
  }
/******************************************************************************************************************************/
/* Bind_zmq_socket: Bind la socket en parametre                                                                               */
/* Entrée: le type, le nom et le port                                                                                         */
/* Sortie: une socket ZMQ ou NUL si erreur                                                                                    */
/******************************************************************************************************************************/
 gboolean Bind_zmq_socket ( void *socket, gchar *type, gchar *nom, gint port )
  { gchar endpoint[80];

    if (!strcmp(type, "inproc"))
     { g_snprintf( endpoint, sizeof(endpoint), "%s://%s", type, nom ); }
    else
     { g_snprintf( endpoint, sizeof(endpoint), "%s://%s:%d", type, nom, port ); }
    if ( zmq_bind (socket, endpoint) == -1 ) 
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: ZMQ Bind '%s' Failed (%s)", __func__,
                                                       endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ Bind '%s' OK", __func__, endpoint );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Bind_zmq_socket: Bind la socket en parametre                                                                               */
/* Entrée: le type, le nom et le port                                                                                         */
/* Sortie: une socket ZMQ ou NUL si erreur                                                                                    */
/******************************************************************************************************************************/
 gboolean Connect_zmq_socket ( void *socket, gchar *type, gchar *nom, gint port )
  { gchar endpoint[80];

    if (!strcmp(type, "inproc"))
     { g_snprintf( endpoint, sizeof(endpoint), "%s://%s", type, nom ); }
    else
     { g_snprintf( endpoint, sizeof(endpoint), "%s://%s:%d", type, nom, port ); }
    if ( zmq_connect (socket, endpoint) == -1 ) 
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: ZMQ Connect '%s' Failed (%s)", __func__,
                                                       endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ Connect '%s' OK", __func__, endpoint );
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
