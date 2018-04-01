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
/* New_zmq: Initialise une socket dont le pattern et le nom sont en parametre                                                 */
/* Entrée: le pattern                                                                                                         */
/* Sortie: une socket ZMQ ou NUL si erreur                                                                                    */
/******************************************************************************************************************************/
 struct ZMQUEUE *New_zmq ( gint pattern, gchar *name )
  { struct ZMQUEUE *zmq;
    zmq = (struct ZMQUEUE *)g_try_malloc0( sizeof(struct ZMQUEUE) );
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
/* Subscribe_zmq: Souscris au topic en parametre                                                                              */
/* Entrée: la queue, le topic                                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Subscribe_zmq ( struct ZMQUEUE *zmq )
  { if ( zmq_setsockopt ( zmq->socket, ZMQ_SUBSCRIBE, NULL, 0 ) == -1 )                          /* Subscribe to all messages */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "%s: ZMQ subscript to all for '%s' failed (%s)",
                 __func__, zmq->name, zmq_strerror(errno) );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ subscript for '%s' OK", __func__, zmq->name );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Bind_zmq: Bind la socket en parametre                                                                                      */
/* Entrée: le type, le nom et le port                                                                                         */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Bind_zmq ( struct ZMQUEUE *zmq, gchar *type, gchar *nom, gint port )
  { if (port) g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s:%d", type, nom, port );
         else g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s", type, nom );
    if ( zmq_bind (zmq->socket, zmq->endpoint) == -1 ) 
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "%s: ZMQ Bind '%s' to '%s' Failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ Bind '%s' to '%s' OK", __func__, zmq->name, zmq->endpoint );
    if (zmq->pattern == ZMQ_SUB) return(Subscribe_zmq ( zmq ));
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Bind_zmq: Bind la socket en parametre                                                                                      */
/* Entrée: le type, le nom et le port                                                                                         */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Connect_zmq ( struct ZMQUEUE *zmq, gchar *type, gchar *nom, gint port )
  { if (port) g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s:%d", type, nom, port );
         else g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s", type, nom );
    if ( zmq_connect (zmq->socket, zmq->endpoint) == -1 ) 
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "%s: ZMQ Connect '%s' to '%s' Failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ Connect '%s' to '%s' OK", __func__, zmq->name, zmq->endpoint );
    if (zmq->pattern == ZMQ_SUB) return(Subscribe_zmq ( zmq ));
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Close_zmq: Ferme une socket ZMQ                                                                                            */
/* Entrée: la queue                                                                                                           */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Close_zmq ( struct ZMQUEUE *zmq )
  { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
              "%s: ZMQ closing '%s'", __func__, zmq->name );
    zmq_close ( zmq->socket );
    g_free(zmq);
  }
/******************************************************************************************************************************/
/* Send_zmq: Envoie un message dans la socket                                                                                 */
/* Entrée: la socket, le message, sa longueur                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Send_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille )
  { if (zmq_send( zmq->socket, buf, taille, 0 ) == -1)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "%s: Send to ZMQ '%s' ('%s') failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: Send %d bytes to ZMQ '%s' ('%s') OK", __func__, taille, zmq->name, zmq->endpoint );
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Send_zmq_with_tag: Envoie un message dans la socket avec le tag en prefixe                                                 */
/* Entrée: la socket, le tag, le message, sa longueur                                                                         */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Send_zmq_with_tag ( struct ZMQUEUE *zmq, gint tag, gchar *target_instance, gchar *target_thread, void *source, gint taille )
  { struct MSRV_EVENT event;
    void *buffer;
    gboolean retour;
    buffer = g_try_malloc( taille + sizeof(struct MSRV_EVENT) );
    if (!buffer)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "%s: Send to ZMQ '%s' ('%s') failed (Memory Error)", __func__, zmq->name, zmq->endpoint );
       return(FALSE);
     }
    
    event.tag = tag;
    if (target_instance) g_snprintf( event.instance, sizeof(event.instance), target_instance );
                    else g_snprintf( event.instance, sizeof(event.instance), "*" );
    if (target_thread) g_snprintf( event.thread, sizeof(event.thread), target_thread);
                  else g_snprintf( event.thread, sizeof(event.thread), "*" );

    memcpy ( buffer, &event, sizeof(struct MSRV_EVENT) );                                                   /* Recopie entete */
    memcpy ( buffer + sizeof(struct MSRV_EVENT), source, taille );                                  /* Recopie buffer payload */
    retour = Send_zmq( zmq, buffer, taille + sizeof(struct MSRV_EVENT) );
    g_free(buffer);
    if (retour==FALSE)    
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "%s: Send to ZMQ '%s' ('%s') failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: Send %d bytes to ZMQ '%s' ('%s') OK", __func__, taille, zmq->name, zmq->endpoint );
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Recv_zmq: Receptionne un message sur le file en paremetre (sans attendre)                                                  */
/* Entrée: la file, le buffer d'accueil, la taille du buffer                                                                  */
/* Sortie: Nombre de caractere lu, -1 si erreur                                                                               */
/******************************************************************************************************************************/
 gint Recv_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille_buf )
  { gint byte;
    byte = zmq_recv ( zmq->socket, buf, taille_buf, ZMQ_DONTWAIT );
    if (byte>0)
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: Recv %d bytes from ZMQ '%s' ('%s')", __func__, byte, zmq->name, zmq->endpoint );
     }
    return(byte);
  }
/******************************************************************************************************************************/
/* Recv_zmq: Receptionne un message sur le file en paremetre (sans attendre)                                                  */
/* Entrée: la file, le buffer d'accueil, la taille du buffer                                                                  */
/* Sortie: Nombre de caractere lu, -1 si erreur                                                                               */
/******************************************************************************************************************************/
 gint Recv_zmq_block ( struct ZMQUEUE *zmq, void *buf, gint taille_buf )
  { gint byte;
    byte = zmq_recv ( zmq->socket, buf, taille_buf, 0 );
    if (byte>0)
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: Recv %d bytes from ZMQ '%s' ('%s')", __func__, byte, zmq->name, zmq->endpoint );
     }
    else 
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "%s: Error for ZMQ '%s' ('%s'): %s", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
     }
    return(byte);
  }
/******************************************************************************************************************************/
/* Recv_zmq: Receptionne un message sur le file en paremetre (sans attendre)                                                  */
/* Entrée: la file, le buffer d'accueil, la taille du buffer                                                                  */
/* Sortie: Nombre de caractere lu, -1 si erreur                                                                               */
/******************************************************************************************************************************/
 gint Recv_zmq_with_tag ( struct ZMQUEUE *zmq, void *buf, gint taille_buf, struct MSRV_EVENT **event, void **payload )
  { gint byte;
    byte = zmq_recv ( zmq->socket, buf, taille_buf, ZMQ_DONTWAIT );
    if (byte>=0)
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: Recv %d bytes from ZMQ '%s' ('%s')", __func__, byte, zmq->name, zmq->endpoint );
       *event = buf;
       *payload = buf+sizeof(struct MSRV_EVENT);
     }
    return(byte);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
