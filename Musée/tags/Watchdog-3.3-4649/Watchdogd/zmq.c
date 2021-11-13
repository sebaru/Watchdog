/******************************************************************************************************************************/
/* Watchdogd/zmq.c        Gestion des echanges ZMQ                                                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    06.01.2018 11:42:29 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * zmq.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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
/* Zmq_instance_is_target: Renvoie TRUE si l'instance actuelle est visée par l'evenement en parametre                         */
/* Entrée: l'evenement                                                                                                        */
/* Sortie: TRUE ou FALSE                                                                                                      */
/******************************************************************************************************************************/
 gboolean Zmq_instance_is_target ( struct ZMQ_TARGET *event )
  { gchar *hostname, *target;
    gboolean retour;

    if (!strcmp( event->dst_instance, "*" )) return(TRUE);

    hostname = g_ascii_strdown ( g_get_host_name(), -1 );
    target   = g_ascii_strdown ( event->dst_instance, -1 );
    retour   = g_str_has_prefix ( hostname, target );
    g_free(hostname);
    g_free(target);
    return(retour);
  }
/******************************************************************************************************************************/
/* Zmq_instance_is_target: Renvoie TRUE si l'instance actuelle est visée par l'evenement en parametre                         */
/* Entrée: l'evenement                                                                                                        */
/* Sortie: TRUE ou FALSE                                                                                                      */
/******************************************************************************************************************************/
 static gboolean Zmq_thread_is_target ( struct ZMQ_TARGET *event, const gchar *thread )
  { if (thread==NULL) return(TRUE);
    if (!strcmp( event->dst_thread, "*" )) return(TRUE);
    if (!strcmp( event->dst_thread, thread )) return(TRUE);
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Zmq_instance_is_target: Renvoie TRUE si l'instance actuelle est visée par l'evenement en parametre                         */
/* Entrée: l'evenement                                                                                                        */
/* Sortie: TRUE ou FALSE                                                                                                      */
/******************************************************************************************************************************/
 gboolean Zmq_other_is_target ( struct ZMQ_TARGET *event )
  { if (!strcmp( event->dst_instance, "*" )) return(TRUE);
    return(!Zmq_instance_is_target(event));
  }
/******************************************************************************************************************************/
/* New_zmq: Initialise une socket dont le pattern et le endpoint sont en parametre                                            */
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
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: ZMQ subscript to all for '%s' failed (%s)",
                 __func__, zmq->name, zmq_strerror(errno) );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ subscribe for '%s' OK", __func__, zmq->name );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Bind_zmq: Bind la socket en parametre                                                                                      */
/* Entrée: le type, le endpoint et le port                                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 struct ZMQUEUE *Bind_zmq ( gint pattern, gchar *name, gchar *type, gchar *endpoint, gint port )
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
       g_free(zmq);
       return(NULL);
     }

    if (port) g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s:%d", type, endpoint, port );
         else g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s", type, endpoint );
    if ( zmq_bind (zmq->socket, zmq->endpoint) == -1 )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "%s: ZMQ Bind '%s' to '%s' Failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       g_free(zmq);
       return(NULL);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ Bind '%s' to '%s' OK", __func__, zmq->name, zmq->endpoint );
    if (zmq->pattern == ZMQ_SUB) Subscribe_zmq(zmq);
    return(zmq);
  }
/******************************************************************************************************************************/
/* Bind_zmq: Bind la socket en parametre                                                                                      */
/* Entrée: le type, le endpoint et le port                                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 struct ZMQUEUE *Connect_zmq ( gint pattern, gchar *name, gchar *type, gchar *endpoint, gint port )
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
       g_free(zmq);
       return(NULL);
     }

    if (port) g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s:%d", type, endpoint, port );
         else g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s", type, endpoint );
    if ( zmq_connect (zmq->socket, zmq->endpoint) == -1 )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "%s: ZMQ Connect '%s' to '%s' Failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       g_free(zmq);
       return(NULL);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ Connect '%s' to '%s' OK", __func__, zmq->name, zmq->endpoint );
    if (zmq->pattern == ZMQ_SUB) Subscribe_zmq(zmq);
    return(zmq);
  }
/******************************************************************************************************************************/
/* Close_zmq: Ferme une socket ZMQ                                                                                            */
/* Entrée: la queue                                                                                                           */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Close_zmq ( struct ZMQUEUE *zmq )
  { if (!zmq) return;
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: ZMQ closing '%s'", __func__, zmq->name );
    zmq_close ( zmq->socket );
    g_free(zmq);
  }
