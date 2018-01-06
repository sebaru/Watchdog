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
 struct ZMQUEUE *New_zmq_socket ( gint pattern, gchar *name )
  { struct ZMQUEUE *zmq;
    zmq = (struct ZMQUEUE *)g_malloc0( sizeof(struct ZMQUEUE) );
    if (!zmq)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: New ZMQ Socket '%s' Failed. Memory Error (%s)",
                 __func__, name, zmq_strerror(errno) );
       return(NULL);
     }
    
    zmq->pattern = pattern;
    g_snprintf( zmq->name, sizeof(zmq->name), "%s", name );
    if ( (zmq->socket = zmq_socket ( Partage->zmq_ctx, pattern )) == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "%s: New ZMQ Socket '%s' Failed (%s)", __func__, name, zmq_strerror(errno) );
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: New ZMQ Socket '%s' OK", __func__, name );
    return(zmq);
  }
/******************************************************************************************************************************/
/* Bind_zmq_socket: Bind la socket en parametre                                                                               */
/* Entrée: le type, le nom et le port                                                                                         */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Bind_zmq_socket ( struct ZMQUEUE *zmq, gchar *type, gchar *nom, gint port )
  { g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s:%d", type, nom, port );
    if ( zmq_bind (zmq->socket, zmq->endpoint) == -1 ) 
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "%s: ZMQ Bind '%s' to '%s' Failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ Bind '%s' to '%s' OK", __func__, zmq->name, zmq->endpoint );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Bind_zmq_socket: Bind la socket en parametre                                                                               */
/* Entrée: le type, le nom et le port                                                                                         */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Connect_zmq_socket ( struct ZMQUEUE *zmq, gchar *type, gchar *nom, gint port )
  { g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s:%d", type, nom, port );
    if ( zmq_connect (zmq->socket, zmq->endpoint) == -1 ) 
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "%s: ZMQ Connect '%s' to '%s' Failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ Connect '%s' to '%s' OK", __func__, zmq->name, zmq->endpoint );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Subscribe_zmq_socket: Souscris au topic en parametre                                                                       */
/* Entrée: la queue, le topic                                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Subscribe_zmq_socket ( struct ZMQUEUE *zmq, gchar *topic )
  { if ( zmq_setsockopt ( zmq->socket, ZMQ_SUBSCRIBE, topic, strlen(topic) ) == -1 )             /* Subscribe to all messages */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "%s: ZMQ subscript '%s' to '%s' failed (%s)",
                 __func__, zmq->name, topic, zmq_strerror(errno) );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ subscript '%s' to '%s' OK", __func__, zmq->name, topic );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Close_zmq_socket: Ferme une socket ZMQ                                                                                     */
/* Entrée: la queue                                                                                                           */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Close_zmq_socket ( struct ZMQUEUE *zmq )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
              "%s: ZMQ closing '%s'", __func__, zmq->name );
    zmq_close ( zmq->socket );
    g_free(zmq);
  }
/******************************************************************************************************************************/
/* Send_zmq_socket: Envoie un message dans la socket                                                                          */
/* Entrée: le type de message, le message, sa longueur                                                                        */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Send_zmq_socket ( struct ZMQUEUE *zmq, void *buf, gint taille )
  { if (zmq_send( zmq->socket, buf, taille, 0 ) == -1)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "%s: Send to ZMQ '%s' ('%s')failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: Send %d bytes to ZMQ '%s' ('%s') OK", __func__, taille, zmq->name, zmq->endpoint );
     }
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
