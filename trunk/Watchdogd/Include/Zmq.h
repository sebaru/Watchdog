/******************************************************************************************************************************/
/* Watchdogd/Zmq.h      Déclarations générales des fonctions d'échanges intra process ou externes                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                     08.01.2018 13:23:50*/
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Zmq.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

 #ifndef _ZMQ_H_
 #define _ZMQ_H_

 #include <glib.h>
 #include <zmq.h>

 #define ZMQUEUE_LIVE_MSGS       "live-msgs"
 #define ZMQUEUE_LIVE_MOTIFS     "live-motifs"
 #define ZMQUEUE_LOCAL_MASTER    "local_msrv"
 #define ZMQUEUE_LOCAL_BUS       "local_bus"

 struct ZMQUEUE
  { void *socket;
    gint pattern;
    gchar name[32];
    gchar endpoint[32];
  };

 struct ZMQ_TARGET
  { gchar src_instance[24];
    gchar src_thread[12];
    gchar dst_instance[24];
    gchar dst_thread[12];
    gchar tag[24];
  };

/************************************************ Définitions des prototypes **************************************************/
 extern struct ZMQUEUE *Connect_zmq ( gint pattern, gchar *name, gchar *type, gchar *endpoint, gint port );     /* Dans zmq.c */
 extern struct ZMQUEUE *Bind_zmq ( gint pattern, gchar *name, gchar *type, gchar *endpoint, gint port );
 extern void Close_zmq ( struct ZMQUEUE *zmq );
 extern gboolean Send_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille );
 extern gboolean Send_zmq_with_tag ( struct ZMQUEUE *zmq,
                                     const gchar *source_instance, const gchar *source_thread,
                                     const gchar *target_instance, const gchar *target_thread,
                                     const gchar *target_tag, void *source, gint taille );
 extern gint Recv_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille_buf );
 extern gint Recv_zmq_with_tag ( struct ZMQUEUE *zmq, const gchar *thread, void *buf, gint taille_buf,
                                 struct ZMQ_TARGET **event, void **payload );
 extern gboolean Zmq_instance_is_target ( struct ZMQ_TARGET *event );
 extern gboolean Zmq_other_is_target ( struct ZMQ_TARGET *event );
 extern void Send_zmq_DI_to_master ( void *zmq, gchar *thread, gchar *tech_id, gchar *acronyme, gboolean etat );
 extern void Send_zmq_AI_to_master ( void *zmq, gchar *thread, gchar *tech_id, gchar *acronyme, gfloat valeur, gboolean in_range);
 extern void Send_zmq_CDE_to_master ( void *zmq, gchar *thread, gchar *tech_id, gchar *acronyme );
 extern JsonNode *Recv_zmq_with_json ( struct ZMQUEUE *zmq, const gchar *thread, gchar *buf, gint taille_buf );
 extern gboolean Send_zmq_with_json ( struct ZMQUEUE *zmq, const gchar *zmq_source_thread,
                                      const gchar *zmq_target_instance, const gchar *zmq_target_thread,
                                      const gchar *zmq_tag, JsonBuilder *builder );
 extern gboolean Send_double_zmq_with_json ( struct ZMQUEUE *zmq1, struct ZMQUEUE *zmq2, const gchar *zmq_src_thread,
                                             const gchar *zmq_dst_instance, const gchar *zmq_dst_thread,
                                             const gchar *zmq_tag, JsonBuilder *builder );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