/******************************************************************************************************************************/
/* Send_zmq: Envoie un message dans la socket                                                                                 */
/* Entrée: la socket, le message, sa longueur                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Send_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille )
  { if (!zmq) return(FALSE);
    if (zmq_send( zmq->socket, buf, taille, 0 ) == -1)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "%s: Send to ZMQ '%s' ('%s') failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
             "%s: Send to ZMQ '%s' ('%s') %d bytes", __func__, zmq->name, zmq->endpoint, taille );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Send_zmq_with_tag: Envoie un message dans la socket avec le tag en prefixe                                                 */
/* Entrée: la socket, le tag, le message, sa longueur                                                                         */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Send_zmq_with_tag ( struct ZMQUEUE *zmq,
                              const gchar *source_instance, const gchar *source_thread,
                              const gchar *target_instance, const gchar *target_thread,
                              const gchar *target_tag, void *source, gint taille )
  { struct ZMQ_TARGET event;
    void *buffer;
    gboolean retour;
    if (!zmq) return(FALSE);
    if (taille==-1) taille = strlen(source)+1;
    buffer = g_try_malloc( taille + sizeof(struct ZMQ_TARGET) );
    if (!buffer)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "%s: Send to ZMQ '%s' ('%s') failed (Memory Error)", __func__, zmq->name, zmq->endpoint );
       return(FALSE);
     }

    g_snprintf( event.src_instance, sizeof(event.src_instance), g_get_host_name() );
    if (source_instance) g_snprintf( event.src_instance, sizeof(event.src_instance), source_instance);
                    else g_snprintf( event.src_instance, sizeof(event.src_instance), g_get_host_name() );
    if (source_thread) g_snprintf( event.src_thread, sizeof(event.src_thread), source_thread);
                  else g_snprintf( event.src_thread, sizeof(event.src_thread), "*" );
    if (target_instance) g_snprintf( event.dst_instance, sizeof(event.dst_instance), target_instance );
                    else g_snprintf( event.dst_instance, sizeof(event.dst_instance), "*" );
    if (target_thread) g_snprintf( event.dst_thread, sizeof(event.dst_thread), target_thread);
                  else g_snprintf( event.dst_thread, sizeof(event.dst_thread), "*" );
    if (target_tag)    g_snprintf( event.tag, sizeof(event.tag), target_tag);
                  else g_snprintf( event.tag, sizeof(event.tag), "none" );

    memcpy ( buffer, &event, sizeof(struct ZMQ_TARGET) );                                                   /* Recopie entete */
    memcpy ( buffer + sizeof(struct ZMQ_TARGET), source, taille );                                  /* Recopie buffer payload */
    retour = Send_zmq( zmq, buffer, taille + sizeof(struct ZMQ_TARGET) );
    g_free(buffer);
    if (retour==FALSE)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                "%s: '%s' ('%s') : ERROR SENDING %s/%s -> %s/%s/%s", __func__, zmq->name, zmq->endpoint,
                 event.src_instance, event.src_thread, event.dst_instance, event.dst_thread, event.tag );
       return(FALSE);
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "%s: '%s' ('%s') : SENDING %s/%s -> %s/%s/%s", __func__, zmq->name, zmq->endpoint,
                 event.src_instance, event.src_thread, event.dst_instance, event.dst_thread, event.tag );
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
    return(byte);
  }
/******************************************************************************************************************************/
/* Recv_zmq: Receptionne un message sur le file en paremetre (sans attendre)                                                  */
/* Entrée: la file, le buffer d'accueil, la taille du buffer                                                                  */
/* Sortie: Nombre de caractere lu, -1 si erreur                                                                               */
/******************************************************************************************************************************/
 gint Recv_zmq_with_tag ( struct ZMQUEUE *zmq, const gchar *thread, void *buf, gint taille_buf,
                          struct ZMQ_TARGET **event, void **payload )
  { gint byte;
    byte = zmq_recv ( zmq->socket, buf, taille_buf-1, ZMQ_DONTWAIT );
    if (byte>=0)
     { *event = buf;
       if (Zmq_instance_is_target ( *event ) && Zmq_thread_is_target ( *event, thread ) )
        { *payload = buf+sizeof(struct ZMQ_TARGET);
          Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                   "%s: '%s' ('%s') : %s/%s -> %s/%s/%s", __func__, zmq->name, zmq->endpoint,
                   (*event)->src_instance, (*event)->src_thread, (*event)->dst_instance, (*event)->dst_thread, (*event)->tag );
          ((gchar *)buf)[byte] = 0;                                                                       /* Caractere nul d'arret forcé */
          return(byte);
        }
       else return(0);                                                                        /* Si pas destinataire, on drop */
     }
    return(byte);
  }
/******************************************************************************************************************************/
/* Smsg_send_status_to_master: Envoie le bit de comm au master selon le status du GSM                                         */
/* Entrée: le status du GSM                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Send_zmq_DI_to_master ( void *zmq, gchar *thread, gchar *tech_id, gchar *acronyme, gboolean etat )
  { JsonBuilder *builder;
    gchar *result;
    gsize taille;

    if (!zmq) return;
    builder = Json_create ();
    if(!builder) return;
    Json_add_string ( builder, "tech_id",  tech_id );
    Json_add_string ( builder, "acronyme", acronyme );
    Json_add_bool   ( builder, "etat", etat );
    result = Json_get_buf ( builder, &taille );
    Send_zmq_with_tag ( zmq, NULL, thread, "*", "msrv", "SET_DI", result, taille );
    g_free(result);
  }
/******************************************************************************************************************************/
/* Smsg_send_status_to_master: Envoie le bit de comm au master selon le status du GSM                                         */
/* Entrée: le status du GSM                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Send_zmq_AI_to_master ( void *zmq, gchar *thread, gchar *tech_id, gchar *acronyme, gfloat valeur, gboolean in_range)
  { JsonBuilder *builder;
    gchar *result;
    gsize taille;

    if (!zmq) return;
    builder = Json_create ();
    if(!builder) return;
    Json_add_string ( builder, "tech_id",  tech_id );
    Json_add_string ( builder, "acronyme", acronyme );
    Json_add_double ( builder, "valeur", valeur );
    Json_add_bool   ( builder, "in_range", in_range );
    result = Json_get_buf ( builder, &taille );
    Send_zmq_with_tag ( zmq, NULL, thread, "*", "msrv", "SET_AI", result, taille );
    g_free(result);
  }
/******************************************************************************************************************************/
/* Smsg_send_status_to_master: Envoie le bit de comm au master selon le status du GSM                                         */
/* Entrée: le status du GSM                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Send_zmq_CDE_to_master ( void *zmq, gchar *thread, gchar *tech_id, gchar *acronyme )
  { JsonBuilder *builder;
    gchar *result;
    gsize taille;

    if (!zmq) return;
    builder = Json_create ();
    if(!builder) return;
    Json_add_string ( builder, "tech_id",  tech_id );
    Json_add_string ( builder, "acronyme", acronyme );
    result = Json_get_buf ( builder, &taille );
    Send_zmq_with_tag ( zmq, NULL, thread, "*", "msrv", "SET_CDE", result, taille );
    g_free(result);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
